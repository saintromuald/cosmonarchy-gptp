#include "briefing.h"
#include <BW/structures.h>
#include <hook_tools.h>

namespace {
void __declspec (naked) update_mission_objectives_wrapper() { // 0046CD30
    __asm {
        PUSHAD
    }
    hooks::update_mission_objectives();
    __asm {
        POPAD
        RETN
    }
}

void __declspec (naked) init_briefing_portrait_window_wrapper() { // 0044EF00
    static bw::Dialog* window;
    __asm {
        PUSHAD
        MOV window, EAX
    }
    hooks::init_briefing_portrait_window(window);
    __asm {
        POPAD
        RETN
    }
}
} // namespace

namespace hooks {
void inject_briefing_hooks() {
//	jump_patch(update_mission_objectives_wrapper,	0x0046CD30, 0);
    jump_patch(init_briefing_portrait_window_wrapper, 0x0044EF00);
}
} // namespace hooks
