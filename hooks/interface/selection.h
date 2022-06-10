#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void unit_selection_click(Unit* clicked_unit);																	// 0046FB40
	u32 sort_all_units(Unit* unit, Unit** unit_list, Unit** units_in_bounds);										// 0046F0F0
	s32 select_multiple_units_from_unit_list(u32 unit_count, Unit** selection, char play_sound, u32 send_command);	// 0049AEF0
	u32 create_new_unit_selections_from_list(Unit** unit_list, u32 count);											// 0049AE40
	void __stdcall get_selected_units_in_box(Box16* coords);														// 0046FA40

	void inject_selection_hooks();
}