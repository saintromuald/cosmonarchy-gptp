#include "minimap.h"
#include <hook_tools.h>

namespace {

    void __declspec(naked) minimap_ping_wrapper() {
        static s32 posX;
        static s32 posY;
        static u32 color;

        __asm {
            PUSH	EBP
            MOV		EBP, ESP
            MOV		EAX, [EBP + 0x8]
            MOV		posX, EAX
            MOV		EAX, [EBP + 0xC]
            MOV		posY, EAX
            MOV		EAX, [EBP + 0x10]
            MOV		color, EAX
            PUSHAD
        }

        hooks::minimap_ping(posX, posY, (u8)color);

        __asm {
            POPAD
            MOV		ESP, EBP
            POP		EBP
            RETN	12
        }
    }

    void __declspec(naked) draw_unit_box_wrapper() {
        static s32 posX;
        static u32 color;
        static s32 posY;
        static u32 sizeX;
        static u32 sizeY;
        static b32 isBuilding;

        __asm {
            PUSH	EBP
            MOV		EBP, ESP
            MOV		posX, EAX
            MOV		color, ECX
            MOV		EAX, [EBP + 0x8]
            MOV		posY, EAX
            MOV		EAX, [EBP + 0xC]
            MOV		sizeX, EAX
            MOV		EAX, [EBP + 0x10]
            MOV		sizeY, EAX
            MOV		EAX, [EBP + 0x14]
            MOV		isBuilding, EAX
            PUSHAD
        }

        hooks::draw_unit_box(posX, (u8)color, posY, sizeX, sizeY, (bool)isBuilding);

        __asm {
            POPAD
            MOV		ESP, EBP
            POP		EBP
            RETN	16
        }
    }

    void __declspec(naked) minimapGameMouseUpdate_wrapper() {
        static bw::Dialog* minimap;
        __asm {
            // 004A4D61 lands mid-function inside vanilla 004A4D20, whose prologue
            // owns this EBP frame; the epilogue below returns straight to vanilla's
            // caller, skipping the rest of 004A4D20.
            MOV		minimap, EDI
            PUSHAD
        }
        hooks::minimap_game_mouse_update(minimap);
        __asm {
            POPAD
            MOV		ESP, EBP
            POP		EBP
            RET
        }
    }

    // 004A3D70(eax = &x, ecx = &y) -> the y it wrote, in ax. The result is read,
    // so it is moved into EAX after POPAD has put the original back.
    void __declspec(naked) minimap_cursor_to_world_wrapper() {
        static u16* x;
        static u16* y;
        static u16 result;
        __asm {
            MOV		x, EAX
            MOV		y, ECX
            PUSHAD
        }
        result = hooks::minimap_cursor_to_world(x, y);
        __asm {
            POPAD
            MOVZX	EAX, result
            RETN
        }
    }

    // 004A3570(this=ECX, event=EDX, user, draw_ctx): the minimap overlay root's
    // update dispatch, replaced wholesale by the C++ reimplementation.
    void __declspec(naked) minimap_overlay_draw_wrapper() {
        static bw::Dialog* dialog;
        static DialogEvent* event;
        static u32 user;
        static void* draw_ctx;
        __asm {
            MOV dialog, ECX
            MOV event, EDX
            MOV EAX, [ESP + 4]
            MOV user, EAX
            MOV EAX, [ESP + 8]
            MOV draw_ctx, EAX
            PUSHAD
        }
        hooks::minimap_overlay_draw(dialog, event, user, draw_ctx);
        __asm {
            POPAD
            RET 8
        }
    }

    void __declspec(naked) setMapSizeConstants_wrapper() {
        __asm {
            PUSHAD
        }
        hooks::set_map_size_constants();
        __asm {
            POPAD
            RET
        }
    }

    void __declspec(naked) minimap_timer_wrapper() {
        __asm {
            PUSHAD
        }
        hooks::minimap_timer();
        __asm {
            POPAD
            RET
        }
    }
}

namespace hooks {
    void inject_minimap_hooks() {
        jump_patch(minimap_ping_wrapper, 0x004A34C0, 1);
        jump_patch(draw_unit_box_wrapper, 0x004A3FD0, 1);
        jump_patch(setMapSizeConstants_wrapper, 0x004A4400, 1);
        memory_patch(0x004A4CEA, &minimap_image_update);
        jump_patch(minimapGameMouseUpdate_wrapper, 0x004A4D61, 2);
        jump_patch(minimap_cursor_to_world_wrapper, 0x004A3D70);
        jump_patch(minimap_timer_wrapper, 0x004A4E00);
        jump_patch(minimap_overlay_draw_wrapper, 0x004A3570);
    }
}