#pragma once
#include <condition_variable>
#include <deque>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

#include <stduuid/uuid.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

using Json = nlohmann::json;

struct InsertMatchRequest {
    // NOTE: camelCase required for macro to work as intended
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InsertMatchRequest, matchUuid, gameVersion, replayFile, desyncFile, randomsFile, meta);

    std::string matchUuid;
    std::string gameVersion;
    std::string replayFile;
    std::string desyncFile;
    std::string randomsFile;
    std::optional<Json> meta;
};

class ReplayKraken {
public:
    ReplayKraken();
    ~ReplayKraken();

    ReplayKraken(const ReplayKraken&) = delete;
    ReplayKraken& operator=(const ReplayKraken&) = delete;

    bool authenticate();

    bool should_upload_last_replay();
    void upload_last_replay();
    void shutdown();

    bool upload_replay(
        std::string uuid_data,
        std::string game_version,
        std::string replay_file,
        std::string desync_file,
        std::string randoms_file,
        Json meta
    );

    std::string token() const { return m_token; }
    std::string username() const { return m_username; }

private:
    struct UploadJob {
        std::string uuid_data;
        std::string game_version;
        std::string replay_file;
        std::string desync_file;
        std::string randoms_file;
        Json meta;
    };

    void enqueue_upload(UploadJob job);
    void upload_worker();
    void log(std::string_view message);

    /// Players, map, matchup and result for the finished game, as the
    /// self-hosted fork's `meta` payload. Empty object when no metadata
    /// could be gathered (never fails the upload).
    static Json build_meta();

    std::ofstream m_logs;
    std::mutex m_log_mutex;
    std::unique_ptr<httplib::Client> m_api_client;
    std::string m_server_url;
    uuids::uuid_name_generator m_uuid_generator{uuids::uuid_namespace_dns};
    std::string m_token;
    std::string m_username;
    bool m_upload_enabled = false;

    std::mutex m_queue_mutex;
    std::condition_variable m_queue_condition;
    std::deque<UploadJob> m_upload_queue;
    bool m_stopping = false;
    std::thread m_upload_thread;
};

inline ReplayKraken g_replay_kraken;