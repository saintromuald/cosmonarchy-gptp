#include "hud.h"

#include "BW/ui.h"
#include "BW/structures.h"
#include "hooks/unit_stats/supply_hooks.h"
#include "graphics/Font.h"
#include "hooks/interface/mainmenu/glucustom.h" // For g_tfontgame_image_data
#include "player_color.h"
#include "hooks/interface/hud/hud.h"
#include "systems/triggers/trigger_system.h"
#include "config.h"
#include "hooks/interface/uifeatures/ui_features.h"

namespace {
struct StatresState {
    u16 timer_index = 1;
    u32 timer_duration = 20;

    char resources_buffer[ResSize][16];
    s32 displayed_gas[PlayerLimit];
    s32 displayed_minerals[PlayerLimit];
    graphics::Font* font;
};
StatresState statres_state;

static void statres_text_with_icon_update(BinDlg* dialog, GrpFile* icons) {
    s16 dialog_top = dialog->bounds.top;
    s16 dialog_height = dialog->bounds.bottom - dialog_top + 1;
    s16 dialog_left = dialog->bounds.left;

    if (dialog->graphic >= icons->frame_count()) dialog->graphic = 0;

    auto& current_frame = icons->frame_headers()[dialog->graphic];
    s16 width = current_frame.w;
    s16 height = current_frame.h;
    if (current_frame.data_offset < 0) {
        current_frame.data_offset &= 0x7FFFFFFF;
        width += 256;
    }

    Bounds bounds{0, 0, width, height, width, height};
    FxnDrawIconData data{width, height, icons->get_frame_pixel_data(dialog->graphic)};
    helper::bw_print_icon(&bounds, &data, dialog_left, dialog_top + (dialog_height - height) / 2);

    if (dialog->text) {
        if (dialog->index < 64) {
            helper::set_font_handle((u32*)dialog->statRes.font_handle);
            g_print_xy_alignment = 0x21;
            helper::get_color_handle(TextColor::Green);
            dialog_left += width + 2;
        } else {
            helper::set_font_handle((u32*)statres_state.font);
            g_print_xy_alignment = 0x44;
            g_font_spacing = 0;
            helper::get_color_handle(TextColor::White);
        }

        g_draw_text = {dialog->bounds.left, dialog->bounds.top, dialog->bounds.right, dialog->bounds.bottom};
        helper::bw_print_xy(dialog->text, dialog_left, dialog_top);
        helper::set_font_handle(NULL);
    }
}

static void __fastcall statres_minerals_update(BinDlg* dialog, int, int, int) {
    statres_text_with_icon_update(dialog, g_hud_icons.mineral());
}

static void __fastcall statres_gas_update(BinDlg* dialog, int, int, int) {
    statres_text_with_icon_update(dialog, g_hud_icons.gas());
}

static void __fastcall statres_supply_update(BinDlg* dialog, int, int, int) {
    statres_text_with_icon_update(dialog, g_hud_icons.supply());
}

static void __fastcall statres_unit_update(BinDlg* dialog, DialogEvent*, u32, void*) {
    statres_text_with_icon_update(dialog, g_hud_icons.general());
}

static void __fastcall statres_colortext_update(BinDlg* dialog, int, int, int) { // 004E5460
    if (dialog->text) {
        helper::set_font_handle((u32*)dialog->statRes.font_handle);
        g_draw_text = {dialog->bounds.left, dialog->bounds.top, dialog->bounds.right, dialog->bounds.bottom};
        g_print_xy_alignment = (s16)dialog->index < 0 ? 0x24 : 0x21;
        g_font_spacing = 1;

        if (dialog->non_user_value & 0x10000) { // use_player_color
            g_saved_font_color = g_current_font_color;
            g_current_font_color = (u32)&g_tfontgame_image_data[304];
            u32 player = dialog->non_user_value >> 24;
            u32 i_0 = 0;

            do {
                g_tfontgame_image_data[305 + i_0] = g_player_in_game_colors[player * ColorSetSize + i_0 * 2];
            } while (++i_0 < 4);
        } else {
            helper::get_color_handle(TextColor::Green);
        }

        helper::bw_print_xy(dialog->text, dialog->bounds.left, dialog->bounds.top);
        helper::set_font_handle(NULL);
    }
}

static void statres_prep_child_dialogs(BinDlg* dialog) {
    if (dialog) {
        BinDlg* parent_offset = dialog + ResSize;

        do { // this is just iterating over the children....
            auto control_type = dialog->control_type;
            if (control_type == DialogControlType::DialogBoxDC) {
                dialog = dialog->child_dialog;
                continue;
            }

            s16 index = dialog->index;

            if (index > 0) {
                if (index & 0xFFC3) { // index % 4 - All dialogs except player name
                    dialog->fxn_update = &statres_unit_update;

                    //Custom pointers to that row's player name and production dialogs, written to the bin file manually ("unknown3" in PyBIN's text export).
                    //It's not necessary to do it this way, but it's faster instead of stepping through the dialog->next pointers to get to them.
                    //It's however unconventional for control types 1+ to have BinDlg pointers, hence the need to initialize it here. -DF
                    u32 child_index = dialog->non_user_value;

                    if (child_index) {
                        dialog->statRes.player_name = child_index + parent_offset;
                        dialog->statRes.player_name->statRes.font_handle = (u32*)font_base[1];
                        dialog->statRes.player_name->fxn_update = &statres_colortext_update;
                        dialog->child_dialog = (RowSize * --child_index) + parent_offset + PlayerLimit + 1;
                    }
                } else { // Player name
                    dialog->fxn_update = &statres_colortext_update;
                }
            }

            dialog = dialog->next;
        } while (dialog);
    }

    if (!statres_state.font) {
        statres_state.font = helper::load_font("font_statres");
    }
}

static void show_supply_info(BinDlg* dialog, u16 graphic, u16 workers, u16 men) {
    if (!(dialog->flags & DialogFlags::Visible)) {
        helper::show_dialog(dialog);
    }

    u32 user_value = (men << 16) | workers;

    if (dialog->non_user_value != user_value || dialog->graphic != graphic || !dialog->text) {
        dialog->non_user_value = user_value;
        dialog->graphic = graphic;
        char* buffer = &statres_state.resources_buffer[dialog->index - 1][0];
        sprintf_s(buffer, 16, "%d/%d",
            workers,
            men);
        dialog->text = buffer;
        u32 len = 0;
        while (buffer[++len]);
        dialog->statRes.font_handle = (u32*)font_base[len < 8 ? 1 : 0];

        if (!(dialog->flags & DialogFlags::UpdateNeeded)) {
            dialog->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(dialog);
        }
    }
}

static void print_stat_res_info(BinDlg* dialog, u16 graphic, s32& display_amount, s32 resource_amount) {
    if (!(dialog->flags & DialogFlags::Visible)) {
        helper::show_dialog(dialog);
    }

    s32 difference = resource_amount - display_amount;
    if (difference) {
        display_amount += g_elapsed_time_frames > 1 ? (((difference > 0) * 2) - 1) + (difference / 16) : difference;
    }

    if (dialog->non_user_value != difference || dialog->graphic != graphic || !dialog->text) {
        dialog->non_user_value = difference;
        dialog->graphic = graphic;
        char* buffer = &statres_state.resources_buffer[dialog->index - 1][0];
        sprintf_s(buffer, 16, "%d", display_amount);
        dialog->text = buffer;
        u32 len = 0;
        while (buffer[++len]);
        dialog->statRes.font_handle = (u32*)font_base[len < 8 ? 1 : 0];

        if (!(dialog->flags & DialogFlags::UpdateNeeded)) {
            dialog->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(dialog);
        }
    }
}

static void print_player_apm(BinDlg* dialog, u32 user_value) {
    if (!(dialog->flags & DialogFlags::Visible)) {
        helper::show_dialog(dialog);
    }

    if (dialog->non_user_value != user_value || !dialog->text) {
        dialog->non_user_value = user_value;
        char* buffer = &statres_state.resources_buffer[dialog->index - 1][0];

        if ((s16)user_value >= 0) {
            sprintf_s(buffer, 16, "%d", (u16)user_value);
        } else {
            sprintf_s(buffer, 16, "CPU");
        }

        dialog->text = buffer;
        u32 len = 0;
        while (buffer[++len]);
        dialog->statRes.font_handle = (u32*)font_base[len < 5 ? 1 : 0];

        if (!(dialog->flags & DialogFlags::UpdateNeeded)) {
            dialog->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(dialog);
        }
    }
}

static void print_player_name(BinDlg* dialog, u32 user_value) {
    if (!(dialog->flags & DialogFlags::Visible)) {
        helper::show_dialog(dialog);
    }

    if (dialog->non_user_value != user_value || !dialog->text) {
        dialog->non_user_value = user_value;
        dialog->bounds.right = user_value & 0xffff;
        u32 player = user_value >> 24;
        dialog->text = g_player_data.full_player_names[player];

        if (!(dialog->flags & DialogFlags::UpdateNeeded)) {
            dialog->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(dialog);
        }
    }
}

static u32 show_production_info(BinDlg* dialog, u8 player, u16 x_offset) {
    if (!dialog) {
        return 0;
    }

    u16 row_size = dialog->index | RowSize;
    u32 user_offset = x_offset << 8;
    u32 i_0;

    for (i_0 = 0; i_0 < RowSize; ++i_0) {
        u32 remaining_time = (u16)g_production.timers[player][i_0];

        if (remaining_time == 65535) {
            break;
        }

        if (!(dialog->flags & DialogFlags::Visible)) {
            helper::show_dialog(dialog);
        }

        x_offset -= 15;
        u16 user_type = g_production.types[player][i_0];
        u8 user_count = g_production.counts[player][i_0];
        u32 user_value = (user_type << 20) | user_offset | user_count;

        if (dialog->non_user_value != user_value) {
            dialog->non_user_value = user_value;
            dialog->statRes.time_cost = (u16)units_dat::time_cost[user_type];
            dialog->graphic = user_type + 12;
            char* buffer = &stat_res_production_buffer[player][i_0][0];
            sprintf_s(buffer, 4, "%d", user_count);
            dialog->text = buffer;
            dialog->bounds.left = x_offset;
            dialog->bounds.right = x_offset + 14;

            if (!(dialog->flags & DialogFlags::UpdateNeeded)) {
                dialog->flags |= DialogFlags::UpdateNeeded;
                helper::update_dialog(dialog);
            }
        }

        u32 time_cost = dialog->statRes.time_cost;

        if (time_cost) {
            u32 x = (u16)dialog->bounds.left;
            u32 y = (u16)dialog->bounds.bottom;
            u32 progress = ((time_cost - remaining_time) * 15) / time_cost;

            if (progress) {
                g_graphics.draw_statres_bar(x, --progress + x, y, 117);
            }
        }

        dialog = dialog->next;
        if (!dialog) {
            goto loc_ret;
        }
    }

    do {
        if (dialog->flags & DialogFlags::Visible) {
            helper::hide_dialog(dialog);
        }

        dialog = dialog->next;
    } while (dialog && dialog->index < row_size);

loc_ret:
    return x_offset - (i_0 ? 2 : 0);
}

static void draw_player_stats(BinDlg** dialog_ptr, v2u16* pos, u32 player,
    bool use_player_color, bool can_show_apm, bool show_production, bool show_name, bool show_color_box, bool count_in_production) {
    // Track worker/military counts. -Veeq7
    u32 workers = 0;
    u32 men = 0;

    for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
        if (count_in_production || unit->status & UnitStatus::Completed) {
            u32 unit_id = bw::is_zerg_egg(unit->id, true, false) ? unit->build_queue[unit->build_queue_slot] : unit->id;

            if (units_dat::base_property[unit_id] & UnitProperty::Worker) {
                ++workers;
            } else if (units_dat::group_flags[unit_id].is_men && !skip_military_tracking_by_unit_id(unit_id) && !(units_dat::base_property[unit_id] & UnitProperty::Subunit)) {
                ++men;
            }
        }
    }

    BinDlg* dialog = *dialog_ptr;
    BinDlg* dialog_name = dialog->statRes.player_name;
    BinDlg* dialog_production = dialog->child_dialog;

    auto icon_index = default_race_icon_index(static_cast<RaceId::Enum>(g_player_table[player].race));

    dialog->fxn_update = &statres_supply_update;
    show_supply_info(dialog, icon_index, workers, men);
    dialog = dialog->next;

    dialog->fxn_update = &statres_gas_update;
    print_stat_res_info(dialog, icon_index, statres_state.displayed_gas[player], resources->gas[player]);
    dialog = dialog->next;

    dialog->fxn_update = &statres_minerals_update;
    print_stat_res_info(dialog, icon_index, statres_state.displayed_minerals[player], resources->minerals[player]);
    dialog = dialog->next;

    v2u16 local_pos = *pos;
    u32 user_value = (player << 24) | (use_player_color << 16);

    if (can_show_apm) {
        u16 display_value = g_player_table[player].type == PlayerType::Human ? g_player_apm_counter[player].action_count / (ActionSampleTime / 60.0) : 0x8000;
        print_player_apm(dialog, user_value | display_value);
        local_pos.x -= 33;
    } else if (dialog->flags & DialogFlags::Visible) {
        helper::hide_dialog(dialog);
    }

    if (show_color_box) {
        local_pos.x -= 12;
        g_graphics.draw_statres_box(local_pos.x + 2, local_pos.y + 2, local_pos.x + 11, local_pos.y + 11, g_player_minimap_colors[player]);
    }

    if (show_production) {
        local_pos.x = show_production_info(dialog_production, player, local_pos.x);
    }

    if (show_name) {
        print_player_name(dialog_name, user_value | local_pos.x);
    } else if (dialog_name->flags & DialogFlags::Visible) {
        helper::hide_dialog(dialog_name);
    }

    pos->y += 15;
    *dialog_ptr = dialog->next;
}

// Hook edited by DarkenedFantasies for new replay UI. -Pr0nogo
static void __fastcall stat_res_timer_proc(BinDlg* dialog, u16 index_unused) { // 004E5640
    if (dialog->control_type != DialogControlType::DialogBoxDC) {
        dialog = dialog->parent;
    }
    dialog = dialog->child_dialog;
    while (dialog->index != 1) { // This while loop is not necessary with a properly organized bin file, but keep it for safety. -DF
        if (!dialog) {
            return; // The original function would set dialog to null and break out of the loop. Surely that can't be good. -DF
        }
        dialog = dialog->next;
    }

    g_graphics.reset_stat_res_graphics();
    u32 local_player = g_local_nation_id;
    v2u16 pos = {434, 3};//x is dialog->bounds.left of the third row element (from the right), minus 2. -DF

    if (bw::in_replay_or_witness()) {
        for (u32 player = 0; player < PlayerLimit; ++player) {
            const auto& cm_player = g_globals->players[player];
            u32 player_type = g_player_table[player].type;
            if ((player_type == PlayerType::Computer || player_type == PlayerType::Human) && !cm_player.witness_mode && !cm_player.hide_in_team_ui) {
                draw_player_stats(&dialog, &pos, player,
                    true,					// use_player_color
                    g_config.show_apm,		// can_show_apm
                    true,					// show_production
                    true,						// show_name
                    true,					// show_color_box
                    *g_is_holding_alt);	// count_in_production
            }
        }
    } else { // Normal player UI. -Veeq7
        draw_player_stats(&dialog, &pos, local_player,
            false,                                                      // use_player_color
            g_config.show_team_ui_self && g_config.show_team_ui_apm,    // can_show_apm
            g_config.show_ingame_global_queue,                          // show_production
            g_config.show_team_ui_self && g_config.show_team_ui_names,  // show_name
            g_config.show_team_ui_self,                                 // show_color_box
            !*g_is_holding_alt);                                        // count_in_production

        if (g_config.show_team_ui) {
            for (u32 player = 0; player < PlayerLimit; ++player) {
                const auto& cm_player = g_globals->players[player];
                u32 player_type = g_player_table[player].type;
                if ((player_type == PlayerType::Computer || player_type == PlayerType::Human) && player != local_player && !cm_player.witness_mode
                    && !cm_player.hide_in_team_ui && bw::is_allied_to(player, local_player)) {
                    draw_player_stats(&dialog, &pos, player,
                        true,                               // use_player_color
                        g_config.show_team_ui_apm,          // can_show_apm
                        g_config.show_ingame_global_queue,  // show_production
                        g_config.show_team_ui_names,        // show_name
                        true,                               // show_color_box
                        !*g_is_holding_alt);                // count_in_production
                }
            }
        }
    }

    while (dialog->index <= ResSize) {
        if (dialog->flags & DialogFlags::Visible) {
            helper::hide_dialog(dialog);
            BinDlg* child = dialog->statRes.player_name; // dialog_name
            if (child) {
                if (child->flags & DialogFlags::Visible) {
                    helper::hide_dialog(child);
                }
                child = dialog->child_dialog; // dialog_production
                for (u32 i_0 = RowSize; i_0; --i_0) {
                    if (child->flags & DialogFlags::Visible) {
                        helper::hide_dialog(child);
                    }

                    child = child->next;
                }
            }
        }
        dialog = dialog->next;
    }
}

static void statres_dialogproc_create(bw::Dialog* dialog) { // 004e58d0
    dialog->flags |= DialogFlags::UseAlternateStyle | DialogFlags::ReverseDialogDirection | DialogFlags::TransparentHideColor;
    statres_prep_child_dialogs(dialog->to_bindlg());
    helper::set_callback_timer(dialog, statres_state.timer_index, statres_state.timer_duration, &stat_res_timer_proc);
}

static u32 __fastcall statres_dialogproc(bw::Dialog* dialog, DialogEvent* event) { // 004e5910
    switch (event->number_order) {
        case DialogEventType_KeyDown:
        case DialogEventType_KeyRepeat:
        case DialogEventType_LButtonDown:
        case DialogEventType_LButtonDouble:
        case DialogEventType_RButtonDown:
        case DialogEventType_RButtonDouble:
        case DialogEventType_Spacebar: {
            return 0;
        } break;
        case DialogEventType_Dialog: {
            switch (event->user) {
                case DialogUserType_MouseOver: { return 0; } break;
                case DialogUserType_Create: {
                    statres_dialogproc_create(dialog);
                } break;
                case DialogUserType_Destroy: {
                    helper::clear_callback_timer(dialog, statres_state.timer_index);
                } break;
            }
        } break;
    }
    return helper::generic_dialog_interact(dialog, event);
}
} // namespace

void load_statres() { // 004e5990
    g_statres_dialog = load_bin_file("rez\\statres.bin").release();
    initialize_bin_dialog(g_statres_dialog, statres_dialogproc);
}