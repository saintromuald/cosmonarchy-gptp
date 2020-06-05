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
	void fireWeaponHook(CUnit* unit, u8 weaponId) {
		//Default StarCraft behavior

		//Retrieve the spawning position for the bullet.
		s32 x, y;

		if (weapons_dat::Behavior[weaponId] == WeaponBehavior::AppearOnTargetUnit) {

			if (unit->orderTarget.unit == NULL)
				return;

			getWeaponHitPos(unit, &x, &y);

		}
		else
			if (weapons_dat::Behavior[weaponId] == WeaponBehavior::AppearOnTargetSite) {
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
			createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);


			if (weaponId == WeaponId::SubterraneanTentacle && scbw::getUpgradeLevel(unit->playerId, UpgradeId::TendrilAmitosis) &&
				unit->orderTarget.unit != NULL) {
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
