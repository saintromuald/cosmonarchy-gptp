#include "ui_features.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include "config.h"
#include "trigger_system/trigger_objectives.h"
#include "hooks/audiovisuals/sfxhooks.h"
#include "hooks/game_hooks.h"
#include "trigger_system/trigger_system.h"
#include <hooks\interface\minimap.h>

ApmCounter local_apm_counter;
ApmCounter player_apm_counter[PlayerLimit];

s32* const fonts = (s32*) 0x006CE0F4;
char* const printXYAlignment = (char*) 0x006CE110;
u32* const printXYPosY = (u32*) 0x006CE0E0;
Box16* const drawTextBox = (Box16*) 0x006CE0C8;

Point16 ui_text_pos;

CmdBtnPalette new_cmdbtn_palette[tIconCount];

bool disable_mouse_scroll;

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
	void draw_text(const char* text, u32 x, u32 y, u32 w, u32 h, char color) {
		helper::get_color_handle(color);
		*printXYAlignment = 0x11;
		drawTextBox->left = x;
		drawTextBox->right = x + w;
		drawTextBox->top = y;
		drawTextBox->bottom = y + h;
		helper::bwfxn_print_xy(text, x, y);
	}

	void draw_ui_text(const char* text, u8 color) {
		draw_text(text, ui_text_pos.x, ui_text_pos.y, 300, ui_text_height * 4, color);
		ui_text_pos.y = *printXYPosY;
	}

	void register_action(ApmCounter* counter, char* buffer) {
		switch (buffer[0]) {
			case 84: // Start game?
			case 55: // Create preplaced unit?
				return;
			case 99: // Custom
				if (buffer[1] == NewGameCommand::CameraSync) return;
		}
		if (counter->action_count < MaxActions) {
			counter->action_time[counter->action_count++] = *g_elapsed_time_frames;
		}
	}

	void init_actions() {
		memset(&local_apm_counter, 0, sizeof(local_apm_counter));
		memset(player_apm_counter, 0, sizeof(player_apm_counter));
	}

	void update_apm_counter(ApmCounter* counter) {
		for (u32 i = 0; i < counter->action_count; ++i) {
			s32 delta = *g_elapsed_time_frames - counter->action_time[i];
			if (delta >= ActionSampleTime * 24) {
				counter->action_time[i--] = counter->action_time[--counter->action_count];
			}
		}
	}

	void update_actions() {
		update_apm_counter(&local_apm_counter);
		for (u32 i = 0; i < PlayerLimit; ++i) {
			update_apm_counter(&player_apm_counter[i]);
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
		ui_text_pos = { 10, 3 };
		if (*leaderboard_type) {
			ui_text_pos.x += 80;
		}

		if (g_config.show_timer) {
			char buffer[48] = {};
			format_time(buffer, 48, "", *g_elapsed_time_frames / 24, false);
			draw_ui_text(buffer, 7);
		}

		bool is_replay_or_witness = *g_is_in_replay || g_globals->players[*g_local_nation_id].witness_mode;
		if (!is_replay_or_witness && g_config.show_apm) {
			char buffer[48] = {};
			u32 apm = local_apm_counter.action_count / (ActionSampleTime / 60.0);
			sprintf_s(buffer, "APM: %u", apm);
			draw_ui_text(buffer, 7);
		}

		for (const auto& [key, objective] : g_objectives) {
			if (objective.show) {
				char buffer[256] = {};
				char* pos = buffer;
				s32 size = 256;

				appendf(&pos, &size, "- ");
				if (objective.max_score == 1) {
					appendf(&pos, &size, "[%c] ", objective.score ? 'X' : ' ', objective.text.c_str());
				} else if (objective.max_score) {
					appendf(&pos, &size, "[%u/%u] ", objective.score, objective.max_score);
				}
				appendf(&pos, &size, "%s", objective.text.c_str());
				if (objective.timer) {
					char time[256] = {};
					format_time(time, 256, "", objective.timer / 24, true);
					appendf(&pos, &size, " (%s)", time);
				}
				draw_ui_text(buffer, 7);
			}
		}
	}

	void queue_game_command(u8* data, u32 size) {
		if (*g_command_queue_size + size <= *g_provider_max_buffer) {
			register_action(&local_apm_counter, (char*)data);
			memcpy((void*)&g_turn_buffer[*g_command_queue_size], data, size);
			*g_command_queue_size += size;
			return;
		}

		if (*sc_main_state == 4) {
			return;
		}

		u32 turns = 0;
		if (helper::snet_get_turns_in_transit(&turns)) {
			if (turns >= 16 - *g_provider_latency_calls) {
				return;
			}
			register_action(&local_apm_counter, (char*)data);
			helper::send_turn();
			memcpy((void*)&g_turn_buffer[*g_command_queue_size], data, size);
			*g_command_queue_size += size;
			return;
		}

		if (!*out_of_game) {
			u32 error = helper::serr_get_last_error();
			helper::packet_error_handle(error, 81, "Starcraft\\SWAR\\lang\\net_mgr.cpp", 225, 1);
		}
	}

	void targeted_order(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, u8 a7, s32 a8, s32 a9, s32 a10) {
		if (a6 && orders_dat::can_be_queued[(u8)a7]) {
			switch (a3->main_order_id) {
				case 2:
				case 3:
				case 23:
				case 93:
				case 152:
				case 175:
					goto LABEL_15;
				default:
					if (a3->order_queue_count < 8u) {
						if (*g_order_count < 1800) {
							helper::queue_new_order((Order*)a5, a1, a4, a3, a7, a8, a2, (Vector32*)a9, (Unit*)a10);
						} else {
							u8 v11 = a3->player;
							if (g_stat_txt_buffer->get_string_count() > 872) {
								helper::show_stat_txt_to_player((s32)g_stat_txt_buffer->get_string(873), v11);
							} else {
								helper::show_stat_txt_to_player((s32)"", v11);
							}
						}
					} else {
						u8 v10 = a3->player;
						if (g_stat_txt_buffer->get_string_count() > 870) {
							helper::show_stat_txt_to_player((s32)g_stat_txt_buffer->get_string(871), v10);
						} else {
							helper::show_stat_txt_to_player((s32)"", v10);
						}
					}
					break;
			}
		} else {
LABEL_15:
			helper::set_order_target(a7, a1, a4, a5, (s32)a3, (Unit*)a8, (u16)a2, (Vector32*)a9, (Unit*)a10);
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
		if (!(dialog->flags & BinDlgFlags::Visible) || dialog->flags & BinDlgFlags::Disabled) {
			return 0;
		}

		const char* label = helper::get_stat_txt_string(dialog->button_user->requirement_string_id);
		if (*label == VK_ESCAPE && hotkey == label[0]) { // escape handling for grid hotkeys
			return 1;
		}
		
		if (g_config.grid_hotkeys && dialog->index > 0) {
			u32 index = dialog->index - 1;
			char grid_hotkey = tolower(g_config.layout[index / 3].at(index % 3));
			return hotkey == grid_hotkey;
		}

		return hotkey == label[0];
	}

	char create_upgrade_level_tooltip(char* text) {
		if (!*g_upgrade_level_tooltip) {
			if (text) {
				return helper::create_tooltip(text);
			}
			return helper::refresh_selection_screen();
		}
		if (!text) {
			return helper::refresh_selection_screen();
		}
		const char* description;
		if (g_stat_txt_buffer->get_string_count() > 794) {
			description = (char*)g_stat_txt_buffer->get_string(795);
		} else {
			description = "";
		}
		char buffer[128];
		sprintf_s(buffer, 128, "%s\n%s %d", text, description, (s16)*g_upgrade_level_tooltip);
		bw::printf("%s", buffer);
		return helper::create_tooltip(buffer);
	}

	// Hooked to improve functionality and to use in our own dialogue system. -Pr0nogo
	s32 trigact_transmission(TriggerAction* trigger, char* a2) // 004C7890
	{
		TriggerAction* trigger_var = trigger;
		if (!(LOBYTE(trigger->source_location))) // Based on this, IDA's struct is just wrong, and we have a second field here. Guessing lobyte = location id based on context. -Pr0nogo
		{
			return 1;
		}
		if ((LOBYTE(trigger->source_location)) > 0xFFu) {
			return 1;
		}
		u32 player_id = *g_current_trigger_player_id;
		u8 player_type = g_player_table[player_id].type;
		if (player_type == PlayerType::Computer
		||  player_type == PlayerType::Rescuable
		||  player_type == PlayerType::UnusedRescueActive
		||  player_type == PlayerType::Neutral)
		{
			return 1;
		}
		if (!g_is_executing_wait[*g_current_trigger_player_id]) {
			char trigger_flags = trigger->flags;
			if (trigger_flags & 1) {
				trigger->flags = trigger_flags & 0xFE;
				return 1;
			}
			if (g_current_trigger_node->trigger.flags & 0x10) {
				return 1;
			}
			u32 trigger_modifier = trigger->modifier - 7;
			u32 trigger_time = trigger_var->time;
			if (trigger_var->modifier == 7) {
				trigger_time = trigger_var->extra;
			} else {
				u32 trigger_modifier_2 = trigger_modifier - 1;
				if (trigger_modifier_2) {
					if (trigger_modifier_2 == 1) {
						u32 trigger_extra = trigger_var->extra;
						if (trigger_extra <= trigger_time) {
							trigger_time -= trigger_extra;
						} else {
							trigger_time = 0;
						}
					}
				} else {
					trigger_time += trigger_var->extra;
				}
			}
			trigger_var->flags = trigger_flags | 1;
			u32 multiplayer_time = *g_multiplayer_mode ? trigger_time : -1;
			g_trigger_wait[player_id] = multiplayer_time;
			g_is_executing_wait[player_id] = 1;
			if (player_id == *g_local_nation_id) {
				helper::trigact_play_wav(trigger_var, a2);
				Unit* doodad = helper::get_doodad_for_unit_state(LOBYTE(trigger_var->source_location) - 1, trigger_var->type, 17);
				Unit* doodad_var = doodad; // Probably not necessary. -Pr0nogo
				if (doodad) {
					doodad->sprite->selection_timer = 45;
					helper::minimap_ping(doodad->sprite->position.x, doodad->sprite->position.y, 17);
					helper::game_talking_portrait(doodad_var->sprite->position.x, trigger_time, trigger_var->type, doodad_var->sprite->position.y);
				} else {
					helper::game_talking_portrait(-1, trigger_time, trigger_var->type, -1);
				}
				if (g_options_list->flags & 0x400 || trigger_var->flags & 4) {
					char* string = helper::get_chk_string(trigger_var->text_string);
					if (trigger_time > helper::get_text_display_time(string)) {
						helper::create_text_message_with_timer(string, trigger_time);
						return 0;
					}
					u32 display_time = helper::get_text_display_time(string);
					helper::create_text_message_with_timer(string, display_time);
				}
			}
		}
		return 0;
	}

	// Hooked to add features and color Vespene Ridges cyan. -Pr0nogo
	void draw_minimap_unit_box(u32 player) { // 004A4650
		u32 unit_id = UnitId::None;
		char color_mod = 0;
		char index_ref = 0;
		if (*g_minimap_state) {
			color_mod = g_palette_index_reference[20];
			if (bw::is_allied_to(*g_local_nation_id, player)) {
				index_ref = g_palette_index_reference[17];
			} else {
				index_ref = color_mod;
			}
		} else {
			color_mod = g_player_minimap_colors[player];
			index_ref = color_mod;
		}
		Unit* unit = first_player_unit->unit[player];
		if (unit) {
			// Required for goto to compile in C++20 mode :( -Veeq7
			u32 visibility;
			u32 unit_status;
			u32 vision;

			u32 checksum = *g_minimap_box_checksum;
			while (true) {
				if (unit_id_ignored_on_minimap(unit->id)) {
					goto next;
				}

				visibility = unit->sprite->visibility_status;
				unit_status = unit->status;
				if (unit_status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) {
					visibility &= unit->detection_status;
				}
				checksum ^= visibility;
				if (*g_minimap_draw_neutrals) {
					break;
				}

				vision = 0;
				if (*g_is_in_replay) {
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
					*g_minimap_box_checksum = checksum;
					return;
				}
			}
			u32 color = g_palette_index_reference[25];
			if (units_dat::base_property[unit_id] & UnitProperty::ResourceContainer) {
				color = index_ref;
			}

			Point16 pos = unit->sprite->position;
			Point16 dims = units_dat::building_dimensions[unit->id];
			helper::draw_unit_box(pos.x, pos.y, dims.x, dims.y, color, units_dat::base_property[unit->id] & UnitProperty::Building);
			goto next;
		}
	}

	// Mirrored from IDA to draw camera positions. -Pr0nogo
	static void draw_camera_box(u32 x, u32 y, u8 color) { // based on 004A3EE0
		Box32 box = {};
		u32 zoom_level = *g_minimap_zoom_level;
		box.x = x / zoom_level;
		box.y = y / zoom_level;
		box.w = box_width;
		box.h = box_height;

		box.x = Clamp(box.x, 0, *g_map_width);
		box.y = Clamp(box.y, 0, *g_map_height);

		if (box.x + box.w >= *g_map_width) {
			box.w = *g_map_width - box.x;
		}

		if (box.y + box.h >= *g_map_height) {
			box.h = *g_map_height - box.y;
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
		u32 player = *g_local_nation_id;
		*g_current_surface = g_minimap_surface_2;
		*g_minimap_unit_box_count = 0;
		*g_minimap_box_checksum = 0;

		u32 index = 12;
		while (index) {
			if (--index < PlayerLimit) {
				if (player != index || *g_is_in_replay) {
					helper::draw_minimap_unit_box_2(index);
				}
			} else {
				helper::draw_minimap_unit_box(index);
			}
		}
		if (!*g_is_in_replay) {
			helper::draw_all_minimap_unit_boxes(player);
		}
		
		for (Thingy* thingy = *g_first_fow_sprite; thingy; thingy = thingy->next) {
			if (thingy->unit_id != UnitId::Spell_CarapaceSwarm) {
				bool not_visible = helper::thingy_visiblity_check(1, thingy) && !unit_id_is_resource_node(thingy->unit_id, true);
				if (!*g_is_in_replay || !not_visible) {
					Sprite* sprite = thingy->sprite;

					char color = g_palette_index_reference[25];
					if (sprite->id != SpriteId::VespeneRidge
					&&	sprite->id != SpriteId::VespeneGeyser
					&&  sprite->id != SpriteId::MineralFieldType1
					&&  sprite->id != SpriteId::MineralFieldType2
					&&  sprite->id != SpriteId::MineralFieldType3) {
						if (*g_minimap_state) {
							if (bw::is_allied_to(player, sprite->player)) {
								color = g_palette_index_reference[17];
							} else {
								color = g_palette_index_reference[20];
							}
						} else {
							color = g_player_minimap_colors[sprite->player];
						}
					}

					Point16 pos = sprite->position;
					/*
					if (sprite->player == 11) {
						ActiveTile* tile = bw::get_active_tile_at_coords(pos);
						if (tile->explored_flags & 1 << *g_local_nation_id) {
							color = dark_pcx_image_map->index[28][color];
						} else {
							color = dark_pcx_image_map->index[32][color];
						}
					}
					*/
					Point16 dims = units_dat::building_dimensions[thingy->unit_id];
					helper::draw_unit_box(pos.x, pos.y, dims.x, dims.y, color, units_dat::base_property[thingy->unit_id] & UnitProperty::Building);
					--*g_minimap_unit_box_count;
				}
			}
		}

		if (g_config.show_minimap_cameras) {
			for (u8 player = 0; player < PlayerLimit; ++player) {
				if (g_player_table[player].type != PlayerType::Human || g_globals->players[player].witness_mode) {
					continue;
				}
				if (!*g_is_in_replay && player == *g_local_nation_id) {
					continue;
				}
				if (!*g_is_in_replay && !g_globals->players[*g_local_nation_id].witness_mode && !bw::is_allied_to(player, *g_local_nation_id)) {
					continue;
				}
				Point16 camera = g_globals->players[player].camera;
				if (camera.x || camera.y) {
					draw_camera_box(camera.x, camera.y, g_player_minimap_colors[player]);
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
		if (transport_ui_unit) {
			if (transport_ui_unit->player == 11) {
				return CursorType_HoveringNeutral;
			}
			if (helper::unit_is_enemy(transport_ui_unit)) {
				return CursorType_HoveringEnemy;
			}
			return CursorType_HoveringAlly;
		}
		if (PtInRect((const RECT*)&g_minimap_rect, *(POINT*)g_mouse)) {
			return (u16)*g_minimap_cursor;
		}
		if (helper::is_outside_of_game_screen(g_mouse->x, g_mouse->y) || !PtInRect(g_cursor_limit_rect, *(POINT*)g_mouse)) {
			return CursorType_Normal;
		}
		if (*g_is_drag_selecting) {
			return CursorType_DragSelect;
		}
		if (*g_is_placing_building) {
			return CursorType_Normal;
		}
		Unit* unit_at_point = helper::find_unit_at_point(g_mouse->x + *g_move_to_x, *g_move_to_y + g_mouse->y);
		if (*g_is_targeting) {
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
		if (*g_active_portrait_unit && (*g_active_portrait_unit)->player == *g_local_nation_id) {
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

	// Hooked to investigate crashes on Fraudscendance. -Pr0nogo
	void draw_vertical_line(u16 size, u16 x, u16 y) { // 0041D7D0
		Bitmap* surface = *g_current_surface;
		for (u32 i = 0; i < size; ++i) {
			surface->buffer[y * surface->width + x] = *g_draw_color_index;
			y += 1;
		}
	}

	// Hooked to improve functionality. -Veeq7
	void __fastcall centerview_unit_group(u8 slot) { // 004967E0
		// Handled in hotkeys.cpp now
	}

	// Hooked to explore colorizing passive icons. -Pr0nogo
	void draw_standard_button_graphic(BinDlg* button, BinDlg* button_, BinDlg* dest) { // 004EFD90
		BinDlg* parent = button->parent;
		BinDlg* button_unk = parent->initialVersion0x36.unk_3e;
		u32 final_flags = button->flags;
		if (button->flags & BinDlgFlags::Disabled
		|| !(button->flags & BinDlgFlags::Visible)
		||  button != button_unk && (button->control_type != 1
		||  button_unk
		||  parent->flags & BinDlgFlags::UseAlternateStyle)) {
			final_flags &= ~BinDlgFlags::Unknown12;
		} else {
			final_flags |= BinDlgFlags::Unknown12;
		}
		button->flags = final_flags;
		if (!(final_flags & BinDlgFlags::RemoveStyles)) {
			u8 graphic = 0;
			if (final_flags & BinDlgFlags::Disabled) {
				graphic = button->graphic;
			} else {
				if (final_flags & BinDlgFlags::NoClickSound) {
					u32 VEEQTISM = (((u8)button->graphic + 2) & 0xFF) | (u32)button & 0xFF00;
					final_flags = helper::draw_clamped_dialog_image(dest, VEEQTISM, button->bounds.x, button->bounds.y, final_flags);
					goto exit;
				}
				graphic = button->graphic + 1;
			}
			final_flags = helper::draw_clamped_dialog_image(dest, graphic, button->bounds.x, button->bounds.y, final_flags);
		}
	exit:
		u32 x_min = button->bounds.x;
		u32 x_max = button->bounds.w;
		u32 y_min = button->bounds.y;
		u32 y_max = button->bounds.h;

		if (button->control_type == 1) {
			*g_draw_color_index = g_palette_index_reference[5];
			helper::bw_draw(y_min, x_min, x_max - x_min + 1, y_max - y_min + 1);
		}
		if (button->flags & BinDlgFlags::Unknown12) {
			*g_draw_color_index = g_palette_index_reference[16];
			helper::bw_draw(y_min, x_min, x_max - x_min + 1, y_max - y_min + 1);
		}
		if (button->text) {
			bool highlight = button->flags & BinDlgFlags::HighlightText;
			*g_print_xy_alignment = 34;
			helper::dialog_draw_text(0, button, highlight, highlight);
		}
	}

	// Hooked to explore colorizing passive icons. -Pr0nogo
	b32 generic_dialog_update(BinDlg* button, u32 width, u32 height, Box16* dims) { // 004EF590
		if (button->buffer) {
			return helper::draw_dialog_image(dims, &button->bounds.width, width, height, 0);
		}
		*g_draw_color_index = (button->flags & BinDlgFlags::TransparentHideColor) == 0 ? g_palette_index_reference[12] : 0;
		return helper::draw_box(dims->x, dims->y, dims->w - dims->x + 1, dims->h - dims->y + 1);
	}

	// Hooked to explore colorizing passive icons. -Pr0nogo
	void draw_stat_button_image(BinDlg* dialog) { // 00456C30
		u8 palette_type = 3;
		if (dialog->flags & BinDlgFlags::Disabled || dialog->control_type != DialogControlTypes::Button) {
			palette_type = 4;
		}

		memcpy(g_palette_remap, new_cmdbtn_palette[palette_type].colors, 16);
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
		u32 is_clicked = (button->flags & BinDlgFlags::NoClickSound) != 0;
		if (is_clicked >= (*g_cmdbtns_handle)->frame_count) { // Original ASM masks frame_count with 0x7FFF, but VS throws warnings with it.
			is_clicked = 0; // Was LOWORD, presumably a flag negation
		}

		GrpFrame* frame = &(*g_cmdbtns_handle)->frames[is_clicked];
		void* image_data = (char*)*g_cmdbtns_handle + frame->data_offset;
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
		if (!*g_is_in_replay) {
			helper::image_render_fxn_0_0(button->bounds.left, button->bounds.top, &image_data - 1, &rect, 0);
		}

		u32 flags = button->flags;
		s32 palette_type = 0;
		if (flags & BinDlgFlags::Disabled) {
			palette_type = 1;
		} else if (flags & BinDlgFlags::NoClickSound) {
			palette_type = 2;
		} else if (button->button_user->requirement_var == TechId::Healing
				&& button->button_user->icon_id != IconId::Heal 
				&& button->button_user->icon_id != IconId::Medic) // Passive icons
		{
			palette_type = 6;
		} else {
			palette_type = button->graphic != *g_highlighted_icon ? 0 : 2;
		}

		memcpy(g_palette_remap, new_cmdbtn_palette[palette_type].colors, 16);
		u16 button_graphic = button->graphic;
		if (button_graphic >= (*g_cmdicons_grp)->frame_count) { // Original ASM masks frame_count with 0x7FFF, but VS throws warnings with it.
			button_graphic = 0;
		}

		frame = &(*g_cmdicons_grp)->frames[button_graphic];
		image_data = (char*)*g_cmdicons_grp + frame->data_offset;
		SetRect((LPRECT)&rect, 0, 0, frame->w, frame->h);
		s32 offset_x = 0;
		s32 offset_y = 0;
		if (*g_is_in_replay) {
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
			is_clicked + offset_x + button->bounds.left + (button->bounds.right - button->bounds.left - frame->w + 1) / 2,
			is_clicked + offset_y + button->bounds.top + (button->bounds.bottom - button->bounds.top - frame->h + 1) / 2,
			&image_data - 1, &rect, 0);
		for (u32 i = 0; i < 16; ++i) {
			g_palette_remap[i] = i;
		}
	}

	// Hooked to extend g_cmdbtn_palette. -Pr0nogo
	void load_statdata_bin() // 00459B90
	{
		*g_active_portrait_buttonset_id = 228;
		*g_buttonset_special_buttonset_id = 228;
		*g_current_buttonset_id = 228;
		u32 console_offset = aztp[*g_console_index];
		*g_highlighted_icon = -1;

		char filename;
		sprintf_s(&filename, 260, "%s%ccmdbtns.grp", "unit\\cmdbtns\\", console_offset);
		*g_cmdbtns_handle = (GrpHead*)helper::load_graphic(&filename, 0, 260, "Starcraft\\SWAR\\lang\\statcmd.cpp", 1086);
		*g_cmdicons_grp = (GrpHead*)helper::load_graphic("unit\\cmdbtns\\cmdicons.grp", 0, 260, "Starcraft\\SWAR\\lang\\statcmd.cpp", 1089);
		if (!helper::sbmp_load_image("unit\\cmdbtns\\ticon.pcx", 0, "", 96, 0, 0, 0))
		{
			u32 error = helper::serr_get_last_error();
			helper::syswarn_file_not_found(error, "unit\\cmdbtns\\ticon.pcx");
		}

		char n_u;
		if (*g_is_in_replay)
		{
			n_u = 'n';
		} else {
			n_u = aztp[*g_console_index];
		}

		sprintf_s(&filename, 260, "rez\\statbtn%c.bin", n_u);
		HANDLE* statdata_bin;
		if (!helper::sfile_open_file_ex(0, &filename, 0, &statdata_bin))
		{
			u32 error_2 = helper::serr_get_last_error();
			helper::syswarn_file_not_found(error_2, &filename);
		}

		u32 size = helper::SFileGetFileSize(statdata_bin, 0);
		if (size == -1)
		{
			u32 error_3 = GetLastError();
			helper::file_fatal(statdata_bin, error_3);
		}
		if (!size)
		{
			helper::syswarn_file_not_found(0x18u, &filename);
		}

		BinDlg* dialog = (BinDlg*)helper::SMemAlloc(size, "Starcraft\\SWAR\\lang\\gamedata.cpp", 210, 0);
		u32 read;
		if (!helper::sfile_read_file(statdata_bin, dialog, size, &read, 0)) // LPDWORD is supposed to be address of so it assigns value to it...
		{
			if (GetLastError() == 38)
			{
				helper::file_fatal(statdata_bin, 0x18u);
			}
			u32 error_4 = GetLastError();
			helper::file_fatal(statdata_bin, error_4);
		}
		if (read != size)
		{
			helper::file_fatal(statdata_bin, 0x18u);
		}

		helper::SFileCloseFile(statdata_bin);
		BinDlg* final_dialog = 0;
		if (dialog)
		{
			dialog->flags |= 4u;
			helper::alloc_init_dialog_data(dialog, (void(__fastcall*)(u32, u32, u32, u32, u32))helper::alloc_background_image, "Starcraft\\SWAR\\lang\\statcmd.cpp", 1096);
			final_dialog = dialog;
		} else {
			final_dialog = 0;
		}
		*g_buttonset_dialog = final_dialog;
		helper::initialize_dialog((u32(__fastcall*)(BinDlg*, DialogEvent*))helper::statbtn_dialog_interact, final_dialog);
	}/*

	void print_stat_res_info(BinDlg* dialog, u16 graphic, u8** recent_spending, u8* resource_amount, b32 instant_update) // 004E52E0
	{
		helper::show_dialog(dialog);
		s32 new_amount = resource_amount - *recent_spending;
		if ((!dialog->text && new_amount) || new_amount) {
			if (instant_update) {
				*recent_spending = resource_amount;
			} else {
				*recent_spending += (2 * (new_amount > 0) - 1) + (new_amount >> 4);
			}
		}

		if ((u8*)dialog->user != resource_amount || dialog->graphic != graphic) {
			dialog->graphic = graphic;
			dialog->user = (u32*)*recent_spending;
			sprintf_s(g_countdown_timer_strings[32 * dialog->index + 4], 32, "%d", *recent_spending);
			dialog->text = g_countdown_timer_strings[32 * dialog->index + 4];

			if (!(dialog->flags & BinDlgFlags::Highlighted)) {
				dialog->flags != BinDlgFlags::Highlighted;
				helper::update_dialog(dialog);
			}
		}
	}*/

	char input_mmb_down(DialogEvent* event) {
		disable_mouse_scroll = true;
		POINT mouse = {};
		GetCursorPos(&mouse);
		*g_mmb_pos_x = mouse.x;
		*g_mmb_pos_y = mouse.y;
		g_input_procs->funcs[3] = (InputProcsFunc*) 0x00484460;
		return helper::refresh_cursor_0();
	}

	int __cdecl input_mmb_up() {
		disable_mouse_scroll = false;
		*g_move_to_x = Clamp(round(*g_move_to_x / 4.0) * 4, 0, *g_map_width_pixels  - 640);
		*g_move_to_y = Clamp(round(*g_move_to_y / 4.0) * 4, 0, *g_map_height_pixels - 376);
		g_input_procs->funcs[3] = (InputProcsFunc*) 0;
		return helper::cursor_refresh();
	}

	static void scroll_screen(s32 dx, s32 dy) {
		*g_move_to_x = Clamp((s32)*g_move_to_x + dx, 0, *g_map_width_pixels  - 640);
		*g_move_to_y = Clamp((s32)*g_move_to_y + dy, 0, *g_map_height_pixels - 376);

		*g_move_to_tile_x = *g_move_to_x >> 5;
		*g_move_to_tile_y = *g_move_to_y >> 5;

		helper::update_screen_position();
	}

	static Vector32f mouse_error;

	void __cdecl input_mmb_move() {
		POINT mouse = {};
		GetCursorPos(&mouse);
		s32 dx = (s32)mouse.x - (s32)*g_mmb_pos_x;
		s32 dy = (s32)mouse.y - (s32)*g_mmb_pos_y;

		f32 sens = g_config.scroll_sensitivity;
		f32 sdx = dx * sens + mouse_error.x;
		f32 sdy = dy * sens + mouse_error.y;
		if (g_config.use_pan_scrolling) {
			scroll_screen(sdx, sdy);
		} else {
			scroll_screen(-sdx, -sdy);
		}
		mouse_error.x = sdx - (s32)sdx;
		mouse_error.y = sdy - (s32)sdy;

		if (dx || dy) {
			SetCursorPos(*g_mmb_pos_x, *g_mmb_pos_y);
		}
	}

	// Hooked to disable functionality when using space for drag. -Pr0nogo
	bool statport_dlg_interact(BinDlg* dialog, DialogEvent* dlg_event) // 0045F290
	{
		bool result = 0;
		switch (dlg_event->number_order)
		{
			case DialogEventType_KeyDown:
			case DialogEventType_KeyRepeat:
			case DialogEventType_MouseMove:
			{
				result = 0;
			} break;
			case DialogEventType_Dialog:
			{
				switch (dlg_event->user)
				{
					case 0:
					{
						helper::statport_button_press(dialog, dialog);
						goto LABEL_10;
					}
					case 1:
					{
						helper::statport_alloc();
						result = helper::generic_dialog_interact(dialog, dlg_event);
					} break;
					case 4:
					{
						result = 1;
					} break;
					default:
					{
						goto LABEL_10;
					}
				}
			} break;
			case DialogEventType_Spacebar:
			{
			//	if (config.use_spacebar_for_scrolling) {
				{
					goto LABEL_15;
				}
				goto RespondToPortraitEvent;
			} break;
			case DialogEventType_MWheelDown:
			{
			//	if (!config.use_spacebar_for_scrolling) {
				{
				RespondToPortraitEvent:
					if (*g_dialog_key)
					{
						s8* key_pressed = *g_dialog_key;
						s8 virtual_key = *(*g_dialog_key);
						while (virtual_key != dlg_event->virtual_key)
						{
							virtual_key = (key_pressed++)[1];
							if (!virtual_key)
							{
								goto LABEL_15;
							}
						}
						Point16 event_position = { g_last_event_position->x, g_last_event_position->y };
						helper::move_screen(event_position);
						result = 1;
					}
					else {
					LABEL_15:
						result = 0;
					}
				}
			} break;
			default:
			{
			LABEL_10:
				result = helper::generic_dialog_interact(dialog, dlg_event);
				break;
			}
		}
		return result;
	}

	// Hooked to prevent witness pings from showing for non-witnesses. -Pr0nogo
	char CMDRECV_minimap_ping(char* buffer) // 004BF7A0
	{
		u16 pos_x = *(u16*)(buffer + 1);
		if (pos_x < *g_map_width_pixels)
		{
			u16 pos_y = *(u16*)(buffer + 3);
			if (pos_y < *g_map_height_pixels)
			{
				*buffer = *g_active_nation_id;
				if (bw::player_is_witness(*g_active_nation_id) && !bw::player_is_witness(*g_local_nation_id)
				|| !bw::player_is_witness(*g_active_nation_id) &&  bw::player_is_witness(*g_local_nation_id)) {
					return *buffer;
				}

				if (bw::is_allied_to(*g_active_nation_id, *g_local_nation_id)
				||	bw::player_is_witness(*g_active_nation_id) && bw::player_is_witness(*g_local_nation_id)) {
					*buffer = helper::minimap_ping(pos_x, pos_y, (*g_active_nation_id != *g_local_nation_id) + 17);
				}
			}
		}
		return *buffer;
	}

	// Hooked to add config option blocking the toggle. -Pr0nogo
	void __cdecl show_or_hide_minimap() // 004A4150
	{
		if (*g_minimap_draw_neutrals != 1 && *g_hide_minimap_terrain && g_config.can_toggle_minimap_terrain) {
			Bitmap* current_surface = *g_current_surface;
			memset(g_minimap_surface_2->buffer, 0, (s16)g_minimap_surface_2->height * (s16)g_minimap_surface_2->width);
			*g_current_surface = current_surface;
		} else {
			helper::dark_minimap_related();
			*g_minimap_vision_CB;
		}
	}
}