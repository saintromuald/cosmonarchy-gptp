#include "give_unit.h"
#include <SCBW/api.h>

namespace hooks {
	/// used in completeAddon, sub_4678A0 (related to Terran construction),
	/// GiveAllUnitPropertiesToPlayerAndMakeIdle, itself, sub_4A0080, AI_CreateUnit,
	/// orders_RescuePassive, orders_InfestMine1
	/// Changes would affect all those functions and maybe more
	void GiveUnit(CUnit* unit, u32 playerId, u32 unkScore) {

		if (unit->playerId != playerId) {

			bool isMorphing = false;
			bool isSelected;
			u32 selectionIndex = 0;

			helper::function_004A1E50(unit, 0);

			if (unit->isRemorphingBuilding()) {
				unit->status |= UnitStatus::Completed;
				isMorphing = true;
			}

			helper::incrementUnitScores(unit, -1);

			if (unit->status & UnitStatus::Completed)
				helper::incrementUnitScoresEx(unit, 0, -1);

			isSelected = (unit->sprite->flags & CSprite_Flags::Selected) == 1;

			if (isSelected) {
				selectionIndex = unit->sprite->selectionIndex;
				helper::DestroyUnitSelection(unit);
			}

			if (unit->player_link.prev != nullptr)
				(unit->player_link.prev)->player_link.next = unit->player_link.next;

			if (unit->player_link.next != nullptr)
				(unit->player_link.next)->player_link.prev = unit->player_link.prev;

			if (unit == firstPlayerUnit->unit[unit->playerId])
				firstPlayerUnit->unit[unit->playerId] = unit->player_link.next;

			unit->player_link.prev = nullptr;
			unit->player_link.next = nullptr;
			unit->playerId = playerId;

			//maybe "if" not needed?
			if (isSelected)
				helper::CreateUnitSelection(unit, selectionIndex);

			unit->player_link.prev = nullptr;
			unit->player_link.next = firstPlayerUnit->unit[playerId];

			if (firstPlayerUnit->unit[playerId] != nullptr)
				(firstPlayerUnit->unit[playerId])->player_link.prev = unit;

			firstPlayerUnit->unit[playerId] = unit;

			helper::incrementUnitScores(unit, 1);

			if (unit->status & UnitStatus::Completed)
				helper::incrementUnitScoresEx(unit, unkScore, 1);

			if (isMorphing)
				unit->status &= ~UnitStatus::Completed;

			helper::function_00476D30(unit);

			scbw::refreshConsole();

			if (
				units_dat::GroupFlags[unit->id].isZerg != 1 &&
				units_dat::GroupFlags[unit->id].isProtoss != 1 &&
				unit->status & UnitStatus::GroundedBuilding
				)
				*canUpdatePoweredStatus = 1;

			helper::function_00463040(unit);

			if (unit->subunit != nullptr && units_dat::BaseProperty[unit->subunit->id] & UnitProperty::Subunit)
				GiveUnit(unit->subunit, playerId, unkScore);

			helper::ApplySpeedUpgradeFromUnitType(unit);

			if (
				units_dat::BaseProperty[unit->id] & UnitProperty::Worker &&
				unit->worker.powerup != nullptr &&
				(unit->worker.powerup)->id == UnitId::Powerup_Flag) {
				if ((unit->worker.powerup)->playerId == playerId || playerId >= 8)
					helper::function_004F3B70(unit->worker.powerup); //return the flag to initial position?
			}
		}
	}
}