#pragma once
#include "SCBW/api.h"
#include "collections.h"

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
extern SmallArray<TriggerEvent, EventCap> kill_events;
extern SmallArray<TriggerEvent, EventCap> combat_events;

void init_trigger_events();
