#pragma once
#include "SCBW/api.h"

extern bool has_custom_name[PlayerLimit];

struct Objective {
	bool show;
	s32 score;
	u32 max_score;
	s32 key;
	char text[256];
};

#define ObjectivesCap 64
struct Objectives {
	u32 count;
	Objective entries[ObjectivesCap];
};
extern Objectives objectives;

void tick_trigger_system(bool in_game);
s32 execute_trigger_conditions(TriggerVector* trigger_vector); // 00489200

void inject_trigger_system_hooks();