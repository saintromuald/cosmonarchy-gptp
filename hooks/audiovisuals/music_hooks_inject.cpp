#include "music_hooks.h"

#include <hook_tools.h>

namespace {
void __declspec(naked) play_music_wrapper() { // 004A6140
    static u32 music_id;
    __asm {
        MOV music_id, EDI
        PUSHAD
    }
    hooks::play_music(music_id);
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) dialog_music_fade_wrapper() { // 004A6310
    static u32 music_id;
    __asm {
        MOV music_id, EAX
        PUSHAD
    }
    hooks::dialog_music_fade(music_id);
    __asm {
        POPAD
        RETN
    }
}
} // namespace

namespace hooks {
void inject_music_hooks() {
    jump_patch(play_music_wrapper,        0x004A6140, 3);
    jump_patch(dialog_music_fade_wrapper, 0x004A6310, 3);

    call_patch(start_game_music, 0x004D9966);
    call_patch(start_scorescreen_music, 0x004B4B79);
    jump_patch(update_sound_volumes, 0x004BC320);

    nop_patch(0x004D1D05, 5); // Original music_update() for menus
    nop_patch(0x004d4913, 5); // Duplicate title dialog_music_fade call
}
} // namespace hooks
