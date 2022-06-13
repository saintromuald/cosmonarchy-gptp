#pragma once
#include "SCBW/api.h"

extern bool has_custom_name[PlayerLimit];

#define ObjectivesCap 64
struct Objectives {
	u32 count;
	Token entries[ObjectivesCap];
};
extern Objectives objectives;

void tick_trigger_system(bool in_game);