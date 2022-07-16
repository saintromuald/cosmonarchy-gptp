#include "trigger_vars.h"
#include "trigger_utils.h"

u32 trigger_vars_count;
TriggerVar trigger_vars[TriggerVarCap];
TriggerVar* trigger_vars_hash[TriggerVarHashCap];

void init_trigger_vars() {
	trigger_vars_count = 0;
	memset(trigger_vars, 0, sizeof(trigger_vars));
	memset(trigger_vars_hash, 0, sizeof(trigger_vars_hash));
}

void update_trigger_vars() {
	for (u32 i = 0; i < trigger_vars_count; ++i) {
		TriggerVar* var = &trigger_vars[i];
		if (var->is_timer) {
			if (var->value) {
				var->value--;
				if (!var->value) {
					var->timer_expired_this_frame = true;
				}
			} else {
				var->timer_expired_this_frame = false;
			}
		}
	}
}

TriggerVar* get_trigger_var(char* name) {
	u32 hash = hash_string(name);
	for (TriggerVar* var = trigger_vars_hash[hash % TriggerVarHashCap]; var; var = var->hash_next) {
		if (strcmp(var->name, name) == 0) {
			return var;
		}
	}
	if (trigger_vars_count < TriggerVarCap) {
		TriggerVar* var = &trigger_vars[trigger_vars_count++];

		var->hash_next = trigger_vars_hash[hash % TriggerVarHashCap];
		trigger_vars_hash[hash % TriggerVarHashCap] = var;

		snprintf(var->name, 32, "%s", name);
		var->value = 0;
		var->is_timer = false;
	}
	return 0;
}