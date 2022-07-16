#pragma once
#include "SCBW/api.h"

enum TriggerVarType {
	TriggerVarType_Var,
	TriggerVarType_Timer,
	TriggerVarType_Count,
};

struct TriggerVar {
	TriggerVar* hash_next;

	char name[32];
	s32 value;
};

#define TriggerVarCap 1000
#define TriggerVarHashCap 250
struct TriggerVars {
	u32 count;
	TriggerVar vars[TriggerVarCap];
	TriggerVar* hash[TriggerVarHashCap];
};

extern TriggerVars trigger_vars[TriggerVarType_Count];

void init_trigger_vars();
void update_trigger_vars();

TriggerVar* find_trigger_var(TriggerVarType type, char* name);
TriggerVar* find_or_create_trigger_var(TriggerVarType type, char* name);
bool unset_trigger_var(TriggerVarType type, char* name);