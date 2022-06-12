#include "use_tech.h"
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"
#include "hook_tools.h"

namespace
{
	void __declspec(naked) btnsact_use_tech_wrapper() // 00423F70
	{
		static u16 id;
		static GrpHead* grphead;
		__asm
		{
			MOV id, CX
			MOV grphead, ESI
			PUSHAD
		}
		// grphead framecount (1-9) is button number in command card
		hooks::btnsact_use_tech(id, grphead);
		__asm
		{
			POPAD
			RETN
		}
	}

	const u32 Hook_Jump_SkipUpgradeOp = 0x0046D5CC;
	const u32 Hook_Jump_UpgradeFunctionality = 0x0046D5BE;
	void __declspec(naked) upgrade_id_comparison_wrapper()
	{
		// EDX (0xffff), CX (firegraft shift), EAX (1)
		static u32 edx_0xffff;
		static u16 opcode;
		static u32 eax_res;
		__asm {
			MOV edx_0xffff, EAX
			MOV opcode, CX
			MOV eax_res, EAX
			PUSHAD
		}
		if (opcode < scbw::get_aise_value(nullptr, nullptr, AiseId::UpgradeLength, 0, 0))
		{
			__asm {
				POPAD
				JMP Hook_Jump_UpgradeFunctionality
			}
		}
		else
		{
			__asm {
				POPAD
				JMP Hook_Jump_SkipUpgradeOp
			}
		}
	}

	void __declspec(naked) tech_use_allowed_wrapper() { // 0046DD80
		static u32 result;
		static Unit* unit;
		static u16 tech_id;
		static u32 player;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			POP EAX
			MOV tech_id, DI
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::tech_use_allowed(unit, tech_id, player);
		__asm {
			POPAD
			MOV EAX, result
			RETN 0x04
		}
	}
}

namespace hooks
{
	void inject_use_tech_hooks()
	{
		jump_patch(btnsact_use_tech_wrapper,		0x00423F70, 1);
		jump_patch(upgrade_id_comparison_wrapper,	0x0046D5B8, 0);
		jump_patch(tech_use_allowed_wrapper,		0x0046DD80, 2);
	}
}