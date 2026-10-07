#include "terrain.h"
#include "../../hook_tools.h"

namespace {
// 0049B8D0(eax = &tile x, ecx = &tile y, [esp+4] = &width, [esp+8] = &height) -> non-zero if survived clipping, callee-clean.
void __declspec(naked) clip_tile_rect_wrapper() {
    static s32* tile_x;
    static s32* tile_y;
    static s32* out_width;
    static s32* out_height;
    static u32 result;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV tile_x, EAX
        MOV tile_y, ECX
        MOV EAX, [EBP + 0x08]
        MOV out_width, EAX
        MOV EAX, [EBP + 0x0C]
        MOV out_height, EAX
        PUSHAD
    }
    result = hooks::clip_tile_rect(tile_x, tile_y, out_width, out_height);
    __asm {
        POPAD
        MOV EAX, result
        MOV ESP, EBP
        POP EBP
        RETN 0x08
    }
}

// 0040AAE0(ecx = record, edx = offset), ret.
void __declspec(naked) blit_overlay_wrapper() {
    static const u8* record;
    static s32 record_offset;   // "offset" is a MASM operator and cannot name a static
    __asm {
        MOV record, ECX
        MOV record_offset, EDX
        PUSHAD
    }
    hooks::blit_overlay(record, record_offset);
    __asm {
        POPAD
        RETN
    }
}

// 0049BE20(eax = tile x, [esp+4] = megatiles, [esp+8] = tile y), callee-clean.
void __declspec(naked) draw_tile_row_wrapper() {
    static const u16* megatiles;
    static u32 tile_y;
    static u32 tile_x;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV tile_x, EAX
        MOV EAX, [EBP + 0x08]
        MOV megatiles, EAX
        MOV EAX, [EBP + 0x0C]
        MOV tile_y, EAX
        PUSHAD
    }
    hooks::draw_tile_row(megatiles, tile_y, tile_x);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x08
    }
}

// 0049BD40(eax = tile y, [esp+4] = megatiles, [esp+8] = tile x), callee-clean.
void __declspec(naked) draw_tile_column_wrapper() {
    static const u16* megatiles;
    static u32 tile_x;
    static u32 tile_y;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV tile_y, EAX
        MOV EAX, [EBP + 0x08]
        MOV megatiles, EAX
        MOV EAX, [EBP + 0x0C]
        MOV tile_x, EAX
        PUSHAD
    }
    static u32 result;
    result = hooks::draw_tile_column(megatiles, tile_x, tile_y);
    __asm {
        POPAD
        MOV EAX, result
        MOV ESP, EBP
        POP EBP
        RETN 0x08
    }
}

// 0049BC20(all four on the stack), callee-clean.
void __declspec(naked) draw_tile_rect_wrapper() {
    static s32 tile_x;
    static s32 tile_y;
    static s32 width;
    static s32 height;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV EAX, [EBP + 0x08]
        MOV tile_x, EAX
        MOV EAX, [EBP + 0x0C]
        MOV tile_y, EAX
        MOV EAX, [EBP + 0x10]
        MOV width, EAX
        MOV EAX, [EBP + 0x14]
        MOV height, EAX
        PUSHAD
    }
    hooks::draw_tile_rect(tile_x, tile_y, width, height);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x10
    }
}

void __declspec(naked) draw_all_tile_rows_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::draw_all_tile_rows();
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) redraw_terrain_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::redraw_terrain();
    __asm {
        POPAD
        RETN
    }
}

// Terrain scrollers return status in eax:
void __declspec(naked) scroll_right_wrapper() {
    static s32 amount;
    static u32 result;
    __asm {
        MOV amount, EDX
        PUSHAD
    }
    result = hooks::scroll_right(amount);
    __asm {
        POPAD
        MOV EAX, result
        RETN
    }
}

void __declspec(naked) scroll_down_wrapper() {
    static s32 amount;
    static u32 result;
    __asm {
        MOV amount, EDX
        PUSHAD
    }
    result = hooks::scroll_down(amount);
    __asm {
        POPAD
        MOV EAX, result
        RETN
    }
}

void __declspec(naked) scroll_left_wrapper() {
    static s32 amount;
    static u32 result;
    __asm {
        MOV amount, EAX
        PUSHAD
    }
    result = hooks::scroll_left(amount);
    __asm {
        POPAD
        MOV EAX, result
        RETN
    }
}

void __declspec(naked) scroll_up_wrapper() {
    static s32 amount;
    static u32 result;
    __asm {
        MOV amount, EAX
        PUSHAD
    }
    result = hooks::scroll_up(amount);
    __asm {
        POPAD
        MOV EAX, result
        RETN
    }
}

} // namespace

namespace hooks {

void inject_terrain_hooks() {
    jump_patch(blit_overlay_wrapper, 0x0040AAE0);
    jump_patch(clip_tile_rect_wrapper, 0x0049B8D0);
    jump_patch(draw_tile_rect_wrapper, 0x0049BC20);
    jump_patch(draw_tile_column_wrapper, 0x0049BD40);
    jump_patch(draw_tile_row_wrapper, 0x0049BE20);
    jump_patch(draw_all_tile_rows_wrapper, 0x0049BF20);
    jump_patch(scroll_right_wrapper, 0x0049C0C0);
    jump_patch(scroll_left_wrapper, 0x0049C1A0);
    jump_patch(scroll_down_wrapper, 0x0049C280);
    jump_patch(scroll_up_wrapper, 0x0049C360);
    jump_patch(redraw_terrain_wrapper, 0x0049C780);
}

} // hooks
