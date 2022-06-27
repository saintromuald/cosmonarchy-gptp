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
			return !attacker->is_target_within_min_range(target, weapons_dat::MinRange[attacker->get_air_weapon()])
			&&		attacker->is_target_within_min_range(target, weapons_dat::MaxRange[attacker->get_air_weapon()]);
		}
		else
		{
			return !attacker->is_target_within_min_range(target, weapons_dat::MinRange[attacker->get_ground_weapon()])
			&&		attacker->is_target_within_min_range(target, weapons_dat::MaxRange[attacker->get_ground_weapon()]);
		}
	}

	// Checks whether the unit can attack from inside a bunker.
	void unit_attacks_from_inside_bunker(Unit* unit)
	{
	//	if (unit->connected_unit
	//	&&  unit->connected_unit->status & (UnitStatus::InAir | UnitStatus::InTransport))
	//	{
	//		return; // Disallow units in Bunkers from firing while Bunker is lifted or in a transport.
					// This is BUGGED! It often stops units from firing even when the bunker is not in transport. Maybe connected_unit is wrong? -Pr0nogo
	//	}
		if (scbw::unit_is_ranged_attacker(unit))
		{
			b32 resultApplyCooldown;
			unit->status |= UnitStatus::HoldingPosition;
			if (unit->subunit != nullptr)
			{
				(unit->subunit)->status |= UnitStatus::HoldingPosition;
			}
			resultApplyCooldown = helper::attack_apply_cooldown(unit);
			if (resultApplyCooldown != 0)
			{
				if (unit->order_target.pt.x != unit->next_target_waypoint.x
				||  unit->order_target.pt.y != unit->next_target_waypoint.y)
				{
					unit->next_target_waypoint.x = unit->order_target.pt.x;
					unit->next_target_waypoint.y = unit->order_target.pt.y;
				}
			}
			else
			{
				if (unit->main_order_timer == 0)
				{
					unit->main_order_timer = 15;
					
					Unit* target = unit->order_target.unit;
					if (!target || !unit->can_attack_target(target) || !is_within_attack_range(unit, target))
					{
						if (unit->connected_unit)
						{
							UnitExt* bunkerExt = globals.get_unit_ext(unit->connected_unit);
							Unit* target = bunkerExt->units[UnitBunkerLockOn];
							if (target && unit->can_attack_target(target) && is_within_attack_range(unit, target))
							{
								unit->order_target.unit = bunkerExt->units[UnitBunkerLockOn]; // TODO: Find a way to set bunker lock on from attack order
							}
						}

						if (!target || !unit->can_attack_target(target) || !is_within_attack_range(unit, target))
						{
							const u32 radius = std::max(unit->get_max_weapon_range(unit->get_ground_weapon()), unit->get_max_weapon_range(unit->get_air_weapon()));
							unit->order_target.unit = scbw::UnitFinder::get_nearest_target(
								unit->get_x() - radius,
								unit->get_y() - radius,
								unit->get_x() + radius,
								unit->get_y() + radius,
								unit,
								[&](Unit* target)
							{
								if (scbw::is_allied_to(unit->player, target->player)
								|| !unit->can_attack_target(target)
								|| !is_within_attack_range(unit, target))
								{
									return false;
								}
								return true;
							});
						}
					}

					if (target != nullptr)
					{
						unit->order_queue_timer = 0;
					}
				}
			}
		}
	}

	void create_bunker_attack_overlay(Unit* unit)
	{
		// Fixes units with subunits not attacking
		Unit* unitVar = unit;
		if (unit->subunit)
		{
			unitVar = unit->subunit;
		}

		Image* bunkerImage = unitVar->connected_unit->sprite->main_graphic;
		u8 frameDirection = (unitVar->current_direction_1 + 16) / 32 % 8;
		const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::get_image_data(ImagesDatEntries::OverlayAttackPtr, bunkerImage->id));
		Point8 offset = loFile->get_offset(bunkerImage->frame_index, frameDirection);

		if (bunkerImage->flags & ImageFlags::Mirrored) // Is inverted
		{
			offset.x = -offset.x;
		}

		u8 frameAngle;
		u16 spriteId;
		if (unitVar->id == UnitId::TerranHarakan)
		{
			frameAngle = ((unitVar->current_direction_1 + 8) / 16 % 16) * 16;
			spriteId = 378; // Firebat flamethrower graphics
		}
		else
		{
			frameAngle = frameDirection * 32;
			spriteId = 377; // Bunker attack overlay
		}

		scbw::decloak_proc(unitVar->connected_unit);

		// TODO: Fix effect nonsense with subunits
		Thingy* bunkerAttackEffect = helper::create_thingy(spriteId, offset.x + unitVar->get_x(), offset.y + unitVar->get_y(), unitVar->player);
		if (!bunkerAttackEffect)
		{
			return;
		}

		bunkerAttackEffect->sprite->elevation_level = unitVar->sprite->elevation_level + 1;
		for (Image* image = bunkerAttackEffect->sprite->images.head;
			image != nullptr;
			image = image->link.next)
		{
			helper::set_image_direction(image, frameAngle);
		}
		helper::set_thingy_visibility_flags(bunkerAttackEffect);
	}
}