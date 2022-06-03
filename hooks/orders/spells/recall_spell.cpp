#include "recall_spell.h"
#include <SCBW/api.h>
#include "SCBW/stat_txt_spec.h"

namespace hooks
{

	// Originally known as sub_4942D0
	// Function performed on each target of Recall
	// by IterateUnitsAtLocationTargetProc_RecallTargets
	// that was validated throug spells_Recall_ActionOnTargetInRange
	// Do the actual unit movement
	void spells_Recall_ActionOnValidTargetForRecall(Unit* target, Unit* caster) {
		static b32* const bAtLeast1UnitWasRecalled = (b32*)(0x0063FF50);

		Point16 previous_pos = {};
		Point16 new_pos = {};

		previous_pos.x = target->sprite->position.x;
		previous_pos.y = target->sprite->position.y;

		// Identical code to set_unit_position() but for some reason, is
		// using another function to do exactly the same thing
		helper::move_unit_duplicate(target, caster->sprite->position.x, caster->sprite->position.y);

		if (!scbw::check_unit_collision_pos(target, &(caster->sprite->position), &new_pos, nullptr, false, 0)) {
			// Collision detected, cancel the movement
			scbw::set_unit_position(target, previous_pos.x, previous_pos.y);
		} else {
			// Finish the teleporting (and cut the recalled unit from a possible unit targeted by an order)
			scbw::prepare_unit_move(target, false);
			target->order_target.unit = nullptr;
			scbw::set_unit_position(target, new_pos.x, new_pos.y);
			scbw::refresh_unit_after_move(target);

			// Stop some currently active orders
			if (target->id != UnitId::ZergCocoon) {
				target->order_computer(units_dat::ReturnToIdleOrder[target->id]);
			}

			// Create the Recall animation over the unit
			//Thingy* recall_effect_sprite = helper::create_thingy(SpriteId::Recall_Field, new_pos.x, new_pos.y, 0);
			//if (recall_effect_sprite)
			//{
			//	recall_effect_sprite->sprite->elevationLevel = target->sprite->elevationLevel + 1;
			//	helper::set_thingy_visibility_flags(recall_effect_sprite);
			//}

			// Disconnect a nuke from a ghost using it
			if (target->id == UnitId::TerranEidolon ||
				target->id == UnitId::Hero_SarahKerrigan ||
				target->id == UnitId::Hero_AlexeiStukov ||
				target->id == UnitId::Hero_SamirDuran ||
				target->id == UnitId::Hero_InfestedDuran) {

				if (target->connected_unit && target->connected_unit->id == UnitId::TerranNuclearMissile) {
					target->connected_unit->connected_unit = nullptr;
					target->connected_unit = nullptr;
				}

			}

			*bAtLeast1UnitWasRecalled = 1;
		}
	}

	// Originally known as sub_494400
	// Function performed on each target of Recall
	// by IterateUnitsAtLocationTargetProc_RecallTargets
	// to choose if the unit can have the Recall effect
	// cast on it or no, and if yes, calling another function
	// to do the Recall effect.
	void spells_Recall_ActionOnTargetInRange(Unit* target, Unit* caster) {
		if (!(target->sprite->flags & SpriteFlags::Hidden) &&
			(target->player == caster->player || scbw::is_allied_to(target->player, caster->player)) &&
			!(units_dat::BaseProperty[target->id] & UnitProperty::Building) &&
			!(target->status & UnitStatus::Invincible) &&
			!(target->status & UnitStatus::Burrowed) &&
			target->id != UnitId::ZergLarva &&
			target->id != UnitId::ZergEgg &&
			!(target->status & UnitStatus::IsHallucination) &&
			target != caster) {
			spells_Recall_ActionOnValidTargetForRecall(target, caster);
		}
	}

	void orders_Recall(Unit* unit) {
		static b32* const bAtLeast1UnitWasRecalled = (b32*)(0x0063FF50);

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

				*bAtLeast1UnitWasRecalled = 0;

				// Apply the recal effect on each valid target found in spell_location
				helper::IterateUnitsAtLocationTargetProc_RecallTargets(unit, &spell_location);

				if (*bAtLeast1UnitWasRecalled != 0) {

					// Original code added (randomize_short(0x12) & 1) to the base sound
					// id Protoss_ARBITER_PAbFol02_WAV then called the playSound function
					// using that value, but since there are only 2 sounds, it's easier to 
					// see what sound is used that way.
					// The compiler may make it back like the original anyway.

					if ((helper::randomize_short(0x12) & 1) == 0) {
						helper::play_sound_from_unit(SoundId::Protoss_ARBITER_PAbFol02_WAV, unit, 1, 0);
					} else {
						helper::play_sound_from_unit(SoundId::Protoss_ARBITER_PAbFol03_WAV, unit, 1, 0);
					}

					Point16 center = {unit->position.x, unit->position.y};
					Point16 top = center;
					top.y -= 3 * 32;
					scbw::create_and_init_thingy(SpriteId::RecallFieldShorter, center.x, center.y, unit->player, unit->sprite->elevationLevel - 1);
					for (u32 i = 0; i < 8; ++i) {
						Point16 next = scbw::rotate_point(center, top, i * (360 / 8));
						scbw::create_and_init_thingy(SpriteId::RecallFieldShorter, next.x, next.y, unit->player, unit->sprite->elevationLevel - 1);
					}
				}
				unit->order_to_idle();
			}
		} else {
			if (!scbw::is_cheat_enabled(CheatFlags::TheGathering) && unit->energy < techdata_dat::EnergyCost[TechId::Recall] * 256) {
				u8 raceId = unit->get_race();
				char* message = (char*)statTxtTblNew->getString(864 + raceId);	//"Not enough energy" text
				if (message && unit->player == *LOCAL_NATION_ID && *ACTIVE_PLAYER_ID == *LOCAL_HUMAN_ID) {
					helper::setUnitStatTxtErrorMsg(message);
				}
				unit->order_to_idle();
			} else {
				if (!scbw::is_cheat_enabled(CheatFlags::TheGathering)) {
					unit->spend_unit_energy(techdata_dat::EnergyCost[TechId::Recall] * 256);
				}

				if (unit->order_target.unit) {
					unit->order_target.pt.x = unit->order_target.unit->sprite->position.x;
					unit->order_target.pt.y = unit->order_target.unit->sprite->position.y;
				}

				scbw::create_and_init_thingy(SpriteId::Recall_Field, unit->position.x, unit->position.y, unit->player, unit->sprite->elevationLevel - 1);
				scbw::create_and_init_thingy(SpriteId::Recall_Field, unit->order_target.pt.x, unit->order_target.pt.y, unit->player, unit->sprite->elevationLevel + 1);

				u32 random_value = 0;
				if (*IS_IN_GAME_LOOP) {
					// Hardcoded (not using a function) RNG use, like the original code did
					static u32* const u32_0051C654 = (u32*)0x0051C654;
					static u32* const u32_0051CA18 = (u32*)0x0051CA18;

					*u32_0051C654 = *u32_0051C654 + 1;
					*u32_0051CA18 = *u32_0051CA18 + 1;
					*lastRandomNumber = (*lastRandomNumber * 0x015A4E35) + 1;

					random_value = ((*lastRandomNumber) / 65536) & 0x7FFF;
				}

				// Original code added (random_value & 1) to the base sound
				// id Protoss_ARBITER_PAbFol00_WAV then called the playSound function
				// using that value, but since there are only 2 sounds, it's easier to 
				// see what sound is used that way.
				// The compiler may make it back like the original anyway.

				u32 sound_id_source = SoundId::Protoss_ARBITER_PAbFol00_WAV;
				u32 sound_id_target = SoundId::Protoss_ARBITER_PAbFol01_WAV;
				if (random_value & 1 == 0) {
					sound_id_source = SoundId::Protoss_ARBITER_PAbFol01_WAV;
					sound_id_target = SoundId::Protoss_ARBITER_PAbFol00_WAV;
				}
				helper::playSoundAtPos(sound_id_source, unit->position.x, unit->position.y, 1, 0);
				helper::playSoundAtPos(sound_id_target, unit->order_target.pt.x, unit->order_target.pt.y, 1, 0);

				scbw::decloak_proc(unit);
				unit->status |= UnitStatus::CanNotReceiveOrders;
				unit->main_order_timer = 3 * 24; // used to be 22
				unit->main_order_state = 1;
			}
		}
	}
}
