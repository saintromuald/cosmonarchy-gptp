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
		if (unit->player == 0) {
			int i = 0;
		}
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
			}
			else if (!unit->main_order_timer) {
			//	unit->main_order_timer = 15;
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

	void create_bunker_attack_overlay(Unit* unit) { // 00477FD0
		// Fixes units with subunits not attacking
		Unit* unitVar = unit;
		if (unit->subunit) {
			unitVar = unit->subunit;
		}

		Image* bunkerImage = unitVar->connected_unit->sprite->main_graphic;
		u8 frameDirection = (unitVar->current_direction_1 + 16) / 32 % 8;
		const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::get_image_data(ImagesDatEntries::OverlayAttackPtr, bunkerImage->id));
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

	// Ported from public GPTP, and partially-unfucked, to remove randomness from attacks. -Pr0nogo
	b32 attack_apply_cooldown(Unit* unit) { // 00478B40

		Unit* target = unit->order_target.unit;
		b32 return_value = 0;
		if (!target)
		{
			return_value = 0;
		}
		else
		{
			helper::pick_new_target_if_needed(unit);
			if (!helper::is_unit_visible(unit)
			||  !unit->can_attack_target(target, true))
			{
				return_value = 0;
			}
			else
			{
				unit->order_target.pt.x = target->sprite->position.x;
				unit->order_target.pt.y = target->sprite->position.y;
				if (unit->subunit && units_dat::base_property[unit->subunit->id] & UnitProperty::Subunit)
				{
					return_value = 1;
				}
				else
				{
					u8 weaponId = unit->get_ground_weapon();
					u8 cooldown = unit->ground_weapon_cooldown;
					if (target->status & UnitStatus::InAir) {
						weaponId = units_dat::AirWeapon[unit->id];
						cooldown = unit->air_weapon_cooldown;
					}

					if (cooldown != 0) {

						if (unit->order_queue_timer > cooldown - 1)
						{
							unit->order_queue_timer = cooldown - 1;
						}
						return_value = 1;
					}
					else if (unit->movement_flags & MovementFlags::StartingAttack)
					{
						return_value = 1;
					}
					else
					{

						u32 distance = helper::get_distance_to_unit(unit, target);
						u32 range_check = weapons_dat::min_range[weaponId];

						if ((range_check != 0 && distance < range_check)
						||   distance > unit->get_max_weapon_range(weaponId))
						{
							return_value = 0;
						}
						else
						{
							s32 angle = scbw::get_angle(unit->sprite->position.x, unit->sprite->position.y, unit->order_target.pt.x, unit->order_target.pt.y);
							angle = unit->current_direction_1 - angle;
							if (angle < 0)
							{
								angle = angle + 256;
							}
							if (angle > 128)
							{
								angle = 256 - angle;
							}

							if (angle > weapons_dat::attack_angle[weaponId])
							{
								if (unit->status & UnitStatus::CanTurnAroundToAttack)
								{
									unit->order_queue_timer = 0;
									return_value = 1;
								}
								else
								{
									return_value = 0;
								}
							}
							else if (unit->status & UnitStatus::CanNotAttack)
							{
								return_value = 0;
							}
							else
							{
								static u32* u32_0064DEB0 = (u32*)0x0064DEB0;
								if (unit->status & UnitStatus::InBuilding)
								{
									helper::create_bunker_attack_overlay(unit);
								}
								unit->movement_flags |= MovementFlags::StartingAttack;
								*u32_0064DEB0 = 0;

								// Previously had random delays here.
								cooldown = helper::get_weapon_cooldown(unit, weaponId);
								unit->ground_weapon_cooldown = cooldown;
								unit->air_weapon_cooldown = cooldown;

								Image* current_image = unit->sprite->images.head;
								IscriptAnimation::Enum iscript_anim = IscriptAnimation::GndAttkRpt;
								if (target->status & UnitStatus::InAir)
								{
									iscript_anim = IscriptAnimation::AirAttkRpt;
								}
								while (current_image)
								{
									current_image->play_iscript_animation(iscript_anim);
									current_image = current_image->link.next;
								}
								return_value = 1;
							}
						}
					}
				}
			}
		}
		return return_value;
	}
}