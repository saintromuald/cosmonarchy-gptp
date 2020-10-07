//Source file for the Weapon Fire hook module.
//This hook controls how weapons are fired.
#include "weapon_fire.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "SCBW/UnitFinder.h"
#include "MathUtils.h"
#include "hooks/byte_operations_utils.h"
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

	// For bulletBehavior_Bounce
	CBullet* changeMovePos(CBullet* a1, s32 a2, s32 a3); // 00495240
	void progressBulletMovement(CBullet* a1); // 0048B250
	CUnit* findNextBounceTarget(CBullet* a1); // 0048B1E0

	// For findNextBounceTargetProc
	bool unitCanSeeCloakedTarget(CUnit* a1, CUnit* a2); // 00401D60
	bool unitIsSubunit(CUnit* a1); // 00401D40
	bool unitIsCarrier(CUnit* a1); // 00401470
	bool unitIsReaver(CUnit* a1); // 00401490
	bool unitHasPathToUnit(CUnit* a1, CUnit* a2); // 0049CBB0
	bool canInfest(CUnit* a1); // 00402750
	s32 canBeInfested(CUnit* a1, CUnit* a2); // 00402210

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
			if (unit->id == UnitId::TerranWyvern && unit->sprite->mainGraphic->animation == IscriptAnimation::CastSpell && scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::WyvernReverseLandgrabOn, 0) == 1) {
				good = false;
			}
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
				if (weaponId == WeaponId::FuriaMissiles && scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::TempoChange,0)==1) {
					weaponId = WeaponId::ImperiosoMissiles;
				}
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
				if (weaponId == WeaponId::SunchaserMissiles) {
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
					// Phobos - Argent Capacitors passive
					if (unit->id == UnitId::TerranPhobos && scbw::get_generic_value(unit, ValueId::PhobosDischarge) == 0) {
						unit->addUnitEnergy(5 * 256);
						if (unit->energy == unit->getMaxEnergy()) {
							scbw::set_generic_value(unit, ValueId::PhobosDischarge, 1);
						}
					}
					// Phobos missile spawn positions
					scbw::setMissileSourcePos(unit, weaponId, ValueId::PhobosMissilePos, 6, ImagesDatEntries::OverlaySpecialPtr);
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
				else {
					// Madcap - Maverick Feeders passive
					if (unit->id == UnitId::TerranMadcap) {
						scbw::add_generic_value(unit, ValueId::MadcapStacks, 1);
					}
					createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
				}				
			}

			// Madrigal Furia Missiles
			if (weaponId == WeaponId::FuriaMissiles && unit->orderTarget.unit != NULL) {
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
		progressBulletMovement(bullet);
		u8 time = bullet->time_remaining--;
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

		if (bEnd)
		{
			target = bullet->attackTarget.unit;
			bullet->behaviourTypeInternal = 5;
			if (target != NULL)
			{
				bullet->attackTarget.pt.x = target->sprite->position.x;
				bullet->attackTarget.pt.y = target->sprite->position.y;
			}
			else
			{
				bullet->attackTarget.unit = 0;
			}
			v4 = bullet->sprite;
			bullet->unknown_0x4E = 0;
			bullet->someUnitType = 228;
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
			if (!(a1->hitFlags & 1))
				changeMovePos(a1, v1->sprite->position.y, v1->sprite->position.x);
		}
		progressBulletMovement(a1);
		if (a1->moveTarget.pt.x == a1->position.x && a1->moveTarget.pt.y == a1->position.y)	{
			v2 = a1->remainingBounces - 1;
			a1->remainingBounces = v2;
			if (v2 && (v3 = findNextBounceTarget(a1), a1->nextBounceUnit = a1->attackTarget.unit, v3)) {
				for (i = a1->sprite->images.head; i; i = i->link.next)
					playImageIscript(i, IscriptAnimation::SpecialState1);
				a1->attackTarget.unit = v3;
			}
			else {
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
				for (j = v6->images.head; j; j = j->link.next)
					playImageIscript(j, IscriptAnimation::Death);
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
		BOOL v9; // eax
		s32 v10; // eax
		CUnit* v11; // ecx
		s32 v13; // eax
		s8 v14; // cl

		pSrc = bulletBounceMissileSource;
		nSrcStatus = bulletBounceMissileSource->status;
		_pIterator = pIterator;
		if (nSrcStatus & 0x400)
			return 0;
		if (bulletBounceMissileSource->lockdownTimer)
			return 0;
		if (bulletBounceMissileSource->stasisTimer)
			return 0;
		if (bulletBounceMissileSource->maelstromTimer)
			return 0;
		if (!pIterator)
			return 0;
		if (pIterator->status & UnitStatus::Invincible)
			return 0;
		if (pIterator->sprite->flags & 0x20)
			return 0;
		ByteOperations::SetLowByte(v4, unitCanSeeCloakedTarget(pIterator, bulletBounceMissileSource));
		if (v4)
			return 0;
		v5 = pSrc->subunit;
		ByteOperations::SetLowByte(v6, unitIsSubunit(pSrc->subunit));
		if (!v6)
			v5 = pSrc;
		ByteOperations::SetLowByte(v7, unitIsCarrier(pSrc));
		if (!v7) {
			ByteOperations::SetLowByte(v8, unitIsReaver(pSrc));
			if (v8)	{
				if (_pIterator->status & UnitStatus::InAir)
					return 0;
				ByteOperations::SetLowByte(v9, unitHasPathToUnit(pSrc, _pIterator));
			}
			else {
				ByteOperations::SetLowByte(v10, canInfest(pSrc));
				if (v10) {
					v9 = canBeInfested(v11, _pIterator) != 0;
				}
				else {
					if (pSrc->id == UnitId::ProtossArbiter && pSrc->pAI)
						return 0;
					if (_pIterator->status & UnitStatus::InAir)
						v9 = units_dat::AirWeapon[(u16)v5->id] != -126;
					else
						v9 = units_dat::GroundWeapon[(u16)v5->id] != -126;
				}
			}
			if (!v9)
				return 0;
		}
		v13 = _pIterator->status;
		if (v13 & 0x300 && !((1 << pSrc->playerId) & _pIterator->visibilityStatus))
			return 0;
		v14 = _pIterator->playerId == 11 ? _pIterator->sprite->playerId : _pIterator->playerId;
		if (playerAlliance[(u8)pSrc->playerId][v14] || bulletBounceMissileTarget == _pIterator)
			return 0;
		return bulletBounceMissileNext != _pIterator;
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
	void progressBulletMovement(CBullet* a1) {
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
	CUnit* findNextBounceTarget(CBullet* a1) {
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


} //unnamed namespace
