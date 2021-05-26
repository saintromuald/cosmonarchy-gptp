#include "supply_hooks.h"
#include <hook_tools.h>
#include "SCBW/api.h"

namespace
{
	void __declspec(naked) getSuppliesAvailable_Wrapper()
	{
		static u8 playerId;
		static s32 race;
		static u32 result;
		__asm
		{
			MOV race, ECX
			MOV playerId, AL
			PUSHAD
		}
		result = hooks::getSuppliesAvailable(playerId, race);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getSuppliesRemaining_Wrapper()
	{
		static u8 playerId;
		static u16 unitId;
		static s32 result;
		__asm
		{
			MOV unitId, DI
			MOV playerId, BL
			PUSHAD
		}
		result = hooks::getSuppliesRemaining(playerId, unitId);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getSuppliesMax_Wrapper()
	{
		static u16 unitId;
		static u8 playerId;
		static s32 result;
		__asm
		{
			MOV playerId, CL
			MOV unitId, AX
			PUSHAD
		}
		result = hooks::getSuppliesMax(unitId, playerId);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getSuppliesUsed_Wrapper()
	{
		static s32 race;
		static u8 playerId;
		static s32 result;
		__asm
		{
			MOV playerId, CL
			MOV race, EAX
			PUSHAD
		}
		result = hooks::getSuppliesUsed(race, playerId);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) initializeUnitCounts_Wrapper()
	{
		static char result;
		__asm
		{
			PUSHAD
		}
		result = hooks::initializeUnitCounts();
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void injectSupplyHooks()
	{
		jmpPatch(getSuppliesAvailable_Wrapper, 0x00488900, 1);
		jmpPatch(getSuppliesRemaining_Wrapper, 0x004889D0, 1);
		jmpPatch(getSuppliesMax_Wrapper, 0x00488990, 4);
		jmpPatch(getSuppliesUsed_Wrapper, 0x004888C0, 1);
		jmpPatch(initializeUnitCounts_Wrapper, 0x00488F90, 0);
	}
}