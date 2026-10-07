#pragma once
#include "../../systems/display/display.h"

namespace hooks {

void __cdecl blit_rect(const s32* rect);  // 0041D3A0
void __cdecl blit_screen();               // 0041D420
s32 __cdecl surface_setup();              // 0041D470

// Plain colorkey copy (palette index 0 skipped), the replacement body for the
// 00416ED0 detour. Storm's fx path stages through the 006D053C scratch, which
// caps out near 640px rows
void colorkey_blit(u8* dest, const u8* src, s32 width, s32 height, s32 src_stride, s32 dest_pitch);

// Translucent dialog blend (palette index 0 colorkeyed, 254 blended), the
// replacement body for the 00416F30 detour. The vanilla body stages through
// Storm's compiled ROP (SBltROP3 executor, ordinal 334), whose staging caps
// rows near 640px
void translucent_fx_blit(u8* dest, const u8* src, s32 width, s32 height, s32 dest_pitch, s32 src_pitch);

void inject_blit_hooks();

} // hooks
