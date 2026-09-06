#include "pch.h"
#include "ReplayKraken.h"

#include <exception>
#include <format>
#include <stdexcept>

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

ReplayKraken::~ReplayKraken() {
    shutdown();
}

void ReplayKraken::shutdown() {
    {
        std::lock_guard lock{m_queue_mutex};
        m_stopping = true;
        m_upload_queue.clear();
    }
    m_api_client.stop();
    m_queue_condition.notify_all();
    if (m_upload_thread.joinable()) {
        m_upload_thread.join();
    }
}

void ReplayKraken::log(std::string_view message) {
    std::lock_guard lock{m_log_mutex};
    m_logs << message << std::endl;
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
    if (g_globals->desync_state.had_desync()) {
        log("Replay auto-upload is enabled for desynced games.");
        return true;
    }
    if (g_globals->forbid_replay_auto_upload) {
        log("Replay auto-upload is forbidden by the config.");
        return false;
    }
    if (bw::human_or_defeated_human_count() < 2) {
        log("Replay auto-upload is disabled for single-player games.");
        return false;
    }
    if (bw::get_game_kind() == GameKind::Ranked) {
        log("Replay auto-upload is enabled for ranked games.");
        return true;
    }
    log("Replay auto-upload is disabled for unranked non-desynced games.");
    return false;
}

void ReplayKraken::upload_last_replay() {
    try {
        log(std::format("Attempting to upload replay at {:%Y-%m-%d %H:%M:%S}",
            std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())));
        if (!should_upload_last_replay()) {
            return;
        }

        std::ifstream replay_stream{bw::get_replay_path() / "LastReplay.rep", std::ios::binary};
        if (!replay_stream) {
            log("Failed to open LastReplay.rep for reading.");
            return;
        }

        replay_stream.seekg(0, std::ios::end);
        const std::streamoff replay_size = replay_stream.tellg();
        if (replay_size <= 0) {
            log("LastReplay.rep is empty or its size could not be read.");
            return;
        }
        replay_stream.seekg(0, std::ios::beg);

        std::string replay(static_cast<size_t>(replay_size), '\0');
        replay_stream.read(replay.data(), static_cast<std::streamsize>(replay.size()));
        if (replay_stream.gcount() != static_cast<std::streamsize>(replay.size())) {
            log("Failed to read all of LastReplay.rep.");
            return;
        }

        std::string desync;
        std::string randoms;
        if (g_globals->desync_state.had_desync()) {
            log("Game had desync. Preparing desync and randoms data for upload.");
            size_t desync_size = 0;
            for (const auto* logger : gptp::g_loggers) {
                desync_size += logger->buffered_view().size();
                desync_size += std::string_view{"============  ============\n\n\n\n"}.size();
                desync_size += std::string_view{logger->name()}.size();
            }
            desync.reserve(desync_size);
            for (const auto* logger : gptp::g_loggers) {
                desync.append("============ ");
                desync.append(logger->name());
                desync.append(" ============\n\n");
                desync.append(logger->buffered_view());
                desync.append("\n\n");
            }
            randoms = bw::g_random_logger.buffered_string();
        }

        enqueue_upload(UploadJob{
            .uuid_data = std::string{g_globals->uuid_rand.begin(), g_globals->uuid_rand.end()},
            .game_version = bw::get_commit_hash(),
            .replay_file = std::move(replay),
            .desync_file = std::move(desync),
            .randoms_file = std::move(randoms),
        });
        log("Replay upload queued.");
    } catch (const std::exception& ex) {
        log(std::format("Failed to queue replay upload: {}", ex.what()));
    } catch (...) {
        log("Failed to queue replay upload with an unknown error.");
    }
}

void ReplayKraken::enqueue_upload(UploadJob job) {
    {
        std::lock_guard lock{m_queue_mutex};
        if (m_stopping) {
            throw std::runtime_error{"Replay upload worker is shutting down"};
        }
        if (!m_upload_thread.joinable()) {
            m_upload_thread = std::thread{&ReplayKraken::upload_worker, this};
        }
        m_upload_queue.push_back(std::move(job));
    }
    m_queue_condition.notify_one();
}

void ReplayKraken::upload_worker() {
    while (true) {
        UploadJob job;
        {
            std::unique_lock lock{m_queue_mutex};
            m_queue_condition.wait(lock, [this] {
                return m_stopping || !m_upload_queue.empty();
            });
            if (m_stopping && m_upload_queue.empty()) {
                return;
            }
            job = std::move(m_upload_queue.front());
            m_upload_queue.pop_front();
        }

        try {
            upload_replay(
                std::move(job.uuid_data),
                std::move(job.game_version),
                std::move(job.replay_file),
                std::move(job.desync_file),
                std::move(job.randoms_file)
            );
        } catch (const std::exception& ex) {
            log(std::format("Replay upload failed with an exception: {}", ex.what()));
        } catch (...) {
            log("Replay upload failed with an unknown exception.");
        }
    }
}

bool ReplayKraken::upload_replay(
    std::string uuid_data,
    std::string game_version,
    std::string replay_file,
    std::string desync_file,
    std::string randoms_file
) {
    const auto replay_size = replay_file.size();
    const auto desync_size = desync_file.size();
    const auto randoms_size = randoms_file.size();
    const auto match_uuid = uuids::to_string(m_uuid_generator(uuid_data));

    InsertMatchRequest request{
        .matchUuid = match_uuid,
        .gameVersion = std::move(game_version),
    };

    request.replayFile = cppcodec::base64_rfc4648::encode(replay_file);
    std::string{}.swap(replay_file);

    if (!desync_file.empty()) {
        request.desyncFile = cppcodec::base64_rfc4648::encode(desync_file);
        std::string{}.swap(desync_file);
    }

    if (!randoms_file.empty()) {
        request.randomsFile = cppcodec::base64_rfc4648::encode(randoms_file);
        std::string{}.swap(randoms_file);
    }

    Json json(request);
    request = {};
    const auto json_str = json.dump();
    json.clear();

    log(std::format(
        "Uploading replay {} (replay: {} bytes, desync: {} bytes, randoms: {} bytes, request: {} bytes).",
        match_uuid, replay_size, desync_size, randoms_size, json_str.size()
    ));
    for (int retry = 0; retry < UploadAttempts; retry++) {
        log(std::format("Attempt {} to upload replay...", retry + 1));
        if (const auto res = m_api_client.Post("/api/matches", json_str, "application/json")) {
            if (res->status >= 200 && res->status < 300) {
                log("Replay uploaded successfully.");
                return true;
            }
            log(std::format("Failed to upload replay. Status: {}, Body: {}", res->status, res->body));
        }
    }
    log(std::format("Failed to upload replay after {} attempts.", UploadAttempts));
    return false;
}
