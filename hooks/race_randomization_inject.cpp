#include "race_randomization.h"
#include <hook_tools.h>

namespace hooks {
void inject_randomization_hooks() {
    call_patch(randomize_races, 0x004a9d69); // init_singleplayer_custom (hooked)
    call_patch(randomize_races, 0x004ad8d6); // init_singleplayer_custom (hooked)
    call_patch(randomize_races, 0x004ef20e); // main_game_run
}
}