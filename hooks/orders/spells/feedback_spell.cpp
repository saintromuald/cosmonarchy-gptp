#include "feedback_spell.h"
#include <SCBW/api.h>

namespace hooks {

	void orders_Feedback(CUnit* unit) {

		CUnit* target = unit->orderTarget.unit;

		if (
			target != nullptr &&
			units_dat::BaseProperty[target->id] & UnitProperty::Spellcaster &&
			!(target->status & UnitStatus::IsHallucination)
			) {

			uint16 techCost;
			int sightRange = unit->getSightRange(true);

			if (
				ordersSpell_Sub_4926D0(
					unit,
					TechId::Feedback,
					&techCost,
					sightRange * 32,
					0x532 //Must target units with energy
				)
				) {

				if (target->status & UnitStatus::IsHallucination) {
					uint8 overlaySize = scbw::hasOverlay(target);
					replaceFeedbackSprite(target, overlaySize + SpriteId::Feedback_Hit_Small);
					unit->spendUnitEnergy(techCost);
					target->remove();
					unit->orderToIdle();
				} else { //F6DE3

					if (target->energy <= 0)
						unit->orderToIdle();
					else {

						target->damageWith(WeaponId::Feedback, target->energy, unit, unit->playerId, 1, 1);
						target->energy = 0;
						unit->spendUnitEnergy(techCost);
						scbw::playSound(SoundId::Protoss_Darchon_feedback_wav, target);

						if (!target->isDead()) {

							uint8 overlaySize = scbw::hasOverlay(target);

							if (target->subunit != nullptr)
								target->subunit->sprite->createTopOverlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);
							else
								target->sprite->createTopOverlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);

							unit->orderToIdle();

						} else { //F6E6D
							uint8 overlaySize = scbw::hasOverlay(target);
							replaceFeedbackSprite(target, overlaySize + SpriteId::Feedback_Hit_Small);
							unit->orderToIdle();
						}

					}

				}

			}


		} else {	//F6E92

			if (unit->orderQueueHead != nullptr) {
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			} else
				if (unit->pAI != nullptr)
					unit->orderComputerCL(OrderId::ComputerAI);
				else
					unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
		}
	}
}