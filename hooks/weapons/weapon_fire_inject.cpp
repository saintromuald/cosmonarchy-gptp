//Injector source file for the Weapon Fire hook module.
#include "weapon_fire.h"
#include <hook_tools.h>
#include <SCBW/UnitFinder.h>
namespace {

	void __declspec(naked) fireWeaponWrapper() {

		static CUnit* unit;
		static u8 weaponId;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOVZX EAX, [EBP+0x08]
			MOV weaponId, AL
			MOV unit, ESI
			PUSHAD
		}

		hooks::fireWeaponHook(unit, weaponId);

		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	/*class empyreanInterceptionMatch : public scbw::UnitFinderCallbackMatchInterface {
	private:
		CBullet* missile;
	public:
		empyreanInterceptionMatch(CBullet* missile)
			: missile(missile) {}

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
			return false;
		}
	};*/

	void __declspec(naked) progressBulletFlight_Wrapper() {
		static CBullet* bullet;
		__asm {
			MOV bullet, EAX
			PUSHAD
		}
		hooks::progressBulletFlight(bullet);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) bulletBehaviour_Bounce_Wrapper() {
		static CBullet* a1;
		__asm {
			MOV a1, EDI
			PUSHAD
		}
		hooks::bulletBehaviour_Bounce(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) findNextBounceTargetProc_Wrapper() {
		static CUnit* pIterator;
		static bool result;
		__asm {
			MOV pIterator, ECX
			PUSHAD
		}
		result = hooks::findNextBounceTargetProc(pIterator);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) findNextBounceTarget_Wrapper() {
		static CBullet* a1;
		static CUnit* result;
		__asm {
			MOV a1, EAX
			PUSHAD
		}
		result = hooks::findNextBounceTarget(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
	/*
	void __declspec(naked) writeBullets_Wrapper() {
		static FILE* a1;
		static int result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOVZX EAX, [EBP + 0x08]
			PUSHAD
		}
		result = hooks::writeBullets(a1);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
	*/
} //Unnamed namespace

extern const u32 Func_FireUnitWeapon;

namespace hooks {

void injectWeaponFireHooks() {
	jmpPatch(fireWeaponWrapper, Func_FireUnitWeapon, 0);
	jmpPatch(progressBulletFlight_Wrapper, 0x0048B3B0, 3);
	jmpPatch(bulletBehaviour_Bounce_Wrapper, 0x0048B2D0, 0);
	jmpPatch(findNextBounceTargetProc_Wrapper, 0x0048AFD0, 3);//proc
//	jmpPatch(findNextBounceTarget_Wrapper, 0x0048B1E0, 1);
	}
} //hooks
