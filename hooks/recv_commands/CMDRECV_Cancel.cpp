#include "CMDRECV_Cancel.h"
#include <SCBW/api.h>
#include "hooks\orders\building_making\building_morph.h"

namespace hooks
{
	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_nuke() {
		*unit_iterator_pos = 0;
		Unit* unit = get_active_player_next_selection();
		while (unit) {
			Unit* nuke = unit->connected_unit;
			if (nuke && nuke->id == UnitId::TerranNuclearMissile) {
				unit->connected_unit = nullptr;
				nuke->connected_unit = nullptr;
			}
			unit = get_active_player_next_selection();
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_addon() {
		*unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (unit->secondary_order_id == OrderId::BuildAddon
				&& unit->status & UnitStatus::GroundedBuilding) {
				Unit* addon = unit->current_build_unit;
				if (addon &&  !(addon->status & UnitStatus::Completed)
				&&  unit->status & UnitStatus::Completed
				&&  unit->player == *ACTIVE_NATION_ID) {
					helper::cancel_addon_proc(unit);
					scbw::refresh_console();
				}
			}
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_upgrade() {
		*unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (unit->status & UnitStatus::Completed && unit->player == *ACTIVE_NATION_ID) {
				helper::cancel_upgrade_proc(unit);
				scbw::refresh_console();
			}
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_research() {
		*unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (units_dat::BaseProperty[unit->id] & UnitProperty::Building
			&&  unit->status & UnitStatus::Completed
			&&  unit->player == *ACTIVE_NATION_ID) {
				helper::cancel_research_proc(unit);
				scbw::refresh_console();
			}
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_train(u16 wUnitID) {
		*unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (unit->id != UnitId::ZergLarva
			&&  unit->id != UnitId::ZergMutalisk
			&&  unit->id != UnitId::ZergHydralisk
			&&  unit->id != UnitId::ZergHatchery
			&&  unit->id != UnitId::ZergLair
			&&  unit->id != UnitId::ZergMuthrokSpire
			&&  unit->id != UnitId::ZergCreepColony
			&&  unit->status & UnitStatus::Completed
			&&  unit->player == *ACTIVE_NATION_ID) {
				if (wUnitID == 0xFE) {
					helper::cancel_last_training(unit);
					scbw::refresh_console();
				} else if (wUnitID == 0xFF) {
					scbw::refresh_console();
				} else {
					helper::refund_queue_slot(unit, wUnitID);
					scbw::refresh_console();
				}
			}
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_unit_morph() {
		*unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (unit->player == *ACTIVE_NATION_ID) {
				helper::ai_cancel_structure(unit);
			}
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_construction() {
		*unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (unit->player == *ACTIVE_NATION_ID) {
				helper::ai_cancel_structure(unit);
			}
		}
	}
}