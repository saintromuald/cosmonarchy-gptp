#pragma once
#include <string>
#include <vector>

namespace ReplayKraken {

bool send_replay(const std::vector<char>& uuid_data, const std::string& game_version, const std::vector<char>& replay_file);

} // namespace ReplayKraken

