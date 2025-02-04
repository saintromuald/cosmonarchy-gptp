#pragma once
#include <BW/api.h>

namespace hooks {
s32 init_placement_box_pos(s32 unused, Point16* placement);																										// 0048D660
u8 draw_placement_box(Point16 ecx0);																															// 0048DDC0
s32 update_placement_state_for_placement_box();																													// 0048DCE0
s32 is_tile_blocked_by(Unit** unit_array, Unit* builder, s32 x_tile, s32 y_tile, s8 dont_ignore_reacting, s8 also_invisible);									// 00473300
int update_creep_building_placement_state(u8 player, u16 x_tile, u16 y_tile, Point16 size_wh, u32 placement_state_entry, u32 check_vision, u32 check_vision_2); // 00473010
u32 update_building_placement_state_map_tile_flags(u16 unit_id, u16 tile_x, u16 tile_y, u8 player, Point16 size, u32 placement_state_entry, u32 check_creep);	// 00473A10
s32 start_building(Unit* builder);																																// 00467030
void place_building_refinery_check(Unit* result);													// 0048E1E0
void place_building_refinery_proc();																// 0048E430
void place_building_proc();																			// 0048E6E0
b32 unit_id_is_refinery(u16 unit_id);																// 0047AFE0
void issue_place_building_order(u16 unit_id, u8 order_id);											// 0048E730
void play_building_placement_sound(Unit* unit);														// 0048F630
void set_worker_error(Unit* unit, u32 error_string);												// 0048F7B0
void play_worker_error_sound(Unit* unit, u32 error_text_index);										// 0048F6C0
Thingy* __stdcall get_placement_restrictions_from_sprite_id(u32 tile_x, u32 tile_y, u32 radius);	// 00487B00
s32 place_building_message(s32 placement_state);													// 0048D930
void input_place_building_click_notify(u8* order_id, u16* building_id, bool is_valid);				// 0048E4E0
s32 vespene_geyser_placebox(u32 tile_x, u32 tile_y, u8 player, u32 check_vision);					// 00473DB0

void inject_placement_hooks();
}