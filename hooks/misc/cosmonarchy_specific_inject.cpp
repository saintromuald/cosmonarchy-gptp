#include "hooks/misc/cosmonarchy_specific.h"
#include "globals.h"
namespace hooks
{
	void __declspec(naked) getFlingyHaltDistanceHookWrapper()
	{
		static Flingy* flingy;
		static u32 result;
		__asm {
			MOV flingy, ECX
			PUSHAD
		}
		result = hooks::getFlingyHaltDistanceHook(flingy);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	const u32 jmp_00496390 = 0x00496390;
	void __declspec(naked) initializeFlingy_96377_JmpWrapper()
	{
		static u32 topSpeed;
		static u32 flingy_id;
		static u16 acceleration;
		static Flingy* flingy;
		__asm {
			PUSH EBX
			XOR EBX, EBX
			MOV[ESI + 0x3C], EBX
			POP EBX
			MOV flingy_id, EAX
			MOV flingy, ESI
			PUSHAD
		}
		topSpeed = hooks::realTopSpeed(flingy, flingy_id);
		acceleration = hooks::realAcceleration(flingy, flingy_id);
		globals.flingy_init_bullet_id = -1;
		__asm {
			POPAD
			MOV ECX, topSpeed
			MOV[ESI + 0x34], ECX
			MOV CX, acceleration
			JMP jmp_00496390
		}
	}

	const u32 jmp_8BEDE = 0x0048BEDE;
	void __declspec(naked) initializeBullet_8BED7_JmpWrapper()
	{
		static u32 weapon_id;
		static u32 player_id;
		static u32 result;
		__asm {
			MOV weapon_id, EAX
			MOV player_id, EBX
			PUSHAD
		}
		result = weapon_flingy(weapon_id, player_id);
		__asm {
			POPAD
			MOV EAX, result
			JMP jmp_8BEDE;
		}
	}

	void flingySpeedPatches() // for madrigals
	{
		jump_patch(getFlingyHaltDistanceHookWrapper, 0x00494F90);
		jump_patch(initializeFlingy_96377_JmpWrapper, 0x00496377, 2);
		jump_patch(initializeBullet_8BED7_JmpWrapper, 0x0048BED7, 2);
	}

	const u32 jmp_004295F8 = 0x004295F8;
	const u32 jmp_004295F3 = 0x004295F3;
	void __declspec(naked) wrapper_295EB()
	{
		static u16 id;
		__asm {
			MOV id, DI
			PUSHAD
		}
		if (!hooks::is_high_templar_conduit(id))
		{
			__asm {
				POPAD
				MOV AL, 0x6A
				JMP jmp_004295F8
			}
		}
		else
		{
			__asm {
				POPAD
				MOV AL, 0x6A
				JMP jmp_004295F3
			}
		}

	}

	const u32 func_getTargetSomething = 0x0043FF90;
	const u32 progressOrderSwitch_e71b = 0x004EC71B;
	const u32 spellOrderCases = 0x004ECADB;
	void __declspec(naked) unusedOrderPatchWrapper()
	{

		static u8 orderId;
		__asm {
			MOV[ESI + 0x85], 0x8
			CALL func_getTargetSomething
			MOV AL, [ESI + 0x4D]
			MOV orderId, AL
			MOVZX EAX, AL
			PUSHAD
		}
		if (hooks::is_unused_order(orderId))
		{
			__asm {
				POPAD
				JMP spellOrderCases
			}
		}
		else
		{
			__asm {
				POPAD
				JMP progressOrderSwitch_e71b
			}
		}
	}

	const u32 jmp_004A0682 = 0x004A0682;
	void __declspec(naked) lobotomyMineJmpWrapper()
	{
		Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		setupSpiderMines(unit);
		__asm {
			POPAD
			JMP jmp_004A0682
		}

	}

	const u32 jmp_0046512B = 0x0046512B;
	void orders_PlaceMine_JmpWrapper()
	{
		Unit* unit;
		u32 result;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::get_mine_id(unit);
		__asm {
			POPAD
			MOV EAX, result
			JMP jmp_0046512B
		}
	}

	const u32 jmp_0046514A = 0x0046514A;
	void orders_PlaceMine_JmpWrapper2()
	{
		Unit* unit;
		u32 result;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::get_mine_id(unit);
		__asm {
			POPAD
			MOV ECX, result
			JMP jmp_0046514A
		}
	}

	void unusedOrderPatches()
	{
		jump_patch(unusedOrderPatchWrapper, 0x004EC70B, 7); // turns unused orders to spell orders
		// later add jump patches to make CTFCop2 and WarpIn useless
	}

	void archonHardcodePatches() // for augurs
	{
		jump_patch(wrapper_295EB, 0x004295EB, 1);
	}

	void lobotomyMinePatches()
	{

		jump_patch(lobotomyMineJmpWrapper, 0x004A066B, 3);  /*
		jump_patch(orders_PlaceMine_JmpWrapper, 0x00465126, 0);
		jump_patch(orders_PlaceMine_JmpWrapper2, 0x00465145, 0);
		nops(0x00465088, 8); // remove tech requirement for any mines */
	}

	const u32 func_techTargetSpellCheck = 0x00491E80;
	const u32 jmp_004920E3 = 0x004920E3;
	void __declspec(naked) getOrderTargetErrorString_JmpWrapper()
	{
		static Unit* caster;
		static u8 playerId;
		static u32 techId;
		static Unit* target;
		static u32 func_result;
		static u32 extended_result;
		__asm {
			MOV caster, EAX
			MOV BL, [EAX + 0x4c]
			MOV EAX, EDI
			MOV target, EAX
			MOV playerId, BL
			CALL func_techTargetSpellCheck
			MOV func_result, EAX
			PUSHAD
		}
		extended_result = hooks::extend_tech_target_check(func_result, caster, target, techId);
		__asm {
			POPAD
			MOV EAX, extended_result
			JMP jmp_004920E3
		}
	}

	const u32 jmp_004922E1 = 0x004922E1;
	void __declspec(naked) ShouldStopOrderedSpell_JmpWrapper()
	{
		static Unit* caster;
		static u8 playerId;
		static u32 techId;
		static Unit* target;
		static u32 func_result;
		static u32 extended_result;
		__asm {
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV techId, EAX
			POP EAX
			MOV playerId, BL
			MOV target, EAX
			MOV caster, EDI
			CALL func_techTargetSpellCheck
			MOV func_result, EAX
			PUSHAD
		}
		extended_result = hooks::extend_tech_target_check(func_result, caster, target, techId);
		__asm {
			POPAD
			MOV EAX, extended_result
			JMP jmp_004922E1
		}
	}

	void lobotomyMineTargetingPatches()
	{
		jump_patch(getOrderTargetErrorString_JmpWrapper, 0x004920D9);
		jump_patch(ShouldStopOrderedSpell_JmpWrapper, 0x004922DC);
	}

	void trapJumpWrappers()
	{
		memory_patch(0x00401438, (u8)0xEB); // change to jump
		u8 patchb030[] = {0x33, 0xc0, 0x90, 0x90};
		memory_patch(0x0047B030, patchb030); // xor eax, eax; nop; nop
		memory_patch(0x0047B070, patchb030); // xor eax, eax; nop; nop
		
		memory_patch(0x004EBE99, (u8)0xEB);
		memory_patch(0x004A4B4D, (u8)0xEB);
		memory_patch(0x004A491B, (u8)0xEB);
		memory_patch(0x004A4829, (u8)0xEB);
		memory_patch(0x004A46CA, (u8)0xEB);
		memory_patch(0x004A0307, (u8)0xEB);
		memory_patch(0x004A3E04, (u8)0xEB);
		
		u8 jmpp[] = {0xEB, 0x3d};
		memory_patch(0x0049FAEF, jmpp);

//		memoryPatch(0x0049FAEF, (u8)0x75);//JNZ
		
		memory_patch(0x0047C05D + 2, (u8)0xe4); // floor gun trap
		memory_patch(0x0047C069 + 2, (u8)0xe4); // floor missile trap


		u8 jmpp2[] = {0xeb,0x2c};
		memory_patch(0x0047BEE7, jmpp2); // JNZ

		memory_patch(0x0047B516, patchb030);
		u8 jmpToEnd[] = {0xC3,0x90,0x90,0x90}; // RETN, NOP (3)
		memory_patch(0x0047B556, jmpToEnd);
		
		memory_patch(0x0047B7B1 + 2, (u8)0xE4);
		nops(0x0047B7B7, 6);
		
		// 0047BDCB
		u8 jmpp3[] = {0xeb,0x25};
		memory_patch(0x0047BDCB, jmpp3); // JNZ
	}

	const u32 progressOrder_jmp = 0x004EC6F1;
	void __declspec(naked) PhaseLinkOrder_Wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		hooks::order_PhaseLink(unit);
		__asm {
			POPAD
			JMP progressOrder_jmp
		}
	}

	void phaseLinkPatches()
	{
		jump_patch(PhaseLinkOrder_Wrapper, 0x004EC629, 2);
	}

	void __declspec(naked) canAttackTarget_Tail_Wrapper()
	{
		static u32 target_flags;
		static Unit* source;
		static u32 result;
		__asm {
			PUSH EAX
			MOV EAX, [EBP - 0x4]
			MOV target_flags, EAX
			POP EAX
			MOV source, EDI
			PUSHAD
		}
		result = hooks::canAttackTarget_EndChunk(source, target_flags);
		__asm {
			POPAD
			MOV EAX, result
			POP EDI
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) getGroundWeaponHookWrapper()
	{
		static Unit* unit;
		static u32 result;
		__asm {
			MOV unit, ECX
			PUSHAD
		}
		result = unit->get_ground_weapon();
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void clarionWrappers()
	{
		// canAttackTarget - end
		// jump_patch(canAttackTarget_Tail_Wrapper, 0x0047682C, 1);
		// tail wrapper disabled, without air weapon it don't have sense anywhere, will be reworked w/ air target functionality
		jump_patch(getGroundWeaponHookWrapper, 0x00475AD0);
	}

	const u32 jmp_skip = 0x00404541;
	const u32 jmp_continue = 0x00404539;
	void __declspec(naked) createBuildingAiWorkaround_Wrapper()
	{
		static BuildingAi* bAi;
		__asm {
			MOV EDX, [ECX + 0xABE0]
			MOV bAi, EDX
			PUSHAD
		}
		if (reinterpret_cast<u32>(bAi) < 0x300000)
		{
			__asm {
				POPAD
				JMP jmp_skip
			}
		}
		else
		{
			__asm {
				POPAD
				JMP jmp_continue
			}
		}
	}

	void __declspec(naked) checkUnitDatRequirements_Wrapper()
	{
		static u32 result;
		static u32 unit_id;
		static Unit* parent;
		static u32 playerId;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit_id, EAX
			MOV parent, ESI
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV playerId, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::checkUnitDatRequirements(playerId, unit_id, parent);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
//		0046E1C0 = CheckUnitDatRequirements(), arg 1 player_id, eax unit_id, esi Unit* parent
	}

	void createBuildingAiWorkaround()
	{
		jump_patch(createBuildingAiWorkaround_Wrapper, 0x00404533, 1);
	}

	static bool warningOnce00 = false;
	const u32 Hook_DropDebugging00 = 0x0047CDDB;
	const u32 Hook_DropDebugging00Back = 0x0047CDE1;
	void __declspec(naked) dropDebugging00()
	{
		static char buffer[256];
		static u8 hashStructIndex;
		__asm {
			JNB short _Jmp
			JMP Hook_DropDebugging00Back
			_Jmp :
			PUSHAD
				MOV EBP, ESP
				MOV hashStructIndex, AL
		}
		if (!warningOnce00)
		{
			sprintf_s(buffer, "Invalid hashStructIndex!!\nhashStructIndex: %u", hashStructIndex);
			scbw::print(buffer, GameTextColor::Red);
			warningOnce00 = true;
		}
		__asm {
			POPAD
			XOR EAX, EAX
			POP EBX
			RETN
		}
	}

	static bool warningOnce01 = false;
	const u32 Hook_DropDebugging01 = 0x0047CCF8;
	void __declspec(naked) dropDebugging01()
	{
		static char buffer[256];
		static u32 activeNationID;

		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV activeNationID, EBX
		}

		if (!warningOnce01)
		{
			sprintf_s(buffer, "Invalid PlayerFlag!!\nactiveNationID: %u", activeNationID);
			scbw::print(buffer, GameTextColor::Red);
			warningOnce01 = true;
		}

		__asm {
			POPAD
			POP EDI
			XOR EAX, EAX
			POP EBX
			POP EBP
			RETN 4
		}
	}

	static bool warningOnce02 = false;
	const u32 Hook_DropDebugging02 = 0x0047CD63;
	void __declspec(naked) dropDebugging02()
	{
//		static char buffer[256];
		static bool isSucceeded;
		static u8 visibilityHash;
		static u8 nationHash;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV isSucceeded, AL
			MOV visibilityHash, CL
			MOV nationHash, DL
		}
		if (!isSucceeded && !warningOnce02)
		{
//			sprintf_s(buffer, "Invalid SpriteVisibility!!\nactiveNationID: %u", *ACTIVE_NATION_ID);
//			scbw::print(buffer, GameTextColor::Red);
			scbw::printf("Invalid SpriteVisibility!!\nactiveNationID: % u", *ACTIVE_NATION_ID);
			scbw::printf("SpriteVisHash (from VisionSync) %d =/= visionHash (from CMD) %d", nationHash, visibilityHash);
			warningOnce02 = true;
		}
		__asm {
			POPAD
//			MOV AL, 1 // disable desync status
			POP EBX
			POP EBP
			RETN 4
		}
	}

	struct hashStruct
	{
		u16 hash;
		u8 hashType;
		u8 byte[0x109];
	};

	static_assert(sizeof(hashStruct) == 0x10C, "Invalid hashStruct Size");
	hashStruct* hashList = (hashStruct*)0x0065EB30;
	
	static bool warningOnce03_00 = false;
	static bool warningOnce03_01 = false;
	const u32 Hook_DropDebugging03 = 0x0047CDF1;
	void __declspec(naked) dropDebugging03()
	{
		static char buffer[256];
		static u8* data;
		static u32 hashStructIndex;
		static u8 hashType;
		static bool isSucceeded;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV data, EDI
			MOV hashStructIndex, ESI
			MOV hashType, BL
		}
		isSucceeded = true;
		if (hashList[hashStructIndex].hash != *(u16*)(data + 2) && !warningOnce03_00)
		{
			isSucceeded = false;
			sprintf_s(buffer, "Invalid Hash!!\nhashStructIndex: %u\nHashType1 = %u\nHash1 = 0x%04x\nHashType2 = %u\nHash2 = 0x%04x",
				hashStructIndex,
				hashList[hashStructIndex].hashType,
				hashList[hashStructIndex].hash,
				hashType & 0x0F,
				*(u16*)(data + 2));
			scbw::print(buffer, GameTextColor::Red);
			warningOnce03_00 = true;
		}
		if (hashList[hashStructIndex].hashType != (hashType & 0x0F) && warningOnce03_01)
		{
			isSucceeded = false;
			sprintf_s(buffer, "Invalid HashType!!\nhashStructIndex: %u\nHashType = %u", hashStructIndex, hashList[hashStructIndex].hashType);
			scbw::print(buffer, GameTextColor::Red);
			warningOnce03_01 = true;
		}
		if (isSucceeded)
		{
			__asm {
				POPAD
				POP ESI
				MOV EAX, 1
				POP EBX
				RETN
			}
		}
		else
		{
			__asm {
				POPAD
				POP ESI
				XOR EAX, EAX
				POP EBX
				RETN
			}
		}
	}

	void desyncTesting()
	{
		// code from BG
		jump_patch(dropDebugging00, Hook_DropDebugging00);
		jump_patch(dropDebugging01, Hook_DropDebugging01);
		jump_patch(dropDebugging02, Hook_DropDebugging02);
		jump_patch(dropDebugging03, Hook_DropDebugging03);
	}

	void __declspec(naked) createDamageOverlay_Wrapper() // 004993C0
	{
		static Sprite* sprite;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV sprite, EAX
			POP EAX
			PUSHAD
		}
		hooks::createDamageOverlay(sprite);
		__asm
		{
			POPAD
//			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
	const u32 DmgOverlay_JmpBack = 0x004993E3;
	const u32 DmgOverlay_JmpEnd = 0x0049956E;
	void __declspec(naked) newDamageOverlay_JmpWrapper(){//
		static Sprite* sprite;
		static u32 cmp1, cmp2;
		__asm {
			MOV sprite,EDX
			MOV cmp1,ESI
			MOV cmp2,EDI
			PUSHAD
		}
		if (debugDamageOverlay(sprite) && cmp1!=cmp2) {
			__asm {
				POPAD
				JMP DmgOverlay_JmpBack
			}
		}
		else {
			__asm {
				POPAD
				JMP DmgOverlay_JmpEnd
			}
		}	
	}

	void damageOverlayDebug()
	{
//		jump_patch(createDamageOverlay_Wrapper, 0x004993C0);
		jump_patch(newDamageOverlay_JmpWrapper, 0x004993DB, 3);
	}

	void __declspec(naked) gameLoop_Wrapper()
	{
		static s32 result;
		__asm
		{
			PUSHAD
		}
		result = hooks::gameLoop();
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) createUnit_Wrapper() // 004A09D0
	{
		static Unit* result;
		static u16 unitType;
		static s32 x;
		static s32 y;
		static s32 playerId;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV y, EAX
			MOV EAX, [EBP + 0xC]
			MOV playerId, EAX
			POP EAX
			MOV x, EAX
			MOV unitType, CX
			PUSHAD
		}
		result = hooks::create_unit(unitType, x, y, playerId);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) transform_unit_main_Wrapper() { // 0049F170
		static Unit* unit;
		static u8 unit_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit_id, AL
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		hooks::transform_unit_main(unit, unit_id);
		__asm {
			POPAD
			MOV EAX, 0
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) ignoreIscriptGoToIdle_Wrapper() // 00476550
	{
		static Unit* unit;
		static char result;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::ignoreIscriptGoToIdle(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void injectCosmonarchySpecific()
	{
		// must be finished first (and add Firegraft offsets)
//		jump_patch(checkUnitDatRequirements_Wrapper, 0x0046E1C0, 1);

		flingySpeedPatches();
		desyncTesting();
		// start of spell order code
		// default case of switch 004EC4FF
		unusedOrderPatches();
		phaseLinkPatches(); // replaces CTFCop2 order
		lobotomyMinePatches();
		trapJumpWrappers();
		clarionWrappers();
		createBuildingAiWorkaround();
		damageOverlayDebug();
//		lobotomyMineTargetingPatches();

		// add later: disable WarpIn functionality, make it useless
		archonHardcodePatches();

		// Hook patches!
//      jump_patch(gameLoop_Wrapper,				0x004D94B0, 3);
		jump_patch(createUnit_Wrapper,				0x004A09D0, 1);
		jump_patch(transform_unit_main_Wrapper,		0x0049F170, 1);
		jump_patch(ignoreIscriptGoToIdle_Wrapper,	0x00476550, 1);
	}
}

// TODO: Cleanup
		/*

00431B26  |.  0F95C1        |SETNE CL
00431B29  |.  8BC1          |MOV EAX,ECX
00431B2B  |>  85C0          |TEST EAX,EAX
00431B2D  |.  0F84 70010000 |JE 00431CA3
		*/

		/*

0043AFCD  |>  8BCB          |MOV ECX,EBX
0043AFCF  |.  E8 FCAA0300   |CALL 00475AD0
0043AFD4  |>  3C 82         |CMP AL,82
0043AFD6  |.  74 23         |JE SHORT 0043AFFB
		*/

		/*

004418D9   .  E8 F2410300   CALL 00475AD0
		*/

		/*

00441900   .  E8 CB410300   CALL 00475AD0
00441905   .  3C 82         CMP AL,82
		*/

		/*

	Line 154324: 00467E37  |.  E8 94DC0000   CALL 00475AD0
	Line 175187: 00475AD0  /$  66:8B41 64    MOV AX,WORD PTR DS:[ECX+64]
	Line 175887: 004761F1  |>  E8 DAF8FFFF   CALL 00475AD0
	Line 176527: 0047684D  |.  E8 7EF2FFFF   CALL 00475AD0
	Line 178261: 00477AE8  |.  E8 E3DFFFFF   CALL 00475AD0
	Line 178537: 00477E36  |.  E8 95DCFFFF   CALL 00475AD0
	Line 179279: 00478640  |.  E8 8BD4FFFF   CALL 00475AD0
	Line 179802: 00478BC4  |.  E8 07CFFFFF   CALL 00475AD0
	Line 207274: 0048B0D1   .  E8 FAA9FEFF   CALL 00475AD0
	Line 321514: 004D818F  |.  E8 3CD9F9FF   |CALL 00475AD0
	Line 344811: 004E8076  |.  E8 55DAF8FF   CALL 00475AD0
		*/