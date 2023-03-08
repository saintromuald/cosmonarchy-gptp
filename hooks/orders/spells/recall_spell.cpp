#include "recall_spell.h"
#include <SCBW/api.h>
#include "SCBW/stat_txt_spec.h"

namespace hooks
{

	// Originally known as sub_4942D0
	// Function performed on each target of Recall
	// by iterate_units_at_location_target_proc_recall_targets
	// that was validated throug spells_recall_action_on_target_in_range
	// Do the actual unit movement
	void spells_recall_action_on_valid_target_for_recall(Unit* target, Unit* caster) {
		static b32* const at_least_1_unit_was_recalled = (b32*)(0x0063FF50);
		Point16 previous_pos = {};
		Point16 new_pos = {};
		previous_pos.x = target->sprite->position.x;
		previous_pos.y = target->sprite->position.y;

		// Identical code to set_unit_position(), but for some reason, is using another function to do exactly the same thing
		helper::move_unit_duplicate(target, caster->sprite->position.x, caster->sprite->position.y);

		if (!scbw::check_unit_collision_pos(target, &(caster->sprite->position), &new_pos, 0, false, 0)) {
			// Collision detected, cancel the movement
			scbw::set_unit_position(target, previous_pos.x, previous_pos.y);
		} else {
			// Finish the teleporting (and cut the recalled unit from a possible unit targeted by an order)
			scbw::prepare_unit_move(target, false);
			target->order_target.unit = 0;
			scbw::set_unit_position(target, new_pos.x, new_pos.y);
			scbw::refresh_unit_after_move(target);

			// Stop some currently active orders
			if (target->id != UnitId::ZergCocoon) {
				target->order_computer(units_dat::return_to_idle_order[target->id]);
			}

			// Originally created a Recall thingy over the unit

			// Disconnect a nuke from a nuker
			if (scbw::unit_can_nuke(target)) {
				if (target->connected_unit && target->connected_unit->id == UnitId::TerranNuclearMissile) {
					target->connected_unit->connected_unit = 0;
					target->connected_unit = 0;
				}
			}
			*at_least_1_unit_was_recalled = 1;
		}
	}

	// Function performed on each target of Recall by iterate_units_at_location_target_proc_recall_targets
	// to choose if the unit can have the Recall effect cast on it or no, and if yes, calling another function to do the Recall effect.
	void spells_recall_action_on_target_in_range(Unit* target, Unit* caster) {
		if (!(target->sprite->flags & SpriteFlags::Hidden)
		&&	 (target->player == caster->player || scbw::is_allied_to(target->player, caster->player))
		&&	!(units_dat::base_property[target->id] & UnitProperty::Building)
		&&	!(target->status & UnitStatus::Invincible)
		&&	!(target->status & UnitStatus::Burrowed)
		&&	  target->id != UnitId::ZergLarva
		&&	  target->id != UnitId::ZergOcsalLarva
		&&	  target->id != UnitId::ZergEgg
		&&	!(target->status & UnitStatus::IsHallucination)
		&&	  target != caster) {
			spells_recall_action_on_valid_target_for_recall(target, caster);
		}
	}

	void orders_Recall(Unit* unit) {
		static b32* const at_least_1_unit_was_recalled = (b32*)(0x0063FF50);

		// Unit has started casting Recall (== went through the other condition)
		if (unit->main_order_state == 1) {
			// Execute the recall effect when timer reach 0
			if (unit->main_order_timer == 0) {
				unit->status &= ~UnitStatus::CanNotReceiveOrders;
				Box16 spell_location;

				spell_location.left = unit->order_target.pt.x - 64;
				spell_location.top = unit->order_target.pt.y - 64;
				spell_location.right = unit->order_target.pt.x + 64;
				spell_location.bottom = unit->order_target.pt.y + 64;

				*at_least_1_unit_was_recalled = 0;

				// Apply the recal effect on each valid target found in spell_location
				helper::iterate_units_at_location_target_proc_recall_targets(unit, &spell_location);

				if (*at_least_1_unit_was_recalled != 0) {

					// Original code added (randomize_short(0x12) & 1) to the base new_sound
					// id Protoss_ARBITER_PAbFol02_WAV then called the playSound function
					// using that value, but since there are only 2 sounds, it's easier to 
					// see what new_sound is used that way.
					// The compiler may make it back like the original anyway.

					if ((helper::randomize_short(0x12) & 1) == 0) {
						helper::play_sound_from_unit(SoundId::Protoss_ARBITER_PAbFol02_WAV, unit, 1, 0);
					} else {
						helper::play_sound_from_unit(SoundId::Protoss_ARBITER_PAbFol03_WAV, unit, 1, 0);
					}

					Point16 center = {unit->position.x, unit->position.y};
					Point16 top = center;
					top.y -= 3 * 32;
					scbw::create_and_init_thingy(SpriteId::RecallFieldShorter, center.x, center.y, unit->player, unit->sprite->elevation - 1);
					for (u32 i = 0; i < 8; ++i) {
						Point16 next = scbw::rotate_point(center, top, i * (360 / 8));
						scbw::create_and_init_thingy(SpriteId::RecallFieldShorter, next.x, next.y, unit->player, unit->sprite->elevation - 1);
					}
				}
				unit->order_to_idle();
			}
		} else {
			if (!scbw::is_cheat_enabled(CheatFlags::TheGathering) && unit->energy < techdata_dat::energy_cost[TechId::Recall] * 256) {
				u8 raceId = unit->get_race();
				char* message = (char*)g_stat_txt_tbl_new->get_string(864 + raceId);	//"Not enough energy" text
				if (message && unit->player == *g_local_nation_id && *g_active_player_id == *g_local_human_id) {
					helper::set_unit_stat_txt_error_message(message);
				}
				unit->order_to_idle();
			} else {
				if (!scbw::is_cheat_enabled(CheatFlags::TheGathering)) {
					unit->spend_unit_energy(techdata_dat::energy_cost[TechId::Recall] * 256);
				}

				if (unit->order_target.unit) {
					unit->order_target.pt.x = unit->order_target.unit->sprite->position.x;
					unit->order_target.pt.y = unit->order_target.unit->sprite->position.y;
				}

				scbw::create_and_init_thingy(SpriteId::Recall_Field, unit->position.x, unit->position.y, unit->player, unit->sprite->elevation - 1);
				scbw::create_and_init_thingy(SpriteId::Recall_Field, unit->order_target.pt.x, unit->order_target.pt.y, unit->player, unit->sprite->elevation + 1);

				u32 random_value = 0;
				if (*g_is_in_game_loop) {
					// Hardcoded (not using a function) RNG use, like the original code did
					static u32* const u32_0051C654 = (u32*)0x0051C654;
					static u32* const u32_0051CA18 = (u32*)0x0051CA18;

					*u32_0051C654 = *u32_0051C654 + 1;
					*u32_0051CA18 = *u32_0051CA18 + 1;
					*g_last_random_number = (*g_last_random_number * 0x015A4E35) + 1;

					random_value = ((*g_last_random_number) / 65536) & 0x7FFF;
				}

				// Original code added (random_value & 1) to the base new_sound
				// id Protoss_ARBITER_PAbFol00_WAV then called the playSound function
				// using that value, but since there are only 2 sounds, it's easier to 
				// see what new_sound is used that way.
				// The compiler may make it back like the original anyway.

				u32 sound_id_source = SoundId::Protoss_ARBITER_PAbFol00_WAV;
				u32 sound_id_target = SoundId::Protoss_ARBITER_PAbFol01_WAV;
				if (random_value & 1 == 0) {
					sound_id_source = SoundId::Protoss_ARBITER_PAbFol01_WAV;
					sound_id_target = SoundId::Protoss_ARBITER_PAbFol00_WAV;
				}
				helper::play_sound_at_pos(sound_id_source, unit->position, 1, 0);
				helper::play_sound_at_pos(sound_id_target, unit->order_target.pt, 1, 0);

				scbw::decloak_proc(unit);
				unit->status |= UnitStatus::CanNotReceiveOrders;
				unit->main_order_timer = 5 * 24; // used to be 22
				unit->main_order_state = 1;
			}
		}
	}
}
