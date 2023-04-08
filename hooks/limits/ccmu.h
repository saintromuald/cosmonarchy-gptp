#pragma once
#include "scbw/structures.h"

struct CCMU_Extender {
	Unit unit_table[UnitLimit];
	int air_splash_list[UnitLimit];
	UnitFinderData unit_finder_x[UnitLimit * 2];
	UnitFinderData unit_finder_y[UnitLimit * 2];

	int __leading_get_all_units_in_bounds_list;
	int get_all_units_in_bounds_list[UnitLimit + 1]; // Both arrays require an extra DWORD at the end to work properly. Looks like they also have an extra one at the beginning, judging by the IDA struct, but seems to work fine without them.
	int __leading_get_all_units_in_bounds_index;
	int get_all_units_in_bounds_index[UnitLimit + 1];

	Sprite sprite_table[SpriteLimit]; 
	Sprite* sprite_list_pointers[SpriteLimit]; // 0x006C2318

	Order order_table[OrderLimit];

	Image image_table[ImageLimit];
	int __leading_image_list_pointers;
	int image_list_pointers[ImageLimit]; // 0x006C4A38, a DWORD leads this array at 6C4A30, but I've never seen it be accessed.

	Thingy thingy_table[ThingyLimit]; // 0x006509D8
	Thingy fow_sprite_table[FowSpriteLimit]; // 0x00652928
};

extern CCMU_Extender ccmu_extender;

namespace hooks {
	void inject_unit_extender();
	void inject_sprite_extender();
	void inject_order_extender();
	void inject_image_extender();
	void inject_thingy_extender();
}