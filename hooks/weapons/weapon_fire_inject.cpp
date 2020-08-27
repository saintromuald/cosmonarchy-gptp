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

	class empyreanInterceptionMatch : public scbw::UnitFinderCallbackMatchInterface {
	private:
		CBullet* missile;
	public:
		empyreanInterceptionMatch(CBullet* missile)
			: missile(missile) {}

		bool match(CUnit* unit) {
			/*if (unit == savant) {
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
			}*/
			return false;
		}
	};

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

} //Unnamed namespace

extern const u32 Func_FireUnitWeapon;

namespace hooks {

void injectWeaponFireHooks() {
	jmpPatch(fireWeaponWrapper, Func_FireUnitWeapon, 0);
	jmpPatch(progressBulletFlight_Wrapper, 0x0048B3B0, 2);
	}
} //hooks
