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

		/*
		//probably is going to be hook instead
		void __declspec(naked) Ai_BuildSupplies_JmpWrapper() {
		}
		00433753  |.  3C 01         CMP AL,1
00433755  |.  8975 F4       MOV DWORD PTR SS:[EBP-C],ESI
00433758  |.  75 09         JNZ SHORT 00433763
0043375A  |.  C745 FC 6D000>MOV DWORD PTR SS:[EBP-4],6D
00433761  |.  EB 0D         JMP SHORT 00433770
00433763  |>  F6D8          NEG AL
00433765  |.  1BC0          SBB EAX,EAX
00433767  |.  83E0 72       AND EAX,72
0043376A  |.  83C0 2A       ADD EAX,2A
0043376D  |.  8945 FC       MOV DWORD PTR SS:[EBP-4],EAX
00433770  |>  8D0CF6        LEA ECX,DWORD PTR DS:[ESI+ESI*8]
00433773  |.  8A048D E9EE57>MOV AL,BYTE PTR DS:[ECX*4+57EEE9]
0043377A  |.  3C 01         CMP AL,1
0043377C  |.  75 07         JNZ SHORT 00433785
0043377E  |.  B8 07000000   MOV EAX,7
00433783  |.  EB 0A         JMP SHORT 0043378F
00433785  |>  F6D8          NEG AL
00433787  |.  1BC0          SBB EAX,EAX
00433789  |.  83E0 17       AND EAX,17
0043378C  |.  83C0 29       ADD EAX,29
0043378F  |>  57            PUSH EDI
00433790  |.  8BF8          MOV EDI,EAX
00433792  |.  E8 39520500   CALL 004889D0
		*/
	

	void injectWorkerHooks() {
		jmpPatch(Ai_ProgressSpendingQueue_JmpWrapper, 0x00434C31);
		//jmpPatch(Ai_BuildSupplies_JmpWrapper, 0x00433753);
	}
	void injectTownhallHooks() {

	}
	void injectEconomicUnitHooks() {
		//worker
		injectWorkerHooks();

		//worker code
		//0043378C, 0043214B
		//town hall
		injectTownhallHooks();
		//townhall code
		//00433f62, 0043A100, 004365f6, 00433EB5, 0044387B, 00446230, 

	}
}