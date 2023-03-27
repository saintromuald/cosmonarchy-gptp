#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void unit_selection_click(Unit* clicked_unit);																	// 0046FB40
	u32 sort_all_units(Unit* unit, Unit** unit_list, Unit** units_in_bounds);										// 0046F0F0
	s32 select_multiple_units_from_unit_list(u32 unit_count, Unit** selection, char play_sound, u32 send_command);	// 0049AEF0
	u32 create_new_unit_selections_from_list(Unit** unit_list, u32 count);											// 0049AE40
	void __stdcall get_selected_units_in_box(Box16* coords);														// 0046FA40
	void create_unit_selection(Unit* unit, char index);																// 004E6180
	Image* create_selection_circle(Sprite* sprite, u8 color, u16 image_id);											// 004D7070
	b32 is_human_player_on_same_team(u32 player_id);																// 0049A110
	char get_team_size(u32 player);																					// 00485710
	b32 game_is_team_game();																						// 00484F50
	char create_dashed_selection_main(Unit* unit, u32 player);														// 004E65CD

	void inject_selection_hooks();
}