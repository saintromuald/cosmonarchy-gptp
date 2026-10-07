#include "camera.h"
#include "../../hook_tools.h"

namespace {

void __declspec(naked) set_camera_map_bounds_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::set_camera_map_bounds();
    __asm {
        POPAD
        RETN
    }
}

// 004C6DE0: Center View trigger action. Subtracts half-viewport before move_screen;
// patches 004C6E6A (`sub ecx, 0xc8`) and 004C6E6F (`sub eax, 0x140`).
void patch_camera_trigger_center_view() {
    memory_patch(0x004C6E6A, u32(g_display.center_offset_y()));   // sub ecx, 0xc8
    memory_patch(0x004C6E6F, u32(g_display.center_offset_x()));   // sub eax, 0x140
}

// 0045E3A0: Stores last event as scroll position with -320/-200 and branchless clamp to 0;
// patches 0045E3A4 (`add eax, -320`) and 0045E3B8 (`add eax, -200`).
void patch_camera_last_event_position() {
    memory_patch(0x0045E3A4, u32(0) - u32(g_display.center_offset_x()));  // add eax, -320
    memory_patch(0x0045E3B8, u32(0) - u32(g_display.center_offset_y()));  // add eax, -200
}

// 004E6020: Ctrl+C follow camera (smooth-follow rewrite; the surrounding
// encoding is patched in initialize.cpp); patches 004E602D (`sub eax, 0x140`)
// and 004E6033 (`sub ecx, 0xc8`).
void patch_follow_camera_center() {
    memory_patch(0x004E602D, u32(g_display.center_offset_x())); // sub eax, 0x140
    memory_patch(0x004E6033, u32(g_display.center_offset_y())); // sub ecx, 0xc8
}

} // namespace

namespace hooks {

void patch_camera_center_immediates() {
    patch_camera_trigger_center_view();
    patch_camera_last_event_position();
    patch_follow_camera_center();
}

void inject_camera_hooks() {
    jump_patch(set_camera_map_bounds_wrapper, 0x0049BB90);

    patch_camera_center_immediates();
}

} // hooks
