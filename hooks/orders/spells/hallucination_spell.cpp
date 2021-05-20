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

			Point32 hit;
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
		Unit* halUnit; // eax
		Unit* v3; // edi
		Unit* halUnit2; // esi
		Unit* result; // eax
		u16 halTimer; // cx
		u16 currentHalTimer; // ax
		s16 unitType; // ax

		v3 = source;
		halUnit = helper::createUnit(source->id, source->sprite->position.x, source->sprite->position.y, player);
		halUnit2 = halUnit;
		if (halUnit)
		{
			helper::applyHallucinationStats(halUnit);
			if (halUnit2->status & UnitStatus::IsHallucination)
			{
				halTimer = 1350;
			}
			else
			{
				halTimer = halUnit2->id != UnitId::ZergRilirokor ? 0 : 1800;
			}
			currentHalTimer = halUnit2->removeTimer;
			if (!currentHalTimer || halTimer < currentHalTimer)
			{
				halUnit2->removeTimer = halTimer;
			}
			unitType = halUnit2->id;
			if (unitType == UnitId::ProtossAccantor) // Originally also ran for Warbringer ID
			{
				helper::hallucinateScarabs(halUnit2, v3);
			}
			//helper::AI_TrainingUnit(halUnit2, halUnit2);
			scbw::initUnitAI(halUnit2);
			result = halUnit2;
		}
		else
		{
			helper::displayLastNetErrorForPlayer(player);
			result = 0;
		}
		return result;
	}
}