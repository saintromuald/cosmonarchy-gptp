#pragma once
#include <string>
#include <vector>
#include <fstream>

#include <uuid.h>
#include <httplib.h>
#include <json.hpp>

using Json = nlohmann::json;

struct InsertMatchRequest {
    // NOTE: camelCase required for macro to work as intended
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InsertMatchRequest, matchUuid, gameVersion, replayFile, desyncFile, randomsFile);

    std::string matchUuid;
    std::string gameVersion;
    std::string replayFile;
    std::string desyncFile;
    std::string randomsFile;
};

class ReplayKraken {
public:
    ReplayKraken();

    bool authenticate();

    bool upload_replay(
        const std::vector<char>& uuid_data,
        const std::string& game_version,
        const std::vector<char>& replay_file,
        const std::vector<char>& desync_file,
        const std::vector<char>& randoms_file,
        std::ofstream& logs
    );

    std::string token() const { return m_token; }
    std::string username() const { return m_username; }

private:
    httplib::Client m_api_client{"http://cosmonarchy.com"};
    uuids::uuid_name_generator m_uuid_generator{uuids::uuid_namespace_dns};
    std::string m_token;
    std::string m_username;
};

inline ReplayKraken g_replay_kraken;
