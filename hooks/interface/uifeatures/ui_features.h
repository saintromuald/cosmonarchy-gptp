#pragma once
#include "BW/bwdata.h"
#include "BW/api.h"

#define ActionSampleTime 10
#define MaxActions 2000

struct ApmCounter {
    u32 action_count;
    u32 action_time[MaxActions];
};

inline const u32 UiTextHeight = 15;
inline Point16 g_ui_text_pos;

inline ApmCounter g_local_apm_counter;
inline ApmCounter g_player_apm_counter[PlayerLimit];

inline bool g_disable_mouse_scroll;

void format_time(char* buffer, u32 size, const char* prefix, u32 seconds, bool allow_short);

namespace hooks {
void draw_text(const char* text, u32 x, u32 y, u32 w, u32 h, TextColor color = TextColor::Grey);
void draw_ui_text(const char* text, TextColor color = TextColor::Grey);

void targeted_order(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, u8 a7, s32 a8, s32 a9, s32 a10);
s32 select_units(Unit** a1, s32 a2, u8 a3, s32 a4);
void queue_game_command(u8* data, u32 size);

b32 check_hotkey_char(BinDlg* dialog, char hotkey);
char create_upgrade_level_tooltip(char* text);
s32 trigact_transmission(TriggerAction* trigger);																		// 004C7890
void draw_minimap_unit_box(u32 player);																					// 004A4650
void draw_all_minimap_boxes();																							// 004A4AC0
u32 get_cursor_type();																									// 004D1460
void cmdact_right_click_minimap(DialogEvent* dlg_event);																// 004563A0
void draw_vertical_line(u16 size, u16 x, u16 y);																		// 0041D7D0
void __fastcall centerview_unit_group(u8 slot);																			// 004967E0
void draw_standard_button_graphic(BinDlg* button1, BinDlg* button2, BinDlg* dest);										// 004EFD90
b32 generic_dialog_update(BinDlg* button, u32 width, u32 height, Box16* dims);											// 004EF590
void draw_stat_button_image(BinDlg* dialog);																			// 00456C30
void statbtn_button_update(BinDlg* button);																				// 00458900
void load_statdata_bin();																								// 00459B90
void print_stat_res_info(BinDlg* dialog, u16 graphic, u8** recent_spending, u8* resource_amount, b32 instant_update);	// 004E52E0

void register_action(ApmCounter* counter, char* buffer);
void init_actions();
void update_actions();
void draw_ui_features();

char input_mmb_down(DialogEvent* event);											// 00484520
int __cdecl input_mmb_up();															// 004843A0
void __cdecl input_mmb_move();														// 00484460

bool statport_dlg_interact(BinDlg* dialog, DialogEvent* dlg_event);					// 0045F290
char CMDRECV_minimap_ping(char* buffer);											// 004BF7A0
void __cdecl show_or_hide_minimap();												// 004A4150
u16 draw_game_menu_context(BinDlg* dialog);                                         // 004F4F70
void get_tile_image_data(s32 offset_arg, vx4 tile);                                 // 0040C3B0
char* get_stat_txt_string(u16 string_id);                                           // 004C36F0
void draw_show_hide_terrain_context(BinDlg* dialog);                                // 004A4C40
void draw_cancel_unit_context(BinDlg* dialog);                                      // 00457B10
void statdata_mouseover_interact(BinDlg* main_dialog, BinDlg* secondary_dialog);    // 00457CE0

void inject_ui_features_hooks();
}

#define tIconCount 7
extern CmdBtnPalette g_new_cmdbtn_palette[tIconCount];