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
#include <set>
//-------- Helper function declarations. Do NOT modify! ---------//

static bool madrigalFuriaMatch(CUnit* madrigal, CUnit* unit)
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

static bool alkajSporeMatch(CBullet* bullet, CUnit* unit)
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

bool powerSiphonDistributeMatch(CUnit* savant, CUnit* unit)
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
	typedef void(__stdcall* GetWeaponHitPosFunc)(CUnit* unit, s32* x, s32* y);
	GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc)0x004762C0;
}

//-------- Actual hook functions --------//

namespace hooks
{

	//Fires a weapon from the @p unit.
	//This hook affects the following iscript opcodes: attackwith, attack, castspell
	//This also affects CUnit::fireWeapon().
	void fireWeaponHook(CUnit* unit, u8 wId)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);
		if (unit == nullptr)
		{
			return;
		}
		//Default StarCraft behavior
		u8 weaponId = wId;
		//Retrieve the spawning position for the bullet.
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
		if (unit->id != UnitId::ProtossCabalist)
		{
			scbw::decloakProc(unit);
		}

		if (weapons_dat::FlingyId[weaponId] != 0)
		{
			bool good = true;
			// Wyvern Land Grab ability
			if (unit->id == UnitId::TerranWyvern
				&& unit->sprite->mainGraphic->animation == IscriptAnimation::CastSpell
				&& unitExt->WyvernReverseLandgrabOn == 1)
			{
				good = false;
			}
			// Lurker Seismic Spines behavior
			if (weaponId == WeaponId::SubterraneanSpines)
			{
				unitExt->LurkerETA = 17;
				auto range = weapons_dat::MaxRange[weaponId] + 16;
				s32 angle = scbw::getAngle(unit->getX(), unit->getY(), unit->orderTarget.unit->getX(), unit->orderTarget.unit->getY());
				s32 targ_x = unit->getX() + scbw::getPolarX(range, angle);
				s32 targ_y = unit->getY() + scbw::getPolarY(range, angle);
				unitExt->SpineX = targ_x;
				unitExt->SpineY = targ_y;
				if (unit->orderTarget.unit != nullptr)
				{
					unitExt->LurkerPickTarget = (u32)unit->orderTarget.unit;
				}
				globals.clear_lakizilisk(unit);
			}
			if (good)
			{
				// Madrigal Tempo Change weapon switch
				if (weaponId == WeaponId::MadrigalFuriaMissiles
					&& scbw::get_generic_rust_value(unit, ValueId::TempoChange) == 1)
				{
					weaponId = WeaponId::MadrigalImperiosoMissiles;
				}
				// Savant Power Siphon ability
				if (weaponId == WeaponId::SavantPowerSiphon)
				{
					u32 energy = 0;
					u32 range = weapons_dat::OuterSplashRadius[WeaponId::SavantPowerSiphon];
					scbw::UnitFinder unitFinder_collect(x - range,
						y - range,
						x + range,
						y + range);
					unitFinder_collect.forEach([&](CUnit* target)
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
					u32 matchCount = unitFinder_distribute.getMatchUnitCount([&](CUnit* target)
					{
						return powerSiphonDistributeMatch(unit, target);
					});
					if (matchCount != 0 && energy > 0)
					{
						u32 perEach = energy / matchCount;
						u32 extra = energy % matchCount;
						CUnit* unitFirst = unitFinder_distribute.getFirst([&](CUnit* target)
						{
							return powerSiphonDistributeMatch(unit, target);
						});
						if (unitFirst != nullptr)
						{
							unitFirst->addUnitEnergy(extra);
						}
						u32 unspent = 0;
						unitFinder_distribute.forEachInMatch([&](CUnit* target)
						{
							return powerSiphonDistributeMatch(unit, target);
						}, [&](CUnit* u)
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
							CUnit* unitNew = unitFinder_distribute.getFirst([&](CUnit* target)
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
					std::set<CUnit*> monoceros;
					monoceros.insert(unit->orderTarget.unit);
					auto old = unit->orderTarget.unit;
					for (s32 i = 0; i < 4; i++)
					{
						auto range = hooks::getMaxWeaponRangeHook(unit, WeaponId::MinotaurAsterionLaserBattery) + 32;
						auto n = unitRangeCount(unit, CFlags::Enemy | CFlags::FrontalArc, {});
						auto newTarget = scbw::UnitFinder::getNearestTarget(
							unit->getX() - range,
							unit->getY() - range,
							unit->getX() + range,
							unit->getY() + range,
							unit,
							[&](CUnit* target)
						{
							if (target == unit
								|| monoceros.find(target) != monoceros.end()
								|| scbw::isAlliedTo(target->playerId, unit->playerId)
								|| !unit->canAttackTarget(target)
								|| !n(target))
							{
								return false;
							}
							return true;
						});
						if (monoceros.size() >= 5)
						{
							break;
						}
						if (newTarget == nullptr)
						{
							break;
						}
						monoceros.insert(newTarget);
						unit->orderTarget.unit = newTarget;
						auto newAngle = scbw::getAngle(unit->getX(), unit->getY(),
							newTarget->getX(), newTarget->getY());
						helper::createBullet(weaponId, unit, unit->getX(), unit->getY(), unit->playerId, newAngle);
						if (unitExt->Deconstruction)
						{
							scbw::doDeconstructionDamage(unit, weaponId, false);
						}
					}
					unit->orderTarget.unit = old;
				}
				// Titan Reactors - Heracles behavior - Refresh timer on attack and add stack
				if (weaponId == WeaponId::HeraclesJavelinCannons)
				{
					unitExt->NemeanReactorsTimer = 24 * 5;
					s32 stacks = unitExt->NemeanReactorsStack;
					if (stacks < 5)
					{
						unitExt->NemeanReactorsStack = stacks++;
						scbw::playSound(1372, unit);
					}
				}
				// Missile spawn positions
				if (weaponId == WeaponId::EmpyreanPlasmaBarrage)
				{
					// Empyrean missile spawn positions
					scbw::setMissileSourcePos(unit, weaponId, &unitExt->EmpyreanMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::CentaurSunchaserMissiles)
				{
					// Centaur missile spawn positions
					scbw::setMissileSourcePos(unit, weaponId, &unitExt->CentaurMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::PhobosArgosLaserBatteries)
				{
					// Phobos - Argent Capacitors passive
					if (unit->id == UnitId::TerranPhobos && unitExt->PhobosDischarge == 0)
					{
						unit->addUnitEnergy(5 * 256);
						if (unit->energy == unit->getMaxEnergy())
						{
							unitExt->PhobosDischarge = 1;
						}
					}
					// Phobos laser spawn positions
					scbw::setMissileSourcePos(unit, weaponId, &unitExt->PhobosLaserPos, 4, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::KalkaliskViscousAcid)
				{
					// Kalkaleth missile spawn position
					auto bullet = scbw::setMissileSourcePosRet(unit, weaponId, &unitExt->KalkalethMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					globals.remove_multihit_bullet(bullet);
				}
				else if (weaponId == WeaponId::KeskathalorScreamingVenom)
				{
					// Keskathalor missile spawn position
					scbw::setMissileSourcePos(unit, weaponId, &unitExt->KeskathalorMissilePos, 2, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::StarSovereignCelestialCannon)
				{
					// Star Sovereign laser spawn position
					scbw::setMissileSourcePos(unit, weaponId, &unitExt->SovereignCannonPos, 2, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::PenumbraErebolShockCannon)
				{
					// Penumbra laser spawn position
					scbw::setMissileSourcePos(unit, weaponId, &unitExt->PenumbraLaserPos, 1, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::BurstLasers)
				{
					// Wraith laser spawn position
					scbw::setMissileSourcePos(unit, weaponId, &unitExt->WraithLaserPos, 3, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::IonCannon)
				{
					// Ion Cannon projectile spawn position
					scbw::setMissileSourcePos(unit, weaponId, &unitExt->IonCannonProjectilePos, 1,
						ImagesDatEntries::OverlaySpecialPtr, 1); // 1 is frameset offset
				}
				else if (weaponId == WeaponId::SirenCharybdalRifle)
				{
					// TODO: Implement multi-unit targeting without inlining Adaptive Amitosis behavior
				}
				else if (weaponId == WeaponId::SalamanderPerditionBarrage)
				{
					// SalamanderFire missile target positions
					auto rotate = unitExt->SalamanderAtkPos;
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
					unitExt->SalamanderAtkPos = rotate;
				}
				else if (weaponId == WeaponId::MadrigalFuriaMissiles && unit->orderTarget.unit != nullptr)
				{
					helper::createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
					if (unitExt->Deconstruction)
					{
						scbw::doDeconstructionDamage(unit, weaponId);
					}
					auto old = unit->orderTarget.unit;
					auto range = weapons_dat::MaxRange[weaponId];
					auto count = 0;
					scbw::UnitFinder unitsInSplash(unit->getX() - range, unit->getY() - range, unit->getX() + range, unit->getY() + range);
					for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
					{
						CUnit* target = unitsInSplash.getUnit(i);
						if (madrigalFuriaMatch(unit, target))
						{
							count++;
							if (target != nullptr)
							{
								unit->orderTarget.unit = target;
								auto newAngle = scbw::getAngle(unit->getX(), unit->getY(),
									target->getX(), target->getY());
								helper::createBullet(weaponId, unit, unit->getX(), unit->getY(), unit->playerId, newAngle);
								if (unitExt->Deconstruction)
								{
									scbw::doDeconstructionDamage(unit, weaponId, false);
								}
							}
						}
					};
					unit->orderTarget.unit = old;
				}
				else
				{
					// Madcap - Maverick Feeders passive
					if (unit->id == UnitId::TerranMadcap)
					{
						s32 stacks = unitExt->MadcapStacks;
						unitExt->MadcapStacks = MathUtils::clamp(stacks + MadcapPassiveDecayTime, 0, MadcapMaxStacks * MadcapPassiveDecayTime);
					}
					if (weaponId == WeaponId::AcidSpore && unit->sprite)
					{
						unitExt->GuardianMissileLaunchPosX = unit->getX();
						unitExt->GuardianMissileLaunchPosY = unit->getY();
					}
					helper::createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
				}
			}
			// Chasmal Colony - Adaptive Amitosis ability
			if (unit->orderTarget.unit != nullptr
				&& unitExt->AdaptiveAmitosis)
			{
				auto range = weapons_dat::MaxRange[weaponId];
				auto a = scbw::UnitFinder::getNearestTarget(
					unit->getX() - range,
					unit->getY() - range,
					unit->getX() + range,
					unit->getY() + range,
					unit, [&](CUnit* target)
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
					unitExt->AdaptiveAmitosis = 1;
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
		if (unitExt->Deconstruction)
		{
			scbw::doDeconstructionDamage(unit, weaponId);
		}
	}

	void progressBulletFlight(CBullet* bullet)
	{ //		
		CBullet* v1; // esi
		CUnit* target; // eax
		CSprite* v4; // edx
		CImage* i; // esi

		if (bullet->weaponType == WeaponId::AlkajeliskParasiticSpores)
		{
			if (bullet->attackTarget.unit == nullptr)
			{
				u32 range = 8 * 32;//seek range
				auto seekTarget = scbw::UnitFinder::getNearestTarget(
					bullet->position.x - range,
					bullet->position.y - range,
					bullet->position.x + range,
					bullet->position.y + range,
					bullet,
					[&](CUnit* unit)
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
		if (bullet->weaponType == WeaponId::AlkajeliskParasiticSpores)
		{
			if (bullet->time_remaining < 135 && bullet->attackTarget.unit == nullptr)
			{ //135 is 255 - 5*24
				time = 0;
			}
		}
		bool bEnd = false;

		// Accelerate missiles over time
		if (bullet->weaponType == WeaponId::EmpyreanPlasmaBarrage
			|| bullet->weaponType == WeaponId::AntiMatterMissiles)
		{
			u8 time_current = 255 - time;
			if (time_current == 8)
			{
				bullet->flingyTopSpeed *= 2;
				bullet->flingyAcceleration *= 2;
			}
		}
		if (bullet->weaponType == WeaponId::ArchitectDispersiveSalvo && bullet->sourceUnit && bullet->attackTarget.unit &&
			bullet->attackTarget.unit->sprite && bullet->sourceUnit->sprite)
		{
			CUnit* targ = bullet->attackTarget.unit;
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
					s32 count = 0;
					for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
					{
						CUnit* target = unitsInSplash.getUnit(i);
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
					globals.add_multihit_empty_bullet(bullet, count + 1);
					for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
					{
						CUnit* target = unitsInSplash.getUnit(i);
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
						auto newBullet = helper::createBulletRet(WeaponId::ArchitectDispersiveSalvo,
							bullet->sourceUnit, bullet->position.x, bullet->position.y, bullet->srcPlayer, newAngle);
						if (newBullet)
						{
							newBullet->position = bullet->position;
							globals.add_multihit_empty_bullet(newBullet, count + 1);
						}
					};
					bullet->sourceUnit->orderTarget.unit = old;
					bullet->sourceUnit->position = oldPos;
					bullet->sourceUnit->sprite->position = oldSpritePos;
				}
			}
		}
		if (time == 0 || bullet->moveTarget.pt.x == bullet->position.x && bullet->moveTarget.pt.y == bullet->position.y)
		{
			bEnd = true;
		}

		// Empyrean missile interception
		else if (bullet->weaponType == WeaponId::EmpyreanPlasmaBarrage)
		{
			u32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](CUnit* unit)
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
		else if (bullet->weaponType == WeaponId::StarSovereignInterventionMissiles)
		{
			u32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](CUnit* unit)
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
		else if (bullet->weaponType == WeaponId::KalkaliskViscousAcid || bullet->weaponType == WeaponId::PenumbraErebolShockCannon)
		{
			// for missiles hitting targets on its way
			u32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](CUnit* unit)
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
				if (bullet->weaponType == WeaponId::PenumbraErebolShockCannon)
				{
					globals.add_bullet_multihit(bullet, collisionTarget);
				}
				else
				{
					globals.add_incremental_bullet_multihit(bullet, collisionTarget);
				}
				if (bullet->weaponType == WeaponId::PenumbraErebolShockCannon)
				{
					//Penumbra Behavior
					scbw::createThingy(644, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					helper::weaponBulletShot(collisionTarget, bullet, 2);
				}
				else
				{
					//Kalkaleth Behavior
//					auto divider = globals.get_multihit_count(bullet);
					helper::weaponBulletShot(collisionTarget, bullet, 2);
					//					scbw::printf("Hit by bullet: %X", bullet);
					scbw::createThingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, bullet->position.x, bullet->position.y, bullet->srcPlayer);
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
			if (/*bullet->weaponType == WeaponId::ViscousAcid || */bullet->weaponType == WeaponId::ArchitectDispersiveSalvo)
			{
				globals.remove_multihit_bullet(bullet);
			}
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
			v4 = bullet->sprite;
			bullet->orderSignal |= 0x80;//new
			bullet->unknown_0x4E = 0;
			bullet->someUnitType = 228;
			//			scbw::printf("Hit by bullet: %X", bullet);
			for (i = v4->images.head; i; i = i->link.next)
				helper::playImageIscript(i, 1);


		}
	}

	void bulletBehaviour_Bounce(CBullet* a1)
	{ // 0048B2D0
		CUnit* v1; // eax
		char v2; // al
		CUnit* v3; // ebx
		CImage* i; // esi
		CUnit* v5; // eax
		CSprite* v6; // edx
		CImage* j; // esi

		v1 = a1->attackTarget.unit;
		if (v1)
		{
			if (!(a1->hitFlags & 1))
			{
				helper::changeMovePos(a1, v1->sprite->position.y, v1->sprite->position.x);
			}
		}
		helper::progressBulletMovement_func(a1);
		if (a1->moveTarget.pt.x == a1->position.x && a1->moveTarget.pt.y == a1->position.y)
		{
			v2 = a1->remainingBounces - 1;
			//
			if (a1->weaponType == WeaponId::GladiusPlasmaFlurry)
			{
				v2 = 1;
			}
			//
			a1->remainingBounces = v2;
			globals.bounce_bullet = a1;
			//			scbw::print("BounceHit");
			if (v2 && (v3 = helper::findNextBounceTarget_func(a1), a1->nextBounceUnit = a1->attackTarget.unit, v3))
			{
				for (i = a1->sprite->images.head; i; i = i->link.next)
				{
					helper::playImageIscript(i, IscriptAnimation::SpecialState1);
				}
				//				scbw::print("Bounce");
				a1->attackTarget.unit = v3;
			}
			else
			{
				//				scbw::print("NoJump");
				if (a1->weaponType == WeaponId::GladiusPlasmaFlurry)
				{
					globals.remove_multihit_bullet(a1);
				}
				v5 = a1->attackTarget.unit;
				a1->behaviourTypeInternal = 5;
				if (v5)
				{
					a1->attackTarget.pt.x = v5->sprite->position.x;
					a1->attackTarget.pt.y = v5->sprite->position.y;
				}
				else
				{
					a1->attackTarget.unit = 0;
				}
				v6 = a1->sprite;
				a1->unknown_0x4E = 0; // Was lowbyte
				a1->someUnitType = UnitId::None;

				for (j = v6->images.head; j; j = j->link.next)
				{
					helper::playImageIscript(j, IscriptAnimation::Death);
				}
			}
		}
	}

	bool findNextBounceTargetProc(CUnit* pIterator)
	{ // 0048AFD0
		CUnit* pSrc; // esi
		s32 nSrcStatus; // eax
		CUnit* _pIterator; // edi
		s32 v4; // eax
		CUnit* v5; // ebx
		s32 v6; // eax
		s32 v7; // eax
		s32 v8; // eax
		bool v9; // eax
		s32 v10; // eax
		CUnit* v11; // ecx
		s32 v13; // eax
		s8 v14; // cl

		pSrc = *bulletBounceMissileSource;
		nSrcStatus = (*bulletBounceMissileSource)->status;
		_pIterator = pIterator;
		// The following checks disallowed bouncing if the source was disabled. This feature is removed in HYDRA.
/*		if (nSrcStatus & UnitStatus::DoodadStatesThing) {
			scbw::print("Source unit is disabled (doodad state)");
			return 0;
		}
		if ((*bulletBounceMissileSource)->lockdownTimer) {
			scbw::print("Source unit is disabled (lockdown)");
			return 0;
		}
		if ((*bulletBounceMissileSource)->stasisTimer) {
			scbw::print("Source unit is disabled (stasis)");
			return 0;
		}
		if ((*bulletBounceMissileSource)->maelstromTimer) {
			scbw::print("Source unit is disabled (maelstrom)");
			return 0;
		}*/
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
			v5 = pSrc->subunit;
		}
		else
		{
			v5 = pSrc;
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
					/*if (pSrc->id == UnitId::ProtossArbiter && pSrc->pAI) {

						return 0;
					}*/
					if (_pIterator->status & UnitStatus::InAir)
					{
						v9 = units_dat::AirWeapon[(u16)v5->id] != -126;
					}
					else
					{
						v9 = units_dat::GroundWeapon[(u16)v5->id] != -126;
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

		// Gladius Binding Flurry behavior
//		scbw::printf("Gladius code start %d %d",(u32)bulletBounceMissileNext,(u32)pIterator);

		if (globals.bounce_bullet->weaponType == WeaponId::GladiusPlasmaFlurry)
		{
			if (pIterator != nullptr && *bulletBounceMissileNext != nullptr
				&& (*bulletBounceMissileNext)->sprite != nullptr
				&& _pIterator->sprite != nullptr)
			{
				if (globals.was_already_hit_by_multihit_bullet(globals.bounce_bullet, _pIterator))
				{
					//scbw::print("NoItr - Already hit");
					return false;
				}
				else
				{
					//scbw::printf("Add hit %d",globals.bounce_bullet);
					globals.add_bullet_multihit(globals.bounce_bullet, _pIterator);
					return *bulletBounceMissileNext != _pIterator;

				}
			}
			else
			{
				//scbw::print("bad bullet");
			}
		}
		//		scbw::print("Gladius code end");
		return *bulletBounceMissileNext != _pIterator;
	}
	CUnit* findNextBounceTarget(CBullet* a1)
	{ // 0048B1E0
		CUnit* v2; // edx
		CUnit* v3; // ecx
		CSprite* v4; // eax
		u16 v5;
		u16 v6;
		Box32 _a1; // [esp+0h] [ebp-8h]

		if (!a1->sourceUnit)
		{
			return 0;
		}
		v2 = a1->nextBounceUnit;
		*bulletBounceMissileSource = a1->sourceUnit;
		v3 = a1->attackTarget.unit;
		v4 = a1->sprite;
		*bulletBounceMissileTarget = v3;
		v5 = v4->position.y;
		v6 = v4->position.x;
		*bulletBounceMissileNext = v2;
		_a1.left = (s16)v6 - 96;
		_a1.top = (s16)v6 + 96;
		_a1.right = (s16)v5 - 96;
		_a1.bottom = (s16)v5 + 96;
		//		return (CUnit*)iterateUnitsAtLocationTargetProc(&_a1, (s32(__fastcall*)(s32, s32))findNextBounceTargetProc, 0);
		GPTP::logger << (u32)(*bulletBounceMissileNext) << " " << (u32)(*bulletBounceMissileSource) << " " << (u32)(*bulletBounceMissileTarget) << '\n';
		return (CUnit*)helper::iterateUnitsAtLocationTargetProc(&_a1, (s32(__fastcall*)(s32, s32))findNextBounceTargetProc, 0);
	}

	/*
	s32 writeBullets(FILE* hFile) {
		s32 v1; // edi
		CBullet* v2; // esi
		CBullet* v3; // eax
		s32 v4; // edx
		s32 v5; // ecx
		s32 v6; // esi
		size_t v7; // eax
		s16* v8; // eax
		u16 v9; // dx
		CBullet* v10; // ebx
		void* v11; // edi
		u32 v12; // esi
		s16* v13; // eax
		CBullet* v14; // ecx
		s32 v15; // edi
		CBullet* v16; // ecx
		s32 v18; // [esp+8h] [ebp-Ch]
		s16* v19; // [esp+Ch] [ebp-8h]
		s32 data; // [esp+10h] [ebp-4h]

		v2 = bulletTable;
		v1 = 100;
		do {
			packBulletData(v2, 1);
			++v2;
			--v1;
		} while (v1);
		v4 = 0;
		data = 0;
		v3 = (CBullet*)((char*)bulletTable + 12);
		v5 = 100;
		do {
			if (v3->previous)
				++v4;
			++v3;
			--v5;
		} while (v5);
		data = v4;
		v7 = _lockwrite(&data, 2u, 1u, hFile);
		v6 = v7 == 1;
		if (v7 == 1) {
			if ((s16)data) {
				v12 = 114 * (unsigned __int16)data;
				v18 = 114 * (unsigned __int16)data;
				v8 = SMemAlloc(114 * (unsigned __int16)data, "Starcraft\\SWAR\\lang\\CBullet.cpp", 1276, 8);
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
				v6 = compressWrite(v11, v12, hFile);
				SMemFree(v11, "Starcraft\\SWAR\\lang\\CBullet.cpp", 1299, 0);
			}
		}
		v14 = bulletTable;
		v15 = 100;
		do {
			unpackBulletData(v14, 1);
			v14 = v16 + 1;
			--v15;
		} while (v15);
		return v6;
	}*/
}