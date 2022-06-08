#pragma once
#include "../../SCBW/structures/Unit.h"
#include "SCBW/structures.h"

namespace hooks
{
	void cmdrecv_building_morph(u16 wUnitType);																				// 004C1910
	void cmdrecv_unit_morph(u16 wUnitType);																					// 004C1990
	void cmdrecv_train(char* buffer);																						// 004C1C20
	void cmdrecv_liftoff(char* buffer);																						// 004C1620
	void btnsact_rally_point(u32 xd);																						// 004244A0

	u32 order_acquire_generic_right_click_unit_proc(Unit* unit, u32 x_pos, u16 y_pos, Unit** target, u16 fow_unit_id);		// 00455CC0
	u32 order_acquire_generic_right_click_unit_proc_2(Unit* unit, u32 x_pos, u16 y_pos, Unit** target, u16 fow_unit_id);	// 00455E00
	void process_right_click_action(Unit* unit, u32 order_unit_id, u16 y_pos, u32 x_pos, Unit* target);						// 00455F20
	u8 get_generic_unit_idle_order(u32 order_unit_id, Unit* unit);															// 00455760
	void input_game_right_mouse_click(DialogEvent* dlg_event);																// 004564E0
	u32 can_do_targeted_order(Unit* unit, u8 order_id);																		// 0049A410
	char get_right_click_action_order(Unit* unit);																			// 004E5EA0

	void inject_cmdrecv_morph_hooks();
}