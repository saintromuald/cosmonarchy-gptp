#include "mouse.h"
#include "../../hook_tools.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"

namespace {
// 004D1940(eax = button bit, esi = packed position, edi = input proc, [esp+4] = event type), callee-clean.
void __declspec(naked) mouse_button_up_wrapper() {
    static u32 button_bit;
    static u32 packed_position;
    static InputProcsFunc* input_proc;
    static u32 event_type;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV button_bit, EAX
        MOV packed_position, ESI
        MOV input_proc, EDI
        MOV EAX, [EBP + 0x08]
        MOV event_type, EAX
        PUSHAD
    }
    hooks::mouse_button_up(button_bit, packed_position, input_proc, event_type);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x04
    }
}

// 004D19C0 and 004D1A50 (same arguments as 004D1940).
void __declspec(naked) mouse_button_down_wrapper() {
    static u32 button_bit;
    static u32 packed_position;
    static InputProcsFunc* input_proc;
    static u32 event_type;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV button_bit, EAX
        MOV packed_position, ESI
        MOV input_proc, EDI
        MOV EAX, [EBP + 0x08]
        MOV event_type, EAX
        PUSHAD
    }
    hooks::mouse_button_down(button_bit, packed_position, input_proc, event_type);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x04
    }
}

void __declspec(naked) set_cursor_clip_rect_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::set_cursor_clip_rect();
    __asm {
        POPAD
        RETN
    }
}

// 004D12A0(edi = &x, esi = &y) -> direction in eax.
void __declspec(naked) edge_scroll_direction_wrapper() {
    static s32* out_x;
    static s32* out_y;
    static s32 result;
    __asm {
        MOV out_x, EDI
        MOV out_y, ESI
        PUSHAD
    }
    result = hooks::edge_scroll_direction(out_x, out_y);
    __asm {
        POPAD
        MOV EAX, result
        RETN
    }
}

// 004D24DC: WM_MOUSEMOVE in main_window_proc (lParam at [ebp+0x14], exits to 004D252B).
void __declspec(naked) window_proc_mouse_move_wrapper() {
    static const u32 jmp_address = 0x004D252B;
    static u32 packed_position;
    __asm {
        MOV EAX, [EBP + 0x14]
        MOV packed_position, EAX
        PUSHAD
    }
    hooks::window_proc_mouse_move(packed_position);
    __asm {
        POPAD
        JMP jmp_address
    }
}

void __declspec(naked) show_os_cursor_wrapper() {
    static u32 inactive;
    __asm {
        MOV inactive, ESI
        PUSHAD
    }
    hooks::show_os_cursor(inactive);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) center_cursor_on_launch_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::center_cursor();
    __asm {
        POPAD
        MOV EAX, dword ptr [0x0051CC34]
        TEST EAX, EAX
        RETN
    }
}

void __declspec(naked) center_cursor_on_game_start_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::center_cursor();
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) is_point_on_console_ui_wrapper() {
    static s32 x;
    static s32 y;
    static u32 result;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV y, EAX
        MOV x, ECX
        PUSHAD
    }
    result = hooks::is_point_on_console_ui(x, y);
    __asm {
        POPAD
        MOV EAX, result
        MOV ESP, EBP
        POP EBP
        RET
    }
}

} // namespace

namespace hooks {

void inject_mouse_hooks() {
    memory_patch(0x004D13F0, u32(&g_world_cursor.x));
    memory_patch(0x004D13FE, u32(&g_world_cursor.y));

    jump_patch(send_mouse_move_event, 0x004D1AE0);
    jump_patch(is_point_on_console_ui_wrapper, 0x004D1140);
    jump_patch(mouse_button_up_wrapper, 0x004D1940);
    jump_patch(mouse_button_down_wrapper, 0x004D19C0);
    jump_patch(mouse_button_down_wrapper, 0x004D1A50);
    jump_patch(edge_scroll_direction_wrapper, 0x004D12A0);
    jump_patch(set_cursor_clip_rect_wrapper, 0x004215E0);
    jump_patch(update_cursor, 0x004D14D0);
    jump_patch(refresh_cursor, 0x004BE120);
    jump_patch(cursor_refresh, 0x004BE060);
    jump_patch(refresh_cursor_0, 0x004BE0B0);

    // 0041C987: jz -> jmp. 0041C810 (dialog control present)
    // Prevent the software cursor from restoring last MMB drag as extra on-screen cursor
    memory_patch(0x0041C987, u8(0xEB));
    jump_patch(show_os_cursor_wrapper, 0x004D1750);
    call_patch(center_cursor_on_launch_wrapper, 0x004E087A, 38);
    call_patch(center_cursor_on_game_start_wrapper, 0x004D996B, 10);

    memory_patch(0x004D2684, u32(window_proc_mouse_move_wrapper));
}

} // hooks
