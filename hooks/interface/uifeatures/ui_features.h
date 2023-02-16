#pragma once
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"

const u32 ui_text_height = 15;
extern Point16 ui_text_pos;

void format_time(char* buffer, u32 size, char* prefix, u32 seconds);

namespace hooks {
	void draw_text(const char* text, u32 x, u32 y, u32 w, u32 h, char color = 2);
	void draw_ui_text(const char* text, u8 color = 2);

	void targeted_order(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, u8 a7, s32 a8, s32 a9, s32 a10);
	s32 select_units(Unit** a1, s32 a2, u8 a3, s32 a4);
	void queue_game_command(u8* data, u32 size);

	b32 check_hotkey_char(BinDlg* dialog, char hotkey);
	char create_upgrade_level_tooltip(char* text);
	s32 trigact_transmission(TriggerAction* trigger, char* a2);													// 004C7890
	void draw_minimap_unit_box(u32 player);																		// 004A4650
	void draw_all_minimap_boxes();																				// 004A4AC0
	u32 get_cursor_type();																						// 004D1460
	void cmdact_right_click_minimap(DialogEvent* dlg_event);													// 004563A0
	s8* draw_vertical_line(s32 unused, u16 iterations, s16 width_offset, u16 buffer_multiplier);				// 0041D7D0

	void inject_ui_features_hooks();

	void register_action(u8 type);
	void init_actions();
	void update_actions();
	void draw_ui_features();
}