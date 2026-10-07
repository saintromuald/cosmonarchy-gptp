#include "frame_pacing.h"
#include "grid.h"
#include "../../BW/bwhelper.h"
#include "../../config.h"

#include <algorithm>
#include <cstdlib>

#define NOMINMAX
#include <Windows.h>

namespace {

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

// World changes present immediately; UI/mouse invalidations are rate-limited to redraw_interval_us.
bool should_present(bool world_changed, u32 now_us) {
    if (world_changed) {
        return true;
    }
    return now_us - g_last_present_us >= redraw_interval_us() && hooks::has_pending_draw();
}

// Milliseconds until next action (tick or scroll clock, capped at ScrollStepMs + 1).
u32 ms_until_due(u32 now_ms, bool tick_declined) {
    const s32 until_tick = s32(g_next_tick_ms - now_ms);
    if (until_tick <= 0) {
        return tick_declined ? DeclinedTickPollMs : 0;
    }
    const s32 until_scroll_step = s32(ScrollStepMs + 1 - (g_last_scroll_step_ms_negated + now_ms));
    s32 wait_ms = std::min(until_tick, std::max(until_scroll_step, 0));
    if (hooks::has_pending_draw()) {
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
        sleep_until_due(ms_until_due(GetTickCount(), tick_declined));

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
        if (s32(now_ms - g_next_tick_ms) >= 0) {
            const b32 ticked = helper::run_game_tick(0);
            const bool still_due = s32(now_ms - g_next_tick_ms) >= 0;
            tick_declined = still_due && !ticked;
            world_changed |= ticked || !still_due;
        }

        helper::update_hud_info();

        const u32 now_us = monotonic_us();
        if (should_present(world_changed, now_us)) {
            g_wants_redraw = 0;
            g_last_present_us = now_us;
            helper::refresh_region();
        }
    }
}

} // hooks
