#include "stat_resource.h"

#include "BW/api.h"
#include "BW/ui.h"
#include "CM/cm_ids.h"
#include "config.h"
#include "hooks/interface/hud/hud.h"
#include "hooks/interface/hud/hud_manager.h"
#include "hooks/interface/mainmenu/glucustom.h"
#include "hooks/interface/minimap.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "player_color.h"
#include "systems/display/hud_layout.h"
#include "systems/graphics/grp.h"
#include "systems/text/print.h"
#include "systems/triggers/trigger_system.h"

#include <charconv>

namespace {
namespace layout {
constexpr s16 ContainerWidth  = 1280;
constexpr s16 ContainerHeight = 256;

constexpr s16 FirstRowY = 4;
constexpr s16 RowPitch  = 30;
constexpr s16 RowHeight = 32;

// Column widths, right to left
constexpr s16 RightInset    = 6;
constexpr s16 SupplyWidth   = 140;
constexpr s16 GasWidth      = 140;
constexpr s16 MineralsWidth = 140;
constexpr s16 ApmWidth      = 60;
constexpr s16 ColorBoxSize  = 18;

// Gaps between the elements above, right to left
constexpr s16 ColumnGap     = 4; // supply -> gas -> minerals -> APM
constexpr s16 ColorBoxGap   = 8; // APM -> color box
constexpr s16 ProductionGap = 4; // color box -> production packing cursor
constexpr s16 NameGap       = 4; // last production slot -> name right edge

constexpr s16 ProductionPitch     = 30;
constexpr s16 ProductionSlotWidth = 28;
constexpr s16 ProductionBarWidth  = 30;

// Vanilla countdown-timer label
constexpr s16 TimerX      = 440;
constexpr s16 TimerWidth  = 160;
constexpr s16 TimerHeight = 30;

// Derived container-relative column lefts
constexpr s16 SupplyX    = ContainerWidth - RightInset - SupplyWidth;
constexpr s16 GasX       = SupplyX   - ColumnGap   - GasWidth;
constexpr s16 MineralsX  = GasX      - ColumnGap   - MineralsWidth;
constexpr s16 ApmX       = MineralsX - ColumnGap   - ApmWidth;
constexpr s16 ColorBoxX  = ApmX      - ColorBoxGap - ColorBoxSize;
constexpr s16 ProductionCursorX = ColorBoxX - ProductionGap;
constexpr s16 NameX = 0;

constexpr u16 TimerIndex = 1;
constexpr u32 TimerPeriod = 20;
constexpr u8 ProductionBarColor = 117;
// Vanilla statres text codes (passed to draw_colortext / g_print_xy_alignment).
constexpr s16 ApmTextCode = 0x21;
constexpr s16 NameTextCode = 0x24;
// old font_statres digits were 4x5px (10px tall at 2x); a 14px em renders ~10px Exo 2 digits
constexpr u32 ProductionTextPx = 14;
constexpr s16 ResourceFontLengthLimit = 8;
constexpr s16 ApmFontLengthLimit      = 5;
} // namespace layout

// Survives HUD switches so the resource odometer continues instead of restarting from zero.
s32 g_displayed_gas[PlayerLimit] = {};
s32 g_displayed_minerals[PlayerLimit] = {};

void ensure_visible(ui::Widget* widget) {
    if (!(widget->dialog()->flags & DialogFlags::Visible)) {
        widget->show();
    }
}

void hide_if_visible(ui::Widget* widget) {
    if (widget->dialog()->flags & DialogFlags::Visible) {
        widget->hide();
    }
}

void set_text_if_changed(ui::Widget* widget, std::string_view text) {
    if (widget->text() != text) {
        widget->set_text(std::string(text));
    }
}

// The per-tick updates compare before writing, but std::format allocated on every
// call; formatting into a stack buffer keeps them allocation-free.
template <typename T>
std::string_view format_int(char (&buffer)[12], T value) {
    char* end = std::to_chars(buffer, buffer + sizeof(buffer), value).ptr;
    return {buffer, size_t(end - buffer)};
}
} // namespace

StatResource::StatResource()
    : OverlayHud(container_bounds()) {
    for (u32 row = 0; row < PlayerLimit; ++row) {
        build_row(row);
    }

    // Vanilla compatibility: the per-second timer tick (FUN_00489cc0) walks g_statres_dialog's
    // children for index 0xFFF6 and shows/hides that label for the countdown timer
    // (FUN_004e5590 / FUN_004e5600 null-deref without it).
    // Vanilla drives the visibility, text, and updates, so this stays out of the row logic.
    ui::Widget* timer_label = label(DialogBounds::from_xywh(layout::TimerX, 0, layout::TimerWidth, layout::TimerHeight), "");
    bw::Dialog* timer_dialog = timer_label->dialog();
    timer_dialog->index = 0xFFF6;
    timer_dialog->type = DialogControlType::LabelRightAlign; // Authored right-aligned in the .bin
    timer_dialog->draw_function = helper::g_default_dialog_draw_functions[std::to_underlying(DialogControlType::LabelRightAlign)];

    initialize();
    for (u32 row = 0; row < PlayerLimit; ++row) {
        hide_row(row);
    }
    timer_label->hide(); // Vanilla shows it only while a countdown is active
    show();

    g_statres_dialog = dialog();

    helper::set_callback_timer(dialog(), layout::TimerIndex, layout::TimerPeriod, &timer_proc);
}

StatResource::~StatResource() {
    helper::clear_callback_timer(dialog(), layout::TimerIndex);
    if (g_statres_dialog == dialog()) {
        g_statres_dialog = nullptr;
    }
}

DialogBounds StatResource::container_bounds() {
    const v2s32 offset = hud_layout::anchor_offset(hud_layout::Anchor::TopBarRight);
    return DialogBounds::from_xywh(static_cast<s16>(offset.x), 0,
        layout::ContainerWidth, layout::ContainerHeight);
}

void StatResource::build_row(u32 row_index) {
    Row& row = m_rows[row_index];
    const s16 y = layout::FirstRowY + static_cast<s16>(row_index) * layout::RowPitch;
    auto row_bounds = [&](s16 x, s16 w) {
        return DialogBounds::from_xywh(x, y, w, layout::RowHeight);
    };

    row.supply = label(row_bounds(layout::SupplyX, layout::SupplyWidth), "");
    row.supply->on_draw([this, row_index](ui::Widget& widget) {
        if (g_hud_manager) draw_resource(widget, g_hud_manager->icons().supply(), m_rows[row_index]);
    });

    row.gas = label(row_bounds(layout::GasX, layout::GasWidth), "");
    row.gas->on_draw([this, row_index](ui::Widget& widget) {
        if (g_hud_manager) draw_resource(widget, g_hud_manager->icons().gas(), m_rows[row_index]);
    });

    row.minerals = label(row_bounds(layout::MineralsX, layout::MineralsWidth), "");
    row.minerals->on_draw([this, row_index](ui::Widget& widget) {
        if (g_hud_manager) draw_resource(widget, g_hud_manager->icons().mineral(), m_rows[row_index]);
    });

    row.apm = label(row_bounds(layout::ApmX, layout::ApmWidth), "");
    row.apm->on_draw([this, row_index](ui::Widget& widget) {
        draw_colortext(widget, m_rows[row_index], layout::ApmTextCode, true);
    });

    row.name = label(DialogBounds::from_ltrb(layout::NameX, y, layout::ProductionCursorX, y + layout::RowHeight), "");
    row.name->on_draw([this, row_index](ui::Widget& widget) {
        draw_colortext(widget, m_rows[row_index], layout::NameTextCode, false);
    });

    row.color_box = label(DialogBounds::from_xywh(layout::ColorBoxX,
        y + (layout::RowHeight - layout::ColorBoxSize) / 2, layout::ColorBoxSize, layout::ColorBoxSize), "");
    row.color_box->on_draw([this, row_index](ui::Widget& widget) {
        draw_color_box(widget, m_rows[row_index]);
    });

    for (u32 slot = 0; slot < RowSize; ++slot) {
        const s16 left = layout::ProductionCursorX - static_cast<s16>(slot + 1) * layout::ProductionPitch;
        row.production[slot] = label(DialogBounds::from_xywh(left, y, layout::ProductionSlotWidth, layout::RowHeight), "");
        row.production[slot]->on_draw([this, row_index, slot](ui::Widget& widget) {
            if (g_hud_manager) draw_production_slot(widget, m_rows[row_index], slot);
        });
    }
}

void __fastcall StatResource::timer_proc(BinDlg* dialog, u16) {
    auto* widget = static_cast<ui::Widget*>(dialog->to_dialog()->user);
    if (widget && widget->parent()) {
        static_cast<StatResource*>(widget->parent())->update();
    }
}

void StatResource::update() {
    const u32 frame = g_elapsed_time_frames;
    const bool alt = *g_is_holding_alt != 0;
    const bool is_hidden = g_hud_manager && g_hud_manager->is_hidden();
    const bool in_replay = bw::in_replay_or_witness();

    if (frame == m_last_frame && alt == m_last_alt && is_hidden == m_last_hidden && in_replay == m_last_in_replay) {
        return;
    }
    m_last_frame = frame;
    m_last_alt = alt;
    m_last_hidden = is_hidden;
    m_last_in_replay = in_replay;

    if (is_hidden) {
        return;
    }

    u32 rows_used = 0;
    if (in_replay) {
        for (u32 player = 0; player < PlayerLimit; ++player) {
            const auto& cm_player = g_globals->players[player];
            const u32 player_type = g_player_table[player].type;
            if ((player_type == PlayerType::Computer || player_type == PlayerType::Human)
                && !cm_player.witness_mode && !cm_player.hide_in_team_ui) {
                draw_player_stats(rows_used++, player,
                    true,               // use_player_color
                    g_config.show_apm,  // can_show_apm
                    true,               // show_production
                    true,               // show_name
                    true,               // show_color_box
                    alt);               // count_in_production
            }
        }
    } else {
        draw_player_stats(rows_used++, g_local_nation_id,
            false,                                                      // use_player_color
            g_config.show_team_ui_self && g_config.show_team_ui_apm,    // can_show_apm
            g_config.show_ingame_global_queue,                          // show_production
            g_config.show_team_ui_self && g_config.show_team_ui_names,  // show_name
            g_config.show_team_ui_self,                                 // show_color_box
            !alt);                                                      // count_in_production

        if (g_config.show_team_ui) {
            for (u32 player = 0; player < PlayerLimit; ++player) {
                const auto& cm_player = g_globals->players[player];
                const u32 player_type = g_player_table[player].type;
                if ((player_type == PlayerType::Computer || player_type == PlayerType::Human)
                    && player != g_local_nation_id && !cm_player.witness_mode
                    && !cm_player.hide_in_team_ui && bw::is_allied_to(player, g_local_nation_id)) {
                    draw_player_stats(rows_used++, player,
                        true,                               // use_player_color
                        g_config.show_team_ui_apm,          // can_show_apm
                        g_config.show_ingame_global_queue,  // show_production
                        g_config.show_team_ui_names,        // show_name
                        true,                               // show_color_box
                        !alt);                              // count_in_production
                }
            }
        }
    }

    for (u32 row = rows_used; row < PlayerLimit; ++row) {
        hide_row(row);
    }
}

void StatResource::draw_player_stats(u32 row_index, u32 player, bool use_player_color, bool can_show_apm,
    bool show_production, bool show_name, bool show_color_box, bool count_in_production) {
    Row& row = m_rows[row_index];
    row.player = player;
    row.use_player_color = use_player_color;
    row.icon_index = default_race_icon_index(static_cast<RaceId::Enum>(g_player_table[player].race));

    u32 workers = 0;
    u32 men = 0;
    for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
        if (count_in_production || unit->status & UnitStatus::Completed) {
            u32 unit_id = bw::is_zerg_egg(unit->id, true, false) ? unit->build_queue[unit->build_queue_slot] : unit->id;

            if (units_dat::base_property[unit_id] & UnitProperty::Worker) {
                ++workers;
            } else if (units_dat::group_flags[unit_id].is_men && !skip_military_tracking_by_unit_id(unit_id)
                && !(units_dat::base_property[unit_id] & UnitProperty::Subunit)) {
                ++men;
            }
        }
    }

    ensure_visible(row.supply);
    char supply_text[12];
    char* supply_end = std::to_chars(supply_text, supply_text + sizeof(supply_text), workers).ptr;
    *supply_end++ = '/';
    supply_end = std::to_chars(supply_end, supply_text + sizeof(supply_text), men).ptr;
    set_text_if_changed(row.supply, {supply_text, size_t(supply_end - supply_text)});

    ensure_visible(row.gas);
    update_displayed_resource(row.gas, g_displayed_gas[player], resources->gas[player]);

    ensure_visible(row.minerals);
    update_displayed_resource(row.minerals, g_displayed_minerals[player], resources->minerals[player]);

    if (can_show_apm) {
        ensure_visible(row.apm);
        const u16 display_value = g_player_table[player].type == PlayerType::Human
            ? u16(g_player_apm_counter[player].action_count / (ActionSampleTime / 60.0))
            : 0x8000; // Computers show "CPU"
        char apm_text[12];
        set_text_if_changed(row.apm, display_value & 0x8000
            ? std::string_view("CPU")
            : format_int(apm_text, display_value));
    } else {
        hide_if_visible(row.apm);
    }

    if (show_color_box) {
        ensure_visible(row.color_box);
    } else {
        hide_if_visible(row.color_box);
    }

    s16 production_cursor = layout::ProductionCursorX;
    u32 active_slots = 0;
    if (show_production) {
        for (u32 slot = 0; slot < RowSize; ++slot) {
            const u16 remaining_time = (u16)g_production.timers[player][slot];
            if (remaining_time == 65535) {
                break;
            }

            ui::Widget* widget = row.production[slot];
            ensure_visible(widget);
            row.production_type[slot] = g_production.types[player][slot];
            char count_text[12];
            set_text_if_changed(widget, format_int(count_text, g_production.counts[player][slot]));
            widget->invalidate();
            production_cursor -= layout::ProductionPitch;
            active_slots = slot + 1;
        }
    }
    for (u32 slot = active_slots; slot < RowSize; ++slot) {
        hide_if_visible(row.production[slot]);
    }

    if (show_name) {
        ensure_visible(row.name);
        set_text_if_changed(row.name, g_player_data.full_player_names[row.player]);

        // The name's right edge sits left of the packed production icons
        const s16 name_right = active_slots ? production_cursor - layout::NameGap : production_cursor;
        bw::Dialog* dialog = row.name->dialog();
        if (dialog->bounds.right() != name_right) {
            dialog->bounds = DialogBounds::from_ltrb(layout::NameX, dialog->bounds.top(), name_right, dialog->bounds.bottom());
            row.name->invalidate_parent(); // Erase the name's previous position
        }
    } else {
        hide_if_visible(row.name);
    }
}

void StatResource::hide_row(u32 row_index) {
    Row& row = m_rows[row_index];
    for (ui::Widget* widget : {row.supply, row.gas, row.minerals, row.apm, row.name, row.color_box}) {
        hide_if_visible(widget);
    }
    for (ui::Widget* widget : row.production) {
        hide_if_visible(widget);
    }
}

void StatResource::update_displayed_resource(ui::Widget* widget, s32& displayed, s32 resource_amount) {
    const s32 difference = resource_amount - displayed;
    if (difference) {
        displayed += g_elapsed_time_frames > 1 ? (((difference > 0) * 2) - 1) + (difference / 16) : difference;
    }
    char text[12];
    set_text_if_changed(widget, format_int(text, displayed));
}

// --------------------------------
// Widget draws
// --------------------------------

// Draws a GRP frame at the dialog's left edge, vertically centered. Returns the frame's width.
s16 StatResource::draw_grp_icon(bw::Dialog* dialog, graphics::GrpFile* icons, u16 frame_index) {
    if (!icons || !icons->frame_count()) {
        return 0;
    }
    if (frame_index >= icons->frame_count()) {
        frame_index = 0;
    }

    const graphics::FrameSize size = icons->frame_size(frame_index);
    const s16 width = s16(size.width);
    const s16 height = s16(size.height);
    Bounds bounds{0, 0, width, height, width, height};
    FxnDrawIconData data{(u16)width, (u16)height, icons->get_frame_pixel_data(frame_index)};
    helper::bw_print_icon(&bounds, &data, dialog->bounds.left(),
        dialog->bounds.top() + (dialog->bounds.h() - height) / 2);
    return width;
}

// Resource value: race icon + right-aligned green text
void StatResource::draw_resource(ui::Widget& widget, graphics::GrpFile* icons, Row& row) {
    bw::Dialog* dialog = widget.dialog();
    const s16 icon_width = draw_grp_icon(dialog, icons, row.icon_index);

    const std::string& text = widget.text();
    if (text.empty()) {
        return;
    }

    helper::set_font_handle((u32*)font_base[text.size() < layout::ResourceFontLengthLimit ? 1 : 0]);
    g_print_xy_alignment = 0x21;
    helper::get_color_handle(TextColor::Green);
    g_draw_text = dialog->bounds;

    helper::bw_print_xy(text.c_str(), dialog->bounds.left() + icon_width + 2, dialog->bounds.top());
    helper::set_font_handle(nullptr);
}

// Production slot: unit icon + white count + progress bar
void StatResource::draw_production_slot(ui::Widget& widget, Row& row, u32 slot) {
    bw::Dialog* dialog = widget.dialog();
    const u16 unit_type = row.production_type[slot];
    draw_grp_icon(dialog, g_hud_manager->icons().general(), unit_type + 12);

    const std::string& text = widget.text();
    if (!text.empty()) {
        const Box16 bounds = dialog->bounds;
        text::print(text, bounds.left, bounds.top, text::PrintOptions{}
            .clip({bounds.left, bounds.top, bounds.right, bounds.bottom})
            .px(layout::ProductionTextPx)
            .halign(text::HAlign::Right)
            .valign(text::VAlign::Bottom)
            .color(TextColor::White));
    }

    // Prevent underflow when queue compacts mid-tick
    const u32 time_cost = units_dat::time_cost[row.production_type[slot]];
    if (time_cost) {
        const u32 remaining_time = g_production.timers[row.player][slot];
        if (remaining_time <= time_cost) {
            const u32 progress = ((time_cost - remaining_time) * layout::ProductionBarWidth) / time_cost;
            if (progress) {
                *g_draw_color_index = layout::ProductionBarColor;
                helper::draw_box(dialog->bounds.left(), dialog->bounds.bottom(), (u16)progress, 1);
            }
        }
    }
}

// Player-colored or green text. font_by_length picks the font from the string length for the APM column.
// Names always use font_base[1].
void StatResource::draw_colortext(ui::Widget& widget, Row& row, s16 alignment, bool font_by_length) {
    const std::string& text = widget.text();
    if (text.empty()) {
        return;
    }

    bw::Dialog* dialog = widget.dialog();
    helper::set_font_handle((u32*)font_base[font_by_length ? (text.size() < layout::ApmFontLengthLimit ? 1 : 0) : 1]);
    g_draw_text = dialog->bounds;
    g_print_xy_alignment = alignment;
    g_font_spacing = 1;

    if (row.use_player_color) {
        g_saved_font_color = g_current_font_color;
        g_current_font_color = (u32)&g_tfontgame_image_data[PlayerFontSlot * 8];
        set_player_font_ramp((u8)helper::get_color_id(row.player));
    } else {
        helper::get_color_handle(TextColor::Green);
    }

    helper::bw_print_xy(text.c_str(), dialog->bounds.left(), dialog->bounds.top());
    helper::set_font_handle(nullptr);
}

// 18x18 minimap-color player box (ex draw_statres_box queue)
void StatResource::draw_color_box(ui::Widget& widget, Row& row) {
    bw::Dialog* dialog = widget.dialog();
    *g_draw_color_index = (char)get_minimap_color(row.player);
    helper::draw_box(dialog->bounds.left(), dialog->bounds.top(), layout::ColorBoxSize, layout::ColorBoxSize);
}
