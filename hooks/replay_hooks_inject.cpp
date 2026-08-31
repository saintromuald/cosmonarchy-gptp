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

void* save_crash_replay() {
    if (!g_game_active || g_is_in_replay) {
        return g_old_exception_filter;
    }
    ReplayData* replay_data = *g_replay_data;
    if (!replay_data || !replay_data->is_recording) {
        return g_old_exception_filter;
    }
    helper::resize_replay_action_set(replay_data);
    if (replay_data->end) {
        if (replay_data->start + replay_data->size - replay_data->end + sizeof(Command_SetReplaySpeed) > 255) { // 255 is some buffer size, not yet investigated. -DF
            helper::new_replay_action(replay_data);
        }
    }
    if (replay_data->last_frame_time != g_elapsed_time_frames) {
        helper::new_replay_action(replay_data);
    }
    u8* replay_action = (u8*)(replay_data->size + replay_data->start);
    replay_action[0] = *g_active_storm_id;
    if (replay_data->size + 10 < replay_data->capacity) {
        replay_data->size += 11;
        replay_action[1] = GameCommand::GameCommand_SetReplaySpeed;
        replay_action[2] = true; // set_to_paused
        *(u32*)&replay_action[3] = g_options_list->speed;
        *(u32*)&replay_action[7] = g_speed_multiplier;
    }
    g_replay_end_frame = g_elapsed_time_frames + 72;
    helper::save_replay((char*)"_LastCrashReplay", true);
    return g_old_exception_filter;
}
} // namespace

namespace hooks {

void inject_replay_hooks() {
    jump_patch(replay_save_wrapper,				0x004EEBEA, 1);
    jump_patch(step_through_replay_data_wrapper, 0x004CDCE0, 1);

    call_patch(save_crash_replay, 0x004D10F1); // TopLevelExceptionFilter @ 4D0F70
}

} // namespace hooks
