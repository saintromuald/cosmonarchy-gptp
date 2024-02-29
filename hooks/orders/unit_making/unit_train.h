#pragma once
#include "../../../BW/structures/Unit.h"

namespace hooks
{
	s32 is_queue_slot_active(Unit* unit, u32 slot);
	void secondary_order_train_fighter(Unit* unit);			// 00466790
	void order_train(Unit* unit);							// 00468420
	char get_max_hangar_space(Unit* unit);					// 004653D0
	b32 _stdcall button_is_training(Unit* unit);			// 00428530
	void update_unit_stats_and_finish_building(Unit* unit);	// 004A01F0

	void inject_unit_train_hooks();
}