#pragma once
#include "../script_common.h"
#include "attack_wave.h"
#include "defense.h"
#include "town.h"
#include "ai.h"

namespace lua::api::ai {

using Module = ScriptModule<
    DefenseArray,
    DefenseBuild,
    DefenseUse,
    Defense,
    Town,
    AttackWave,
    AI
>;

} // namespace lua::api::ai
