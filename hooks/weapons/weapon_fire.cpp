//Source file for the Weapon Fire hook module.
//This hook controls how weapons are fired.
#include "weapon_fire.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "SCBW/UnitFinder.h"
#include "hooks/cosmonarchy_finder.h"
#include "math_utils.h"
#include "hooks/byte_operations_utils.h"
#include "globals.h"
#include "hooks/unit_stats/weapon_range.h"
#include "weapon_damage.h"
#include "weapon_spellhit.h"
#include <set>
#include "weapon_cooldown.h"

//-------- Helper function declarations. Do NOT modify! ---------//

static bool madrigalFuriaMatch(Unit* madrigal, Unit* unit)
{
	if (unit == madrigal)
	{
		return false;
	}
	if (unit == madrigal->orderTarget.unit)
	{
		return false;
	}
	if (scbw::isAlliedTo(madrigal->player, unit->player))
	{
		return false;
	}
	if (!madrigal->canAttackTarget(unit))
	{
		return false;
	}
	return true;
}

static bool alkajSporeMatch(Bullet* bullet, Unit* unit)
{
	if (unit->status & UnitStatus::Invincible)
	{
		return false;
	}
	if (scbw::isAlliedTo(bullet->srcPlayer, unit->player))
	{
		return false;
	}
	return true;
}

namespace
{
	typedef void(__stdcall* GetWeaponHitPosFunc)(Unit* unit, s32* x, s32* y);
	GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc)0x004762C0;
}

//-------- Actual hook functions --------//

namespace hooks
{
	static void launchMissile(Unit* attacker, UnitExt* attackerExt, Unit* target, u8 weaponId)
	{
		s32 posX;
		s32 posY;
		if (weapons_dat::Behavior[weaponId] & WeaponBehavior::AppearOnTargetUnit)
		{
			posX = target->getX();
			posY = target->getY();
		}
		else
		{
			posX = attacker->getX();
			posY = attacker->getY();
		}
		auto newAngle = scbw::getAngle(attacker->getX(), attacker->getY(), target->getX(), target->getY());
		helper::createBullet(weaponId, attacker, posX, posY, attacker->player, newAngle);
		if (attackerExt->values[ValueDeconstruction])
		{
			scbw::doDeconstructionDamage(attacker, weaponId, false);
		}
	}

	bool fireMultiple(Unit* attacker, UnitExt* attackerExt, u32 weaponId, u32 maxTargets, u32 flags, bool ignoreMain)
	{
		bool fired = false;
		std::unordered_set<Unit*> targets;
		if (!ignoreMain
			&& attacker->orderTarget.unit
			&& attacker->mainOrderId != OrderId::Follow
			&& attacker->mainOrderId != OrderId::Patrol)
		{
			targets.insert(attacker->orderTarget.unit);
			launchMissile(attacker, attackerExt, attacker->orderTarget.unit, weaponId);
			fired = true;
		}
		Unit* old = attacker->orderTarget.unit;
		for (s32 i = 0; i < maxTargets || maxTargets <= 0; i++)
		{
			u32 range = hooks::getMaxWeaponRangeHook(attacker, weaponId, 0);
			auto filter = cosmonarchyFinderMatch(attacker, flags, {});
			Unit* newTarget = scbw::UnitFinder::getNearestTarget(
				attacker->getX() - range,
				attacker->getY() - range,
				attacker->getX() + range,
				attacker->getY() + range,
				attacker,
				[&](Unit* target)
			{
				if (target == attacker
					|| targets.find(target) != targets.end()
					|| scbw::isAlliedTo(attacker->player, target->player)
					|| !attacker->canAttackTarget(target)
					|| !filter(target)
					|| (targets.size() >= maxTargets && maxTargets > 0))
				{
					return false;
				}
				return true;
			});
			if ((targets.size() >= maxTargets && maxTargets > 0) || !newTarget)
			{
				break;
			}
			attacker->orderTarget.unit = newTarget;
			launchMissile(attacker, attackerExt, newTarget, weaponId);
			fired = true;
			targets.insert(newTarget);
		}
		attacker->orderTarget.unit = old;
		return fired;
	}

	// Fires a weapon from the @p unit.
	// This hook affects the following iscript opcodes: attackwith, attack, castspell
	// This also affects Unit::fireWeapon().
	void fireWeaponHook(Unit* unit, u8 wId)
	{
		if (unit == nullptr)
		{
			return;
		}
		UnitExt* unitExt = globals.get_unit_ext(unit);
		u8 weaponId = wId;
		unitExt->values[ValueAI_KitingSignal] += (1 << 8) / weapons_dat::DamageFactor[weaponId];
		// Retrieve the spawning position for the bullet.
		s32 x, y;
		u8 behavior = weapons_dat::Behavior[weaponId];
		if (weaponId == WeaponId::SavantPowerSiphon)
		{
			behavior = WeaponBehavior::AppearOnTargetSite;
		}
		if (behavior == WeaponBehavior::AppearOnTargetUnit)
		{
			if (unit->orderTarget.unit == nullptr)
			{
				return;
			}
			getWeaponHitPos(unit, &x, &y);
		}
		else if (behavior == WeaponBehavior::AppearOnTargetSite)
		{
			x = unit->orderTarget.pt.x;
			y = unit->orderTarget.pt.y;
		}
		else
		{
			s32 forwardOffset = weapons_dat::ForwardOffset[weaponId];

			x = unit->getX() + scbw::getPolarX(forwardOffset, unit->currentDirection1);
			y = unit->getY() + scbw::getPolarY(forwardOffset, unit->currentDirection1)
				- weapons_dat::VerticalOffset[weaponId];
		}

		// Cloaked Units - Decloak
		// Cabalist uses silent strike, so the check has to be in weapon damage
		// It is a melee unit, so this check is just a failsafe
		if (unit->id != UnitId::ProtossCabalist)
		{
			scbw::decloakProc(unit);
		}

		if (weapons_dat::FlingyId[weaponId] != 0)
		{
			// Lurker Seismic Spines behavior
			if (weaponId == WeaponId::SubterraneanSpines)
			{
				unitExt->timers[TimerLurkerETA] = 17;
				auto range = weapons_dat::MaxRange[weaponId] + 16;
				s32 angle = scbw::getAngle(unit->getX(), unit->getY(), unit->orderTarget.unit->getX(), unit->orderTarget.unit->getY());
				s32 targ_x = unit->getX() + scbw::getPolarX(range, angle);
				s32 targ_y = unit->getY() + scbw::getPolarY(range, angle);
				unitExt->values[ValueSpineX] = targ_x;
				unitExt->values[ValueSpineY] = targ_y;
				if (unit->orderTarget.unit != nullptr)
				{
					unitExt->values[ValueLurkerPickTarget] = (u32)unit->orderTarget.unit;
				}
				globals.clear_lakizilisk(unit);
			}

			if (unit->id != UnitId::TerranWyvern // disable default weapon for custom behaviors
				&& unit->id != UnitId::ProtossAnthelion)
			{
				// Madrigal Tempo Change weapon switch
				if (weaponId == WeaponId::MadrigalFuriaMissiles
					&& scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange) == 1)
				{
					weaponId = WeaponId::MadrigalImperiosoMissiles;
				}

				// Nemean Reactors - Heracles behavior - Refresh timer on attack and add stack
				if (weaponId == WeaponId::HeraclesJavelinCannons)
				{
					unitExt->values[ValueNemeanReactorsStack]++;
					unitExt->timers[TimerNemeanReactors] = 49;
					helper::playSoundFromSource(1372, unit, 1, 0);
				}

				// Madcap - Maverick Feeders passive
				if (unit->id == UnitId::TerranMadcap)
				{
					s32 stacks = unitExt->values[ValueMadcapStacks];
					unitExt->values[ValueMadcapStacks] = MathUtils::clamp(stacks + MadcapPassiveDecayTime, 0, MadcapMaxStacks * MadcapPassiveDecayTime);
				}

				// Track launch position
				unitExt->lastBulletLaunchPos = unit->position;

				switch (weaponId)
				{
					// Missile spawn positions start
					case WeaponId::AxitriliskSearingVenom:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueAxitriliskMissilePos, 1, ImagesDatEntries::OverlaySpecialPtr);
					} break;
					case WeaponId::CharlatanFerventBlaster:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueCharlatanMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::ExemplarPlasmaTorpedo:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueExemplarMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::EmpyreanPlasmaBarrage:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueEmpyreanMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::StarSovereignCelestialCannon:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueSovereignCannonPos, 2, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::ClaymoreHarpoonRailgun:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueClaymoreMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::PenumbraErebolShockCannon:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValuePenumbraLaserPos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::BurstLasers:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueWraithLaserPos, 3, ImagesDatEntries::OverlayAttackPtr, 0, 2);
					} break;
					case WeaponId::CentaurSunchaserMissiles:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueCentaurMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::PhobosArgosLaserBatteries:
					{
						// Phobos - Argent Capacitors passive
						if (unit->id == UnitId::TerranPhobos && unitExt->values[ValuePhobosDischarge] == 0)
						{
							unit->addUnitEnergy(5 * 256);
							if (unit->energy == unit->getMaxEnergy())
							{
								unitExt->values[ValuePhobosDischarge] = 1;
								unit->groundWeaponCooldown = getModifiedWeaponCooldownHook(unit, WeaponId::PhobosArgosLaserBatteries);
								unit->airWeaponCooldown = getModifiedWeaponCooldownHook(unit, WeaponId::PhobosArgosLaserBatteries);
								helper::playSoundFromSource(SoundId::PhobosArgentCapacitorsProc, unit, 1, 0);
								unit->sprite->createOverlay(ImageId::OpticalFlareHit_Large);
							}
						}
						scbw::setMissileSourcePos(unit, weaponId, ValuePhobosLaserPos, 4, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::SkithrokorSkitteringSpores:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueSkithrokorMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::LiiraliskHollowedSpores:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueLiiraliskMissilePos, 2, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::BactaliskCorrosiveAcid:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueBactaliskMissilePos, 2, ImagesDatEntries::OverlaySpecialPtr);
					} break;
					case WeaponId::KalkaliskViscousAcid:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueKalkaliskMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::KeskathalorScreamingVenom:
					{
						scbw::setMissileSourcePos(unit, weaponId, ValueKeskathalorMissilePos, 2, ImagesDatEntries::OverlayAttackPtr, 0, 1);
					} break;
					// Missile spawn positions end
					case WeaponId::SalamanderPerditionBarrage:
					{
						// Salamander missile target positions
						auto rotate = unitExt->values[ValueSalamanderAtkPos];
						if (rotate == 1)
						{
							helper::createBullet(weaponId, unit, x, y, unit->player, unit->currentDirection1);
						}
						else
						{
							scbw::rangeModifiedFire(weaponId, unit, x, y, 32 - 32 * rotate);
						}
						rotate++;
						if (rotate > 2)
						{
							rotate = 0;
						}
						unitExt->values[ValueSalamanderAtkPos] = rotate;
					} break;
					case WeaponId::MinotaurAsterionLaserBattery:
					{
						// Monoceros Targeting - Minotaur
						fireMultiple(unit, unitExt, weaponId, 5, Flags::Enemy | Flags::FrontalArc);
					} break;
					case WeaponId::SirenCharybdalRifle:
					{
						u32 count = 0;

						s32 radius = 3 * 32;
						scbw::UnitFinder(
							unit->position.x - radius,
							unit->position.y - radius,
							unit->position.x + radius,
							unit->position.y + radius
						).forEach([&](Unit* pickedUnit) {
							if (!(pickedUnit->status & UnitStatus::Completed)
								|| unit == pickedUnit
								|| scbw::isAlliedTo(pickedUnit->player, unit->player)
								|| scbw::getDistanceFast(unit->position, pickedUnit->position) > radius)
							{
								return;
							}

							count += 1;
						});
						if (count) {
							helper::playSoundFromSource(SoundId::SirenTertiaryEyesCast01, unit, 1, 0);
						}

						// Tertiary Eyes - Siren
						fireMultiple(unit, unitExt, weaponId, 1 + count, Flags::Enemy | Flags::FrontalArc);
					} break;
					case WeaponId::MadrigalFuriaMissiles:
					{
						// Madrigal Furia Salvo
						if (unit->orderTarget.unit != nullptr)
						{
							fireMultiple(unit, unitExt, WeaponId::MadrigalFuriaMissiles, 0, Flags::Enemy);
						}
					} break;
					case WeaponId::IzirokorEnergizedBioplasma:
					{
						// Crushing Tide - Izirokor
						fireMultiple(unit, unitExt, weaponId, 3, Flags::Enemy);
					} break;
					default:
					{
						helper::createBullet(weaponId, unit, x, y, unit->player, unit->currentDirection1);
					} break;
				}
			}

			// Chasmal Colony - Adaptive Amitosis ability
			if (unit->orderTarget.unit != nullptr
				&& unitExt->values[ValueAdaptiveAmitosis])
			{
				auto range = weapons_dat::MaxRange[weaponId];
				auto a = scbw::UnitFinder::getNearestTarget(
					unit->getX() - range,
					unit->getY() - range,
					unit->getX() + range,
					unit->getY() + range,
					unit, [&](Unit* target)
				{
					if (target == unit)
					{
						return false;
					}
					if (target == unit->orderTarget.unit)
					{
						return false;
					}
					if (scbw::isAlliedTo(unit->player, target->player))
						return false;
					return scbw::canWeaponTargetUnit(weaponId, target, unit);
				});
				if (a != nullptr)
				{
					auto old = unit->orderTarget;
					unit->orderTarget.unit = a;
					unit->orderTarget.pt = a->sprite->position;
					unitExt->values[ValueAdaptiveAmitosis] = 1;
					// TODO: verify that this will work with all weapon types
					const u32 Iscript_UseWeapon = 0x00479C30;
					__asm
					{
						PUSHAD
						MOV ESI, unit
						PUSH weaponId
						CALL Iscript_UseWeapon
						POPAD
					}
					unit->orderTarget = old;
				}
			}
		}

		// Deconstruction - Pazuzu passive
		if (unitExt->values[ValueDeconstruction])
		{
			scbw::doDeconstructionDamage(unit, weaponId);
		}
	}

	void progressBulletFlight(Bullet* bullet)
	{
		Bullet* v1;
		Unit* target;
		Sprite* bulletSprite;
		Image* i;

		if (bullet->weaponId == WeaponId::AlkajeliskParasiticSpores)
		{
			if (bullet->attackTarget.unit == nullptr)
			{
				u32 range = AlkajeliskParasiticSporesSeekRange; // seek range
				auto seekTarget = scbw::UnitFinder::getNearestTarget(
					bullet->position.x - range,
					bullet->position.y - range,
					bullet->position.x + range,
					bullet->position.y + range,
					bullet,
					[&](Unit* unit)
				{
					return alkajSporeMatch(bullet, unit);
				});
				if (seekTarget != nullptr)
				{
					bullet->attackTarget.unit = seekTarget;
					bullet->attackTarget.pt = seekTarget->position;
					bullet->moveTarget.unit = seekTarget;
					bullet->moveTarget.pt = seekTarget->position;
					bullet->targetPosition = seekTarget->position;
					bullet->targetPosition2 = seekTarget->position;
					helper::progressBulletMovement_func(bullet);
				}
			}
			else
			{
				helper::progressBulletMovement_func(bullet);
			}
		}
		else
		{
			helper::progressBulletMovement_func(bullet);
		}

		u8 time = bullet->time_remaining--;
		if (bullet->weaponId == WeaponId::AlkajeliskParasiticSpores)
		{
			if (bullet->time_remaining < AlkajeliskParasiticSporesTimer && bullet->attackTarget.unit == nullptr)
			{
				time = 0;
			}
		}
		bool bEnd = false;

		// Accelerate missiles over time
		if (bullet->weaponId == WeaponId::EmpyreanPlasmaBarrage)
		{
			u8 time_current = 255 - time;
			if (time_current == 8)
			{
				bullet->flingyTopSpeed *= 2;
				bullet->flingyAcceleration *= 2;
			}
		}

		if (bullet->weaponId == WeaponId::ArchitectDispersiveSalvo
			&& bullet->sourceUnit
			&& bullet->attackTarget.unit
			&& bullet->attackTarget.unit->sprite && bullet->sourceUnit->sprite)
		{
			Unit* targ = bullet->attackTarget.unit;

			UnitExt* unitExt = globals.get_unit_ext(bullet->sourceUnit);
			const u32 distFromSource = scbw::getDistanceFast(bullet->position, unitExt->lastBulletLaunchPos);
			const u32 distFromTarget = scbw::getDistanceFast(bullet->position, targ->position);

			if (distFromSource > 5 * 32
				&& distFromTarget < 3 * 32)
			{
				if (!globals.multihit_bullet_is_set(bullet))
				{
					auto old = bullet->sourceUnit->orderTarget.unit;
					auto oldSpritePos = bullet->sourceUnit->sprite->position;
					auto oldPos = bullet->sourceUnit->position;
					bullet->sourceUnit->sprite->position = bullet->position;
					bullet->sourceUnit->position = bullet->position;
					auto r = 3 * 32;
					scbw::UnitFinder unitsInSplash(bullet->position.x - r, bullet->position.y - r,
						bullet->position.x + r, bullet->position.y + r);
					globals.add_multihit_empty_bullet(bullet, 1);
					for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
					{
						Unit* target = unitsInSplash.getUnit(i);
						if (target == nullptr)
						{
							continue;
						}
						if (target == targ)
						{
							continue;
						}
						if (!alkajSporeMatch(bullet, target))
						{
							continue;
						}
						bullet->sourceUnit->orderTarget.unit = target;
						auto newAngle = scbw::getAngle(bullet->position.x, bullet->position.y, target->getX(), target->getY());
						auto newBullet = helper::createBullet(WeaponId::ArchitectDispersiveSalvo,
							bullet->sourceUnit, bullet->position.x, bullet->position.y, bullet->srcPlayer, newAngle);
						if (newBullet)
						{
							helper::playSoundFromSource(SoundId::ArchitectWeaponSplit01, (Unit*)bullet, 1, 0);
							newBullet->position = bullet->position;
							globals.add_multihit_empty_bullet(newBullet, 1);
						}
					};
					bullet->sourceUnit->orderTarget.unit = old;
					bullet->sourceUnit->position = oldPos;
					bullet->sourceUnit->sprite->position = oldSpritePos;
				}
			}
		}

		if (time == 0 || (bullet->moveTarget.pt.x == bullet->position.x && bullet->moveTarget.pt.y == bullet->position.y))
		{
			bEnd = true;
		}

		// Missile interception
		// Empyrean Heavenly Passage
		// Valkyrie Charon Afterburners
		else if (bullet->weaponId == WeaponId::EmpyreanPlasmaBarrage
			|| bullet->weaponId == WeaponId::HaloRockets)
		{
			u32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](Unit* unit)
			{
				if (!(unit->status & UnitStatus::InAir))
				{
					return false;
				}
				if (unit->status & UnitStatus::Invincible)
				{
					return false;
				}
				if (scbw::isAlliedTo(bullet->srcPlayer, unit->player))
				{
					return false;
				}
				auto box = units_dat::UnitBounds[unit->id];
				if (bullet->position.x < unit->getLeft())
				{
					return false;
				}
				if (bullet->position.x > unit->getRight())
				{
					return false;
				}
				if (bullet->position.y < unit->getTop())
				{
					return false;
				}
				if (bullet->position.y > unit->getBottom())
				{
					return false;
				}
				return true;
			}
			);
			if (collisionTarget != nullptr)
			{
				bullet->attackTarget.unit = collisionTarget;
				bEnd = true;
			}
		}

		// Star Sovereign Intervention
		else if (bullet->weaponId == WeaponId::StarSovereignInterventionMissiles)
		{
			u32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](Unit* unit)
			{
				if (unit->status & UnitStatus::Invincible)
				{
					return false;
				}
				if (scbw::isAlliedTo(bullet->srcPlayer, unit->player))
				{
					return false;
				}
				auto box = units_dat::UnitBounds[unit->id];
				if (bullet->position.x < unit->getLeft())
				{
					return false;
				}
				if (bullet->position.x > unit->getRight())
				{
					return false;
				}
				if (bullet->position.y < unit->getTop())
				{
					return false;
				}
				if (bullet->position.y > unit->getBottom())
				{
					return false;
				}
				return true;
			}
			);
			if (collisionTarget != nullptr)
			{
				bullet->attackTarget.unit = collisionTarget;
				bEnd = true;
			}
		}

		// for missiles hitting targets mid-flight
		else if (scbw::weaponPiercesTargets(bullet->weaponId))
		{
			u32 range = 0;
			switch (bullet->weaponId)
			{
				case WeaponId::KalkaliskViscousAcid:
				{
					range = 96;
				} break;
				case WeaponId::PenumbraErebolShockCannon:
				{
					range = 128;
				} break;
				case WeaponId::ClaymoreHarpoonRailgun:
				{
					range = 160;
				} break;
				default:
				{
				}
			}
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range,
				bullet->position.y - range,
				bullet->position.x + range,
				bullet->position.y + range,
				bullet, [&](Unit* unit)
			{
				if (unit->status & UnitStatus::Invincible)
				{
					return false;
				}
				if (bullet->attackTarget.unit == unit)
				{
					return false;
				}
				if (scbw::isAlliedTo(bullet->srcPlayer, unit->player))
				{
					return false;
				}
				auto box = units_dat::UnitBounds[unit->id];
				if (bullet->position.x < unit->getLeft())
				{
					return false;
				}
				if (bullet->position.x > unit->getRight())
				{
					return false;
				}
				if (bullet->position.y < unit->getTop())
				{
					return false;
				}
				if (bullet->position.y > unit->getBottom())
				{
					return false;
				}
				if (globals.was_already_hit_by_multihit_bullet(bullet, unit))
				{
					return false;
				}
				return true;
			}
			);
			if (collisionTarget != nullptr)
			{
				if (bullet->weaponId == WeaponId::PenumbraErebolShockCannon)
				{
					// Penumbra - Shroud Piercer - pierce targets
					globals.add_bullet_multihit(bullet, collisionTarget);
					scbw::createAndInitThingy(SpriteId::BurstLasersImpactDamageless, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					hooks::WeaponBulletHit(bullet, collisionTarget, 2);
				}
				else if (bullet->weaponId == WeaponId::ClaymoreHarpoonRailgun)
				{
					// Claymore - Kelvin Munitions - pierce targets
					globals.add_bullet_multihit(bullet, collisionTarget);
					scbw::createAndInitThingy(SpriteId::BurstLasersImpactBlue_Damageless, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					hooks::WeaponBulletHit(bullet, collisionTarget, 1);

					// Claymore - Kelvin Munitions - slow pierced targets
					UnitExt* collisionTargetExt = globals.get_unit_ext(collisionTarget);
					if (!collisionTargetExt->values[ValueKelvinMunitionsSlow])
					{
						collisionTargetExt->values[ValueKelvinMunitionsSlow] = 1;
					}
				}
				else if (bullet->weaponId == WeaponId::KalkaliskViscousAcid) {
					// Kalkalisk behavior
					globals.add_incremental_bullet_multihit(bullet, collisionTarget);
					auto count = globals.get_multihit_count(bullet);
					helper::createThingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					collisionTarget->damageWithCustom((weapons_dat::DamageAmount[WeaponId::KalkaliskViscousAcid] - count + 1) * 256,
						weapons_dat::DamageType[WeaponId::KalkaliskViscousAcid],
						bullet->sourceUnit, bullet->srcPlayer, 0, 1, false);

					if (count >= 4)
					{
						bullet->time_remaining = 0;
					}
					else
					{
						bullet->moveTarget.unit = bullet->attackTarget.unit;
						if (bullet->moveTarget.unit != nullptr)
						{
							bullet->targetPosition = bullet->moveTarget.unit->position;
							bullet->targetPosition2 = bullet->moveTarget.unit->position;
						}
					}
				}
			}
		}

		if (bEnd)
		{
			target = bullet->attackTarget.unit;
			bullet->behaviourTypeInternal = 5;
			if (target != nullptr && target->sprite != nullptr)
			{
				bullet->attackTarget.pt.x = target->sprite->position.x;
				bullet->attackTarget.pt.y = target->sprite->position.y;
			}
			else
			{
				bullet->attackTarget.unit = 0;
			}
			bulletSprite = bullet->sprite;
			bullet->orderSignal |= 0x80; // new
			bullet->unknown_0x4E = 0;
			bullet->someUnitType = 228;
			for (i = bulletSprite->images.head; i; i = i->link.next)
			{
				helper::playImageIscript(i, IscriptAnimation::Death);
			}
		}
	}

	void bulletBehaviour_Bounce(Bullet* bullet) // 0048B2D0
	{
		Unit* v1;
		char nextBounce;
		Unit* target;
		Image* i;
		Unit* target_unit;
		Sprite* sprite;
		Image* j;

		v1 = bullet->attackTarget.unit;
		if (v1)
		{
			if (!(bullet->hitFlags & 1))
			{
				helper::changeMovePos(bullet, v1->sprite->position.y, v1->sprite->position.x);
			}
		}
		helper::progressBulletMovement_func(bullet);
		if (bullet->moveTarget.pt.x == bullet->position.x && bullet->moveTarget.pt.y == bullet->position.y)
		{
			nextBounce = bullet->remainingBounces - 1;
			// Gladius infinite bouncing
			if (bullet->weaponId == WeaponId::GladiusPlasmaFlurry)
			{
				nextBounce = 1;
			}

			bullet->remainingBounces = nextBounce;
			globals.bounce_bullet = bullet;
			if (nextBounce && (target = helper::findNextBounceTarget_func(bullet), bullet->nextBounceUnit = bullet->attackTarget.unit, target))
			{
				for (i = bullet->sprite->images.head; i; i = i->link.next)
				{
					helper::playImageIscript(i, IscriptAnimation::SpecialState1);
				}
				bullet->attackTarget.unit = target;
			}
			else
			{
				if (bullet->weaponId == WeaponId::GladiusPlasmaFlurry)
				{
					globals.remove_multihit_bullet(bullet);
				}
				target_unit = bullet->attackTarget.unit;
				bullet->behaviourTypeInternal = 5;
				if (target_unit)
				{
					bullet->attackTarget.pt.x = target_unit->sprite->position.x;
					bullet->attackTarget.pt.y = target_unit->sprite->position.y;
				}
				else
				{
					bullet->attackTarget.unit = 0;
				}
				sprite = bullet->sprite;
				bullet->unknown_0x4E = 0;
				bullet->someUnitType = UnitId::None;

				for (j = sprite->images.head; j; j = j->link.next)
				{
					helper::playImageIscript(j, IscriptAnimation::Death);
				}
			}
		}
	}

	bool findNextBounceTargetProc(Unit* pIterator) // 0048AFD0
	{
		Unit* pSrc = *bulletBounceMissileSource;
		s32 nSrcStatus = (*bulletBounceMissileSource)->status;
		Unit* _pIterator = pIterator;

		// Previously disallowed bouncing if unit was disabled
		if (!pIterator)
		{
			return 0;
		}
		if (pIterator->status & UnitStatus::Invincible)
		{
			return 0;
		}
		if (pIterator->sprite->flags & SpriteFlags::Hidden)
		{
			return 0;
		}
		if (helper::unitCanSeeCloakedTarget(pIterator, (*bulletBounceMissileSource)))
		{
			return 0;
		}

		Unit* bounceSource = pSrc;
		if (pSrc->subunit != nullptr && helper::unitIsSubunit(pSrc->subunit))
		{
			bounceSource = pSrc->subunit;
		}
		if (!(helper::unitIsCarrier(pSrc)))
		{
			bool miscBool;
			if (helper::unitIsReaver(pSrc))
			{
				if (_pIterator->status & UnitStatus::InAir)
				{
					return 0;
				}
				miscBool = helper::unitHasPathToUnit(pSrc, _pIterator);
			}
			else
			{
				if (helper::unitCanInfest(pSrc))
				{
					Unit* unusedUnit = nullptr;
					miscBool = helper::canBeInfested(unusedUnit, _pIterator) != 0; // unusedUnit is not used..?
				}
				else
				{
					if (_pIterator->status & UnitStatus::InAir)
					{
						miscBool = units_dat::AirWeapon[(u16)bounceSource->id] != -126;
					}
					else
					{
						miscBool = units_dat::GroundWeapon[(u16)bounceSource->id] != -126;
					}
				}
			}
			if (!miscBool)
			{
				return 0;
			}
		}

		s32 unitStatus = _pIterator->status;
		if (unitStatus & 0x300 && !((1 << pSrc->player) & _pIterator->visibilityStatus))
		{
			return 0;
		}

		s8 playerId = _pIterator->player == 11 ? _pIterator->sprite->playerId : _pIterator->player;
		if (playerAlliance[(u8)pSrc->player].flags[playerId] || *bulletBounceMissileTarget == _pIterator)
		{
			return 0;
		}

		// Gladius Binding Plasma behavior
		if (globals.bounce_bullet->weaponId == WeaponId::GladiusPlasmaFlurry)
		{
			if (pIterator != nullptr && *bulletBounceMissileNext != nullptr
				&& (*bulletBounceMissileNext)->sprite != nullptr
				&& _pIterator->sprite != nullptr)
			{
				if (globals.was_already_hit_by_multihit_bullet(globals.bounce_bullet, _pIterator))
				{
					return false;
				}
				else
				{
					globals.add_bullet_multihit(globals.bounce_bullet, _pIterator);
					return *bulletBounceMissileNext != _pIterator;

				}
			}
			else
			{
//				scbw::print("bad bullet");
			}
		}
		return *bulletBounceMissileNext != _pIterator;
	}

	Unit* findNextBounceTarget(Bullet* bullet) // 0048B1E0
	{
		if (!bullet->sourceUnit)
		{
			return 0;
		}
		Unit* nextBounce = bullet->nextBounceUnit;
		*bulletBounceMissileSource = bullet->sourceUnit;
		Unit* target = bullet->attackTarget.unit;
		Sprite* bulletSprite = bullet->sprite;
		*bulletBounceMissileTarget = target;
		u16 posY = bulletSprite->position.x;
		u16 posX = bulletSprite->position.y;
		*bulletBounceMissileNext = nextBounce;
		Box16 coords;
		coords.left = posX - 96;
		coords.top = posX + 96;
		coords.right = posY - 96;
		coords.bottom = posY + 96;
//		return (Unit*)helper::iterateUnitsAtLocationTargetProc(&coords, findNextBounceTargetProc, 0);
	}

	// Hooked to resolve issues with multihit bullets - Pr0nogo
	Bullet* deleteBullet (Bullet* bullet) // 0048A560
	{
		if (!bullet->sprite)
		{
			if (*firstBullet == bullet)
			{
				*firstBullet = bullet->next;
			}
			if (*lastBullet == bullet)
			{
				*lastBullet = bullet->previous;
			}
			if (bullet->previous)
			{
				bullet->previous->next = bullet->next;
			}
			Bullet* v1 = bullet->next;
			if (v1)
			{
				v1->previous = bullet->previous;
			}
			bullet->previous = 0;
			bullet->next = 0;
			Bullet* nextBounce = *firstFreeBullet;
			if (*firstFreeBullet)
			{
				if (*lastFreeBullet == *firstFreeBullet)
				{
					*lastFreeBullet = bullet;
				}
				bullet->previous = *firstFreeBullet;
				bullet->next = nextBounce->next;
				Bullet* target = nextBounce->next;
				if (target)
				{
					target->previous = bullet;
				}
				nextBounce->next = bullet;
				*bulletCount -= 1;
			}
			else
			{
				*lastFreeBullet = bullet;
				*firstFreeBullet = bullet;
				*bulletCount -= 1;
			}
			globals.remove_multihit_bullet(bullet);
		}
		return bullet;
	}

	// Hooked to stop Valkyries from being cucked by lardcode - Pr0nogo
	Bullet* createBullet(s32 weaponId, Unit* parent, s32 x, s16 y, char playerId, s32 angle) // 0048C260
	{
		Bullet* pBullet;
		s32 parentStatus;
		Bullet* result;
		Bullet* pNextBullet;
		Bullet* firstBulletVar;
		Bullet* nextBulletVar;

		pBullet = *firstFreeBullet;
		if (!*firstFreeBullet)
		{
			return 0;
		}
		parentStatus = parent->status;
		if (parentStatus & UnitStatus::CanNotAttack)
		{
			return 0;
		}
		if (!*firstFreeBullet)
		{
			return 0;
		}
		if (!hooks::initializeBullet(playerId, parent, y, *firstFreeBullet, weaponId, x, angle))
		{
			return 0;
		}
		if (*firstFreeBullet == pBullet)
		{
			*firstFreeBullet = pBullet->next;
		}
		if (*lastFreeBullet == pBullet)
		{
			*lastFreeBullet = pBullet->previous;
		}
		if (pBullet->previous)
		{
			pBullet->previous->next = pBullet->next;
		}
		pNextBullet = pBullet->next;
		if (pNextBullet)
		{
			pNextBullet->previous = pBullet->previous;
		}
		pBullet->previous = 0;
		pBullet->next = 0;
		firstBulletVar = *firstBullet;
		if (*firstBullet)
		{
			if (*lastBullet == *firstBullet)
			{
				*lastBullet = pBullet;
			}
			pBullet->previous = *firstBullet;
			pBullet->next = firstBulletVar->next;
			nextBulletVar = firstBulletVar->next;
			if (nextBulletVar)
			{
				nextBulletVar->previous = pBullet;
			}
			firstBulletVar->next = pBullet;
			++*bulletCount;
			result = pBullet;
		}
		else
		{
			++*bulletCount;
			*lastBullet = pBullet;
			*firstBullet = pBullet;
			result = pBullet;
		}
		return result;
	}

	// Hooked so we don't need aise for Bactalisks - Pr0nogo
	s32 initializeBullet(char playerId, Unit* parent, s16 y, Bullet* bullet, s32 weaponId, s32 x, s32 angle) // 0048BEC0
	{
		if (!helper::initializeFlingy(weapons_dat::FlingyId[weaponId], y, x, (Flingy*)bullet, playerId, angle)) // Unit/Bullet/Flingy have a shared header, so this is presumably fine
		{
			return 0;
		}

		bullet->srcPlayer = playerId;
		bullet->someUnitType = 228;
		bullet->unknown_0x4E = 0;
		bullet->attackTarget.unit = nullptr;
		bullet->attackTarget.pt.y = 0;
		bullet->attackTarget.pt.x = 0;
		bullet->unknown_0x54 = 0;
		bullet->weaponId = weaponId;
		bullet->time_remaining = weapons_dat::RemoveAfter[weaponId];
		bullet->someUnitType = 228;
		bullet->remainingBounces = 0;
		bullet->hitFlags = 0;
		bullet->movementFlags |= MovementFlags::StartingAttack;
		bullet->behaviourTypeInternal = 0;
		bullet->attackTarget.unit = nullptr;
		bullet->attackTarget.pt.y = 0;
		bullet->attackTarget.pt.x = 0;
		bullet->unknown_0x4E = 0;
		hooks::bulletBehaviorReassign(bullet);

		u8 launchSpin = weapons_dat::LaunchSpin[weaponId];
		if (launchSpin)
		{
			s32 launchSpinVariance = 0;
			if (parent == *lastBulletCreator)
			{
				launchSpinVariance = *launchSpinRandom == 0;
			}
			else
			{
				launchSpinVariance = helper::RandomizeShort(0) & 1;
			}

			*launchSpinRandom = launchSpinVariance;
			bool invertSpin = launchSpinVariance == 0;
			launchSpinVariance = launchSpin;
			if (invertSpin)
			{
				launchSpinVariance = -(s8)launchSpin;
			}
			char launchSpinDirection = launchSpinVariance + bullet->direction2;
			bullet->direction2 = launchSpinDirection;
			bullet->direction1 = launchSpinDirection;
			*lastBulletCreator = parent;
		}

		bullet->sourceUnit = parent;
		if (units_dat::BaseProperty[parent->id] & UnitProperty::Subunit)
		{
			bullet->sourceUnit = parent->subunit;
		}
		if (parent->id == UnitId::ProtossPlasmaShell)
		{
			bullet->sourceUnit = parent->interceptor.parent;
		}
		if (parent->status & UnitStatus::IsHallucination)
		{
			bullet->hitFlags |= 2u;
		}
		bullet->nextBounceUnit = nullptr;

		Unit* targetUnit = parent->orderTarget.unit;
		if (targetUnit)
		{
			bullet->sprite->elevationLevel = targetUnit->sprite->elevationLevel + 1;
			bullet->attackTarget.unit = targetUnit;
			bullet->attackTarget.pt.x = targetUnit->sprite->position.x;
			bullet->attackTarget.pt.y = targetUnit->sprite->position.y;
		}
		else
		{
			bullet->sprite->elevationLevel = parent->sprite->elevationLevel + 1;
			bullet->attackTarget.unit = 0;
			bullet->attackTarget.pt.x = parent->orderTarget.pt.x;
			bullet->attackTarget.pt.y = parent->orderTarget.pt.y;
		}

		switch (weapons_dat::Behavior[weaponId])
		{
			case WeaponBehavior::Fly_DoNotFollowTarget:
			case WeaponBehavior::Fly_FollowTarget:
			{
				if (targetUnit != nullptr)
				{
					if (parent != nullptr)
					{
						helper::weaponBehaviorCanHitTargetCheck(targetUnit, parent);
					}
				}
				helper::attackFlyToTarget(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);
				return 1;
			}
			case WeaponBehavior::AppearOnTargetUnit:
			case WeaponBehavior::AppearOnTargetSite:
			{
				if (!targetUnit)
				{
					return 1;
				}
				if (!bullet->sourceUnit)
				{
					return 1;
				}
				helper::weaponBehaviorCanHitTargetCheck(targetUnit, bullet->sourceUnit);
				return 1;
			}
			case WeaponBehavior::PersistOnTargetSite:
			{
				helper::appearOnTarget(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);
				return 1;
			}
			case WeaponBehavior::AppearOnAttacker:
			{
				return 1;
			}
			case WeaponBehavior::AttackAndSelfDestruct:
			{
				if (!bullet->sourceUnit)
				{
					return 1;
				}
				bullet->sourceUnit->status |= UnitStatus::IsSelfDestructing;
				bullet->sourceUnit->userActionFlags |= 0x4;
				helper::orders_SelfDestructing(bullet->sourceUnit);
				return 1;
			}
			case WeaponBehavior::Bounce:
			{
				switch (weaponId)
				{
					case WeaponId::BactaliskCorrosiveAcid:
					{
						const u32 dist = scbw::getDistanceFast(bullet->sourceUnit->position, bullet->attackTarget.pt);
						if (dist >= 6 * 32)
						{
							bullet->remainingBounces = 2;
						}
						else
						{
							bullet->remainingBounces = 1;
						}
					} break;
					case WeaponId::IzirokorEnergizedBioplasma:
					{
						bullet->remainingBounces = 2;
					} break;
					default:
					{
						bullet->remainingBounces = 3;
					}
				}
				if (targetUnit)
				{
					if (bullet->sourceUnit)
					{
						helper::weaponBehaviorCanHitTargetCheck(targetUnit, bullet->sourceUnit);
					}
				}
				helper::attackFlyToTarget(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);
				return 1;
			}
			case WeaponBehavior::AttackNearbyArea:
			{
				bullet->cyclicMissileIndex = parent->offsetIndex3by3;
				bullet->attackTarget.pt.x -= (s16)attackCurve[parent->offsetIndex3by3].x;
				bullet->attackTarget.pt.y -= (s16)attackCurve[parent->offsetIndex3by3].y;
				helper::attackFlyToTarget(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);

				s16 finalOffset = parent->offsetIndex3by3 + 1;
				if (!attackCurve[finalOffset].x)
				{
					finalOffset = 0;
				}
				parent->offsetIndex3by3 = finalOffset;
				return 1;
			}
			case WeaponBehavior::GoToMaxRange:
			{
				s32 bulletDirection = bullet->direction1;
				s32 maxRangeVar = (weapons_dat::MaxRange[bullet->weaponId] + 20) << 8;
				bullet->attackTarget.pt.x = parent->sprite->position.x + ((u32)(maxRangeVar * angleDistance[bulletDirection].x) >> 16);
				bullet->attackTarget.pt.y = parent->sprite->position.y + ((u32)(maxRangeVar * angleDistance[bulletDirection].y) >> 16);
				helper::attackFlyToTarget(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);
				return 1;
			}
		}
		return 0;
	}

	// Hooked to investigate weaponsdat extender crashes - Pr0nogo
	void progressBulletFrames() // 0048C3D0
	{
		Bullet* bulletVar = *firstBullet;
		Sprite* bulletSprite = nullptr;
		s32 bulletBehavior = 0;
		Bullet* nextBullet = *firstBullet;
		while (nextBullet)
		{
			nextBullet = bulletVar->next;
			*activeIscriptEntity = (Unit*)bulletVar;
			*activeIscriptBullet = bulletVar;
			helper::setThingyVisibilityFlags((Thingy*)bulletVar);
			bulletSprite = bulletVar->sprite;
			if (bulletSprite)
			{
				helper::sprite_to_iscript_loop(bulletSprite);
				if (!bulletVar->sprite->images.head)
				{
					bulletVar->sprite = 0;
				}
			}
			bulletBehavior = (u8)bulletVar->behaviourTypeInternal;
			switch (bulletBehavior)
			{
				case 0:
				{
					hooks::bulletBehaviorReassign(bulletVar);
				} break;
				case 1:
				{
					helper::bulletBehaviorFly(bulletVar);
				} break;
				case 2:
				{
					helper::bulletBehaviorFollow(bulletVar);
				} break;
				case 3:
				{
					helper::bulletBehaviorBounce(bulletVar);
				} break;
				case 4:
				{
					helper::bulletBehaviorPersist(bulletVar);
				} break;
				case 5:
				{
					helper::bulletBehaviorInstant(bulletVar);
				} break;
				case 6:
				{
					helper::bulletBehaviorArea(bulletVar);
				} break;
				default:
				{
				} break;
			}
			bulletVar = nextBullet;
		}
		*activeIscriptEntity = 0;
		*activeIscriptBullet = 0;
	}

	// Hooked to investigate firstFreeBullet nonsense - Pr0nogo
	void bulletBehaviorReassign(Bullet* bullet) // 0048BD50
	{
		while (2) // xd
		{
			if (bullet->unknown_0x4E & 1)
			{
				bullet->unknown_0x4E = bullet->unknown_0x4E & 0xFE;
				Image* bulletOverlay = nullptr;
				switch (weapons_dat::Behavior[bullet->weaponId])
				{
					case WeaponBehavior::AppearOnTargetUnit:
					case WeaponBehavior::AppearOnTargetSite:
					case WeaponBehavior::AppearOnAttacker:
					case WeaponBehavior::AttackAndSelfDestruct:
					{
						bullet->behaviourTypeInternal = WeaponBehavior::AppearOnAttacker;
						if (bullet->attackTarget.unit)
						{
							bullet->attackTarget.pt.x = bullet->attackTarget.unit->sprite->position.x;
							bullet->attackTarget.pt.y = bullet->attackTarget.unit->sprite->position.y;
						}
						else
						{
							bullet->attackTarget.unit = nullptr;
						}
						bullet->unknown_0x4E = 0;
						bullet->someUnitType = 228;
						helper::playSpriteIscript(bullet->sprite, 1, 1);
					} break;
					case WeaponBehavior::Fly_FollowTarget:
					{
						helper::assignWeaponData(bullet, 2);
						helper::playSpriteIscript(bullet->sprite, 2, 1);
					}  break;
					case WeaponBehavior::Bounce:
					{
						helper::assignWeaponData(bullet, 3);
						helper::playSpriteIscript(bullet->sprite, 2, 1);
					}  break;
					case WeaponBehavior::PersistOnTargetSite:
					{
						helper::assignWeaponData(bullet, 4);
						Image* bulletOverlay = nullptr;
						bulletOverlay = bullet->sprite->images.head;
						if (bulletOverlay)
						{
							do
							{
								helper::playImageIscript(bulletOverlay, 14);
								bulletOverlay = bulletOverlay->link.next;
							}
							while (bulletOverlay);
						}
					} break;
					case WeaponBehavior::AttackNearbyArea:
					{
						helper::assignWeaponData(bullet, 6);
						Image* bulletOverlay = bullet->sprite->images.head;
						if (bulletOverlay)
						{
							do
							{
								helper::playImageIscript(bulletOverlay, 2);
								bulletOverlay = bulletOverlay->link.next;
							}
							while (bulletOverlay);
						}
					} break;
					default:
					{
						helper::assignWeaponData(bullet, 1);
						Image* bulletOverlay = bullet->sprite->images.head;
						if (bulletOverlay)
						{
							do
							{
								helper::playImageIscript(bulletOverlay, 2);
								bulletOverlay = bulletOverlay->link.next;
							}
							while (bulletOverlay);
						}
					} break;
				}

				switch (bullet->behaviourTypeInternal)
				{
					case 0:
					{
						continue;
					}
					case 1:
					{
						helper::bulletBehaviorFly(bullet);
					} break;
					case 2:
					{
						helper::bulletBehaviorFollow(bullet);
					} break;
					case 3:
					{
						helper::bulletBehaviorBounce(bullet);
					} break;
					case 4:
					{
						helper::bulletBehaviorPersist(bullet);
					} break;
					case 5:
					{
						helper::bulletBehaviorInstant(bullet);
					} break;
					case 6:
					{
						helper::bulletBehaviorArea(bullet);
					} break;
					default:
					{
						return;
					}
				}
			}
			break;
		}
	}
}