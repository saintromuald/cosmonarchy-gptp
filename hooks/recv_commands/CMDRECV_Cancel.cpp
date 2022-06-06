#include "CMDRECV_Cancel.h"
#include <SCBW/api.h>
#include "hooks\orders\building_making\building_morph.h"

namespace hooks
{
	// hooked to allow non-ghosts units to cancel nukes
	void CMDRECV_CancelNuke()
	{
		*unit_iterator_pos = 0;
		Unit* current_unit = get_active_player_next_selection();
		while (current_unit != nullptr)
		{
			Unit* nuke = current_unit->connected_unit;
			if (nuke != nullptr && nuke->id == UnitId::TerranNuclearMissile)
			{
				current_unit->connected_unit = nullptr;
				nuke->connected_unit = nullptr;
			}
			current_unit = get_active_player_next_selection();
		}
	}

	// hooked because the default code didn't allow for multiple unit
	void CMDRECV_CancelAddon()
	{
		*unit_iterator_pos = 0;
		for (Unit* unit = get_active_player_next_selection(); unit; unit = get_active_player_next_selection()) {
			if (unit->secondary_order_id == OrderId::BuildAddon
				&& unit->status & UnitStatus::GroundedBuilding)
			{
				Unit* addon = unit->current_build_unit;
				if (addon != nullptr
					&& !(addon->status & UnitStatus::Completed)
					&& unit->status & UnitStatus::Completed
					&& unit->player == *ACTIVE_NATION_ID)
				{
					helper::cancel_addon_proc(unit);
					scbw::refresh_console();
				}
			}
		}
	}

	// hooked because the default code didn't allow for multiple unit
	void CMDRECV_CancelUpgrade()
	{
		*unit_iterator_pos = 0;
		for (Unit* unit = get_active_player_next_selection(); unit; unit = get_active_player_next_selection()) {
			if (unit->status & UnitStatus::Completed
				&& unit->player == *ACTIVE_NATION_ID)
			{
				helper::cancel_upgrade_proc(unit);
				scbw::refresh_console();
			}
		}
	}

	// hooked because the default code didn't allow for multiple unit
	void CMDRECV_CancelResearch()
	{
		*unit_iterator_pos = 0;
		for (Unit* unit = get_active_player_next_selection(); unit; unit = get_active_player_next_selection()) {
			if (units_dat::BaseProperty[unit->id] & UnitProperty::Building
				&& unit->status & UnitStatus::Completed
				&& unit->player == *ACTIVE_NATION_ID)
			{
				helper::CancelResearch_Proc(unit);
				scbw::refresh_console();
			}
		}
	}

	// hooked because the default code didn't allow for multiple unit
	void CMDRECV_CancelTrain(u16 wUnitID)
	{
		*unit_iterator_pos = 0;
		for (Unit* unit = get_active_player_next_selection(); unit; unit = get_active_player_next_selection()) {
			if (unit->id != UnitId::ZergLarva
				&& unit->id != UnitId::ZergMutalisk
				&& unit->id != UnitId::ZergHydralisk
				&& unit->id != UnitId::ZergHatchery
				&& unit->id != UnitId::ZergLair
				&& unit->id != UnitId::ZergMuthrokSpire
				&& unit->id != UnitId::ZergCreepColony
				&& unit->status & UnitStatus::Completed
				&& unit->player == *ACTIVE_NATION_ID)
			{
				if (wUnitID == 0xFE)
				{
					helper::cancel_last_training(unit);
					scbw::refresh_console();
				}
				else if (wUnitID == 0xFF)
				{
					scbw::refresh_console();
				}
				else
				{
					helper::refund_queue_slot(unit, wUnitID);
					scbw::refresh_console();
				}
			}
		}
	}

	void CMDRECV_CancelUnitMorph() {
		*unit_iterator_pos = 0;
		for (Unit* unit = get_active_player_next_selection(); unit; unit = get_active_player_next_selection()) {
			if (unit->player == *ACTIVE_NATION_ID) {
				helper::ai_cancel_structure(unit);
			}
		}
	}

	// hooked because the default code didn't allow for multiple unit
	void CMDRECV_CancelConstruction() {
		*unit_iterator_pos = 0;
		for (Unit* unit = get_active_player_next_selection(); unit; unit = get_active_player_next_selection()) {
			//if (unit->id == 158 || unit->id == 161) {
			//	if (unit->get_overlay(ImageId::WarpAnchor)) {
			//		unit->remove_overlay(ImageId::WarpAnchor);
			//	}
			//}

			if (unit->player == *ACTIVE_NATION_ID) {
				helper::ai_cancel_structure(unit);
			}
		}
	}
}