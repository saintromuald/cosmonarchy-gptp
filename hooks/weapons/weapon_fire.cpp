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


namespace {
	typedef void(__stdcall* GetWeaponHitPosFunc)(CUnit* unit, s32* x, s32* y);
	GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc)0x004762C0;
	void createBullet(u8 weaponId, CUnit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction);

	// For progressBulletFlight
	void progressBulletMovement(CBullet* a1); // 0048B250
	void playImageIscript(CImage* a1, char a2); // 004D8470
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
				if (weaponId==WeaponId::FuriaMissiles && scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::TempoChange,0)==1) {
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
				if (weaponId == WeaponId::EmpyreanMissiles) {
					auto rotate = scbw::get_generic_value(unit, ValueId::EmpyreanMissilePos);
					const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayAttackPtr, 
											unit->sprite->mainGraphic->id));
					u8 frameDirection = unit->sprite->mainGraphic->direction;
					Point8 offset = loFile->getOffset(frameDirection, rotate);
					if (unit->sprite->mainGraphic->flags & CImage_Flags::Mirrored) {
						offset.x = -offset.x;
					}
//					scbw::printFormattedText("Empyrean Pos: %d, frameDir %d, direct %d, %d %d", rotate, frameDirection, unit->currentDirection1, offset.x, offset.y);
					rotate++;
					if (rotate > 3) {
						rotate = 0;
					}
					scbw::set_generic_value(unit, ValueId::EmpyreanMissilePos, rotate);
					auto X = x + offset.x;
					auto Y = y + offset.y;
					createBullet(weaponId, unit, X,Y, unit->playerId, scbw::getAngle(X,Y,
						unit->orderTarget.unit->position.x,unit->orderTarget.unit->position.y));
				}
				else {
					createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
				}
				
			}

			if (weaponId == WeaponId::SubterraneanTentacle && unit->orderTarget.unit != NULL) {
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
			}
		}
	}

	void progressBulletFlight(CBullet* bullet) { // 0048B3B0
		CBullet* v1; // esi
		CUnit* target; // eax
		CSprite* v4; // edx
		CImage* i; // esi
		progressBulletMovement(bullet);
		u8 time = bullet->time_remaining--;
		bool bEnd = false;
		if (time == 0 || bullet->moveTarget.pt.x == bullet->position.x && bullet->moveTarget.pt.y == bullet->position.y) {
			bEnd = true;
		}
		else if (bullet->weaponType == WeaponId::EmpyreanMissiles) {
			u32 range = 128;
			auto a = scbw::UnitFinder::getNearestTarget(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, empyreanMissileMatch(bullet)
			);
			if (a != NULL) {
				bullet->attackTarget.unit = a;
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

} //unnamed namespace
