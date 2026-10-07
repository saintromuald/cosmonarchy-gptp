#include "hud_manager.h"

#include "BW/api.h"
#include "BW/bwdata.h"
#include "BW/bwhelper.h"
#include "BW/ui.h"
#include "hud.h"
#include "stat_resource.h"
#include "statport.h"
#include "hooks/display/grid.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "storm/storm.h"
#include "systems/display/display.h"
#include "systems/display/hud_layout.h"

#include <algorithm>
#include <format>
#include <memory>

#include <rfl/yaml.hpp>

namespace {
constexpr auto HudsYamlPath = "config\\huds.yml";

// Frames to force full HUD repaints after placement before reverting to dirty checks.
constexpr u32 ForcedRepaintFrames = 4;

// The lattice 0041C200 snaps the dialog refresh region onto.
constexpr s32 RegionGrain = 16;

// 0041E0D0 takes inclusive (left, bottom, top, right); engine defaults to 0, 479, 0, 639.
void refresh_whole_surface() {
    helper::bw_refresh(0, g_display.surface_height() - 1, 0, g_display.surface_width() - 1);
}
} // namespace

// 004C35F0 allocates the backdrop without freeing, so the stale create-time cache
// (blitted at unscaled band coordinates) is freed first and re-blit at the
// dialog's placed screen bounds.
void refresh_dialog_backdrop(bw::Dialog* dialog) {
    if (dialog->bitmap.buffer) SMemFree(dialog->bitmap.buffer);
    helper::bindlg_blit_surface(dialog);
    dialog->flags |= DialogFlags::UpdateNeeded;
    helper::update_dialog(dialog);
}

std::optional<DialogBounds> HudManager::Config::chat_textbox() const {
    if (chat_textbox_bounds.size() != 4) return {};
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
}

void HudManager::install(RaceId::Enum race) {
    switch (race) {
        case RaceId::Zerg:    install("zerg");    break;
        case RaceId::Terran:  install("terran");  break;
        case RaceId::Protoss: install("protoss"); break;
        case RaceId::Askosi:  install("askosi");  break;
        default:              install("terran");  break; // Shouldn't this be neutral? -Veeq7
    }
}

void HudManager::install_replay() {
    install("neutral");
}

HudManager::~HudManager() {
    if (m_hidden && g_hud_surface && m_saved_hud_mask) {
        g_hud_surface->surface = m_saved_hud_mask;
        m_saved_hud_mask = nullptr;
    }
    unload_all();

    // STransCreateE handles are released via STransDelete (storm 434, matching 0041D5A0) after unload_all.
    if (m_transparent_mask) {
        STransDelete(m_transparent_mask);
        m_transparent_mask = nullptr;
    }
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
            set_hotkey_enabled(true);
        } break;
        case HudViewMode::Hidden: {
            hide();
            set_hotkey_enabled(true);
        } break;
        case HudViewMode::Cinematic: {
            hide();
            set_hotkey_enabled(false);
        } break;
    }
    m_view_mode = mode;
}

void HudManager::hide() {
    if (m_hidden) return;

    if (g_hud_surface) {
        if (!m_transparent_mask) {
            // Sized to full surface because the mask sizes Storm's blit chain; a narrower mask clips world blits.
            const u32 width = g_display.surface_width();
            const u32 height = g_display.surface_height();
            auto zeros = std::make_unique<u8[]>(size_t(width) * height);
            STransCreateE(zeros.get(), width, height, 8, 0, 0x1000000, &m_transparent_mask);
            if (!m_transparent_mask) return;
        }
        m_saved_hud_mask = g_hud_surface->surface;
        g_hud_surface->surface = m_transparent_mask;
        helper::hud_update_surface_masks();
    }

    m_hidden = true;
    apply_visibility(false);
    refresh_whole_surface();
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
    refresh_whole_surface();
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
    update_position(g_stat_f10_dialog, m_saved_stat_f10_bounds, m_saved_stat_f10_flags, 289, 12);
    update_position(g_statport_dialog->to_dialog(), m_saved_statport_bounds, m_saved_statport_flags, 289, 12);

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
    if (m_stat_resource) {
        if (visible) {
            m_stat_resource->show();
        } else {
            m_stat_resource->hide();
        }
    }
    update_visibility(g_statlb_dialog);
    update_visibility(g_textbox_dialog);
    update_visibility(g_buttonset_dialog->to_dialog());
    update_visibility(g_minimap_bin->to_dialog());
    for (auto& entry : m_statfluf_dialogs) update_visibility(entry.dialog);
}

// Re-implements 0041C7B0 dirty-rect invalidation for HUD dialogs when the world renders under them.
// DialogFlags::ReverseDialogDirection cannot be used because 0041C810 blits +0x36 as a bitmap buffer,
// which is invalid for statfluf dialogs. Calls invalidate_dialog_region (0041C4F0) per dirty dialog.
void HudManager::invalidate_dirty_dialogs() {
    const bool forced = m_forced_repaints > 0;
    if (forced) {
        --m_forced_repaints;
    }

    auto ask = [forced](BinDlg* dialog) {
        if (!dialog || !(dialog->flags & DialogFlags::Visible)) {
            return;
        }
        const Box16& bounds = dialog->dialog_bounds;

        const s32 left = bounds.left & ~(RegionGrain - 1);
        const s32 top = bounds.top & ~(RegionGrain - 1);
        const s32 right = (bounds.right + RegionGrain) & ~(RegionGrain - 1);
        const s32 bottom = (bounds.bottom + RegionGrain) & ~(RegionGrain - 1);

        if (!forced && !hooks::rect_is_dirty(left, top, right, bottom)) {
            return;
        }

        RECT region{bounds.left, bounds.top, bounds.right, bounds.bottom};
        helper::invalidate_dialog_region(&region);
    };

    for (const auto& entry : m_statfluf_dialogs) {
        ask(entry.dialog->to_bindlg());
    }
    for (BinDlg* dialog : {g_statdata_dialog, g_statport_dialog, g_statlb_dialog->to_bindlg(), g_textbox_dialog->to_bindlg(), g_buttonset_dialog, g_stat_f10_dialog->to_bindlg(), g_minimap_bin}) {
        ask(dialog);
    }
    if (m_stat_resource) {
        ask(m_stat_resource->dialog()->to_bindlg());
    }
}

const HudManager::Config& HudManager::current() const {
    static const Config empty{};
    return m_current ? *m_current : empty;
}

void HudManager::load_definitions() {
    m_configs.clear();

    auto loaded = sfile::load_file(HudsYamlPath);
    const std::string contents{loaded.data.get(), loaded.size};

    auto result = rfl::yaml::read<ConfigMap>(contents);
    if (!result) {
        auto message = std::format("{}: {}", HudsYamlPath, result.error().what());
        helper::fatal_file_not_found(sfile::ErrorInvalidData, message.c_str());
    }

    m_configs = std::move(result.value());
}

// Order is important
void HudManager::load_all() {
    load_hud_image();
    if (!SBmpLoadImage("game\\thpbar.pcx", 0, &g_thpbar_colors[0], sizeof(g_thpbar_colors), 0, 0, 0)) {
        helper::fatal_file_not_found(SErrGetLastError(), "game\\thpbar.pcx");
    }
    load_statfluf_decoration_dialogs();
    // statport's create handler sizes the portrait decode buffer from the child
    // bounds, so it loads scaled (hooks/interface/hud/statport.cpp).
    hooks::load_statport();
    hud_layout::place_dialog(g_statport_dialog, hud_layout::Anchor::HudRight);
    refresh_dialog_backdrop(g_statport_dialog->to_dialog());
    load_statlb();
    m_stat_resource = std::make_unique<StatResource>();
    // The vanilla loaders create these trees at 1x and cache their backdrops before
    // we scale — scale, place at the split anchors, then re-cache from the mask.
    helper::load_stat_data();
    ui::scale_bin(*g_statdata_dialog->to_dialog());
    hud_layout::place_dialog(g_statdata_dialog, hud_layout::Anchor::HudLeft);
    refresh_dialog_backdrop(g_statdata_dialog->to_dialog());
    load_wireframe_palette();
    load_command_buttons();
    load_statf10();
    load_textbox();
    // The in-game minimap is rebuilt at 1x by vanilla's loader, so it gets the
    // same scale/place/re-cache treatment as the other vanilla trees.
    helper::load_minimap();
    ui::scale_bin(*g_minimap_bin->to_dialog());
    hud_layout::place_dialog(g_minimap_bin, hud_layout::Anchor::HudLeft);
    refresh_dialog_backdrop(g_minimap_bin->to_dialog());
    finish_hud_image();
}

// Load only the elements needed to change the hud
void HudManager::load_hud_unique() {
    auto reload_bitmap = [](Bitmap& target, const char* path) {
        if (target.buffer) SMemFree(target.buffer);
        target = {};
        load_bitmap(path, target, nullptr);
    };

    load_hud_image();
    load_statfluf_decoration_dialogs();
    // Portrait and unit status are preserved from initial load and retain their
    // placed bounds; re-cache their backdrops from the new mask.
    if (g_statport_dialog) {
        refresh_dialog_backdrop(g_statport_dialog->to_dialog());
        reload_bitmap(g_conover_image, m_current->hud_overlay.c_str());
    }
    m_stat_resource = std::make_unique<StatResource>();
    if (g_statdata_dialog) {
        refresh_dialog_backdrop(g_statdata_dialog->to_dialog());
        reload_bitmap(g_statdata_progressbar_empty, m_current->progressbar_empty.c_str());
        reload_bitmap(g_statdata_progressbar_full,  m_current->progressbar_full.c_str());
    }
    load_command_buttons();
    // The leaderboard and F10 menu are preserved from initial load and retain their placement.
    if (g_stat_f10_dialog) {
        refresh_dialog_backdrop(g_stat_f10_dialog);
    }
    load_textbox();
    // unload_hud_unique destroyed the minimap tree, so vanilla's loader rebuilds
    // it at 1x — same scale/place/re-cache as load_all.
    helper::load_minimap();
    ui::scale_bin(*g_minimap_bin->to_dialog());
    hud_layout::place_dialog(g_minimap_bin, hud_layout::Anchor::HudLeft);
    refresh_dialog_backdrop(g_minimap_bin->to_dialog());
    finish_hud_image();
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
    m_stat_resource.reset(); // ~StatResource clears the timer and nulls g_statres_dialog
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
    m_stat_resource.reset(); // ~StatResource clears the timer and nulls g_statres_dialog
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
        const auto message = std::format("hud '{}' not defined in {}", name, HudsYamlPath);
        helper::fatal_file_not_found(sfile::ErrorInvalidData, message.c_str());
    }
    m_current = &it->second;
    m_current_name = it->first;

    auto require = [&](std::string_view field, const std::string& value) {
        if (!value.empty()) return;
        const auto message = std::format("hud '{}' missing required field '{}' in {}", m_current_name, field, HudsYamlPath);
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

    // is_outside_of_game_screen (004D1140) treats y >= g_hud_last_content_row as off-screen without checking mask.
    // Clamp to surface height so rows below HUD on taller surfaces use per-pixel mask testing (004564E0, 0048E5D0, 004BD500).
    if (g_hud_last_content_row < height) {
        g_hud_last_content_row = height;
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

// Updates g_command_card_rect (005136CC) tested by 00484620 PtInRect to reset buttonsets on right click.
// Uses card bounds + 1 because PtInRect right/bottom edges are exclusive whereas BinDlg bounds are inclusive.
static void publish_command_card_rect() {
    const BinDlg* card = g_buttonset_dialog;
    if (!card) {
        return;
    }

    SetRect(&g_command_card_rect, card->bounds.left, card->bounds.top,
        card->bounds.right + 1, card->bounds.bottom + 1);
}

// Loads the split HUD art into the surface-sized transparency mask, bottom-anchored:
// art row r lands at surface row surface_height - art_height + r. g_hud_image stays
// bound to the mask for 004C35F0 backdrop caches until finish_hud_image detaches it.
void HudManager::load_hud_image() {
    Bitmap art{};
    load_bitmap(m_current->image.c_str(), art);

    const u32 width = g_display.surface_width();
    const u32 height = g_display.surface_height();
    if (art.width != width || art.height == 0 || art.height > height) {
        SMemFree(art.buffer);
        auto message = std::format("{}: HUD image must be exactly {}px wide and at most {}px tall",
            m_current->image.c_str(), width, height);
        helper::fatal_file_not_found(sfile::ErrorInvalidData, message.c_str());
    }

    m_mask.assign(size_t(width) * height, 0);
    const u32 art_top = height - art.height;
    for (u32 row = 0; row < art.height; ++row) {
        std::copy_n(art.buffer + size_t(row) * art.width, width,
            m_mask.begin() + size_t(art_top + row) * width);
    }
    SMemFree(art.buffer);

    g_hud_image.width = u16(width);
    g_hud_image.height = u16(height);
    g_hud_image.buffer = m_mask.data();

    resolve_statfluf_regions();
}

// Stamps placed HUD dialogs into the transparency mask so controls are not overwritten by world blits.
void HudManager::stamp_hud_dialogs_into_mask() {
    constexpr u8 MaskOpaque = 1;

    const s32 width = s32(g_display.surface_width());
    const s32 height = s32(g_display.surface_height());

    auto stamp = [&](const BinDlg* dialog) {
        if (!dialog) {
            return;
        }
        const Box16& bounds = dialog->dialog_bounds;
        const s32 left = std::clamp<s32>(bounds.left, 0, width);
        const s32 right = std::clamp<s32>(bounds.right + 1, left, width);
        const s32 top = std::clamp<s32>(bounds.top, 0, height);
        const s32 bottom = std::clamp<s32>(bounds.bottom + 1, top, height);

        for (s32 y = top; y < bottom; ++y) {
            auto row = m_mask.begin() + size_t(y) * width;
            std::fill(row + left, row + right, MaskOpaque);
        }
    };

    for (const auto& entry : m_statfluf_dialogs) {
        stamp(entry.dialog->to_bindlg());
    }
    for (const BinDlg* dialog : {g_minimap_bin, g_statdata_dialog, g_statport_dialog, g_stat_f10_dialog->to_bindlg(), g_buttonset_dialog}) {
        stamp(dialog);
    }
}

// Mask pixels where the dialog's backup surface art is opaque. Transparent art pixels
// stay gameplay, so the world presents (and clicks pass) through them.
void HudManager::write_dialog_mask_stamp(bw::Dialog* dialog) {
    const auto& surface = dialog->data<bw::MenuData>()->backup_surface;
    if (!surface.buffer) {
        return;
    }

    const s32 surface_width = s32(g_display.surface_width());
    const s32 surface_height = s32(g_display.surface_height());
    const Box16& bounds = dialog->to_bindlg()->dialog_bounds;

    const s32 left = std::clamp<s32>(bounds.left, 0, surface_width);
    const s32 right = std::clamp<s32>(bounds.right + 1, left, surface_width);
    const s32 top = std::clamp<s32>(bounds.top, 0, surface_height);
    const s32 bottom = std::clamp<s32>(bounds.bottom + 1, top, surface_height);

    for (s32 y = top; y < bottom; ++y) {
        const s32 art_y = y - bounds.top;
        if (art_y < 0 || art_y >= surface.height) {
            continue;
        }
        const u8* art_row = surface.buffer + size_t(art_y) * surface.width;
        auto mask_row = m_mask.begin() + size_t(y) * surface_width;
        for (s32 x = left; x < right; ++x) {
            const s32 art_x = x - bounds.left;
            if (art_x >= 0 && art_x < surface.width && art_row[art_x] != 0) {
                mask_row[x] = 1;
            }
        }
    }
}

// Phase 2 after the dialog loads: stamps the placed dialogs into the mask and
// creates the HUD transparency surface.
void HudManager::finish_hud_image() {
    publish_command_card_rect();
    m_forced_repaints = ForcedRepaintFrames;

    stamp_hud_dialogs_into_mask();

    hud_find_vertical_bounds(&g_hud_image);
    g_hud_surface = hud_create_surface(0, 0, &g_hud_image);

    // Detach g_hud_image buffer; storage is owned by m_mask.
    g_hud_image.buffer = nullptr;
}

// Authored 1x band-space bounds (inclusive l/t/r/b) of the always-visible primary
// widgets, paired with the split anchor that places them — the opaque scan must
// not grow statfluf decorations over them.
struct PrimaryRect { s16 left, top, right, bottom; hud_layout::Anchor anchor; };
static constexpr PrimaryRect PrimarySubtractions[] = {
    {0,     0, 1279, 255, hud_layout::Anchor::TopBarRight},  // statres
    {0,   315, 137, 479, hud_layout::Anchor::HudLeft},     // minimap
    {138, 388, 407, 479, hud_layout::Anchor::HudLeft},     // statdata
    {408, 408, 495, 479, hud_layout::Anchor::HudRight},    // statport
    {408, 388, 495, 407, hud_layout::Anchor::HudRight},    // statf10
    {496, 354, 639, 479, hud_layout::Anchor::HudRight},    // statbtn
};

static bool inside_primary(s16 x, s16 y) {
    for (const auto& rect : PrimarySubtractions) {
        const v2s32 offset = hud_layout::anchor_offset(rect.anchor);
        const s32 left = s32(rect.left) * ui_scale::Factor + offset.x;
        const s32 right = s32(rect.right) * ui_scale::Factor + offset.x;
        const s32 top = s32(rect.top) * ui_scale::Factor + offset.y;
        const s32 bottom = s32(rect.bottom) * ui_scale::Factor + offset.y;
        if (x >= left && x <= right && y >= top && y <= bottom) {
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

// Scans decoration rects out of the composed mask; they are already in screen
// space, so each becomes a dialog at its final position.
void HudManager::resolve_statfluf_regions() {
    m_statfluf_dialogs.clear();
    for (const auto& bounds : scan_opaque_rects(g_hud_image)) {
        m_statfluf_dialogs.push_back({bounds, nullptr});
    }
}

void HudManager::load_statfluf_decoration_dialogs() {
    for (auto& entry : m_statfluf_dialogs) {
        // The bin is a single generic element; its bounds and surface are resized
        // from the art-scanned region below, so it needs no scaling.
        auto* dialog = load_bin_file("rez\\statfluf.bin").release();
        entry.dialog = dialog;

        // 00419D20 converts a root's bounds from x/y/w/h to l/t/r/b itself, so it
        // is handed the width and height, the way statfluf.bin stores them.
        // Given the resolved rectangle it adds left to right and top to bottom
        // and the decoration overhangs into the world below and right of it.
        auto& bounds = dialog->to_bindlg()->bounds;
        bounds.x = entry.bounds.x();
        bounds.y = entry.bounds.y();
        bounds.w = static_cast<s16>(entry.bounds.w() - 1);
        bounds.h = static_cast<s16>(entry.bounds.h() - 1);

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

    g_buttonset_dialog = load_bin_file_scaled(m_current->command_buttons_bin.c_str()).release()->to_bindlg();
    initialize_bin_dialog(g_buttonset_dialog, helper::statbtn_dlgproc);
    hud_layout::place_dialog(g_buttonset_dialog, hud_layout::Anchor::HudRight);
    refresh_dialog_backdrop(g_buttonset_dialog->to_dialog());
}

void HudManager::load_textbox() { // 004f37c0
    // TODO: dynamically create from textbox bounds -Jesse
    auto initial = get_race_initial((RaceId::Enum)g_hud_index).value_or('t');
    auto filename = std::format("rez\\{}textbox.bin", initial);
    g_textbox_dialog = load_bin_file_scaled(filename.c_str()).release();
    initialize_bin_dialog(g_textbox_dialog, helper::textbox_dlgproc);
    hud_layout::place_dialog(g_textbox_dialog->to_bindlg(), hud_layout::Anchor::HudLeft);
    refresh_dialog_backdrop(g_textbox_dialog);
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

    g_statlb_dialog = load_bin_file_scaled("rez\\statlb.bin").release();
    initialize_bin_dialog(g_statlb_dialog, helper::statlb_dlgproc);
}

void HudManager::load_wireframe_palette() { // 00456a90
    g_wirefram_grp = sfile::load_file("unit\\wirefram\\wirefram.grp").unsafe_cast<GrpHead>().release();
    if (!SBmpLoadImage("game\\twire.pcx", 0, &g_wireframe_colors[0], sizeof(g_wireframe_colors), 0, 0, 0)) {
        helper::fatal_file_not_found(SErrGetLastError(), "game\\twire.pcx");
    }
}

void HudManager::load_statf10() { // 004f5300
    g_stat_f10_dialog = load_bin_file_scaled("rez\\stat_f10.bin").release();
    initialize_bin_dialog(g_stat_f10_dialog, helper::stat_f10_dlgproc);
    hud_layout::place_dialog(g_stat_f10_dialog->to_bindlg(), hud_layout::Anchor::HudRight);
    refresh_dialog_backdrop(g_stat_f10_dialog);
}
