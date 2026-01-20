#include "replay.h"
#include "../hook_tools.h"

namespace hooks {
// Hooked to investigate the replay score screen freeze. -Pr0nogo
void step_through_replay_data(GameAction* action_set) { // 004CDCE0
    s32 action = action_set->start;
    s32 size = action_set->size;
    s32 action_end = size + action;

    s32 command_size = 0;
    s32 data = 0;
    while (action < action_end) {
        s32 action_length = *(s32*)(action + 4);
        action += 5;
        for (s32 current_length = 0; current_length < action_length; action = command_size + data) {
            s32 command_id = *(s32*)(action + 1);
            command_size = g_turn_command_size[command_id];
            if (command_size < 0) {
                return;
            }
            data = action + 1;
            if (command_id >= 9 && command_id <= 11) {
                command_size = 2 * data[(s32*)1] + 2;
            }
            current_length += command_size + 1;
        }
    }
}

// Injection \\

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

void inject_replay_hooks() {
    jump_patch(hooks::step_through_replay_data_wrapper, 0x004CDCE0, 1);
}
} // namespace hooks