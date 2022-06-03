#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	b32 weaponBehaviourCanHitTargetCheck(Unit* caster, Unit* attacker, Unit* target)
	{
		b32 returnValue;
		const u32* offset_0x005873F0 = (u32*)0x005873F0;
		u8 isUnderDarkSwarmStatus;

		// code equivalent to is_under_dark_swarm @ 0x00476210, a function different from scbw::is_under_dark_swarm(target) that just seek a nearby 
		// Dark Swarm active spell and is used in here.

		if (target->status & UnitStatus::InAir)
		{
			isUnderDarkSwarmStatus = 0;
		}
		else
			if ((units_dat::BaseProperty[target->id] & UnitProperty::Building)
				|| (*offset_0x005873F0 == 0)
				|| !scbw::is_under_dark_swarm(target))
			{
				s32 targetX = target->get_x();
				s32 targetY = target->get_y();

				if (targetX < 0)
				{
					targetX += 31;
				}
				if (targetY < 0)
				{
					targetY += 31;
				}

				if (scbw::get_stupid_reference_to_active_tile_at(targetX, targetY).has_doodad_cover)
				{
					isUnderDarkSwarmStatus = OrderId::DarkSwarm; /*0x77*/
				}
				else
				{
					isUnderDarkSwarmStatus = 0;
				}
			}
			else
			{
				isUnderDarkSwarmStatus = 0xFF;
			}

		// back to weaponBehaviourCanHitTargetCheck code

		if (attacker->status & UnitStatus::InAir)
		{
			returnValue = isUnderDarkSwarmStatus;
		}
		else if (target->status & UnitStatus::InAir)
		{
			returnValue = isUnderDarkSwarmStatus;
		}
		else
		{
			if (scbw::get_ground_height_at_pos(attacker->get_x(), attacker->get_y()) >= scbw::get_ground_height_at_pos(target->get_x(), target->get_y()))
			{
				returnValue = isUnderDarkSwarmStatus;
			}
			else if (isUnderDarkSwarmStatus <= OrderId::DarkSwarm)	// code use 0x77, the order seems to fit, but maybe that's just an accident and it's unrelated
			{
				returnValue = OrderId::DarkSwarm;
			}
			else
			{
				returnValue = isUnderDarkSwarmStatus;
			}
		}
		return returnValue;
	}

	bool DarkSwarmTargetFinderMatch(Unit* caster, Unit* target)
	{
		bool bReturnValue;
		if (isTargetWorthHitting(target, caster))
		{
			Unit* targetOfTarget = target->order_target.unit;

			if (targetOfTarget != nullptr)
			{

				if (targetOfTarget->player < PLAYABLE_PLAYER_COUNT
					&& !caster->isTargetEnemy(targetOfTarget)
					&& targetOfTarget->subunit == nullptr
					&& !(targetOfTarget->status & UnitStatus::InAir))
				{

					u8 totGroundWeapon = targetOfTarget->get_ground_weapon();

					if (totGroundWeapon != WeaponId::None && weapons_dat::Behavior[totGroundWeapon] == WeaponBehavior::AppearOnAttacker)
					{
						u8 targetGroundWeapon;

						if (target->subunit->is_subunit())
						{
							target = target->subunit;
						}

						targetGroundWeapon = target->get_ground_weapon();
						if (targetGroundWeapon != WeaponId::None)
						{
							u8 weapon_behavior = weapons_dat::Behavior[targetGroundWeapon];

							if (weapons_dat::Behavior[targetGroundWeapon] == WeaponBehavior::Fly_DoNotFollowTarget
								|| weapons_dat::Behavior[targetGroundWeapon] == WeaponBehavior::Fly_FollowTarget
								|| weapons_dat::Behavior[targetGroundWeapon] == WeaponBehavior::AppearOnTargetUnit
								|| weapons_dat::Behavior[targetGroundWeapon] == WeaponBehavior::AppearOnTargetSite
								|| weapons_dat::Behavior[targetGroundWeapon] == WeaponBehavior::Bounce)
							{
								bReturnValue = weaponBehaviourCanHitTargetCheck(caster, target, targetOfTarget) == 0;
							}
							else
							{
								bReturnValue = false;
							}
						}
						else
						{
							bReturnValue = false;
						}
					}
					else
					{
						bReturnValue = false;
					}
				}
				else
				{
					bReturnValue = false;
				}
			}
			else
			{
				bReturnValue = false;
			}
		}
		else
		{
			bReturnValue = false;
		}
		return bReturnValue;
	}

	// Based on AI_AssignBestDarkSwarmTarget @ 0x00442840
	Unit* findBestDarkSwarmTarget(Unit* caster, bool isUnderAttack)
	{

		s32 bounds;
		if (isUnderAttack)
		{
			bounds = 32 * 9;	/*0x120*/
		}
		else if (isUmsMode(caster->player))
		{
			bounds = 32 * 64;	/*0x800*/
		}
		else
		{
			bounds = 32 * 32;	/*0x400*/
		}
		// Based on AI_BestUnit @ 0x004E8740 and canHitUnit @ 0x00441870
		Unit* result = scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster,
			[&](Unit* target)
		{
			return DarkSwarmTargetFinderMatch(caster, target);
		});

		// Specific to 0x00442840
		if ((result != nullptr) && (result->status & UnitStatus::InAir))
		{
			result = result->order_target.unit;
		}
		return result;
	}
}