#include "perf_tracker.h"

#define NOMINMAX
#include <Windows.h>

namespace {

constexpr u64 WindowUs = 1000000;

struct Accumulator {
    u32 count = 0;
    u64 total_us = 0;
    u32 max_us = 0;

    void record(u32 us) {
        ++count;
        total_us += us;
        if (us > max_us) {
            max_us = us;
        }
    }

    u32 avg_us() const {
        return count ? u32(total_us / count) : 0;
    }
};

perf::Window g_last_window;
u64 g_window_start_us = 0;
u64 g_busy_total_us = 0;
u32 g_dirty_percent = 0;
Accumulator g_tick;
Accumulator g_present;
Accumulator g_resample;
Accumulator g_blit;
u32 g_presents_tick = 0;
u32 g_presents_scroll = 0;
u32 g_presents_ui = 0;

// Resample and blit run at most once per present, so their averages divide by
// the present count; otherwise a sparse window would read near zero.
u32 avg_over_presents(const Accumulator& part) {
    return g_present.count ? u32(part.total_us / g_present.count) : 0;
}

void roll_window(u64 now) {
    const u64 elapsed_us = now - g_window_start_us;

    g_last_window.tick_avg_us = g_tick.avg_us();
    g_last_window.tick_max_us = g_tick.max_us;
    g_last_window.present_avg_us = g_present.avg_us();
    g_last_window.present_max_us = g_present.max_us;
    g_last_window.resample_avg_us = avg_over_presents(g_resample);
    g_last_window.blit_avg_us = avg_over_presents(g_blit);
    g_last_window.busy_percent = elapsed_us ? u32(g_busy_total_us * 100 / elapsed_us) : 0;
    g_last_window.dirty_percent = g_dirty_percent;
    g_last_window.present_count = g_present.count;
    g_last_window.present_tick = g_presents_tick;
    g_last_window.present_scroll = g_presents_scroll;
    g_last_window.present_ui = g_presents_ui;

    g_tick = {};
    g_present = {};
    g_resample = {};
    g_blit = {};
    g_busy_total_us = 0;
    g_presents_tick = 0;
    g_presents_scroll = 0;
    g_presents_ui = 0;
    g_window_start_us = now;
}

} // namespace

namespace perf {

u64 now_us() {
    static const LARGE_INTEGER frequency = []() {
        LARGE_INTEGER value;
        QueryPerformanceFrequency(&value);
        return value;
    }();
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    // Seconds and the sub-second remainder scale separately; scaling the raw
    // counter by 1000000 overflows u64 after days of uptime.
    const u64 ticks = u64(counter.QuadPart);
    const u64 ticks_per_second = u64(frequency.QuadPart);
    const u64 seconds = ticks / ticks_per_second;
    const u64 remainder = ticks % ticks_per_second;
    return seconds * 1000000 + remainder * 1000000 / ticks_per_second;
}

void record_busy(u64 busy_us) {
    const u64 now = now_us();
    if (!g_window_start_us) {
        g_window_start_us = now;
    }
    g_busy_total_us += busy_us;
    if (now - g_window_start_us >= WindowUs) {
        roll_window(now);
    }
}

void record_tick(u32 us) {
    g_tick.record(us);
}

void record_present(u32 us, PresentReason reason) {
    g_present.record(us);
    switch (reason) {
        case PresentReason::Tick:
            ++g_presents_tick;
            break;
        case PresentReason::Scroll:
            ++g_presents_scroll;
            break;
        case PresentReason::Ui:
            ++g_presents_ui;
            break;
    }
}

void record_resample(u32 us) {
    g_resample.record(us);
}

void record_blit(u32 us) {
    g_blit.record(us);
}

void record_dirty_cells(u32 dirty, u32 total) {
    g_dirty_percent = total ? u32(u64(dirty) * 100 / total) : 0;
}

const Window& last_window() {
    return g_last_window;
}

} // namespace perf