#pragma once
#include "BW/api.h"

enum class SpawnType {
	None,
	Augur,
	AugurDamage,
	Terran,
	Zerg,
	Teleport,
	Burrow,
};

struct SpawnSequence {
	u32 count;
	u32 unit_id;
	u32 player;
	u32 type;
	u32 delay;
	u32 stagger;
	v2u16 source_pos;
	v2u16 move_pos;
};

#define SpawnSequenceCap 1000
extern u32 spawn_sequence_count;
extern SpawnSequence spawn_sequences[SpawnSequenceCap];

void init_spawn();
void update_spawn();