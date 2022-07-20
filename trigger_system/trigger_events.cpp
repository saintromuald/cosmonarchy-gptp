#include "trigger_events.h"

u32 kill_events_count;
KillEvent kill_events[EventCap];

void init_trigger_events() {
	kill_events_count = 0;
	memset(kill_events, 0, sizeof(kill_events));
}

void update_trigger_events() {

}