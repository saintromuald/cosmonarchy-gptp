#include "give_unit.h"
#include <SCBW/api.h>

namespace hooks
{
	// used in itself, completeAddon, beginGasBuilding, GiveAllUnitPropertiesToPlayerAndMakeIdle, removeSubunitOrGasContainer, AI_CreateUnit, orders_RescuePassive, orders_InfestMine1
	// Changes would affect all those functions and maybe more
	void giveUnit(Unit* unit, u32 playerId, u32 bCountAsCompleted)
	{
		if (unit->playerId != playerId)
		{
			helper::AI_RemoveUnitAI(unit, 0);

			bool isMorphing = false;
			if (unit->isRemorphingBuilding())
			{
				unit->status |= UnitStatus::Completed;
				isMorphing = true;
			}

			helper::incrementUnitScores(unit, -1);
			if (unit->status & UnitStatus::Completed)
			{
				helper::incrementUnitScoresEx(unit, 0, -1);
			}

			u32 selectionIndex = 0;
			bool isSelected = (unit->sprite->flags & SpriteFlags::Selected) == 1;
			if (isSelected)
			{
				selectionIndex = unit->sprite->selectionIndex;
				helper::DestroyUnitSelection(unit);
			}

			if (unit->player_link.prev != nullptr)
			{
				(unit->player_link.prev)->player_link.next = unit->player_link.next;
			}
			if (unit->player_link.next != nullptr)
			{
				(unit->player_link.next)->player_link.prev = unit->player_link.prev;
			}
			if (unit == firstPlayerUnit->unit[unit->playerId])
			{
				firstPlayerUnit->unit[unit->playerId] = unit->player_link.next;
			}

			unit->player_link.prev = nullptr;
			unit->player_link.next = nullptr;
			unit->playerId = playerId;

			// maybe "if" not needed?
			if (isSelected)
			{
				helper::CreateUnitSelection(unit, selectionIndex);
			}

			unit->player_link.prev = nullptr;
			unit->player_link.next = firstPlayerUnit->unit[playerId];

			if (firstPlayerUnit->unit[playerId] != nullptr)
			{
				(firstPlayerUnit->unit[playerId])->player_link.prev = unit;
			}

			firstPlayerUnit->unit[playerId] = unit;
			helper::incrementUnitScores(unit, 1);
			if (unit->status & UnitStatus::Completed)
			{
				helper::incrementUnitScoresEx(unit, bCountAsCompleted, 1);
			}
			if (isMorphing)
			{
				unit->status &= ~UnitStatus::Completed;
			}

			helper::recheckAttacks(unit);
			scbw::refreshConsole();
			if (units_dat::GroupFlags[unit->id].isZerg != 1
				&& units_dat::GroupFlags[unit->id].isProtoss != 1
				&& unit->status & UnitStatus::GroundedBuilding)
			{
				*canUpdatePoweredStatus = 1;
			}

			helper::giveAI(unit);
			if (unit->subunit != nullptr && units_dat::BaseProperty[unit->subunit->id] & UnitProperty::Subunit)
			{
				giveUnit(unit->subunit, playerId, bCountAsCompleted);
			}

			helper::ApplySpeedUpgradeFromUnitType(unit);

			/* Disabled since Flag ID is used in Cosmonarchy
			if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker
				&& unit->worker.powerup != nullptr
				&& (unit->worker.powerup)->id == UnitId::Powerup_Flag)
			{
				if ((unit->worker.powerup)->playerId == playerId || playerId >= 8)
				{
					helper::function_004F3B70(unit->worker.powerup); //return the flag to initial position?
				}
			}
			*/
		}
	}

	// Hooked to rule out unit AI type issues - Pr0nogo
	void giveAI(Unit* unit) // 00463040
	{
		s32 player = unit->playerId;
		if (player < 8 && playerTable[player].type == PlayerType::Computer)
		{
			if (unit->id == UnitId::TerranCleric)
			{
				Unit* freeMedic = AIScriptController[player].mainMedic;				
				if (freeMedic == nullptr)
				{
					freeMedic = unit;
				}
			}

			if (units_dat::ComputerAIInternal[unit->id] & 2) // "Non-military"
			{
				AiTown* firstTown = AiActiveTownArray[player].first;
				if (firstTown)
				{
					helper::AI_AddWorkerStructureAI(unit, firstTown);
				}
			}
			else
			{
				helper::AI_AddGuardAiToUnit(unit);
			}
		}
	}
}