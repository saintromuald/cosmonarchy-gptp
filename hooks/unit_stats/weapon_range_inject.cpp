#include "weapon_range.h"
#include <hook_tools.h>

namespace {
void __declspec(naked) get_seek_range_wrapper() { // 00476000
    static Unit* unit;
    static u8 result;
    __asm {
        MOV unit, EDX
        PUSHAD
    }
    result = hooks::get_seek_range(unit, unit->order_target.unit);
    __asm {
        POPAD
        MOVZX EAX, result
        RETN
    }
}

void __declspec(naked) get_max_weapon_range_wrapper() { // 00475870
    static Unit* unit;
    static WeaponId::Enum weapon_id;
    static u32 result;
    __asm {
        MOV unit, EAX
        MOV weapon_id, BX
        PUSHAD
    }
    result = hooks::get_max_weapon_range(unit, weapon_id, unit->order_target.unit);
    __asm {
        POPAD
        MOV EAX, result
        RETN
    }
}

void __declspec(naked) reassign_target_seek_range_wrapper() { // 0049E010
    __asm {
        PUSHAD
    }
    hooks::reassign_target_seek_range();
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) get_standard_attack_target_wrapper() { // 00442DA0
    static Unit* target;
    static Unit* attacker;
    __asm {
        MOV target, ECX
        MOV attacker, EDX
        PUSHAD
    }
    hooks::get_standard_attack_target(target, attacker);
    __asm {
        POPAD
        MOV EAX, 0 // Always 0
        RETN
    }
}
}

// Defined in SCBW/structures/Unit.cpp
extern const u32 Func_GetMaxWeaponRange;
extern const u32 Func_GetSeekRange;

namespace hooks {
    void inject_weapon_range_hooks() {
        jump_patch(get_seek_range_wrapper,		0x00476000, 3);
        jump_patch(get_max_weapon_range_wrapper,	0x00475870, 1);
        jump_patch(reassign_target_seek_range_wrapper,	0x0049E010, 0);
        jump_patch(get_standard_attack_target_wrapper,	0x00442DA0, 0);
    }
} // namespace hooks