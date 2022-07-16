#pragma once
#include "SCBW/api.h"

struct TriggerVar {
	TriggerVar* hash_next;

	char name[32];
	s32 value;

	bool is_timer;
	bool timer_expired_this_frame;
};

#define TriggerVarCap 1000
#define TriggerVarHashCap 250

extern u32 trigger_vars_count;
extern TriggerVar trigger_vars[TriggerVarCap];
extern TriggerVar* trigger_vars_hash[TriggerVarHashCap];

void init_trigger_vars();
void update_trigger_vars();

TriggerVar* get_trigger_var(char* name);