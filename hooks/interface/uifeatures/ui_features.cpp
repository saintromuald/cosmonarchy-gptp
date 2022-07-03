#include "ui_features.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include "config.h"
#include "trigger_system.h"

u64 total_client_actions;

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
	}

	void clear_actions() { // Called on map start
		total_client_actions = 0;
	}

	void draw_ui_features() {
		ui_text_pos = {10, 3};
		if (*leaderboard_type) {
			ui_text_pos.x += 80;
		}

		if (config.show_timer) {
			char buffer[32] = {};
			format_time(buffer, 32, "", *g_elapsed_time_frames / 24);
			draw_ui_text(buffer, 7);
		}

		if (config.show_apm) {
			u32 apm = total_client_actions / (*g_elapsed_time_frames / 24.0 / 60.0);

			char buffer[32] = {};
			sprintf_s(buffer, "APM: %u", apm);
			draw_ui_text(buffer, 7);
		}

		for (u32 i = 0; i < objectives.count; ++i) {
			Objective* objective = &objectives.entries[i];
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
		if (a6 && orders_dat::CanBeQueued[(u8)a7]) {
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

		char* label = helper::getStatTxtString(dialog->button_user->requirement_string_id);
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
			if ((*g_current_trigger_node)->trigger->flags & 0x10)
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
			u32 multiplayer_time = multiplayer_mode ? trigger_time : -1;
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
				if (options_list->flags & 0x400 || trigger_var->flags & 4)
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
	char draw_minimap_unit_box(u32 player) // 004A4650
	{
		u32 xd = *g_minimap_state;
		u32 unit_id = UnitId::None;
		char color_mod = 0;
		char index_ref = 0;
		if (*g_minimap_state)
		{
			xd = 3 * *g_local_nation_id;
			if (scbw::is_allied_to(*g_local_nation_id, player))
			{
				xd = g_palette_index_reference[17];
				index_ref = g_palette_index_reference[17];
				goto DrawMore;
			}
			color_mod = g_palette_index_reference[20];
		}
		else
		{
			color_mod = g_player_minimap_colors[player];
		}
		index_ref = color_mod;
	DrawMore:
		Unit* first_unit = *(first_player_unit[player].unit);
		if (first_unit)
		{
			u32 checksum = *g_minimap_box_checksum;
			while (1)
			{
				unit_id = first_unit->id;
				xd = first_unit->id;
				if (units_dat::base_property[unit_id] & UnitProperty::Subunit || unit_id == UnitId::Spell_DarkSwarm)
				{
					goto DrawBox;
				}

				xd = first_unit->sprite->visibility_flags;
				u32 unit_status = first_unit->status;
				if (unit_status & 0x300)
				{
					xd &= first_unit->visibility_status;
				}
				checksum ^= xd;
				if (*g_minimap_draw_neutrals)
				{
					break;
				}

				u32 vision = 0;
				if (g_is_in_replay)
				{
					if (xd & *g_replay_visions)
					{
						break;
					}
					xd = *g_replay_reveal_all;
					vision = *g_replay_reveal_all == 0;
				}
				else
				{
					vision = (xd & *g_player_visions) == 0;
				}
				if (!vision)
				{
					break;
				}
			DrawBox:
				first_unit = first_unit->player_link.next;
				if (!first_unit)
				{
					*g_minimap_box_checksum = checksum;
					return xd;
				}
			}
			u32 color = g_palette_index_reference[25];
			if (!(units_dat::base_property[unit_id] & UnitProperty::ResourceContainer))
			{
				color = index_ref;
			}
			xd = helper::draw_unit_box(first_unit->sprite->position.x, color, first_unit->sprite->position.y,
									   units_dat::building_dimensions[first_unit->id].x, units_dat::building_dimensions[first_unit->id].y,
									   units_dat::base_property[first_unit->id] & UnitProperty::Building);

			goto DrawBox;
		}
		return xd;
	}

	// Hooked to add features and color Vespene Ridges cyan. -Pr0nogo
	void draw_all_minimap_boxes() // 004A4AC0
	{
		Bitmap* surface = *g_current_surface;
		Bitmap* surface_2 = *g_current_surface;
		u32 player = *g_local_nation_id;
		*g_current_surface = g_minimap_surface_2;
		*g_minimap_unit_box_count = 0;
		*g_minimap_box_checksum = 0;
		u32 btw = 12;
		do
		{
			if (--btw < 8)
			{
				if (player != btw || *g_is_in_replay)
				{
					helper::draw_minimap_unit_box_2(btw);
				}
			}
			else
			{
				helper::draw_minimap_unit_box(btw);
			}
		} while (btw);
		if (!*g_is_in_replay)
		{
			helper::draw_all_minimap_unit_boxes(player);
		}
		Thingy* thingy = *first_used_thingy;
		if (*first_used_thingy)
		{
			do
			{
				u32 unit_id = thingy->unit_id;
				if (unit_id != UnitId::Spell_DarkSwarm)
				{
					u32 is_visible = helper::thingy_is_visible(1, thingy);
					player = *g_local_nation_id;
					if (!*g_is_in_replay || !is_visible)
					{
						Sprite* sprite = thingy->sprite;
						u32 sprite_id = sprite->sprite_id;
						char ref = 0;

						if (sprite_id != SpriteId::Vespene_Geyser
						&&  sprite_id != SpriteId::Mineral_Field_Type1
						&&  sprite_id != SpriteId::Mineral_Field_Type2
						&&  sprite_id != SpriteId::Mineral_Field_Type3
						&&  sprite_id != SpriteId::VespeneRidge)
						{
							if (*g_minimap_state)
							{
								if (scbw::is_allied_to(player, sprite->player))
								{
									ref = g_palette_index_reference[17];
								}
								else
								{
									ref = g_palette_index_reference[20];
								}
							}
							else
							{
								ref = g_player_minimap_colors[(unsigned __int8)sprite->player];
							}
						}
						else
						{
							ref = g_palette_index_reference[25];
						}
						helper::draw_unit_box(thingy->sprite->position.x, ref, thingy->sprite->position.y, units_dat::building_dimensions[(thingy->unit_id)].x, units_dat::building_dimensions[(thingy->unit_id)].y, 1);
						--*g_minimap_unit_box_count;
					}
				}
				thingy = thingy->next;
			} while (thingy);
			*g_current_surface = surface_2;
		}
		else
		{
			*g_current_surface = surface;
		}
	}
}