#include "bunker_hooks.h"
#include <SCBW/scbwdata.h>
#include <SCBW\api.h>
#include "../SCBW/enumerations.h"
#include "../SCBW/structures.h"
#include "globals.h"
#include "scbw/UnitFinder.h"
#include "hooks/unit_stats/weapon_range.h"

namespace hooks
{
	static inline bool is_within_attack_range(Unit* attacker, Unit* target) {
		if (!target) {
			return false;
		}
		u32 weapon_id = target->status & UnitStatus::InAir ? attacker->get_air_weapon() : attacker->get_ground_weapon();
		if (weapon_id == WeaponId::None) {
			return false;
		}
		u32 min_range = weapons_dat::min_range[weapon_id];
		u32 range = hooks::get_max_weapon_range(attacker, weapon_id, target);

		return !attacker->is_target_within_min_range(target, min_range)
		&&		attacker->is_target_within_min_range(target, range);
	}

	// Checks whether the unit can attack from inside a bunker.
	void unit_attacks_from_inside_bunker(Unit* unit) { // 004790A0
		if (unit->connected_unit && scbw::unit_id_is_ground_bunker(unit->connected_unit->id)
		&&  unit->connected_unit->status & (UnitStatus::InAir | UnitStatus::InTransport)) {
			return; // Disallow units in Bunkers from firing while Bunker is lifted or in a transport.
		}
		if (unit->subunit)
		{
			if (unit->subunit->connected_unit && scbw::unit_id_is_ground_bunker(unit->subunit->connected_unit->id)
			&&	unit->subunit->connected_unit->status & (UnitStatus::InAir | UnitStatus::InTransport)) {
				return; // Disallow units in Bunkers from firing while Bunker is lifted or in a transport.
			}
		}

		if (scbw::unit_is_ranged_attacker(unit)) {
			unit->status |= UnitStatus::HoldingPosition;
			if (unit->subunit) {
				unit->subunit->status |= UnitStatus::HoldingPosition;
			}
			if (helper::attack_apply_cooldown(unit)) {
				if (unit->order_target.pt.x != unit->next_target_waypoint.x
				||  unit->order_target.pt.y != unit->next_target_waypoint.y) {
					unit->next_target_waypoint.x = unit->order_target.pt.x;
					unit->next_target_waypoint.y = unit->order_target.pt.y;
				}
			}
			else if (!unit->main_order_timer) {
			//	unit->main_order_timer = 15;
				Unit* target = unit->order_target.unit;
				if (!target || !unit->can_attack_target(target) || !is_within_attack_range(unit, target)) {
					if (unit->connected_unit) {
						UnitExt* bunker_ext = get_unit_ext(unit->connected_unit);
						Unit* target = bunker_ext->units[UnitBunkerLockOn];
						if (target && unit->can_attack_target(target) && is_within_attack_range(unit, target)) {
							unit->order_target.unit = bunker_ext->units[UnitBunkerLockOn]; // TODO: Find a way to set bunker lock on from attack order
						}
					}

					if (!target || !unit->can_attack_target(target) || !is_within_attack_range(unit, target)) {
						const u32 radius = std::max(unit->get_max_weapon_range(unit->get_ground_weapon()), unit->get_max_weapon_range(unit->get_air_weapon()));
						unit->order_target.unit = scbw::UnitFinder::get_nearest_target(
							unit->get_left()   - radius,
							unit->get_top()    - radius,
							unit->get_right()  + radius,
							unit->get_bottom() + radius,
							unit,
							[&](Unit* target) {
							if (scbw::is_allied_to(unit->player, target->player)
							|| !unit->can_attack_target(target)
							|| !is_within_attack_range(unit, target)) {
								return false;
							}
							return true;
						});
					}
				}

				if (target) {
					unit->order_queue_timer = 0;
				}
			}
		}
	}

	void create_bunker_attack_overlay(Unit* unit) { // 00477FD0
		// Fixes units with subunits not attacking
		Unit* unit_var = unit;
		if (unit->subunit) {
			unit_var = unit->subunit;
		}

		Image* bunker_image = unit_var->connected_unit->sprite->main_graphic;
		u8 frame_direction = (unit_var->current_direction_1 + 16) / 32 % 8;
		const LO_Header* attack_overlay = lo_files->attack_overlays[bunker_image->id];
		Point8 offset = attack_overlay->get_offset(bunker_image->frame_index, frame_direction);

		if (bunker_image->flags & ImageFlags::Mirrored) { // Is inverted
			offset.x = -offset.x;
		}

		u8 frame_angle = frame_direction * 32;
		u16 sprite_id = SpriteId::BunkerAttackOverlay_Orange; // Bunker attack overlay
		switch (unit_var->get_race())
		{
			case RaceId::Zerg:
			{
				sprite_id = SpriteId::BunkerAttackOverlay_Green;
			} break;
			case RaceId::Protoss:
			{
				sprite_id = SpriteId::BunkerAttackOverlay_Blue;
			} break;
			default:
			{
			} break;
		}
		if (unit_var->id == UnitId::TerranHarakan) {
			frame_angle = ((unit_var->current_direction_1 + 8) / 16 % 16) * 16;
			sprite_id = SpriteId::HarakanFlamethrower;
		}

		scbw::decloak_proc(unit_var->connected_unit);

		// TODO: Fix effect nonsense with subunits
		Thingy* bunker_attack_effect = helper::create_thingy(sprite_id, offset.x + unit_var->get_x(), offset.y + unit_var->get_y(), unit_var->player);
		if (!bunker_attack_effect) {
			return;
		}

		bunker_attack_effect->sprite->elevation = unit_var->sprite->elevation + 1;
		for (Image* image = bunker_attack_effect->sprite->images.head; image; image = image->link.next) {
			helper::set_image_direction(image, frame_angle);
		}
		helper::set_thingy_visibility_flags(bunker_attack_effect);
	}

	// Ported from public GPTP, and partially-unfucked, to remove randomness from attacks. -Pr0nogo
	b32 attack_apply_cooldown(Unit* unit) { // 00478B40
		Unit* target = unit->order_target.unit;
		if (!target) {
			return false;
		}

		helper::pick_new_target_if_needed(unit);
		if (!helper::is_unit_visible(unit) || !unit->can_attack_target(target, true)) {
			return false;
		}

		unit->order_target.pt.x = target->sprite->position.x;
		unit->order_target.pt.y = target->sprite->position.y;
		if (unit->subunit && units_dat::base_property[unit->subunit->id] & UnitProperty::Subunit) {
			return true;
		}

		u8 weapon_id = unit->get_ground_weapon();
		u8 cooldown = unit->ground_weapon_cooldown;
		if (target->status & UnitStatus::InAir) {
			weapon_id = units_dat::air_weapon[unit->id];
			cooldown = unit->air_weapon_cooldown;
		}

		if (cooldown != 0) { 
			if (unit->order_queue_timer > cooldown - 1) {
				unit->order_queue_timer = cooldown - 1;
			}
			return true;
		} 
		
		if (unit->movement_flags & MovementFlags::StartingAttack) {
			return true;
		}

		u32 distance = helper::get_distance_to_unit(unit, target);
		u32 range_check = weapons_dat::min_range[weapon_id];
		if ((range_check != 0 && distance < range_check) || distance > unit->get_max_weapon_range(weapon_id)) {
			return false;
		}

		s32 angle = scbw::get_angle(unit->sprite->position.x, unit->sprite->position.y, unit->order_target.pt.x, unit->order_target.pt.y);
		angle = unit->current_direction_1 - angle;
		if (angle < 0) {
			angle = angle + 256;
		}
		if (angle > 128) {
			angle = 256 - angle;
		}

		if (angle > weapons_dat::attack_angle[weapon_id]) {
			if (unit->status & UnitStatus::CanTurnAroundToAttack) {
				unit->order_queue_timer = 0;
				return true;
			}
			return false;
		}
		if (unit->status & UnitStatus::CanNotAttack) {
			return 0;
		}
		
		if (unit->id == UnitId::TerranIonCannon) {
			UnitExt* unit_ext = get_unit_ext(unit);
			unit_ext->values[ValueIonCannonTargetPosX] = unit->order_target.unit->position.x;
			unit_ext->values[ValueIonCannonTargetPosY] = unit->order_target.unit->position.y;
			unit_ext->units[UnitIonCannonTarget] = unit->order_target.unit;
			unit->order_target.unit = 0;
		}
		if (unit->status & UnitStatus::InBuilding)
		{
			helper::create_bunker_attack_overlay(unit);
		}
		unit->movement_flags |= MovementFlags::StartingAttack;
		*g_last_bullet_creator = 0;

		// Previously had random delays here.
		
		// Cooldown is now applied in fire bullet and attack melee.
	//	cooldown = helper::get_weapon_cooldown(unit, weapon_id);
	//	unit->ground_weapon_cooldown = cooldown;
	//	unit->air_weapon_cooldown = cooldown;

		IscriptAnimation::Enum iscript_anim = IscriptAnimation::GndAttkRpt;
		if (target->status & UnitStatus::InAir) {
			iscript_anim = IscriptAnimation::AirAttkRpt;
		}
		for (Image* current_image = unit->sprite->images.head; current_image; current_image = current_image->link.next) {
			current_image->play_iscript_animation(iscript_anim);
		}
		return true;
	}
}