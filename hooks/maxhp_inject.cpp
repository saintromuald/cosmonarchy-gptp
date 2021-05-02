#include "maxhp.h"
#include "SCBW\api.h"
#include "hook_tools.h"

namespace hooks {
	const u32 jmp_0x0040140B = 0x0040140B;
	void __declspec(naked) getMaxHpWrapper() {
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
			JMP jmp_0x0040140B
		}
	}

	const u32 jmp_0x0040227B = 0x0040227B;
	void __declspec(naked) isOnYellowHealthWrapper() {
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
			JMP jmp_0x0040227B
		}
	}

	const u32 jmp_0x004022CB = 0x004022CB;
	void _declspec(naked) isOnBurningHealthWrapper() {
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
			JMP jmp_0x004022CB
		}
	}

	const u32 jmp_0x00402731 = 0x00402731;
	void _declspec(naked) getMaxHealthWrapper() {
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
			JMP jmp_0x00402731
		}
	}

	const u32 jmp_0x004263F7 = 0x004263F7;
	void _declspec(naked) ssDrawHpEnergyWrapper() {
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
			JMP jmp_0x004263F7
		}
	}

	const u32 jmp_0x00431291 = 0x00431291;
	void _declspec(naked) calculateBaseStrengthWrapper() {
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
			JMP jmp_0x00431291
		}
	}

	const u32 jmp_0x00433CBA = 0x00433CBA;
	void _declspec(naked) aiRepairSomethingWrapper() {
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
			JMP jmp_0x00433CBA
		}
	}

	const u32 jmp_0x00440037 = 0x00440037;
	void _declspec(naked) aiStimIfNeededWrapper() {
		static u16 id;
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
			JMP jmp_0x00440037
		}
	}

	const u32 jmp_0x0045673B = 0x0045673B;
	void _declspec(naked) unkWireRandomDraw56730_Wrapper() {
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
			JMP jmp_0x0045673B
		}
	}

	const u32 jmp_0x004567EE = 0x004567EE;
	void _declspec(naked) unkWireRandomDraw567C0_Wrapper() {
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
			JMP jmp_0x004567EE
		}
	}

	const u32 jmp_0x00463CCF = 0x00463CCF;
	void _declspec(naked) doMedicWrapper() {
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
			JMP jmp_0x00463CCF
		}
	}

	const u32 jmp_0x00466D73 = 0x00466D73;
	void _declspec(naked) getRepairInfoWrapper() {
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
			JMP jmp_0x00466D73
		}
	}

	const u32 jmp_0x00467351 = 0x00467351;
	void _declspec(naked) setHpWrapper() {
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
			JMP jmp_0x00467351
		}
	}

	const u32 jmp_0x0047A87E = 0x0047A87E;
	void _declspec(naked) DrawImage_HpBar_Wrapper() {
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
			JMP jmp_0x0047A87E
		}
	}

	const u32 jmp_0x0047A90D = 0x0047A90D;
	void _declspec(naked) DrawImage_HpBar_Wrapper2() {
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
			JMP jmp_0x0047A90D
		}
	}

	const u32 jmp_0x0047A913 = 0x0047A913;
	void _declspec(naked) setBuildHpWrapper() {
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
			JMP jmp_0x0047A913
		}
	}

	const u32 jmp_0x0047C814 = 0x0047C814;
	void _declspec(naked) orderFollowWrapper() {
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
			JMP jmp_0x0047C814
		}
	}

	const u32 jmp_0x0048AD44 = 0x0048AD44;
	void _declspec(naked) getUnitBulletDamageWrapper() {
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
			JMP jmp_0x0048AD44
		}
	}

	const u32 jmp_0x0049ECC6 = 0x0049ECC6;
	void _declspec(naked) showFinishedUnitWrapper() {
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
			JMP jmp_0x0049ECC6
		}
	}

	const u32 jmp_0x0049F22F = 0x0049F22F;
	void _declspec(naked) transformUnit_Main_Wrapper() {
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
			JMP jmp_0x0049F22F
		}
	}

	const u32 jmp_0x0049F242 = 0x0049F242;
	void _declspec(naked) transformUnit_Main_Wrapper2() {
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
			JMP jmp_0x0049F242
		}
	}

	const u32 jmp_0x004A019E = 0x004A019E;
	void _declspec(naked) unk_ai_subunit_Destructor_Wrapper() {
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
			JMP jmp_0x004A019E
		}
	}

	const u32 jmp_0x004A020A = 0x004A020A;
	void _declspec(naked) updateUnitStatsFinishBuilding_Wrapper() {
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
			JMP jmp_0x004A020A
		}
	}

	const u32 jmp_0x004C7B84 = 0x004C7B84;
	void _declspec(naked) modifyUnitHp_Wrapper() {
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
			JMP jmp_0x004C7B84
		}
	}

	const u32 jmp_0x004E609D = 0x004E609D;
	void _declspec(naked) updateUnitDamageOverlay_Wrapper() {
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
			JMP jmp_0x004E609D
		}
	}

	const u32 jmp_0x004E6F8F = 0x004E6F8F;
	void _declspec(naked) ai_findTransport_Wrapper() {
		static u16 id;
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
			JMP jmp_0x004E6F8F
		}
	}

	const u32 jmp_0x004F61EE = 0x004F61EE;
	void _declspec(naked) ConvertToHallucination_Wrapper() {
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
			JMP jmp_0x004F61EE
		}
	}

	void __declspec(naked) getTotalHitPointsPlusShields_Wrapper() {
		static CUnit* a1;
		static u32 result;
		__asm {
			MOV a1, EDX
			PUSHAD
		}
		result = hooks::getTotalHitPointsPlusShields(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) drawUnitInfo_Wrapper() {
		static BinDlg* a1;
		__asm {
			MOV a1, EAX
			PUSHAD
		}
		hooks::drawUnitInfo(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) drawSupplyUnitInfo_Wrapper() {
		static s32 a1;
		__asm {
			MOV a1, EAX
			PUSHAD
		}
		hooks::drawSupplyUnitInfo(a1);
		__asm {
			POPAD
			RETN
		}
	}



	void __declspec(naked) AI_TargetEnemyProc_Wrapper() {
		static CUnit* a1;
		static CUnit* a2;
		static bool result;
		__asm {
			MOV a1, ECX
			MOV a2, EDX
			PUSHAD
		}
		result = hooks::AI_TargetEnemyProc(a1, a2);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) drawBuildingInfo_Wrapper() {
		static void* a1;
		__asm {
			MOV a1, ECX
			PUSHAD
		}
		hooks::drawBuildingInfo(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) calcUnitStrength_Wrapper() {
		static s32 a1;
		static u8 a2;
		static s32 result;
		__asm {
			MOV a1, EAX
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV a2, AL
			POP EAX
			PUSHAD
		}
		result = hooks::calcUnitStrength(a1, a2);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) getUnitStrength_Wrapper() {
		static CUnit* a1;
		static s32 a2;
		static bool a3;
		static s32 result;
		__asm {
			MOV a1, EAX
			MOV a2, ECX
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			PUSH EAX
			MOV EAX, [EBP + 0x0C]
			MOV a3, AL
			POP EAX
			PUSHAD
		}
		result = hooks::getUnitStrengthHook(a1, a2, a3);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) AI_RestorationRequirementsProc_Wrapper() {
		static CUnit* a1;
		static CUnit* a2;
		static bool result;
		__asm {
			MOV a1, ECX
			MOV a2, EDX
			PUSHAD
		}
		result = hooks::AI_RestorationRequirementsProc(a1, a2);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) AI_EMPShieldRequirementsProc_Wrapper() {
		static CUnit* a1;
		static CUnit* a2;
		static bool result;
		__asm {
			MOV a1, ECX
			MOV a2, EDX
			PUSHAD
		}
		result = hooks::AI_EMPShieldRequirementsProc(a1, a2);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) attackOverlayAndNotify_Wrapper() {
		static CUnit* a1;
		static CUnit* a2;
		static u8 a3;
		static char a4;
		static char result;
		__asm {
			MOV a1, EAX
			MOV a2, ESI
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			PUSH EAX
			MOV EAX, [EBP + 0x0C]
			MOV a3, AL
			MOV EAX, [EBP + 0x10]
			MOV a4, AL
			POP EAX
			PUSHAD
		}
		result = hooks::attackOverlayAndNotify(a1, a2, a3, a4);
		__asm {
			POPAD
			MOVZX EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) drawImageHPBar_Wrapper() {
		static s32 screenX;
		static s32 screenY;
		static GrpFrame* pFrame;
		static Box32* grpRect;
		static s32 colorData;
		__asm {
			MOV screenX, ECX
			MOV screenY, EDX
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			PUSH EAX
			MOV EAX, [EBP + 0x0C]
			MOV pFrame, EAX
			MOV EAX, [EBP + 0x10]
			MOV grpRect, EAX
			MOV EAX, [EBP + 0x14]
			MOV colorData, EAX
			POP EAX
			PUSHAD
		}
		hooks::drawImageHPBar(screenX, screenY, pFrame, grpRect, colorData);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x14
		}
	}

	void __declspec(naked) setBuildShieldGain_Wrapper() {
		static CUnit* a1;
		__asm {
			MOV a1, EBX
			PUSHAD
		}
		hooks::setBuildShieldGain(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) getUnitBulletDamage_Wrapper() {
		static CUnit* victim;
		static CBullet* bullet;
		static s32 result;
		__asm {
			MOV victim, EAX
			MOV bullet, EDX
			PUSHAD
		}
		result = hooks::getUnitBulletDamage(victim, bullet);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) canCastSpell_Wrapper() {
		static CUnit* a1;
		static s32 result;
		__asm {
			MOV a1, EDI
			PUSHAD
		}
		result = hooks::canCastSpell(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) modifyUnitShields_Wrapper() {
		static s32 a1;
		static s32 a2;
		static s32 result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			PUSH EAX
			MOV EAX, [EBP + 0x04]
			MOV a1, EAX
			MOV EAX, [EBP + 0x08]
			MOV a2, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::modifyUnitShields(a1, a2);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) compileHealthBar_Wrapper() {
		static s32 a1;
		static s32 a2;
		__asm {
			MOV a1, ESI
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV a2, EAX
			POP EAX
			PUSHAD
		}
		hooks::compileHealthBar(a1, a2);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) AI_FindTransport_Wrapper() {
		static CUnit* a1;
		static s32 a2;
		static s32 result;
		__asm {
			MOV a1, EBX
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV a2, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::AI_FindTransport(a1, a2);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) mapDataTransfer_Wrapper() {
		static char a1;
		static s32 a2;
		static s32 a3;
		static s32 result;
		__asm {
			MOV a1, CL
			MOV a2, EDX
			MOV a3, EAX
			PUSHAD
		}
		result = hooks::mapDataTransfer(a1, a2, a3);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void injectMaxHpHooks() {
		//		jmpPatch(getTotalHitPointsPlusShields_Wrapper, 0x00402710, 5);
		//		jmpPatch(drawUnitInfo_Wrapper, 0x00426190, 3);
		//		jmpPatch(drawSupplyUnitInfo_Wrapper, 0x00427540, 1);
		//		jmpPatch(AI_TargetEnemyProc_Wrapper, 0x00440A20, 2);
		//		jmpPatch(drawBuildingInfo_Wrapper, 0x00427890, 3);
		//		jmpPatch(calcUnitStrength_Wrapper, 0x00431270, 5);
		//		jmpPatch(getUnitStrength_Wrapper, 0x00431800, 0);
		//		jmpPatch(AI_RestorationRequirementsProc_Wrapper, 0x004407E0, 1);
		//		jmpPatch(AI_EMPShieldRequirementsProc_Wrapper, 0x004413D0, 1);
		//		jmpPatch(attackOverlayAndNotify_Wrapper, 0x00479730, 1);
		//		jmpPatch(drawImageHPBar_0_Wrapper, 0x0047A820, 1);
		//		jmpPatch(setBuildShieldGain_Wrapper, 0x0047B6A0, 0);
		//		jmpPatch(getUnitBulletDamage_Wrapper, 0x0048ACD0, 1);
		//		jmpPatch(canCastSpell_Wrapper, 0x00492140, 1);
		//		jmpPatch(modifyUnitShields_Wrapper, 0x004C5A20, 2);
		//		jmpPatch(compileHealthBar_Wrapper, 0x004D6010, 1);
		//		jmpPatch(AI_FindTransport_Wrapper, 0x004E6EF0, 1);
		//		jmpPatch(mapDataTransfer_Wrapper, 0x004EAFE0, 1);

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