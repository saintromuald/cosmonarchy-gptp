#include "feedback_spell.h"
#include <BW/api.h>

namespace hooks {
	void orders_feedback(Unit* unit) { // 004F6D40
		Unit* target = unit->order_target.unit;
		if (target && units_dat::base_property[target->id] & UnitProperty::Spellcaster && !(target->status & UnitStatus::IsHallucination)) {
			u16 tech_cost;
			s32 sight_range = unit->get_sight_range(true) * 32;
			if (helper::spell_order(unit, TechId::Feedback, &tech_cost, sight_range, 0x532)) { // Must target units with energy
				if (target->status & UnitStatus::IsHallucination) {
					u8 overlay_size = bw::overlay_size(target);
					helper::replace_feedback_sprite(target, overlay_size + SpriteId::Feedback_Hit_Small);
					unit->spend_energy(tech_cost);
					target->bwfunc_remove();
					unit->order_to_idle();
				} else {
					if (target->energy <= 0) {
						unit->order_to_idle();
					} else {
						target->damage_with(WeaponId::Feedback, target->energy, unit, unit->player, 1, 1);
						target->spend_energy(target->energy);
						unit->spend_energy(tech_cost);
						helper::play_sound_from_unit(SoundId::Protoss_Darchon_feedback_wav, target, 1, 0);

						if (!target->is_dead()) {
							u8 overlay_size = bw::overlay_size(target);
							if (target->subunit) {
								target->subunit->sprite->create_top_overlay(overlay_size + ImageId::Feedback_Small, 0, 0, 0);
							} else {
								target->sprite->create_top_overlay(overlay_size + ImageId::Feedback_Small, 0, 0, 0);
							}
							unit->order_to_idle();
						} else {
							u8 overlay_size = bw::overlay_size(target);
							helper::replace_feedback_sprite(target, overlay_size + SpriteId::Feedback_Hit_Small);
							unit->order_to_idle();
						}
					}
				}
			}
		} else {
			if (unit->order_queue_head) {
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			} else if (unit->unit_ai) {
				unit->order_computer(OrderId::ComputerAI);
			} else {
				unit->order_computer(units_dat::return_to_idle_order[unit->id]);
			}
		}
	}
}