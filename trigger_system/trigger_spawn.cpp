#include "trigger_spawn.h"

u32 spawn_sequence_count;
SpawnSequence spawn_sequences[SpawnSequenceCap];

void init_spawn() {
	spawn_sequence_count = 0;
	memset(spawn_sequences, 0, sizeof(spawn_sequences));
}

void update_spawn() {

}