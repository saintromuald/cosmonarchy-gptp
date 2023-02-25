#include "recall_spell.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	void orders_cast_mind_control(Unit* unit)
	{
		UnitExt* unitExt = get_unit_ext(unit);
		Unit* target = unit->order_target.unit;
		if (!target || unit->player == target->player || scbw::is_allied_to(unit->player, target->player)) {
			unit->status &= ~UnitStatus::CanNotReceiveOrders;
			unit->order_to_idle();
		} else {
			u16 tech_energy_cost = 0;
			// Energy cost is initialized in this function
			// Corresponding error message is "Invalid Target"

			if (!unit->main_order_state) {
				if (helper::spell_order(unit, TechId::Tyranny, &tech_energy_cost, unit->get_sight_range(true) * 32, 0x052E)) {
					// Start the channel
					unit->play_iscript_animation(IscriptAnimation::Unused2);
					unit->main_order_state = 1;
					unit->main_order_timer = 24;
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->spend_unit_energy(tech_energy_cost);
				}
			} else {
				// Cancel when way out of range or lost sight of target
				u32 distance = scbw::get_distance_fast(unit->position, target->position);
				if (distance > unit->get_sight_range(true) * 32 * 2 || !target->is_visible_to(unit->player)) {
					unit->status &= ~UnitStatus::CanNotReceiveOrders;
					unit->order_to_idle();
					return;
				}

				// Wait for channel
				if (unit->main_order_timer) {
					return;
				}
				unit->status &= ~UnitStatus::CanNotReceiveOrders;

				if (target->status & UnitStatus::IsHallucination) {
					target->remove();
				} else {
					u32 overlayId = scbw::has_overlay(target) + ImageId::MindControlHit_Small;
					if (target->subunit) {
						target->subunit->sprite->create_top_overlay(overlayId, 0, 0, 0);
					} else {
						target->sprite->create_top_overlay(overlayId, 0, 0, 0);
					}
					helper::mind_control_hit(unit, target);
					target->order_to_idle();
				}
				helper::play_sound_from_unit(SoundId::Protoss_Darchon_mind_wav, target, 1, 0);
				unit->order_to_idle();
			}
		}
	}
}