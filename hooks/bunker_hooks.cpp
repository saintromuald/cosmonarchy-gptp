#include "bunker_hooks.h"
#include <SCBW/scbwdata.h>
#include <SCBW\api.h>
#include "../SCBW/enumerations.h"
#include "../SCBW/structures.h"
#include "globals.h"
#include "scbw/UnitFinder.h"

namespace hooks
{
	static inline bool is_within_attack_range(Unit* attacker, Unit* target)
	{
		if (!target)
		{
			return false;
		}
		if (target->status & UnitStatus::InAir)
		{
			return !attacker->is_target_within_min_range(target, weapons_dat::min_range[attacker->get_air_weapon()])
			&&		attacker->is_target_within_min_range(target, weapons_dat::max_range[attacker->get_air_weapon()]);
		}
		else
		{
			return !attacker->is_target_within_min_range(target, weapons_dat::min_range[attacker->get_ground_weapon()])
			&&		attacker->is_target_within_min_range(target, weapons_dat::max_range[attacker->get_ground_weapon()]);
		}
	}

	// Checks whether the unit can attack from inside a bunker.
	void unit_attacks_from_inside_bunker(Unit* unit) { // 004790A0
		if (unit->connected_unit && unit->connected_unit->id == UnitId::TerranBunker
		&&  unit->connected_unit->status & (UnitStatus::InAir | UnitStatus::InTransport)) {
			return; // Disallow units in Bunkers from firing while Bunker is lifted or in a transport.
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
			} else {
				if (unit->main_order_timer == 0) {
					unit->main_order_timer = 15;
					
					Unit* target = unit->order_target.unit;
					if (!target || !unit->can_attack_target(target) || !is_within_attack_range(unit, target)) {
						if (unit->connected_unit) {
							UnitExt* bunkerExt = get_unit_ext(unit->connected_unit);
							Unit* target = bunkerExt->units[UnitBunkerLockOn];
							if (target && unit->can_attack_target(target) && is_within_attack_range(unit, target)) {
								unit->order_target.unit = bunkerExt->units[UnitBunkerLockOn]; // TODO: Find a way to set bunker lock on from attack order
							}
						}

						if (!target || !unit->can_attack_target(target) || !is_within_attack_range(unit, target)) {
							const u32 radius = std::max(unit->get_max_weapon_range(unit->get_ground_weapon()), unit->get_max_weapon_range(unit->get_air_weapon()));
							unit->order_target.unit = scbw::UnitFinder::get_nearest_target(
								unit->get_x() - radius,
								unit->get_y() - radius,
								unit->get_x() + radius,
								unit->get_y() + radius,
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
	}

	void create_bunker_attack_overlay(Unit* unit) { // 00477FD0
		// Fixes units with subunits not attacking
		Unit* unitVar = unit;
		if (unit->subunit) {
			unitVar = unit->subunit;
		}

		Image* bunkerImage = unitVar->connected_unit->sprite->main_graphic;
		u8 frameDirection = (unitVar->current_direction_1 + 16) / 32 % 8;
		const LO_Header* loFile = lo_files->attackOverlays[bunkerImage->id];
		Point8 offset = loFile->get_offset(bunkerImage->frame_index, frameDirection);

		if (bunkerImage->flags & ImageFlags::Mirrored) { // Is inverted
			offset.x = -offset.x;
		}

		u8 frameAngle = frameDirection * 32;
		u16 spriteId = 377; // Bunker attack overlay
		if (unitVar->id == UnitId::TerranHarakan) {
			frameAngle = ((unitVar->current_direction_1 + 8) / 16 % 16) * 16;
			spriteId = 378; // Firebat flamethrower graphics
		}

		scbw::decloak_proc(unitVar->connected_unit);

		// TODO: Fix effect nonsense with subunits
		Thingy* bunkerAttackEffect = helper::create_thingy(spriteId, offset.x + unitVar->get_x(), offset.y + unitVar->get_y(), unitVar->player);
		if (!bunkerAttackEffect) {
			return;
		}

		bunkerAttackEffect->sprite->elevation_level = unitVar->sprite->elevation_level + 1;
		for (Image* image = bunkerAttackEffect->sprite->images.head; image; image = image->link.next) {
			helper::set_image_direction(image, frameAngle);
		}
		helper::set_thingy_visibility_flags(bunkerAttackEffect);
	}

	// TODO: Fix holdpos bug; units do not turn to face hostiles
	// Hooked to investigate Bunker issues. -Pr0nogo
	u32 attack_apply_cooldown(Unit* unit, u32 holdpos_status) { // 00478B40
		if (!unit->order_target.unit) {
			return false;
		}

		helper::pick_new_target_if_needed(unit); // pick_new_target_if_needed originally used holdpos_status arg, but it seems to be completely unused. -Pr0nogo
		if (!helper::is_unit_visible(unit)) {
			return false;
		}

		if (!helper::unit_can_attack_target(unit->order_target.unit, unit, true)) {
			return false;
		}

		Unit* target = unit->order_target.unit;
		unit->order_target.pt = target->sprite->position;
		Unit* subunit = unit->subunit;
		if (subunit && units_dat::base_property[subunit->id] & UnitProperty::Subunit) {
			return true;
		}

		u32 weapon_id = units_dat::ground_weapon[unit->id];
		u8 timer = unit->ground_weapon_cooldown;
		if (target->status & UnitStatus::InAir) {
			weapon_id = units_dat::air_weapon[unit->id];
			timer = unit->air_weapon_cooldown;
		}

		if (timer) {
			if (unit->order_queue_timer > timer - 1) {
				unit->order_queue_timer = timer - 1;
			}
			return true;
		}
		if (unit->movement_flags & MovementFlags::StartingAttack) {
			return true;
		}

		u32 distance = helper::get_distance_to_unit(unit, target);
		u32 min_range = weapons_dat::min_range[weapon_id];
		if (helper::get_unit_max_weapon_range(unit, weapon_id) < distance || (min_range && min_range > distance)) {
			return false;
		}

		s32 direction = unit->current_direction_1 - helper::get_direction_from_points(unit->order_target.pt.y, unit->sprite->position.x, unit->order_target.pt.x, unit->sprite->position.y);
		s32 angle = direction + (-(direction < 0) & 0x100);
		if (angle > 128) {
			angle -= 256;
		}
		if (angle > weapons_dat::attack_angle[weapon_id]) {
			if (!(unit->status & UnitStatus::CanTurnAroundToAttack)) {
				unit->order_queue_timer = 0;
				return true;
			}
			return false;
		}
		if (unit->status & UnitStatus::CanNotAttack) {
			return false;
		}
		if (unit->status & UnitStatus::InBuilding) {
			helper::create_bunker_attack_overlay(unit);
		}
		unit->movement_flags |= MovementFlags::StartingAttack;
		*g_last_bullet_creator = 0;
		u32 cooldown = helper::get_weapon_cooldown(unit, weapon_id);
		unit->ground_weapon_cooldown = cooldown;
		unit->air_weapon_cooldown = cooldown;

		for (Image* image = unit->sprite->images.head; image; image = image->link.next) {
			helper::play_image_iscript(image, target->status & UnitStatus::InAir ? IscriptAnimation::AirAttkRpt : IscriptAnimation::GndAttkRpt);
		}
		return true;
	}
}