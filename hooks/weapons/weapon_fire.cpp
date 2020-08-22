//Source file for the Weapon Fire hook module.
//This hook controls how weapons are fired.
#include "weapon_fire.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "SCBW/UnitFinder.h"

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

class powerSiphonCollectMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CUnit* savant;
public:
	powerSiphonCollectMatch(CUnit* savant)
		: savant(savant) {}

	bool match(CUnit* unit) {
		if (unit == savant) {
			return false;
		}
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster)) {
			return false;
		}
		if (unit->energy == 0) {
			return false;
		}
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
	}
};


namespace {
	typedef void(__stdcall* GetWeaponHitPosFunc)(CUnit* unit, s32* x, s32* y);
	GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc)0x004762C0;
	void createBullet(u8 weaponId, CUnit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction);
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

		if (weapons_dat::Behavior[weaponId] == WeaponBehavior::AppearOnTargetUnit) {

			if (unit->orderTarget.unit == NULL)
				return;

			getWeaponHitPos(unit, &x, &y);

		}
		else if (weapons_dat::Behavior[weaponId] == WeaponBehavior::AppearOnTargetSite) {
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
				/*	u32 energy = 0;
					powerSiphonCollectMatch Orders = powerSiphonCollectMatch(unit);
					u32 range = weapons_dat::OuterSplashRadius[WeaponId::PowerSiphon];
					auto pos = unit->orderTarget.pt;
					if (unit->orderTarget.unit != NULL) {
						pos = unit->orderTarget.unit->position;
					}
					scbw::UnitFinder unitFinder(pos.x - range,
						pos.y - range,
						pos.x + range,
						pos.y + range);
					auto a = unitFinder.s;*/
				}
				createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
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

} //unnamed namespace
