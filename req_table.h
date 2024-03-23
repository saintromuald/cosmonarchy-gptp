#pragma once
#include "BW\api.h"

enum RequireUnitFlags {
	RequireUnitFlags_Connected		= (1 << 1),
	RequireUnitFlags_Completed		= (1 << 2),
	RequireUnitFlags_Landed			= (1 << 3),
};

struct RequireUnit {
	u16 flags;
	u16 unit_id;
};

#define RequireUnitCap 4
struct ReqEntry {
	u16 unit_id;
	u8 require_faction;
	u8 require_units_count;
	RequireUnit require_units[RequireUnitCap];
};

#define ReqEntryCap 9
struct ReqTableEntry {
	u16 units_count;
	ReqEntry units[ReqEntryCap];
};

extern ReqTableEntry req_table[UnitId::Count];

void init_req_table();

Button_State::Enum get_button_state(Button* button, Unit* unit, Button_State::Enum default_return);

inline bool player_has_unit_id(u32 player, u32 unit_id) {
	return Player::number_of_units_of_id(player, unit_id);
}

inline bool player_has_completed_unit_id(u32 player, u32 unit_id) {
	return Player::number_of_completed_units_of_id(player, unit_id);
}

inline bool player_has_completed_landed_unit_id(u32 player, u32 unit_id) {
	return Player::number_of_completed_landed_units_of_id(player, unit_id);
}

inline bool player_has_completed_addon_unit_id(u32 player, u32 unit_id) {
	return Player::number_of_completed_addon_units_of_id(player, unit_id);
}

inline bool player_has_require_unit(u32 player, RequireUnit* require_unit) {
	if (require_unit->unit_id == UnitId::None || require_unit->unit_id > UnitId::Count) {
		return 0;
	}
	u32 count = 0;
	for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
		if (unit->id != require_unit->unit_id) {
			continue;
		}
		if (require_unit->flags & RequireUnitFlags_Completed && !(unit->status & UnitStatus::Completed)) {
			continue;
		}
		if (require_unit->flags & RequireUnitFlags_Landed && unit->status & UnitStatus::InAir) {
			continue;
		}
		count++;
	}
	return count;
}
