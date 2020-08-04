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
	void __declspec(naked) AddUnitAi_JmpWrapper() {
		static u8 race;
		static u16 unit_id;
		static u32 cmp_id;
		__asm {
			MOV race,AL
			MOV unit_id,CX
			PUSHAD
		}
		cmp_id = get_townhall_from_race(race);
		if (unit_id == cmp_id) {
			__asm {
				POPAD
				MOV EAX,cmp_id
				MOVZX ECX,CX
				JMP jmp_33F7E
			}
		}
		else {
			__asm {
				POPAD
				MOV EAX, cmp_id
				MOVZX ECX, CX
				JMP jmp_33FD1_ret
			}
		}
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
		__asm {
			PUSH EAX
			MOV AX,[ECX+0x68FF02]
			MOV unit_id,AX
			POP EAX
			PUSHAD
		}
		if (is_terran_supply(unit_id)) {
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
	}
	void injectSupplyDepotHooks() {
		jmpPatch(Ai_TryProgressSpendingQueue_WorkerJmpWrapper, 0x00434912, 3);
	}
	void injectEconomicUnitHooks() {
		injectWorkerHooks();
		//town hall
		injectTownhallHooks();
		injectSupplyDepotHooks();//besides Ai_SupplyDepot which is part of worker hooks, there's no other hardcode related to supply
		//add later:
		//supplies
		//00434912 | .  66:83B9 02FF6 > CMP WORD PTR DS : [ECX + 68FF02] , 6D

		//
	}
}