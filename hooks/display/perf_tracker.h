#pragma once
#include "../../types.h"

// In-game performance tracker. frame_pacing records loop busy time, tick cost,
// and present cost per reason; grid records the zoom resample, the storm blit,
// and dirty-cell coverage. Samples roll into a completed one-second window that
// draw_ui_features prints when config show_perf_stats is set.
namespace perf {

enum class PresentReason : u8 { Tick, Scroll, Ui };

u64 now_us();

// Called once per loop iteration with the awake time; also rolls the window.
void record_busy(u64 busy_us);

void record_tick(u32 us);
void record_present(u32 us, PresentReason reason);
void record_resample(u32 us);
void record_blit(u32 us);
void record_dirty_cells(u32 dirty, u32 total);

// Latest completed one-second window.
struct Window {
    u32 tick_avg_us = 0;
    u32 tick_max_us = 0;
    u32 present_avg_us = 0;
    u32 present_max_us = 0;
    u32 resample_avg_us = 0;
    u32 blit_avg_us = 0;
    u32 busy_percent = 0;
    u32 dirty_percent = 0;
    u32 present_count = 0;
    u32 present_tick = 0;
    u32 present_scroll = 0;
    u32 present_ui = 0;
};

const Window& last_window();

} // namespace perf