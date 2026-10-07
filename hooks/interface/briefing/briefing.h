#pragma once

namespace bw { struct Dialog; }

namespace hooks{
void update_mission_objectives(); // 0046CD30

void init_briefing_portrait_window(bw::Dialog* window); // 0044EF00
void inject_briefing_hooks();
}
