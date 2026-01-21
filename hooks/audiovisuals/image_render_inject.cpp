#include "image_render.h"
#include <hook_tools.h>

namespace {
static int var0;
static int var1;
static int var2;
static int var3;

void __declspec(naked) updateSpriteFlagOnPalette_wrapper() {
    __asm {
        MOV		var0, EAX // sprite
        PUSHAD
    }
    hooks::update_sprite_flag_on_palette((Sprite*)var0);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) make_sprite_visible_wrapper() {
    __asm {
        MOV		var0, EAX // sprite
        PUSHAD
    }
    hooks::make_sprite_visible((Sprite*)var0);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) make_cloaked_wrapper() {
    __asm {
        PUSH	EBP
        MOV		EBP, ESP
        MOV		var0, EAX // cloakH
        MOV		var1, ESI // sprite
        MOV		EAX, [EBP + 0x08]
        MOV		var2, EAX // cloakedStatus
        MOV		EAX, [EBP + 0x0C]
        MOV		var3, EAX // cloakL
        PUSHAD
    }
    hooks::make_cloaked((u8)var0, (Sprite*)var1, (u8)var2, (u8)var3);
    __asm {
        POPAD
        POP		EBP
        RETN	8
    }
}

void __declspec(naked) sub_4980F0_wrapper() {
    __asm {
        MOV		var0, ESI // sprite
        PUSHAD
    }
    hooks::sub_4980F0((Sprite*)var0);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) showImagesHiddenByCloak_wrapper() {
    __asm {
        MOV		var0, EAX // sprite
        PUSHAD
    }
    hooks::showImagesHiddenByCloak((Sprite*)var0);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) make_sprite_invisible_wrapper() {
    __asm {
        MOV		var0, EAX // sprite
        PUSHAD
    }
    hooks::make_sprite_invisible((Sprite*)var0);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) UpdateSpriteImagesForCloak_wrapper() {
    __asm {
        MOV		var0, EDI // sprite
        PUSHAD
    }
    hooks::UpdateSpriteImagesForCloak((Sprite*)var0);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) sub_498B50_wrapper() {
    __asm {
        PUSH	EBP
        MOV		EBP, ESP
        MOV		var0, EAX // sprite
        MOV		EAX, [EBP + 0x08]
        MOV		var1, EAX // flipped
        PUSHAD
    }
    hooks::sub_498B50((Sprite*)var0, (bool)var1);
    __asm {
        POPAD
        POP		EBP
        RETN	4
    }
}

void __declspec(naked) duplicate_sprite_wrapper() {
    __asm {
        PUSH	EBP
        MOV		EBP, ESP
        MOV		EAX, [EBP + 0x08]
        MOV		var0, EAX // sprite
        PUSHAD
    }
    var0 = (int)hooks::duplicate_sprite((Sprite*)var0);
    __asm {
        POPAD
        MOV		EAX, var0
        MOV		ESP, EBP
        POP		EBP
        RETN	4
    }
}

void __declspec(naked) sub_4BCD70_wrapper() {
    __asm {
        MOV		var0, EAX // paletteEntry
        PUSHAD
    }
    hooks::sub_4BCD70((u8*)var0);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) drawSprite_patch_98C84() {
    __asm {
        MOV		AL, [EAX + 0x0A]
        CMP		AL, 18 // PaletteType::WarpAnchor
        JB		not_warp_anchor
        MOV		AL, 6 // PaletteType::VisibleCloaked

    not_warp_anchor:
        XOR		ECX, ECX
        RET
    }
}

void __declspec(naked) set_image_palette_type_wrapper() {
    __asm {
        MOV var0, EAX // image
        MOV var1, EBX // type
        PUSHAD
    }
    hooks::set_image_palette_type((Image*)var0, (u8)var1);
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) set_image_palette_type2_wrapper() {
    __asm {
        MOV var0, EAX // image
        MOV var1, EBX // type
        PUSHAD
    }
    hooks::set_image_palette_type2((Image*)var0, (u8)var1);
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) copy_image_palette_type_wrapper() {
    __asm {
        MOV var0, EBX // image
        PUSHAD
    }
    hooks::copy_image_palette_type((Image*)var0);
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) draw_if_cloaked_ebx() {
    __asm {
        MOV var0, EBX // image
        MOV var1, EDX // image_id
        PUSHAD
    }
    var0 = (int)hooks::draw_if_cloaked((Image*)var0, var1);
    __asm {
        POPAD
        MOV EAX, var0
        RETN
    }
}

void __declspec(naked) AI_CloakUnit_wrapper() {
    __asm {
        MOV var0, EDI // unit
        PUSHAD
    }
    hooks::AI_CloakUnit((Unit*)var0);
    __asm {
        POPAD
        RETN
    }
}
}

namespace hooks {
void inject_image_render_hooks() {
    jump_patch(updateSpriteFlagOnPalette_wrapper, 0x00497E80);
    jump_patch(make_sprite_visible_wrapper, 0x00497EDB, 1);
    jump_patch(make_cloaked_wrapper, 0x00497FD0, 1);
    jump_patch(sub_4980F0_wrapper, 0x004980F0, 2);
    jump_patch(showImagesHiddenByCloak_wrapper, 0x00498170);
    jump_patch(make_sprite_invisible_wrapper, 0x00498A10, 2);
    jump_patch(UpdateSpriteImagesForCloak_wrapper, 0x00498A90);
    jump_patch(sub_498B50_wrapper, 0x00498B50, 1);
    jump_patch(duplicate_sprite_wrapper, 0x00499210, 2);
    jump_patch(sub_4BCD70_wrapper, 0x004BCD70);
    call_patch(set_image_palette_type_wrapper, 0x00498FD2, 0);
    call_patch(set_image_palette_type_wrapper, 0x004D5E4E, 0);
    jump_patch(set_image_palette_type2_wrapper, 0x004D58B0, 1);
    call_patch(copy_image_palette_type_wrapper, 0x004D6EE1, 15);
    call_patch(draw_if_cloaked_ebx, 0x004D6EC7, 1);
    nops(0x004A07DC, 14); // unit_die @ 0x004A0740; Allow buildings to be removed from cloaked units list.
    call_patch(AI_CloakUnit_wrapper, 0x0049B60B); // DoCloak @ 0x0049B5B0; Fix unburrowing units being perma-cloaked by arbiters.

    call_patch(drawSprite_patch_98C84, 0x00498C84); // drawSprite @ 0x00498C50
    memory_patch(0x0040B0D7, &threshold_map); // drawFunc_CloakThreshold @ 0x0040AFD5
    memory_patch(0x0040B10E, &threshold_map);
    memory_patch(0x0040B92D, &threshold_map); // drawFunc_CloakThreshold2 @ 0x0040B824
    memory_patch(0x0040B963, &threshold_map);
    memory_patch(0x004D562D, &sgUpdateFuncs[0].fxn); // unpackImageGrpData @ 0x004D5600
    memory_patch(0x004D5618, &sgRenderFuncs[0].fxn_mirror);
    memory_patch(0x004D5621, &sgRenderFuncs[0].fxn);
    memory_patch(0x004D58B9, &sgUpdateFuncs[0].fxn); // set_image_palette_type @ 0x004D58B0
    memory_patch(0x004D58CF, &sgRenderFuncs[0].fxn_mirror);
    memory_patch(0x004D58D8, &sgRenderFuncs[0].fxn);
    memory_patch(0x004D58E6, (u8)0x32);
    memory_patch(0x004D58EA, (u8)0x33);
    memory_patch(0x004D66C7, &sgUpdateFuncs[0].fxn); // initialize_image_continued @ 0x004D66B0
    memory_patch(0x004D66DD, &sgRenderFuncs[0].fxn_mirror);
    memory_patch(0x004D66E6, &sgRenderFuncs[0].fxn);
    memory_patch(0x004D66F4, (u8)0x32);
    memory_patch(0x004D66F8, (u8)0x33);
    memory_patch(0x004D753C, (u8)0x88); // iscript_opcode_cases @ 0x004D74C0
}
}