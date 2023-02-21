#pragma once
#include "scbw/structures.h"

#include "extender.h"
#include <vector>

struct CCMUExtender : public Extender
{
	virtual std::vector<u32> get_address_list();
	virtual std::vector<u32> get_size_list();
	std::vector<u32> table_start_list();
	std::vector<u32> table_end_list();
	std::vector<u32> sprite_offset_list();
	std::vector<u32> orderingXlist();
	std::vector<u32> orderingXlist_off4();
	std::vector<u32> orderingYlist();
	std::vector<u32> orderingYlist_off4();
	std::vector<u32> orderingY_min18endlist();
	UnitLayout units[UNIT_ARRAY_LENGTH];
	UnitFinderData orderingX[UNIT_ARRAY_LENGTH * 2];
	UnitFinderData orderingY[UNIT_ARRAY_LENGTH * 2];
	Unit* position_search_units[UNIT_ARRAY_LENGTH + 1];
	u32 position_search_temp[UNIT_ARRAY_LENGTH + 1];
	Unit* air_splash_results[UNIT_ARRAY_LENGTH];

	virtual void extend();
	virtual s32 identifier(std::string str);
	virtual u32 parse_entry_value(std::string str, s32 id);
	virtual bool is_string_id(s32 id);
	virtual void set_value(s32 entry, s32 id, std::string str);
	virtual void set_value(s32 entry, s32 id, s32 val);
	virtual void set_extended_data();
};

namespace hooks
{
	void injectCCMUHooks();
}

extern CCMUExtender new_unit_table;


/* // DF - start

#define UnitLimit 4095
#define UnitIDMask 0xFFF // Happens to be the same value as UnitLimit here, but it doesn't necessarily have to be.
extern Unit newer_unit_table[UnitLimit];
extern UnitFinderData new_unit_finderX[UnitLimit * 2];
extern UnitFinderData new_unit_finderY[UnitLimit * 2];
extern int new_get_all_units_in_bounds_list[UnitLimit + 1];

#define SpriteLimit 8875 // Must be a multiple of 5, as well as a multiple of 3 when decremented by one.
extern Sprite new_sprite_table[SpriteLimit];

#define OrderLimit 4080
#define newCheckLimit OrderLimit - (OrderLimit / 10) // Reserve 10% of order array for non-queued orders.
extern Order new_order_table[OrderLimit];

#define ImageLimit 16385 // Must be a multiple of 5.
extern Image new_image_table[ImageLimit];

namespace hooks {
	void inject_unit_extender();
	void inject_sprite_extender();
	void inject_order_extender();
	void inject_image_extender();
}

*/