#include "hud_manager.h"

#include "BW/api.h"
#include "BW/bwdata.h"
#include "BW/bwhelper.h"
#include "BW/ui.h"
#include "hud.h"
#include "logo_banner.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "storm/storm.h"
#include "storm/smem.hpp"

#include <format>
#include <memory>

#include <rfl/yaml.hpp>

namespace {
struct DefinitionsFile {
    HudManager::ConfigMap huds;
};

constexpr auto DefinitionsPath = "game\\huds.yml";
} // namespace

std::optional<DialogBounds> HudManager::Config::chat_textbox() const {
    if (chat_textbox_bounds.size() != 4) return std::nullopt;
    return DialogBounds::from_xywh(
        chat_textbox_bounds[0],
        chat_textbox_bounds[1],
        static_cast<u16>(chat_textbox_bounds[2]),
        static_cast<u16>(chat_textbox_bounds[3])
    );
}

void HudManager::install(std::string_view name) {
    g_hud_manager.reset(new HudManager());
    g_hud_manager->load_definitions();
    g_hud_manager->resolve_current(name);
    g_hud_manager->load_all();
    reset_ui_state();

    g_hud_manager->m_logo_banner = std::make_unique<LogoBanner>();
    g_hud_manager->m_logo_banner->hide();
}

HudManager::~HudManager() {
    m_logo_banner.reset();
    if (m_hidden && g_hud_surface && m_saved_hud_mask) {
        g_hud_surface->surface = m_saved_hud_mask;
        m_saved_hud_mask = nullptr;
    }
    unload_all();
}

void HudManager::change(std::string_view name) {
    const auto was_hidden = m_hidden;
    if (was_hidden) {
        show();
    }
    unload_hud_unique();
    resolve_current(name);
    load_hud_unique();
    reset_ui_state();
    if (was_hidden) {
        hide();
    }
}

void HudManager::set_view_mode(HudViewMode mode) {
    switch (mode) {
        case HudViewMode::Visible: {
            show();
            m_logo_banner->show_logo(true);
            set_hotkey_enabled(true);
        } break;
        case HudViewMode::Hidden: {
            hide();
            m_logo_banner->show_logo(true);
            set_hotkey_enabled(true);
        } break;
        case HudViewMode::Cinematic: {
            hide();
            m_logo_banner->show_logo(false);
            set_hotkey_enabled(false);
        } break;
    }
    m_view_mode = mode;
}

void HudManager::hide() {
    if (m_hidden) return;

    if (g_hud_surface) {
        if (!m_transparent_mask) {
            auto zeros = std::make_unique<u8[]>(640 * 480);
            STransCreateE(zeros.get(), 640, 480, 8, 0, 0x1000000, &m_transparent_mask);
            if (!m_transparent_mask) return;
        }
        m_saved_hud_mask = g_hud_surface->surface;
        g_hud_surface->surface = m_transparent_mask;
        helper::hud_update_surface_masks();
    }

    m_hidden = true;
    apply_visibility(false);
    if (m_logo_banner) m_logo_banner->show();
    helper::bw_refresh(0, 479, 0, 639);
}

void HudManager::show() {
    if (!m_hidden) return;
    m_hidden = false;

    if (g_hud_surface && m_saved_hud_mask) {
        g_hud_surface->surface = m_saved_hud_mask;
        m_saved_hud_mask = nullptr;
        helper::hud_update_surface_masks();
    }

    apply_visibility(true);
    if (m_logo_banner) m_logo_banner->hide();
    helper::bw_refresh(0, 479, 0, 639);
}

void HudManager::apply_visibility(bool visible) {
    auto update_position = [visible](bw::Dialog* bindlg, DialogBounds& saved, DialogFlags::Enum& saved_flags, s16 dx, s16 dy) {
        if (!bindlg) return;
        bw::Dialog& dialog = *bindlg;
        if (visible) {
            dialog.bounds = saved;
            dialog.flags = saved_flags;
        } else {
            saved = dialog.bounds;
            saved_flags = dialog.flags;
            dialog.bounds = dialog.bounds.offset(dx, dy);
            dialog.flags |= DialogFlags::ReverseDialogDirection;
        }
        dialog.flags |= DialogFlags::Visible | DialogFlags::UpdateNeeded;
        helper::update_dialog(bindlg);
    };
    update_position(g_stat_f10_dialog, m_saved_stat_f10_bounds, m_saved_stat_f10_flags, 144, 12);
    update_position(g_statport_dialog->to_dialog(), m_saved_statport_bounds, m_saved_statport_flags, 144, 12);

    auto update_visibility = [visible](bw::Dialog* dialog) {
        if (!dialog) return;
        if (visible) {
            dialog->flags |= DialogFlags::Visible;
        } else {
            dialog->flags &= ~DialogFlags::Visible;
        }
        dialog->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(dialog);
    };
    update_visibility(g_statdata_dialog->to_dialog());
    update_visibility(g_statres_dialog);
    update_visibility(g_statlb_dialog);
    update_visibility(g_textbox_dialog);
    update_visibility(g_buttonset_dialog->to_dialog());
    update_visibility(g_minimap_bin->to_dialog());
    for (auto& entry : m_statfluf_dialogs) update_visibility(entry.dialog);
}

const HudManager::Config& HudManager::current() const {
    static const Config empty{};
    return m_current ? *m_current : empty;
}

void HudManager::load_definitions() {
    m_configs.clear();

    auto loaded = sfile::load_file(DefinitionsPath);
    const std::string contents{loaded.data.get(), loaded.size};

    auto result = rfl::yaml::read<DefinitionsFile>(contents);
    if (!result) {
        auto message = std::format("{}: {}", DefinitionsPath, result.error().what());
        helper::fatal_file_not_found(sfile::ErrorInvalidData, message.c_str());
    }

    m_configs = std::move(result.value().huds);
}

// Order is important
void HudManager::load_all() {
    load_hud_image();
    auto image_buffer = smem::ptr <u8[]>{g_hud_image.buffer};

    if (!SBmpLoadImage("game\\thpbar.pcx", 0, &g_thpbar_colors[0], sizeof(g_thpbar_colors), 0, 0, 0)) {
        helper::fatal_file_not_found(SErrGetLastError(), "game\\thpbar.pcx");
    }
    load_statfluf_decoration_dialogs();
    helper::load_statport();
    load_statlb();
    load_statres();
    helper::load_stat_data();
    load_wireframe_palette();
    load_command_buttons();
    load_statf10();
    load_textbox();
    helper::load_minimap();

    g_hud_image.buffer = nullptr;
}

// Load only the elements needed to change the hud
void HudManager::load_hud_unique() {
    auto reload_bitmap = [](Bitmap& target, const char* path) {
        if (target.buffer) SMemFree(target.buffer);
        target = {};
        load_bitmap(path, target, nullptr);
    };
    auto refresh_backdrop = [](bw::Dialog* dialog) {
        if (dialog->bitmap.buffer) SMemFree(dialog->bitmap.buffer);
        helper::bindlg_blit_surface(dialog);
        dialog->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(dialog);
    };

    load_hud_image();
    load_statfluf_decoration_dialogs();
    if (g_statport_dialog) {
        refresh_backdrop(g_statport_dialog->to_dialog());
        reload_bitmap(g_conover_image, m_current->hud_overlay.c_str());
    }
    load_statres();
    if (g_statdata_dialog) {
        refresh_backdrop(g_statdata_dialog->to_dialog());
        reload_bitmap(g_statdata_progressbar_empty, m_current->progressbar_empty.c_str());
        reload_bitmap(g_statdata_progressbar_full,  m_current->progressbar_full.c_str());
    }
    load_command_buttons();
    load_statf10();
    load_textbox();
    helper::load_minimap();

    if (g_hud_image.buffer) {
        SMemFree(g_hud_image.buffer);
        g_hud_image.buffer = nullptr;
    }
}

// Order is important
void HudManager::unload_all() {
    if (g_minimap_bin) {
        helper::destroy_dialog(g_minimap_bin); // Dialog is expected to free itself in its dialogproc
        g_minimap_bin = nullptr;
    }
    if (g_minimap_blink_grp) {
        SMemFree(g_minimap_blink_grp);
        g_minimap_blink_grp = nullptr;
    }
    if (g_textbox_dialog) {
        helper::destroy_dialog(g_textbox_dialog);
        g_textbox_dialog = nullptr;
    }
    if (g_stat_f10_dialog) {
        helper::destroy_dialog(g_stat_f10_dialog);
        g_stat_f10_dialog = nullptr;
    }
    if (g_wirefram_grp) {
        SMemFree(g_wirefram_grp);
        g_wirefram_grp = nullptr;
    }
    helper::unload_statdata_bar();
    if (g_statres_dialog) {
        helper::destroy_dialog(g_statres_dialog);
        g_statres_dialog = nullptr;
    }
    if (g_icons_grp) {
        SMemFree(g_icons_grp);
        g_icons_grp = nullptr;
    }
    if (g_statlb_dialog) {
        helper::destroy_dialog(g_statlb_dialog);
        g_statlb_dialog = nullptr;
    }
    helper::unload_buttonset();
    helper::unload_statport();
    unload_statfluf_mouse_regions();

    auto free_bitmap = [](Bitmap& bitmap) {
        if (bitmap.buffer) SMemFree(bitmap.buffer);
        bitmap = {};
    };
    free_bitmap(g_statdata_progressbar_empty);
    free_bitmap(g_statdata_progressbar_full);
    free_bitmap(g_cmdbuttons_progressbar_empty);
    free_bitmap(g_cmdbuttons_progressbar_full);

    helper::reset_status_screen_area();

    if (g_hud_surface) {
        helper::hud_destroy_top_surface(g_hud_surface);
        g_hud_surface = nullptr;
    }
}

// Unload only the elements needed to change the hud
void HudManager::unload_hud_unique() {
    if (g_minimap_bin) {
        helper::destroy_dialog(g_minimap_bin);
        g_minimap_bin = nullptr;
    }
    if (g_minimap_blink_grp) {
        SMemFree(g_minimap_blink_grp);
        g_minimap_blink_grp = nullptr;
    }
    if (g_textbox_dialog) {
        helper::destroy_dialog(g_textbox_dialog);
        g_textbox_dialog = nullptr;
    }
    if (g_stat_f10_dialog) {
        helper::destroy_dialog(g_stat_f10_dialog);
        g_stat_f10_dialog = nullptr;
    }
    if (g_statres_dialog) {
        helper::destroy_dialog(g_statres_dialog);
        g_statres_dialog = nullptr;
    }
    if (g_icons_grp) {
        SMemFree(g_icons_grp);
        g_icons_grp = nullptr;
    }
    helper::unload_buttonset();
    unload_statfluf_mouse_regions();

    auto free_bitmap = [](Bitmap& bitmap) {
        if (bitmap.buffer) SMemFree(bitmap.buffer);
        bitmap = {};
    };
    free_bitmap(g_cmdbuttons_progressbar_empty);
    free_bitmap(g_cmdbuttons_progressbar_full);

    if (g_hud_surface) {
        helper::hud_destroy_top_surface(g_hud_surface);
        g_hud_surface = nullptr;
    }
}

void HudManager::reset_ui_state() {
    g_control_under_mouse = nullptr;
    g_control_under_mouse_value = nullptr;
    g_force_button_refresh = 1;
    g_portrait_redraw = 1;
    g_status_screen_redraw = 1;
}

bool HudManager::has_hud(std::string_view name) const {
    return m_configs.contains(std::string{name});
}

void HudManager::resolve_current(std::string_view name) {
    auto it = m_configs.find(std::string{name});
    if (it == m_configs.end()) {
        const auto message = std::format("hud '{}' not defined in {}", name, DefinitionsPath);
        helper::fatal_file_not_found(sfile::ErrorInvalidData, message.c_str());
    }
    m_current = &it->second;
    m_current_name = it->first;

    auto require = [&](std::string_view field, const std::string& value) {
        if (!value.empty()) return;
        const auto message = std::format("hud '{}' missing required field '{}' in {}", m_current_name, field, DefinitionsPath);
        helper::fatal_file_not_found(sfile::ErrorInvalidData, message.c_str());
    };
    require("image",               m_current->image);
    require("hud_overlay",         m_current->hud_overlay);
    require("progressbar_empty",   m_current->progressbar_empty);
    require("progressbar_full",    m_current->progressbar_full);
    require("command_buttons",     m_current->command_buttons);
    require("command_buttons_bin", m_current->command_buttons_bin);
}

static void hud_find_vertical_bounds(Bitmap* hud) {
    if (!hud || !hud->buffer) return;

    auto buffer = hud->buffer;
    auto width = hud->width;
    auto height = hud->height;

    auto is_opaque = [](u8 pixel) { return pixel != 0; };

    g_hud_first_content_row = 0;
    for (u16 row = 0; row < height; ++row) {
        auto row_start = buffer + row * width;
        if (std::any_of(row_start, row_start + width, is_opaque)) {
            break;
        }
        g_hud_first_content_row++;
    }

    g_hud_last_content_row = g_hud_first_content_row;
    for (u16 row = g_hud_first_content_row; row < height; ++row) {
        auto row_start = buffer + row * width;
        if (std::any_of(row_start, row_start + width, is_opaque)) {
            g_hud_last_content_row = row + 1;
        }
    }
}

static SurfaceContext* hud_create_surface(u32 offset_x, u32 offset_y, Bitmap* bitmap) {
    void* surface_handle;
    SurfaceContext* surface_out{};

    STransCreateE(bitmap->buffer, bitmap->width, bitmap->height, 8, 0, 0x1000000, &surface_handle);

    auto* surface_node = static_cast<SurfaceContext*>(SMemAlloc(sizeof(SurfaceContext), "", 0, '\b'));
    if (surface_node) {
        surface_node->next = nullptr;
        surface_node->prev = nullptr;
        surface_out = surface_node;
    } else {
        surface_node = reinterpret_cast<SurfaceContext*>(&g_surface_list_head);
    }

    if (surface_node->next) {
        surface_node->next->prev = surface_node->prev;
        if (surface_node->prev) {
            surface_node->prev->next = surface_node->next;
        }
        surface_node->next = nullptr;
        surface_node->prev = nullptr;
    }

    surface_node->next = g_surface_list_head;
    if (g_surface_list_head) {
        surface_node->prev = g_surface_list_head->prev;
        g_surface_list_head->prev = surface_out;
    }
    g_surface_list_head = surface_node;

    if (surface_out) {
        surface_out->surface = surface_handle;
        surface_out->left = offset_x;
        surface_out->top = offset_y;
        surface_out->right = offset_x + bitmap->width;
        surface_out->bottom = offset_y + bitmap->height;
    }

    helper::hud_update_surface_masks();
    return surface_out;
}

void HudManager::load_hud_image() {
    load_bitmap(m_current->image.c_str(), g_hud_image);
    hud_find_vertical_bounds(&g_hud_image);
    g_hud_surface = hud_create_surface(0, 0, &g_hud_image);
    resolve_statfluf_regions();
}

// Bounds of always-visible primary widgets (inclusive l/t/r/b)
struct PrimaryRect { s16 left, top, right, bottom; };
static constexpr PrimaryRect PrimarySubtractions[] = {
    {220, 0,   639, 19},   // statres
    {0,   315, 137, 479},  // minimap
    {138, 388, 407, 479},  // statdata
    {408, 408, 495, 479},  // statport
    {408, 388, 495, 407},  // statf10
    {496, 354, 639, 479},  // statbtn
};

static bool inside_primary(s16 x, s16 y) {
    for (const auto& rect : PrimarySubtractions) {
        if (x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom) {
            return true;
        }
    }
    return false;
}

// Emits one rectangle per vertical run of same-x-extent opaque pixels,
// skipping anything inside PrimarySubtractions
static std::vector<DialogBounds> scan_opaque_rects(const Bitmap& pcx) {
    std::vector<DialogBounds> result;
    if (!pcx.buffer || pcx.width == 0 || pcx.height == 0) return result;

    struct ActiveRect { s16 left, right, top; };
    std::vector<ActiveRect> active;
    std::vector<ActiveRect> carried;

    struct Run { s16 left, right; };
    std::vector<Run> runs;

    for (u16 y = 0; y < pcx.height; ++y) {
        runs.clear();
        const u8* row = pcx.buffer + static_cast<size_t>(y) * pcx.width;
        u16 x = 0;
        while (x < pcx.width) {
            while (x < pcx.width && (row[x] == 0 || inside_primary(static_cast<s16>(x), static_cast<s16>(y)))) {
                ++x;
            }
            if (x >= pcx.width) break;
            const s16 run_left = static_cast<s16>(x);
            while (x < pcx.width && row[x] != 0 && !inside_primary(static_cast<s16>(x), static_cast<s16>(y))) {
                ++x;
            }
            runs.push_back({run_left, static_cast<s16>(x - 1)});
        }

        carried.clear();
        std::vector<u8> matched(active.size(), 0);
        std::vector<u8> run_used(runs.size(), 0);

        for (size_t r = 0; r < runs.size(); ++r) {
            for (size_t a = 0; a < active.size(); ++a) {
                if (matched[a]) continue;
                if (active[a].left != runs[r].left) continue;
                if (active[a].right != runs[r].right) continue;
                carried.push_back(active[a]);
                matched[a] = 1;
                run_used[r] = 1;
                break;
            }
        }

        for (size_t a = 0; a < active.size(); ++a) {
            if (matched[a]) continue;
            result.push_back(DialogBounds::from_ltrb(
                active[a].left,
                active[a].top,
                active[a].right,
                static_cast<s16>(y - 1))
            );
        }

        for (size_t r = 0; r < runs.size(); ++r) {
            if (run_used[r]) continue;
            carried.push_back({runs[r].left, runs[r].right, static_cast<s16>(y)});
        }

        active = std::move(carried);
    }

    for (const auto& rect : active) {
        result.push_back(DialogBounds::from_ltrb(
            rect.left,
            rect.top,
            rect.right,
            static_cast<s16>(pcx.height - 1))
        );
    }

    return result;
}

void HudManager::resolve_statfluf_regions() {
    m_statfluf_dialogs.clear();
    auto rects = scan_opaque_rects(g_hud_image);
    m_statfluf_dialogs.reserve(rects.size());
    for (const auto& bounds : rects) {
        m_statfluf_dialogs.push_back({bounds, nullptr});
    }
}

void HudManager::load_statfluf_decoration_dialogs() {
    for (auto& entry : m_statfluf_dialogs) {
        auto* dialog = load_bin_file("rez\\statfluf.bin").release();
        entry.dialog = dialog;

        dialog->bounds = entry.bounds;

        auto& surface = dialog->data<bw::MenuData>()->backup_surface;
        surface.width = entry.bounds.w();
        surface.height = entry.bounds.h();

        initialize_bin_dialog(dialog, helper::statfluf_dlgproc);
    }
}

void HudManager::unload_statfluf_mouse_regions() {
    for (auto& entry : m_statfluf_dialogs) {
        if (entry.dialog) {
            helper::destroy_dialog(entry.dialog->to_bindlg());
            entry.dialog = nullptr;
        }
    }
}

void HudManager::load_command_buttons() { // 00459b90
    g_active_portrait_buttonset_id   = UnitId::Buttons_Blank;
    g_buttonset_special_buttonset_id = UnitId::Buttons_Blank;
    g_current_buttonset_id           = UnitId::Buttons_Blank;
    g_highlighted_icon = -1;

    g_cmdbtns_grp = sfile::load_file(m_current->command_buttons.c_str()).unsafe_cast<GrpHead>().release();
    g_cmdicons_grp = sfile::load_file("unit\\cmdbtns\\cmdicons.grp").unsafe_cast<GrpHead>().release();

    if (!SBmpLoadImage("unit\\cmdbtns\\ticon.pcx", 0, (u8*)&g_new_cmdbtn_palette, sizeof(g_new_cmdbtn_palette), 0, 0, 0)) {
        helper::fatal_file_not_found(SErrGetLastError(), "unit\\cmdbtns\\ticon.pcx");
    }

    g_buttonset_dialog = load_bin_file(m_current->command_buttons_bin.c_str()).release()->to_bindlg();
    initialize_bin_dialog(g_buttonset_dialog, helper::statbtn_dlgproc);
}

void HudManager::load_textbox() { // 004f37c0
    // TODO: dynamically create from textbox bounds -Jesse
    auto initial = get_race_initial((RaceId::Enum)g_hud_index).value_or('t');
    auto filename = std::format("rez\\{}textbox.bin", initial);
    g_textbox_dialog = load_bin_file(filename.c_str()).release();
    initialize_bin_dialog(g_textbox_dialog, helper::textbox_dlgproc);
}

void HudManager::load_statlb() { // 004bed70
    bool is_standard_melee = (g_game_data.victory_condition == VictoryCondition::Default &&
        g_game_data.starting_units == StartingUnits::MapUnits &&
        g_game_data.tournament_mode_enabled == 0);

    bool should_show_statlb = !g_multiplayer_mode ||
        g_game_data.victory_condition == VictoryCondition::Greed ||
        g_game_data.victory_condition == VictoryCondition::Slaughter ||
        is_standard_melee ||
        g_is_in_replay;

    if (!should_show_statlb) {
        return;
    }

    g_statlb_unk = 0;
    g_show_statlb = true;

    g_statlb_dialog = load_bin_file("rez\\statlb.bin").release();
    initialize_bin_dialog(g_statlb_dialog, helper::statlb_dlgproc);
}

void HudManager::load_wireframe_palette() { // 00456a90
    g_wirefram_grp = sfile::load_file("unit\\wirefram\\wirefram.grp").unsafe_cast<GrpHead>().release();
    if (!SBmpLoadImage("game\\twire.pcx", 0, &g_wireframe_colors[0], sizeof(g_wireframe_colors), 0, 0, 0)) {
        helper::fatal_file_not_found(SErrGetLastError(), "game\\twire.pcx");
    }
}

void HudManager::load_statf10() { // 004f5300
    g_stat_f10_dialog = load_bin_file("rez\\stat_f10.bin").release();
    initialize_bin_dialog(g_stat_f10_dialog, helper::stat_f10_dlgproc);
}
