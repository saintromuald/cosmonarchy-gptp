#pragma once
#include "BW/api.h"

struct Objective {
	bool show;
	s32 score;
	u32 max_score;
	s32 key;
	u32 timer;
	char text[256];
};

#define ObjectivesCap 64
extern u32 objectives_count;
extern Objective objectives[ObjectivesCap];

void init_objectives();
void tick_objectives();

Objective* find_objective(s32 key);
Objective* find_or_create_objective(s32 key);
void remove_objective(Objective* objective);