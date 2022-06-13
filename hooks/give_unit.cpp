#include "give_unit.h"
#include "globals.h"
#include <SCBW/api.h>
#include "base_layouts.h"

namespace hooks
{
	// used in itself, completeAddon, begin_gas_building, GiveAllUnitPropertiesToPlayerAndMakeIdle, removeSubunitOrGasContainer, ai_create_unit, orders_RescuePassive, orders_InfestMine1
	// Changes would affect all those functions and maybe more
	void giveUnit(Unit* unit, u32 player, u32 bCountAsCompleted)
	{
		if (unit->player != player)
		{
			helper::remove_unit_ai(unit, 0);

			bool is_morphing = false;
			if (unit->isRemorphingBuilding())
			{
				unit->status |= UnitStatus::Completed;
				is_morphing = true;
			}

			helper::increment_unit_scores(unit, -1);
			if (unit->status & UnitStatus::Completed)
			{
				helper::increment_unit_scores_ex(unit, 0, -1);
			}

			u32 selectionIndex = 0;
			bool isSelected = (unit->sprite->flags & SpriteFlags::Selected) == 1;
			if (isSelected)
			{
				selectionIndex = unit->sprite->selectionIndex;
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
			if (unit == firstPlayerUnit->unit[unit->player])
			{
				firstPlayerUnit->unit[unit->player] = unit->player_link.next;
			}

			unit->player_link.prev = nullptr;
			unit->player_link.next = nullptr;
			unit->player = player;

			// maybe "if" not needed?
			if (isSelected)
			{
				helper::create_unit_selection(unit, selectionIndex);
			}

			unit->player_link.prev = nullptr;
			unit->player_link.next = firstPlayerUnit->unit[player];

			if (firstPlayerUnit->unit[player])
			{
				(firstPlayerUnit->unit[player])->player_link.prev = unit;
			}

			firstPlayerUnit->unit[player] = unit;
			helper::increment_unit_scores(unit, 1);
			if (unit->status & UnitStatus::Completed)
			{
				helper::increment_unit_scores_ex(unit, bCountAsCompleted, 1);
			}
			if (is_morphing)
			{
				unit->status &= ~UnitStatus::Completed;
			}

			helper::recheck_attacks(unit);
			scbw::refresh_console();
			if (units_dat::GroupFlags[unit->id].is_zerg != 1
				&& units_dat::GroupFlags[unit->id].is_protoss != 1
				&& unit->status & UnitStatus::GroundedBuilding)
			{
				*can_update_powered_status = 1;
			}

			helper::give_ai(unit);
			if (unit->subunit && units_dat::BaseProperty[unit->subunit->id] & UnitProperty::Subunit)
			{
				giveUnit(unit->subunit, player, bCountAsCompleted);
			}

			helper::apply_speed_upgrade_from_unit_type(unit);

			/* -- Disabled since Flag ID is used in Cosmonarchy
			if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker
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
	void giveAI(Unit* unit) // 00463040
	{
		s32 player = unit->player;
		if (player < 8 && playerTable[player].type == PlayerType::Computer)
		{
			if (unit->id == UnitId::TerranCleric)
			{
				Unit* freeMedic = AIScriptController[player].main_medic;				
				if (freeMedic == nullptr)
				{
					freeMedic = unit;
				}
			}

			if (units_dat::ComputerAIInternal[unit->id] & 2) // "Non-military"
			{
				// Originally used AiActiveTownArray[player].first

				AiTown* town = scbw::get_closest_town(player, unit->position);
				//u32 closest_resarea = scbw::get_closest_resarea(unit->position);
				//if (default_ai_expansion_block[player] && scbw::unit_id_is_town_center(unit->id) && (!town || town->resarea != closest_resarea)) {
				//	AiScript* aiscript = 0;
				//	for (AiScript* it = *first_active_aiscript; it; it = it->link.next) {
				//		if (it->player == player) {
				//			aiscript = it;
				//			break;
				//		}
				//	}
				//	if (aiscript) {
				//		if (helper::ai_try_expanding_to(closest_resarea)) {
				//			helper::ai_expand_to(aiscript, closest_resarea, default_ai_expansion_block[aiscript->player]);
				//			town = scbw::get_closest_town(player, unit->position);
				//		}
				//	}
				//}
				if (town) {
					helper::add_worker_structure_ai(unit, town);
				}
			}
			else
			{
				UnitExt* unitExt = globals.get_unit_ext(unit);
				if (unitExt->values[ValueBlissfulEmbraceOriginalPlayer] || unitExt->units[UnitMindController])
				{
					helper::ai_training_simple_military(unit, unit);
				}
				else
				{
					helper::add_guard_ai_to_unit(unit);
				}
			}
		}
	}
}