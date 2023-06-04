#include "trigger_objectives.h"

u32 objectives_count;
Objective objectives[ObjectivesCap];

void init_objectives() {
	objectives_count = 0;
	memset(&objectives, 0, sizeof(objectives));
}

void tick_objectives() {
	for (u32 i = 0; i < objectives_count; ++i) {
		Objective* objective = &objectives[i];
		if (objective->timer > 1) { // We treat 1 as elapsed, since we want timer to still show up. 0 means no timer should show.
			objective->timer--;
		}
	}
}

static s32 compare_objective(const void* a, const void* b) {
	Objective* objective_a = (Objective*) a;
	Objective* objective_b = (Objective*) b;
	return objective_a->key - objective_b->key;
}

Objective* find_objective(s32 key) {
	// Since we are always sorted by key, we could use a bisect here to be faster?
	// although objective counts are never even triple digit, so we would probably be slower?
	for (u32 i = 0; i < objectives_count; ++i) {
		if (objectives[i].key == key) {
			return &objectives[i];
		}
	}
	return 0;
}

Objective* find_or_create_objective(s32 key) {
	Objective* objective = find_objective(key);
	if (!objective && objectives_count < ObjectivesCap) {
		objective = &objectives[objectives_count++];
		*objective = {};
		objective->key = key;
		qsort(objectives, objectives_count, sizeof(Objective), compare_objective);
	}
	return objective;
}

void remove_objective(Objective* objective) {
	if (objectives_count) {
		*objective = objectives[--objectives_count];
		qsort(objectives, objectives_count, sizeof(Objective), compare_objective);
	}
}