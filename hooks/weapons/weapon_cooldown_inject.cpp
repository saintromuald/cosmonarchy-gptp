#include "weapon_cooldown.h"
#include "../../hook_tools.h"

namespace {
	void __declspec(naked) get_modified_weapon_cooldown_wrapper() {
		static Unit* unit;
		static WeaponId::Enum weapon_id;
		static u32 result;
		__asm {
			MOV unit, ESI
			MOV weapon_id, AX
			PUSHAD
		}
		result = hooks::get_modified_weapon_cooldown(unit, weapon_id);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks {
	void inject_weapon_cooldown_hooks() {
		jump_patch(get_modified_weapon_cooldown_wrapper, 0x00475DC0, 2);
	}
}