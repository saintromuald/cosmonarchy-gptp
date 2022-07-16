#include "trigger_vars.h"
#include "trigger_utils.h"

TriggerVars trigger_vars[TriggerVarType_Count];

void init_trigger_vars() {
	memset(trigger_vars, 0, sizeof(trigger_vars));
}

void update_trigger_vars() {
	for (u32 i = 0; i < trigger_vars[TriggerVarType_Timer].count; ++i) {
		TriggerVar* var = &trigger_vars[TriggerVarType_Timer].vars[i];
		if (var && var->value) {
			var->value--;
		}
	}
}

TriggerVar* find_trigger_var(TriggerVarType type, char* name) {
	u32 hash = hash_string(name);
	for (TriggerVar* var = trigger_vars[type].hash[hash % TriggerVarHashCap]; var; var = var->hash_next) {
		if (strcmp(var->name, name) == 0) {
			return var;
		}
	}
	return 0;
}

TriggerVar* find_or_create_trigger_var(TriggerVarType type, char* name) {
	TriggerVar* var = find_trigger_var(type, name);
	if (var) {
		return var;
	}
	if (trigger_vars[type].count < TriggerVarCap) {
		var = &trigger_vars[type].vars[trigger_vars[type].count++];

		snprintf(var->name, 32, "%s", name);
		var->value = 0;

		u32 var_hash = hash_string(var->name); // it's important to take var->name, since it might be different from name due to size limitation
		var->hash_next = trigger_vars[type].hash[var_hash % TriggerVarHashCap];
		trigger_vars[type].hash[var_hash % TriggerVarHashCap] = var;
	}
	return var;
}

bool unset_trigger_var(TriggerVarType type, char* name) {
	u32 hash = hash_string(name);
	TriggerVar* prev = 0;
	for (TriggerVar* var = trigger_vars[type].hash[hash % TriggerVarHashCap]; var; var = var->hash_next) {
		if (strcmp(var->name, name) == 0) {
			if (prev) {
				prev->hash_next = var->hash_next;
			} else {
				trigger_vars[type].hash[hash % TriggerVarHashCap] = var->hash_next;
			}
			return true;
		}
		prev = var;
	}
	return false;
}