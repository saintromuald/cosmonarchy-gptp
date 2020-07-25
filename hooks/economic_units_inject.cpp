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
	void injectWorkerHooks() {
		jmpPatch(Ai_ProgressSpendingQueue_JmpWrapper, 0x00434C31);
		jmpPatch(Ai_BuildSupplies_Wrapper, 0x00433730);
		jmpPatch(Ai_GetDefaultWorkerFromRace, 0x00432130);
	}
	void injectTownhallHooks() {

	}
	void injectEconomicUnitHooks() {
		//worker
		injectWorkerHooks();

		//worker code
		//0043214B

		//town hall
		injectTownhallHooks();
		//00433f62, 0043A100, 004365f6, 00433EB5, 0044387B, 00446230, 

	}
}