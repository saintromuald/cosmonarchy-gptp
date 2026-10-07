#pragma once
#include "../types.h"

// HUD and dialog geometry render at 2x; 1x is one rebuild away (see
// project_plans/ui-2x-hud.md). Band art must match this factor.
namespace ui_scale {
constexpr u32 Factor = 2;
}
