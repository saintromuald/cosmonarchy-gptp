#include "larva_creep_spawn.h"
#include <hook_tools.h>

namespace {

	//0x0049D660
	void __declspec(naked) function_0049D660_Wrapper() {

		static CUnit* main_building;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			MOV main_building, ESI
			PUSHAD
		}

		hooks::function_0049D660(main_building);

		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}

	}

	;

	//0x004E8C80
	void __declspec(naked) function_004E8C80_Wrapper() {

		static CUnit* main_building;
		static CUnit* unit;
		static bool bResult;

		__asm {
			MOV unit, ECX
			MOV main_building, EDX
			PUSHAD
		}

		bResult = hooks::function_004E8C80(unit, main_building);

		if (bResult)
			__asm {
			POPAD
			MOV EAX, 0x00000001
			RET
		}
		else
			__asm {
			POPAD
			XOR EAX, EAX
			RET
		}

	}

	;

	//0x004EA780
	void __declspec(naked) secondaryOrd_SpawningLarva_Wrapper() {

		static CUnit* unit;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			MOV unit, ESI
			PUSHAD
		}

		hooks::secondaryOrd_SpawningLarva(unit);

		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}

	}

	;

	//0x004EA880
	void __declspec(naked) secondaryOrd_SpreadCreepSpawningLarva_Wrapper() {

		static CUnit* unit;

		__asm {
			MOV unit, EAX
			PUSHAD
		}

		hooks::secondaryOrd_SpreadCreepSpawningLarva(unit);

		__asm {
			POPAD
			RET
		}

	}

	;


	//0x00414680
	/*void __declspec(naked) spreadCreep_Wrapper() {
	// u8 spreadCreep(u16 unitId, s32 x, s32 y);

	static u16 unitId;
	static s32 x;
	static s32 y;
	static u8 result;

	__asm {
	MOV unitId, DX
	MOV x, ECX
	MOV y, EAX
	PUSHAD
	}

	result = hooks::spreadCreep(unitId, x, y);

	__asm {
	POPAD
	MOV AL, result
	RET
	}

	}*/

	// 0x00413DB0
	void __declspec(naked) getCreepRegion_Wrapper() {
		// bool getCreepRegion(Box32* rect, u16 unitId, s32 x, s32 y, bool defRetVal);

		static Box32* rect;
		static u16 unitId;
		static s32 x;
		static s32 y;
		static u32 isComplete;
		static u32 result;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV rect, ECX
			MOV unitId, DX
			MOV isComplete, EAX
			MOV EAX, [EBP + 0x08]
			MOV x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV y, EAX
			PUSHAD
		}

		result = hooks::getCreepRegion(rect, unitId, x, y, isComplete);

		__asm {
			POPAD
			MOV EAX, result
			POP EBP
			RETN 8
		}

	}

	// 0x00413870
	void __declspec(naked) spreadsCreep_Wrapper() {
		// bool spreadsCreep(u16 unitId, bool baseRetVal);

		static u16 unitId;
		static u32 isComplete;
		static u32 result;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unitId, AX
			MOV EAX, [EBP + 0x08]
			MOV isComplete, EAX
			PUSHAD
		}

		result = hooks::spreadsCreep(unitId, isComplete);

		__asm {
			POPAD
			MOV EAX, result
			POP EBP
			RETN 4
		}

	}

	// 0x004140A0
	void __declspec(naked) runCreepRandomizer_Wrapper() {
		// bool runCreepRandomizer(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg);

		static u16* tileBackup;
		static u8* creepOverlay;
		static u32* retval;
		static u16* tile;
		static u32 arg;
		static u32 result;

		__asm {
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

		__asm {
			POPAD
			MOV EAX, result
			POP EBP
			RETN 20
		}

	}

	// Not a real function lol -- I think it is actually an inline function because every instance is almost exactly the same
	void __declspec(naked) inCreepArea_Call() {
		static s32 x, y;
		static u32 r;

		__asm {
			MOV x, EDX
			MOV y, EAX
			PUSHAD
		}
		r = hooks::inCreepArea(x, y);
		__asm {
			POPAD
			MOV EDX, r
			RET
		}
	}
	const u32 func_ret = 0x0047DD48;
	const u32 func_call = 0x00414290;
	const u32 func_cont = 0x0047DCD3;
	void __declspec(naked) bugWrapper() {
		static void* disappearingCreepPtr;
		__asm {
			CALL func_call
			MOV ESI, disappearingCreepPtr
			PUSHAD
		}
		if (disappearingCreepPtr == NULL) {
			__asm {
				POPAD
				JMP func_ret
			}
		}
		else {
			__asm {
				POPAD
				JMP func_cont
			}
		}
	}

} //unnamed namespace

namespace hooks {

	void injectLarvaCreepSpawnHooks() {
		// Code blocks to replace what I can only assume is an inline function. First instructions are NOP's, last instruction is JE using the existing immediate value
		//0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30
		u8 func_a[31] = { 0x8D,0xB6,0x00,0x00,0x00,0x00,0x8D,0xBC,0x27,0x00,0x00,0x00,0x00,0x8B,0xC8,0x8B,0x45,0xF0,0x8B,0x55,0xF4,0xE8,0   ,0   ,0   ,0   ,0x8B,0xC1,0x85,0xD2,0x74 };
		u8 func_bcd[25] = { 0x90,0x8D,0xB4,0x26,0x00,0x00,0x00,0x00,0x8D,0xBC,0x27,0x00,0x00,0x00,0x00,0x8B,0xD1,0xE8,0   ,0   ,0   ,0   ,0x85,0xD2,0x74 };
		u8 func_e[25] = { 0x89,0xF6,0x8D,0xBC,0x27,0x00,0x00,0x00,0x00,0x8B,0xD8,0x8B,0xC1,0x8B,0xD3,0xE8,0   ,0   ,0   ,0   ,0x8B,0xC3,0x85,0xD2,0x74 };
		u8 func_f[15] = { 0x8B,0xD1,0xE8,0   ,0   ,0   ,0   ,0x33,0xC0,0x85,0xD2,0x0F,0x94,0xC0,0xC3 };



		jmpPatch(function_0049D660_Wrapper, 0x0049D660, 1);
		jmpPatch(function_004E8C80_Wrapper, 0x004E8C80);
		jmpPatch(secondaryOrd_SpawningLarva_Wrapper, 0x004EA780, 1);
		jmpPatch(secondaryOrd_SpreadCreepSpawningLarva_Wrapper, 0x004EA880, 3);

		//jmpPatch(spreadCreep_Wrapper, 0x00414680); // not really useful
		
		jmpPatch(getCreepRegion_Wrapper, 0x00413DB0);
		jmpPatch(spreadsCreep_Wrapper, 0x00413870);
		jmpPatch(runCreepRandomizer_Wrapper, 0x004140A0);


		//
		jmpPatch(bugWrapper, 0x0047DCCA);//attempt to fix Bug
		//

		const u32 offs_a = 0x004147A4;
		const u32 offs_b = 0x004145E7;
		const u32 offs_c = 0x0047D810;
		const u32 offs_d = 0x0047DEE7;
		const u32 offs_e = 0x0047E057;
		const u32 offs_f = 0x00413850;

		// Insert function offset
		*(u32*)(&func_a[22]) = (u32)(&inCreepArea_Call) - (offs_a + 22 + 4);
		memoryPatch(offs_a, func_a, 31);

		*(u32*)(&func_bcd[18]) = (u32)(&inCreepArea_Call) - (offs_b + 18 + 4);
		memoryPatch(offs_b, func_bcd, 25);

		*(u32*)(&func_bcd[18]) = (u32)(&inCreepArea_Call) - (offs_d + 18 + 4);
		memoryPatch(offs_d, func_bcd, 25);

		func_bcd[16] = 0xD6;
		*(u32*)(&func_bcd[18]) = (u32)(&inCreepArea_Call) - (offs_c + 18 + 4);
		memoryPatch(offs_c, func_bcd, 25);

		*(u32*)(&func_e[16]) = (u32)(&inCreepArea_Call) - (offs_e + 16 + 4);
		memoryPatch(offs_e, func_e, 25);

		*(u32*)(&func_f[3]) = (u32)(&inCreepArea_Call) - (offs_f + 3 + 4);
		memoryPatch(offs_f, func_f, sizeof(func_f));
	}
}
