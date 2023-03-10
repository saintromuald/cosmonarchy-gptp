#pragma once
#include "scbw/structures.h"

#define UnitLimit 4095
#define UnitIDMask 0xFFF // Happens to be the same value as UnitLimit here, but it doesn't necessarily have to be.
extern Unit new_unit_table[UnitLimit];
extern UnitFinderData new_unit_finder_x[UnitLimit * 2];
extern UnitFinderData new_unit_finder_y[UnitLimit * 2];
extern int new_get_all_units_in_bounds_list[UnitLimit + 1];

#define SpriteLimit 8875 // Must be a multiple of 5, as well as a multiple of 3 when decremented by one.
extern Sprite new_sprite_table[SpriteLimit];

#define OrderLimit 4080
#define NewCheckLimit (OrderLimit - (OrderLimit / 10)) // Reserve 10% of order array for non-queued orders.
extern Order new_order_table[OrderLimit];

#define ImageLimit 16385 // Must be a multiple of 5.
extern Image new_image_table[ImageLimit];

#define ThingyLimit1 2048
#define ThingyLimit2 2048

extern Thingy new_thingy_table1[ThingyLimit1];	// 0x006509D8
extern Thingy new_thingy_table2[ThingyLimit2];	// 0x00652928

namespace hooks {
	void inject_unit_extender();
	void inject_sprite_extender();
	void inject_order_extender();
	void inject_image_extender();
	void inject_thingy_extender();
}