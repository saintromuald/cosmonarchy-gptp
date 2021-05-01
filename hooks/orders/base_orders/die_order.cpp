#include "die_order.h"
#include <SCBW/api.h>

namespace hooks {

	void orders_Die(CUnit* unit) {
		bool jump_to_795AB = false;
		if (unit->userActionFlags & 4) {
			hideAndDisableUnit(unit);
		}

		if (unit->subunit != nullptr) {
			clearUnitSprites(unit->sprite, unit->subunit->sprite);
			if (unit->subunit->subunit != nullptr) {
				if (units_dat::BaseProperty[(unit->subunit->subunit)->id] & UnitProperty::Subunit) {
					UnitDestructor(unit->subunit->subunit);
					unit->subunit->subunit = nullptr;
				}
				_UnitDestructor(unit->subunit);
				unit->subunit->sprite->free();
				unit->subunit->sprite = nullptr;
				unit->subunit = nullptr;
			}
		}

		unit->mainOrderState = 1;
		if (unit->sprite != nullptr) {
			if (!(unit->sprite->flags & CSprite_Flags::Hidden)) {
				if (!(unit->status & UnitStatus::IsHallucination)
					|| unit->status & UnitStatus::IsSelfDestructing) {
					jump_to_795AB = true;
				}
				else {
					CThingy* hallucination_smoke;
					scbw::playSound(SoundId::Protoss_TEMPLAR_PTeHal01_WAV, unit);
					hallucination_smoke =
						createThingy(
							SpriteId::Hallucination_Death1,
							unit->getX(),
							unit->getY(),
							0
						);

					if (hallucination_smoke != nullptr) {
						//allow players to see smoke if could already see unit vanishing
						function_00497480(hallucination_smoke->sprite, unit->sprite->visibilityFlags);
					}
				}
			}
		}

		if (!jump_to_795AB) {//79566
			hideAndDisableUnit(unit);
			if (unit->subunit != nullptr) {
				if (units_dat::BaseProperty[unit->subunit->id] & UnitProperty::Subunit) {
					UnitDestructor(unit->subunit);
					unit->subunit = nullptr;
				}
			}
			_UnitDestructor(unit);
			unit->sprite->free();
			unit->sprite = nullptr;
		} else {
			unit->sprite->playIscriptAnim(IscriptAnimation::Death, true);
			_UnitDestructor(unit);
		}
	}
}