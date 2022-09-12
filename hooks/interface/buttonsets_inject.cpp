#include "buttonsets.h"
#include <hook_tools.h>

#include "SCBW/stat_txt_spec.h"
namespace
{
	void __declspec(naked) update_buttonset_ex_wrapper()
	{
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::update_buttonset_ex();
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}

	void __declspec(naked) highlight_button_wrapper()
	{
		__asm PUSHAD
		hooks::highlight_button();
		__asm {
			POPAD
			RET
		}
	}

	void __declspec(naked) update_buttonset_wrapper()
	{
		__asm PUSHAD
		hooks::update_buttonset();
		__asm {
			POPAD
			RET
		}
	}

	void __declspec(naked) update_command_card_wrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::update_command_card();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) update_current_buttonset_wrapper()
	{
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::update_current_buttonset();
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}

	void __declspec(naked) statbtn_bin_custom_ctrl_id_wrapper()
	{
		static BinDlg* dialog;
		__asm {
			MOV dialog, EAX
			PUSHAD
		}
		hooks::statbtn_bin_custom_ctrl_id(dialog);
		__asm {
			POPAD
			RETN
		}
	}

	// Modify offset 004591ED to use the custom get_buttonset function
	// You can (and should) disable this if you're already using the hooked 4591D0 function.
	const u32 func_update_command_card = 0x004591ED;
	const u32 patch_for_update_command_card = 0x004591F4;
	void __declspec(naked) update_command_card_patch()
	{
		static u32 index;
		static ButtonSet* result;
		__asm {
			MOV index, ECX
			PUSHFD
			PUSHAD
		}
		// In assembly, the index get multiplied to fit the count of the variable to get (1st by 3 then in the replaced code by 4), which is why the  value must get divided here.
		result = hooks::get_buttonset(index / 3);
		__asm {
			POPAD
			POPFD
			MOV ECX, result
			PUSH patch_for_update_command_card
			RET
		}
	}
}

namespace hooks
{
	void inject_buttonset_hooks()
	{
	//	jump_patch(update_command_card_patch,			func_update_command_card, 2);
		jump_patch(update_buttonset_ex_wrapper,			0x00458BC0, 1);
		jump_patch(highlight_button_wrapper,			0x00458D50, 2);
		jump_patch(update_buttonset_wrapper,			0x00458DE0, 0);
		jump_patch(update_command_card_wrapper,			0x004591D0, 1);
		jump_patch(update_current_buttonset_wrapper,	0x004599A0, 6);
		jump_patch(statbtn_bin_custom_ctrl_id_wrapper,	0x00459AD0, 2);
	}
}