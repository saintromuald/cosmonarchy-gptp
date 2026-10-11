#include "bunker_hooks.h"
#include "../hook_tools.h"

namespace {
void __declspec(naked) unit_attacks_from_inside_bunker_wrapper() { // 004790A0
    static Unit* unit;
    __asm {
        MOV unit, EAX
        PUSHAD
    }
    hooks::unit_attacks_from_inside_bunker(unit);
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) create_bunker_attack_overlay_wrapper() { // 00477FD0
    static Unit* unit;
    __asm {
        MOV unit, EAX
        PUSHAD
    }
    hooks::create_bunker_attack_overlay(unit);
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) attack_apply_cooldown_wrapper() { // 00478B40
    static u32 result;
    static Unit* unit;
    __asm {
        MOV unit, EAX
        PUSHAD
    }
    result = hooks::attack_apply_cooldown(unit);
    __asm {
        POPAD
        MOV EAX, result
        RETN
    }
}
}

namespace hooks {
void inject_bunker_hooks() {
    jump_patch(unit_attacks_from_inside_bunker_wrapper,	0x004790A0, 2);
    jump_patch(create_bunker_attack_overlay_wrapper,		0x00477FD0, 3);
    jump_patch(attack_apply_cooldown_wrapper,				0x00478B40, 1);
}
}