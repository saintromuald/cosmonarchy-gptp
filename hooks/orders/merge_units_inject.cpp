#include "merge_units.h"
#include <hook_tools.h>

namespace {
void __declspec(naked) templar_merge_partner_wrapper() { // 004BFA80
    static Unit* unit;
    static Unit** units_list;
    static u32 units_list_length;
    static Unit* nearestPartner;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        SUB ESP, 0x08
        MOV units_list_length, EAX
        MOV EAX, [EBP + 0x0C]
        MOV units_list, EAX
        MOV EAX, [EBP + 0x08]
        MOV unit, EAX
        PUSHAD
    }
    nearestPartner = hooks::templar_merge_partner(unit, units_list, units_list_length);
    __asm {
        POPAD
        MOV EAX, nearestPartner
        MOV ESP, EBP
        POP EBP
        RETN 0x0008
    }
}

void __declspec(naked) orders_completing_archon_summon_wrapper() { // 00493B10
    static Unit* unit;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        SUB ESP, 0x08
        MOV unit, ESI
        PUSHAD
    }
    hooks::orders_completing_archon_summon(unit);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RET
    }
}

void __declspec(naked) orders_warping_mind_tyrant_wrapper() { // 00494690
    static Unit* unit;
    __asm {
        MOV unit, EAX
        PUSHAD
    }
    hooks::orders_warping_mind_tyrant(unit);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) orders_warping_archon_wrapper() { // 004948B0
    static Unit* unit;
    __asm {
        MOV unit, EAX
        PUSHAD
    }
    hooks::orders_warping_archon(unit);
    __asm {
        POPAD
        RET
    }
}
}

namespace hooks {
void inject_merge_units_hooks() {
    jump_patch(templar_merge_partner_wrapper,		 		0x004BFA80, 1);
    jump_patch(orders_completing_archon_summon_wrapper,	0x00493B10, 1);
    jump_patch(orders_warping_mind_tyrant_wrapper,		0x00494690, 2);
    jump_patch(orders_warping_archon_wrapper,				0x004948B0, 2);
}
}
