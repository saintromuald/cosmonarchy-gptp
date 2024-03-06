#include "give_unit.h"
#include "globals.h"
#include <BW/api.h>
#include "ai_layouts.h"

namespace hooks
{
	// Used in itself, complete_addon, begin_gas_building, GiveAllUnitPropertiesToPlayerAndMakeIdle, removeSubunitOrGasContainer, ai_create_unit, orders_RescuePassive, orders_InfestMine1
	// Changes would affect all those functions and maybe more
	void give_unit(Unit* unit, u32 player, u32 count_as_completed)
	{
		if (unit->player != player)
		{
			helper::remove_unit_ai(unit, 0);

			bool is_morphing = false;
			if (unit->is_remorphing_building())
			{
				unit->status |= UnitStatus::Completed;
				is_morphing = true;
			}

			helper::increment_unit_scores(unit, -1);
			if (unit->status & UnitStatus::Completed)
			{
				helper::increment_unit_scores_ex(unit, 0, -1);
			}

			u32 selection_index = 0;
			bool isSelected = (unit->sprite->flags & SpriteFlags::Selected) == 1;
			if (isSelected)
			{
				selection_index = unit->sprite->selection_index;
				helper::destroy_unit_selection(unit);
			}

			if (unit->player_link.prev)
			{
				(unit->player_link.prev)->player_link.next = unit->player_link.next;
			}
			if (unit->player_link.next)
			{
				(unit->player_link.next)->player_link.prev = unit->player_link.prev;
			}
			if (unit == first_player_unit->unit[unit->player])
			{
				first_player_unit->unit[unit->player] = unit->player_link.next;
			}

			unit->player_link.prev = nullptr;
			unit->player_link.next = nullptr;
			unit->player = player;

			// maybe "if" not needed?
			if (isSelected)
			{
				helper::create_unit_selection(unit, selection_index);
			}

			unit->player_link.prev = nullptr;
			unit->player_link.next = first_player_unit->unit[player];

			if (first_player_unit->unit[player])
			{
				(first_player_unit->unit[player])->player_link.prev = unit;
			}

			first_player_unit->unit[player] = unit;
			helper::increment_unit_scores(unit, 1);
			if (unit->status & UnitStatus::Completed)
			{
				helper::increment_unit_scores_ex(unit, count_as_completed, 1);
			}
			if (is_morphing)
			{
				unit->status &= ~UnitStatus::Completed;
			}

			helper::recheck_attacks(unit);
			bw::refresh_console();
			if (units_dat::group_flags[unit->id].is_zerg != 1
				&& units_dat::group_flags[unit->id].is_protoss != 1
				&& unit->status & UnitStatus::GroundedBuilding)
			{
				*g_can_update_powered_status = 1;
			}

			helper::give_ai(unit);
			if (unit->subunit && units_dat::base_property[unit->subunit->id] & UnitProperty::Subunit)
			{
				give_unit(unit->subunit, player, count_as_completed);
			}

			helper::apply_speed_upgrade_from_unit_type(unit);

			/* -- Disabled since Flag ID is used in Cosmonarchy
			if (units_dat::base_property[unit->id] & UnitProperty::Worker
				&& unit->worker.powerup
				&& (unit->worker.powerup)->id == UnitId::Powerup_Flag)
			{
				if ((unit->worker.powerup)->player == player || player >= 8)
				{
					helper::drop_powerup(unit->worker.powerup); //return the flag to initial position?
				}
			}
			*/
		}
	}

	// Hooked to rule out unit AI type issues - Pr0nogo
	void give_ai(Unit* unit) { // 00463040
		u32 player = unit->player;
		if (player < PlayerLimit && g_player_table[player].type == PlayerType::Computer) {
			if (unit->id == UnitId::TerranCleric) {
				Unit* free_medic = g_aiscript_controller[player].main_medic;
				if (!free_medic) {
					free_medic = unit;
				}
			}

			if (units_dat::computer_ai_internal[unit->id] & 2) { // "Non-military"
				AiTown* town = bw::get_closest_town(player, unit->position);
			//	u32 closest_resarea = bw::get_closest_resarea(unit->position);
			//	if (default_ai_expansion_block[player] && bw::unit_id_is_town_center(unit->id) && (!town || town->resarea != closest_resarea)) {
			//		AiScript* aiscript = 0;
			//		for (AiScript* it = *g_first_active_aiscript; it; it = it->link.next) {
			//			if (it->player == player) {
			//				aiscript = it;
			//				break;
			//			}
			//		}
			//		if (aiscript) {
			//			if (helper::ai_try_expanding_to(closest_resarea)) {
			//				helper::ai_expand_to(aiscript, closest_resarea, default_ai_expansion_block[aiscript->player]);
			//				town = bw::get_closest_town(player, unit->position);
			//			}
			//		}
			//	}
				if (town) {
					helper::add_worker_structure_ai(unit, town);
				}
			} else {
				UnitExt* unit_ext = get_unit_ext(unit);
				if (unit_ext->values[ValueInfested] || unit_ext->values[ValueMindControlPlayer]) {
					helper::ai_training_simple_military(unit, unit);
				} else {
					helper::add_guard_ai_to_unit(unit);
				}
			}
		}
	}
}