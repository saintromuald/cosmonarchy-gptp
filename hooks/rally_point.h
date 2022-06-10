#pragma once
#include "../SCBW/structures/Unit.h"
#include "SCBW/structures.h"

namespace hooks
{
	void set_rally_unit(Unit* unit, Unit* target);				// 00466B40
	void order_new_unit_to_rally(Unit* unit, Unit* factory);	// 00466F50
	void set_rally_position(Unit* unit, u16 x, u16 y);			// 00466910
	u32 btnscond_rally(u32 unused, u32 player, Unit* unit);		// 00429520

	Order* remove_references(Unit* unit, Unit* dying);			// 0049E590
	Unit* right_click_command(u32 x, u32 y, Unit *unit, u32 fow_unit_id, char queued); // 004560D0

	bool unit_can_have_rally(Unit* unit);						// 0047B2E0
	bool unit_can_rally(Unit* unit);
	bool unit_can_have_rally_original(Unit* unit);

	u32 order_acquire_generic_right_click_unit_proc_2(Unit* unit, u32 x_pos, u16 y_pos, Unit** target, u16 fow_unit_id);	// 00455E00
	void process_right_click_action(Unit* unit, u32 order_unit_id, u16 y_pos, u32 x_pos, Unit* target);						// 00455F20
	u8 get_generic_unit_idle_order(u32 order_unit_id, Unit* unit);															// 00455760
	void input_game_right_mouse_click(DialogEvent* dlg_event);																// 004564E0
	u32 can_do_targeted_order(Unit* unit, u8 order_id);																		// 0049A410
	char get_right_click_action_order(Unit* unit);																			// 004E5EA0
	s32 __stdcall btnscond_can_move_special_case(u32 buffer);																// 00428D40
	s32 __stdcall btnscond_can_move(u32 buffer);																			// 00428DA0
	s32 __stdcall btnscond_can_attack(u32 buffer);																			// 00428F30
	s32 unit_can_move(Unit* unit);																							// 00455660
	void drone_build_execute(Unit* unit);																					// 0045D2E0

	void inject_rally_hooks();
}