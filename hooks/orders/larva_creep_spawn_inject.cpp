#include "larva_creep_spawn.h"
#include <hook_tools.h>
#include <SCBW/scbwdata.h>

namespace
{	
	void __declspec(naked) function_0049D660_Wrapper() // 0x0049D660
	{
		static Unit* main_building;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			MOV main_building, ESI
			PUSHAD
		}
		hooks::function_0049D660(main_building);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}

	void __declspec(naked) function_004E8C80_Wrapper() // 0x004E8C80
	{
		static Unit* main_building;
		static Unit* unit;
		static bool bResult;
		__asm
		{
			MOV unit, ECX
			MOV main_building, EDX
			PUSHAD
		}
		bResult = hooks::function_004E8C80(unit, main_building);
		if (bResult)
		{
			__asm
			{
				POPAD
				MOV EAX, 0x00000001
				RET
			}
		}
		else
		{
			__asm
			{
				POPAD
				XOR EAX, EAX
				RET
			}
		}
	}
		
	void __declspec(naked) secondaryOrd_SpawningLarva_Wrapper() // 0x004EA780
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			MOV unit, ESI
			PUSHAD
		}
		hooks::secondaryOrd_SpawningLarva(unit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}

	void __declspec(naked) secondaryOrd_SpreadCreepSpawningLarva_Wrapper() // 0x004EA880
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::secondaryOrd_SpreadCreepSpawningLarva(unit);
		__asm
		{
			POPAD
			RET
		}
	}

	/*
	void __declspec(naked) spreadCreep_Wrapper() // 0x00414680
	{
		u8 spread_creep(u16 unit_id, s32 x, s32 y);

		static u16 unit_id;
		static s32 x;
		static s32 y;
		static u8 result;

		__asm
		{
		MOV unit_id, DX
		MOV x, ECX
		MOV y, EAX
		PUSHAD
		}
		result = hooks::spread_creep(unit_id, x, y);
		__asm
		{
		POPAD
		MOV AL, result
		RET
		}
	}
	*/

	void __declspec(naked) getCreepRegion_Wrapper() // 0x00413DB0
	{
		static Box32* rect;
		static u16 unit_id;
		static s32 x;
		static s32 y;
		static u32 isComplete;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV rect, ECX
			MOV unit_id, DX
			MOV isComplete, EAX
			MOV EAX, [EBP + 0x08]
			MOV x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV y, EAX
			PUSHAD
		}
		result = hooks::getCreepRegion(rect, unit_id, x, y, isComplete);
		__asm
		{
			POPAD
			MOV EAX, result
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) spreadsCreep_Wrapper() // 0x00413870
	{
		static u16 unit_id;
		static u32 isComplete;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit_id, AX
			MOV EAX, [EBP + 0x08]
			MOV isComplete, EAX
			PUSHAD
		}
		result = hooks::spreads_creep(unit_id, isComplete);
		__asm
		{
			POPAD
			MOV EAX, result
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) runCreepRandomizer_Wrapper() // 0x004140A0
	{
		static u16* tileBackup;
		static u8* creepOverlay;
		static u32* retval;
		static u16* tile;
		static u32 arg;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV tileBackup, EAX
			MOV EAX, [EBP + 0x0C]
			MOV creepOverlay, EAX
			MOV EAX, [EBP + 0x10]
			MOV retval, EAX
			MOV EAX, [EBP + 0x14]
			MOV tile, EAX
			MOV EAX, [EBP + 0x18]
			MOV arg, EAX
			PUSHAD
		}
		result = hooks::runCreepRandomizer(tileBackup, creepOverlay, retval, tile, arg);
		__asm
		{
			POPAD
			MOV EAX, result
			POP EBP
			RETN 20
		}
	}

	// Not a real function lol -- I think it is actually an inline function because every instance is almost exactly the same
	void __declspec(naked) inCreepArea_Call()
	{
		static s32 x, y;
		static u32 r;
		__asm
		{
			MOV x, EDX
			MOV y, EAX
			PUSHAD
		}
		r = hooks::inCreepArea(x, y);
		__asm
		{
			POPAD
			MOV EDX, r
			RET
		}
	}

	const u32 func_ret = 0x0047DD48;
	const u32 func_call = 0x00414290;
	const u32 func_cont = 0x0047DCD3;
	void __declspec(naked) bugWrapper()
	{
		static void* disappearingCreepPtr;
		__asm
		{
			CALL func_call
			MOV disappearingCreepPtr, ESI
			PUSHAD
		}
		if (disappearingCreepPtr == nullptr)
		{
			__asm
			{
				POPAD
				JMP func_ret
			}
		}
		else
		{
			__asm
			{
				POPAD
				MOVZX ECX, [ESI + 0x12]
				JMP func_cont
			}
		}
	}

	void __declspec(naked) BTNSCOND_ZergBuildingRally_Wrapper() // 00429740
	{
		static u32 unused;
		static u32 player;
		static Unit* unit;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			MOV player, EDX
			MOV unused, ECX
			PUSHAD
		}
		result = hooks::BTNSCOND_ZergBuildingRally(unused, player, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) UpdateCreepDisappearanceUnit_Wrapper() // 0047DF90
	{
		static Unit* unit;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::UpdateCreepDisappearanceUnit(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) should_disappear_creep_unit_wrapper() {
		static const u32 jmp_address = 0x0047DFC1 + 5;
		static u32 result;
		__asm {
			PUSHAD
		}
		result = 1;
		__asm {
			POPAD
			CMP result, 1
			JMP jmp_address
		}
	}

	void __declspec(naked) should_disappear_creep_unit_id_wrapper() {
		static const u32 jmp_address = 0x0047DE61 + 5;
		static u32 result;
		static u32 unit_id;
		__asm {
			PUSHAD
		}
		result = 1;
		__asm {
			POPAD
			CMP result, 1
			JMP jmp_address
		}
	}
}

namespace hooks
{
	void injectLarvaCreepSpawnHooks()
	{
		// Code blocks to replace what I can only assume is an inline function. First instructions are NOP's, last instruction is JE using the existing immediate value
		//0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30
		u8 func_a[31] = {0x8D,0xB6,0x00,0x00,0x00,0x00,0x8D,0xBC,0x27,0x00,0x00,0x00,0x00,0x8B,0xC8,0x8B,0x45,0xF0,0x8B,0x55,0xF4,0xE8,0   ,0   ,0   ,0   ,0x8B,0xC1,0x85,0xD2,0x74};
		u8 func_bcd[25] = {0x90,0x8D,0xB4,0x26,0x00,0x00,0x00,0x00,0x8D,0xBC,0x27,0x00,0x00,0x00,0x00,0x8B,0xD1,0xE8,0   ,0   ,0   ,0   ,0x85,0xD2,0x74};
		u8 func_e[25] = {0x89,0xF6,0x8D,0xBC,0x27,0x00,0x00,0x00,0x00,0x8B,0xD8,0x8B,0xC1,0x8B,0xD3,0xE8,0   ,0   ,0   ,0   ,0x8B,0xC3,0x85,0xD2,0x74};
		u8 func_f[15] = {0x8B,0xD1,0xE8,0   ,0   ,0   ,0   ,0x33,0xC0,0x85,0xD2,0x0F,0x94,0xC0,0xC3};

		jump_patch(function_0049D660_Wrapper, 0x0049D660, 1);
		jump_patch(function_004E8C80_Wrapper, 0x004E8C80);
		jump_patch(secondaryOrd_SpawningLarva_Wrapper, 0x004EA780, 1);
		jump_patch(secondaryOrd_SpreadCreepSpawningLarva_Wrapper, 0x004EA880, 3);

//		jump_patch(spreadCreep_Wrapper, 0x00414680); // not really useful

		jump_patch(getCreepRegion_Wrapper, 0x00413DB0);
		jump_patch(spreadsCreep_Wrapper, 0x00413870);
		jump_patch(runCreepRandomizer_Wrapper, 0x004140A0);
//		jump_patch(UpdateCreepDisappearanceUnit_Wrapper, 0x0047DF90, 0); // broken

		jump_patch(bugWrapper, 0x0047DCCA); // attempt to fix Bug

//		jump_patch(BTNSCOND_ZergBuildingRally_Wrapper, 0x00429740, 2);
		
		const u32 offs_a = 0x004147A4;
		const u32 offs_b = 0x004145E7;
		const u32 offs_c = 0x0047D810;
		const u32 offs_d = 0x0047DEE7;
		const u32 offs_e = 0x0047E057;
		const u32 offs_f = 0x00413850;

		// Insert function offset
		*(u32*)(&func_a[22]) = (u32)(&inCreepArea_Call) - (offs_a + 22 + 4);
		memory_patch(offs_a, func_a, 31);

		*(u32*)(&func_bcd[18]) = (u32)(&inCreepArea_Call) - (offs_b + 18 + 4);
		memory_patch(offs_b, func_bcd, 25);

		*(u32*)(&func_bcd[18]) = (u32)(&inCreepArea_Call) - (offs_d + 18 + 4);
		memory_patch(offs_d, func_bcd, 25);

		func_bcd[16] = 0xD6;
		*(u32*)(&func_bcd[18]) = (u32)(&inCreepArea_Call) - (offs_c + 18 + 4);
		memory_patch(offs_c, func_bcd, 25);

		*(u32*)(&func_e[16]) = (u32)(&inCreepArea_Call) - (offs_e + 16 + 4);
		memory_patch(offs_e, func_e, 25);

		*(u32*)(&func_f[3]) = (u32)(&inCreepArea_Call) - (offs_f + 3 + 4);
		memory_patch(offs_f, func_f, sizeof(func_f));

		// Allow spreading underneath structures
		u32 zero = 0;
		memory_patch(0x00414777 + 3, zero);
		memory_patch(0x00414780 + 2, zero);
		memory_patch(0x004149B5 + 3, zero);
		memory_patch(0x0045CE07 + 3, zero);
		jump_patch(should_disappear_creep_unit_wrapper,    0x0047DFC1, 0);
		jump_patch(should_disappear_creep_unit_id_wrapper, 0x0047DE61, 0);
	}
}
