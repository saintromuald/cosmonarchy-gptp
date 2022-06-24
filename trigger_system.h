#pragma once
#include "SCBW/api.h"

extern bool has_custom_name[PlayerLimit];

struct Objective {
	bool show;
	u32 score;
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