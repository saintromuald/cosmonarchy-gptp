#pragma once
#include "types.h"

#include <json.hpp>

namespace ReplayKraken {

using Json = nlohmann::json;

struct InsertMatchRequestPlayer {
    // NOTE: camelCase required for macro to work as intended
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InsertMatchRequestPlayer, name, race, type, team, won, discordTag);

    std::string name;
    std::string race;
    std::string type;
    std::string team;
    bool        won;
    std::string discordTag;
};

struct InsertMatchRequest {
    // NOTE: camelCase required for macro to work as intended
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InsertMatchRequest, uuid, mapName, gameVersion, hype, players, uploaderId, replayBase64);

    std::string                           uuid;
    std::string                           mapName;
    std::string                           gameVersion;
    f32                                   hype;
    std::vector<InsertMatchRequestPlayer> players;
    u32                                   uploaderId;
    std::string                           replayBase64;
};

bool send_replay(const InsertMatchRequest& request);

} // namespace ReplayKraken

