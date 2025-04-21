#pragma once
#include "../../BW/structures/Unit.h"
#include "BW/structures.h"

char g_grpwire_buffer[GroupWireframesCap][8];

namespace hooks {
void unit_selection_click(Unit* clicked_unit);																	// 0046FB40
u32 sort_all_units(Unit* unit, Unit** unit_list, Unit** units_in_bounds);										// 0046F0F0
s32 select_multiple_units_from_unit_list(u32 unit_count, Unit** selection, char play_sound, u32 send_command);	// 0049AEF0
u32 create_new_unit_selections_from_list(Unit** unit_list, u32 count);											// 0049AE40
void __stdcall get_selected_units_in_box(Box16* coords);														// 0046FA40
void create_unit_selection(Unit* unit, char index);																// 004E6180
Image* create_selection_circle(Sprite* sprite, u8 color, u16 image_id);											// 004D7070
b32 is_human_player_on_same_team(u32 player_id);																// 0049A110
u8 get_team_size(u32 player);																					// 00485710
b32 game_is_team_game();																						// 00484F50
void create_dashed_selection_main(Unit* unit, u32 player);														// 004E65C0
s32 remove_from_selections(Unit* unit);																			// 0049A7F0
s32 select_dual_birth_units(Unit* unit);																		// 0045D040
bool is_selected_unit_group_enabled();																			// 0049A2C0
char select_unit_initial();																						// 0049B690
char create_all_selection_circles();																			// 00499A60
void update_selected_unit_data();																				// 004C38B0
void status_screen_button(BinDlg* dialog);																		// 00458220
void generate_group_wireframes(BinDlg* dialog);																	// 00425960
void destroy_unit_selection(Unit* unit);                                                                        // 004E6290

bool can_add_unit_to_active_selection(Unit* unit, u32 slot);													// partial helper

void inject_selection_hooks();
}