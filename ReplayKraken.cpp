#include "pch.h"
#include "ReplayKraken.h"

#include <exception>
#include <format>
#include <future>

#include <cppcodec/base64_rfc4648.hpp>
#include <Windows.h>

#include "BW/api.h"
#include "systems/triggers/trigger_system.h"
#include "logger.h"

static constexpr auto UploadAttempts = 3;

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
    std::error_code ec;
    fs::create_directories("logs/", ec);
    m_logs.open("logs/replay_kraken.log", std::ios::out | std::ios::app);

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

bool ReplayKraken::should_upload_last_replay() {
    if (g_globals->forbid_replay_auto_upload) {
        m_logs << "Replay auto-upload is forbidden by the config." << std::endl;
        return false;
    }
    if (bw::human_or_defeated_human_count() < 2) {
        m_logs << "Replay auto-upload is disabled for single-player games." << std::endl;
        return false;
    }
    if (bw::get_game_kind() == GameKind::Ranked) {
        m_logs << "Replay auto-upload is enabled for ranked games." << std::endl;
        return true;
    }
    if (g_globals->had_desync) {
        m_logs << "Replay auto-upload is enabled for desynced games." << std::endl;
        return true;
    }
    m_logs << "Replay auto-upload is disabled for unranked non-desynced games." << std::endl;
    return false;
}

void ReplayKraken::upload_last_replay() {
    m_logs << std::format("Attempting to upload replay {} at ", g_globals->uuid_rand)
        << std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()))
        << std::endl;
    if (!should_upload_last_replay()) {
        return;
    }

    std::ifstream replay_stream{bw::get_replay_path() / "LastReplay.rep", std::ios::binary};
    if (!replay_stream) {
        m_logs << "Failed to open LastReplay.rep for reading." << std::endl;
        return;
    }
    std::vector<char> replay{std::istreambuf_iterator<char>(replay_stream), std::istreambuf_iterator<char>()};

    std::vector<char> desync;
    std::vector<char> randoms;
    if (g_globals->had_desync) {
        m_logs << "Game had desync. Preparing desync and randoms data for upload." << std::endl;
        std::stringstream desync_file;
        for (const auto* logger : gptp::g_loggers) {
            desync_file << "============ " << logger->name() << " ============\n\n";
            desync_file << logger->buffered_string();
            desync_file << "\n\n";
        }
        std::string desync_str = desync_file.str();
        desync.assign(desync_str.begin(), desync_str.end());

        std::string randoms_str = bw::g_random_logger.buffered_string();
        randoms.assign(randoms_str.begin(), randoms_str.end());
    }

    m_logs << "Replay upload initiated." << std::endl;
    std::async(std::launch::async, [replay = std::move(replay), desync = std::move(desync), randoms = std::move(randoms)] {
        g_replay_kraken.upload_replay(
            g_globals->uuid_rand,
            bw::get_commit_hash(),
            replay,
            desync,
            randoms
        );
    });
}

bool ReplayKraken::upload_replay(
    const std::vector<char>& uuid_data,
    const std::string& game_version,
    const std::vector<char>& replay_file,
    const std::vector<char>& desync_file,
    const std::vector<char>& randoms_file
) {
    InsertMatchRequest request{
        .matchUuid = uuids::to_string(m_uuid_generator(std::string{std::begin(uuid_data), std::end(uuid_data)})),
        .gameVersion = game_version,
        .replayFile = cppcodec::base64_rfc4648::encode(replay_file),
        .desyncFile = !desync_file.empty() ? cppcodec::base64_rfc4648::encode(desync_file) : "",
        .randomsFile = !randoms_file.empty() ? cppcodec::base64_rfc4648::encode(randoms_file) : "",
    };

    const Json json(request);
    const auto json_str = json.dump();
    m_logs << std::format("Uploading replay: {}", json_str) << std::endl;
    for (int retry = 0; retry < UploadAttempts; retry++) {
        m_logs << std::format("Attempt {} to upload replay...", retry + 1) << std::endl;
        if (const auto res = m_api_client.Post("/api/matches", json_str, "application/json")) {
            if (res->status >= 200 && res->status < 300) {
                m_logs << "Replay uploaded successfully." << std::endl;
                return true;
            } else {
                m_logs << std::format("Failed to upload replay. Status: {}, Body: {}", res->status, res->body) << std::endl;
            }
        }
    }
    m_logs << std::format("Failed to upload replay after {} attempts.", UploadAttempts) << std::endl;
    return false;
}

