#pragma once
#include "types.h"

namespace hooks {
void play_music(u32 music_id);          // 004A6140
void dialog_music_fade(u32 music_id);   // 004A6310
void start_game_music();                // 004D9966
void start_scorescreen_music();         // 004B4B79
void update_sound_volumes();            // 004BC320
void inject_music_hooks();
} // namespace hooks
