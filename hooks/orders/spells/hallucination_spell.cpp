#include "hallucination_spell.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks {

	void orders_Hallucination1(CUnit* unit) {
		int sightRange = unit->getSightRange(true);
		uint16 techCost;
		if (
			ordersSpell_Sub_4926D0(
				unit,
				TechId::Hallucination,
				&techCost,
				sightRange * 32,
				0x899 // Invalid Target
				)
			) {
			int counter = 1;

			// TODO: cleanup
			/*
			while(counter > 0) {
			
				counter--;
			
				CUnit* hal_unit = hallucinationHelpers::createHallucinationUnit(unit->orderTarget.unit,unit->playerId);
			
				if(hal_unit == nullptr)
					counter = 0;
				else
				if(!hallucinationHelpers::function_004F66D0(hal_unit)) {
					counter = 0;
					hallucinationHelpers::UnitDestructor(hal_unit);
				}			
			}
			*/

			Point32 hit;
			if (CUnit* target = unit->orderTarget.unit) {
				hit = {target->getX(), target->getY()};
			} else {
				Point16 pt = unit->orderTarget.pt;
				hit = {pt.x, pt.y};
			}

			globals.createGenericPersistImpact(ImpactId::Hallucination, 20 * 24, {hit.x, hit.y}, 1.5 * 32, -1, 5, unit->playerId); // TODO: Add Hallucination area sprite
			if (!(*CHEAT_STATE & CheatFlags::TheGathering)) {
				unit->spendUnitEnergy(techCost);
			}

			if (unit->orderQueueHead != nullptr) {
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			} else if (unit->pAI != nullptr) {
				unit->orderComputerCL(OrderId::ComputerAI);
			} else {
				unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
	}

	// Hooked to rule out AI issues -- Pr0nogo
	CUnit* createHallucination(CUnit* source, int32 player) {
		CUnit* halUnit; // eax
		CUnit* v3; // edi
		CUnit* halUnit2; // esi
		CUnit* result; // eax
		uint16 halTimer; // cx
		uint16 currentHalTimer; // ax
		int16 unitType; // ax

		v3 = source;
		halUnit = createUnit(source->id, source->sprite->position.x, source->sprite->position.y, player);
		halUnit2 = halUnit;
		if (halUnit) {
			applyHallucinationStats(halUnit);
			if (halUnit2->status & UnitStatus::IsHallucination) {
				halTimer = 1350;
			}
			else {
				halTimer = halUnit2->id != UnitId::ZergBroodling ? 0 : 1800;
			}
			currentHalTimer = halUnit2->removeTimer;
			if (!currentHalTimer || halTimer < currentHalTimer) {
				halUnit2->removeTimer = halTimer;
			}
			unitType = halUnit2->id;
			if (unitType == UnitId::ProtossAccantor) { // Originally also ran for Warbringer ID
				hallucinateScarabs(halUnit2, v3);
			}
			AI_TrainingUnit(halUnit2, halUnit2);
			result = halUnit2;
		} else {
			displayLastNetErrorForPlayer(player);
			result = 0;
		}
		return result;
	}
}