#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	b32 weapon_behavior_can_hit_target_check(Unit* caster, Unit* attacker, Unit* target)
	{
		// code equivalent to is_under_carapace_swarm @ 0x00476210:
		// a function different from scbw::is_under_carapace_swarm(target) that just seeks a nearby Carapace Swarm active spell and is used in here.

		b32 return_value;
		u8 is_under_swarm_status;
		if (target->status & UnitStatus::InAir)
		{
			is_under_swarm_status = 0;
		} else if ((units_dat::base_property[target->id] & UnitProperty::Building) || (*g_neutral_carapace_swarm == 0) || !scbw::is_under_carapace_swarm(target)) {
			s32 target_x = target->get_x();
			s32 target_y = target->get_y();

			if (target_x < 0)
			{
				target_x += 31;
			}
			if (target_y < 0)
			{
				target_y += 31;
			}

			if (scbw::get_active_tile_at_coords(target_x, target_y)->has_doodad_cover)
			{
				is_under_swarm_status = OrderId::CarapaceSwarm; /*0x77*/
			} else {
				is_under_swarm_status = 0;
			}
		} else {
			is_under_swarm_status = 0xFF;
		}

		// back to weapon_behavior_can_hit_target_check code
		if (attacker->status & UnitStatus::InAir)
		{
			return_value = is_under_swarm_status;
		} else if (target->status & UnitStatus::InAir) {
			return_value = is_under_swarm_status;
		} else {
			if (scbw::get_ground_height_at_pos(attacker->get_x(), attacker->get_y()) >= scbw::get_ground_height_at_pos(target->get_x(), target->get_y()))
			{
				return_value = is_under_swarm_status;
			} else if (is_under_swarm_status <= OrderId::CarapaceSwarm) { // code use 0x77, the order seems to fit, but maybe that's just an accident and it's unrelated
				return_value = OrderId::CarapaceSwarm;
			} else {
				return_value = is_under_swarm_status;
			}
		}
		return return_value;
	}

	bool dark_swarm_target_finder_match(Unit* caster, Unit* target)
	{
		bool return_value;
		if (is_target_worth_hitting(target, caster))
		{
			Unit* target_of_target = target->order_target.unit;
			if (target_of_target)
			{
				if  (target_of_target->player < PLAYABLE_PLAYER_COUNT
				&&  !caster->is_target_enemy(target_of_target)
				&&  !target_of_target->subunit
				&& !(target_of_target->status & UnitStatus::InAir))
				{
					u8 tot_ground_weapon = target_of_target->get_ground_weapon();
					if (tot_ground_weapon != WeaponId::None && weapons_dat::behavior[tot_ground_weapon] == WeaponBehavior_AppearOnAttacker)
					{
						u8 target_ground_weapon;
						if (target->subunit->is_subunit())
						{
							target = target->subunit;
						}

						target_ground_weapon = target->get_ground_weapon();
						if (target_ground_weapon != WeaponId::None)
						{
							u8 weapon_behavior = weapons_dat::behavior[target_ground_weapon];
							if (weapons_dat::behavior[target_ground_weapon] == WeaponBehavior_FlyDoNotFollowTarget
							||  weapons_dat::behavior[target_ground_weapon] == WeaponBehavior_FlyFollowTarget
							||  weapons_dat::behavior[target_ground_weapon] == WeaponBehavior_AppearOnTargetUnit
							||  weapons_dat::behavior[target_ground_weapon] == WeaponBehavior_AppearOnTargetSite
							||  weapons_dat::behavior[target_ground_weapon] == WeaponBehavior_Bounce)
							{
								return_value = weapon_behavior_can_hit_target_check(caster, target, target_of_target) == 0;
							} else {
								return_value = false;
							}
						} else {
							return_value = false;
						}
					} else {
						return_value = false;
					}
				} else {
					return_value = false;
				}
			} else {
				return_value = false;
			}
		} else {
			return_value = false;
		}
		return return_value;
	}

	// Based on AI_AssignBestDarkSwarmTarget @ 0x00442840
	Unit* find_best_carapace_swarm_target(Unit* caster, bool isUnderAttack)
	{

		s32 bounds;
		if (isUnderAttack)
		{
			bounds = 32 * 9;	/*0x120*/
		}
		else if (is_campaign_ai(caster->player))
		{
			bounds = 32 * 64;	/*0x800*/
		}
		else
		{
			bounds = 32 * 32;	/*0x400*/
		}
		// Based on AI_BestUnit @ 0x004E8740 and canHitUnit @ 0x00441870
		Unit* result = scbw::UnitFinder::get_nearest_target(
			caster->position.x - bounds,
			caster->position.y - bounds,
			caster->position.x + bounds,
			caster->position.y + bounds,
			caster,
			[&](Unit* target)
		{
			return dark_swarm_target_finder_match(caster, target);
		});

		// Specific to 0x00442840
		if ((result != nullptr) && (result->status & UnitStatus::InAir))
		{
			result = result->order_target.unit;
		}
		return result;
	}
}