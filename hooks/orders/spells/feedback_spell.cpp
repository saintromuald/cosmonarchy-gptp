#include "feedback_spell.h"
#include <SCBW/api.h>

namespace hooks
{

	void orders_Feedback(Unit* unit)
	{
		Unit* target = unit->orderTarget.unit;
		if (target != nullptr
			&& units_dat::BaseProperty[target->id] & UnitProperty::Spellcaster
			&& !(target->status & UnitStatus::IsHallucination))
		{
			u16 techCost;
			s32 sightRange = unit->getSightRange(true);
			if (helper::ordersSpell_Sub_4926D0(unit, TechId::Feedback, &techCost, sightRange * 32, 0x532)) // Must target units with energy
			{
				if (target->status & UnitStatus::IsHallucination)
				{
					u8 overlaySize = scbw::hasOverlay(target);
					helper::replaceFeedbackSprite(target, overlaySize + SpriteId::Feedback_Hit_Small);
					unit->spendUnitEnergy(techCost);
					target->remove();
					unit->orderToIdle();
				}
				else
				{ // 004F6DE3

					if (target->energy <= 0)
					{
						unit->orderToIdle();
					}
					else
					{
						target->damageWith(WeaponId::Feedback, target->energy, unit, unit->playerId, 1, 1);
						target->energy = 0;
						unit->spendUnitEnergy(techCost);
						helper::playSoundFromSource(SoundId::Protoss_Darchon_feedback_wav, target, 1, 0);

						if (!target->isDead())
						{
							u8 overlaySize = scbw::hasOverlay(target);
							if (target->subunit != nullptr)
							{
								target->subunit->sprite->createTopOverlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);
							}
							else
							{
								target->sprite->createTopOverlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);
							}
							unit->orderToIdle();
						}
						else
						{ // 004F6E6D
							u8 overlaySize = scbw::hasOverlay(target);
							helper::replaceFeedbackSprite(target, overlaySize + SpriteId::Feedback_Hit_Small);
							unit->orderToIdle();
						}
					}
				}
			}
		}
		else
		{ // 004F6E92
			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
				if (unit->pAI != nullptr)
				{
					unit->orderComputerCL(OrderId::ComputerAI);
				}
				else
				{
					unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
				}
		}
	}
}