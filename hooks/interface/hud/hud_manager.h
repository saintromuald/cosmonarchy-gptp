#pragma once
#include "BW/enumerations.h"
#include "BW/structures.h"
#include "storm/sfile.h"
#include "systems/graphics/grp.h"
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

class HudIcons {
public:
    enum class Kind { Mineral, Gas, Supply, General };

    HudIcons() {
        m_mineral = sfile::load_file("game\\mineralicons.grp").unsafe_cast<graphics::GrpFile>();
        m_gas     = sfile::load_file("game\\gasicons.grp").unsafe_cast<graphics::GrpFile>();
        m_supply  = sfile::load_file("game\\supplyicons.grp").unsafe_cast<graphics::GrpFile>();
        m_general = sfile::load_file("game\\icons.grp").unsafe_cast<graphics::GrpFile>();
    }

    graphics::GrpFile* mineral() { return m_mineral.get(); }
    graphics::GrpFile* gas()     { return m_gas.get(); }
    graphics::GrpFile* supply()  { return m_supply.get(); }
    graphics::GrpFile* general() { return m_general.get(); }

    graphics::GrpFile* get(Kind kind) {
        switch (kind) {
            case Kind::Mineral: return mineral();
            case Kind::Gas:     return gas();
            case Kind::Supply:  return supply();
            case Kind::General: return general();
        }
        return nullptr;
    }

private:
    smem::ptr<graphics::GrpFile> m_mineral;
    smem::ptr<graphics::GrpFile> m_gas;
    smem::ptr<graphics::GrpFile> m_supply;
    smem::ptr<graphics::GrpFile> m_general;
};

// Frees a dialog's create-time backdrop cache and re-blits it from the HUD
// mask at the dialog's current bounds. Run after placing a root dialog.
void refresh_dialog_backdrop(bw::Dialog* dialog);

class StatResource;

class HudManager {
public:
    struct Config {
        // Split HUD art: exactly surface width, bottom-anchored, palette index 0 transparent.
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

    // Repaints HUD dialogs dirtied by world rendering; called once per frame from present_frame.
    void invalidate_dirty_dialogs();

    // Exclude HUD art + dialog mask from game present. Nonzero = HUD owned pixel
    const u8* present_protection() const { return m_hidden ? nullptr : m_mask.data(); }

    bool covers(s32 x, s32 y) const;
    bool loaded() const { return m_loaded; }

    const Config& current() const;
    std::string_view current_name() const { return m_current_name; }
    const ConfigMap& available() const { return m_configs; }
    bool has_hud(std::string_view name) const;

    HudIcons& icons() { return m_icons; }

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
    void finish_hud_image();
    void stamp_hud_dialogs_into_mask();
    void write_dialog_mask_stamp(bw::Dialog* dialog);
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
    bool m_loaded = false;
    HudViewMode m_view_mode = HudViewMode::Visible;
    bool m_hotkey_enabled = true;
    std::vector<StatflufDialog> m_statfluf_dialogs;

    HudIcons m_icons;
    std::unique_ptr<StatResource> m_stat_resource;

    // Number of frames to force full HUD repaints after placement. See invalidate_dirty_dialogs.
    u32 m_forced_repaints = 0;

    // Surface-sized HUD art + dialog mask, bottom-anchored: nonzero = HUD owned
    // pixel. 004C35F0 reads it via g_hud_image for dialog backdrops.
    std::vector<u8> m_mask;

    // Original bounds + flags of dialogs we reposition (rather than hide)
    // in the hidden view mode. Captured on hide, restored on show
    DialogBounds m_saved_statport_bounds{};
    DialogBounds m_saved_stat_f10_bounds{};
    DialogFlags::Enum m_saved_statport_flags{};
    DialogFlags::Enum m_saved_stat_f10_flags{};
};

inline std::unique_ptr<HudManager> g_hud_manager;
