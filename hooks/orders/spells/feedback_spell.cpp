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
			s32 sightRange = unit->get_sight_range(true);
			if (helper::spell_order(unit, TechId::Feedback, &techCost, sightRange * 32, 0x532)) // Must target units with energy
			{
				if (target->status & UnitStatus::IsHallucination)
				{
					u8 overlaySize = scbw::has_overlay(target);
					helper::replace_feedback_sprite(target, overlaySize + SpriteId::Feedback_Hit_Small);
					unit->spend_unit_energy(techCost);
					target->remove();
					unit->order_to_idle();
				}
				else
				{ // 004F6DE3

					if (target->energy <= 0)
					{
						unit->order_to_idle();
					}
					else
					{
						target->damage_with(WeaponId::Feedback, target->energy, unit, unit->player, 1, 1);
						target->energy = 0;
						unit->spend_unit_energy(techCost);
						helper::play_sound_from_unit(SoundId::Protoss_Darchon_feedback_wav, target, 1, 0);

						if (!target->is_dead())
						{
							u8 overlaySize = scbw::has_overlay(target);
							if (target->subunit != nullptr)
							{
								target->subunit->sprite->create_top_overlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);
							}
							else
							{
								target->sprite->create_top_overlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);
							}
							unit->order_to_idle();
						}
						else
						{ // 004F6E6D
							u8 overlaySize = scbw::has_overlay(target);
							helper::replace_feedback_sprite(target, overlaySize + SpriteId::Feedback_Hit_Small);
							unit->order_to_idle();
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
			else if (unit->unit_ai != nullptr)
			{
				unit->order_computer(OrderId::ComputerAI);
			}
			else
			{
				unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
	}
}