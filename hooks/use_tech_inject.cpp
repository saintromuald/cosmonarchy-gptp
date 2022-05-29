#include "use_tech.h"
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"
#include "hook_tools.h"

namespace
{
	void __declspec(naked) UseTechWrapper()
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
		hooks::BTNSACT_UseTech(id, grphead);
		__asm
		{
			POPAD
			RETN
		}
	}

	const u32 Hook_Jump_SkipUpgradeOp = 0x0046D5CC;
	const u32 Hook_Jump_UpgradeFunctionality = 0x0046D5BE;
	void __declspec(naked) UpgradeIdComparison_Wrapper()
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
}

namespace hooks
{
	void injectUseTechHooks()
	{
		jump_patch(UseTechWrapper,				0x00423F70);
		jump_patch(UpgradeIdComparison_Wrapper,	0x0046D5B8);
	}
}