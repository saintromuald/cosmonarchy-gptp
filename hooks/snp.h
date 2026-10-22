#pragma once

#include "BW/enumerations.h"
#include "BW/bwhelper.h"
#include "BW/bwdata.h"
#include "storm/storm.h"
#include "BW/api.h"

#include <array>
#include <bit>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>


namespace snp {
enum class TurnsPerSecond : u32 {
    CNLK = 8,  // For Backwards Compatibility
    CLDB = 4,  // For Backwards Compatibility
    UltraLow = 4,
    Low = 6,
    Standard = 8,
    Medium = 10,
    High = 12
};

using StatusCallback = void(*)(CrowServeState state, void* user_data);
using LobbyCallback = void(*)(const AdFile*, P2PState peer_status, void*);
using RegisterCallback = void(__stdcall*)(StatusCallback callback, void* user_data);
using IterateLobbies = void(__stdcall*)(LobbyCallback callback, void* user_data);
using SetTurnsPerSecond = b32(__stdcall*)(TurnsPerSecond turns_per_second);
using GetTurnsPerSecond = TurnsPerSecond(__stdcall*)();
using SetLobbyPassword = b32(__stdcall*)(const char* password);
using GetLobbyPassword = b32(__stdcall*)(char* output, u32 output_size);
using GetStatus = CrowServeState(__stdcall*)();
using GetVersion = u32(__stdcall*)();
using SetStatusLobby = void(__stdcall*)(bool enable);
using SetMapNameEdit = void(__stdcall*)(bool enable);

struct CrownLinkApi {
    RegisterCallback register_callback;
    IterateLobbies iterate_lobbies;
    SetTurnsPerSecond set_turns_per_second;
    GetTurnsPerSecond get_turns_per_second;
    SetLobbyPassword set_lobby_password;
    GetLobbyPassword get_lobby_password;
    SetStatusLobby set_status_lobby;
    SetMapNameEdit set_map_name_edit;
    GetStatus get_status;
    GetVersion get_version;
};

struct ProviderId {
    u32 id;

    constexpr ProviderId(const char(&str)[5]) {
        std::array<char, 4> chars = {str[3], str[2], str[1], str[0]};
        id = std::bit_cast<u32>(chars);
    }

    constexpr ProviderId(u32 provider_id) : id(provider_id) {}

    std::string to_string() const {
        auto chars = std::bit_cast<std::array<char, 4>>(id);
        return std::string{chars[3], chars[2], chars[1], chars[0]};
    }

    constexpr operator u32() const { return id; }

    friend std::ostream& operator<<(std::ostream& os, const ProviderId& pid) {
        return os << pid.to_string();
    }
};

class ProviderManager {
public:
    static ProviderManager& instance() {
        static ProviderManager provider_manager;
        return provider_manager;
    }

    std::vector<SPIProvider>& available_providers();
    bool provider_available(ProviderId);
    std::vector<SPIProvider>& providers() { return m_providers; }
    bool is_crownlink();

    bool initialize_provider(ProviderId id);
    void destroy_provider();
    u32 provider_id() const { return m_provider_id; };
    bool register_crownlink_status_callback(StatusCallback callback, void* user_data);
    bool iterate_lobbies(LobbyCallback callback, void* user_data);
    bool set_turns_per_second(TurnsPerSecond turns_per_second);
    bool get_turns_per_second(TurnsPerSecond& output);
    bool set_lobby_password(const char* password);
    bool get_lobby_password(std::string& output);
    void set_status_lobby(bool enable);
    void set_map_name_edit(bool enable);
    bool get_crownlink_status(CrowServeState& output);
    bool get_crownlink_version(u32 output);

private:
    ProviderManager() = default;

    static b32 __stdcall add_provider_to_list(u32 id, const char* name, const char* description, Caps* caps);

private:
    std::vector<SPIProvider> m_providers;
    u32 m_provider_id;
    CrownLinkApi m_crownlink_api;
};
} // namespace snp

template <>
struct std::formatter<snp::ProviderId> : std::formatter<std::string_view> {
    auto format(const snp::ProviderId& pid, format_context& ctx) const {
        return std::formatter<std::string_view>::format(pid.to_string(), ctx);
    }
};

namespace hooks {
bool glu_connect_init_provider();
void glu_destroy_provider();
void inject_snp_hooks();
}