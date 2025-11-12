#include "replay_hooks.h"
#include <fstream>
#include <BW/api.h>
#include <ReplayKraken.h>
#include <config.h>
#include <trigger_system\trigger_system.h>
#include <player_color.h>
#include "../hook_tools.h"

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

static bool should_auto_upload_replay() {
    if (g_globals->forbid_replay_auto_upload || bw::human_count() < 2) {
        return false;
    }
    return bw::get_game_kind() == GameKind::Ranked || g_globals->had_desync;
}

void upload_last_replay() {
    std::ifstream replay_stream{get_replay_path() / "LastReplay.rep", std::ios::binary};
    if (!replay_stream) {
        return;
    }
    std::vector<char> replay{std::istreambuf_iterator<char>(replay_stream), std::istreambuf_iterator<char>()};

    std::vector<char> desync;
    if (g_globals->had_desync) {
        if (std::ifstream desync_stream{"desync.txt", std::ios::binary}) {
            desync.assign(std::istreambuf_iterator<char>(desync_stream), std::istreambuf_iterator<char>());
        }
    }

    ReplayKraken::upload_replay(g_globals->uuid_rand, bw::get_commit_hash(), replay, desync);
}

} // namespace

namespace hooks {

void replay_save() {
    if (g_config.auto_save_replays) {
        auto_save_replay();
    }
    if (should_auto_upload_replay()) {
        upload_last_replay();
    }
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