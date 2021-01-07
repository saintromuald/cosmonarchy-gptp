#include "hooks\economic_units.h"
#include "hook_tools.h"

namespace hooks {

	const u32 jmp_34c53 = 0x00434C53;
	const u32 jmp_4C80_ret = 0x00434C80;
	void __declspec(naked) Ai_ProgressSpendingQueue_JmpWrapper() {
		static u32 unit_id;
		static u32 result;
		__asm {
			MOV unit_id, EAX
			PUSHAD
		}
		result = get_order_from_unit_id(unit_id);
		if(result==0){//default case
			__asm {
				POPAD
				JMP jmp_4C80_ret
			}
		}
		else {
			__asm {
				POPAD
				MOV EAX,result
				JMP jmp_34c53
			}
		}
	}

		
	//probably is going to be hook instead
	void __declspec(naked) Ai_BuildSupplies_Wrapper() {
		static AiTown* town;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV town, EAX
			POP EAX
			PUSHAD
		}
		Ai_BuildSupplies(town);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	//Ai_GetDefaultWorkerFromRace
	void __declspec(naked) Ai_GetDefaultWorkerFromRace() {
		static u32 unit_id;
		static u32 race;
		__asm {
			MOV race,EAX
			PUSHAD
		}
		unit_id = get_worker_from_race(race);
		__asm {
			POPAD
			MOV EAX,unit_id
			RETN
		}
	}

	const u32 jmp_33F7E = 0x00433F7E;
	const u32 jmp_33FD1_ret = 0x00433FD1;
//	const u32 jmp_33F7A = 0x00433F7A;
	const u32 jmp_33F77 = 0x00433F77;
	void __declspec(naked) AddUnitAi_JmpWrapper() {
		static u8 race;
		static u16 unit_id;//not in original function
		static u32 cmp_id;
		static CUnit* unit;
		__asm {
			MOV race,AL
			MOV unit,EBX
			MOV unit_id,CX
			MOVZX ECX,CX
			PUSHAD
		}
		cmp_id = get_townhall_from_race(race);
		if (unit_is_correct_townhall(unit_id, cmp_id, unit)) {
			__asm {
				POPAD
				JMP jmp_33FD1_ret
			}
		}
		else {
			__asm {
				POPAD
				JMP jmp_33F7E
			}
		}
		/*if (unit_id == UnitId::tactical_center) {
			__asm {
				POPAD
				MOV EAX, 0x6a
				MOV ECX, 0x6a
				JMP jmp_33F77
			}
		}
		else {*/

		/*
			__asm {
				POPAD
				MOV EAX, cmp_id
				JMP jmp_33F77
			}*/

		//}
		
	}
	const u32 jmp_3660D = 0x0043660D;
	void __declspec(naked) Ai_UpgradesFinished_JmpWrapper() {
		static u8 race;
		static u32 unit_id;
		__asm {
			MOV race, AL
			PUSHAD
		}
		unit_id = get_townhall_from_race(race);
		if (unit_id == 0xe4) {
			unit_id = UnitId::nexus;
		}
		__asm {
			POPAD
			MOV EAX, unit_id
			JMP jmp_3660D
		}
	}


	const u32 jmp_38f8 = 0x004438F8;
	const u32 jmp_3889 = 0x00443889;
	void __declspec(naked) Ai_RemoveDangerousRegions_JmpWrapper() {
		static u32 unit_id;
		__asm {
			MOV unit_id, EAX
			PUSHAD
		}
		if (is_townhall(unit_id)) {
			__asm {
				POPAD
				JMP jmp_38f8
			}
		}
		else {
			__asm {
				POPAD
				JMP jmp_3889
			}
		}
	}
	const u32 jmp_27a = 0x0044627A;
	const u32 jmp_240 = 0x00446240;
	void __declspec(naked) Ai_GetBuildLocation_JmpWrapper() {
		static u32 unit_id;
		__asm {
			MOV unit_id, EBX
			PUSHAD
		}
		if (is_townhall(unit_id)) {
			__asm {
				POPAD
				JMP jmp_27a
			}
		}
		else {
			__asm {
				POPAD
				JMP jmp_240
			}
		}
	}
	const u32 jmp_920 = 0x00434920;
	const u32 jmp_c80 = 0x00434C80;
	void __declspec(naked) Ai_TryProgressSpendingQueue_WorkerJmpWrapper() {
		static u16 unit_id;
		static CUnit* worker;
		__asm {
			MOV worker,EDI
			PUSH EAX
			MOV AX,[ECX+0x68FF02]
			MOV unit_id,AX
			POP EAX
			PUSHAD
		}
		if (is_terran_supply(unit_id,worker)) {
			__asm {
				POPAD
				JMP jmp_920
			}

		}
		else {
			__asm {
				POPAD
				JMP jmp_c80
			}
		}
	}
	void __declspec(naked) AI_AddTownSpendingRequest_Wrapper() { // 00448400
		static s32 result;
		static s32 playerId;
		static s32 priority;
		static s32 unitId;
		static void* param;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV param, EAX
			POP EAX
			MOV unitId, ESI
			MOV priority, EDI
			MOV playerId, EAX
			PUSHAD
		}
		result = hooks::AI_AddTownSpendingRequest(playerId, priority, unitId, param);
		__asm {
			POPAD
			MOV result, EAX
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}
	void injectWorkerHooks() {
		jmpPatch(Ai_ProgressSpendingQueue_JmpWrapper, 0x00434C31);
		jmpPatch(Ai_BuildSupplies_Wrapper, 0x00433730);
		jmpPatch(Ai_GetDefaultWorkerFromRace, 0x00432130);
	}
	void injectTownhallHooks() {
		jmpPatch(AddUnitAi_JmpWrapper, 0x00433F60, 4);
		jmpPatch(Ai_UpgradesFinished_JmpWrapper, 0x004365F6, 4);
		jmpPatch(Ai_RemoveDangerousRegions_JmpWrapper, 0x00443876);
		jmpPatch(Ai_GetBuildLocation_JmpWrapper, 0x0044622B);
		jmpPatch(AI_AddTownSpendingRequest_Wrapper, 0x00448400, 0);
	}
	void injectSupplyDepotHooks() {
		jmpPatch(Ai_TryProgressSpendingQueue_WorkerJmpWrapper, 0x00434912, 3);
	}
	const u32 jmp_00467F24 = 0x00467F24;
	const u32 jmp_00467F2C = 0x00467F2C;
	void __declspec(naked) orders_SCVBuild2_Wrapper() {
		static u16 id;	
		__asm {
			MOV id,AX
			PUSHAD
		}
		if (hooks::is_gasholder_structure(id)) {
			__asm {
				POPAD
				JMP jmp_00467F24
			}
		}
		else {
			__asm {
				POPAD
				JMP jmp_00467F2C
			}
		}
	}
	void __declspec(naked) orders_isGeyserTopUnitHookWrapper() {
		static CUnit* unit;
		static u32 result;
		__asm {
			MOV unit,EAX
			PUSHAD
		}
		result = hooks::orders_isGeyserTopUnitHook(unit);
		__asm {
			POPAD
			MOV EAX,result
			RETN
		}
	}
	const u32 jmp_00468952 = 0x00468952;
	const u32 jmp_00468966 = 0x00468966;
	void __declspec(naked) orders_isGeyserTopUnitEx_Wrapper() {
		static u16 id;
		__asm {
			MOV id, AX
			PUSHAD
		}
		if (hooks::is_gasholder_structure(id)) {
			__asm {
				POPAD
				JMP jmp_00468952
			}
		}
		else {
			__asm {
				POPAD
				JMP jmp_00468966
			}
		}
	}
	void injectGasHooks() {
		jmpPatch(orders_SCVBuild2_Wrapper, 0x00467F12, 1);
		jmpPatch(orders_isGeyserTopUnitHookWrapper, 0x4688b0, 1);
		jmpPatch(orders_isGeyserTopUnitEx_Wrapper, 0x0046893C, 3);

		/*
		0046893C  |.  66:8B41 64    MOV AX,WORD PTR DS:[ECX+64]
00468940  |.  66:3D 6E00    CMP AX,6E
00468944  |.  74 0C         JE SHORT 00468952
00468946  |.  66:3D 9D00    CMP AX,9D
0046894A  |.  74 06         JE SHORT 00468952
0046894C  |.  66:3D 9500    CMP AX,95
00468950  |.  75 14         JNZ SHORT 00468966
		*/

	}
	void injectEconomicUnitHooks() {
		injectWorkerHooks();
		//town hall
		injectTownhallHooks();
		//injectSupplyDepotHooks();
		//besides Ai_SupplyDepot which is part of worker hooks, there's no other hardcode related to supply
		//add later:
		//supplies
		//00434912 | .  66:83B9 02FF6 > CMP WORD PTR DS : [ECX + 68FF02] , 6D
		//

		injectGasHooks();
		/*
		//REFINERY ONLY
		Line 74437: 004320DE  |.  B8 6E000000   MOV EAX,6E
		Line 75147: 0043271D  |.  B9 6E000000   MOV ECX,6E
		Line 76614: 004336F7  |.  BE 6E000000   MOV ESI,6E
		Line 108002: 00448438  |.  B8 6E000000   MOV EAX,6E
		Line 154535: 004680D0  |.  66:83F9 6E    CMP CX,6E

		//EXTRACTOR ONLY
		Line 139439: 0045DAD4  |.  66:817B 64 95>CMP WORD PTR DS:[EBX+64],95
		Line 140056: 0045E20E  |>  66:81FB 9500  CMP BX,95
		Line 212082: 0048E458  |.  66:81FB 9500  |CMP BX,95
		Line 212210: 0048E5B2   .  66:3D 9500    CMP AX,95

		//PROBE/REFINERY/EXTRACTOR
		Line 154392: 00467F18  |.  66:3D 9D00    CMP AX,9D// DONE
		Line 155209: 004688C3  |.  66:3D 9D00    CMP AX,9D// DONE
		Line 155272: 00468946  |.  66:3D 9D00    CMP AX,9D
		Line 155935: 0046901D   .  66:81F9 9D00  CMP CX,9D
		Line 172723: 00474526  |>  66:81F9 9D00  CMP CX,9D
		Line 183356: 0047AFE6  |.  66:3D 9D00    CMP AX,9D
		Line 211909: 0048E1EE  |.  66:81FB 9D00  CMP BX,9D
		Line 212080: 0048E451  |.  66:81FB 9D00  |CMP BX,9D
		Line 212208: 0048E5AC   .  66:3D 9D00    CMP AX,9D //?
		Line 212310: 0048E70A   .  66:3D 9D00    CMP AX,9D
		Line 212350: 0048E7A0  |.  66:3D 9D00    CMP AX,9D
		Line 339974: 004E4D58   .  66:81BC47 980>CMP WORD PTR DS:[EDI+EAX*2+98],9D
		Line 340011: 004E4DCE   .  66:81BC57 980>CMP WORD PTR DS:[EDI+EDX*2+98],9D
		Line 340029: 004E4E0D   .  66:81FE 9D00  CMP SI,9D
		Line 340059: 004E4E72   .  66:81FE 9D00  CMP SI,9D
		Line 340086: 004E4EBF   .  66:8178 64 9D>CMP WORD PTR DS:[EAX+64],9D



		*/

	}
}