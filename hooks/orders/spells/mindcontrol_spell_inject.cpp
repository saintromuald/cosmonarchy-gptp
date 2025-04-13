#include "mindcontrol_spell.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_cast_tyranny_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_cast_maledictus(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	const u32 give_units = 0x004C8040;
	const u32 MindControl_Hit_F6938 = 0x004F6938;
	void __declspec(naked) tyranny_hit_patch() {
		__asm {
			CALL	give_units
			PUSH	EAX
			MOV		EAX, [EBX + 0xC]//unit->sprite
			MOV		AL, [EAX + 0xA]//unit->sprite->player
			PUSH	EDX
			MOV		EDX, [ESI + 0xC]//this->sprite
			MOV[EDX + 0xA], AL//this->sprite->player = unit->sprite->player
			MOV		EDX, [ESI + 0x70]//this->subunit
			TEST	EDX, EDX
			JZ		offset no_subunit
			MOV		EDX, [EDX + 0xC]//this->subunit->sprite
			MOV[EDX + 0xA], AL//this->subunit->sprite->player = unit->sprite->player

			no_subunit :
			POP		EDX
				POP		EAX
				JMP		MindControl_Hit_F6938
		}
	}
}

namespace hooks
{
	void inject_tyranny_spell_hooks()
	{
		jump_patch(orders_cast_tyranny_wrapper,	0x004F6950, 0);

		jump_patch(tyranny_hit_patch, 0x004F6933);	// No longer used since we have this hooked, stopped Tyrants from draining controlled Tyrants
	}
}