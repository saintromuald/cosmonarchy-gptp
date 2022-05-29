#include "research_upgrade_orders.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_UpgradeWrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_Upgrade(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_ResearchTechWrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_ResearchTech(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}
}

namespace hooks
{
	void injectResearchUpgradeOrdersHooks()
	{
		jump_patch(orders_UpgradeWrapper,		0x004546A0, 1);
		jump_patch(orders_ResearchTechWrapper,	0x004548B0, 1);
	}
}