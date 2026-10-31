#include "frame_pacing.h"
#include "grid.h"
#include "perf_tracker.h"
#include "../../BW/bwhelper.h"
#include "../../config.h"
#include "../../systems/ui/tooltip.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>

#define NOMINMAX
#include <Windows.h>

namespace {
// --- Freeze watchdog --------------------------------------------------------
// The Unit Tester map froze the game thread in an infinite wait with no
// diagnostics: the loop simply stopped iterating (the game process sat at
// 0% CPU in NtWaitForSingleObject). A detached monitor thread watches the
// loop's heartbeat; when the loop stalls in-game for 15 s it suspends the
// game thread, captures its Windows-side context and stack, and appends the
// raw addresses plus the loop state to logs/gptp_freeze.log. The addresses
// resolve to module+offset afterwards (exe 0x400000, gptp.qdp base, etc.).
namespace freeze_watchdog {

volatile u32 g_heartbeat_ms = 0;
volatile bool g_started = false;
volatile DWORD g_game_thread_id = 0;

void monitor_thread() {
    u32 last_dump_ms = 0;
    for (;;) {
        Sleep(1000);
        const u32 now_ms = GetTickCount();
        if (!g_started || !g_game_active) {
            continue;
        }
        if (now_ms - g_heartbeat_ms < 15000 || now_ms - last_dump_ms < 10000) {
            continue;
        }
        last_dump_ms = now_ms;

        std::ostringstream out;
        out << "freeze watchdog: game loop stalled (" << (now_ms - g_heartbeat_ms)
            << " ms without a heartbeat)\n";
        out << "  tick_count=" << now_ms << " next_tick_ms=" << g_next_tick_ms
            << " turn_stalled=" << unsigned{g_turn_stalled}
            << " turn_ticks_left=" << g_turn_ticks_left
            << " gptp_base=0x" << std::hex << u32(reinterpret_cast<u32>(GetModuleHandleA("gptp.qdp")))
            << std::dec << "\n";

        HANDLE thread = OpenThread(
            THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,
            FALSE, g_game_thread_id);
        if (thread) {
            SuspendThread(thread);
            CONTEXT ctx{};
            ctx.ContextFlags = CONTEXT_FULL;
            if (GetThreadContext(thread, &ctx)) {
                out << "  eip=0x" << std::hex << ctx.Eip << " esp=0x" << ctx.Esp << std::dec << "\n";
                // The thread is suspended, so reading its Windows stack
                // directly is safe: the return addresses of the wait chain.
                const u32* sp = reinterpret_cast<u32*>(ctx.Esp);
                out << "  stack:";
                for (int i = 0; i < 24; i++) {
                    out << " 0x" << std::hex << sp[i];
                }
                out << std::dec << "\n";
            } else {
                out << "  GetThreadContext failed\n";
            }
            ResumeThread(thread);
            CloseHandle(thread);
        } else {
            out << "  OpenThread failed\n";
        }

        std::filesystem::create_directories("logs/");
        std::ofstream log{"logs/gptp_freeze.log", std::ios::app};
        if (log) {
            log << out.str() << std::endl;
        }
    }
}

// Called at the top of every loop iteration: heartbeat + one-time thread start.
void beat() {
    g_game_thread_id = GetCurrentThreadId();
    g_heartbeat_ms = GetTickCount();
    if (!g_started) {
        g_started = true;
        std::thread{monitor_thread}.detach();
    }
}

} // namespace freeze_watchdog

constexpr u32 ScrollStepMs = 10;
constexpr u32 DeclinedTickPollMs = 1;

// Target frame interval in microseconds from display refresh rate (EnumDisplaySettingsW) or config.
u32 redraw_interval_us() {
    static const u32 interval = []() -> u32 {
        if (g_config.ui_redraw_cap_hz != 0) {
            return 1000000u / g_config.ui_redraw_cap_hz;
        }

        DEVMODEW mode = {};
        mode.dmSize = sizeof(mode);
        if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode) && mode.dmDisplayFrequency > 1) {
            return 1000000u / mode.dmDisplayFrequency;
        }
        return 1000000u / 60u;
    }();
    return interval;
}

u32 monotonic_us() {
    static const LARGE_INTEGER frequency = []() {
        LARGE_INTEGER value;
        QueryPerformanceFrequency(&value);
        return value;
    }();
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    // Seconds and the sub-second remainder scale separately; scaling the raw
    // counter by 1000000 overflows s64 after days of uptime.
    const u64 ticks = u64(counter.QuadPart);
    const u64 ticks_per_second = u64(frequency.QuadPart);
    const u64 seconds = ticks / ticks_per_second;
    const u64 remainder = ticks % ticks_per_second;
    return u32(seconds * 1000000 + remainder * 1000000 / ticks_per_second);
}

u32 g_last_present_us;

// Tick presents are immediate; scroll steps and UI/mouse invalidations coalesce at
// redraw_interval_us. Scroll steps fire every ScrollStepMs (faster than refresh), so
// without the cap the full present pipeline runs up to 100x/sec while scrolling.
bool should_present(bool tick_present, bool world_changed, u32 now_us) {
    if (tick_present) {
        return true;
    }
    return now_us - g_last_present_us >= redraw_interval_us() && (world_changed || display::has_pending_draw());
}

// Milliseconds until next action (tick or scroll clock, capped at ScrollStepMs + 1).
u32 ms_until_due(u32 now_ms, bool tick_declined) {
    const s32 until_tick = s32(g_next_tick_ms - now_ms);
    if (until_tick <= 0) {
        return tick_declined ? DeclinedTickPollMs : 0;
    }
    const s32 until_scroll_step = s32(ScrollStepMs + 1 - (g_last_scroll_step_ms_negated + now_ms));
    s32 wait_ms = std::min(until_tick, std::max(until_scroll_step, 0));
    if (display::has_pending_draw()) {
        const s32 until_present_us = s32(g_last_present_us + redraw_interval_us() - monotonic_us());
        wait_ms = std::min(wait_ms, std::max((until_present_us + 999) / 1000, 0));
    }
    return u32(wait_ms);
}

// MsgWaitForMultipleObjects with MWMO_INPUTAVAILABLE wakes on input or timer expiration.
void sleep_until_due(u32 wait_ms) {
    if (wait_ms == 0) {
        return;
    }
    MsgWaitForMultipleObjectsEx(0, nullptr, wait_ms, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
}

} // namespace

namespace hooks {

void __cdecl game_loop_top() {
    g_turn_ticks_left = 1;
    g_turn_stalled = 0;
    g_next_tick_ms = GetTickCount();
    bool tick_declined = false;

    while (g_game_active) {
        freeze_watchdog::beat();
        sleep_until_due(ms_until_due(GetTickCount(), tick_declined));
        const u64 busy_start = perf::now_us();

        helper::pump_messages(3);
        const u32 now_ms = GetTickCount();

        // Tick clock snaps to current time if drift exceeds 3 ticks (g_tick_resync_threshold_ms).
        if (!g_turn_stalled && u32(std::abs(s32(g_next_tick_ms - now_ms))) > g_tick_resync_threshold_ms[g_options_list->speed]) {
            g_next_tick_ms = now_ms;
        }

        helper::update_cursor();

        bool world_changed = false;
        if (g_last_scroll_step_ms_negated + now_ms > ScrollStepMs) {
            g_last_scroll_step_ms_negated = 0u - now_ms;
            const v2s32 origin_before{g_move_to_x, g_move_to_y};
            helper::scroll_screen_step();
            world_changed = v2s32{g_move_to_x, g_move_to_y} != origin_before;
        }

        helper::receive_network_messages();
        helper::send_bnet_league_query();

        // Advances game tick; polls at DeclinedTickPollMs if tick declined (e.g. paused replay or turn not arrived).
        tick_declined = false;
        bool tick_present = false;
        if (s32(now_ms - g_next_tick_ms) >= 0) {
            const u64 tick_start = perf::now_us();
            const b32 ticked = helper::run_game_tick(0);
            perf::record_tick(u32(perf::now_us() - tick_start));
            const bool still_due = s32(now_ms - g_next_tick_ms) >= 0;
            tick_declined = still_due && !ticked;
            tick_present = ticked || !still_due;
            world_changed |= tick_present;
        }

        helper::update_hud_info();

        if (ui::g_game_tooltips) {
            ui::g_game_tooltips->tick(now_ms);
        }

        const u32 now_us = monotonic_us();
        if (should_present(tick_present, world_changed, now_us)) {
            g_wants_redraw = 0;
            g_last_present_us = now_us;
            const perf::PresentReason reason = tick_present ? perf::PresentReason::Tick
                : world_changed ? perf::PresentReason::Scroll : perf::PresentReason::Ui;
            const u64 present_start = perf::now_us();
            helper::refresh_region();
            perf::record_present(u32(perf::now_us() - present_start), reason);
        }

        perf::record_busy(perf::now_us() - busy_start);
    }
}

} // hooks
