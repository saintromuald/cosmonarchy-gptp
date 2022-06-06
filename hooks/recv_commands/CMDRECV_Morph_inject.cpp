#include "CMDRECV_Morph.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) cmdrecv_building_morph_wrapper()
	{
		static u16 wUnitType;
		__asm {
			MOV AX, [EAX + 0x01]
			MOV wUnitType, AX
			PUSHAD
		}
		hooks::CMDRECV_BuildingMorph(wUnitType);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_unit_morph_wrapper()
	{
		static u16 wUnitType;
		__asm {
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV AX, [EAX + 0x01]
			MOV wUnitType, AX
			POP EAX
			PUSHAD
		}
		hooks::CMDRECV_UnitMorph(wUnitType);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) cmdrecv_train_wrapper()
	{
		static char* buffer;
		__asm {
			MOV buffer, EAX
			PUSHAD
		}
		hooks::cmdrecv_train(buffer);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_liftoff_wrapper()
	{
		static char* buffer;
		__asm {
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV buffer, EAX
			POP EAX
			PUSHAD
		}
		hooks::cmdrecv_liftoff(buffer);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}
}

namespace hooks
{
	void injectCMDRECV_MorphHooks()
	{
		jump_patch(cmdrecv_building_morph_wrapper,	0x004C1910, 7);
		jump_patch(cmdrecv_unit_morph_wrapper,		0x004C1990, 3);
		jump_patch(cmdrecv_train_wrapper,			0x004C1C20, 6);
		jump_patch(cmdrecv_liftoff_wrapper,			0x004C1620, 1);
	}
}
