#include "die_order.h"
#include <SCBW/api.h>

namespace hooks
{

	void orders_Die(Unit* unit)
	{
		bool jump_to_795AB = false;
		if (unit->user_action_flags & 4)
		{
			helper::hide_and_disable_unit(unit);
		}

		if (unit->subunit != nullptr)
		{
			helper::clear_unit_sprites(unit->sprite, unit->subunit->sprite);
			if (unit->subunit->subunit != nullptr)
			{
				if (units_dat::base_property[(unit->subunit->subunit)->id] & UnitProperty::Subunit)
				{
					helper::remove_unit(unit->subunit->subunit);
					unit->subunit->subunit = nullptr;
				}
				helper::unit_die(unit->subunit);
				unit->subunit->sprite->free();
				unit->subunit->sprite = nullptr;
				unit->subunit = nullptr;
			}
		}

		unit->main_order_state = 1;
		if (unit->sprite != nullptr)
		{
			if (!(unit->sprite->flags & SpriteFlags::Hidden))
			{
				if (!(unit->status & UnitStatus::IsHallucination)
					|| unit->status & UnitStatus::IsSelfDestructing)
				{
					jump_to_795AB = true;
				}
				else
				{
					Thingy* hallucination_smoke;
					helper::play_sound_from_unit(SoundId::Protoss_TEMPLAR_PTeHal01_WAV, unit, 1, 0);
					hallucination_smoke =
						helper::create_thingy(
							SpriteId::Hallucination_Death1,
							unit->get_x(),
							unit->get_y(),
							0
						);

					if (hallucination_smoke != nullptr)
					{
						//allow players to see smoke if could already see unit vanishing
						helper::refresh_all_visible_images_at_screen_pos(hallucination_smoke->sprite, unit->sprite->visibility_status);
					}
				}
			}
		}

		if (!jump_to_795AB)
		{//79566
			helper::hide_and_disable_unit(unit);
			if (unit->subunit != nullptr)
			{
				if (units_dat::base_property[unit->subunit->id] & UnitProperty::Subunit)
				{
					helper::remove_unit(unit->subunit);
					unit->subunit = nullptr;
				}
			}
			helper::unit_die(unit);
			unit->sprite->free();
			unit->sprite = nullptr;
		}
		else
		{
			unit->sprite->play_iscript_animation(IscriptAnimation::Death, true);
			helper::unit_die(unit);
		}
	}
}