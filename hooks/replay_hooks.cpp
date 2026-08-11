#include "replay_hooks.h"
#include <fstream>
#include <BW/api.h>
#include <ReplayKraken.h>
#include <config.h>
#include <trigger_system\trigger_system.h>
#include <player_color.h>
#include "../hook_tools.h"
#include <future>
#include <utility>
#include "logger.h"
#include <system_error>
#include <filesystem>

namespace {

constexpr std::string_view replay_ext = ".ext";
constexpr auto replay_full_size = 31; // Some weird magic BW const we would want to extend 
constexpr auto replay_name_size = replay_full_size - replay_ext.size();

fs::path get_replay_path() {
    return bw::get_starcraft_path() / "maps" / "replays";
}

void copy_last_replay(const std::string& dir, const std::string& name) {
    std::ifstream from{get_replay_path() / "LastReplay.rep", std::ios::binary};
    if (!from) {
        return;
    }

    fs::path to_path = get_replay_path() / "Autosave" / dir / name;
    if (fs::exists(to_path)) {
        return; // Do not overwrite existing files
    }

    std::error_code error;
    fs::create_directories(to_path.parent_path(), error);

    std::ofstream to{to_path, std::ios::binary};
    if (!to) {
        return;
    }

    to << from.rdbuf();
}

class ReplayUploader {
public:
    ReplayUploader() {
        std::error_code ec;
        if (fs::create_directories("logs/", ec)) {
            m_logs.open("logs/replay_kraken.log", std::ios::out | std::ios::app);
        }
    }

    bool should_upload() {
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

    void upload() {
        m_logs << "Attempting to upload replay at "
            << std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()))
            << std::endl;
        if (!should_upload()) {
            return;
        }

        std::ifstream replay_stream{get_replay_path() / "LastReplay.rep", std::ios::binary};
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
        std::async(std::launch::async, [replay = std::move(replay), desync = std::move(desync), randoms = std::move(randoms), logs = std::move(m_logs)] mutable {
            g_replay_kraken.upload_replay(
                g_globals->uuid_rand,
                bw::get_commit_hash(),
                replay,
                desync,
                randoms,
                logs
            );
        });
    }

private:
    std::ofstream m_logs;
};

void auto_save_replay() {
    constexpr std::string_view ext = ".rep";
    const auto now = time(0);
    const auto ltm = localtime(&now);
    const auto commit = bw::get_commit_hash();

    std::stringstream dir_ss;
    dir_ss << std::put_time(ltm, "%Y-%m-%d");
    if (!commit.empty()) {
        dir_ss << "_" << commit;
    }

    auto dir = dir_ss.str();
    if (dir.size() > replay_name_size) {
        dir.resize(replay_name_size);
    }

    std::stringstream file_ss;
    if (!commit.empty()) {
        file_ss << commit << "_";
    }
    file_ss << std::put_time(ltm, "%H-%M") << "_" << g_current_map_name;

    auto file = file_ss.str();
    if (file.size() > replay_name_size) {
        file.resize(replay_name_size);
    }

    copy_last_replay(dir, file + std::string{ext});
}

} // namespace

namespace hooks {

void replay_save() {
    if (g_config.auto_save_replays) {
        auto_save_replay();
    }
    ReplayUploader uploader;
    uploader.upload();
}

// Hooked to investigate the replay score screen freeze. -Pr0nogo
void step_through_replay_data(GameAction* action_set) { // 004CDCE0
    s32 action = action_set->start;
    if (!action) {
        return; // Fixes crash when no action is taken
    }
    s32 size = action_set->size;
    s32 action_end = size + action;

    s32 command_size = 0;
    s32 data = 0;
    while (action < action_end) {
        s32 action_length = *(s32*)(action + 4);
        action += 5;
        for (s32 current_length = 0; current_length < action_length; action = command_size + data) {
            s32 command_id = *(s32*)(action + 1);
            command_size = g_turn_command_size[command_id];
            if (command_size < 0) {
                return;
            }
            data = action + 1;
            if (command_id >= 9 && command_id <= 11) {
                command_size = 2 * data[(s32*)1] + 2;
            }
            current_length += command_size + 1;
        }
    }
}

} // namespace hooks