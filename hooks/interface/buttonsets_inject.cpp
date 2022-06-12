#include "buttonsets.h"
#include <hook_tools.h>

#include "SCBW/stat_txt_spec.h"
namespace
{
	void __declspec(naked) updateButtonSetExWrapper()
	{
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::updateButtonSetEx();
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}

	void __declspec(naked) updateButtonSet_Sub458D50Wrapper()
	{
		__asm PUSHAD
		hooks::updateButtonSet_Sub458D50();
		__asm {
			POPAD
			RET
		}
	}

	void __declspec(naked) updateButtonSetWrapper()
	{
		__asm PUSHAD
		hooks::updateButtonSet();
		__asm {
			POPAD
			RET
		}
	}

	void __declspec(naked) function_4591D0Wrapper()
	{
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::updateButtonSet_Sub4591D0();
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}

	void __declspec(naked) updateCurrentButtonsetWrapper()
	{
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::updateCurrentButtonset();
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}

	void __declspec(naked) statbtn_BIN_CustomCtrlIDWrapper()
	{
		static BinDlg* dialog;
		__asm {
			MOV dialog, EAX
			PUSHAD
		}
		hooks::statbtn_BIN_CustomCtrlID(dialog);
		__asm {
			POPAD
			RETN
		}
	}

	// Modify offset 004591ED to use the custom getButtonSet function
	// Don't replace the entire function
	// You can (and should) disable this if you're already using the hooked 4591D0 function.
	const u32 Func_Sub_4591ED = 0x004591ED;
	const u32 Patch_For_Sub4591D0_Back = 0x004591F4;
	void __declspec(naked) getButtonSetPatch_For_Sub4591D0()
	{
		static u32 index;
		static ButtonSet* returnValue;
		__asm {
			MOV index, ECX
			PUSHFD
			PUSHAD
		}
		// In assembly, the index get multiplied to fit
		// the size of the variable to get (1st by 3 then
		// in the replaced code by 4), which is why the 
		// value must get divided here.
		returnValue = hooks::getButtonSet(index / 3);
		__asm {
			POPAD
			POPFD
			MOV ECX, returnValue
			PUSH Patch_For_Sub4591D0_Back
			RET
		}
	}
}

void __declspec(naked) get_stattxt_string_Wrapper()
{
	static u16 stringId;
	static const char* message;
	__asm {

		MOV stringId, CX
		PUSHAD
	}
	// Escalations were here
	for (s32 i = 0; i < generictoggles; i++)
	{
		if (stringId == string_toggle_on[i])
		{
			if (scbw::get_generic_rust_value(*active_portrait_unit, toggle_values[i]))
			{
				stringId = string_toggle_off[i];
			}
		}
	}
	if (stringId == 1633)
	{
		if (scbw::get_aise_value(*active_portrait_unit, nullptr, AiseId::SendCyprianValue, 0, 0) == 1)
		{
			stringId = 1634;
		}
	}
	message = (char*)statTxtTblNew->get_string(stringId);
	__asm {
		POPAD
		MOV EAX, message
		RETN
	}
}

namespace hooks
{
	void injectButtonSetHooks()
	{
		jump_patch(getButtonSetPatch_For_Sub4591D0,		Func_Sub_4591ED, 2);
		jump_patch(updateButtonSetExWrapper,			0x00458BC0, 1);
		jump_patch(updateButtonSet_Sub458D50Wrapper,	0x00458D50, 2);
		jump_patch(updateButtonSetWrapper,				0x00458DE0);
		jump_patch(function_4591D0Wrapper,				0x004591D0, 1);
		jump_patch(updateCurrentButtonsetWrapper,		0x004599A0, 6);
		jump_patch(statbtn_BIN_CustomCtrlIDWrapper,		0x00459AD0, 2);

		// for escalations
		call_patch(get_stattxt_string_Wrapper,			0x00459411);
		call_patch(get_stattxt_string_Wrapper,			0x0045947E);
		call_patch(get_stattxt_string_Wrapper,			0x00425C28);
		call_patch(get_stattxt_string_Wrapper,			0x00457729);
		call_patch(get_stattxt_string_Wrapper,			0x00457791);
		call_patch(get_stattxt_string_Wrapper,			0x00457805);
		call_patch(get_stattxt_string_Wrapper,			0x0045786D);
		call_patch(get_stattxt_string_Wrapper,			0x004578C9);
		call_patch(get_stattxt_string_Wrapper,			0x004588E1);
		call_patch(get_stattxt_string_Wrapper,			0x0049276F);
		call_patch(get_stattxt_string_Wrapper,			0x0049298B);
		call_patch(get_stattxt_string_Wrapper,			0x004BE56A);
		call_patch(get_stattxt_string_Wrapper,			0x0045947E);
	}
}