#include "give_unit.h"
#include <SCBW/api.h>

namespace hooks {
	/// used in completeAddon, sub_4678A0 (related to Terran construction),
	/// GiveAllUnitPropertiesToPlayerAndMakeIdle, itself, sub_4A0080, AI_CreateUnit,
	/// orders_RescuePassive, orders_InfestMine1
	/// Changes would affect all those functions and maybe more
	void GiveUnit(CUnit* unit, uint32 playerId, uint32 unkScore) {

		if (unit->playerId != playerId) {

			bool isMorphing = false;
			bool isSelected;
			uint32 selectionIndex = 0;

			function_004A1E50(unit, 0);

			if (unit->isRemorphingBuilding()) {
				unit->status |= UnitStatus::Completed;
				isMorphing = true;
			}

			incrementUnitScores(unit, -1);

			if (unit->status & UnitStatus::Completed)
				incrementUnitScoresEx(unit, 0, -1);

			isSelected = (unit->sprite->flags & CSprite_Flags::Selected) == 1;

			if (isSelected) {
				selectionIndex = unit->sprite->selectionIndex;
				DestroyUnitSelection(unit);
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
				CreateUnitSelection(unit, selectionIndex);

			unit->player_link.prev = nullptr;
			unit->player_link.next = firstPlayerUnit->unit[playerId];

			if (firstPlayerUnit->unit[playerId] != nullptr)
				(firstPlayerUnit->unit[playerId])->player_link.prev = unit;

			firstPlayerUnit->unit[playerId] = unit;

			incrementUnitScores(unit, 1);

			if (unit->status & UnitStatus::Completed)
				incrementUnitScoresEx(unit, unkScore, 1);

			if (isMorphing)
				unit->status &= ~UnitStatus::Completed;

			function_00476D30(unit);

			scbw::refreshConsole();

			if (
				units_dat::GroupFlags[unit->id].isZerg != 1 &&
				units_dat::GroupFlags[unit->id].isProtoss != 1 &&
				unit->status & UnitStatus::GroundedBuilding
				)
				*canUpdatePoweredStatus = 1;

			function_00463040(unit);

			if (unit->subunit != nullptr && units_dat::BaseProperty[unit->subunit->id] & UnitProperty::Subunit)
				GiveUnit(unit->subunit, playerId, unkScore);

			ApplySpeedUpgradeFromUnitType(unit);

			if (
				units_dat::BaseProperty[unit->id] & UnitProperty::Worker &&
				unit->worker.powerup != nullptr &&
				(unit->worker.powerup)->id == UnitId::Powerup_Flag) {
				if ((unit->worker.powerup)->playerId == playerId || playerId >= 8)
					function_004F3B70(unit->worker.powerup); //return the flag to initial position?
			}
		}
	}
}