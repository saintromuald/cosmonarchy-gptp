#include "maxhp.h"
#include "SCBW\api.h"

namespace hooks {
	void __declspec(naked) getMaxHpWrapper() {
		const u32 jmp = 0x0040140B;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, ECX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void __declspec(naked) isOnYellowHealthWrapper() {
		const u32 jmp = 0x0040227B;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, EDX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId) / 3 * 2;
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) isOnBurningHealthWrapper() {
		const u32 jmp = 0x004022CB;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, ECX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId) / 3;
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) getMaxHealthWrapper() {
		const u32 jmp = 0x00402731;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, EDX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) ssDrawHpEnergyWrapper() {
		const u32 jmp = 0x004263F7;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EDI
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) calculateBaseStrengthWrapper() {
		const u32 jmp = 0x00431291;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, ECX
			MOV unit, EAX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) aiRepairSomethingWrapper() {
		const u32 jmp = 0x00433CBA;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) aiStimIfNeededWrapper() {
		const u32 jmp = 0x00440037;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, CX
			MOV unit, EDI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) unkWireRandomDraw56730_Wrapper() {
		const u32 jmp = 0x0045673B;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, ECX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) unkWireRandomDraw56730_Wrapper() {
		const u32 jmp = 0x0045673B;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, ECX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) unkWireRandomDraw567C0_Wrapper() {
		const u32 jmp = 0x004567EE;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) doMedicWrapper() {
		const u32 jmp = 0x00463CCF;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, ECX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EDX, hp
			JMP jmp
		}
	}

	void _declspec(naked) getRepairInfoWrapper() {
		const u32 jmp = 0x00466D73;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, EBX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EDI, hp
			JMP jmp
		}
	}

	void _declspec(naked) setHpWrapper() {
		const u32 jmp = 0x00466D73;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) setHpWrapper() {
		const u32 jmp = 0x00466D73;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) DrawImage_HpBar_Wrapper() {
		const u32 jmp = 0x0047A87E;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EDI
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) DrawImage_HpBar_Wrapper2() {
		const u32 jmp = 0x0047A90D;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EDI
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) setBuildHpWrapper() {
		const u32 jmp = 0x0047A913;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, EDI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV ECX, hp
			JMP jmp
		}
	}

	void _declspec(naked) orderFollowWrapper() {
		const u32 jmp = 0x0047C814;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, ECX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) getUnitBulletDamageWrapper() {
		const u32 jmp = 0x0048AD44;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, ECX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) showFinishedUnitWrapper() {
		const u32 jmp = 0x0049ECC6;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, ECX
			MOV unit, EAX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EDX, hp
			JMP jmp
		}
	}

	void _declspec(naked) transformUnit_Main_Wrapper() {
		const u32 jmp = 0x0049F22F;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EDX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) transformUnit_Main_Wrapper2() {
		const u32 jmp = 0x0049F242;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EDX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV ECX, hp
			JMP jmp
		}
	}

	void _declspec(naked) unk_ai_subunit_Destructor_Wrapper() {
		const u32 jmp = 0x004A019E;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, EDI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EDX, hp
			JMP jmp
		}
	}

	void _declspec(naked) updateUnitStatsFinishBuilding_Wrapper() {
		const u32 jmp = 0x004A020A;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, EDI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV ECX, hp
			JMP jmp
		}
	}

	void _declspec(naked) modifyUnitHp_Wrapper() {
		const u32 jmp = 0x004C7B84;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV ECX, hp
			JMP jmp
		}
	}

	void _declspec(naked) updateUnitDamageOverlay_Wrapper() {
		const u32 jmp = 0x004E609D;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EDI, hp
			JMP jmp
		}
	}

	void _declspec(naked) ai_findTransport_Wrapper() {
		const u32 jmp = 0x004E6F8F;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, AX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) ai_findTransport_Wrapper() {
		const u32 jmp = 0x004E6F8F;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, AX
			MOV unit, ESI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP jmp
		}
	}

	void _declspec(naked) ConvertToHallucination_Wrapper() {
		const u32 jmp = 0x004F61EE;
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EDX
			MOV unit, EDI
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV ECX, hp
			JMP jmp
		}
	}

	void injectMaxHpHooks() {
		
		//required for Heatshield
		/*jmpPatch(getMaxHpWrapper, 0x00401404, 2);//done
		jmpPatch(isOnYellowHealthWrapper, 0x00402274, 2);//done										
		jmpPatch(isOnBurningHealthWrapper, 0x004022C4, 2);//done	
		jmpPatch(getMaxHealthWrapper, 0x0040272A, 2);//done	
		jmpPatch(ssDrawHpEnergyWrapper, 0x004263F0, 2);//done	
		jmpPatch(calculateBaseStrengthWrapper, 0x0043128A, 2);//done, registers may be wrong
		jmpPatch(aiRepairSomethingWrapper, 0x00433CB3, 2);//done
		jmpPatch(aiStimIfNeededWrapper, 0x00440030, 2);//done
		jmpPatch(unkWireRandomDraw56730_Wrapper, 0x00456734, 2);//done
		jmpPatch(unkWireRandomDraw567C0_Wrapper, 0x004567E7, 2);//done
		jmpPatch(startZergBuildingWrapper, 0x0045D392, 2);//MUL DWORD PTR DS:[EDX*4+662350]
		//add changes in building_morph, building morph hooks are required (0045D6A7, 0045D6A0)
		//add changes to nydus hooks nydus hooks are required (orders_Build5)
		jmpPatch(Ai_TryReturnHomeWrapper, 0x00462EEA, 2);////CMP EDX,DWORD PTR DS:[EAX*4+662350], wrapper is not be enough, hook may be needed
		jmpPatch(Ai_OrderHealWrapper, 0x00463584, 1);////CMP EAX,DWORD PTR DS:[EDI+662350], wrapper is not be enough, hook may be needed
		jmpPatch(doMedicWrapper, 0x00463CC8, 2);////done
		jmpPatch(getRepairInfoWrapper, 0x00466D6C, 2);////done
		jmpPatch(setHpWrapper, 0x0046734A, 2);////done
		//add changes to order_repair hook, it is requirement (0x004673ED, 004676BB)
		//add changes to orders_TerranBuildSelf, it is required (0x00467781, 0x004677C2, 0x0046780C, 0046785E) 
		jmpPatch(createGeyserUnitWrapper, 0x0046797E, 2);//MUL DWORD PTR DS:[ECX*4+662350]
		jmpPatch(canTargetOrderWrapper, 0x00474DF8, 1);//CMP EDX,DWORD PTR DS:[EAX+662350], hook may be required
		jmpPatch(canTargetOrderWrapper, 0x00474E56, 1);//CMP ECX,DWORD PTR DS:[EAX+662350], hook may be required
		jmpPatch(DrawImage_HpBar_Wrapper, 0x0047A877, 2);////done
		jmpPatch(DrawImage_HpBar_Wrapper2, 0x0047A906, 2);////done
		jmpPatch(DrawImage_HpBar_Wrapper3, 0x0047A877, 2);////duplicate address?
		jmpPatch(DrawImage_HpBar_Wrapper4, 0x0047A906, 2);////duplicate address?
		jmpPatch(setBuildHpWrapper, 0x0047B195, 2);////done
		jmpPatch(orderFollowWrapper, 0x0047C80D, 2);////done
		jmpPatch(getUnitBulletDamageWrapper, 0x0048AD3D, 2);////done
		jmpPatch(showFinishedUnitWrapper, 0x0049ECBF, 2);////done
		jmpPatch(transformUnit_Main_Wrapper, 0x0049F228, 2);////done
		jmpPatch(transformUnit_Main_Wrapper2, 0x0049F23B, 2);////done
		jmpPatch(unk_ai_subunit_Destructor_Wrapper, 0x004A0197, 2);////done
		jmpPatch(updateUnitStatsFinishBuilding_Wrapper, 0x004A0203, 2);////done
		jmpPatch(initializeUnitBase_Wrapper, 0x004A0521, 0);////MUL DWORD PTR DS:[ECX*4+662350]
		jmpPatch(modifyUnitHp_Wrapper, 0x004C7B7D, 2);////done
		jmpPatch(unk_ccc4e_Wrapper, 0x004CCC4E, 3);////IMUL ECX,DWORD PTR DS:[EAX*4+662350]
		jmpPatch(editUnitFlags_Wrapper, 0x004CD493, 3);////IMUL ECX,DWORD PTR DS:[EAX*4+662350]
		jmpPatch(updateUnitDamageOverlay_Wrapper, 0x004E6096, 2);//done
		jmpPatch(ai_findTransport_Wrapper, 0x004E6F88, 2);//done
		//infestation hooks required, 0x004EA624 (morph into infested)
		jmpPatch(unk_ebdc8_Wrapper, 0x004EBDC8, 1);//CMP EDX,DWORD PTR DS:[ECX+662350], hook may be required
		//update unitState hook must be enabled (0x004EC37C)
		jmpPatch(ConvertToHallucination_Wrapper, 0x004F61E7, 2);//done
		*/



		//List of shield hooks - required for Autovitality
		/*
	Line 2489: 0040271E  |.  0FB70C45 000E>MOVZX ECX,WORD PTR DS:[EAX*2+660E00]
	Line 56698: 00426418  |>  66:813C7D 000>CMP WORD PTR DS:[EDI*2+660E00],270F
	Line 56745: 004264B7  |.  0FB7144D 000E>MOVZX EDX,WORD PTR DS:[ECX*2+660E00]
	Line 72973: 0043127E  |.  0FB70C45 000E>MOVZX ECX,WORD PTR DS:[EAX*2+660E00]
	Line 73609: 0043186F  |.  0FB73475 000E>MOVZX ESI,WORD PTR DS:[ESI*2+660E00]
	Line 129042: 004566C6  |.  66:833C4D 000>CMP WORD PTR DS:[ECX*2+660E00],0
	Line 129046: 004566D9  |.  0FB70C45 000E>MOVZX ECX,WORD PTR DS:[EAX*2+660E00]
	Line 152261: 004666E9  |.  0FB7144D 000E>MOVZX EDX,WORD PTR DS:[ECX*2+660E00]
	Line 182693: 0047A958  |.  0FB70C7D 000E>MOVZX ECX,WORD PTR DS:[EDI*2+660E00]
	Line 184045: 0047B6B8  |.  0FB70C7D 000E>MOVZX ECX,WORD PTR DS:[EDI*2+660E00]
	Line 184047: 0047B6C1  |.  8D347D 000E66>LEA ESI,DWORD PTR DS:[EDI*2+660E00]
	Line 206962: 0048AD61  |>  0FB70C4D 000E>MOVZX ECX,WORD PTR DS:[ECX*2+660E00]
	Line 219467: 004934B9  |.  0FB7047D 000E>MOVZX EAX,WORD PTR DS:[EDI*2+660E00]
	Line 219488: 004934F7  |.  0FB7047D 000E>MOVZX EAX,WORD PTR DS:[EDI*2+660E00]
	Line 219524: 0049356B  |.  0FB70C55 000E>MOVZX ECX,WORD PTR DS:[EDX*2+660E00]
	Line 220346: 00493EDB   .  0FB70C45 000E>MOVZX ECX,WORD PTR DS:[EAX*2+660E00]
	Line 236966: 0049EDA4  |.  0FB70C45 000E>MOVZX ECX,WORD PTR DS:[EAX*2+660E00]
	Line 238692: 004A020D  |.  0FB71445 000E>MOVZX EDX,WORD PTR DS:[EAX*2+660E00]
	Line 284159: 004BF03D  |.  68 000E6600   PUSH 00660E00
	Line 284447: 004BF3AD  |.  B8 000E6600   MOV EAX,00660E00
	Line 293902: 004C5A47  |.  0FB70C45 000E>MOVZX ECX,WORD PTR DS:[EAX*2+660E00]
	Line 303298: 004CC1B7  |.  0FB70C45 000E>MOVZX ECX,WORD PTR DS:[EAX*2+660E00]
	Line 304996: 004CD4BB  |.  0FB70C55 000E>MOVZX ECX,WORD PTR DS:[EDX*2+660E00]
	Line 340325: 004E516A  |.  0FB71C5D 000E>MOVZX EBX,WORD PTR DS:[EBX*2+660E00]
	Line 340332: 004E5181  |.  0FB70455 000E>MOVZX EAX,WORD PTR DS:[EDX*2+660E00]
	Line 343285: 004E6FD4  |.  0FB7147D 000E>|MOVZX EDX,WORD PTR DS:[EDI*2+660E00]
	Line 349378: 004EB0FE   .  0FB70445 000E>MOVZX EAX,WORD PTR DS:[EAX*2+660E00]
	Line 351280: 004EC2D1   .  0FB70445 000E>MOVZX EAX,WORD PTR DS:[EAX*2+660E00]
		*/
	}
}