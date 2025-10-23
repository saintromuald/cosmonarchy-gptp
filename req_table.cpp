#include "req_table.h"
#include "trigger_system\trigger_system.h"
#include "hooks\grafting\button_functions.h"

ReqTableEntry req_table[UnitId::Count];
static bool req_table_init;

static ReqEntry* last_unit_req_entry;

static ReqEntry* add_req_entry(u16 factory_unit_id, u16 unit_id) {
	ReqTableEntry* entry = &req_table[factory_unit_id];
	if (entry->units_count >= ReqEntryCap) {
		return 0;
	}
	ReqEntry* unit = &entry->units[entry->units_count++];
	unit->unit_id = unit_id;
	last_unit_req_entry = unit;
	return unit;
}

static RequireUnit* require_unit(u16 unit_id) {
	ReqEntry* req_entry = last_unit_req_entry;
	if (req_entry->require_units_count >= RequireUnitCap) {
		return 0;
	}
	RequireUnit* unit = &req_entry->require_units[req_entry->require_units_count++];
	unit->unit_id = unit_id;
	unit->flags = RequireUnitFlags_Completed;
	return unit;
}

static RequireUnit* require_addon(u16 unit_id) {
	RequireUnit* unit = require_unit(unit_id);
	if (unit) {
		unit->flags |= RequireUnitFlags_Connected;
	}
}

static RequireUnit* require_landed(u16 unit_id) {
	RequireUnit* unit = require_unit(unit_id);
	if (unit) {
		unit->flags |= RequireUnitFlags_Landed;
	}
}

static RequireUnit* require_not_completed(u16 unit_id) {
	RequireUnit* unit = require_unit(unit_id);
	if (unit) {
		unit->flags &= ~RequireUnitFlags_Completed;
	}
}

void init_req_table() {
	if (req_table_init) {
		return;
	}
	req_table_init = true;

	add_req_entry(UnitId::TerranMantle, UnitId::TerranRamesses);
	add_req_entry(UnitId::TerranMantle, UnitId::TerranDurendal);
	add_req_entry(UnitId::TerranMantle, UnitId::TerranCataphract);

	add_req_entry(UnitId::TerranApothecary, UnitId::TerranDilettante);
	add_req_entry(UnitId::TerranApothecary, UnitId::TerranSevenstep);
	add_req_entry(UnitId::TerranApothecary, UnitId::TerranHypnagogue);
	add_req_entry(UnitId::TerranApothecary, UnitId::TerranSilvertongue);

	add_req_entry(UnitId::TerranStarport, UnitId::TerranPythean);
}

static u32 button_state_compare_value(Button_State::Enum state) {
	if (state == Button_State::Invisible) return 0;
	if (state == Button_State::Disabled)  return 1;
	return 2;
}

static void update_button_state_result(Button_State::Enum* result, Button_State::Enum new_result) {
	if (button_state_compare_value(new_result) < button_state_compare_value(*result)) {
		*result = new_result;
	}
}

Button_State::Enum get_button_state(Button* button, Unit* unit, Button_State::Enum default_return) {
	if (!unit || unit->id == UnitId::None || unit->id > UnitId::Count) {
		return default_return;
	}
	if (button->requirement_func != (ReqFunc*)ButtonRequirements::Can_Create_UnitorBuilding) {
		return default_return;
	}

	ReqTableEntry* entry = &req_table[unit->id];
	ReqEntry* unit_entry = 0;
	for (u32 i = 0; i < entry->units_count; ++i) {
		if (entry->units[i].unit_id == button->requirement_var) {
			unit_entry = &entry->units[i];
			break;
		}
	}
	if (!unit_entry) {
		return default_return;
	}

	bool has_addon = unit->is_building() && unit->building.addon;
	bool in_air = unit->is_in_air();
	bool is_training = unit->build_queue[unit->build_queue_slot] != UnitId::None;
	bool is_burrowed = unit->is_burrowed();
	bool is_adding_on = unit->building.addon_build_id && unit->main_order_id == OrderId::BuildAddon;

	Button_State::Enum result = Button_State::Enabled;
	if (units_dat::base_property[unit_entry->unit_id] & UnitProperty::Addon) {
		if (in_air || is_training || has_addon) {
			update_button_state_result(&result, Button_State::Invisible);
		}
	}

	if (!unit->is_grounded_building()) {
		update_button_state_result(&result, Button_State::Invisible);
	}
	
	for (u32 i = 0; i < unit_entry->require_units_count; ++i) {
		RequireUnit* require_unit = &unit_entry->require_units[i];
		if (require_unit->flags & RequireUnitFlags_Connected) {
			if (!bw::all_selected_units_have_addon(require_unit->unit_id)) {
				update_button_state_result(&result, Button_State::Invisible);
			}
		} else {
			if (!player_has_require_unit(unit->player, require_unit)) {
				update_button_state_result(&result, Button_State::Disabled);
			}
		}
	}

	if (unit_entry->require_faction != Faction::None && g_globals->players[unit->player].faction != unit_entry->require_faction) {
		update_button_state_result(&result, Button_State::Invisible);
	}

	return result;
}