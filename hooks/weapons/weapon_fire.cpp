//Source file for the Weapon Fire hook module.
//This hook controls how weapons are fired.
#include "weapon_fire.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "SCBW/UnitFinder.h"
#include "MathUtils.h"
#include "hooks/byte_operations_utils.h"
#include "globals.h"
//-------- Helper function declarations. Do NOT modify! ---------//


class tendrilMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CUnit* sunken;
public:
	tendrilMatch(CUnit* sunken)
		: sunken(sunken) {}

	bool match(CUnit* unit) {
		if (unit == sunken) {
			return false;
		}
		if (unit == sunken->orderTarget.unit) {
			return false;
		}
		if (scbw::isAlliedTo(sunken->playerId, unit->playerId))
			return false;
		return scbw::canWeaponTargetUnit(WeaponId::SubterraneanTentacle, unit, sunken);
	}
};

class madrigalFuriaMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CUnit* madrigal;
public:
	madrigalFuriaMatch(CUnit* madrigal)
		: madrigal(madrigal) {}

	bool match(CUnit* unit) {
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
};

class empyreanMissileMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CBullet* missile;
public:
	empyreanMissileMatch(CBullet* missile)
		: missile(missile) {}

	bool match(CUnit* unit) {
		if (!(unit->status & UnitStatus::InAir)) {
			return false;
		}
		if (unit->status & UnitStatus::Invincible) {
			return false;
		}
		if (scbw::isAlliedTo(missile->srcPlayer, unit->playerId)) {
			return false;
		}
		auto box = units_dat::UnitBounds[unit->id];
		if (missile->position.x < unit->getLeft()) {
			return false;
		}
		if (missile->position.x > unit->getRight()) {
			return false;
		}
		if (missile->position.y < unit->getTop()) {
			return false;
		}
		if (missile->position.y > unit->getBottom()) {
			return false;
		}
		return true;
	}
};

class starsovereignMissileMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CBullet* missile;
public:
	starsovereignMissileMatch(CBullet* missile)
		: missile(missile) {}

	bool match(CUnit* unit) {
		if (unit->status & UnitStatus::Invincible) {
			return false;
		}
		if (scbw::isAlliedTo(missile->srcPlayer, unit->playerId)) {
			return false;
		}
		auto box = units_dat::UnitBounds[unit->id];
		if (missile->position.x < unit->getLeft()) {
			return false;
		}
		if (missile->position.x > unit->getRight()) {
			return false;
		}
		if (missile->position.y < unit->getTop()) {
			return false;
		}
		if (missile->position.y > unit->getBottom()) {
			return false;
		}
		return true;
	}
};

class missilePassMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CBullet* missile;
public:
	missilePassMatch(CBullet* missile)
		: missile(missile) {}

	bool match(CUnit* unit) {
		if (unit->status & UnitStatus::Invincible) {
			return false;
		}
		if (missile->attackTarget.unit == unit) {
			return false;
		}
		if (scbw::isAlliedTo(missile->srcPlayer, unit->playerId)) {
			return false;
		}
		auto box = units_dat::UnitBounds[unit->id];
		if (missile->position.x < unit->getLeft()) {
			return false;
		}
		if (missile->position.x > unit->getRight()) {
			return false;
		}
		if (missile->position.y < unit->getTop()) {
			return false;
		}
		if (missile->position.y > unit->getBottom()) {
			return false;
		}
		if (Globals.was_already_hit_by_multihit_bullet(missile, unit)) {
			return false;
		}

		return true;
	}
};
class architectBulletMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CBullet* missile;
public:
	architectBulletMatch(CBullet* missile)
		: missile(missile) {}

	bool match(CUnit* unit) {
		if (unit->status & UnitStatus::Invincible) {
			return false;
		}
		if (missile->attackTarget.unit == unit) {
			return false;
		}
		if (scbw::isAlliedTo(missile->srcPlayer, unit->playerId)) {
			return false;
		}
		return true;
	}
};

class alkajSporeMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CBullet* missile;
public:
	alkajSporeMatch(CBullet* missile)
		: missile(missile) {}

	bool match(CUnit* unit) {
		if (unit->status & UnitStatus::Invincible) {
			return false;
		}
		if (scbw::isAlliedTo(missile->srcPlayer, unit->playerId)) {
			return false;
		}
		return true;
	}
};


class powerSiphonCollectProc : public scbw::UnitFinderCallbackProcInterface {
private:
	CUnit* savant;
	u32& energyCollected;
	
public:
	powerSiphonCollectProc(CUnit* savant, u32& energyCollected)
		: savant(savant), energyCollected(energyCollected) {}

	void proc(CUnit* unit) {
		constexpr u32 ENERGY_PROC_MAX = 50 * 256;
		if (energyCollected == ENERGY_PROC_MAX) {
			return;
		}
		if (unit == savant) {
			return;
		}
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster)) {
			return;
		}
		if (unit->energy == 0) {
			return;
		}
		u32 energyLeft = ENERGY_PROC_MAX - energyCollected;
		u32 drain = MathUtils::clamp<u32>(unit->energy, 0, energyLeft);
		energyCollected += drain;
		unit->spendUnitEnergy(drain);
		u8 overlaySize = scbw::hasOverlay(unit);
		if (unit->subunit != NULL)
			unit->subunit->sprite->createTopOverlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);
		else
			unit->sprite->createTopOverlay(overlaySize + ImageId::Feedback_Small, 0, 0, 0);
	}
};

class powerSiphonDistributeMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CUnit* savant;
public:
	powerSiphonDistributeMatch(CUnit* savant)
		: savant(savant) {}

	bool match(CUnit* unit) {
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
};

namespace { // Helper functions
	typedef void(__stdcall* GetWeaponHitPosFunc)(CUnit* unit, s32* x, s32* y);
	GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc)0x004762C0;
	void createBullet(u8 weaponId, CUnit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction);

	// For progressBulletFlight
	void progressBulletMovement(CBullet* a1); // 0048B250
	void playImageIscript(CImage* a1, char a2); // 004D8470
	void doMissileDmg(CBullet * bullet);  //0048B770

	// For bulletBehavior_Bounce
	CBullet* changeMovePos(CBullet* a1, s32 a2, s32 a3); // 00495240
	void progressBulletMovement_func(CBullet* a1); // 0048B250
	CUnit* findNextBounceTarget_func(CBullet* a1); // 0048B1E0

	// For findNextBounceTargetProc
	bool unitCanSeeCloakedTarget(CUnit* a1, CUnit* a2); // 00401D60
	bool unitIsSubunit(CUnit* a1); // 00401D40
	bool unitIsCarrier(CUnit* a1); // 00401470
	bool unitIsReaver(CUnit* a1); // 00401490
	bool unitHasPathToUnit(CUnit* a1, CUnit* a2); // 0049CBB0
	bool canInfest(CUnit* a1); // 00402750
	s32 canBeInfested(CUnit* a1, CUnit* a2); // 00402210

	// For findNextBounceTarget
	s32 iterateUnitsAtLocationTargetProc(Box32* a1, s32 (__fastcall *a2)(s32, s32), CUnit* a3); // 004E8280

	// For writeBullets
/*	void packBulletData(CBullet* a1, s32 a2); // 0048A9A0
	size_t _lockwrite(const void* a1, size_t a2, size_t a3, FILE* a4); // 00411931
	void* SMemAlloc(s32 a1, char* a2, s32 a3, s32 a4); // 0041006A
	s32 compressWrite(void* a1, u32 a2, FILE* a3); // 004C3450
	bool SMemFree(void* a1, char* a2, s32 a3, char a4); // 00410070
	CBullet* unpackBulletData(CBullet* a1, s32 a2); // 0048A8D0	*/

} //unnamed namespace


//-------- Actual hook functions --------//

namespace hooks {

	//Fires a weapon from the @p unit.
	//This hook affects the following iscript opcodes: attackwith, attack, castspell
	//This also affects CUnit::fireWeapon().
	void fireWeaponHook(CUnit* unit, u8 wId) {
		//Default StarCraft behavior
		u8 weaponId = wId;
		//Retrieve the spawning position for the bullet.
		s32 x, y;
		u8 behavior = weapons_dat::Behavior[weaponId];
		if (weaponId == WeaponId::PowerSiphon || weaponId==WeaponId::EnergyDecay) {
			behavior = WeaponBehavior::AppearOnTargetSite;
		}
		if (behavior == WeaponBehavior::AppearOnTargetUnit) {
			if (unit->orderTarget.unit == NULL) {
				return;
			}
			getWeaponHitPos(unit, &x, &y);

		}
		else if (behavior == WeaponBehavior::AppearOnTargetSite) {
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
		if (weapons_dat::FlingyId[weaponId] != 0) {
			bool good = true;

			// Wyvern Land Grab ability
			if (unit->id == UnitId::TerranWyvern
			&& unit->sprite->mainGraphic->animation == IscriptAnimation::CastSpell
			&& scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::WyvernReverseLandgrabOn, 0) == 1) {
				good = false;
			}

			// Lurker Seismic Spines behavior
			if (weaponId == WeaponId::SubterraneanSpines) {
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::LurkerETA, 17, 0);
				auto range = weapons_dat::MaxRange[weaponId] + 16;
				int angle = scbw::getAngle(unit->getX(), unit->getY(), unit->orderTarget.unit->getX(), unit->orderTarget.unit->getY());
				int targ_x = unit->getX() + scbw::getPolarX(range, angle);
				int targ_y = unit->getY() + scbw::getPolarY(range, angle);
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::SpineX, targ_x, 0);
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::SpineY, targ_y, 0);
			}

			if (good) {

				// Madrigal Tempo Change weapon switch
				if (weaponId == WeaponId::FuriaMissiles
				&& scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::TempoChange,0)==1) {
					weaponId = WeaponId::ImperiosoMissiles;
				}

				// Savant Power Siphon ability
				if (weaponId == WeaponId::PowerSiphon) {
					u32 energy = 0;
					powerSiphonCollectProc collectProc = powerSiphonCollectProc(unit,energy);
					u32 range = weapons_dat::OuterSplashRadius[WeaponId::PowerSiphon];
					scbw::UnitFinder unitFinder_collect(x - range,
						y - range,
						x + range,
						y + range);
					unitFinder_collect.forEach(collectProc);
					range = 4 * 32;
					powerSiphonDistributeMatch distrMatch = powerSiphonDistributeMatch(unit);
					scbw::UnitFinder unitFinder_distribute(unit->getX() - range,
						unit->getY() - range,
						unit->getX() + range,
						unit->getY() + range);
					u32 matchCount = unitFinder_distribute.getMatchUnitCount(distrMatch);
					if (matchCount != 0 && energy>0) {
						u32 perEach = energy / matchCount;
						u32 extra = energy % matchCount;
						unitFinder_distribute.getFirst(distrMatch)->addUnitEnergy(extra);
						u32 unspent = 0;
						unitFinder_distribute.forEachInMatch(distrMatch, [&](CUnit* u) {
							u->addUnitEnergy(unspent);
							energy -= unspent;
							unspent = 0;
							u32 howMuch = MathUtils::clamp<u32>(perEach, 0, u->getMaxEnergy() - u->energy);
							u->energy += howMuch;
							energy -= howMuch;
							u8 overlaySize = scbw::hasOverlay(u);
							if (u->subunit != NULL)
								u->subunit->sprite->createTopOverlay(overlaySize + ImageId::MindControlHit_Small, 0, 0, 0);
							else
								u->sprite->createTopOverlay(overlaySize + ImageId::MindControlHit_Small, 0, 0, 0);
							unspent = perEach - howMuch;
						});
						if (energy > 0) {
							unitFinder_distribute.getFirst(distrMatch)->addUnitEnergy(energy);
						}
					}
				}
				
				if (weaponId == WeaponId::PlasmaBarrage) {
					// Empyrean missile spawn positions
					scbw::setMissileSourcePos(unit, weaponId, ValueId::EmpyreanMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::SunchaserMissiles) {
					// Centaur missile spawn positions
					scbw::setMissileSourcePos(unit, weaponId, ValueId::CentaurMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::ArgosLaserBatteries) {
					// Phobos - Argent Capacitors passive
					if (unit->id == UnitId::TerranPhobos && scbw::get_generic_value(unit, ValueId::PhobosDischarge) == 0) {
						unit->addUnitEnergy(5 * 256);
						if (unit->energy == unit->getMaxEnergy()) {
							scbw::set_generic_value(unit, ValueId::PhobosDischarge, 1);
						}
					}
					// Phobos laser spawn positions
					scbw::setMissileSourcePos(unit, weaponId, ValueId::PhobosLaserPos, 4, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::AcheronMissiles) {
					// Phobos - Argent Capacitors behavior
					if (unit->id == UnitId::TerranPhobos && scbw::get_generic_value(unit, ValueId::PhobosDischarge) == 0) {
						unit->addUnitEnergy(5 * 256);
						if (unit->energy == unit->getMaxEnergy()) {
							scbw::set_generic_value(unit, ValueId::PhobosDischarge, 1);
						}
					}
					// Phobos missile spawn positions
					scbw::setMissileSourcePos(unit, weaponId, ValueId::PhobosMissilePos, 6, ImagesDatEntries::OverlaySpecialPtr);
				}
				else if (weaponId == WeaponId::ViscousAcid) {
					// Kalkaleth missile spawn position
					auto bullet = scbw::setMissileSourcePosRet(unit, weaponId, ValueId::KalkalethMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					Globals.remove_multihit_bullet(bullet);
				}
				else if (weaponId == WeaponId::ScreamingVenom) {
					// Keskathalor missile spawn position
					scbw::setMissileSourcePos(unit, weaponId, ValueId::KeskathalorMissilePos, 2, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::RetractorBatteries) {
					// Pegasus laser spawn position
					scbw::setMissileSourcePos(unit, weaponId, ValueId::PegasusLaserPos, 2, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::LancetMissiles) {
					// Pegasus missile spawn position
					scbw::setMissileSourcePos(unit, weaponId, ValueId::PegasusMissilePos, 4, ImagesDatEntries::OverlaySpecialPtr);
				}
				else if (weaponId == WeaponId::CelestialCannon) {
					// Star Sovereign laser spawn position
					scbw::setMissileSourcePos(unit, weaponId, ValueId::SovereignCannonPos, 2, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::RetractorBatteries) {
					// Penumbra laser spawn position
					scbw::setMissileSourcePos(unit, weaponId, ValueId::PenumbraLaserPos, 1, ImagesDatEntries::OverlayAttackPtr);
				}
				else if (weaponId == WeaponId::PerditionBarrage) {
					// Salamander missile target positions
					auto rotate = scbw::get_generic_value(unit, ValueId::SalamanderAtkPos);
					if (rotate == 1) {
						createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
					}
					else {
						scbw::rangeModifiedFire(weaponId, unit, x, y, 32-32*rotate);
					}
					rotate++;
					if (rotate > 2) {
						rotate = 0;
					}
					scbw::set_generic_value(unit, ValueId::SalamanderAtkPos, rotate);
				}
				else if (weaponId == WeaponId::FuriaMissiles && unit->orderTarget.unit != NULL) {
					scbw::createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
					auto old = unit->orderTarget.unit;
					auto finderMatch = madrigalFuriaMatch(unit);
					auto range = weapons_dat::MaxRange[weaponId];
					auto count = 0;
					scbw::UnitFinder unitsInSplash(unit->getX() - range, unit->getY() - range, unit->getX() + range, unit->getY() + range);
					for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
						CUnit* target = unitsInSplash.getUnit(i);
						if (finderMatch.match(target)) {
							count++;
							if (target != NULL) {
								unit->orderTarget.unit = target;
								auto newAngle = scbw::getAngle(unit->getX(), unit->getY(),
									target->getX(), target->getY());
								scbw::createBullet(weaponId, unit, unit->getX(), unit->getY(), unit->playerId, newAngle);
							}
						}
					};
					unit->orderTarget.unit = old;
				}
				else {
					// Madcap - Maverick Feeders passive
					if (unit->id == UnitId::TerranMadcap) {
						scbw::add_generic_value(unit, ValueId::MadcapStacks, 1);
					}
					if (weaponId == WeaponId::AcidSpore && unit->sprite) {
						scbw::set_generic_value(unit, ValueId::GuardianMissileLaunchPosX, unit->getX());
						scbw::set_generic_value(unit, ValueId::GuardianMissileLaunchPosY, unit->getY());
					}
					createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
				}				
			}

			// Tendril Amitosis
			/*if (weaponId == WeaponId::SubterraneanTentacle && unit->orderTarget.unit != NULL) {
				auto range = weapons_dat::MaxRange[WeaponId::SubterraneanTentacle];
				auto a = scbw::UnitFinder::getNearestTarget(
					unit->getX() - range, unit->getY() - range,
					unit->getX() + range, unit->getY() + range,
					unit, tendrilMatch(unit)
				);
				if (a != NULL) {
					auto old = unit->orderTarget;
					unit->orderTarget.unit = a;
					unit->orderTarget.pt = a->sprite->position;
					//						createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
					scbw::sendGPTP_aise_cmd(unit->orderTarget.unit, NULL, GptpId::SetGenericValue, ValueId::Amitosis, 1, 0);
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
			}/**/
		}
	}

	void progressBulletFlight(CBullet* bullet) { //		
		CBullet* v1; // esi
		CUnit* target; // eax
		CSprite* v4; // edx
		CImage* i; // esi
		
		if (bullet->weaponType == WeaponId::ParasiticSpores) {
			if (bullet->attackTarget.unit == NULL) {
				u32 range = 8*32;//seek range
				auto seekTarget = scbw::UnitFinder::getNearestTarget(
					bullet->position.x - range, bullet->position.y - range,
					bullet->position.x + range, bullet->position.y + range,
					bullet, alkajSporeMatch(bullet)
				);
				if (seekTarget != NULL) {
					bullet->attackTarget.unit = seekTarget;
					bullet->attackTarget.pt = seekTarget->position;
					bullet->moveTarget.unit = seekTarget;
					bullet->moveTarget.pt = seekTarget->position;
					bullet->targetPosition = seekTarget->position;
					bullet->targetPosition2 = seekTarget->position;
					progressBulletMovement_func(bullet);
				}
			}
			else {
				progressBulletMovement_func(bullet);
			}
		}
		else {
			progressBulletMovement_func(bullet);
		}
		u8 time = bullet->time_remaining--;
		if (bullet->weaponType == WeaponId::ParasiticSpores) {
			if (bullet->time_remaining < 135 && bullet->attackTarget.unit==NULL) { //135 is 255 - 5*24
				time = 0;
			}
		}
		bool bEnd = false;
		
		// Accelerate missiles over time
		if (bullet->weaponType == WeaponId::PlasmaBarrage
		|| bullet->weaponType == WeaponId::AntiMatterMissiles
		||	bullet->weaponType == WeaponId::AcheronMissiles) {
			u8 time_current = 255 - time;
			if (time_current == 8) {
				bullet->flingyTopSpeed *= 2;
				bullet->flingyAcceleration *= 2;
			}
		}
		if (bullet->weaponType == WeaponId::DispersiveSalvo && bullet->sourceUnit && bullet->attackTarget.unit && 
				bullet->attackTarget.unit->sprite && bullet->sourceUnit->sprite) {
			CUnit* targ = bullet->attackTarget.unit;
			if(scbw::getDistanceFast(bullet->position.x,bullet->position.y,targ->getX(),targ->getY())<3*32){
				if (!Globals.multihit_bullet_is_set(bullet)) {
					Globals.add_multihit_empty_bullet(bullet,1);
					auto finderMatch = architectBulletMatch(bullet);
					auto old = bullet->sourceUnit->orderTarget.unit;
					auto oldSpritePos = bullet->sourceUnit->sprite->position;
					auto oldPos = bullet->sourceUnit->position;
					bullet->sourceUnit->sprite->position = bullet->position;
					bullet->sourceUnit->position = bullet->position;
					auto r = 3*32;
					scbw::UnitFinder unitsInSplash(bullet->position.x - r, bullet->position.y - r,
											    	bullet->position.x + r, bullet->position.y + r);
					int count=0;
					for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
						CUnit* target = unitsInSplash.getUnit(i);
						if (finderMatch.match(target)) {
							if (target != NULL) {
								count++;
							}
						}
					};
					for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
						CUnit* target = unitsInSplash.getUnit(i);
						if (finderMatch.match(target)) {
							if (target != NULL) {
								bullet->sourceUnit->orderTarget.unit = target;
								auto newAngle = scbw::getAngle(bullet->position.x, bullet->position.y,
									target->getX(), target->getY());
								auto newBullet = scbw::createBulletRet(WeaponId::DispersiveSalvo, bullet->sourceUnit,
									bullet->position.x, bullet->position.y, bullet->srcPlayer, newAngle);
								newBullet->position = bullet->position;
								Globals.add_multihit_empty_bullet(newBullet,count+1);
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
		else if (bullet->weaponType == WeaponId::PlasmaBarrage) {
			u32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, empyreanMissileMatch(bullet)
			);
			if (collisionTarget != NULL) {
				bullet->attackTarget.unit = collisionTarget;
				bEnd = true;
			}
		}
		else if (bullet->weaponType == WeaponId::InterventionMissiles) {
			u32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, starsovereignMissileMatch(bullet)
			);
			if (collisionTarget != NULL) {
				bullet->attackTarget.unit = collisionTarget;
				bEnd = true;
			}
		}
		else if (bullet->weaponType == WeaponId::ViscousAcid || bullet->weaponType==WeaponId::ErebolShockCannon) {
			//for missiles hitting targets on its way.
			u32 range = 128;
			auto collisionTarget = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, missilePassMatch(bullet)
			);
			if (collisionTarget != NULL) {
				if (bullet->weaponType == WeaponId::ErebolShockCannon) {
					Globals.add_bullet_multihit(bullet, collisionTarget);
				}
				else {
					Globals.add_incremental_bullet_multihit(bullet, collisionTarget);
				}
				if (bullet->weaponType == WeaponId::ErebolShockCannon) {
					//Penumbra Behavior
					scbw::createThingy(644, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					scbw::weaponBulletShot(collisionTarget, bullet, 2);
				}
				else {
					//Kalkaleth Behavior
					auto divider = Globals.get_multihit_count(bullet);
					scbw::weaponBulletShot(collisionTarget, bullet, divider);
//					scbw::printFormattedText("Hit by bullet: %X", bullet);
					scbw::createThingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					bullet->moveTarget.unit = bullet->attackTarget.unit;
					if (bullet->moveTarget.unit != NULL) {
						bullet->targetPosition = bullet->moveTarget.unit->position;
						bullet->targetPosition2 = bullet->moveTarget.unit->position;
					}
				}
			}
		}

		if (bEnd) {
			if (/*bullet->weaponType == WeaponId::ViscousAcid || */bullet->weaponType == WeaponId::DispersiveSalvo) {
				Globals.remove_multihit_bullet(bullet);
			}
			target = bullet->attackTarget.unit;
			bullet->behaviourTypeInternal = 5;

			if (target != NULL)	{
				bullet->attackTarget.pt.x = target->sprite->position.x;
				bullet->attackTarget.pt.y = target->sprite->position.y;
			}
			else {
				bullet->attackTarget.unit = 0;
			}
			v4 = bullet->sprite;
			bullet->orderSignal |= 0x80;//new
			bullet->unknown_0x4E = 0;
			bullet->someUnitType = 228;
//			scbw::printFormattedText("Hit by bullet: %X", bullet);
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
		if (v1)	{
			if (!(a1->hitFlags & 1)) {
				changeMovePos(a1, v1->sprite->position.y, v1->sprite->position.x);
			}
		}
		progressBulletMovement_func(a1);
		if (a1->moveTarget.pt.x == a1->position.x && a1->moveTarget.pt.y == a1->position.y)	{
			v2 = a1->remainingBounces - 1;
			//
			if (a1->weaponType == WeaponId::PlasmaFlurry) {
				v2 = 1;
			}
			//
			a1->remainingBounces = v2;
			Globals.bounce_bullet = a1;
//			scbw::printText("BounceHit");
			if (v2 && (v3 = findNextBounceTarget_func(a1), a1->nextBounceUnit = a1->attackTarget.unit, v3)) {
				for (i = a1->sprite->images.head; i; i = i->link.next) {
					playImageIscript(i, IscriptAnimation::SpecialState1);
				}
//				scbw::printText("Bounce");
				a1->attackTarget.unit = v3;
			}
			else {
//				scbw::printText("NoJump");
				if (a1->weaponType == WeaponId::PlasmaFlurry) {
					Globals.remove_multihit_bullet(a1);
				}
				v5 = a1->attackTarget.unit;
				a1->behaviourTypeInternal = 5;
				if (v5) {
					a1->attackTarget.pt.x = v5->sprite->position.x;
					a1->attackTarget.pt.y = v5->sprite->position.y;
				}
				else {
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
			scbw::printText("Source unit is disabled (doodad state)");
			return 0;
		}
		if ((*bulletBounceMissileSource)->lockdownTimer) {
			scbw::printText("Source unit is disabled (lockdown)");
			return 0;
		}
		if ((*bulletBounceMissileSource)->stasisTimer) {
			scbw::printText("Source unit is disabled (stasis)");
			return 0;
		}
		if ((*bulletBounceMissileSource)->maelstromTimer) {
			scbw::printText("Source unit is disabled (maelstrom)");
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
		if (pSrc->subunit != NULL
		&&	unitIsSubunit(pSrc->subunit)) {
			v5 = pSrc->subunit;
		}
		else {
			v5 = pSrc;
		}	
		if (!(unitIsCarrier(pSrc))) {
			if (unitIsReaver(pSrc))	{
				if (_pIterator->status & UnitStatus::InAir)
					return 0;
				v9 = unitHasPathToUnit(pSrc, _pIterator);
			}
			else {
				if (canInfest(pSrc)) {
					v9 = canBeInfested(v11, _pIterator) != 0; // v11 is not used..?
				}
				else {
					/*if (pSrc->id == UnitId::ProtossArbiter && pSrc->pAI) {
					
						return 0;
					}*/
					if (_pIterator->status & UnitStatus::InAir) {
						v9 = units_dat::AirWeapon[(u16)v5->id] != -126;
					}
					else {
						v9 = units_dat::GroundWeapon[(u16)v5->id] != -126;
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
		if (playerAlliance[(u8)pSrc->playerId].flags[v14] || *bulletBounceMissileTarget == _pIterator) {
			return 0;
		}
		
		// Gladius Binding Flurry behavior
//		scbw::printFormattedText("Gladius code start %d %d",(u32)bulletBounceMissileNext,(u32)pIterator);

		if (Globals.bounce_bullet->weaponType == WeaponId::PlasmaFlurry){
			if (pIterator != NULL && *bulletBounceMissileNext != NULL
				&& (*bulletBounceMissileNext)->sprite != NULL
				&& _pIterator->sprite != NULL) {
				if (Globals.was_already_hit_by_multihit_bullet(Globals.bounce_bullet, _pIterator)) {
					//scbw::printText("NoItr - Already hit");
					return false;
				}
				else {

					//scbw::printFormattedText("Add hit %d",Globals.bounce_bullet);
					Globals.add_bullet_multihit(Globals.bounce_bullet, _pIterator);
					return *bulletBounceMissileNext != _pIterator;
						
				}
			}
			else {
				//scbw::printText("bad bullet");
			}
		}
//		scbw::printText("Gladius code end");
		return *bulletBounceMissileNext != _pIterator;	
	}
	CUnit* findNextBounceTarget(CBullet* a1) { // 0048B1E0
		CUnit* v2; // edx
		CUnit* v3; // ecx
		CSprite* v4; // eax
		u16 v5;
		u16 v6;
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
		_a1.left = (s16)v6 - 96;
		_a1.top = (s16)v6 + 96;
		_a1.right = (s16)v5 - 96;
		_a1.bottom = (s16)v5 + 96;
//		return (CUnit*)iterateUnitsAtLocationTargetProc(&_a1, (int(__fastcall*)(s32, s32))findNextBounceTargetProc, 0);
		GPTP::logger << (u32)(*bulletBounceMissileNext) << " " << (u32)(*bulletBounceMissileSource) << " " << (u32)(*bulletBounceMissileTarget) << '\n';
		return (CUnit*)iterateUnitsAtLocationTargetProc(&_a1, (int(__fastcall*)(s32, s32))findNextBounceTargetProc, 0);
	}

	/*
	int writeBullets(FILE* hFile) {
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

} //hooks


//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	const u32 Helper_CreateBullet = 0x0048C260;
	void createBullet(u8 weaponId, CUnit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction) {

		static u32 attackingPlayer_;
		static u32 direction_;
		static s32 x_;
		static s32 y_;

		attackingPlayer_ = attackingPlayer;
		direction_ = direction;
		x_ = x; y_ = y;
		if (weaponId == WeaponId::CrusaderMissiles) {
			if (source->sprite->mainGraphic->direction == 0
			|| source->sprite->mainGraphic->direction == 1
			|| source->sprite->mainGraphic->direction == 2
			|| source->sprite->mainGraphic->direction == 3) {
				x_ = source->sprite->position.x + 6;
				y_ = source->sprite->position.y - 45;
			}
			else if (source->sprite->mainGraphic->direction == 4
			|| source->sprite->mainGraphic->direction == 5) {
				x_ = source->sprite->position.x + 4;
				y_ = source->sprite->position.y - 44;
			}
			else if (source->sprite->mainGraphic->direction == 6
			|| source->sprite->mainGraphic->direction == 7
			|| source->sprite->mainGraphic->direction == 8
			|| source->sprite->mainGraphic->direction == 9) {
				x_ = source->sprite->position.x + 1;
				y_ = source->sprite->position.y - 43;
			}
			else if (source->sprite->mainGraphic->direction == 10
			|| source->sprite->mainGraphic->direction == 11) {
				x_ = source->sprite->position.x - 3;
				y_ = source->sprite->position.y - 42;
			}
			else if (source->sprite->mainGraphic->direction == 12
			|| source->sprite->mainGraphic->direction == 13) {
				x_ = source->sprite->position.x - 6;
				y_ = source->sprite->position.y - 41;
			}
			else if (source->sprite->mainGraphic->direction == 14
			|| source->sprite->mainGraphic->direction == 15) {
				x_ = source->sprite->position.x - 8;
				y_ = source->sprite->position.y - 44;
			}
			else if (source->sprite->mainGraphic->direction == 16) {
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

	const u32 Func_progressBulletMovement = 0x0048B250;
	void progressBulletMovement_func(CBullet* a1) {
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_progressBulletMovement
			POPAD
		}
	}

	const u32 Func_playImageIscript = 0x004D8470;
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

	const u32 Func_changeMovePos = 0x00495240;
	CBullet* changeMovePos(CBullet* a1, s32 a2, s32 a3) {
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

	const u32 Func_findNextBounceTarget = 0x0048B1E0;
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

	const u32 Func_unitCanSeeCloakedTarget = 0x00401D60;
	bool unitCanSeeCloakedTarget(CUnit* a1, CUnit* a2) {
		static s32 result;
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

	const u32 Func_unitIsSubunit = 0x00401D40;
	bool unitIsSubunit(CUnit* a1) {
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsSubunit
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_unitIsCarrier = 0x00401470;
	bool unitIsCarrier(CUnit* a1) {
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsCarrier
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_unitIsReaver = 0x00401490;
	bool unitIsReaver(CUnit* a1) {
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsReaver
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_unitHasPathToUnit = 0x0049CBB0;
	bool unitHasPathToUnit(CUnit* a1, CUnit* a2) {
		static s32 result;
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

	const u32 Func_canInfest = 0x00402750;
	bool canInfest(CUnit* a1) {
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_canInfest
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_canBeInfested = 0x00402210;
	s32 canBeInfested(CUnit* a1, CUnit* a2) {
		static s32 result;
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

	const u32 Func_iterateUnitsAtLocationTargetProc = 0x004E8280;
	s32 iterateUnitsAtLocationTargetProc(Box32* a1, s32 (__fastcall *a2)(s32, s32), CUnit* a3) {
		static s32 result;
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
	const u32 func_doMissileDmg = 0x0048B770;
	void doMissileDmg(CBullet* bullet) {
		__asm {
			PUSHAD
			PUSH bullet
			CALL func_doMissileDmg
			POPAD
		}
	};

} //unnamed namespace
