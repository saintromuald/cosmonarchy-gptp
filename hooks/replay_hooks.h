#pragma once
#include "BW\bwdata.h"

namespace hooks {

void replay_save();									   // 004EEBEA
void step_through_replay_data(GameAction* action_set); // 004CDCE0

void inject_replay_hooks();

} // namespace hooks
