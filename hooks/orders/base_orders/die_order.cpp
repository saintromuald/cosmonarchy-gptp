#include "die_order.h"
#include <BW/api.h>

namespace hooks
{
	void orders_die(Unit* unit) // 00479480
	{
		if (unit->user_action_flags & UserActionFlags_SilentRemove) {
			helper::hide_and_disable_unit(unit);
		}

		if (unit->subunit) {
			helper::clear_unit_sprites(unit->sprite, unit->subunit->sprite);
			if (unit->subunit->subunit) {
				if (units_dat::base_property[(unit->subunit->subunit)->id] & UnitProperty::Subunit) {
					helper::remove_unit(unit->subunit->subunit);
					unit->subunit->subunit = 0;
				}
				helper::unit_die(unit->subunit);
				unit->subunit->sprite->free();
				unit->subunit->sprite = 0;
				unit->subunit = 0;
			}
		}

		unit->main_order_state = 1;
		if (AssertSprite(unit->sprite)) {
			if (!(unit->sprite->flags & SpriteFlags::Hidden)) {
				if (!(unit->status & UnitStatus::IsHallucination) || unit->status & UnitStatus::IsSelfDestructing) {
					unit->sprite->play_iscript_animation(IscriptAnimation::Death, true);
					helper::unit_die(unit);
					return;
				} else {
					helper::play_sound_from_unit(SoundId::Protoss_TEMPLAR_PTeHal01_WAV, unit, 1, 0);
					Thingy* hallucination_smoke = helper::create_thingy(SpriteId::Hallucination_Death1, unit->sprite->position.x, unit->sprite->position.y, 0);
					if (hallucination_smoke) {
						hallucination_smoke->sprite->elevation = unit->sprite->elevation + 1;
						// allow players to see smoke if could already see unit vanishing
						helper::refresh_all_visible_images_at_screen_pos(hallucination_smoke->sprite, unit->sprite->visibility_status);
					}
				}
			}
			helper::hide_and_disable_unit(unit);
			if (unit->subunit) {
				if (units_dat::base_property[unit->subunit->id] & UnitProperty::Subunit) {
					helper::remove_unit(unit->subunit);
					unit->subunit = 0;
				}
			}
			helper::unit_die(unit);
			unit->sprite->free();
			unit->sprite = 0;
		}
	}
}