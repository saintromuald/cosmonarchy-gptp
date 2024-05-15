#pragma once
#include "BW/api.h"
#include "CM/cm.h"

void init_upstart_mode();
void end_upstart_mode();
void init_upstart_spawn(CHKUnit* unit, u32 flags);
void spawn_upstarts();
u32 upstart_spawner_basic(u32 index);
u32 upstart_spawn_basic(u32 spawner_id);