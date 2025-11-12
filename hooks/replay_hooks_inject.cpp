#include "replay_hooks.h"
#include <hook_tools.h>

namespace {

void __declspec(naked) replay_save_wrapper() { // 004EEBEA
    static const u32 jmp_address = 0x004EEBF0;
    __asm {
        PUSHAD
    }
    hooks::replay_save();
    static u8 is_league = *g_is_league_game;
    __asm {
        POPAD
        CMP bl, is_league // original code
        JMP jmp_address
    }
}

void __declspec(naked) step_through_replay_data_wrapper() { // 004CDCE0
    static GameAction* action_set;
    __asm {
        MOV action_set, ECX
        PUSHAD
    }
    hooks::step_through_replay_data(action_set);
    __asm {
        POPAD
        RETN
    }
}
} // namespace

namespace hooks {

void inject_replay_hooks() {
    jump_patch(replay_save_wrapper,				0x004EEBEA, 1);
    jump_patch(step_through_replay_data_wrapper, 0x004CDCE0, 1);
}

} // namespace hooks
