#pragma once
#include "types.h"
#include "BW/enumerations.h"
#include "BW/structures.h"
#include <functional>
#include <span>
#include <unordered_map>
#include <string>

// Forward declarations and interface only
struct CommandResult {
    enum class Status { Success, InsufficientData, InvalidCommand, Abort };

    Status status = Status::Success;
    size_t bytes_consumed = 0;
    std::string error_message;

    static CommandResult success(size_t bytes);
    static CommandResult insufficient_data();
    static CommandResult invalid_command();
    static CommandResult abort(const std::string& reason = "");
};

using CommandProcessor = std::function<CommandResult(std::span<const u8>, u32)>;

struct CommandInfo {
    size_t required_size;
    CommandProcessor processor;
    std::string name;

    CommandInfo(size_t size, CommandProcessor proc, std::string n);
};

class LobbyCommandProcessor {
public:
    LobbyCommandProcessor();
    void process_command_data(std::span<const u8> packet_data, u32 sender_id);

private:
    CommandResult process_single_command(std::span<const u8> data, u32 sender_id);
    void register_handlers();

private:
    std::unordered_map<u8, CommandInfo> m_handlers;
};

void broadcast_player_update(Player& player);

void queue_lobby_commands();

inline LobbyCommandProcessor g_lobby_processor{};

namespace hooks {
void inject_lobby_command_hooks();
} // namespace hooks