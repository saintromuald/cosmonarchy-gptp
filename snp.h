#pragma once

#include "BW/bwhelper.h"
#include "BW/bwdata.h"
#include "BW/Storm.h"
#include "BW/api.h"
#include <string>

namespace snp {
enum class CrownLinkStatus {
    Disconnected,
    Connecting,
    Ready
};

enum TurnsPerSecond {
    UltraLow    = 4,
    Low         = 6,
    Standard    = 8,
    Medium      = 10,
    High        = 12,
};

inline std::string to_string(CrownLinkStatus value) {
    switch (value) {
    case CrownLinkStatus::Disconnected: return "Disconnected";
    case CrownLinkStatus::Connecting: return "Connecting";
    case CrownLinkStatus::Ready: return "Ready";
    }
    return std::to_string(enums::to_base(value));
}

using StatusCallback = std::function<void(CrownLinkStatus)>;
using RegisterCallback = void(__stdcall*)(StatusCallback callback, bool enable_status_lobby, bool edit_name);
using SetTurnsPerSecond = bool(__stdcall*)(u32 turns_per_second);

class ProviderManager {
public:
    static ProviderManager& instance() {
        static ProviderManager provider_manager;
        return provider_manager;
    }

    std::vector<SPIProvider>& available_providers();
    std::vector<SPIProvider>& providers() { return m_providers; }
    bool is_crownlink();

    bool initialize_provider(u32 id);
    void destroy_provider();
    bool register_crownlink_status_callback(StatusCallback callback, bool enable_status_lobby, bool edit_name);
    bool set_turns_per_second(u32 turns_per_second);

private:
    ProviderManager() = default;

    static b32 __stdcall add_provider_to_list(u32 id, const char* name, const char* description, Caps* caps);

private:
    std::vector<SPIProvider> m_providers;
    u32 m_provider_id;
    RegisterCallback m_register_callback;
    SetTurnsPerSecond m_set_turns_per_second;
};
} // namespace snp