#include "ui_features.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <format>
#include <Windows.h>
#include "config.h"
#include "systems/triggers/trigger_objectives.h"
#include "hooks/audiovisuals/sfxhooks.h"
#include "hooks/game_hooks.h"
#include "hooks/interface/hud/hud_manager.h"
#include "systems/triggers/trigger_system.h"
#include "hooks/interface/minimap.h"
#include "hooks/display/mouse.h"
#include "hooks/display/grid.h"
#include "hooks/display/perf_tracker.h"
#include "Game.h"
#include "hooks/interface/weapon_armor_tooltip.h"
#include "hooks/snp.h"
#include "hooks/interface/mainmenu/custom_glues.h"
#include "hooks/race_randomization.h"
#include "storm/storm.h"
#include "systems/display/display.h"
#include "systems/display/zoom.h"
#include "CM/cm_ids.h"

static s32* const g_fonts = (s32*)0x006CE0F4;
static u32* const g_print_xy_pos_y = (u32*)0x006CE0E0;
static Box16* const g_draw_text_box = (Box16*)0x006CE0C8;

CmdBtnPalette g_new_cmdbtn_palette[tIconCount];

void format_time(char* buffer, u32 size, const char* prefix, u32 seconds, bool allow_short) {
    u32 sec = seconds % 60;
    u32 min = seconds / 60 % 60;
    u32 hrs = seconds / 60 / 60;

    if (allow_short && !hrs) {
        sprintf_s(buffer, size, "%s%02u:%02u", prefix, min, sec);
    } else {
        sprintf_s(buffer, size, "%s%02u:%02u:%02u", prefix, hrs, min, sec);
    }
}

namespace hooks {
    void draw_text(const char* text, u32 x, u32 y, u32 w, u32 h, TextColor color) {
        helper::get_color_handle(color);
        g_print_xy_alignment = 0x11;
        g_draw_text_box->left = x;
        g_draw_text_box->right = x + w;
        g_draw_text_box->top = y;
        g_draw_text_box->bottom = y + h;
        helper::bwfxn_print_xy(text, x, y);
    }

    void draw_ui_text(const char* text, TextColor color) {
        draw_text(text, g_ui_text_pos.x, g_ui_text_pos.y, 300, UiTextHeight * 4, color);
        g_ui_text_pos.y = *g_print_xy_pos_y;
    }

    void register_action(ApmCounter* counter, char* buffer) {
        switch (buffer[0]) {
            case 84: // Start game?
            case 55: // Create preplaced unit?
                return;
            case 99: // Custom
                switch (buffer[1]) {
                case NewGameCommand::CameraSync:
                case NewGameCommand::UuidRand:
                    return;
                }
        }
        if (counter->action_count < MaxActions) {
            counter->action_time[counter->action_count++] = g_elapsed_time_frames;
        }
    }

    void init_actions() {
        memset(&g_local_apm_counter, 0, sizeof(g_local_apm_counter));
        memset(g_player_apm_counter, 0, sizeof(g_player_apm_counter));
    }

    void update_apm_counter(ApmCounter* counter) {
        for (u32 i = 0; i < counter->action_count; ++i) {
            s32 delta = g_elapsed_time_frames - counter->action_time[i];
            if (delta >= ActionSampleTime * 24) {
                counter->action_time[i--] = counter->action_time[--counter->action_count];
            }
        }
    }

    void update_actions() {
        update_apm_counter(&g_local_apm_counter);
        for (u32 i = 0; i < PlayerLimit; ++i) {
            update_apm_counter(&g_player_apm_counter[i]);
        }
    }

    void appendf(char** buffer_ptr, s32* size, const char* format, ...) {
        if (*size <= 0) {
            return;
        }

        va_list va;
        va_start(va, format);
        u32 chars_written = vsnprintf(*buffer_ptr, *size, format, va);
        va_end(va);

        *buffer_ptr += chars_written;
        *size       -= chars_written;
    }

    void draw_ui_features() {
        if (g_hud_manager && g_hud_manager->is_hidden()) return;

        g_ui_text_pos = { 10, 3 };
        if (*leaderboard_type) {
            g_ui_text_pos.x += 80;
        }

        if (g_config.show_timer) {
            char buffer[48] = {};
            format_time(buffer, 48, "", g_elapsed_time_frames * 0.042, false);
            draw_ui_text(buffer, TextColor::Green);
        }

        bool is_replay_or_witness = g_is_in_replay || g_globals->players[g_local_nation_id].witness_mode;
        if (!is_replay_or_witness && g_config.show_apm) {
            char buffer[48] = {};
            u32 apm = g_local_apm_counter.action_count / (ActionSampleTime / 60.0);
            sprintf_s(buffer, "APM: %u", apm);
            draw_ui_text(buffer, TextColor::Green);
        }

        if (g_config.show_perf_stats) {
            const perf::Window& window = perf::last_window();
            const s32 text_left = g_ui_text_pos.x;
            const s32 text_top = g_ui_text_pos.y;
            char lines[3][64] = {};

            char* pos = lines[0];
            s32 size = sizeof(lines[0]);
            appendf(&pos, &size, "tick %u.%ums pres %u.%ums (max %u.%u/%u.%u)",
                window.tick_avg_us / 1000, window.tick_avg_us / 100 % 10,
                window.present_avg_us / 1000, window.present_avg_us / 100 % 10,
                window.tick_max_us / 1000, window.tick_max_us / 100 % 10,
                window.present_max_us / 1000, window.present_max_us / 100 % 10);

            pos = lines[1];
            size = sizeof(lines[1]);
            appendf(&pos, &size, "res %u.%ums blt %u.%ums busy %u%% dirty %u%%",
                window.resample_avg_us / 1000, window.resample_avg_us / 100 % 10,
                window.blit_avg_us / 1000, window.blit_avg_us / 100 % 10,
                window.busy_percent, window.dirty_percent);

            pos = lines[2];
            size = sizeof(lines[2]);
            appendf(&pos, &size, "presents %u/s (tick %u scroll %u ui %u)",
                window.present_count, window.present_tick, window.present_scroll, window.present_ui);

            // At rest nothing else marks the readout's cells; mark only when the text
            // changed so updates reach the primary without forcing extra presents.
            static char previous[3][64] = {};
            const bool changed = std::memcmp(previous, lines, sizeof(lines)) != 0;
            if (changed) {
                std::memcpy(previous, lines, sizeof(lines));
            }
            for (const char* line : lines) {
                draw_ui_text(line, TextColor::Green);
            }
            if (changed) {
                mark_rect(text_left, text_top, text_left + 300, g_ui_text_pos.y - 1);
            }
        }

        for (const auto& [key, objective] : g_objectives) {
            if (objective.show) {
                char buffer[256] = {};
                char* pos = buffer;
                s32 size = 256;

                appendf(&pos, &size, "- ");
                if (objective.max_score == 1) {
                    appendf(&pos, &size, "%c ", objective.score ? 142 : 141, objective.text.c_str());
                } else if (objective.max_score) {
                    appendf(&pos, &size, "[%u/%u] ", objective.score, objective.max_score);
                }
                appendf(&pos, &size, "%s", objective.text.c_str());
                if (objective.timer) {
                    char time[256] = {};
                    format_time(time, 256, "", objective.timer / 24, true);
                    appendf(&pos, &size, " (%s)", time);
                }
                draw_ui_text(buffer, TextColor::Green);
            }
        }

        constexpr u32 ZoomIndicatorMs = 500;
        constexpr s32 ZoomIndicatorOffsetX = 16;
        constexpr s32 ZoomIndicatorOffsetY = 3;
        constexpr s32 ZoomIndicatorWidth = 64;

        static bool indicator_shown = false;
        static s32 indicator_left = 0;
        static s32 indicator_top = 0;
        static std::string indicator_text;

        std::string text;
        s32 left = 0;
        s32 top = 0;
        const bool visible = GetTickCount() - g_zoom.last_adjusted_tick() < ZoomIndicatorMs;
        if (visible) {
            text = std::format("{}%", g_zoom.player_percent());
            left = std::clamp(g_mouse->x + ZoomIndicatorOffsetX, 0, s32(g_display.display_width()) - ZoomIndicatorWidth);
            top = std::clamp(g_mouse->y + ZoomIndicatorOffsetY, 0, s32(g_display.display_height()) - s32(UiTextHeight));
        }

        const bool text_changed = text != indicator_text;
        const bool moved = visible && (left != indicator_left || top != indicator_top);

        if (indicator_shown && (moved || text_changed || !visible)) {
            mark_rect(indicator_left, indicator_top, indicator_left + ZoomIndicatorWidth, indicator_top + s32(UiTextHeight));
        }
        if (visible) {
            draw_text(text.c_str(), u32(left), u32(top), u32(ZoomIndicatorWidth), UiTextHeight * 4, TextColor::White);
            if (moved || text_changed || !indicator_shown) {
                mark_rect(left, top, left + ZoomIndicatorWidth, top + s32(UiTextHeight));
            }
        }
        if (text_changed) {
            indicator_text = text;
        }
        indicator_left = left;
        indicator_top = top;
        indicator_shown = visible;
    }

    void queue_game_command(u8* data, u32 size) {
        if (*g_command_queue_size + size <= *g_provider_max_buffer) {
            register_action(&g_local_apm_counter, (char*)data);
            memcpy((void*)&g_turn_buffer[*g_command_queue_size], data, size);
            *g_command_queue_size += size;
            return;
        }

        if (g_sc_main_state == MainState::Glues) {
            return;
        }

        u32 turns = 0;
        if (SNetGetTurnsInTransit(&turns)) {
            if (turns >= 16 - g_provider_turns_in_transit) {
                return;
            }
            register_action(&g_local_apm_counter, (char*)data);
            helper::send_turn();
            memcpy((void*)&g_turn_buffer[*g_command_queue_size], data, size);
            *g_command_queue_size += size;
            return;
        }

        if (!g_out_of_game) {
            u32 error = SErrGetLastError();
            helper::packet_error_handle(error, 81, "Starcraft\\SWAR\\lang\\net_mgr.cpp", 225, 1);
        }
    }

    s32 select_units(Unit** a1, s32 a2, u8 a3, s32 a4) {
        s32 v4 = helper::unit_to_unit_id(*a1);
        s32 result = helper::select_single_unit_from_id(v4); // eax

        if (a2 != 1 || !g_is_keycode_used[18] || result == 0) {
            result = helper::select_multiple_units_from_unit_list(a2, a1, a3, a4);
        }
        return result;
    }

    b32 check_hotkey_char(BinDlg* dialog, char hotkey) {
        if (dialog->index < 0 || dialog->index > 9) {
            return 0;
        }
        if (!(dialog->flags & DialogFlags::Visible) || (dialog->flags & DialogFlags::Disabled)) {
            return 0;
        }

        const char* label = helper::get_stat_txt_string(dialog->button_user->requirement_string_id);
        if (*label == VK_ESCAPE && hotkey == label[0]) { // escape handling for grid hotkeys
            return 1;
        }
        
        if (g_config.grid_hotkeys && dialog->index > 0) {
            u32 index = dialog->index - 1;
            char grid_hotkey = tolower(g_config.grid_layout[index / 3].at(index % 3));
            return hotkey == grid_hotkey;
        }

        return hotkey == label[0];
    }

    char create_upgrade_level_tooltip(char* text) {
        if (!g_upgrade_level_tooltip) {
            if (text) {
                return helper::create_tooltip(text);
            }
            return helper::refresh_selection_screen();
        }
        if (!text) {
            return helper::refresh_selection_screen();
        }
        const char* description;
        if (g_stat_txt_buffer.get_string_count32() > 794) {
            description = (char*)g_stat_txt_buffer.get_string32(795);
        } else {
            description = "";
        }
        char buffer[128];
        sprintf_s(buffer, 128, "%s\n%s %d", text, description, (s16)g_upgrade_level_tooltip);
        bw::printf("%s", buffer);
        return helper::create_tooltip(buffer);
    }

    // Hooked to improve functionality and to use in our own dialogue system. -Pr0nogo
    bool trigact_transmission(TriggerAction* trigger) { // 004C7890
        if (trigger->source_location_id == 0 || trigger->source_location_id > 255) {
            return true;
        }
        u32 player_id = *g_current_trigger_player_id;
        u8 player_type = g_player_table[player_id].type;
        if (player_type == PlayerType::Computer || player_type == PlayerType::Rescuable || player_type == PlayerType::UnusedRescueActive || player_type == PlayerType::Neutral) {
            return true;
        }
        if (!g_globals->disable_transmission_waits && g_is_executing_wait[player_id]) {
            return false;
        }
        u8 trigger_flags = trigger->flags;
        if (trigger_flags & 1) {
            trigger->flags = trigger_flags & ~1;
            return true;
        }
        if (g_current_trigger_node->trigger.flags & 0x10) {
            return true;
        }

        u32 trigger_time = trigger->time;
        switch (trigger->modifier) {
            case 7: {
                trigger_time = trigger->extra;
            } break;
            case 8: {
                trigger_time += trigger->extra;
            } break;
            case 9: {
                if (trigger->extra <= trigger_time) {
                    trigger_time -= trigger->extra;
                } else {
                    trigger_time = 0;
                }
            } break;
        }

        if (!g_globals->disable_transmission_waits) {
            trigger->flags = trigger_flags | 1;
            u32 multiplayer_time = g_multiplayer_mode ? trigger_time : -1;
            g_trigger_wait[player_id] = multiplayer_time;
            g_is_executing_wait[player_id] = 1;
        }

        if (player_id == g_local_nation_id) {
            helper::trigact_play_wav(trigger);
            Unit* speaker = helper::get_doodad_for_unit_state(trigger->source_location_id - 1, trigger->type, 17);
            if (speaker) {
                speaker->sprite->selection_timer = 45;
                helper::minimap_ping(speaker->sprite->position.x, speaker->sprite->position.y, 17);
                helper::game_talking_portrait(speaker->sprite->position.x, trigger_time, trigger->type, speaker->sprite->position.y);
            } else {
                helper::game_talking_portrait(-1, trigger_time, trigger->type, -1);
            }
            if (g_options_list->flags & SoundOptionFlags::Subtitles || trigger->flags & TriggerFlags::AlwaysDisplay) {
                char* string = helper::get_chk_string(trigger->text_string);
                u32 display_time = helper::get_text_display_time(string);
                if (trigger_time > display_time) {
                    helper::create_text_message_with_timer(string, trigger_time);
                } else {
                    helper::create_text_message_with_timer(string, display_time);
                }
            }
        }
        return g_globals->disable_transmission_waits;
    }

    // Hooked to add features and color Vespene Ridges cyan. -Pr0nogo
    void draw_minimap_unit_box(u32 player) { // 004A4650
        u32 unit_id = UnitId::None;
        auto color_id = get_minimap_color(player);
        Unit* unit = first_player_unit->unit[player];
        if (unit) {
            // Required for goto to compile in C++20 mode :( -Veeq7
            u32 visibility;
            u32 vision;

            u32 checksum = g_minimap_box_checksum;
            while (true) {
                if (unit_id_ignored_on_minimap(unit->id)) {
                    goto next;
                }

                visibility = unit->sprite->visibility_status;
                if (unit->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) {
                    visibility &= unit->detection_status;
                }
                checksum ^= visibility;
                if (g_minimap_draw_neutrals) {
                    break;
                }

                vision = 0;
                if (g_is_in_replay) {
                    if (visibility & *g_replay_visions) {
                        break;
                    }
                    vision = !*g_replay_reveal_all;
                } else {
                    vision = (visibility & *g_player_visions) == 0;
                }
                if (!vision) {
                    break;
                }
            next:
                unit = unit->player_link.next;
                if (!unit) {
                    g_minimap_box_checksum = checksum;
                    return;
                }
            }
            u32 color = g_palette_index_reference[25];
            if ((!unit_id_is_minerals(unit->id) && !unit_id_is_uncapped_gas_node(unit->id)) || player == NeutralHostilePlayer) {
                color = color_id;
            }

            v2u16 pos = unit->sprite->position;
            v2u16 dims = units_dat::building_dimensions[unit->id];
            helper::draw_unit_box(pos.x, pos.y, dims.x, dims.y, color, unit->is_building());
            goto next;
        }
    }

    // Mirrored from IDA to draw camera positions. -Pr0nogo
    static void draw_camera_box(u32 x, u32 y, u8 color) { // based on 004A3EE0
        Box32 box = {};
        u32 zoom_level = *g_minimap_zoom_level;
        box.x = x / zoom_level;
        box.y = y / zoom_level;
        // Other players' zoom levels aren't synced (no network message, not in
        // replay serialization); draw their boxes at the 200% default
        // equivalent — visible world = display / 2, with the engine's +16 slack.
        const u32 assumed_visible_width = g_display.display_width() / 2;
        const u32 assumed_visible_height = g_display.display_height() / 2;
        box.w = u16(std::max((assumed_visible_width + zoom_level - 1) / zoom_level, 2u));
        box.h = u16(std::max((assumed_visible_height + 16 + zoom_level - 1) / zoom_level, 1u));

        box.x = std::clamp<s32>(box.x, 0, g_map_width);
        box.y = std::clamp<s32>(box.y, 0, g_map_height);

        if (box.x + box.w >= g_map_width) {
            box.w = g_map_width - box.x;
        }

        if (box.y + box.h >= g_map_height) {
            box.h = g_map_height - box.y;
        }

        *g_draw_color_index = color;
        helper::draw_box(box.x,             box.y,             box.w, 1);
        helper::draw_box(box.x,             box.y,             1,     box.h);
        helper::draw_box(box.x + box.w - 1, box.y,             1,     box.h);
        helper::draw_box(box.x,             box.y + box.h - 1, box.w, 1);
    }

    // Hooked to add features and color Vespene Ridges cyan. -Pr0nogo
    void draw_all_minimap_boxes() { // 004A4AC0
        Bitmap* surface = *g_current_surface;
        u32 player = g_local_nation_id;
        *g_current_surface = g_minimap_surface_fog;
        *g_minimap_unit_box_count = 0;
        g_minimap_box_checksum = 0;

        u32 index = 12;
        while (index) {
            if (--index < PlayerLimit) {
                if (player != index || g_is_in_replay) {
                    helper::draw_minimap_unit_box_2(index);
                }
            } else {
                helper::draw_minimap_unit_box(index);
            }
        }
        if (!g_is_in_replay) {
            helper::draw_all_minimap_unit_boxes(player);
        }
        
        for (Thingy* thingy = *g_first_fow_sprite; thingy; thingy = thingy->next) {
            if (unit_id_is_spell(thingy->unit_id)) {
                continue;
            }
            bool not_visible = helper::thingy_visiblity_check(1, thingy) && !unit_id_is_resource_node(thingy->unit_id, true);
            if (g_is_in_replay && not_visible) {
                continue;
            }

            Sprite* sprite = thingy->sprite;
            auto color_id = g_palette_index_reference[25]; // Neutral
            if (!SpriteId::is_resource_node((SpriteId::Enum)sprite->id)) {
                color_id = get_minimap_color(sprite->player);
            }
            v2u16 pos = sprite->position;
            // Previously revealed resource nodes here, but we implemented that in game_hooks long ago. -Pr0nogo
            v2u16 dims = units_dat::building_dimensions[thingy->unit_id];
            helper::draw_unit_box(pos.x, pos.y, dims.x, dims.y, color_id, units_dat::base_property[thingy->unit_id] & UnitProperty::Building);
            --*g_minimap_unit_box_count;
        }

        if (g_config.show_minimap_cameras) {
            for (u8 player = 0; player < PlayerLimit; ++player) {
                if (g_player_table[player].type != PlayerType::Human || g_globals->players[player].witness_mode) {
                    continue;
                }
                if (!g_is_in_replay && player == g_local_nation_id) {
                    continue;
                }
                if (!g_is_in_replay && !g_globals->players[g_local_nation_id].witness_mode && !bw::is_allied_to(player, g_local_nation_id)) {
                    continue;
                }
                v2u16 camera = g_globals->players[player].camera;
                if (camera.x || camera.y) {
                    draw_camera_box(camera.x, camera.y, get_minimap_color(player));
                }
            }
        }

        *g_current_surface = surface;
    }

    enum CursorType {
        CursorType_Normal	        = 0,
        CursorType_TargetingNeutral = 2,
        CursorType_TargetingEnemy   = 3,
        CursorType_TargetingAlly    = 4,
        CursorType_TargetingNoUnit  = 5,
        CursorType_HoveringAlly     = 6,
        CursorType_HoveringEnemy    = 7,
        CursorType_HoveringNeutral  = 8,
        CursorType_DragSelect       = 9,
    };

    // Hooked for Veeq7's transport UI. -Pr0nogo
    u32 get_cursor_type() { // 004D1460
        if (buttons_hovers()) {
            return CursorType_HoveringAlly;
        }
        if (g_game && g_game->m_transport_ui_unit) {
            if (g_game->m_transport_ui_unit->player == 11) {
                return CursorType_HoveringNeutral;
            }
            if (helper::unit_is_enemy(g_game->m_transport_ui_unit)) {
                return CursorType_HoveringEnemy;
            }
            return CursorType_HoveringAlly;
        }
        if (PtInRect(g_minimap_rect, *(POINT*)g_mouse)) {
            return g_minimap_cursor;
        }
        // 004D1140 expects zoom-mapped world coordinates (the input procs receive
        // mapped events); the raw cursor would test the top-left visible-rect
        // portion of the window instead of the whole magnified view.
        if (helper::is_outside_of_game_screen(g_world_cursor.x, g_world_cursor.y) || !PtInRect(g_cursor_limit_rect, *(POINT*)g_mouse)) {
            return CursorType_Normal;
        }

        if (hud_covers(g_mouse->x, g_mouse->y)) {
            return CursorType_Normal;
        }
        if (*g_is_drag_selecting) {
            return CursorType_DragSelect;
        }
        if (g_is_placing_building) {
            return CursorType_Normal;
        }
        // Hover check queries the world under the cursor — the zoom-mapped
        // position, not the raw cursor (identity at 100%, wrong once magnified).
        Unit* unit_at_point = helper::find_unit_at_point(g_world_cursor.x + g_move_to_x, g_move_to_y + g_world_cursor.y);
        if (g_is_targeting) {
            if (!unit_at_point) {
                return CursorType_TargetingNoUnit;
            }
            if (unit_at_point->player == 11) {
                return CursorType_TargetingNeutral;
            }
            if (helper::unit_is_enemy(unit_at_point)) {
                return CursorType_TargetingEnemy;
            }
            return CursorType_TargetingAlly;
        }
        if (!unit_at_point) {
            return CursorType_Normal;
        }
        if (unit_at_point->player == 11) {
            return CursorType_HoveringNeutral;
        }
        if (helper::unit_is_enemy(unit_at_point)) {
            return CursorType_HoveringEnemy;
        }
        return CursorType_HoveringAlly;
    }

    // Hooked to allow for aggressive unit responses. -Pr0nogo
    void cmdact_right_click_minimap(DialogEvent* dlg_event) // 004563A0
    {
        if (*g_active_portrait_unit && (*g_active_portrait_unit)->player == g_local_nation_id) {
            u32 selection = 0;
            while (1)
            {
                Unit* selected = g_client_selection_group->unit[selection];
                if (selected) {
                    if (helper::accepts_right_click_commands(selected)) {
                        break;
                    }
                }
                if (++selection >= SelectionCap) {
                    return;
                }
            }
            if (selection < SelectionCap) {
                u32 selection_2 = 0;
                while (1)
                {
                    Unit* selected_2 = g_client_selection_group->unit[selection_2];
                    if (selected_2) {
                        if (helper::can_move(selected_2)) {
                            break;
                        }
                    }
                    if (++selection_2 >= SelectionCap) {
                        return;
                    }
                }
                if (selection_2 < SelectionCap) {
                    u32 x_pos = dlg_event->current.x;
                    u32 y_pos = dlg_event->current.y;
                    helper::get_minimap_cursor_pos((u16*)&x_pos, (u16*)&y_pos);
                    u16 mouse_pos_x = x_pos;
                    u16 mouse_pos_y = y_pos;
                    u8 command[10] = {};
                    command[0] = 20;
                    *(u16*)&command[1] = x_pos;
                    *(u16*)&command[3] = y_pos;
                    *(u16*)&command[5] = 0;
                    *(u16*)&command[7] = 228;
                    command[9] = g_is_keycode_used[16];
                    queue_game_command(command, 10);
                    hooks::play_yes_unit_response(*g_active_portrait_unit);
                    helper::show_cursor_marker(mouse_pos_x, mouse_pos_y);
                }
            }
        }
    }

    // Hooked to bandage null string crashes. -Pr0nogo
    // TODO: Fix lack of vertical lines introduced by this hook.
    u8* draw_vertical_line(u16 iterations, s16 width_offset, u16 buffer_multiplier) { // 0041D7D0
        if (!buffer_multiplier) {
            return 0;
        }
        Bitmap* surface = *g_current_surface;
        u8* result = &surface->buffer[buffer_multiplier * surface->width + width_offset];
        if (iterations) {
            u8 index = *g_draw_color_index;
            do {
                *result = index;
                result += surface->width;
                --iterations;
            } while (iterations);
        }
        return result;
    }

    // Hooked to improve functionality. -Veeq7
    void __fastcall centerview_unit_group(u8 slot) { // 004967E0
        // Handled in hotkeys.cpp now
    }

    // Hooked to explore colorizing passive icons. -Pr0nogo
    b32 generic_dialog_update(BinDlg* button, u32 width, u32 height, Box16* dims) { // 004EF590
        if (button->bitmap.buffer) {
            return helper::draw_dialog_image(dims, &button->bitmap, width, height, 0);
        }
        *g_draw_color_index = !(button->flags & DialogFlags::TransparentHideColor) ? g_palette_index_reference[12] : 0;
        return helper::draw_box(dims->x, dims->y, dims->w - dims->x + 1, dims->h - dims->y + 1);
    }

    // Hooked to explore colorizing passive icons. -Pr0nogo
    void draw_stat_button_image(BinDlg* dialog) { // 00456C30
        u8 palette_type = 3;
        if ((dialog->flags & DialogFlags::Disabled) || dialog->control_type != DialogControlType::Button) {
            palette_type = 4;
        }

        memcpy(g_palette_remap, g_new_cmdbtn_palette[palette_type].colors, 16);
        StatusScreenState* state = dialog->stat_button_user;
        GrpHead* grp = state->button_grp;
        u16 frame_index = state->grp_frame;
        if (frame_index >= grp->frame_count) { // Original ASM masks frame_count with 0x7FFF, but VS throws warnings with it.
            frame_index = 0;
        }

        GrpFrame* frame = &grp->frames[frame_index];
        void* image_data = (char*)grp + frame->data_offset; // I believe it's necessary for image_data to be assigned immediately after frame, to get the correct address offset for image_render_fxn_0_0 (including in the second hook).
        Box32 rect = {};
        SetRect((LPRECT)&rect, 0, 0, frame->w, frame->h);
        helper::image_render_fxn_0_0(
            dialog->bounds.left + (dialog->bounds.right - dialog->bounds.left - frame->w + 1) / 2,
            dialog->bounds.top + (dialog->bounds.bottom - dialog->bounds.top - frame->h + 1) / 2,
            &image_data - 1, &rect, 0);

        for (u32 i = 0; i < 16; ++i) {
            g_palette_remap[i] = i;
        }
    }

    // Hooked to explore colorizing passive icons. -Pr0nogo
    void statbtn_button_update(BinDlg* button) { // 00458900
        //u32 is_clicked = (button->flags & DialogFlags::DrawActiveState) != 0;
        //if (is_clicked >= g_cmdbtns_grp->frame_count) { // Original ASM masks frame_count with 0x7FFF, but VS throws warnings with it.
        //    is_clicked = 0; // Was LOWORD, presumably a flag negation
        //}
        auto draw_frame = 0;
        if ((button->flags & DialogFlags::DrawActiveState) && g_cmdbtns_grp->frame_count) {
            draw_frame = 1;
        }

        GrpFrame* frame = &g_cmdbtns_grp->frames[draw_frame];
        void* image_data = (char*)g_cmdbtns_grp + frame->data_offset;
        u32 bound_horizontal = button->bounds.right - button->bounds.left + 1;
        if (frame->w < bound_horizontal) {
            bound_horizontal = frame->w;
        }
        u32 bound_vertical = button->bounds.bottom - button->bounds.top + 1;
        if (frame->h < bound_vertical) {
            bound_vertical = frame->h;
        }

        Box32 rect = {};
        SetRect((LPRECT)&rect, 0, 0, bound_horizontal, bound_vertical);
        if (!g_is_in_replay) {
            helper::image_render_fxn_0_0(button->bounds.left, button->bounds.top, &image_data - 1, &rect, 0);
        }

        s32 palette_type = 0;
        if (button->flags & DialogFlags::Disabled) {
            palette_type = 1;
        } else if (button->flags & DialogFlags::DrawActiveState) {
            palette_type = 2;
        } else if (button->button_user->requirement_var == TechId::Healing
                && button->button_user->icon_id != IconId::Heal 
                && button->button_user->icon_id != IconId::Cleric) // Passive icons
        {
            palette_type = 6;
        } else {
            palette_type = button->graphic != g_highlighted_icon ? 0 : 2;
        }

        memcpy(g_palette_remap, g_new_cmdbtn_palette[palette_type].colors, 16);
        u16 button_graphic = button->graphic;
        if (button_graphic >= (g_cmdicons_grp)->frame_count) { // Original ASM masks frame_count with 0x7FFF, but VS throws warnings with it.
            button_graphic = 0;
        }

        frame = &(g_cmdicons_grp)->frames[button_graphic];
        image_data = (char*)g_cmdicons_grp + frame->data_offset;
        SetRect((LPRECT)&rect, 0, 0, frame->w, frame->h);
        s32 offset_x = 0;
        s32 offset_y = 0;
        if (g_is_in_replay) {
            switch (button_graphic) {
            case IconId::PlayReplay:
            case IconId::PauseReplay:
                offset_x = -2;
                break;
            case IconId::SpeedUpReplay:
                offset_x = -4;
                break;
            case IconId::SlowDownReplay:
                offset_x = -3;
            default:
                break;
            }
            offset_y = 3;
        }

        helper::image_render_fxn_0_0(
            draw_frame + offset_x + button->bounds.left + (button->bounds.right - button->bounds.left - frame->w + 1) / 2,
            draw_frame + offset_y + button->bounds.top + (button->bounds.bottom - button->bounds.top - frame->h + 1) / 2,
            &image_data - 1, &rect, 0);
        for (u32 i = 0; i < 16; ++i) {
            g_palette_remap[i] = i;
        }
    }

    // Drag scrolls camera relative to grab origin to avoid warping during drag.
    struct MiddleDrag {
        v2s32 mouse_origin;
        v2f32 camera;
    };
    static MiddleDrag g_middle_drag;

    char input_mmb_down(DialogEvent* event) {
        // Disallow dragging when F10 menu is open
        if (g_popup_dialog) {
            return 0;
        }

        // Ctrl+MMB resets zoom (view center held fixed, unlike the cursor-anchored wheel).
        const bool control_held = GetKeyState(VK_CONTROL) & (1 << 15);
        if (control_held && g_zoom.reset_to_default({ g_display.display_width() / 2, g_display.display_height() / 2 })) {
            return 0;
        }

        g_disable_mouse_scroll = true;

        g_middle_drag = MiddleDrag{ .mouse_origin = *g_mouse, .camera = { f32(g_move_to_x), f32(g_move_to_y) } };
        begin_middle_drag_cursor(g_middle_drag.mouse_origin);

        g_input_procs->funcs[3] = (InputProcsFunc*) 0x00484460;
        return 0;
    }

    // Idempotent middle-drag unwind. Popup dialog roots consume MButtonUp before it
    // reaches the input procs (game_menu_dialog_interact returns 1 for it), so every
    // path that ends a drag calls this instead of relying on the release dispatch.
    void end_middle_drag() {
        if (!g_middle_drag_cursor) {
            return;
        }

        g_disable_mouse_scroll = false;
        end_middle_drag_cursor();

        g_input_procs->funcs[3] = (InputProcsFunc*) 0;
    }

    int __cdecl input_mmb_up() {
        end_middle_drag();
        return 0;
    }

    constexpr s32 CameraPositionAlign = 8;

    static void scroll_screen(s32 dx, s32 dy) {
        // Keep camera positions 8-aligned — the invariant move_screen maintains
        // for every vanilla camera write. The campaign Center View glide
        // (0047EB70) stores an 8-aligned destination and arrives by exact
        // equality (0047F0E0); a 4-aligned position left by a drag makes the
        // glide land on destination ± 4 forever, freezing the camera and
        // pausing trigger execution indefinitely.
        g_move_to_x = align_down(std::clamp<s32>(s32(g_move_to_x) + dx, 0, g_display.scroll_max_x(g_map_width_pixels)), CameraPositionAlign);
        g_move_to_y = align_down(std::clamp<s32>(s32(g_move_to_y) + dy, 0, g_display.scroll_max_y(g_map_height_pixels)), CameraPositionAlign);

        g_move_to_tile_x = g_move_to_x >> 5;
        g_move_to_tile_y = g_move_to_y >> 5;

        helper::update_screen_position();
    }

    void __cdecl input_mmb_move() {
        const s32 scroll_max_x = g_display.scroll_max_x(g_map_width_pixels);
        const s32 scroll_max_y = g_display.scroll_max_y(g_map_height_pixels);

        const f32 gain = g_config.scroll_sensitivity * (g_config.use_pan_scrolling ? 1.0f : -1.0f);

        // The OS cursor is invisible and clipped to the window during the drag, so
        // travel would saturate at the window edge (center grab → only half a
        // window of scroll). Wrap it to the opposite edge when it pins against
        // one, shifting the grab origin by the same distance so the drag amount
        // stays continuous — scrolling is unbounded.
        constexpr s32 WrapMargin = 8;
        const s32 width = s32(g_display.surface_width());
        const s32 height = s32(g_display.surface_height());
        v2s32 mouse = *g_mouse;
        v2s32 shift = {};

        if (mouse.x < WrapMargin) {
            shift.x = width - 2 * WrapMargin - mouse.x;
            mouse.x += shift.x;
        } else if (mouse.x > width - 1 - WrapMargin) {
            shift.x = 2 * WrapMargin - mouse.x - 1;
            mouse.x += shift.x;
        }
        if (mouse.y < WrapMargin) {
            shift.y = height - 2 * WrapMargin - mouse.y;
            mouse.y += shift.y;
        } else if (mouse.y > height - 1 - WrapMargin) {
            shift.y = 2 * WrapMargin - mouse.y - 1;
            mouse.y += shift.y;
        }
        if (shift.x || shift.y) {
            g_middle_drag.mouse_origin += shift;
            *g_mouse = mouse;
            warp_cursor(mouse);
        }

        const v2s32 moved = mouse - g_middle_drag.mouse_origin;
        g_middle_drag.mouse_origin = mouse;

        g_middle_drag.camera.x = std::clamp(g_middle_drag.camera.x + gain * f32(moved.x) / f32(width) * f32(scroll_max_x), 0.0f, f32(scroll_max_x));
        g_middle_drag.camera.y = std::clamp(g_middle_drag.camera.y + gain * f32(moved.y) / f32(height) * f32(scroll_max_y), 0.0f, f32(scroll_max_y));

        scroll_screen(s32(std::lround(g_middle_drag.camera.x)) - g_move_to_x,
            s32(std::lround(g_middle_drag.camera.y)) - g_move_to_y);
    }

    // Hooked to disable functionality when using space for drag. -Pr0nogo
    bool statport_dlg_interact(BinDlg* dialog, DialogEvent* dlg_event) { // 0045F290
        bool result = 0;
        switch (dlg_event->number_order) {
            case DialogEventType_KeyDown:
            case DialogEventType_KeyRepeat:
            case DialogEventType_MouseMove: {
                result = 0;
            } break;
            case DialogEventType_Dialog: {
                switch (dlg_event->user) {
                    case 0: {
                        helper::statport_button_press(dialog, dialog);
                        goto Interact;
                    }
                    case 1: {
                        helper::statport_alloc();
                        result = helper::generic_dialog_interact(dialog, dlg_event);
                    } break;
                    case 4: {
                        result = 1;
                    } break;
                    default: {
                        goto Interact;
                    }
                }
            } break;
            case DialogEventType_Spacebar: {
            //	if (config.use_spacebar_for_scrolling) {
                    goto EarlyExit;
            //	}
                goto RespondToPortraitEvent;
            } break;
            case DialogEventType_MWheelDown: {
            //	if (!config.use_spacebar_for_scrolling) {
                RespondToPortraitEvent:
                    if (*g_dialog_key)
                    {
                        s8* key_pressed = *g_dialog_key;
                        s8 virtual_key = *(*g_dialog_key);
                        while (virtual_key != dlg_event->virtual_key) {
                            virtual_key = (key_pressed++)[1];
                            if (!virtual_key) {
                                goto EarlyExit;
                            }
                        }
                        v2u16 event_position(g_last_event_position->x, g_last_event_position->y);
                        helper::move_screen(event_position);
                        result = 1;
                    } else {
                    EarlyExit:
                        result = 0;
                    }
            //	}
            } break;
            default: {
            Interact:
                result = helper::generic_dialog_interact(dialog, dlg_event);
                break;
            }
        }
        return result;
    }

    // Hooked to prevent witness pings from showing for non-witnesses. -Pr0nogo
    char CMDRECV_minimap_ping(char* buffer) { // 004BF7A0
        u16 pos_x = *(u16*)(buffer + 1);
        if (pos_x < g_map_width_pixels) {
            u16 pos_y = *(u16*)(buffer + 3);
            if (pos_y < g_map_height_pixels) {
                *buffer = *g_active_nation_id;
                if (bw::player_is_witness(*g_active_nation_id) && !bw::player_is_witness(g_local_nation_id)
                || !bw::player_is_witness(*g_active_nation_id) &&  bw::player_is_witness(g_local_nation_id)) {
                    return *buffer;
                }

                if (bw::is_allied_to(*g_active_nation_id, g_local_nation_id)
                ||	bw::player_is_witness(*g_active_nation_id) && bw::player_is_witness(g_local_nation_id)) {
                    *buffer = helper::minimap_ping(pos_x, pos_y, (*g_active_nation_id != g_local_nation_id) + 17);
                }
            }
        }
        return *buffer;
    }

    // Hooked to add config option blocking the toggle. -Pr0nogo
    void __cdecl show_or_hide_minimap() { // 004A4150
        if (!g_minimap_draw_neutrals && g_hide_minimap_terrain && g_config.can_toggle_minimap_terrain) {
            Bitmap* current_surface = *g_current_surface;
            memset(g_minimap_surface_fog->buffer, 0, (s16)g_minimap_surface_fog->height * (s16)g_minimap_surface_fog->width);
            *g_current_surface = current_surface;
        } else {
            helper::dark_minimap_related();
            g_minimap_vision_callback();
        }
    }

    // Hooked to make stat_txt.tbl a .txt. -Pr0nogo
    u16 draw_game_menu_context(BinDlg* dialog) { // 004F4F70
        return helper::create_context_tooltip(dialog->bounds.right + dialog->parent->bounds.left,
            dialog->bounds.top + dialog->parent->bounds.top,
            (char*)g_stat_txt_buffer.get_string32(814)); // Game Menu (F10)
    }

    // Hooked to eventually turn stat_txt.tbl into a .txt. -Pr0nogo
    char* get_stat_txt_string(u16 string_id) { // 004C36F0
        return (char*)g_stat_txt_buffer.get_string32(string_id);
    }

    // Hooked to eventually turn stat_txt.tbl into a .txt. -Pr0nogo
    void draw_show_hide_terrain_context(BinDlg* dialog) { // 004A4C40
        if (!g_hide_minimap_terrain) {
            // string is 'Hide Terrain in Minimap'
            helper::create_context_tooltip(dialog->bounds.right + dialog->parent->bounds.left, dialog->bounds.top + dialog->parent->bounds.top, (char*)g_stat_txt_buffer.get_string32(811));
        } else {
            // string is 'Show Terrain in Minimap'
            helper::create_context_tooltip(dialog->bounds.right + dialog->parent->bounds.left, dialog->bounds.top + dialog->parent->bounds.top, (char*)g_stat_txt_buffer.get_string32(810));
        }
    }

    // Hooked to eventually turn stat_txt.tbl into a .txt. -Pr0nogo
    void draw_cancel_unit_context(BinDlg* dialog) { // 00457B10
        const auto* user = dialog->stat_button_user;
        if (user->type != 3) {
            helper::refresh_selection_screen();
            return;
        }
        Unit* active_unit = *g_active_portrait_unit;
        if ((unit_id_is_zerg_egg(active_unit->id)) && active_unit->is_frozen()) {
            helper::refresh_selection_screen();
            return;
        }

        const char* cancel_string = "(%s)";
        if (bw::in_replay_or_witness() || active_unit->player != g_local_nation_id) {
            switch (active_unit->get_race()) {
                case RaceId::Terran:  cancel_string = "Deploying (%s)"; break;
                case RaceId::Protoss: cancel_string = "Warping (%s)";   break;
                case RaceId::Zerg:    cancel_string = "Morphing (%s)";  break;
                case RaceId::Askosi:  cancel_string = "Raising (%s)";   break;
            }
        } else {
            switch (active_unit->get_race()) {
                case RaceId::Terran:  cancel_string = "Cancel Deployment (%s)"; break;
                case RaceId::Protoss: cancel_string = "Cancel Warp (%s)";       break;
                case RaceId::Zerg:    cancel_string = "Cancel Morph (%s)";      break;
                case RaceId::Askosi:  cancel_string = "Cancel Raising (%s)";    break;
            }
        }
        const char* name = helper::get_unit_name_stat_string(user->id);
        char dest[128];
        sprintf_s(dest, 128, cancel_string, name);
        s32 bounds_top = 0;
        s32 bounds_left = 0;
        if (dialog->control_type != DialogControlType::DialogBoxDC) {
            bounds_left = dialog->parent->bounds.left;
            bounds_top = dialog->parent->bounds.top;
        }
        helper::create_context_tooltip(bounds_left + dialog->bounds.right, bounds_top + dialog->bounds.top, dest);
    }

    // Hooked to eventually turn stat_txt.tbl into a .txt. -Pr0nogo
    void statdata_mouseover_interact(BinDlg* main_dialog, BinDlg* secondary_dialog) { // 00457CE0
        if (!*g_active_portrait_unit || !main_dialog || secondary_dialog == main_dialog) {
            helper::refresh_selection_screen();
            return;
        }

        u16 index = main_dialog->index;
        // Don't yet have an enum for index here.
        switch (index) {
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 17: {
                helper::draw_cancel_unit_context(main_dialog);
            } break;
            case 9:
            case 10:
            case 11:
            case 12: {
                bw::statdata_user_interact_internal(main_dialog);
            } break;
            case 15: {
                helper::draw_cancel_upgrade_context(main_dialog);
            } break;
            default: {
                if (index < 18 || index > 32) {
                    if (index < 33 || index > 44) {
                        helper::refresh_selection_screen();
                    } else {
                        helper::create_context_from_dialog(main_dialog, g_stat_txt_buffer.get_string32(809)); // Selection instructions
                    }
                } else {
                    helper::draw_unload_unit_context(main_dialog);
                }
            } break;
        }
    }

    // 004E6020: Ctrl+C and portrait-click centering. camera_for_center inverts
    // the edge-aware crop slide, so the unit lands mid-screen at every zoom level.
    void move_screen_to_unit(Unit* unit) { // 004E6020
        const v2s32 camera = g_zoom.camera_for_center({ unit->sprite->position.x, unit->sprite->position.y });
        helper::move_screen({ u16(std::max<s32>(camera.x, 0)), u16(std::max<s32>(camera.y, 0)) });
    }

    // Hooked to improve shift-tab functionality. -Pr0nogo
    s32 get_color_id(u8 player) { // 0049B0E0
        if (g_minimap_state != MinimapState::RecolorGame) {
            return player;
        }
        auto color_override = PlayerColor::Teal; // For own units.
        if (bw::in_replay_or_witness()) {
            switch (player) { // Use vanilla player color logic in replay/witness modes.
                case 0: color_override = PlayerColor::Red;    break;
                case 1: color_override = PlayerColor::Blue;   break;
                case 2: color_override = PlayerColor::Teal;   break;
                case 3: color_override = PlayerColor::Purple; break;
                case 4: color_override = PlayerColor::Orange; break;
                case 5: color_override = *g_current_tileset != TilesetType::Desert ? PlayerColor::Brown : PlayerColor::Green; break;
                case 6: color_override = *g_current_tileset != TilesetType::Ice    ? PlayerColor::White : PlayerColor::Green; break;
                case 7: color_override = PlayerColor::Yellow; break;
            }
        } else {
            if (!bw::is_allied_to(g_local_nation_id, player)) {
                color_override = PlayerColor::Red;
            } else if (g_local_nation_id != player) {
                color_override = player != 11 ? PlayerColor::Yellow : PlayerColor::RoyalBlue;
            }
        }

        auto final_color = static_cast<s32>(color_override);
        for (u8 player_it = 0; player_it < PlayerLimit; player_it++) {
            if (g_factions_colors_ordering[player_it] == final_color) {
                return player_it;
            }
        }
        return final_color;
    }

    // Hooked to improve shift-tab. -Pr0nogo
    void __fastcall draw_minimap_unit_box_2(u32 player) { // 004A47B0
        auto minimap_color = g_player_minimap_colors[player]; // Default
        if (g_minimap_state != MinimapState::Normal) {
            minimap_color = get_minimap_color(player);
        }
        Unit* first_unit = first_player_unit->unit[player];
        if (!first_unit) {
            return;
        }
        u32 checksum = g_minimap_box_checksum;
        bool reveal_all = false;
        for (Unit* first_unit = first_player_unit->unit[player]; first_unit; first_unit = first_unit->player_link.next) {
            if (!unit_id_ignored_on_minimap(first_unit->id)) {
                auto first_visibility = first_unit->sprite->visibility_status;
                if (first_unit->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) {
                    first_visibility &= first_unit->detection_status;
                }
                checksum ^= first_visibility;
                if (g_minimap_draw_neutrals != 1) {
                    if (g_is_in_replay) {
                        if (first_visibility & *g_replay_visions) {
                            helper::draw_unit_box(
                                first_unit->sprite->position.x, first_unit->sprite->position.y,
                                units_dat::building_dimensions[first_unit->id].x, units_dat::building_dimensions[first_unit->id].y,
                                minimap_color, units_dat::base_property[first_unit->id] & UnitProperty::Building);
                            continue;
                        }
                        reveal_all = g_replay_reveal_all == 0;
                    } else {
                        reveal_all = (first_visibility & *g_player_visions) == 0;
                    }
                    if (reveal_all) {
                        continue;
                    }
                    helper::draw_unit_box(
                        first_unit->sprite->position.x, first_unit->sprite->position.y,
                        units_dat::building_dimensions[first_unit->id].x, units_dat::building_dimensions[first_unit->id].y,
                        minimap_color, units_dat::base_property[first_unit->id] & UnitProperty::Building);
                }
            }
        }
        g_minimap_box_checksum = checksum;
    }

    bool __fastcall latency_dialog_proc(BinDlg* dialog, DialogEvent* dialog_event) { // 004e8ae0
        switch (dialog_event->number_order) {
            case DialogEventType::DialogEventType_Dialog: {
                switch (dialog_event->user) {
                    case DialogUserType_Create: {
                        helper::latency_dialog_create(dialog);
                    } break;
                    case DialogUserType_Destroy: {
                        helper::latency_dialog_destroy(dialog, dialog_event);
                        return true;
                    } break;
                    case DialogUserType_Activate: {
                        latency_dialog_activate(dialog, dialog_event);
                        return true;
                    } break;
                    case DialogUserType_InitChildren: {
                        std::array latency_dialog_functions{
                            &latency_interaction,
                            &latency_interaction,
                            &latency_interaction,
                            &latency_interaction,
                            &latency_interaction,
                        };
                        helper::register_user_dialog_action(dialog, latency_dialog_functions.data(), sizeof(latency_dialog_functions));
                    } break;
                }
            }
            case DialogEventType::DialogEventType_KeyDown:
            case DialogEventType::DialogEventType_KeyRepeat:
            case DialogEventType::DialogEventType_MouseMove:
            case DialogEventType::DialogEventType_LButtonDown:
            case DialogEventType::DialogEventType_LButtonUp:
            case DialogEventType::DialogEventType_LButtonDouble:
            case DialogEventType::DialogEventType_RButtonDown:
            case DialogEventType::DialogEventType_RButtonUp:
            case DialogEventType::DialogEventType_RButtonDouble:
            case DialogEventType::DialogEventType_Spacebar: {
                helper::generic_dialog_interact(dialog, dialog_event);
                return true;
            } break;
            default: {
                return helper::generic_dialog_interact(dialog, dialog_event);
            } break;
        }
    }

    void __fastcall latency_dialog_activate(BinDlg* dialog, DialogEvent* dialog_event) { // 004e8a30
        if (last_control_index == DialogButtonIndex::Ok) {
            for (auto control = dialog->child_dialog; control; control = control->next) {
                if (control->radioButton0x36.is_selected) {
                    SetLatencyCommand command;
                    command.value = control->index - 1;
                    helper::queue_game_command((u8*)&command, sizeof(command));
                }
            }
        }
        helper::save_options_menu_result(dialog);
        if (bin_interact == 1) {
            bin_interact = 0;
            helper::destroy_dialog(dialog);
            return;
        }
        bin_interact -= 2;
        last_bindlg_menu_function = helper::main_menu_options_custom_interact;
        helper::load_dialog("rez\\options.bin", helper::game_menu_dialog_interact);
    }

    std::string latency_value_string(u8 latency) {
        switch (latency) {
            case 0: return "None";
            case 1: return "Low";
            case 2: return "Medium";
            case 3: return "High";
            case 4: return "Extra-High";
        }
        return "";
    }

    b32 __fastcall latency_interaction(BinDlg* dialog, DialogEvent* event) {
        if (event->number_order == DialogEventType_Dialog) {
            switch (event->user) {
            case DialogUserType_InitChildren: {
                if (g_additional_turns_in_transit == dialog->index - 1) {
                    helper::latency_activate_radio(dialog);
                }
            } break;
            case DialogUserType_Activate:
                helper::latency_activate_radio(dialog);
                return true;
            case DialogUserType_SetSelectionIndex: {
                if (!event->param) return true;
                helper::latency_activate_radio(dialog);
                return true;
            } break;
            }
        }
        return helper::button_shared_interact(dialog, event);
    }

    void set_latency(u8 latency) {
        g_additional_turns_in_transit = std::min<u8>(latency, 4);

        auto name = g_is_in_replay ? g_replay_player_table[g_active_player_id].name : g_player_table[g_active_player_id].name;
        auto message = std::format("{} set latency compensation to {}", name, latency_value_string(latency));
        helper::latency_print_result(0, message.c_str());
    }

    void load_latency_dialog() {
        helper::load_dialog("rez\\netdlg.bin", &latency_dialog_proc);
    }

    bool is_valid_race(RaceId::Enum race) {
        switch (race) {
        case RaceId::Zerg: return true;
        case RaceId::Terran: return true;
        case RaceId::Protoss: return true;
        case RaceId::Askosi: return true;
        case RaceId::Random: return true;
        }
        return false;
    }

    static void combobox_add_race(BinDlg* dialog, RaceId::Enum selected_race, u8& selection_index, RaceId::Enum race, std::optional<u32> slot_id) {
        u8 index = 0;
        if (slot_id && race == RaceId::Random) {
            auto player_id = slot_id_to_index(slot_id.value());
            auto& race_randomizer = RaceRandomizer::instance();
            auto& race_string = race_randomizer.race_string(player_id);
            index = helper::combobox_add_item(dialog, BinDlgComboboxFlags::ProvideStrings, race_string.c_str());
        } else {
            index = helper::combobox_add_item(dialog, BinDlgComboboxFlags::ProvideStrings, race_to_table_string(race));
        }
        //auto index = helper::combobox_add_item(dialog, BinDlgComboboxFlags::ProvideStrings, race_to_table_string(race));
        dialog->combobox_output_array_u32[index] = race;
        if (race == selected_race) selection_index = index;
    }

    u8 load_races_dropdown(BinDlg* dialog, RaceId::Enum race, std::optional<u32>slot_id) {
        u8 selected_index = 0;
        combobox_add_race(dialog, race, selected_index, RaceId::Zerg, {});
        combobox_add_race(dialog, race, selected_index, RaceId::Terran, {});
        combobox_add_race(dialog, race, selected_index, RaceId::Protoss, {});
        combobox_add_race(dialog, race, selected_index, RaceId::Askosi, {});
        combobox_add_race(dialog, race, selected_index, RaceId::Random, slot_id);
        return selected_index;
    }

    void load_races_dropdown_singleplayer(BinDlg* dialog) {
        dialog->flags |= DialogFlags::PropagateUpdate;
        auto selected_index = load_races_dropdown(dialog, RaceId::Random);
        if (dialog->flags) {
            dialog->flags ^= DialogFlags::PropagateUpdate;
            helper::generic_combobox_update(dialog);
        }

        if (selected_index < dialog->ComboBox0x46.item_count) {
            DialogEvent event{};
            event.number_order = DialogEventType_Dialog;
            event.user = DialogUserType_SetSelectionIndex;
            event.param = selected_index;
            dialog->interaction_function(dialog, &event);
        }
    }
}