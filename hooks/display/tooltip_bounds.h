#pragma once

// Adjusts tooltip boundary clamps in 00458850 (command card buttons; caller 00459030 in aise.qdp)
// and 00481510 (context help), and parks dismissed help offscreen (00481480) to avoid 0041CA00 dirty rects.
namespace hooks {

void inject_tooltip_bounds_hooks();

} // hooks
