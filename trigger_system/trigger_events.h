#pragma once
#include "SCBW/api.h"

struct TriggerEvent {
	u8 seen_by;
	u8 source_player;
	u8 target_player;
	u16 source_unit_id;
	u16 target_unit_id;
	u32 timestamp;
	Point16 position;
};

#define EventCap 100'000
struct TriggerEventArray {
	u32 count;
	u32 index;
	TriggerEvent events[EventCap];
};

extern TriggerEventArray kill_events;
extern TriggerEventArray combat_events;

void init_trigger_events();
void trigger_event_array_add(TriggerEventArray* array, TriggerEvent event);