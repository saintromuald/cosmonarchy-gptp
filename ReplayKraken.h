#pragma once
#include <string>
#include <vector>

namespace ReplayKraken {

bool upload_replay(const std::vector<char>& uuid_data, const std::string& game_version,
    const std::vector<char>& replay_file, const std::vector<char>& desync_file);

} // namespace ReplayKraken

