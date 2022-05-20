#include "hallucination_spell.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	void orders_Hallucination1(Unit* unit)
	{
		s32 sightRange = unit->getSightRange(true);
		u16 techCost;
		if (helper::spell_order(unit, TechId::Hallucination, &techCost, sightRange * 32, 0x899))
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
			else if (unit->unit_ai != nullptr)
			{
				unit->orderComputer(OrderId::ComputerAI);
			}
			else
			{
				unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
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
		if (halUnit)
		{
			u16 halTimer = 0;
			helper::apply_hallucination_stats(halUnit);
			if (halUnit->status & UnitStatus::IsHallucination)
			{
				halTimer = 1350;
			}
			else
			{
				halTimer = (halUnit->id != UnitId::ZergRilirokor && halUnit->id != UnitId::ZergRililisk) ? 0 : 1800;
			}
			u16 currentHalTimer = halUnit->removeTimer;
			if (!currentHalTimer || halTimer < currentHalTimer)
			{
				halUnit->removeTimer = halTimer;
			}
			s16 unitType = halUnit->id;
			if (unitType == UnitId::ProtossAccantor) // Originally also ran for Warbringer ID
			{
				helper::hallucinate_scarabs(halUnit, source);
			}
			if (source->unit_ai)
			{
				if (units_dat::BaseProperty[halUnit->id] & (UnitProperty::Worker | UnitProperty::Building)
					|| scbw::unitIdIsZergSupply(source->id))
				{
					WorkerAi* sourceAI = (WorkerAi*)source->unit_ai;
					helper::AI_AddWorkerStructureAI(halUnit, sourceAI->town);
				}
				else
				{
					helper::AI_TrainingSimpleMilitary(halUnit, halUnit);
				}
			}
			result = halUnit;
		}
		else
		{
			helper::displayLastNetErrorForPlayer(player);
			result = nullptr;
		}
		return result;
	}
}