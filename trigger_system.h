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

enum SpawnType {
	SpawnType_None,
	SpawnType_Augur,
	SpawnType_AugurDamage,

	SpawnType_Count,
};

const char* spawn_type_names[SpawnType_Count] = {
	"None",
	"Augur",
	"AugurDamage",
};

void tick_trigger_system(bool in_game);

void inject_trigger_system_hooks();
