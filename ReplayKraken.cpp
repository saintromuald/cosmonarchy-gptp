// NOTE: This file is not part of unity build,
//       so httplib doesn't see Windows.h includes we have everywhere,
//       which would lead to compiler errors
#include "pch.h"
#include "ReplayKraken.h"

#include <httplib.h>
#include <json.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <uuid.h>

using Json = nlohmann::json;

namespace ReplayKraken {

static uuids::uuid_name_generator g_uuid_generator{uuids::uuid_namespace_dns};

struct InsertMatchRequest {
    // NOTE: camelCase required for macro to work as intended
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InsertMatchRequest, matchUuid, gameVersion, replayFile, desyncFile);

    std::string matchUuid;
    std::string gameVersion;
    std::string replayFile;
    std::string desyncFile;
};

bool upload_replay(const std::vector<char>& uuid_data, const std::string& game_version,
    const std::vector<char>& replay_file, const std::vector<char>& desync_file) {
    InsertMatchRequest request{
        .matchUuid = uuids::to_string(g_uuid_generator(std::string{std::begin(uuid_data), std::end(uuid_data)})),
        .gameVersion = game_version,
        .replayFile = cppcodec::base64_rfc4648::encode(replay_file),
    };
    if (!desync_file.empty()) {
        request.desyncFile = cppcodec::base64_rfc4648::encode(desync_file);
    }

    const Json json(request);
    const auto json_str = json.dump();

    httplib::Client cli{"http://cosmonarchy.com"};
    for (int retry = 0; retry < 3; retry++) {
        if (const auto res = cli.Post("/api/matches", json_str, "application/json")) {
            if (res->status >= 200 && res->status < 300) {
                return true;
            }
        }
    }
    return false;
}

} // namespace ReplayKraken
