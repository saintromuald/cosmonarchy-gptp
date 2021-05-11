//Source file for the Weapon Fire hook module.
//This hook controls how weapons are fired.
#include "weapon_fire.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "SCBW/UnitFinder.h"
#include "hooks/hydraFinder.h"
#include "MathUtils.h"
#include "hooks/byte_operations_utils.h"
#include "globals.h"
#include "hooks/unit_stats/weapon_range.h"
#include "weapon_damage.h"
#include "weapon_spellhit.h"
#include <set>

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
	if (scbw::isAlliedTo(madrigal->playerId, unit->playerId))
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
	if (scbw::isAlliedTo(bullet->srcPlayer, unit->playerId))
	{
		return false;
	}
	return true;
}

bool powerSiphonDistributeMatch(Unit* savant, Unit* unit)
{
	if (!savant)
	{
		return false;
	}
	if (unit == savant)
	{
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster))
	{
		return false;
	}
	if (!scbw::isAlliedTo(savant->playerId, unit->playerId))
	{
		return false;
	}
	if (unit->energy == unit->getMaxEnergy())
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
		helper::createBullet(weaponId, attacker, posX, posY, attacker->playerId, newAngle);
		if (attackerExt->values[ValueDeconstruction])
		{
			scbw::doDeconstructionDamage(attacker, weaponId, false);
		}
	}

	static void fireMultiple(Unit* attacker, UnitExt* attackerExt, u32 weaponId, u32 maxTargets, u32 flags)
	{
		std::unordered_set<Unit*> targets;
		if (attacker->orderTarget.unit)
		{
			targets.insert(attacker->orderTarget.unit);
			launchMissile(attacker, attackerExt, attacker->orderTarget.unit, weaponId);
		}
		Unit* old = attacker->orderTarget.unit;
		for (s32 i = 0; i < maxTargets || maxTargets <= 0; i++)
		{
			u32 range = hooks::getMaxWeaponRangeHook(attacker, weaponId);
			auto filter = hydraFinderMatch(attacker, flags, {});
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
					|| scbw::isAlliedTo(attacker->playerId, target->playerId)
					|| !attacker->canAttackTarget(target)
					|| !filter(target))
				{
					return false;
				}
				return true;
			});
			if ((targets.size() >= maxTargets && maxTargets > 0) || !newTarget)
			{
				break;
			}
			launchMissile(attacker, attackerExt, newTarget, weaponId);
			targets.insert(newTarget);
			attacker->orderTarget.unit = newTarget;
		}
		attacker->orderTarget.unit = old;
	}

	// Fires a weapon from the @p unit.
	// This hook affects the following iscript opcodes: attackwith, attack, castspell
	// This also affects Unit::fireWeapon().
	void fireWeaponHook(Unit* unit, u8 wId)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);
		if (unit == nullptr)
		{
			return;
		}
		u8 weaponId = wId;
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
		// It is a melee unit, so this check is just a fail safe
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

			if (unit->id != UnitId::TerranWyvern)
			{
				// Madrigal Tempo Change weapon switch
				if (weaponId == WeaponId::MadrigalFuriaMissiles
					&& scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange) == 1)
				{
					weaponId = WeaponId::MadrigalImperiosoMissiles;
				}

				// Savant Power Siphon ability
				if (weaponId == WeaponId::SavantPowerSiphon)
				{
					u32 energy = 0;
					u32 range = weapons_dat::OuterSplashRadius[WeaponId::SavantPowerSiphon];
					scbw::UnitFinder unitFinder_collect(
						x - range,
						y - range,
						x + range,
						y + range);
					unitFinder_collect.forEach([&](Unit* target)
					{
						constexpr u32 ENERGY_PROC_MAX = 50 * 256;
						if (energy == ENERGY_PROC_MAX)
						{
							return;
						}
						if (target == unit)
						{
							return;
						}
						if (!(units_dat::BaseProperty[target->id] & UnitProperty::Spellcaster))
						{
							return;
						}
						if (target->energy == 0)
						{
							return;
						}
						u32 energyLeft = ENERGY_PROC_MAX - energy;
						u32 drain = MathUtils::clamp<u32>(target->energy, 0, energyLeft);
						energy += drain;
						target->spendUnitEnergy(drain);
						u8 overlaySize = scbw::hasOverlay(target);
						if (target->subunit != nullptr)
						{
							target->subunit->sprite->createTopOverlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);
						}
						else
						{
							target->sprite->createTopOverlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);
						}
					});
					range = 4 * 32;
					scbw::UnitFinder unitFinder_distribute(
						unit->getX() - range,
						unit->getY() - range,
						unit->getX() + range,
						unit->getY() + range
					);
					u32 matchCount = unitFinder_distribute.getMatchUnitCount([&](Unit* target)
					{
						return powerSiphonDistributeMatch(unit, target);
					});
					if (matchCount != 0 && energy > 0)
					{
						u32 perEach = energy / matchCount;
						u32 extra = energy % matchCount;
						Unit* unitFirst = unitFinder_distribute.getFirst([&](Unit* target)
						{
							return powerSiphonDistributeMatch(unit, target);
						});
						if (unitFirst != nullptr)
						{
							unitFirst->addUnitEnergy(extra);
						}
						u32 unspent = 0;
						unitFinder_distribute.forEachInMatch([&](Unit* target)
						{
							return powerSiphonDistributeMatch(unit, target);
						}, [&](Unit* u)
						{
							u->addUnitEnergy(unspent);
							energy -= unspent;
							unspent = 0;
							u32 howMuch = MathUtils::clamp<u32>(perEach, 0, u->getMaxEnergy() - u->energy);
							u->energy += howMuch;
							energy -= howMuch;
							u8 overlaySize = scbw::hasOverlay(u);
							if (u->subunit != nullptr)
							{
								u->subunit->sprite->createTopOverlay(overlaySize + ImageId::MindControlHit_Small, 0, 0, 0);
							}
							else
							{
								u->sprite->createTopOverlay(overlaySize + ImageId::MindControlHit_Small, 0, 0, 0);
							}
							unspent = perEach - howMuch;
						});
						if (energy > 0)
						{
							Unit* unitNew = unitFinder_distribute.getFirst([&](Unit* target)
							{
								return powerSiphonDistributeMatch(unit, target);
							});
							if (unitNew != nullptr)
							{
								unitNew->addUnitEnergy(extra);
							}
						}
					}
				}

				// Monoceros Targeting - Battlecruiser behavior
				else if (weaponId == WeaponId::MinotaurAsterionLaserBattery)
				{
					fireMultiple(unit, unitExt, weaponId, 5, Flags::Enemy | Flags::FrontalArc);
				}

				// Tertiary Eyes - Siren behavior
				else if (weaponId == WeaponId::SirenCharybdalRifle)
				{
					fireMultiple(unit, unitExt, weaponId, 1 + unitExt->unitVectors[UnitVectorTertiaryEyes].size(), Flags::Enemy);
				}

				// Titan Reactors - Heracles behavior - Refresh timer on attack and add stack
				if (weaponId == WeaponId::HeraclesJavelinCannons)
				{
					unitExt->timers[TimerNemeanReactors] = 24 * 5;
					s32 stacks = unitExt->values[ValueNemeanReactorsStack];
					if (stacks < 5)
					{
						unitExt->values[ValueNemeanReactorsStack] = stacks++;
						helper::playSoundFromSource(1372, unit, 1, 0);
					}
				}

				// Missile spawn positions start
				if (weaponId == WeaponId::ExemplarPlasmaTorpedo)
				{
					scbw::setMissileSourcePos(unit, weaponId, ValueExemplarMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::EmpyreanPlasmaBarrage)
				{
					scbw::setMissileSourcePos(unit, weaponId, ValueEmpyreanMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::StarSovereignCelestialCannon)
				{
					scbw::setMissileSourcePos(unit, weaponId, ValueSovereignCannonPos, 2, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::ClaymoreHarpoonRailgun)
				{
					scbw::setMissileSourcePos(unit, weaponId, ValueClaymoreMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::PenumbraErebolShockCannon)
				{
					scbw::setMissileSourcePos(unit, weaponId, ValuePenumbraLaserPos, 1, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::BurstLasers)
				{
					scbw::setMissileSourcePos(unit, weaponId, ValueWraithLaserPos, 3, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::CentaurSunchaserMissiles)
				{
					scbw::setMissileSourcePos(unit, weaponId, ValueCentaurMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::PhobosArgosLaserBatteries)
				{
					// Phobos - Argent Capacitors passive
					if (unit->id == UnitId::TerranPhobos && unitExt->values[ValuePhobosDischarge] == 0)
					{
						unit->addUnitEnergy(5 * 256);
						if (unit->energy == unit->getMaxEnergy())
						{
							unitExt->values[ValuePhobosDischarge] = 1;
						}
					}

					scbw::setMissileSourcePos(unit, weaponId, ValuePhobosLaserPos, 4, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::IonCannon)
				{
					scbw::setMissileSourcePos(unit, weaponId, ValueIonCannonProjectilePos, 1, ImagesDatEntries::OverlaySpecialPtr, 1);
				}
				else if (weaponId == WeaponId::KalkaliskViscousAcid)
				{
					auto bullet = scbw::setMissileSourcePos(unit, weaponId, ValueKalkalethMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					globals.remove_multihit_bullet(bullet);
				}
				else if (weaponId == WeaponId::KeskathalorScreamingVenom)
				{
					scbw::setMissileSourcePos(unit, weaponId, ValueKeskathalorMissilePos, 2, ImagesDatEntries::OverlayAttackPtr);
				}
				// Missile spawn positions end

				else if (weaponId == WeaponId::SalamanderPerditionBarrage)
				{
					// Salamander missile target positions
					auto rotate = unitExt->values[ValueSalamanderAtkPos];
					if (rotate == 1)
					{
						helper::createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
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
				}

				// Madrigal Furia Salvo
				else if (weaponId == WeaponId::MadrigalFuriaMissiles && unit->orderTarget.unit != nullptr)
				{
					fireMultiple(unit, unitExt, WeaponId::MadrigalFuriaMissiles, 0, Flags::Enemy); 
				}
				else
				{
					// Madcap - Maverick Feeders passive
					if (unit->id == UnitId::TerranMadcap)
					{
						s32 stacks = unitExt->values[ValueMadcapStacks];
						unitExt->values[ValueMadcapStacks] = MathUtils::clamp(stacks + MadcapPassiveDecayTime, 0, MadcapMaxStacks * MadcapPassiveDecayTime);
					}

					// Geszithalor 
					if (weaponId == WeaponId::AcidSpore && unit->sprite)
					{
						unitExt->values[ValueGuardianMissileLaunchPosX] = unit->getX();
						unitExt->values[ValueGuardianMissileLaunchPosY] = unit->getY();
					}
					helper::createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
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
					if (scbw::isAlliedTo(unit->playerId, target->playerId))
						return false;
					return scbw::canWeaponTargetUnit(weaponId, target, unit);
				}
				);
				if (a != nullptr)
				{
					auto old = unit->orderTarget;
					unit->orderTarget.unit = a;
					unit->orderTarget.pt = a->sprite->position;
					unitExt->values[ValueAdaptiveAmitosis] = 1;
					// TODO: verify that this will work with all weapon types
					const u32 Iscript_UseWeapon = 0x00479C30;
					__asm {
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

		// Akistrokor - Enthralling Assault ability
		if (weaponId == WeaponId::AkistrokorEnthrallingAssault && unit->orderTarget.unit != nullptr)
		{
			UnitExt* targetExt = globals.GetUnitExt(unit->orderTarget.unit);
			unitExt->units[UnitAttachedTarget] = unit->orderTarget.unit;
			targetExt->units[UnitAttachedAkistrokor] = unit;
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
				}
				);
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

		if (bullet->weaponId == WeaponId::ArchitectDispersiveSalvo && bullet->sourceUnit && bullet->attackTarget.unit &&
			bullet->attackTarget.unit->sprite && bullet->sourceUnit->sprite)
		{
			Unit* targ = bullet->attackTarget.unit;
			if (scbw::getDistanceFast(bullet->position.x, bullet->position.y, targ->getX(), targ->getY()) < 3 * 32)
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
					s32 count = 1;
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
						count++;
					};
					globals.add_multihit_empty_bullet(bullet, count);
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
							newBullet->position = bullet->position;
							globals.add_multihit_empty_bullet(newBullet, count);
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

		// Empyrean missile interception
		else if (bullet->weaponId == WeaponId::EmpyreanPlasmaBarrage)
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
				if (scbw::isAlliedTo(bullet->srcPlayer, unit->playerId))
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
				if (scbw::isAlliedTo(bullet->srcPlayer, unit->playerId))
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
			u32 range;
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
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
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
				if (scbw::isAlliedTo(bullet->srcPlayer, unit->playerId))
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
				if (bullet->weaponId == WeaponId::PenumbraErebolShockCannon
					|| bullet->weaponId == WeaponId::ClaymoreHarpoonRailgun)
				{
					globals.add_bullet_multihit(bullet, collisionTarget);
				}
				else if (bullet->weaponId == WeaponId::KalkaliskViscousAcid)
				{
					globals.add_incremental_bullet_multihit(bullet, collisionTarget);
				}
				if (bullet->weaponId == WeaponId::PenumbraErebolShockCannon)
				{
					// Penumbra - Shroud Piercer - pierce targets
					scbw::createAndInitThingy(SpriteId::BurstLasersImpactDamageless, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					hooks::WeaponBulletHit(bullet, collisionTarget, 2);
				}
				else if (bullet->weaponId == WeaponId::ClaymoreHarpoonRailgun)
				{
					// Claymore - Kelvin Munitions - pierce targets
					scbw::createAndInitThingy(SpriteId::BurstLasersImpactDamageless, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					hooks::WeaponBulletHit(bullet, collisionTarget, 2);

					// Claymore - Kelvin Munitions - Slow targets
					UnitExt* collisionTargetExt = globals.GetUnitExt(collisionTarget);
					if (!collisionTargetExt->values[ValueKelvinMunitionsSlow])
					{
						collisionTargetExt->values[ValueKelvinMunitionsSlow] = 1;
					}
				}
				else if (bullet->weaponId == WeaponId::KalkaliskViscousAcid) {
					// Kalkalisk behavior
//					auto divider = globals.get_multihit_count(bullet);
					helper::weaponBulletShot(collisionTarget, bullet, 2);
					helper::createThingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					bullet->moveTarget.unit = bullet->attackTarget.unit;
					if (bullet->moveTarget.unit != nullptr)
					{
						bullet->targetPosition = bullet->moveTarget.unit->position;
						bullet->targetPosition2 = bullet->moveTarget.unit->position;
					}
				}
			}
		}

		if (bEnd)
		{
			target = bullet->attackTarget.unit;
			bullet->behaviourTypeInternal = 5;
			if (target != nullptr)
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
				helper::playImageIscript(i, 1);
			}
		}
	}

	void bulletBehaviour_Bounce(Bullet* bullet) // 0048B2D0
	{
		Unit* v1;
		char nextBounce;
		Unit* target;
		Image* i;
		Unit* posY;
		Sprite* posX;
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
			//
			if (bullet->weaponId == WeaponId::GladiusPlasmaFlurry)
			{
				nextBounce = 1;
			}
			//
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
				posY = bullet->attackTarget.unit;
				bullet->behaviourTypeInternal = 5;
				if (posY)
				{
					bullet->attackTarget.pt.x = posY->sprite->position.x;
					bullet->attackTarget.pt.y = posY->sprite->position.y;
				}
				else
				{
					bullet->attackTarget.unit = 0;
				}
				posX = bullet->sprite;
				bullet->unknown_0x4E = 0; // Was lowbyte
				bullet->someUnitType = UnitId::None;

				for (j = posX->images.head; j; j = j->link.next)
				{
					helper::playImageIscript(j, IscriptAnimation::Death);
				}
			}
		}
	}

	bool findNextBounceTargetProc(Unit* pIterator) // 0048AFD0
	{
		Unit* pSrc;
		s32 nSrcStatus;
		Unit* _pIterator;
		s32 bulletSprite;
		Unit* posY;
		s32 posX;
		s32 v7;
		s32 v8;
		bool v9;
		s32 v10;
		Unit* v11;
		s32 v13;
		s8 v14;

		pSrc = *bulletBounceMissileSource;
		nSrcStatus = (*bulletBounceMissileSource)->status;
		_pIterator = pIterator;

		// Previously disallowed bouncing if unit was disabled

		if (!pIterator)
		{
			return 0;
		}
		if (pIterator->status & UnitStatus::Invincible)
		{
			return 0;
		}
		if (pIterator->sprite->flags & CSprite_Flags::Hidden)
		{
			return 0;
		}
		if (helper::unitCanSeeCloakedTarget(pIterator, (*bulletBounceMissileSource)))
		{
			return 0;
		}
		if (pSrc->subunit != nullptr
			&& helper::unitIsSubunit(pSrc->subunit))
		{
			posY = pSrc->subunit;
		}
		else
		{
			posY = pSrc;
		}
		if (!(helper::unitIsCarrier(pSrc)))
		{
			if (helper::unitIsReaver(pSrc))
			{
				if (_pIterator->status & UnitStatus::InAir)
					return 0;
				v9 = helper::unitHasPathToUnit(pSrc, _pIterator);
			}
			else
			{
				if (helper::unitCanInfest(pSrc))
				{
					v9 = helper::canBeInfested(v11, _pIterator) != 0; // v11 is not used..?
				}
				else
				{
					if (_pIterator->status & UnitStatus::InAir)
					{
						v9 = units_dat::AirWeapon[(u16)posY->id] != -126;
					}
					else
					{
						v9 = units_dat::GroundWeapon[(u16)posY->id] != -126;
					}
				}
			}
			if (!v9)
			{
				return 0;
			}
		}
		v13 = _pIterator->status;
		if (v13 & 0x300 && !((1 << pSrc->playerId) & _pIterator->visibilityStatus))
		{
			return 0;
		}
		v14 = _pIterator->playerId == 11 ? _pIterator->sprite->playerId : _pIterator->playerId;
		if (playerAlliance[(u8)pSrc->playerId].flags[v14] || *bulletBounceMissileTarget == _pIterator)
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
		//return (Unit*)helper::iterateUnitsAtLocationTargetProc(&coords, findNextBounceTargetProc, 0);
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
		}
		globals.remove_multihit_bullet(bullet);
		return bullet;
	}

	/*
	s32 writeBullets(FILE* hFile) {
		s32 v1; // edi
		Bullet* nextBounce; // esi
		Bullet* target; // eax
		s32 bulletSprite; // edx
		s32 posY; // ecx
		s32 posX; // esi
		size_t v7; // eax
		s16* v8; // eax
		u16 v9; // dx
		Bullet* v10; // ebx
		void* v11; // edi
		u32 v12; // esi
		s16* v13; // eax
		Bullet* v14; // ecx
		s32 v15; // edi
		Bullet* v16; // ecx
		s32 v18; // [esp+8h] [ebp-Ch]
		s16* v19; // [esp+Ch] [ebp-8h]
		s32 data; // [esp+10h] [ebp-4h]

		nextBounce = bulletTable;
		v1 = 100;
		do {
			packBulletData(nextBounce, 1);
			++nextBounce;
			--v1;
		} while (v1);
		bulletSprite = 0;
		data = 0;
		target = (Bullet*)((char*)bulletTable + 12);
		posY = 100;
		do {
			if (target->previous)
				++bulletSprite;
			++target;
			--posY;
		} while (posY);
		data = bulletSprite;
		v7 = _lockwrite(&data, 2u, 1u, hFile);
		posX = v7 == 1;
		if (v7 == 1) {
			if ((s16)data) {
				v12 = 114 * (unsigned __int16)data;
				v18 = 114 * (unsigned __int16)data;
				v8 = SMemAlloc(114 * (unsigned __int16)data, "Starcraft\\SWAR\\lang\\Bullet.cpp", 1276, 8);
				v11 = v8;
				v19 = v8;
				data = 0;
				v9 = 0;
				v10 = bulletTable;
				do
				{
					if (v10->sprite)
					{
						*v8 = v9;
						v13 = v8 + 1;
						qmemcpy(v13, v10, 0x70u);
						v11 = v19;
						v12 = v18;
						v8 = v13 + 56;
						++data;
					}
					++v9;
					++v10;
				} while (v9 < 0x64u);
				posX = compressWrite(v11, v12, hFile);
				SMemFree(v11, "Starcraft\\SWAR\\lang\\Bullet.cpp", 1299, 0);
			}
		}
		v14 = bulletTable;
		v15 = 100;
		do {
			unpackBulletData(v14, 1);
			v14 = v16 + 1;
			--v15;
		} while (v15);
		return posX;
	}*/
}