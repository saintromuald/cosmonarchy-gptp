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
		draw_text(text, ui_text_pos.x, ui_text_pos.y, 200, ui_y_increment, color);
		ui_text_pos.y += ui_y_increment;
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
			format_time(buffer, 32, "", *elapsed_time_frames / 24);
			draw_ui_text(buffer, 7);
		}

		if (config.show_apm) {
			u32 apm = total_client_actions / (*elapsed_time_frames / 24.0 / 60.0);

			char buffer[32] = {};
			sprintf_s(buffer, "APM: %u", apm);
			draw_ui_text(buffer, 7);
		}

		for (u32 i = 0; i < objectives.count; ++i) {
			draw_ui_text(objectives.entries[i].data, 7);
		}
	}

	void queue_game_command(u8* data, u32 size) {
		if (*command_queue_size + size <= *provider_MaxBuffer) {
			register_action(data[0]);
			memcpy((void*)&TurnBuffer[*command_queue_size], data, size);
			*command_queue_size += size;
			return;
		}

		if (*scMainState == 4) {
			return;
		}

		u32 turns = 0;
		if (helper::snet_get_turns_in_transit(&turns)) {
			if (turns >= 16 - *provider_LatencyCalls) {
				return;
			}
			register_action(data[0]);
			helper::send_turn();
			memcpy((void*)&TurnBuffer[*command_queue_size], data, size);
			*command_queue_size += size;
			return;
		}

		if (!*outOfGame) {
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
						if (*COrderCount < 1800) {
							helper::queue_new_order((Order*)a5, a1, a4, a3, a7, a8, a2, (Vector32*)a9, (Unit*)a10);
						} else {
							u8 v11 = a3->player;
							if (*statTxtTbl > 0x368u) {
								helper::show_stat_txt_to_player(*statTxtTbl + statTxtTbl[873], v11);
							} else {
								helper::show_stat_txt_to_player((s32)"", v11);
							}
						}
					} else {
						u8 v10 = a3->player;
						if (*statTxtTbl > 0x366u) {
							helper::show_stat_txt_to_player(*statTxtTbl + statTxtTbl[871], v10);
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

		if (a2 != 1 || !IS_KEYCODE_USED[18] || result == 0) {
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
		if (!*upgradeLevelTooltip) {
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
		if (**statTxtBuffer > 794u) {
			description = (char *)*statTxtBuffer + (*statTxtBuffer)[795];
		} else {
			description = "";
		}
		char buffer[128];
		sprintf_s(buffer, 128, "%s\n%s %d", text, description, (s16)*upgradeLevelTooltip);
		scbw::printf("%s", buffer);
		return helper::create_tooltip(buffer);
	}
}