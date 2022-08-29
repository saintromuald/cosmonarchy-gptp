#include "trigger_events.h"

TriggerEventArray kill_events;
TriggerEventArray combat_events;

void init_trigger_events() {
	memset(&kill_events, 0, sizeof(kill_events));
	memset(&combat_events, 0, sizeof(combat_events));
}

void trigger_event_array_add(TriggerEventArray* array, TriggerEvent event) {
	if (array->count < EventCap) {
		array->events[array->count++] = event;
		array->index = array->count;
	} else {
		array->events[array->index++ % EventCap] = event;
	}
}