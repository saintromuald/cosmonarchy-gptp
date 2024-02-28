#include "die_order.h"
#include <SCBW/api.h>

namespace hooks
{
	void orders_die(Unit* unit) // 00479480
	{
		bool play_death_animation = false;
		if (unit->user_action_flags & UserActionFlags_HideUnit) {
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
		if (unit->sprite) {
			if (!(unit->sprite->flags & SpriteFlags::Hidden)) {
				if (!(unit->status & UnitStatus::IsHallucination) || unit->status & UnitStatus::IsSelfDestructing) {
					play_death_animation = true;
				} else {
					helper::play_sound_from_unit(SoundId::Protoss_TEMPLAR_PTeHal01_WAV, unit, 1, 0);
					Thingy* hallucination_smoke = helper::create_thingy( SpriteId::Hallucination_Death1, unit->get_x(), unit->get_y(), 0);
					if (hallucination_smoke) {
						// allow players to see smoke if could already see unit vanishing
						helper::refresh_all_visible_images_at_screen_pos(hallucination_smoke->sprite, unit->sprite->visibility_status);
					}
				}
			}
		}

		if (!play_death_animation) {
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
		} else {
			unit->sprite->play_iscript_animation(IscriptAnimation::Death, true);
			helper::unit_die(unit);
		}
	}
}