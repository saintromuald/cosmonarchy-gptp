#include "trigger_events.h"

SmallArray<TriggerEvent, EventCap> kill_events;
SmallArray<TriggerEvent, EventCap> combat_events;

void init_trigger_events() {
	kill_events.clear();
	combat_events.clear();
}

