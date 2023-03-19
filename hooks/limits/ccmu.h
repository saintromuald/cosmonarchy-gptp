#pragma once
#include "scbw/structures.h"

extern Unit new_unit_table[UnitLimit];
extern int new_air_splash_list[UnitLimit];
extern UnitFinderData new_unit_finder_x[UnitLimit * 2];
extern UnitFinderData new_unit_finder_y[UnitLimit * 2];
extern int new_get_all_units_in_bounds_list[UnitLimit + 1];
extern int new_get_all_units_in_bounds_index[UnitLimit + 1];

extern Sprite new_sprite_table[SpriteLimit];
extern Sprite* new_sprite_list_pointers[SpriteLimit];

extern Order new_order_table[OrderLimit];

extern Image new_image_table[ImageLimit];
extern int new_image_list_pointers[ImageLimit]; // 0x006C4A38, a DWORD leads this array at 6C4A30, but I've never seen it be accessed.

extern Thingy new_thingy_table[ThingyLimit];
extern Thingy new_fow_sprite_table[FowSpriteLimit];

namespace hooks {
	void inject_unit_extender();
	void inject_sprite_extender();
	void inject_order_extender();
	void inject_image_extender();
	void inject_thingy_extender();
}