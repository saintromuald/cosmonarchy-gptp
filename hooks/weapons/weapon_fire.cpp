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

static bool madrigalFuriaMatch(CUnit* madrigal, CUnit* unit) {
	if (unit == madrigal) {
		return false;
	}
	if (unit == madrigal->orderTarget.unit) {
		return false;
	}
	if (scbw::isAlliedTo(madrigal->playerId, unit->playerId)) {
		return false;
	}
	if (!madrigal->canAttackTarget(unit)) {
		return false;
	}
	return true;
}

static bool alkajSporeMatch(CBullet* bullet, CUnit* unit) {
	if (unit->status & UnitStatus::Invincible) {
		return false;
	}
	if (scbw::isAlliedTo(bullet->srcPlayer, unit->playerId)) {
		return false;
	}
	return true;
}

bool powerSiphonDistributeMatch(CUnit* savant, CUnit* unit) {
	if (!savant) {
		return false;
	}
	if (unit == savant) {
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster)) {
		return false;
	}
	if (!scbw::isAlliedTo(savant->playerId, unit->playerId)) {
		return false;
	}
	if (unit->energy == unit->getMaxEnergy()) {
		return false;
	}
	return true;
}

namespace { // Helper functions
	typedef void(__stdcall* GetWeaponHitPosFunc)(CUnit* unit, int32* x, int32* y);
	GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc)0x004762C0;
	void createBullet(uint8 weaponId, CUnit* source, int16 x, int16 y, uint8 attackingPlayer, uint8 direction);

	// For progressBulletFlight
	void progressBulletMovement(CBullet* a1); // 0048B250
	void playImageIscript(CImage* a1, char a2); // 004D8470
	void doMissileDmg(CBullet* bullet);  //0048B770

	// For bulletBehavior_Bounce
	CBullet* changeMovePos(CBullet* a1, int32 a2, int32 a3); // 00495240
	void progressBulletMovement_func(CBullet* a1); // 0048B250
	CUnit* findNextBounceTarget_func(CBullet* a1); // 0048B1E0

	// For findNextBounceTargetProc
	bool unitCanSeeCloakedTarget(CUnit* a1, CUnit* a2); // 00401D60
	bool unitIsSubunit(CUnit* a1); // 00401D40
	bool unitIsCarrier(CUnit* a1); // 00401470
	bool unitIsReaver(CUnit* a1); // 00401490
	bool unitHasPathToUnit(CUnit* a1, CUnit* a2); // 0049CBB0
	bool canInfest(CUnit* a1); // 00402750
	int32 canBeInfested(CUnit* a1, CUnit* a2); // 00402210

	// For findNextBounceTarget
	int32 iterateUnitsAtLocationTargetProc(Box32* a1, int32(__fastcall* a2)(int32, int32), CUnit* a3); // 004E8280

	// For writeBullets
/*	void packBulletData(CBullet* a1, int32 a2); // 0048A9A0
	size_t _lockwrite(const void* a1, size_t a2, size_t a3, FILE* a4); // 00411931
	void* SMemAlloc(int32 a1, char* a2, int32 a3, int32 a4); // 0041006A
	int32 compressWrite(void* a1, uint32 a2, FILE* a3); // 004C3450
	bool SMemFree(void* a1, char* a2, int32 a3, char a4); // 00410070
	CBullet* unpackBulletData(CBullet* a1, int32 a2); // 0048A8D0	*/

} //unnamed namespace


//-------- Actual hook functions --------//

namespace hooks {

	//Fires a weapon from the @p unit.
	//This hook affects the following iscript opcodes: attackwith, attack, castspell
	//This also affects CUnit::fireWeapon().
	void fireWeaponHook(CUnit* unit, uint8 wId) {
		if (unit == nullptr) {
			return;
		}
		//Default StarCraft behavior
		uint8 weaponId = wId;
		//Retrieve the spawning position for the bullet.
		int32 x, y;
		uint8 behavior = weapons_dat::Behavior[weaponId];
		if (weaponId == WeaponId::SavantPowerSiphon) {
			behavior = WeaponBehavior::AppearOnTargetSite;
		}
		if (behavior == WeaponBehavior::AppearOnTargetUnit) {
			if (unit->orderTarget.unit == nullptr) {
				return;
			}
			getWeaponHitPos(unit, &x, &y);
		} else if (behavior == WeaponBehavior::AppearOnTargetSite) {
			x = unit->orderTarget.pt.x;
			y = unit->orderTarget.pt.y;
		} else {
			int32 forwardOffset = weapons_dat::ForwardOffset[weaponId];

			x = unit->getX() + scbw::getPolarX(forwardOffset, unit->currentDirection1);
			y = unit->getY() + scbw::getPolarY(forwardOffset, unit->currentDirection1)
				- weapons_dat::VerticalOffset[weaponId];
		}

		// Cloaked Units - Decloak
		if (unit->id != UnitId::ProtossCabalist) {
			scbw::decloakProc(unit);
		}

		if (weapons_dat::FlingyId[weaponId] != 0) {
			bool good = true;
			// Wyvern Land Grab ability
			if (unit->id == UnitId::TerranWyvern
				&& unit->sprite->mainGraphic->animation == IscriptAnimation::CastSpell
				&& scbw::get_generic_value(unit, ValueId::WyvernReverseLandgrabOn) == 1) {
				good = false;
			}
			// Lurker Seismic Spines behavior
			if (weaponId == WeaponId::SubterraneanSpines) {
				scbw::set_generic_timer(unit, ValueId::LurkerETA, 17);
				auto range = weapons_dat::MaxRange[weaponId] + 16;
				int angle = scbw::getAngle(unit->getX(), unit->getY(), unit->orderTarget.unit->getX(), unit->orderTarget.unit->getY());
				int targ_x = unit->getX() + scbw::getPolarX(range, angle);
				int targ_y = unit->getY() + scbw::getPolarY(range, angle);
				scbw::set_generic_value(unit, ValueId::SpineX, targ_x);
				scbw::set_generic_value(unit, ValueId::SpineY, targ_y);
				if (unit->orderTarget.unit != nullptr) {
					scbw::set_generic_value(unit, ValueId::LurkerPickTarget, (uint32)unit->orderTarget.unit);
				}
				globals.clear_lakizilisk(unit);
			}
			if (good) {
				// Madrigal Tempo Change weapon switch
				if (weaponId == WeaponId::MadrigalFuriaMissiles
					&& scbw::get_generic_rust_value(unit, ValueId::TempoChange) == 1) {
					weaponId = WeaponId::MadrigalImperiosoMissiles;
				}
				// Savant Power Siphon ability
				if (weaponId == WeaponId::SavantPowerSiphon) {
					uint32 energy = 0;
					uint32 range = weapons_dat::OuterSplashRadius[WeaponId::SavantPowerSiphon];
					scbw::UnitFinder unitFinder_collect(x - range,
						y - range,
						x + range,
						y + range);
					unitFinder_collect.forEach([&](CUnit* target) {
						constexpr uint32 ENERGY_PROC_MAX = 50 * 256;
						if (energy == ENERGY_PROC_MAX) {
							return;
						}
						if (target == unit) {
							return;
						}
						if (!(units_dat::BaseProperty[target->id] & UnitProperty::Spellcaster)) {
							return;
						}
						if (target->energy == 0) {
							return;
						}
						uint32 energyLeft = ENERGY_PROC_MAX - energy;
						uint32 drain = MathUtils::clamp<uint32>(target->energy, 0, energyLeft);
						energy += drain;
						target->spendUnitEnergy(drain);
						uint8 overlaySize = scbw::hasOverlay(target);
						if (target->subunit != nullptr) {
							target->subunit->sprite->createTopOverlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);
						} else {
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
					uint32 matchCount = unitFinder_distribute.getMatchUnitCount([&](CUnit* target) {
						return powerSiphonDistributeMatch(unit, target);
					});
					if (matchCount != 0 && energy > 0) {
						uint32 perEach = energy / matchCount;
						uint32 extra = energy % matchCount;
						CUnit* unitFirst = unitFinder_distribute.getFirst([&](CUnit* target) {
							return powerSiphonDistributeMatch(unit, target);
						});
						if (unitFirst != nullptr) {
							unitFirst->addUnitEnergy(extra);
						}
						uint32 unspent = 0;
						unitFinder_distribute.forEachInMatch([&](CUnit* target) {
							return powerSiphonDistributeMatch(unit, target);
						}, [&](CUnit* u) {
							u->addUnitEnergy(unspent);
							energy -= unspent;
							unspent = 0;
							uint32 howMuch = MathUtils::clamp<uint32>(perEach, 0, u->getMaxEnergy() - u->energy);
							u->energy += howMuch;
							energy -= howMuch;
							uint8 overlaySize = scbw::hasOverlay(u);
							if (u->subunit != nullptr) {
								u->subunit->sprite->createTopOverlay(overlaySize + ImageId::MindControlHit_Small, 0, 0, 0);
							} else {
								u->sprite->createTopOverlay(overlaySize + ImageId::MindControlHit_Small, 0, 0, 0);
							}
							unspent = perEach - howMuch;
						});
						if (energy > 0) {
							CUnit* unitNew = unitFinder_distribute.getFirst([&](CUnit* target) {
								return powerSiphonDistributeMatch(unit, target);
							});
							if (unitNew != nullptr) {
								unitNew->addUnitEnergy(extra);
							}
						}
					}
				}
				// Monoceros Targeting - Battlecruiser behavior
				else if (weaponId == WeaponId::MinotaurAsterionLaserBattery) {
					std::set<CUnit*> monoceros;
					monoceros.insert(unit->orderTarget.unit);
					scbw::createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
					if (scbw::get_generic_value(unit, ValueId::Deconstruction) > 0) {
						scbw::doDeconstructionDamage(unit, weaponId);
					}
					auto old = unit->orderTarget.unit;
					for (int i = 0; i < 4; i++) {
						auto range = hooks::getMaxWeaponRangeHook(unit, WeaponId::MinotaurAsterionLaserBattery) + 32;
						auto n = unitRangeCount(unit, CFlags::Enemy | CFlags::FrontalArc, {});
						auto newTarget = scbw::UnitFinder::getNearestTarget(
							unit->getX() - range,
							unit->getY() - range,
							unit->getX() + range,
							unit->getY() + range,
							unit,
							[&](CUnit* target) {
							if (target == unit
								|| monoceros.find(target) != monoceros.end()
								|| scbw::isAlliedTo(target->playerId, unit->playerId)
								|| !unit->canAttackTarget(target)
								|| !n(target)) {
								return false;
							}
							return true;
						});
						if (monoceros.size() >= 5) {
							break;
						}
						if (newTarget == nullptr) {
							break;
						}
						monoceros.insert(newTarget);
						unit->orderTarget.unit = newTarget;
						auto newAngle = scbw::getAngle(unit->getX(), unit->getY(),
							newTarget->getX(), newTarget->getY());
						scbw::createBullet(weaponId, unit, unit->getX(), unit->getY(), unit->playerId, newAngle);
						if (scbw::get_generic_value(unit, ValueId::Deconstruction) > 0) {
							scbw::doDeconstructionDamage(unit, weaponId, false);
						}
					}
					unit->orderTarget.unit = old;
				}
				// Titan Reactors - Heracles behavior - Refresh timer on attack and add stack
				if (weaponId == WeaponId::HeraclesJavelinCannons) {
					scbw::set_generic_timer(unit, ValueId::NemeanReactorsTimer, 24 * 5);
					int32 stacks = scbw::get_generic_value(unit, ValueId::NemeanReactorsStack);
					if (stacks < 5) {
						scbw::set_generic_value(unit, ValueId::NemeanReactorsStack, stacks + 1);
						scbw::playSound(1372, unit);
					}
				}
				// Missile spawn positions
				if (weaponId == WeaponId::EmpyreanPlasmaBarrage) {
					// Empyrean missile spawn positions
					scbw::setMissileSourcePos(unit, weaponId, ValueId::EmpyreanMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
				} else if (weaponId == WeaponId::CentaurSunchaserMissiles) {
					// Centaur missile spawn positions
					scbw::setMissileSourcePos(unit, weaponId, ValueId::CentaurMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
				} else if (weaponId == WeaponId::PhobosArgosLaserBatteries) {
					// Phobos - Argent Capacitors passive
					if (unit->id == UnitId::TerranPhobos && scbw::get_generic_value(unit, ValueId::PhobosDischarge) == 0) {
						unit->addUnitEnergy(5 * 256);
						if (unit->energy == unit->getMaxEnergy()) {
							scbw::set_generic_value(unit, ValueId::PhobosDischarge, 1);
						}
					}
					// Phobos laser spawn positions
					scbw::setMissileSourcePos(unit, weaponId, ValueId::PhobosLaserPos, 4, ImagesDatEntries::OverlayAttackPtr);
				} else if (weaponId == WeaponId::KalkaliskViscousAcid) {
					// Kalkaleth missile spawn position
					auto bullet = scbw::setMissileSourcePosRet(unit, weaponId, ValueId::KalkalethMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					globals.remove_multihit_bullet(bullet);
				} else if (weaponId == WeaponId::KeskathalorScreamingVenom) {
					// Keskathalor missile spawn position
					scbw::setMissileSourcePos(unit, weaponId, ValueId::KeskathalorMissilePos, 2, ImagesDatEntries::OverlayAttackPtr);
				} else if (weaponId == WeaponId::StarSovereignCelestialCannon) {
					// Star Sovereign laser spawn position
					scbw::setMissileSourcePos(unit, weaponId, ValueId::SovereignCannonPos, 2, ImagesDatEntries::OverlayAttackPtr);
				} else if (weaponId == WeaponId::PenumbraErebolShockCannon) {
					// Penumbra laser spawn position
					scbw::setMissileSourcePos(unit, weaponId, ValueId::PenumbraLaserPos, 1, ImagesDatEntries::OverlayAttackPtr);
				} else if (weaponId == WeaponId::BurstLasers) {
					// Wraith laser spawn position
					scbw::setMissileSourcePos(unit, weaponId, ValueId::WraithLaserPos, 3, ImagesDatEntries::OverlayAttackPtr);
				} else if (weaponId == WeaponId::IonCannon) {
					// Ion Cannon projectile spawn position
					scbw::setMissileSourcePos(unit, weaponId, ValueId::IonCannonProjectilePos, 1,
						ImagesDatEntries::OverlaySpecialPtr, 1); // 1 is frameset offset
				} else if (weaponId == WeaponId::SirenCharybdalRifle) {
					// TODO: Implement multi-unit targeting without inlining Adaptive Amitosis behavior
				} else if (weaponId == WeaponId::SalamanderPerditionBarrage) {
					// SalamanderFire missile target positions
					auto rotate = scbw::get_generic_value(unit, ValueId::SalamanderAtkPos);
					if (rotate == 1) {
						createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
					} else {
						scbw::rangeModifiedFire(weaponId, unit, x, y, 32 - 32 * rotate);
					}
					rotate++;
					if (rotate > 2) {
						rotate = 0;
					}
					scbw::set_generic_value(unit, ValueId::SalamanderAtkPos, rotate);
				} else if (weaponId == WeaponId::MadrigalFuriaMissiles && unit->orderTarget.unit != nullptr) {
					scbw::createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
					if (scbw::get_generic_value(unit, ValueId::Deconstruction) > 0) {
						scbw::doDeconstructionDamage(unit, weaponId);
					}
					auto old = unit->orderTarget.unit;
					auto range = weapons_dat::MaxRange[weaponId];
					auto count = 0;
					scbw::UnitFinder unitsInSplash(unit->getX() - range, unit->getY() - range, unit->getX() + range, unit->getY() + range);
					for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
						CUnit* target = unitsInSplash.getUnit(i);
						if (madrigalFuriaMatch(unit, target)) {
							count++;
							if (target != nullptr) {
								unit->orderTarget.unit = target;
								auto newAngle = scbw::getAngle(unit->getX(), unit->getY(),
									target->getX(), target->getY());
								scbw::createBullet(weaponId, unit, unit->getX(), unit->getY(), unit->playerId, newAngle);
								if (scbw::get_generic_value(unit, ValueId::Deconstruction) > 0) {
									scbw::doDeconstructionDamage(unit, weaponId, false);
								}
							}
						}
					};
					unit->orderTarget.unit = old;
				} else {
					// Madcap - Maverick Feeders passive
					if (unit->id == UnitId::TerranMadcap) {
						int stacks = scbw::get_generic_value(unit, ValueId::MadcapStacks);
						scbw::set_generic_value(unit, ValueId::MadcapStacks, MathUtils::clamp(stacks + MadcapPassiveDecayTime, 0, MadcapMaxStacks * MadcapPassiveDecayTime));
					}
					if (weaponId == WeaponId::AcidSpore && unit->sprite) {
						scbw::set_generic_value(unit, ValueId::GuardianMissileLaunchPosX, unit->getX());
						scbw::set_generic_value(unit, ValueId::GuardianMissileLaunchPosY, unit->getY());
					}
					createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
				}
			}
			// Chasmal Colony - Adaptive Amitosis ability
			if (unit->orderTarget.unit != nullptr
				&& scbw::get_generic_value(unit, ValueId::AdaptiveAmitosis) > 0) {
				auto range = weapons_dat::MaxRange[weaponId];
				auto a = scbw::UnitFinder::getNearestTarget(
					unit->getX() - range, unit->getY() - range,
					unit->getX() + range, unit->getY() + range,
					unit, [&](CUnit* target) {
					if (target == unit) {
						return false;
					}
					if (target == unit->orderTarget.unit) {
						return false;
					}
					if (scbw::isAlliedTo(unit->playerId, target->playerId))
						return false;
					return scbw::canWeaponTargetUnit(weaponId, target, unit);
				}
				);
				if (a != nullptr) {
					auto old = unit->orderTarget;
					unit->orderTarget.unit = a;
					unit->orderTarget.pt = a->sprite->position;
					scbw::set_generic_value(unit->orderTarget.unit, ValueId::Amitosis, 1);
					// TODO: verify that this will work with all weapon types
					const uint32 Iscript_UseWeapon = 0x00479C30;
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
		if (scbw::get_generic_value(unit, ValueId::Deconstruction) > 0) {
			scbw::doDeconstructionDamage(unit, weaponId);
		}
	}

	void progressBulletFlight(CBullet* bullet) { //		
		CBullet* v1; // esi
		CUnit* target; // eax
		CSprite* v4; // edx
		CImage* i; // esi

		if (bullet->weaponType == WeaponId::AlkajeliskParasiticSpores) {
			if (bullet->attackTarget.unit == nullptr) {
				uint32 range = 8 * 32;//seek range
				auto seekTarget = scbw::UnitFinder::getNearestTarget(
					bullet->position.x - range,
					bullet->position.y - range,
					bullet->position.x + range,
					bullet->position.y + range,
					bullet,
					[&](CUnit* unit) {
					return alkajSporeMatch(bullet, unit);
				}
				);
				if (seekTarget != nullptr) {
					bullet->attackTarget.unit = seekTarget;
					bullet->attackTarget.pt = seekTarget->position;
					bullet->moveTarget.unit = seekTarget;
					bullet->moveTarget.pt = seekTarget->position;
					bullet->targetPosition = seekTarget->position;
					bullet->targetPosition2 = seekTarget->position;
					progressBulletMovement_func(bullet);
				}
			} else {
				progressBulletMovement_func(bullet);
			}
		} else {
			progressBulletMovement_func(bullet);
		}
		uint8 time = bullet->time_remaining--;
		if (bullet->weaponType == WeaponId::AlkajeliskParasiticSpores) {
			if (bullet->time_remaining < 135 && bullet->attackTarget.unit == nullptr) { //135 is 255 - 5*24
				time = 0;
			}
		}
		bool bEnd = false;

		// Accelerate missiles over time
		if (bullet->weaponType == WeaponId::EmpyreanPlasmaBarrage
			|| bullet->weaponType == WeaponId::AntiMatterMissiles) {
			uint8 time_current = 255 - time;
			if (time_current == 8) {
				bullet->flingyTopSpeed *= 2;
				bullet->flingyAcceleration *= 2;
			}
		}
		if (bullet->weaponType == WeaponId::ArchitectDispersiveSalvo && bullet->sourceUnit && bullet->attackTarget.unit &&
			bullet->attackTarget.unit->sprite && bullet->sourceUnit->sprite) {
			CUnit* targ = bullet->attackTarget.unit;
			if (scbw::getDistanceFast(bullet->position.x, bullet->position.y, targ->getX(), targ->getY()) < 3 * 32) {
				if (!globals.multihit_bullet_is_set(bullet)) {
					globals.add_multihit_empty_bullet(bullet, 1);
					auto old = bullet->sourceUnit->orderTarget.unit;
					auto oldSpritePos = bullet->sourceUnit->sprite->position;
					auto oldPos = bullet->sourceUnit->position;
					bullet->sourceUnit->sprite->position = bullet->position;
					bullet->sourceUnit->position = bullet->position;
					auto r = 3 * 32;
					scbw::UnitFinder unitsInSplash(bullet->position.x - r, bullet->position.y - r,
						bullet->position.x + r, bullet->position.y + r);
					int count = 0;
					for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
						CUnit* target = unitsInSplash.getUnit(i);
						if (alkajSporeMatch(bullet, target)) {
							if (target != nullptr) {
								count++;
							}
						}
					};
					for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
						CUnit* target = unitsInSplash.getUnit(i);
						if (alkajSporeMatch(bullet, target)) {
							if (target != nullptr) {
								bullet->sourceUnit->orderTarget.unit = target;
								auto newAngle = scbw::getAngle(bullet->position.x, bullet->position.y,
									target->getX(), target->getY());
								auto newBullet = scbw::createBulletRet(WeaponId::ArchitectDispersiveSalvo, bullet->sourceUnit,
									bullet->position.x, bullet->position.y, bullet->srcPlayer, newAngle);
								if (newBullet) {
									newBullet->position = bullet->position;
									globals.add_multihit_empty_bullet(newBullet, count + 1);
								}
							}
						}
					};
					bullet->sourceUnit->orderTarget.unit = old;
					bullet->sourceUnit->position = oldPos;
					bullet->sourceUnit->sprite->position = oldSpritePos;
				}
			}
		}
		if (time == 0 || bullet->moveTarget.pt.x == bullet->position.x && bullet->moveTarget.pt.y == bullet->position.y) {
			bEnd = true;
		}

		// Empyrean missile interception
		else if (bullet->weaponType == WeaponId::EmpyreanPlasmaBarrage) {
			uint32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](CUnit* unit) {
				if (!(unit->status & UnitStatus::InAir)) {
					return false;
				}
				if (unit->status & UnitStatus::Invincible) {
					return false;
				}
				if (scbw::isAlliedTo(bullet->srcPlayer, unit->playerId)) {
					return false;
				}
				auto box = units_dat::UnitBounds[unit->id];
				if (bullet->position.x < unit->getLeft()) {
					return false;
				}
				if (bullet->position.x > unit->getRight()) {
					return false;
				}
				if (bullet->position.y < unit->getTop()) {
					return false;
				}
				if (bullet->position.y > unit->getBottom()) {
					return false;
				}
				return true;
			}
			);
			if (collisionTarget != nullptr) {
				bullet->attackTarget.unit = collisionTarget;
				bEnd = true;
			}
		} else if (bullet->weaponType == WeaponId::StarSovereignInterventionMissiles) {
			uint32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](CUnit* unit) {
				if (unit->status & UnitStatus::Invincible) {
					return false;
				}
				if (scbw::isAlliedTo(bullet->srcPlayer, unit->playerId)) {
					return false;
				}
				auto box = units_dat::UnitBounds[unit->id];
				if (bullet->position.x < unit->getLeft()) {
					return false;
				}
				if (bullet->position.x > unit->getRight()) {
					return false;
				}
				if (bullet->position.y < unit->getTop()) {
					return false;
				}
				if (bullet->position.y > unit->getBottom()) {
					return false;
				}
				return true;
			}
			);
			if (collisionTarget != nullptr) {
				bullet->attackTarget.unit = collisionTarget;
				bEnd = true;
			}
		} else if (bullet->weaponType == WeaponId::KalkaliskViscousAcid || bullet->weaponType == WeaponId::PenumbraErebolShockCannon) {
			//for missiles hitting targets on its way.
			uint32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](CUnit* unit) {
				if (unit->status & UnitStatus::Invincible) {
					return false;
				}
				if (bullet->attackTarget.unit == unit) {
					return false;
				}
				if (scbw::isAlliedTo(bullet->srcPlayer, unit->playerId)) {
					return false;
				}
				auto box = units_dat::UnitBounds[unit->id];
				if (bullet->position.x < unit->getLeft()) {
					return false;
				}
				if (bullet->position.x > unit->getRight()) {
					return false;
				}
				if (bullet->position.y < unit->getTop()) {
					return false;
				}
				if (bullet->position.y > unit->getBottom()) {
					return false;
				}
				if (globals.was_already_hit_by_multihit_bullet(bullet, unit)) {
					return false;
				}

				return true;
			}
			);
			if (collisionTarget != nullptr) {
				if (bullet->weaponType == WeaponId::PenumbraErebolShockCannon) {
					globals.add_bullet_multihit(bullet, collisionTarget);
				} else {
					globals.add_incremental_bullet_multihit(bullet, collisionTarget);
				}
				if (bullet->weaponType == WeaponId::PenumbraErebolShockCannon) {
					//Penumbra Behavior
					scbw::createThingy(644, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					scbw::weaponBulletShot(collisionTarget, bullet, 2);
				} else {
					//Kalkaleth Behavior
//					auto divider = globals.get_multihit_count(bullet);
					scbw::weaponBulletShot(collisionTarget, bullet, 2);
					//					scbw::printf("Hit by bullet: %X", bullet);
					scbw::createThingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					bullet->moveTarget.unit = bullet->attackTarget.unit;
					if (bullet->moveTarget.unit != nullptr) {
						bullet->targetPosition = bullet->moveTarget.unit->position;
						bullet->targetPosition2 = bullet->moveTarget.unit->position;
					}
				}
			}
		}

		if (bEnd) {
			if (/*bullet->weaponType == WeaponId::ViscousAcid || */bullet->weaponType == WeaponId::ArchitectDispersiveSalvo) {
				globals.remove_multihit_bullet(bullet);
			}
			target = bullet->attackTarget.unit;
			bullet->behaviourTypeInternal = 5;

			if (target != nullptr) {
				bullet->attackTarget.pt.x = target->sprite->position.x;
				bullet->attackTarget.pt.y = target->sprite->position.y;
			} else {
				bullet->attackTarget.unit = 0;
			}
			v4 = bullet->sprite;
			bullet->orderSignal |= 0x80;//new
			bullet->unknown_0x4E = 0;
			bullet->someUnitType = 228;
			//			scbw::printf("Hit by bullet: %X", bullet);
			for (i = v4->images.head; i; i = i->link.next)
				playImageIscript(i, 1);


		}
	}

	void bulletBehaviour_Bounce(CBullet* a1) { // 0048B2D0
		CUnit* v1; // eax
		char v2; // al
		CUnit* v3; // ebx
		CImage* i; // esi
		CUnit* v5; // eax
		CSprite* v6; // edx
		CImage* j; // esi

		v1 = a1->attackTarget.unit;
		if (v1) {
			if (!(a1->hitFlags & 1)) {
				changeMovePos(a1, v1->sprite->position.y, v1->sprite->position.x);
			}
		}
		progressBulletMovement_func(a1);
		if (a1->moveTarget.pt.x == a1->position.x && a1->moveTarget.pt.y == a1->position.y) {
			v2 = a1->remainingBounces - 1;
			//
			if (a1->weaponType == WeaponId::GladiusPlasmaFlurry) {
				v2 = 1;
			}
			//
			a1->remainingBounces = v2;
			globals.bounce_bullet = a1;
			//			scbw::print("BounceHit");
			if (v2 && (v3 = findNextBounceTarget_func(a1), a1->nextBounceUnit = a1->attackTarget.unit, v3)) {
				for (i = a1->sprite->images.head; i; i = i->link.next) {
					playImageIscript(i, IscriptAnimation::SpecialState1);
				}
				//				scbw::print("Bounce");
				a1->attackTarget.unit = v3;
			} else {
				//				scbw::print("NoJump");
				if (a1->weaponType == WeaponId::GladiusPlasmaFlurry) {
					globals.remove_multihit_bullet(a1);
				}
				v5 = a1->attackTarget.unit;
				a1->behaviourTypeInternal = 5;
				if (v5) {
					a1->attackTarget.pt.x = v5->sprite->position.x;
					a1->attackTarget.pt.y = v5->sprite->position.y;
				} else {
					a1->attackTarget.unit = 0;
				}
				v6 = a1->sprite;
				a1->unknown_0x4E = 0; // Was lowbyte
				a1->someUnitType = UnitId::None;

				for (j = v6->images.head; j; j = j->link.next) {
					playImageIscript(j, IscriptAnimation::Death);
				}
			}
		}
	}

	bool findNextBounceTargetProc(CUnit* pIterator) { // 0048AFD0
		CUnit* pSrc; // esi
		int32 nSrcStatus; // eax
		CUnit* _pIterator; // edi
		int32 v4; // eax
		CUnit* v5; // ebx
		int32 v6; // eax
		int32 v7; // eax
		int32 v8; // eax
		bool v9; // eax
		int32 v10; // eax
		CUnit* v11; // ecx
		int32 v13; // eax
		int8 v14; // cl

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
		if (!pIterator) {
			return 0;
		}
		if (pIterator->status & UnitStatus::Invincible) {
			return 0;
		}
		if (pIterator->sprite->flags & CSprite_Flags::Hidden) {
			return 0;
		}
		if (unitCanSeeCloakedTarget(pIterator, (*bulletBounceMissileSource))) {
			return 0;
		}
		if (pSrc->subunit != nullptr
			&& unitIsSubunit(pSrc->subunit)) {
			v5 = pSrc->subunit;
		} else {
			v5 = pSrc;
		}
		if (!(unitIsCarrier(pSrc))) {
			if (unitIsReaver(pSrc)) {
				if (_pIterator->status & UnitStatus::InAir)
					return 0;
				v9 = unitHasPathToUnit(pSrc, _pIterator);
			} else {
				if (canInfest(pSrc)) {
					v9 = canBeInfested(v11, _pIterator) != 0; // v11 is not used..?
				} else {
					/*if (pSrc->id == UnitId::ProtossArbiter && pSrc->pAI) {

						return 0;
					}*/
					if (_pIterator->status & UnitStatus::InAir) {
						v9 = units_dat::AirWeapon[(uint16)v5->id] != -126;
					} else {
						v9 = units_dat::GroundWeapon[(uint16)v5->id] != -126;
					}
				}
			}
			if (!v9) {
				return 0;
			}
		}
		v13 = _pIterator->status;
		if (v13 & 0x300 && !((1 << pSrc->playerId) & _pIterator->visibilityStatus)) {
			return 0;
		}
		v14 = _pIterator->playerId == 11 ? _pIterator->sprite->playerId : _pIterator->playerId;
		if (playerAlliance[(uint8)pSrc->playerId].flags[v14] || *bulletBounceMissileTarget == _pIterator) {
			return 0;
		}

		// Gladius Binding Flurry behavior
//		scbw::printf("Gladius code start %d %d",(uint32)bulletBounceMissileNext,(uint32)pIterator);

		if (globals.bounce_bullet->weaponType == WeaponId::GladiusPlasmaFlurry) {
			if (pIterator != nullptr && *bulletBounceMissileNext != nullptr
				&& (*bulletBounceMissileNext)->sprite != nullptr
				&& _pIterator->sprite != nullptr) {
				if (globals.was_already_hit_by_multihit_bullet(globals.bounce_bullet, _pIterator)) {
					//scbw::print("NoItr - Already hit");
					return false;
				} else {
					//scbw::printf("Add hit %d",globals.bounce_bullet);
					globals.add_bullet_multihit(globals.bounce_bullet, _pIterator);
					return *bulletBounceMissileNext != _pIterator;

				}
			} else {
				//scbw::print("bad bullet");
			}
		}
		//		scbw::print("Gladius code end");
		return *bulletBounceMissileNext != _pIterator;
	}
	CUnit* findNextBounceTarget(CBullet* a1) { // 0048B1E0
		CUnit* v2; // edx
		CUnit* v3; // ecx
		CSprite* v4; // eax
		uint16 v5;
		uint16 v6;
		Box32 _a1; // [esp+0h] [ebp-8h]

		if (!a1->sourceUnit) {
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
		_a1.left = (int16)v6 - 96;
		_a1.top = (int16)v6 + 96;
		_a1.right = (int16)v5 - 96;
		_a1.bottom = (int16)v5 + 96;
		//		return (CUnit*)iterateUnitsAtLocationTargetProc(&_a1, (int(__fastcall*)(int32, int32))findNextBounceTargetProc, 0);
		GPTP::logger << (uint32)(*bulletBounceMissileNext) << " " << (uint32)(*bulletBounceMissileSource) << " " << (uint32)(*bulletBounceMissileTarget) << '\n';
		return (CUnit*)iterateUnitsAtLocationTargetProc(&_a1, (int(__fastcall*)(int32, int32))findNextBounceTargetProc, 0);
	}

	/*
	int writeBullets(FILE* hFile) {
		int32 v1; // edi
		CBullet* v2; // esi
		CBullet* v3; // eax
		int32 v4; // edx
		int32 v5; // ecx
		int32 v6; // esi
		size_t v7; // eax
		int16* v8; // eax
		uint16 v9; // dx
		CBullet* v10; // ebx
		void* v11; // edi
		uint32 v12; // esi
		int16* v13; // eax
		CBullet* v14; // ecx
		int32 v15; // edi
		CBullet* v16; // ecx
		int32 v18; // [esp+8h] [ebp-Ch]
		int16* v19; // [esp+Ch] [ebp-8h]
		int32 data; // [esp+10h] [ebp-4h]

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
			if ((int16)data) {
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

} //hooks


//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	const uint32 Helper_CreateBullet = 0x0048C260;
	void createBullet(uint8 weaponId, CUnit* source, int16 x, int16 y, uint8 attackingPlayer, uint8 direction) {

		static uint32 attackingPlayer_;
		static uint32 direction_;
		static int32 x_;
		static int32 y_;

		attackingPlayer_ = attackingPlayer;
		direction_ = direction;
		x_ = x; y_ = y;
		if (weaponId == WeaponId::PaladinCrusaderMissiles) {
			if (source->sprite->mainGraphic->direction == 0
				|| source->sprite->mainGraphic->direction == 1
				|| source->sprite->mainGraphic->direction == 2
				|| source->sprite->mainGraphic->direction == 3) {
				x_ = source->sprite->position.x + 6;
				y_ = source->sprite->position.y - 45;
			} else if (source->sprite->mainGraphic->direction == 4
				|| source->sprite->mainGraphic->direction == 5) {
				x_ = source->sprite->position.x + 4;
				y_ = source->sprite->position.y - 44;
			} else if (source->sprite->mainGraphic->direction == 6
				|| source->sprite->mainGraphic->direction == 7
				|| source->sprite->mainGraphic->direction == 8
				|| source->sprite->mainGraphic->direction == 9) {
				x_ = source->sprite->position.x + 1;
				y_ = source->sprite->position.y - 43;
			} else if (source->sprite->mainGraphic->direction == 10
				|| source->sprite->mainGraphic->direction == 11) {
				x_ = source->sprite->position.x - 3;
				y_ = source->sprite->position.y - 42;
			} else if (source->sprite->mainGraphic->direction == 12
				|| source->sprite->mainGraphic->direction == 13) {
				x_ = source->sprite->position.x - 6;
				y_ = source->sprite->position.y - 41;
			} else if (source->sprite->mainGraphic->direction == 14
				|| source->sprite->mainGraphic->direction == 15) {
				x_ = source->sprite->position.x - 8;
				y_ = source->sprite->position.y - 44;
			} else if (source->sprite->mainGraphic->direction == 16) {
				x_ = source->sprite->position.x - 8;
				y_ = source->sprite->position.y - 48;
			}
		}
		__asm {
			PUSHAD
			PUSH direction_
			PUSH attackingPlayer_
			PUSH y_
			PUSH x_
			MOV EAX, source
			MOVZX ECX, weaponId
			CALL Helper_CreateBullet
			POPAD
		}

	}

	// For progressBulletMovement

	const uint32 Func_progressBulletMovement = 0x0048B250;
	void progressBulletMovement_func(CBullet* a1) {
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_progressBulletMovement
			POPAD
		}
	}

	const uint32 Func_playImageIscript = 0x004D8470;
	void playImageIscript(CImage* a1, char a2) {
		__asm {
			PUSHAD
			MOV ECX, a1
			PUSH a2
			CALL Func_playImageIscript
			POPAD
		}
	}

	// For bulletBehavior_Bounce

	const uint32 Func_changeMovePos = 0x00495240;
	CBullet* changeMovePos(CBullet* a1, int32 a2, int32 a3) {
		static CBullet* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			MOV ESI, a3
			CALL Func_changeMovePos
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_findNextBounceTarget = 0x0048B1E0;
	CUnit* findNextBounceTarget_func(CBullet* a1) {
		static CUnit* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_findNextBounceTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_unitCanSeeCloakedTarget = 0x00401D60;
	bool unitCanSeeCloakedTarget(CUnit* a1, CUnit* a2) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH a2
			MOV EAX, a1
			CALL Func_unitCanSeeCloakedTarget
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_unitIsSubunit = 0x00401D40;
	bool unitIsSubunit(CUnit* a1) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsSubunit
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_unitIsCarrier = 0x00401470;
	bool unitIsCarrier(CUnit* a1) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsCarrier
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_unitIsReaver = 0x00401490;
	bool unitIsReaver(CUnit* a1) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsReaver
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_unitHasPathToUnit = 0x0049CBB0;
	bool unitHasPathToUnit(CUnit* a1, CUnit* a2) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_unitHasPathToUnit
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_canInfest = 0x00402750;
	bool canInfest(CUnit* a1) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_canInfest
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_canBeInfested = 0x00402210;
	int32 canBeInfested(CUnit* a1, CUnit* a2) {
		static int32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL Func_canBeInfested
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For findNextBounceTarget

	const uint32 Func_iterateUnitsAtLocationTargetProc = 0x004E8280;
	int32 iterateUnitsAtLocationTargetProc(Box32* a1, int32(__fastcall* a2)(int32, int32), CUnit* a3) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EBX, a2
			MOV EAX, a1
			CALL Func_iterateUnitsAtLocationTargetProc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	//for progressBullet
	const uint32 func_doMissileDmg = 0x0048B770;
	void doMissileDmg(CBullet* bullet) {
		__asm {
			PUSHAD
			PUSH bullet
			CALL func_doMissileDmg
			POPAD
		}
	};

} //unnamed namespace
