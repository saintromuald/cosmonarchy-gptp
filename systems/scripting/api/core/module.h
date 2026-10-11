#pragma once
#include "../script_common.h"
#include "../../../../BW/enumerations.h"
#include "control_flow.h"
#include "vars.h"
#include "prints.h"
#include "time.h"
#include "spatial.h"
#include "player.h"

namespace lua::api::core {

using Module = ScriptModule<
    UnitId::Enum,
    WeaponId::Enum,
    OrderId::Enum,
    ImageId::Enum,
    SpriteId::Enum,
    flingy_id::Enum,
    SoundId::Enum,
    IconId::Enum,
    PortraitId::Enum,
    PlayerType::Enum,
    RaceId::Enum,
    Faction,
    TextColor,

    ControlFlow,
    PlayerVars,
    Vars,
    Prints,
    Time,

    Point,
    Area,
    Player
>;

} // namespace lua::api::core
