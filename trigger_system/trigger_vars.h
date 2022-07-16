#pragma once
#include "SCBW/api.h"

struct TriggerVar {
	TriggerVar* hash_next;

	bool set;
	char name[32];
	s32 value;

	bool is_timer;
};

#define TriggerVarCap 1000
#define TriggerVarHashCap 250

extern u32 trigger_vars_count;
extern TriggerVar trigger_vars[TriggerVarCap];
extern TriggerVar* trigger_vars_hash[TriggerVarHashCap];

void init_trigger_vars();
void update_trigger_vars();

TriggerVar* find_or_create_trigger_var(char* name);