#include "CMDRECV_Cancel.h"
#include <BW/api.h>
#include "hooks\orders\building_making\building_morph.h"

namespace hooks
{
	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_nuke() {
		*g_unit_iterator_pos = 0;
		Unit* unit = get_active_player_next_selection();
		while (unit) {
			Unit* nuke = unit->connected_unit;
			if (nuke && nuke->id == UnitId::TerranMorningstar) {
				unit->connected_unit = nullptr;
				nuke->connected_unit = nullptr;
			}
			unit = get_active_player_next_selection();
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_addon() {
		*g_unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (unit->secondary_order_id == OrderId::BuildAddon
				&& unit->status & UnitStatus::GroundedBuilding) {
				Unit* addon = unit->current_build_unit;
				if (addon &&  !(addon->status & UnitStatus::Completed)
				&&  unit->status & UnitStatus::Completed
				&&  unit->player == *g_active_nation_id) {
					helper::cancel_addon_proc(unit);
					bw::refresh_console();
				}
			}
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_upgrade() {
		*g_unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (unit->status & UnitStatus::Completed && unit->player == *g_active_nation_id) {
				helper::cancel_upgrade_proc(unit);
				bw::refresh_console();
			}
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_research() {
		*g_unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (units_dat::base_property[unit->id] & UnitProperty::Building
			&&  unit->status & UnitStatus::Completed
			&&  unit->player == *g_active_nation_id) {
				helper::cancel_research_proc(unit);
				bw::refresh_console();
			}
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_train(u16 slot) // 004C0100
	{
		*g_unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection())
		{
			if (unit->id != UnitId::ZergLarva
			&&  unit->id != UnitId::ZergOcsalLarva
			&& !bw::is_conventional_unit_morph_source(unit->id)
			&&  unit->status & UnitStatus::Completed
			&&  unit->player == *g_active_nation_id)
			{
				if (slot == 254) {
					helper::cancel_last_training(unit);
					bw::refresh_console();
				} else if (slot == 255) {
					bw::refresh_console();
				} else {
					helper::refund_queue_slot(unit, slot);
					bw::refresh_console();
				}
			}
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_unit_morph() {
		*g_unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (unit->player == *g_active_nation_id) {
				helper::cancel_unit(unit);
			}
		}
	}

	// Hooked because the default code didn't allow for multiple units
	void cmdrecv_cancel_construction() {
		*g_unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection()) {
			if (unit->player == *g_active_nation_id) {
				helper::cancel_unit(unit);
			}
		}
	}
}