#include "buttonsets.h"
#include <hook_tools.h>

#include "SCBW/stat_txt_spec.h"
namespace
{

	//Originally 00458BC0  updateButtonSetEx
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

	;

	//Originally 00458D50  sub_458D50
	void __declspec(naked) updateButtonSet_Sub458D50Wrapper()
	{

		__asm PUSHAD

		hooks::updateButtonSet_Sub458D50();

		__asm {
			POPAD
			RET
		}

	}

	;

	//Originally 00458DE0  updateButtonSet
	void __declspec(naked) updateButtonSetWrapper()
	{

		__asm PUSHAD

		hooks::updateButtonSet();

		__asm {
			POPAD
			RET
		}

	}

	;

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

	;

	//Originally 004599A0  updateCurrentButtonset
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

	;

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

	;




	//Modify offset 004591ED to use the custom
	//getButtonSet function
	//Don't replace the entire function
	//You can (and should) disable this if you're
	//already using the hooked 4591D0 function.
	const u32 Func_Sub_4591ED = 0x004591ED;
	const u32 Patch_For_Sub4591D0_Back = 0x004591F4;
	void __declspec(naked) getButtonSetPatch_For_Sub4591D0()
	{

		static u32 index;
		static BUTTON_SET* returnValue;

		__asm {
			MOV index, ECX
			PUSHFD
			PUSHAD
		}

		//In assembly, the index get multiplied to fit
		//the size of the variable to get (1st by 3 then
		//in the replaced code by 4), which is why the 
		//value must get divided here.

		returnValue = hooks::getButtonSet(index / 3);

		__asm {
			POPAD
			POPFD
			MOV ECX, returnValue
			PUSH Patch_For_Sub4591D0_Back
			RET
		}

	}

	;

} //unnamed namespace

void __declspec(naked) get_stattxt_string_Wrapper()
{
	static u16 stringId;
	static const char* message;
	__asm {

		MOV stringId, CX
		PUSHAD
	}

	/*
	if (stringId == 1637 ) {
		if (scbw::get_generic_value(nullptr, nullptr, AiseId::RecvEscalationValue,
			*ACTIVE_PLAYER_ID, UnitId::firebat) == UnitId::mercenary_gunship) {
			//add firebat
			stringId = 1764;
		}
	}
	if (stringId == 1684) {
		if (scbw::get_generic_value(nullptr, nullptr, AiseId::RecvEscalationValue,
			*ACTIVE_PLAYER_ID, UnitId::medic) == UnitId::civilian) {//

			//add firebat
			stringId = 1685;
		}
	}
	if (stringId == 1563) {
		if (scbw::get_generic_value(nullptr, nullptr, AiseId::RecvEscalationValue,
			*ACTIVE_PLAYER_ID, UnitId::firebat) == UnitId::mercenary_gunship) {
			//add firebat
			stringId = 1564;
		}
	}
	if (stringId == 1683) {
		if (scbw::get_generic_value(nullptr, nullptr, AiseId::RecvEscalationValue,
			*ACTIVE_PLAYER_ID, UnitId::medic) == UnitId::civilian) {
			//add firebat
			stringId = 1783;
		}
	}
	*/

	for (s32 i = 0; i < generictoggles; i++)
	{
		if (stringId == string_toggle_on[i])
		{
			if (scbw::get_generic_value(*activePortraitUnit, toggle_values[i]) == 1)
			{
				stringId = string_toggle_off[i];
			}
		}
	}

	if (stringId == 1633)
	{
		if (scbw::get_aise_value(*activePortraitUnit, nullptr, AiseId::SendCyprianValue, 0, 0) == 1)
		{
			stringId = 1634;
		}
	}

	message = (char*)statTxtTblNew->getString(stringId);

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
		jmpPatch(getButtonSetPatch_For_Sub4591D0, Func_Sub_4591ED, 2);
		jmpPatch(updateButtonSetExWrapper, 0x00458BC0, 1);
		jmpPatch(updateButtonSet_Sub458D50Wrapper, 0x00458D50, 2);
		jmpPatch(updateButtonSetWrapper, 0x00458DE0);
		jmpPatch(function_4591D0Wrapper, 0x004591D0, 1);
		jmpPatch(updateCurrentButtonsetWrapper, 0x004599A0, 6);
		jmpPatch(statbtn_BIN_CustomCtrlIDWrapper, 0x00459AD0, 2);


		callPatch(get_stattxt_string_Wrapper, 0x00459411);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x0045947E);//for escalation

		callPatch(get_stattxt_string_Wrapper, 0x00425C28);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x00457729);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x00457791);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x00457805);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x0045786D);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x004578C9);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x004588E1);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x0049276F);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x0049298B);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x004BE56A);//for escalation
		callPatch(get_stattxt_string_Wrapper, 0x0045947E);//for escalation

	}

} //hooks
