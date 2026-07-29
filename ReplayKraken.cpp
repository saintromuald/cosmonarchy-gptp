#include "pch.h"
#include "ReplayKraken.h"

#include <exception>
#include <format>

#include <cppcodec/base64_rfc4648.hpp>
#include <Windows.h>

static std::string read_token() {
    BYTE buffer[512]{};
    DWORD size = sizeof(buffer);
    const auto result = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\The No-Frauds Club\\Cosmonarchy", "AccountToken",
        RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY, nullptr, buffer, &size);
    if (result != ERROR_SUCCESS) {
        return {};
    }
    return reinterpret_cast<const char*>(buffer);
}

ReplayKraken::ReplayKraken() {
    m_token = read_token();
    if (!m_token.empty()) {
        m_api_client.set_default_headers({
            {"Authorization", "Bearer " + m_token},
        });
        authenticate();
    }
}

bool ReplayKraken::authenticate() {
    for (int retry = 0; retry < 3; retry++) {
        if (const auto res = m_api_client.Get("/api/users/@me")) {
            if (res->status == 200) {
                const auto json = Json::parse(res->body);
                try {
                    m_username = json.at("username").get<std::string>();
                    return true;
                } catch (const std::exception& ex) {
                    return false;
                }
            }
        }
    }
    return false;
}

bool ReplayKraken::upload_replay(const std::vector<char>& uuid_data, const std::string& game_version,
    const std::vector<char>& replay_file, const std::vector<char>& desync_file, const std::vector<char>& randoms_file) {
    InsertMatchRequest request{
        .matchUuid = uuids::to_string(m_uuid_generator(std::string{std::begin(uuid_data), std::end(uuid_data)})),
        .gameVersion = game_version,
        .replayFile = cppcodec::base64_rfc4648::encode(replay_file),
        .desyncFile = !desync_file.empty() ? cppcodec::base64_rfc4648::encode(desync_file) : "",
        .randomsFile = !randoms_file.empty() ? cppcodec::base64_rfc4648::encode(randoms_file) : "",
    };

    const Json json(request);
    const auto json_str = json.dump();
    for (int retry = 0; retry < 3; retry++) {
        if (const auto res = m_api_client.Post("/api/matches", json_str, "application/json")) {
            if (res->status >= 200 && res->status < 300) {
                return true;
            }
        }
    }
    return false;
}

