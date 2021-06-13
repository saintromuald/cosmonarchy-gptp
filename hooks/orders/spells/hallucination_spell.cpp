#include "hallucination_spell.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	void orders_Hallucination1(Unit* unit)
	{
		s32 sightRange = unit->getSightRange(true);
		u16 techCost;
		if (helper::ordersSpell_Sub_4926D0(unit, TechId::Hallucination, &techCost, sightRange * 32, 0x899))
		{
			s32 counter = 1;

			Vector32 hit;
			if (Unit* target = unit->orderTarget.unit)
			{
				hit = {target->getX(), target->getY()};
			}
			else
			{
				Point16 pt = unit->orderTarget.pt;
				hit = {pt.x, pt.y};
			}
			
			scbw::decloakProc(unit);

			helper::createBullet(WeaponId::CantavisHallucinationCast, unit, unit->getX(), unit->getY(), unit->playerId, unit->currentDirection1);

			if (!(*CHEAT_STATE & CheatFlags::TheGathering))
			{
				unit->spendUnitEnergy(techCost);
			}

			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else if (unit->pAI != nullptr)
			{
				unit->orderComputerClear(OrderId::ComputerAI);
			}
			else
			{
				unit->orderComputerClear(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
	}

	// Hooked to rule out AI issues -- Pr0nogo
	Unit* createHallucination(Unit* source, s32 player)
	{
		if (units_dat::BaseProperty[source->id] & UnitProperty::Building)
		{
			return nullptr;
		}

		Unit* result = nullptr;
		Unit* halUnit = helper::createUnit(source->id, source->sprite->position.x, source->sprite->position.y, player);
		Unit* halUnit2 = halUnit;
		if (halUnit != nullptr)
		{
			u16 halTimer = 0;
			helper::applyHallucinationStats(halUnit);
			if (halUnit2->status & UnitStatus::IsHallucination)
			{
				halTimer = 1350;
			}
			else
			{
				halTimer = (halUnit2->id != UnitId::ZergRilirokor && halUnit2->id != UnitId::ZergRililisk) ? 0 : 1800;
			}
			u16 currentHalTimer = halUnit2->removeTimer;
			if (!currentHalTimer || halTimer < currentHalTimer)
			{
				halUnit2->removeTimer = halTimer;
			}
			s16 unitType = halUnit2->id;
			if (unitType == UnitId::ProtossAccantor) // Originally also ran for Warbringer ID
			{
				helper::hallucinateScarabs(halUnit2, source);
			}
			if (!(units_dat::BaseProperty[halUnit2->id] & (UnitProperty::Worker | UnitProperty::Building)))
			{
				helper::AI_TrainingUnit(halUnit2, halUnit2);
			}
			else
			{
				WorkerAi* sourceAI = (WorkerAi*)source->pAI;
				helper::AI_AddWorkerStructureAI(halUnit2, sourceAI->town);
			}
			result = halUnit2;
		}
		else
		{
			helper::displayLastNetErrorForPlayer(player);
			result = nullptr;
		}
		return result;
	}
}