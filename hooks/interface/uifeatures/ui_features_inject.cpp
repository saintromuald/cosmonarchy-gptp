#include "ui_features.h"
#include "hook_tools.h"
#include "data_ext.h"
#include "config.h"

namespace
{
	const u32 SetFontHandle = 0x0041F610;
	const u32 drawUIFeatures_jmp = 0x0048D0AE;
	void _declspec(naked) draw_ui_features_wrapper()
	{
		_asm {
			PUSHAD
		}
		hooks::draw_ui_features();
		_asm {
			POPAD
			CALL SetFontHandle
			JMP drawUIFeatures_jmp
		}
	}

	void _declspec(naked) queue_game_command_wrapper() {
		static u8* data;
		static u32 len;
		static u8 result;
		_asm {
			MOV data, ECX
			MOV len, EDX
			PUSHAD
		}
		hooks::queue_game_command(data, len);
		_asm {
			POPAD
			MOV EAX, 0
			RETN
		}
	}

	void __declspec(naked) targeted_order_wrapper()
	{
		static s16 a1;
		static s32 a2;
		static Unit* a3;
		static s16 a4;
		static s32 a5;
		static s32 a6;
		static s32 a7;
		static s32 a8;
		static s32 a9;
		static s32 a10;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV a1, DX
			MOV a2, EBX
			MOV a3, ESI
			MOV a4, AX
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV a5, EAX
			MOV EAX, [EBP + 0xC]
			MOV a6, EAX
			MOV EAX, [EBP + 0x10]
			MOV a7, EAX
			MOV EAX, [EBP + 0x14]
			MOV a8, EAX
			MOV EAX, [EBP + 0x18]
			MOV a9, EAX
			MOV EAX, [EBP + 0x1C]
			MOV a10, EAX
			POP EAX
			PUSHAD
		}
		hooks::targeted_order(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x18
		}
	}

	void __declspec(naked) select_units_wrapper()
	{
		static Unit** a1;
		static s32 a2;
		static s32 a3;
		static s32 a4;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV a1, EBX
			MOV a2, EDI
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV a3, EAX
			MOV EAX, [EBP + 0xC]
			MOV a4, EAX
			POP EAX
			PUSHAD
		}
		hooks::select_units(a1, a2, a3, a4);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) check_hotkey_char_wrapper()
	{
		static b32 result;
		static BinDlg* dialog;
		static char* hotkey;
		__asm
		{
			MOV dialog, ECX
			MOV hotkey, EDX
			PUSHAD
		}
		result = hooks::check_hotkey_char(dialog, *hotkey);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) create_upgrade_level_tooltip_wrapper()
	{
		static char result;
		static char* text;
		__asm
		{
			MOV text, ECX
			PUSHAD
		}
		result = hooks::create_upgrade_level_tooltip(text);
		__asm
		{
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) trigact_transmission_wrapper() // 004C7890
	{
		static u32 result;
		static TriggerAction* trigger;
		static char* unused;
		__asm
		{
			MOV unused, EDI
			MOV trigger, ECX
			PUSHAD
		}
		result = hooks::trigact_transmission(trigger, unused);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) draw_minimap_unit_box_wrapper() { // 004A4650
		static char result;
		static u32 player;
		__asm {
			MOV player, ECX
			PUSHAD
		}
		hooks::draw_minimap_unit_box(player);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) draw_all_minimap_boxes_wrapper() { // 004A4AC0
		__asm {
			PUSHAD
		}
		hooks::draw_all_minimap_boxes();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) get_cursor_type_wrapper() { // 004D1460
		static u32 result;
		__asm {
			PUSHAD
		}
		result = hooks::get_cursor_type();
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) cmdact_right_click_minimap_wrapper() { // 004563A0
		static DialogEvent* dlg_event;
		__asm {
			MOV dlg_event, EAX
			PUSHAD
		}
		hooks::cmdact_right_click_minimap(dlg_event);
		__asm {
			POPAD
			RETN
		}
	}

	bool show_fow_unit_check(Unit* unit) {
		if (unit->status & (UnitStatus::Burrowed | UnitStatus::Cloaked | UnitStatus::CloakingForFree)) {
			if (helper::get_detecting_players(unit) & *g_player_visions) {
				return true;
			}
			return false;
		}
		return true;
	}

	void __declspec(naked) show_fow_unit_check_wrapper() {
		static const u32 jmp_success = 0x004EBEA1;
		static const u32 jmp_fail = 0x004EBEC2;
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		if (show_fow_unit_check(unit)) {
			__asm {
				POPAD
				JMP jmp_success
			}
		}
		__asm {
			POPAD
			JMP jmp_fail
		}
	}

	bool include_sprite_in_vischeck(Sprite* sprite) {
		SpriteExt* sprite_ext = get_sprite_ext(sprite);
		if (sprite_ext->is_fow_sprite) {
			return false;
		}
		return true;
	}
	
	void __declspec(naked) include_sprite_in_vischeck_wrapper() {
		static const u32 jmp_address = 0x00497C72;
		static Sprite* sprite;
		__asm {
			MOV sprite, EAX
			PUSHAD
		}
		if (include_sprite_in_vischeck(sprite)) {
			//__asm {
			//	POPAD
			//	MOV DL, [EAX+0x0C]
			//	XOR [ECX], DL
			//	JMP jmp_address
			//}
		}
		__asm {
			POPAD
			JMP jmp_address
		}
	}
	
	void create_fow_sprite_partial(Thingy* thingy, Sprite* sprite) {
		thingy->unit_id = 1; // Original functionality replaced by the jump patch

		SpriteExt* sprite_ext = get_sprite_ext(sprite);
		sprite_ext->is_fow_sprite = true;
	}

	void __declspec(naked) create_fow_sprite_partial_wrapper() {
		static const u32 jmp_address = 0x00488474;
		static Thingy* thingy;
		static Sprite* sprite;
		__asm {
			MOV thingy, ESI
			MOV sprite, EAX
			PUSHAD
		}
		create_fow_sprite_partial(thingy, sprite);
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	void __declspec(naked) sprite_destructor_wrapper() {
		static Sprite* sprite;
		__asm {
			MOV sprite, EDI
			PUSHAD
		}
		sprite->free();
		__asm {
			POPAD
			MOV EAX, 0
			RETN
		}
	}

	void __declspec(naked) draw_vertical_line_wrapper() { // 0041D7D0
		static u16 size_;
		static u16 x;
		static u16 y;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV x, AX
			MOV EAX, [EBP + 0x0C]
			MOV y, AX
			POP EAX
			MOV size_, DX
			PUSHAD
		}
		hooks::draw_vertical_line(size_, x, y);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			MOV EAX, 0
			RETN 0x08
		}
	}

	void __declspec(naked) draw_standard_button_graphic_wrapper() { // 004EFD90
		static BinDlg* dialog1;
		static BinDlg* dialog2;
		static BinDlg* dest;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV dest, EAX
			POP EAX
			MOV dialog2, EDI
			MOV dialog1, ECX
			PUSHAD
		}
		hooks::draw_standard_button_graphic(dialog1, dialog2, dest);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) generic_dialog_update_wrapper() { // 004EF590
		static b32 result;
		static BinDlg* button;
		static u32 width;
		static u32 height;
		static Box16* dims;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV height, EAX
			MOV EAX, [EBP + 0x0C]
			MOV dims, EAX
			POP EAX
			MOV width, EDX
			MOV button, ECX
			PUSHAD
		}
		result = hooks::generic_dialog_update(button, width, height, dims);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) draw_stat_button_image_wrapper() { // 00456C30
		static BinDlg* dialog;
		__asm {
			MOV dialog, ESI
			PUSHAD
		}
		hooks::draw_stat_button_image(dialog);
		__asm {
			POPAD
			MOV EAX, 0
			RETN
		}
	}

	void __declspec(naked) statbtn_button_update_wrapper() { // 00458900
		static void* result;
		static BinDlg* dialog;
		__asm {
			MOV dialog, ECX
			PUSHAD
		}
		hooks::statbtn_button_update(dialog);
		__asm {
			POPAD
			MOV EAX, 0
			RETN 0x08
		}
	}

	void __declspec(naked) load_statdata_bin_wrapper() // 00459B90
	{
		__asm {
			PUSHAD
		}
		hooks::load_statdata_bin();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) print_stat_res_info_wrapper() // 004E52E0
	{
		static BinDlg* dialog;
		static u16 graphic;
		static u8** recent_spending;
		static u8* resource_amount;
		static b32 instant_update;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV graphic, AX
			MOV EAX, [EBP + 0x0C]
			MOV recent_spending, EAX
			MOV EAX, [EBP + 0x10]
			MOV resource_amount, EAX
			MOV EAX, [EBP + 0x14]
			MOV instant_update, EAX
			POP EAX
			MOV dialog, EBX
			PUSHAD
		}
		hooks::print_stat_res_info(dialog, graphic, recent_spending, resource_amount, instant_update);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) input_mmb_down_wrapper() { // 00484520
		static char result;
		static DialogEvent* event;
		__asm {
			MOV event, ECX
			PUSHAD
		}
		result = hooks::input_mmb_down(event);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) statport_dlg_interact_wrapper() { // 0045F290
		static bool result;
		static BinDlg* dialog;
		static DialogEvent* dlg_event;
		__asm {
			MOV dialog, ECX
			MOV dlg_event, EDX
			PUSHAD
		}
		result = hooks::statport_dlg_interact(dialog, dlg_event);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}

	// Non-hooks \\

	const u32 replayStatBtns_27AB5 = 0x00427AB5;
	const u32 replayStatBtns_27AE4 = 0x00427AE4;
	const u32 show_dialog = 0x004186A0;
	void __declspec(naked) replayStatBtns_patch_27AB0() {
		static b32 cfg_value = config.show_replay_progress_bar;
		__asm {
			MOV		EAX, cfg_value
			TEST	EAX, EAX
			JE		offset hide_progress_bar
			CALL	show_dialog
			JMP		replayStatBtns_27AB5
			hide_progress_bar :
			MOV		byte ptr[EBP - 3], 0
			MOV		byte ptr[EBP - 4], 3
			JMP		replayStatBtns_27AE4
		}
	}

	void __declspec(naked) get_scroll_directions_partial() { // 0047F155
		static const u32 jmp_back = 0x004D12A0;
		if (disable_mouse_scroll) {
			__asm {
				MOV EAX, 0
				RET
			}
		}
		__asm {
			JMP jmp_back
		}
	}
}

namespace hooks
{
	void inject_ui_features_hooks()
	{
		jump_patch(draw_ui_features_wrapper,				0x0048D0A9, 0);
	//	jump_patch(targeted_order_wrapper,					0x004754F0, 1);
	//	jump_patch(select_units_wrapper,					0x0046FA00, 1);
		jump_patch(queue_game_command_wrapper,				0x00485BD0, 4);
		jump_patch(check_hotkey_char_wrapper,				0x004588C0, 3);
	//	jump_patch(create_upgrade_level_tooltip_wrapper,	0x00459150, 3);
	//	jump_patch(trigact_transmission_wrapper,			0x004C7890, 0);
		jump_patch(draw_minimap_unit_box_wrapper,			0x004A4650, 4);
		jump_patch(draw_all_minimap_boxes_wrapper,			0x004A4AC0, 0);
		jump_patch(get_cursor_type_wrapper,					0x004D1460, 0);
		jump_patch(cmdact_right_click_minimap_wrapper,		0x004563A0, 1);

		// Allow units in fow (seems to desync)
	//	jump_patch(show_fow_unit_check_wrapper,				0x004EBE88, 2); // Allow all unit ids to create fow sprites
	//	jump_patch(include_sprite_in_vischeck_wrapper,		0x00497C6D, 0); // Exclude fow sprites from visibility checks as they are not synced
	//	jump_patch(create_fow_sprite_partial_wrapper,		0x0048846D, 2); // Mark sprites as fow sprites in SpriteExt

		jump_patch(sprite_destructor_wrapper,               0x00497B40, 1); // Clear SpriteExt on sprite destruction
	//	jump_patch(draw_vertical_line_wrapper,				0x0041D7D0, 4); // crashes
		jump_patch(centerview_unit_group,					0x004967E0, 1);
	//	jump_patch(draw_standard_button_graphic_wrapper,	0x004EFD90, 2); // not needed
		jump_patch(generic_dialog_update_wrapper,			0x004EF590, 1);
		jump_patch(draw_stat_button_image_wrapper,			0x00456C30, 1);
		jump_patch(statbtn_button_update_wrapper,			0x00458900, 1);
	//	jump_patch(load_statdata_bin_wrapper,				0x00459B90, 4);
	//	jump_patch(print_stat_res_info_wrapper,				0x004E52E0, 0); // Crashes
		jump_patch(input_mmb_down_wrapper,					0x00484520, 0);
		jump_patch(input_mmb_up,							0x004843A0, 0);
		jump_patch(input_mmb_move,							0x00484460, 1);
	//	jump_patch(statport_dlg_interact_wrapper,			0x0045F290, 2);

		call_patch(get_scroll_directions_partial,			0x0047F155, 0);
		jump_patch(replayStatBtns_patch_27AB0,				0x00427AB0);	// Don't show replay progress bar based on config. This doesn't hide it, just never shows it. -DF

		// Passive icon colorization -- extension of color remaps. -DF
		memory_patch(0x00459C1D, (u8)0x70); // Since it's a byte value push, this is the largest buffer size with this mempatch. Will need a jump_patch or hook to support more colors.
		memory_patch(0x00456C59, &new_cmdbtn_palette);
		memory_patch(0x004589D5, &new_cmdbtn_palette);
		memory_patch(0x00459C1F, &new_cmdbtn_palette);
	}
}