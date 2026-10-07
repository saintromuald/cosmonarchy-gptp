#include "mapblit.h"
#include "../../hook_tools.h"
#include "../../BW/bwdata.h"
#include "../../graphics/Bitmap.h"

namespace {
// 0040C200(eax = src offset, edx = rows, esi = src, edi = dest), ret.
void __declspec(naked) blit_map_rows_wrapper() {
    static s32 src_offset;
    static s32 rows;
    static const u8* src;
    static u8* dest;
    __asm {
        MOV src_offset, EAX
        MOV rows, EDX
        MOV src, ESI
        MOV dest, EDI
        PUSHAD
    }
    hooks::blit_map_rows(src_offset, rows, src, dest);
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) blit_map_viewport_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::blit_map_viewport();
    __asm {
        POPAD
        RETN
    }
}

// 0040C2BD(ecx = y, edx = x, [esp+4] = width, [esp+8] = src offset),
// callee-clean.
void __declspec(naked) blit_map_cell_row_wrapper() {
    static s32 y;
    static s32 x;
    static s32 width;
    static s32 src_offset;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV y, ECX
        MOV x, EDX
        MOV EAX, [EBP + 0x08]
        MOV width, EAX
        MOV EAX, [EBP + 0x0C]
        MOV src_offset, EAX
        PUSHAD
    }
    hooks::blit_map_cell_row(y, x, width, src_offset);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x08
    }
}

} // namespace

namespace hooks {

void inject_mapblit_hooks() {
    jump_patch(blit_map_rows_wrapper, 0x0040C200);
    jump_patch(blit_map_viewport_wrapper, 0x0040C253);
    jump_patch(blit_map_cell_row_wrapper, 0x0040C2BD);
}

} // hooks
