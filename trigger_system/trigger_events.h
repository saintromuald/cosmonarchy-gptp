#pragma once
#include "SCBW/api.h"

struct KillEvent {
	u8 seen_by;
	u8 dying_player;
	u8 killing_player;
	u16 dying_unit_id;
	u16 killing_unit_id;
	u32 timestamp;
};

//struct CombatEvent {
//	u8 receiving_player;
//	u8 dealing_player;
//	u16 receiving_unit_id;
//	u16 dealing_unit_id;
//	u32 timestamp[8];
//};

#define EventCap 1'000'000
extern u32 kill_events_count;
extern KillEvent kill_events[EventCap];
//CombatEvent combat_events[EventCap];

void init_trigger_events();
void update_trigger_events();