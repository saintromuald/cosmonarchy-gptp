#include "recall_spell.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	void ordersCastMindControl(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
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
				{
					unit->orderComputer(OrderId::ComputerAI);
				}
				else
				{
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
		else
		{
			if (unit->playerId == target->playerId)
			{
				unit->orderToIdle();
			}
			else
			{
				u16 techEnergyCost = 0;
				// Energy cost is initialized in this function
				// Corresponding error message is "Invalid Target"
				if (helper::spell_order(unit, TechId::MindControl, &techEnergyCost, unit->getSightRange(true) * 32, 0x052E))
				{
					// Channel
					if (!unit->mainOrderState) {
						unit->playIscriptAnim(IscriptAnimation::Unused2);
						unit->mainOrderState = 1;
						unit->mainOrderTimer = 24;
						unit->status |= UnitStatus::CanNotReceiveOrders;
					}
					if (unit->mainOrderTimer) {
						return;
					}
					unit->status &= ~UnitStatus::CanNotReceiveOrders;

					if (target->status & UnitStatus::IsHallucination)
					{
						target->remove();
					}
					else
					{
						u32 overlayId = scbw::hasOverlay(target) + ImageId::MindControlHit_Small;
						if (target->subunit)
						{
							target->subunit->sprite->createTopOverlay(overlayId, 0, 0, 0);
						}
						else
						{
							target->sprite->createTopOverlay(overlayId, 0, 0, 0);
						}
						helper::MindControlHit(unit, target);
						target->orderToIdle();
					}
					if (!(*CHEAT_STATE & CheatFlags::TheGathering))
					{
						unit->spendUnitEnergy(techEnergyCost);
					}
					helper::playSoundFromSource(SoundId::Protoss_Darchon_mind_wav, target, 1, 0);
					unit->orderToIdle();
				}
				else
				{
					// Reset timer if target out of range
					unit->mainOrderState = 0;
				}
			}
		}
	}
}