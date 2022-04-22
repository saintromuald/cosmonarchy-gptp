#include "recall_spell.h"
#include <SCBW/api.h>

namespace hooks
{
	void ordersCastMindControl(Unit* unit)
	{
		Unit* target = unit->orderTarget.unit;
		if (target == nullptr)
		{
			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{
				if (unit->pAI != nullptr)
					unit->orderComputer(OrderId::ComputerAI);
				else
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
		else
		{
			//F69B4
			if (unit->playerId == target->playerId)
				unit->orderToIdle();
			else
			{

				u16 techEnergyCost = 0;
				bool spellCastSuccess;

				//F69CB

				u32 unitSightRangeForSpell = unit->getSightRange(true);

				unitSightRangeForSpell *= 32;

				//energy cost is initialized in this function
				//Corresponding error message is "Invalid Target"
				spellCastSuccess = helper::spellOrder(unit, TechId::MindControl, &techEnergyCost, unitSightRangeForSpell, 0x052E);

				if (spellCastSuccess)
				{

					if (target->status & UnitStatus::IsHallucination)
						target->remove();
					else
					{

						//overlayId = overlaySize (0,1,2) + MinimumSizeImageId
						u32 overlayId = scbw::hasOverlay(target) + ImageId::MindControlHit_Small;

						////F6A08 (direct implementation of 0x00047B720 hasOverlay)

						//if(units_dat::BaseProperty[target->id] & UnitProperty::MediumOverlay)
						//	overlayId = ImageId::MindControlHit_Medium;
						//else
						//if(units_dat::BaseProperty[target->id] & UnitProperty::LargeOverlay)
						//	overlayId = ImageId::MindControlHit_Large;
						//else
						//	overlayId = ImageId::MindControlHit_Small;

						if (target->subunit != nullptr)
							target->subunit->sprite->createTopOverlay(overlayId, 0, 0, 0);
						else
							target->sprite->createTopOverlay(overlayId, 0, 0, 0);
						scbw::sendGPTP_aise_cmd(unit, target, GptpId::SetMindControlLink, 0, 0, 0);
						helper::MindControlHit(unit, target);
						//scbw::print("Set...");

						target->orderToIdle();

					}

					//F6A42

//					unit->shields = 0;

					if (!(*CHEAT_STATE & CheatFlags::TheGathering))
					{
						unit->spendUnitEnergy(techEnergyCost);
					}
					//F6A5E
					helper::playSoundFromSource(SoundId::Protoss_Darchon_mind_wav, target, 1, 0);

					unit->orderToIdle();

				} //if(spellCastSuccess)

			} //if (unit->playerId != target->playerId)

		} //if (target != nullptr)
	}
}