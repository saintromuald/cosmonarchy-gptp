#include "trigger_events.h"

StaticArray<TriggerEvent, EventCap> kill_events;
StaticArray<TriggerEvent, EventCap> combat_events;

void init_trigger_events() {
	kill_events.clear();
	combat_events.clear();
}

