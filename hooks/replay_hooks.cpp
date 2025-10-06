#include "replay_hooks.h"
#include <fstream>
#include <BW/api.h>
#include <ReplayKraken.h>
#include <config.h>
#include <trigger_system\trigger_system.h>
#include <player_color.h>

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

void auto_save_replay() {
    constexpr std::string_view ext = ".rep";
    time_t now = time(0);
    tm* ltm = localtime(&now);
    std::string commit = bw::get_commit_hash();

    std::stringstream dir_ss;
    dir_ss << std::put_time(ltm, "%Y-%m-%d");
    if (!commit.empty()) {
        dir_ss << "_" << commit;
    }

    std::string dir = dir_ss.str();
    if (dir.size() > replay_name_size) {
        dir.resize(replay_name_size);
    }

    std::stringstream file_ss;
    if (!commit.empty()) {
        file_ss << commit << "_";
    }
    file_ss << std::put_time(ltm, "%H-%M") << "_" << g_current_map_name;

    std::string file = file_ss.str();
    if (file.size() > replay_name_size) {
        file.resize(replay_name_size);
    }

    /*
    bool first = true;
    for (u32 i = 0; i < PlayerLimit; ++i) {
        if (g_player_table[i].type != PlayerType::Human && g_player_table[i].type != PlayerType::Computer) {
            continue;
        }
        if (globals->players[i].is_witness) {
            continue;
        }
        if (first) {
            first = false;
            strcat_s(name, ", ");
        } else {
            strcat_s(name, " - ");
        }
        strcat_s(name, player_data.full_player_names[i].data);
    }
    */

    copy_last_replay(dir, file + std::string{ext});
}

void upload_last_replay() {
    std::ifstream from{get_replay_path() / "LastReplay.rep", std::ios::binary};
    if (!from) {
        return;
    }
    std::vector<char> replay{std::istreambuf_iterator<char>(from), std::istreambuf_iterator<char>()};
    ReplayKraken::send_replay(g_globals->uuid_rand, bw::get_commit_hash(), replay);
}

static bool should_upload_replay() {
    if (g_globals->forbid_replay_auto_upload) return false;
    return g_config.auto_upload_all_replays
        || g_globals->force_replay_auto_upload;
        // TODO: || bw::get_game_kind() == GameKind::Ranked;
}

} // namespace

namespace hooks {

void replay_save() {
    if (g_config.auto_save_replays) {
        auto_save_replay();
    }
    if (should_upload_replay()) {
        upload_last_replay();
    }
}

} // namespace hooks