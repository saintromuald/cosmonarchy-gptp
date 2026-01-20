#pragma once
#include "BW/structures.h"

namespace hooks {
    void step_through_replay_data(GameAction* buffer); // 004CDCE0

    void inject_replay_hooks();
} // namespace hooks