#include "tiles.h"
#include "../../hook_tools.h"

namespace {

// 004BCEA0: cdecl void(), caller-clean. Sole caller 004CD0B0 (map load).
void __declspec(naked) set_map_tiles_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::set_map_tiles();
    __asm {
        POPAD
        RET
    }
}

// 0049C4C0: cdecl void(), caller-clean. Sole caller 004D94B0 (periodic update
// while creep is receding).
void __declspec(naked) progress_creep_disappearance_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::progress_creep_disappearance();
    __asm {
        POPAD
        RET
    }
}

// 0047E2D0(x, y, mode, tile_id, rect*): cdecl, callee-clean (RETN 0x14). Called by
// the creep growth system and tile triggers.
void __declspec(naked) __cdecl update_single_tile_wrapper(s32 /*tile_x*/, s32 /*tile_y*/, u32 /*mode*/, u16 /*tile_id*/, const void* /*refresh_rect*/) {
    static s32 tile_x;
    static s32 tile_y;
    static u32 mode;
    static u16 tile_id;
    static const void* refresh_rect;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV EAX, [EBP + 0x08]
        MOV tile_x, EAX
        MOV EAX, [EBP + 0x0C]
        MOV tile_y, EAX
        MOV EAX, [EBP + 0x10]
        MOV mode, EAX
        MOV EAX, [EBP + 0x14]
        MOV tile_id, AX
        MOV EAX, [EBP + 0x18]
        MOV refresh_rect, EAX
        PUSHAD
    }
    hooks::update_single_tile(tile_x, tile_y, mode, tile_id, refresh_rect);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x14
    }
}

} // namespace

namespace hooks {

// 0047D920: redraw request; the affected pixel rect travels in EAX (vanilla
// convention). Calls draw_tile_rect, so it lands in the C++ terrain code.
void __declspec(naked) __cdecl refresh_map_area(const void* /*rect*/) {
    static u32 func_address = 0x0047d920;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        PUSHAD
        MOV EAX, [EBP + 0x08]
        CALL func_address
        POPAD
        MOV ESP, EBP
        POP EBP
        RET
    }
}

// 0047DC10(x, y): refreshes the creep group membership of the tile's 8 neighbors.
// Callee-clean (RETN 8) in vanilla.
void __declspec(naked) __cdecl update_creep_groups_near(s32 /*tile_x*/, s32 /*tile_y*/) {
    static u32 func_address = 0x0047dc10;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        PUSHAD
        MOV EAX, [EBP + 0x08]
        PUSH DWORD PTR [EBP + 0x0C]
        PUSH EAX
        CALL func_address
        POPAD
        MOV ESP, EBP
        POP EBP
        RET
    }
}

void inject_tiles_hooks() {
    jump_patch(set_map_tiles_wrapper, 0x004BCEA0);
    jump_patch(update_single_tile_wrapper, 0x0047E2D0);
    jump_patch(progress_creep_disappearance_wrapper, 0x0049C4C0);

    // ".vx4" -> ".vx4ex": the tileset extension strings sit in an 8-byte-aligned
    // table, so "ex" over the null+padding of ".vx4\0\0\0\0" yields ".vx4ex"
    // without touching the ".grp" entry behind it. Replaces vx4ex.qdp.
    const u8 vx4ex_suffix[] = {'e', 'x'};
    memory_patch(0x00502BC4, vx4ex_suffix, sizeof(vx4ex_suffix));

    // Extended megatiles: widen the megatile id masks (0x00007FFF -> 0x0000FFFF) of
    // the vanilla consumers that stay in assembly. The first two replace aise.qdp's
    // patches; the third is one aise never patched, so movement reads the right
    // VF4 minitile flags for ids > 0x7FFF.
    memory_patch(0x0042BCB1 + 3, (u8)0xFF);   // 0042BC30 AI contours walkability build
    memory_patch(0x004832EF + 2, (u8)0xFF);   // 00483260 per-tile walkability mask generator
    memory_patch(0x0042D5BE + 3, (u8)0xFF);   // 0042D560 per-pixel walkability check

    // Extended tile groups (12-bit), migrated from initialize.cpp. The patches for
    // set_map_tiles, update_single_tile, update_tile_row and progress_creep_disappearance
    // were absorbed by the C++ replacements above; these cover the vanilla functions
    // that remain.

    // Group index masks 0x7FF -> 0xFFF (second immediate byte 0x07 -> 0x0F).
    memory_patch(0x004BD13F + 3, (u8)0x0F);  // 004BD0F0 pixel terrain query

    // "Allow building on extended tile groups": megatile id masks 0x7FFF -> 0xFFFF
    // (second immediate byte 0x7F -> 0xFF) across the creep relational, buildability
    // and placement systems that stay vanilla.
    memory_patch(0x00413F38 + 2, (u8)0xFF);
    memory_patch(0x00413F8D + 2, (u8)0xFF);
    memory_patch(0x00414210 + 2, (u8)0xFF);
    memory_patch(0x00414252 + 2, (u8)0xFF);
    memory_patch(0x004142C5 + 3, (u8)0xFF);
    memory_patch(0x004142EF + 3, (u8)0xFF);
    memory_patch(0x00414326 + 3, (u8)0xFF);
    memory_patch(0x0041435A + 3, (u8)0xFF);
    memory_patch(0x00414390 + 3, (u8)0xFF);
    memory_patch(0x004143C6 + 3, (u8)0xFF);
    memory_patch(0x004143F2 + 3, (u8)0xFF);
    memory_patch(0x0041441A + 2, (u8)0xFF);
    memory_patch(0x00414467 + 2, (u8)0xFF);
    memory_patch(0x004145D5 + 3, (u8)0xFF);
    memory_patch(0x00414720 + 3, (u8)0xFF);
    memory_patch(0x00414882 + 2, (u8)0xFF);
    memory_patch(0x0047D75A + 2, (u8)0xFF);
    memory_patch(0x0047DC99 + 2, (u8)0xFF);
    memory_patch(0x0047DED5 + 3, (u8)0xFF);
    memory_patch(0x0047E045 + 3, (u8)0xFF);
}

} // hooks