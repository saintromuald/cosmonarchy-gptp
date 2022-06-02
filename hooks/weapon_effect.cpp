#include "weapon_effect.h"
#include <SCBW/UnitFinder.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "hooks/weapons/weapon_spellhit.h"
#include "globals.h"
#include "weapons/weapon_damage.h"
#include "limits/ccmu.h"

namespace
{
	// SC internal data for Lurker spines
	struct SpineHitTable
	{
		struct
		{
			Unit* source;
			Unit* target;
		} hits[32][16];
	};
	SpineHitTable* const spineHitTable = (SpineHitTable*)(0x0064DEC8);
	u32* const spineHitRow = (u32*)(0x64EEC8);
	u32* const spineHitCol = (u32*)(0x64DEA8);
}

namespace hooks
{
	void darkSwarmDamage(u16 x, u16 y, u16 r, Bullet* bullet, u32 weaponId)
	{
		Unit* dsw = scbw::get_dark_swarm_from_position(x, y, r);
		if (dsw != nullptr)
		{
			u32 damage = (weapons_dat::DamageAmount[weaponId] * 256) +
				(weapons_dat::DamageBonus[weaponId] * 256 * scbw::get_upgrade_level(bullet->srcPlayer, weapons_dat::DamageUpgrade[weaponId]));
			if (bullet->hitFlags & 2)
			{
				damage /= 2;
			}
			if (damage >= dsw->hitPoints)
			{
				dsw->remove();
			}
			else
			{
				u32 old = dsw->hitPoints;
				dsw->hitPoints -= damage;
			}
		}
	}

	bool nathrokorCondition(Bullet* bullet, Unit* target)
	{
		if (bullet->sourceUnit == nullptr || target == nullptr)
		{
			return true;
		}
		if (bullet->weaponId == WeaponId::NathrokorPlasmidBreath)
		{
			Unit* nathrokor = bullet->sourceUnit;
			u16 angle = scbw::get_angle(nathrokor->position.x, nathrokor->position.y,
				target->position.x, target->position.y);
			u16 unit_angle = nathrokor->velocityDirection1;
			s32 arc = (f64)15 / 1.40625;
			// 30 degrees
			s32 delta = unit_angle - angle;
			delta = abs((delta + 128) % 256 - 128);
			if (delta > arc)
			{
				return false;
			}
			else
			{
			}
		}
		return true;
	}

	void splash(Bullet* bullet, u32 weaponId, u8 splashType, Unit* guaranteedHit)
	{
		s32 splash = splashType;
		if (bullet == nullptr)
			return;
		if (bullet->sourceUnit != nullptr && bullet->sourceUnit->sprite != nullptr)
		{
			UnitExt* bulletSourceExt = globals.get_unit_ext(bullet->sourceUnit);

			if (bullet->sourceUnit->isBlind)
			{
				splash = SplashType::Radial;
			}

			Point16 hit = bullet->position;
			switch (weaponId)
			{
				case WeaponId::SalamanderPerditionBarrage:
				{
					globals.create_effect(EffectId::SalamanderFire, 5 * 24, {hit.x, hit.y}, 0.5 * 32, SpriteId::SalamanderImpact, 5, bullet->srcPlayer, bullet->sourceUnit);
					break;
				}
				case WeaponId::KeskathalorScreamingVenom:
				{
					globals.create_effect(EffectId::CausticDischarge, 5 * 24, {hit.x, hit.y}, 1.5 * 32, SpriteId::CausticDischarge, 5, bullet->srcPlayer, bullet->sourceUnit);
					break;
				}
				case WeaponId::IonCannon:
				{
					globals.create_effect(EffectId::IonField, 10 * 24, {hit.x, hit.y}, 3 * 32, SpriteId::IonField, 5, bullet->srcPlayer, bullet->sourceUnit);
					break;
				}
				case WeaponId::CantavisHallucinationCast:
				{
					globals.create_effect(EffectId::Hallucination, 10 * 24, {hit.x, hit.y}, 1.5 * 32, SpriteId::ProtossHallucinationEffect, 5, bullet->srcPlayer, bullet->sourceUnit);
					break;
				}
			}
		}

		s16 x, y, radius;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		radius = weapons_dat::OuterSplashRadius[weaponId];

		// Probing Incisors - Gorgoleth passive
		if (isChargedGorgolethBullet(bullet, weaponId))
		{
			radius = 45;
		}
		// Nathrokor weapon
		if (weaponId == WeaponId::NathrokorPlasmidBreath)
		{
			radius = 32;
		}

		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(
			x - radius,
			y - radius,
			x + radius,
			y + radius
		);
		darkSwarmDamage(x, y, radius, bullet, weaponId);

		bool hasGuaranteedHitUnit = guaranteedHit != nullptr;
		if (hasGuaranteedHitUnit && nathrokorCondition(bullet, bullet->attackTarget.unit))
		{
			helper::weaponBulletShot(guaranteedHit, bullet, 1);
		}
		for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
		{
			Unit* target = unitsInSplash.getUnit(i);
			if (hasGuaranteedHitUnit && target == guaranteedHit)
			{ // skips target check if hasGuaranteedHitUnit is unset, just for little bit of performance
				continue; // guaranteed hit unit was already damaged before splash!
			}

			// Units can't splash themselves -- except psi storm
			if (target == bullet->sourceUnit && bullet->weaponId != WeaponId::PsiStorm) continue;

			// Don't hit allied units -- normally just checks target->playerId == bullet->srcPlayer (also checks if type is 'splash enemy' or 'nuclear missile')
			// if(scbw::isUnitEnemy(bullet->srcPlayer, target) == false && target != bullet->attackTarget.unit) continue;

			if (splash == SplashType::Enemy)
			{
				if (bullet->srcPlayer == target->player && target != bullet->attackTarget.unit) continue;
			}

			else if (splash == SplashType::FriendlyNew)
			{
				if (scbw::isUnitEnemy(bullet->srcPlayer, target) == false && target != bullet->attackTarget.unit) continue;
			}

			// Can the unit be targeted?
			if (!scbw::can_weapon_target_unit(bullet->weaponId, target, target)) continue;

			// We found units in a square around the target, but is the target within the splash radius?
			s16 outer = weapons_dat::OuterSplashRadius[bullet->weaponId];

			// Probing Incisors - Gorgoleth passive
			if (isChargedGorgolethBullet(bullet, weaponId))
			{
				outer = 45;
			}

			// Nathrokor weapon
			if (weaponId == WeaponId::NathrokorPlasmidBreath)
			{
				outer = 32;
			}

			if (weaponId == WeaponId::HeraclesJavelinCannons)
			{
				if (bullet->sourceUnit)
				{
					UnitExt* bulletSourceExt = globals.get_unit_ext(bullet->sourceUnit);

					const f64 splashBase = weapons_dat::OuterSplashRadius[WeaponId::HeraclesJavelinCannons];
					const u32 stacks = bulletSourceExt->values[ValueNemeanReactorsStack];
					outer = (s16)(splashBase * (1 + 0.1 * stacks));
				}
			}

			if (!helper::isUnitDistanceWithin(target, bullet, outer)) continue;

			// Don't storm a unit that's already being stormed
			if (bullet->weaponId == WeaponId::PsiStorm)
			{
				if (target->isUnderStorm) continue;
				target->isUnderStorm = 1;
			}

			// Inner radius 
			s16 inner = weapons_dat::InnerSplashRadius[bullet->weaponId];

			// Probing Incisors - Gorgoleth passive
			if (isChargedGorgolethBullet(bullet, weaponId))
			{
				inner = 30;
			}

			if (weaponId == WeaponId::HeraclesJavelinCannons)
			{
				if (bullet->sourceUnit)
				{
					UnitExt* bulletSourceExt = globals.get_unit_ext(bullet->sourceUnit);

					const f64 splashBase = weapons_dat::InnerSplashRadius[WeaponId::HeraclesJavelinCannons];
					const u32 stacks = bulletSourceExt->values[ValueNemeanReactorsStack];
					inner = (s16)(splashBase * (1 + 0.1 * stacks));
				}
			}

			if (helper::isUnitDistanceWithin(target, bullet, inner) && nathrokorCondition(bullet, target))
			{
				helper::weaponBulletShot(target, bullet, 1); // 100% damage
				continue;
			}

			// Burrowed units can only be hit by inner radius
			if (target->status & UnitStatus::Burrowed) continue;

			// Medium radius
			s16 medium = weapons_dat::MediumSplashRadius[bullet->weaponId];

			// Probing Incisors - Gorgoleth passive
			if (isChargedGorgolethBullet(bullet, weaponId))
			{
				medium = 37;
			}

			if (weaponId == WeaponId::HeraclesJavelinCannons)
			{
				if (bullet->sourceUnit)
				{
					UnitExt* bulletSourceExt = globals.get_unit_ext(bullet->sourceUnit);

					const f64 splashBase = weapons_dat::MediumSplashRadius[WeaponId::HeraclesJavelinCannons];
					const u32 stacks = bulletSourceExt->values[ValueNemeanReactorsStack];
					medium = (s16)(splashBase * (1 + 0.1 * stacks));
				}
			}

			if (weaponId == WeaponId::NathrokorPlasmidBreath)
			{
				radius = 32;
			}

			if (helper::isUnitDistanceWithin(target, bullet, medium) && nathrokorCondition(bullet, target))
			{
				helper::weaponBulletShot(target, bullet, 2); // 50% damage
				continue;
			}

			// Outer radius
			if (nathrokorCondition(bullet, target))
			{
				helper::weaponBulletShot(target, bullet, 4); // 25% damage
			}
		}
	}

	void lurkerSplash(Bullet* bullet, u32 weaponId, u8 splash)
	{
		s16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::OuterSplashRadius[weaponId];

		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);

		for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
		{
			Unit* target = unitsInSplash.getUnit(i);

			// Units can't splash themselves
			if (target == bullet->sourceUnit) continue;

			// Don't hit allied units -- normally just checks target->playerId == bullet->srcPlayer
//			if (scbw::isUnitEnemy(bullet->srcPlayer, target) == false && target != bullet->attackTarget.unit) continue;
			if (splash == SplashType::Enemy)
			{
				if (bullet->srcPlayer == target->player && target != bullet->attackTarget.unit) continue;
			}
			else if (splash == SplashType::FriendlyNew)
			{
				bool lurkerOverride = false;
				if (bullet->sourceUnit)
				{
					UnitExt* bulletSourceExt = globals.get_unit_ext(bullet->sourceUnit);
					if ((Unit*)bulletSourceExt->values[ValueLurkerPickTarget] == target)
					{
						lurkerOverride = true;
					}
				}
					
				if (scbw::isUnitEnemy(bullet->srcPlayer, target) == false && !lurkerOverride && target != bullet->attackTarget.unit)
				{
					continue;
				}
			}

			// Can the unit be targetted?
			if (!scbw::can_weapon_target_unit(bullet->weaponId, target, target)) continue;

			// We found units in a square around the target, but is the target within the splash radius?
			if (!helper::isUnitDistanceWithin(target, bullet, weapons_dat::InnerSplashRadius[bullet->weaponId]))
			{
				continue;
			}

			if (bullet->sourceUnit != nullptr)
			{
				bool cont = false;
				if (globals.spine_pair_exists(bullet->sourceUnit, target))
				{
					cont = true;
					break;
				}
				if (cont)
					continue;
				globals.add_lurker_spine_pair(bullet->sourceUnit, target);
			}
			helper::weaponBulletShot(target, bullet, 1);
		}
	}

	bool isChargedGorgolethBullet(Bullet* bullet, u32 weaponId)
	{
		if (weaponId == WeaponId::GorgrokorSerratedCleavers)
		{
			if (bullet != nullptr && bullet->sourceUnit != nullptr && bullet->sourceUnit->id == UnitId::ZergGorgrokor)
			{
				return true;
			}
		}
		return false;
	}

	bool isUnsafeCyprianBullet(Bullet* bullet, u32 weaponId)
	{
		if (weaponId == 26)
		{
			if (bullet != nullptr)
			{
				if (bullet->sourceUnit != nullptr)
				{
					if (scbw::get_aise_value(bullet->sourceUnit, nullptr, AiseId::SendCyprianValue, 0, 0) == 1)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	// clone of 'Normal' damage for reference
	void normalDamage(Bullet* bullet, u32 weaponId)
	{
		// Create Lobotomy Mines
		if (bullet->sourceUnit != nullptr && weaponId == WeaponId::Lockdown)
		{
			Unit* mine = scbw::create_unit_ingame(UnitId::TerranLobotomyMine, bullet->srcPlayer, bullet->position.x, bullet->position.y);
			if (mine != nullptr)
			{
				helper::turnUnit(mine, bullet->sourceUnit->currentDirection1);
			}
		}

		if (isChargedGorgolethBullet(bullet, weaponId))
		{
			splash(bullet, weaponId, SplashType::FriendlyNew, bullet->attackTarget.unit);
			return;
		}

		Unit* target = bullet->attackTarget.unit;
		if (target == nullptr)
		{
			return;
		}

		darkSwarmDamage(bullet->attackTarget.unit->position.x, bullet->attackTarget.unit->position.y, 1, bullet, weaponId);

		// Kalkalisk behavior
		if (bullet->weaponId == WeaponId::KalkaliskViscousAcid)
		{
			auto count = globals.get_multihit_count(bullet);

			if (count >= 4)
			{
				return;
			}

			target->damage_with_custom((weapons_dat::DamageAmount[WeaponId::KalkaliskViscousAcid] - count) * 256,
				weapons_dat::DamageType[WeaponId::KalkaliskViscousAcid],
				bullet->sourceUnit, bullet->srcPlayer, 0, 1, false);

			return;
		}

		u8 divisor = 1;
		if (weaponId == WeaponId::ArchitectDispersiveSalvo)
		{
			divisor = globals.get_divider(bullet);
		}

		else if (weaponId == WeaponId::AcidSpore
			&& bullet->sourceUnit
			&& bullet->sourceUnit->sprite
			&& target->sprite)
		{
			// Searing Acid - Geszithalor behavior
			UnitExt* bulletSourceExt = globals.get_unit_ext(bullet->sourceUnit);
			auto X = bulletSourceExt->last_bullet_launch_position.x;
			auto Y = bulletSourceExt->last_bullet_launch_position.y;
			divisor = scbw::get_distance_fast(X, Y, target->get_x(), target->get_y()) / (32 * 3);
		}

		if (bullet->hitFlags & 1) // Miss?
		{
			if (target->unit_ai == nullptr)
			{
				return;
			}
			if (bullet->sourceUnit == nullptr)
			{
				return;
			}
			helper::doWeaponHit(target, bullet->sourceUnit, divisor);
			return;
		}
		else
		{
			if (weapons_dat::Behavior[weaponId] == WeaponBehavior::Bounce)
			{
				if (bullet)
				{
					if (bullet->sourceUnit && bullet->sourceUnit->id == UnitId::ZergBactalisk)
					{
						divisor = 1;
					}
					else
					{
						const s32 bounceCount = 2;
						divisor = bounceCount - bullet->remainingBounces;
					}
				}
			}
			helper::weaponBulletShot(target, bullet, divisor);
		}
	}

	// case WPNEffect_Splash_Air:
	void airSplash(Bullet* bullet, u32 weaponId) // 0048B790
	{
		Unit* target = bullet->attackTarget.unit;
		if (target == nullptr)
		{
			return;
		}
		s16 radius = weapons_dat::OuterSplashRadius[weaponId];
		Sprite* bulletSprite = bullet->sprite;
		s16 posX = bulletSprite->position.x;
		s16 posY = bulletSprite->position.y;
		s32 list;
		Box16* searchRadius;

		*airSplashListCount = 0;
		searchRadius->left = posX - radius;
		searchRadius->top = posY - radius;
		searchRadius->right = radius + posX;
		searchRadius->bottom = radius + posY;

		s16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::OuterSplashRadius[weaponId];
		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);

		darkSwarmDamage(x, y, r, bullet, weaponId);

		s32 result;
		for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
		{
			// airSplashProc
			s8 weaponIdLoop;
			s32 listCount;
			Unit* target = unitsInSplash.getUnit(i);

			weaponIdLoop = bullet->weaponId;
			if (target != bullet->sourceUnit && (target->player != bullet->srcPlayer || target == bullet->attackTarget.unit))
			{
				if (scbw::can_weapon_target_unit(weaponIdLoop, target, target))
				{
					if (helper::isUnitDistanceWithin(target, bullet, weapons_dat::OuterSplashRadius[weaponIdLoop]))
					{
						if (helper::isUnitDistanceWithin(target, bullet, weapons_dat::InnerSplashRadius[weaponIdLoop])
							&& target == bullet->attackTarget.unit)
						{
							if (globals.extended_unit_table)
							{
								new_unit_table.air_splash_results[0] = target;
							}
							else
							{
								airSplashList[0] = target;
							}
							result = 1;
							*airSplashListCount = 1;
							break;
						}
						listCount = *airSplashListCount;
						if (globals.extended_unit_table)
						{
							new_unit_table.air_splash_results[*airSplashListCount] = target;
						}
						else
						{
							airSplashList[*airSplashListCount] = target;
						}
						*airSplashListCount = listCount + 1;
					}
				}
			}
		}

		if (airSplashListCount)
		{
			if (*airSplashListCount == 1)
			{
				list = 0;
			}
			else
			{
				list = helper::randomize_short(58) % *airSplashListCount;
			}
			if (globals.extended_unit_table)
			{
				helper::weaponBulletShot(target, bullet, (s32)new_unit_table.air_splash_results[list]);
			}
			else
			{
				helper::weaponBulletShot(target, bullet, (s32)airSplashList[list]);
			}
		}
		// airSplashProc2
		splash(bullet, weaponId, SplashType::FriendlyNew);
	}

	void nuke(Bullet* bullet, u32 weaponId)
	{
		Vector32 pos = { bullet->position.x, bullet->position.y } ;

		globals.create_effect(EffectId::GroundZero, 24, pos, 0, -1, 5, bullet->srcPlayer, bullet->sourceUnit);

		splash(bullet, weaponId, SplashType::Radial);
	}

	// clone of 'Yamato' damage for reference
	void yamatoDamage(Bullet* bullet, u32 weaponId)
	{
		if (bullet->attackTarget.unit == nullptr)
		{
			return;
		}
		darkSwarmDamage(bullet->attackTarget.unit->position.x, bullet->attackTarget.unit->position.y, 1, bullet, weaponId);
		helper::weaponBulletShot(bullet->attackTarget.unit, bullet, 1);
	}

	void parasite(Bullet* bullet, u32 weaponId)
	{
		if (bullet->sourceUnit == nullptr)
		{
			return;
		}
		constexpr u32 radius = 96;
		scbw::UnitFinder(
			bullet->position.x - radius / 2,
			bullet->position.y - radius / 2,
			bullet->position.x + radius / 2,
			bullet->position.y + radius / 2
		).for_each([&](Unit* unit)
		{
			scbw::applyParasite(unit, bullet->sourceUnit->player, bullet->sourceUnit);
		});
	}

	void broodlings(Bullet* bullet, u32 weaponId)
	{
		if (bullet->sourceUnit == nullptr)
		{
			return;
		}
		Unit* target = bullet->attackTarget.unit;
		hooks::BroodlingHit(bullet->sourceUnit, target, bullet);
	}

	void lockdown(Bullet* bullet, u32 weaponId)
	{
		auto targPos = bullet->targetPosition;
		s16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::OuterSplashRadius[WeaponId::LobotomyMineExplosion];
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);
		for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
		{
			Unit* target = unitsInSplash.getUnit(i);
			if (!target->isDead()
				&& !units_dat::GroupFlags[target->id].isBuilding
				&& !(units_dat::BaseProperty[target->id] & UnitProperty::ResourceContainer)
				&& target->id != UnitId::TerranSpiderMine
				&& target->id != UnitId::TerranLobotomyMine)
			{
				hooks::LockdownHit(target, LobotomyMineLockdownTimer, bullet);
			}
		}
	}
}