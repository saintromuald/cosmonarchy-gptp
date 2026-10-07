#include "blit.h"
#include "../../hook_tools.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../graphics/Bitmap.h"

namespace {

void __declspec(naked) blit_rect_wrapper() { // 0041D3A0
    static const s32* rect;
    __asm {
        MOV rect, ESI
        PUSHAD
    }
    hooks::blit_rect(rect);
    __asm {
        POPAD
        RETN
    }
}

// 00416ED0(ecx = dest pitch, edx = width, eax = src stride, [esp+4] = dest,
// [esp+8] = src, [esp+0xC] = height), callee-clean. The fx dispatch shared by
// render_bitmap mode 1 and the shade-7 delegations from modes 3/4/5; replaced
// by hooks::colorkey_blit because Storm's staging caps rows near 640px.
void __declspec(naked) fx_colorkey_blit_wrapper() {
    static u8* dest;
    static const u8* src;
    static s32 width;
    static s32 height;
    static s32 src_stride;
    static s32 dest_pitch;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV dest_pitch, ECX
        MOV width, EDX
        MOV src_stride, EAX
        MOV EAX, [EBP + 0x08]
        MOV dest, EAX
        MOV EAX, [EBP + 0x0C]
        MOV src, EAX
        MOV EAX, [EBP + 0x10]
        MOV height, EAX
        PUSHAD
    }
    hooks::colorkey_blit(dest, src, width, height, src_stride, dest_pitch);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x0C
    }
}

// 00416F30 ([esp+4]=dest, +8=src, +12=width, +16=height, +20=dest_pitch,
// +24=src_pitch), callee-clean. The prologue (PUSH EBP; MOV EBP,ESP; PUSH ECX)
// is 4 bytes, so the 5-byte jump orphans the last five bytes of
// MOV EAX,[0x6D5EB8]
void __declspec(naked) translucent_fx_blit_wrapper() {
    static u8* dest;
    static const u8* src;
    static s32 width;
    static s32 height;
    static s32 dest_pitch;
    static s32 src_pitch;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV EAX, [EBP + 0x08]
        MOV dest, EAX
        MOV EAX, [EBP + 0x0C]
        MOV src, EAX
        MOV EAX, [EBP + 0x10]
        MOV width, EAX
        MOV EAX, [EBP + 0x14]
        MOV height, EAX
        MOV EAX, [EBP + 0x18]
        MOV dest_pitch, EAX
        MOV EAX, [EBP + 0x1C]
        MOV src_pitch, EAX
        PUSHAD
    }
    hooks::translucent_fx_blit(dest, src, width, height, dest_pitch, src_pitch);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x18
    }
}

} // namespace

namespace hooks {

void inject_blit_hooks() {
    jump_patch(surface_setup, 0x0041D470);
    jump_patch(blit_rect_wrapper, 0x0041D3A0);
    jump_patch(blit_screen, 0x0041D420);
    // The 00416ED0 prologue (PUSH EBP; MOV EBP,ESP; PUSH ESI; MOV ESI,[EBP+8])
    // is 7 bytes, so the 5-byte jump orphans the last two bytes of MOV ESI,[EBP+8]
    jump_patch(fx_colorkey_blit_wrapper, 0x00416ED0, 2);
    jump_patch(translucent_fx_blit_wrapper, 0x00416F30, 5);
}

} // hooks
