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

#define ActionSampleTime 10
#define MaxActions 2000
struct ClientAction {
	u32 time;
};
u32 total_client_actions;
u32 client_actions_count;
ClientAction client_actions[MaxActions];

s32* const fonts = (s32*)0x006CE0F4;
char* const printXYAlignment = (char*)0x006CE110;
u32* const printXYPosY = (u32*)0x006CE0E0;
Box16* const drawTextBox = (Box16*)0x006CE0C8;

Point16 ui_text_pos;

void format_time(char* buffer, u32 size, char* prefix, u32 seconds) {
	u32 sec = seconds % 60;
	u32 min = seconds / 60 % 60;
	u32 hrs = seconds / 60 / 60;

	sprintf_s(buffer, size, "%s%02u:%02u:%02u", prefix, hrs, min, sec);
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
		//ui_text_pos.y += ui_text_height;
		ui_text_pos.y = *printXYPosY;
	}

	void register_action(u8 type) {
		switch (type) {
			case 84: // Start game?
			case 55: // Create preplaced unit?
				return;
		}
		total_client_actions++;
		client_actions[client_actions_count++].time = *g_elapsed_time_frames;
	}

	void init_actions() {
		total_client_actions = 0;
		client_actions_count = 0;
		memset(client_actions, 0, sizeof(client_actions));
	}

	void update_actions() {
		for (u32 i = 0; i < client_actions_count; ++i) {
			s32 delta = *g_elapsed_time_frames - client_actions[i].time;
			if (delta >= ActionSampleTime * 24) {
				client_actions[i--] = client_actions[--client_actions_count];
			}
		}
	}

	void draw_ui_features() {
		ui_text_pos = {10, 3};
		if (*leaderboard_type) {
			ui_text_pos.x += 80;
		}

		if (config.show_timer) {
			char buffer[32] = {};
			format_time(buffer, 32, "", *g_elapsed_time_frames * 0.042);
			draw_ui_text(buffer, 7);
		}

		if (config.show_apm) {
			char buffer[32] = {};

			u32 apm = client_actions_count / (ActionSampleTime / 60.0);
			sprintf_s(buffer, "APM: %u", apm);
			draw_ui_text(buffer, 7);

			//u32 total_apm = total_client_actions / (*g_elapsed_time_frames / 24.0 / 60.0);
			//sprintf_s(buffer, "AVG APM: %u", total_apm);
			//draw_ui_text(buffer, 7);
		}

		for (u32 i = 0; i < objectives_count; ++i) {
			Objective* objective = &objectives[i];
			if (objective->show) {
				char buffer[256] = {};
				if (objective->max_score == 1) {
					snprintf(buffer, 256, "- [%c] %s", objective->score ? 'X' : ' ', objective->text);
				} else if (objective->max_score) {
					snprintf(buffer, 256, "- [%u/%u] %s", objective->score, objective->max_score, objective->text);
				} else {
					snprintf(buffer, 256, "- %s", objective->text);
				}

				draw_ui_text(buffer, 7);
			}
		}
	}

	void queue_game_command(u8* data, u32 size) {
		if (*g_command_queue_size + size <= *g_provider_max_buffer) {
			register_action(data[0]);
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
			register_action(data[0]);
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
							if (*g_stat_txt_tbl > 0x368u) {
								helper::show_stat_txt_to_player(*g_stat_txt_tbl + g_stat_txt_tbl[873], v11);
							} else {
								helper::show_stat_txt_to_player((s32)"", v11);
							}
						}
					} else {
						u8 v10 = a3->player;
						if (*g_stat_txt_tbl > 0x366u) {
							helper::show_stat_txt_to_player(*g_stat_txt_tbl + g_stat_txt_tbl[871], v10);
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

		char* label = helper::get_stat_txt_string(dialog->button_user->requirement_string_id);
		if (*label == 27 && hotkey == label[0]) {  // escape handling for grid hotkeys
			return 1;
		}

		if (config.grid_hotkeys && dialog->index > 0) {
			int index = dialog->index - 1;
			int x = index % 3;
			int y = index / 3;
			return hotkey == tolower(config.layout[y].at(x));
		}

		return hotkey == label[0];
	}

	char create_upgrade_level_tooltip(char* text) {
		if (!*g_upgrade_level_tooltip) {
			if (text)
			{
				return helper::create_tooltip(text);
			}
			return helper::refresh_selection_screen();
		}
		if (!text) {
			return helper::refresh_selection_screen();
		}
		const char* description;
		if (**g_stat_txt_buffer > 794u) {
			description = (char *)*g_stat_txt_buffer + (*g_stat_txt_buffer)[795];
		} else {
			description = "";
		}
		char buffer[128];
		sprintf_s(buffer, 128, "%s\n%s %d", text, description, (s16)*g_upgrade_level_tooltip);
		scbw::printf("%s", buffer);
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
		if ((LOBYTE(trigger->source_location)) > 0xFFu)
		{
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
		if (!g_is_executing_wait[*g_current_trigger_player_id])
		{
			char trigger_flags = trigger->flags;
			if (trigger_flags & 1)
			{
				trigger->flags = trigger_flags & 0xFE;
				return 1;
			}
			if ((*g_current_trigger_node)->trigger.flags & 0x10)
			{
				return 1;
			}
			u32 trigger_modifier = trigger->modifier - 7;
			u32 trigger_time = trigger_var->time;
			if (trigger_var->modifier == 7)
			{
				trigger_time = trigger_var->extra;
			}
			else
			{
				u32 trigger_modifier_2 = trigger_modifier - 1;
				if (trigger_modifier_2)
				{
					if (trigger_modifier_2 == 1)
					{
						u32 trigger_extra = trigger_var->extra;
						if (trigger_extra <= trigger_time)
						{
							trigger_time -= trigger_extra;
						}
						else
						{
							trigger_time = 0;
						}
					}
				}
				else
				{
					trigger_time += trigger_var->extra;
				}
			}
			trigger_var->flags = trigger_flags | 1;
			u32 multiplayer_time = *g_multiplayer_mode ? trigger_time : -1;
			g_trigger_wait[player_id] = multiplayer_time;
			g_is_executing_wait[player_id] = 1;
			if (player_id == *g_local_nation_id)
			{
				helper::trigact_play_wav(trigger_var, a2);
				Unit* doodad = helper::get_doodad_for_unit_state(LOBYTE(trigger_var->source_location) - 1, trigger_var->type, 17);
				Unit* doodad_var = doodad; // Probably not necessary. -Pr0nogo
				if (doodad)
				{
					doodad->sprite->selection_timer = 45;
					helper::minimap_ping(doodad->sprite->position.x, doodad->sprite->position.y, 17);
					helper::game_talking_portrait(doodad_var->sprite->position.x, trigger_time, trigger_var->type, doodad_var->sprite->position.y);
				}
				else
				{
					helper::game_talking_portrait(-1, trigger_time, trigger_var->type, -1);
				}
				if (g_options_list->flags & 0x400 || trigger_var->flags & 4)
				{
					char* string = helper::get_chk_string(trigger_var->text_string);
					if (trigger_time > helper::get_text_display_time(string))
					{
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
			if (scbw::is_allied_to(*g_local_nation_id, player)) {
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
			u32 checksum = *g_minimap_box_checksum;
			while (true) {
				if (units_dat::base_property[unit->id] & UnitProperty::Subunit || unit->id == UnitId::Spell_DarkSwarm) {
					goto next;
				}

				u32 visibility = unit->sprite->visibility_status;
				u32 unit_status = unit->status;
				if (unit_status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) {
					visibility &= unit->detection_status;
				}
				checksum ^= visibility;
				if (*g_minimap_draw_neutrals) {
					break;
				}

				u32 vision = 0;
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

	// Hooked to add features and color Vespene Ridges cyan. -Pr0nogo
	void draw_all_minimap_boxes() { // 004A4AC0
		Bitmap* surface = *g_current_surface;
		u32 player = *g_local_nation_id;
		*g_current_surface = g_minimap_surface_2;
		*g_minimap_unit_box_count = 0;
		*g_minimap_box_checksum = 0;
		u32 index = 12;
		while (index) {
			if (--index < 8) {
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
			if (thingy->unit_id != UnitId::Spell_DarkSwarm) {
				u32 is_visible = helper::thingy_is_visible(1, thingy);
				if (!*g_is_in_replay || !is_visible) {
					Sprite* sprite = thingy->sprite;

					char color = g_palette_index_reference[25];
					if (sprite->id != SpriteId::VespeneRidge
					&&	sprite->id != SpriteId::VespeneGeyser
					&&  sprite->id != SpriteId::MineralFieldType1
					&&  sprite->id != SpriteId::MineralFieldType2
					&&  sprite->id != SpriteId::MineralFieldType3) {
						if (*g_minimap_state) {
							if (scbw::is_allied_to(player, sprite->player)) {
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
						ActiveTile* tile = scbw::get_active_tile_at_coords(pos);
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
		if (*g_active_portrait_unit && (*g_active_portrait_unit)->player == *g_local_nation_id)
		{
			u32 selection = 0;
			while (1)
			{
				Unit* selected = g_client_selection_group->unit[selection];
				if (selected)
				{
					if (helper::accepts_right_click_commands(selected))
					{
						break;
					}
				}
				if (++selection >= SelectionCap)
				{
					return;
				}
			}
			if (selection < SelectionCap)
			{
				u32 selection_2 = 0;
				while (1)
				{
					Unit* selected_2 = g_client_selection_group->unit[selection_2];
					if (selected_2)
					{
						if (helper::can_move(selected_2))
						{
							break;
						}
					}
					if (++selection_2 >= SelectionCap)
					{
						return;
					}
				}
				if (selection_2 < SelectionCap)
				{
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
					helper::ground_attack_init(mouse_pos_y, mouse_pos_x);
				}
			}
		}
	}

	bool is_newline(char c) {
		return c == '\r' || c == '\n';
	}

	void skip_newlines(char** c) {
		while (is_newline(**c)) {
			*c += 1;
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
}