#pragma once
#include "BW/enumerations.h"
#include "BW/structures.h"
#include "storm/sfile.h"
#include "utils/rfl.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

enum class HudViewMode {
    Visible,
    Hidden,
    Cinematic,
};

class LogoBanner;

class HudIcons {
public:
    HudIcons() {
        m_mineral = sfile::load_file("game\\mineralicons.grp").unsafe_cast<GrpFile>();
        m_gas     = sfile::load_file("game\\gasicons.grp").unsafe_cast<GrpFile>();
        m_supply  = sfile::load_file("game\\supplyicons.grp").unsafe_cast<GrpFile>();
        m_general = sfile::load_file("game\\icons.grp").unsafe_cast<GrpFile>();
    }

    GrpFile* mineral() { return m_mineral.get(); }
    GrpFile* gas()     { return m_gas.get(); }
    GrpFile* supply()  { return m_supply.get(); }
    GrpFile* general() { return m_general.get(); }

private:
    smem::ptr<GrpFile> m_mineral;
    smem::ptr<GrpFile> m_gas;
    smem::ptr<GrpFile> m_supply;
    smem::ptr<GrpFile> m_general;
};

class HudManager {
public:
    struct Config {
        SFilePath image;
        SFilePath hud_overlay;
        SFilePath progressbar_empty;
        SFilePath progressbar_full;
        SFilePath command_buttons;
        SFilePath command_buttons_bin;

        u32 mineral_icon = 0;
        u32 gas_icon = 0;
        u32 supply_icon = 0;

        std::vector<s16> chat_textbox_bounds{}; // TODO: Need to implement this -Jesse
        std::optional<DialogBounds> chat_textbox() const;
    };

    using ConfigMap = std::map<std::string, Config>;

    static void install(std::string_view name);
    static void install(RaceId::Enum race);
    static void install_replay();
    ~HudManager();

    HudManager(const HudManager&) = delete;
    HudManager& operator=(const HudManager&) = delete;
    HudManager(HudManager&&) = delete;
    HudManager& operator=(HudManager&&) = delete;

    void change(std::string_view name);

    void set_view_mode(HudViewMode mode);
    HudViewMode view_mode() const { return m_view_mode; }

    void set_hotkey_enabled(bool enabled) { m_hotkey_enabled = enabled; }
    bool hotkey_enabled() const { return m_hotkey_enabled; }

    void hide();
    void show();
    bool is_hidden() const { return m_hidden; }

    const Config& current() const;
    std::string_view current_name() const { return m_current_name; }
    const ConfigMap& available() const { return m_configs; }
    bool has_hud(std::string_view name) const;

private:
    HudManager() = default;

    void load_definitions();
    void resolve_current(std::string_view name);
    void load_all();
    void load_hud_unique();
    void unload_all();
    void unload_hud_unique();
    static void reset_ui_state();

    void load_hud_image();
    void load_statfluf_decoration_dialogs();
    void unload_statfluf_mouse_regions();
    void load_command_buttons();
    void load_textbox();
    void load_statlb();
    void load_wireframe_palette();
    void load_statf10();

    void resolve_statfluf_regions();
    void apply_visibility(bool visible);

    struct StatflufDialog {
        DialogBounds bounds;
        bw::Dialog* dialog = nullptr;
    };

private:
    ConfigMap m_configs;
    const Config* m_current = nullptr;
    std::string m_current_name;
    bool m_hidden = false;
    HudViewMode m_view_mode = HudViewMode::Visible;
    bool m_hotkey_enabled = true;
    std::vector<StatflufDialog> m_statfluf_dialogs;
    std::unique_ptr<LogoBanner> m_logo_banner;

    // While hidden we swap masks
    void* m_transparent_mask = nullptr;
    void* m_saved_hud_mask = nullptr;

    // Original bounds + flags of dialogs we reposition (rather than hide)
    // in the hidden view mode. Captured on hide, restored on show
    DialogBounds m_saved_statport_bounds{};
    DialogBounds m_saved_stat_f10_bounds{};
    DialogFlags::Enum m_saved_statport_flags{};
    DialogFlags::Enum m_saved_stat_f10_flags{};
};

inline std::unique_ptr<HudManager> g_hud_manager;
