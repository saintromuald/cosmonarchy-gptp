#include "grid.h"
#include "../../hook_tools.h"
#include "../../BW/bwdata.h"
#include "../../graphics/Bitmap.h"

namespace {
// 0041E0D0(eax = left, ecx = top, edx = bottom, [esp+4] = right), callee-clean.
void __declspec(naked) mark_rect_wrapper() {
    static s32 left;
    static s32 top;
    static s32 right;
    static s32 bottom;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV left, EAX
        MOV top, ECX
        MOV bottom, EDX
        MOV EAX, [EBP + 0x08]
        MOV right, EAX
        PUSHAD
    }
    hooks::mark_rect(left, top, right, bottom);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x04
    }
}

void __declspec(naked) mark_rect_cells_wrapper() {
    static s32* rect;
    __asm {
        MOV rect, ESI
        PUSHAD
    }
    hooks::mark_rect_cells(rect);
    __asm {
        POPAD
        RETN
    }
}

// 0045CC90(eax = rect) -> non-zero when rectangle clipped away to nothing.
void __declspec(naked) clip_rect_to_world_wrapper() {
    static s32* rect;
    static u32 result;
    __asm {
        MOV rect, EAX
        PUSHAD
    }
    result = hooks::clip_rect_to_world(rect);
    __asm {
        POPAD
        MOV EAX, result
        RETN
    }
}

// 00497000(eax = object) -> non-zero when object needs redrawing.
void __declspec(naked) object_is_dirty_wrapper() {
    static const u8* object;
    static u32 result;
    __asm {
        MOV object, EAX
        PUSHAD
    }
    result = hooks::object_is_dirty(object);
    __asm {
        POPAD
        MOV EAX, result
        RETN
    }
}

void __declspec(naked) expire_text_line_wrapper() {
    static u32 index;
    __asm {
        MOV index, EAX
        PUSHAD
    }
    hooks::expire_text_line(index);
    __asm {
        POPAD
        RETN
    }
}

} // namespace

namespace hooks {

void inject_grid_hooks() {
    jump_patch(begin_frame, 0x0041D710);
    jump_patch(rect_is_dirty, 0x0041DE20);
    jump_patch(flush_region, 0x0041E000);
    jump_patch(mark_rect_wrapper, 0x0041E0D0);
    jump_patch(present_frame, 0x0041E280);
    jump_patch(mark_rect_cells_wrapper, 0x0042D280);
    jump_patch(clip_rect_to_world_wrapper, 0x0045CC90);
    jump_patch(draw_box, 0x004E1D20);
    jump_patch(expire_text_line_wrapper, 0x0048CB80);
    jump_patch(object_is_dirty_wrapper, 0x00497000);
    jump_patch(refresh_world_cells, 0x004BCDC0);
    jump_patch(init_game_layer, 0x004BD630);
}

} // hooks
