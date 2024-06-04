#pragma once
#include "BW/api.h"

enum SpawnType {
	SpawnType_None,
	SpawnType_Augur,
	SpawnType_AugurDamage,
	SpawnType_Terran,
	SpawnType_Zerg,
	SpawnType_Teleport,
	SpawnType_Burrow,

	SpawnType_Count,
};

const char* spawn_type_names[SpawnType_Count] = {
	"None",
	"Augur",
	"AugurDamage",
	"Terran",
	"Zerg",
	"Teleport",
	"Burrow",
};

struct SpawnSequence {
	u32 count;
	u32 unit_id;
	u32 player;
	u32 type;
	u32 delay;
	u32 stagger;
	Point16 source_pos;
	Point16 move_pos;
};

#define SpawnSequenceCap 1000
extern u32 spawn_sequence_count;
extern SpawnSequence spawn_sequences[SpawnSequenceCap];

void init_spawn();
void update_spawn();