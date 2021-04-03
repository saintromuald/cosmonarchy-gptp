#include "hooks/limits/extender.h"
#include "hooks/limits/ccmu.h"
#include "hooks/limits/sfx_extender.h"
#include "hooks/limits/tbl_extender.h"
#include "hooks/limits/misc_extenders.h"
#include "hooks/limits/fake_unitid_extender.h"
#include "hooks/limits/attack_extender.h"
#include "logger.h"
#include "globals.h"
#include <cassert>
namespace hooks {
	void loadAiseSamaseData() {
		hooks::sfxExtenderGetCount();
		hooks::unitsDatWrapperPatches();
	}


	void AI_RemoveFromAttack(int32 unitId_, uint32 player) {
		int unitId = unitId_;
		if (unitId == UnitId::TerranPhalanxSiegeMode)
			unitId = UnitId::TerranPhalanxTankMode;
		unitId++;
		for (int i = 0; i < ATTACK_LIMIT; i++) {
/*			if (AIScriptController[player].attackers[i] == unitId) {
				AIScriptController[player].attackers[i] = UnitId::None + 1;*/
			if(Globals.attackers.attacks[player][i] == unitId){
				Globals.attackers.attacks[player][i] = UnitId::None + 1;
				return;
			}
		}
	}

	const uint32 jmp_004DAF36 = 0x004DAF36;
	void __declspec(naked) preInitDataWrapper() {
		__asm {
			PUSHAD
		}
		loadAiseSamaseData();
		__asm {
			POPAD
			PUSH 0x004D2700
			JMP jmp_004DAF36
		}
	}
	void __declspec(naked) FakeUnitFlingyIdState_Wrapper() {
		//jmpPatch(FakeUnitFlingyIdState_Wrapper, 0x004AAF23);
		static uint32 flingyId;
		static uint32 index;
		__asm {
			MOV index,ECX
			PUSHAD
		}
		flingyId = fakeExtFlingyIds[index];
		__asm {
			POPAD
			MOV EAX,flingyId
			RETN
		}
	}
	void fraudFlingyIdCopy() {
		for (int i = 0; i < 228; i++) {
			fakeExtFlingyIds[i] = units_dat::Graphic[i];
		}
	}
	void __declspec(naked) psychosisFlingyId_Wrapper() {
		__asm {
			PUSHAD
		}
		fraudFlingyIdCopy();
		__asm {
			POPAD
			RETN
		}
	}
	void set_flingyid_value(uint32 index, uint32 value) {
		fakeExtFlingyIds[index] = value;
	}
	void __declspec(naked) madnessSetFlingyId_Wrapper(){
		static uint32 index;
		static uint32 value;
		__asm {
			MOV index,EAX
			MOV value,ECX
			PUSHAD
		}
		set_flingyid_value(index, value);
		__asm {
			POPAD
			RETN
		}
	}
	uint32 buildingVisionSyncHook() {
		int result = 0;
		uint8* visHash = (uint8*)0x0063fd30;
		uint32* bWantVisibilityHashUpdate = (uint32*)0x00629d90;
		uint8* regionVisionHash = (uint8*)0x0063fe38;
		int* nCurrentHashOffset = (int*)0x0063fd28;
		//
		memset(visHash, 0, 64);
		*bWantVisibilityHashUpdate = 0;
		*regionVisionHash = 0;
		*nCurrentHashOffset = 0;
		int i = 0;
		do {
			if (!(units_dat::BaseProperty[i] & UnitProperty::Building)) {
				if (units_dat::StarEditAvailFlags[i] & 0x1) {
					auto flingy_id = fakeExtFlingyIds[i];
					auto sprite_id = scbw::getFlingyData(FlingyDatEntries::SpriteId, flingy_id);
					scbw::setSpriteData(SpritesDatEntries::IncludeInVision, sprite_id, 1);
				}
			}
			i++;
		} while (i < 228);
		return i;
	}
	void __declspec(naked) buildingVisionSyncHook_Wrapper() {
		static uint32 result;
		__asm {
			PUSHAD
		}
		result = buildingVisionSyncHook();
		__asm {
			POPAD
			MOV EAX,result
			RETN
		}

	}
	void dumbCodeInject(uint32 address, uint32* ids, std::vector<uint8> code, uint32 nop) {
		memoryPatch(address + code.size(), ids);
		for (int i = 0; i < code.size(); i++) {
			memoryPatch(address + i, code[i]);
		}
		nops(address + code.size() + 4, nop);
	}
	void insaneFlingyIdExtender() {
		jmpPatch(FakeUnitFlingyIdState_Wrapper, 0x004AAFE3);	
		jmpPatch(psychosisFlingyId_Wrapper, 0x004AB045);
		jmpPatch(madnessSetFlingyId_Wrapper, 0x004AB147);

		std::vector<uint8> eax_eax = { 0x8b, 0x04, 0x85 };
		dumbCodeInject(0x00454317, fakeExtFlingyIds, eax_eax, 0);//+
		std::vector<uint8> ecx_edi = { 0x8b, 0x0c, 0xbd };
		dumbCodeInject(0x00463A71, fakeExtFlingyIds, ecx_edi, 0);
		std::vector<uint8> edx_ecx = { 0x8b, 0x14, 0x8d };
	//	dumbCodeInject(0x004683C6, fakeExtFlingyIds, edx_ecx, 2);//unit_morph - AI Wrapper
		std::vector<uint8> edx_eax = { 0x8b, 0x14, 0x85 };
		//dumbCodeInject(0x0047B854, fakeExtFlingyIds, edx_eax, 2);
		//dumbCodeInject(0x0047B8A4, fakeExtFlingyIds, edx_eax, 2);
		//wrappers 3-5 if gptp speed hooks are enabled
		std::vector<uint8> ecx_eax = { 0x8b, 0x0c, 0x85 };
		dumbCodeInject(0x0047B8F4, fakeExtFlingyIds, ecx_eax, 0);
		dumbCodeInject(0x0048D756, fakeExtFlingyIds, ecx_eax, 0);
		dumbCodeInject(0x0048DA8A, fakeExtFlingyIds, ecx_eax, 0);
		//building visibility
//		jmpPatch(buildingVisionSyncHook_Wrapper, 0x00497110);
		
		dumbCodeInject(0x00497148, fakeExtFlingyIds, edx_eax, 0);
		dumbCodeInject(0x0049716F, fakeExtFlingyIds+1, edx_eax, 0);
		dumbCodeInject(0x00497196, fakeExtFlingyIds+2, edx_eax, 0);
		dumbCodeInject(0x004971BD, fakeExtFlingyIds+3, edx_eax, 0);
		dumbCodeInject(0x004971E4, fakeExtFlingyIds+4, edx_eax, 0);
		dumbCodeInject(0x0049720B, fakeExtFlingyIds+5, edx_eax, 0);
		//
		std::vector<uint8> eax_ebx = { 0x8b, 0x04, 0x9d };
		dumbCodeInject(0x0049ED0B, fakeExtFlingyIds, eax_ebx, 0);//crashes hexrays
		std::vector<uint8> eax_edx = { 0x8b, 0x04, 0x95 };
		dumbCodeInject(0x004E9A10, fakeExtFlingyIds, eax_edx, 0);
		
		
	}


	void __declspec(naked) GetAttackUnit_Wrapper() {
		static uint32 unitId;
		static uint32 playerId;
		static uint32 index;
		__asm {
			MOV playerId, ECX
			MOV index, EBX
			PUSHAD
		}
		assert(playerId < 8);
		assert(index < ATTACK_LIMIT);
		//unitId = AIScriptController[playerId].attackers[index];//is actually "unitId+1"
		unitId = Globals.attackers.attacks[playerId][index];
		__asm {
			POPAD
			MOV EAX, unitId
			RETN
		}
	}
	void __declspec(naked) GetAttackLength_Wrapper() {
		__asm {
			PUSHAD
		}
		__asm {
			POPAD
			MOV EAX, 0x3F
			RETN
		}
	}
	void __declspec(naked) SetAttackUnit_Wrapper() {
		static uint32 unitId;
		static uint32 index;
		static uint32 playerId;
		__asm {
			MOV unitId, ECX
			MOV index, EBX
			MOV playerId, EDX
			PUSHAD
		}
		assert(playerId < 8);
		assert(index < ATTACK_LIMIT);//change later
//		AIScriptController[playerId].attackers[index] = unitId;//is actually "unitId+1"
		Globals.attackers.attacks[playerId][index] = unitId;
		__asm {
			POPAD
			RETN
		}
	}
	const uint32 GetMissingJmpWrapper = 0x00438067;

	void __declspec(naked) AI_GetMissingAttackForceUnits_JmpWrapper() {
		static uint32 playerId;
		static uint16* arrayStart;
		__asm {
			MOV playerId, ECX
			PUSHAD
		}
//		arrayStart = &AIScriptController[playerId].attackers[0];
		arrayStart = &Globals.attackers.attacks[playerId][0];
		__asm {
			POPAD
			MOV ECX,arrayStart
			JMP GetMissingJmpWrapper
		}
	}

	const uint32 AIAddToAttack_NullJmp = 0x00447274;
	const uint32 AIAddToAttack_Continue = 0x00447265;
	void __declspec(naked) AI_AddToAttack_JmpWrapper1() {
		static uint32 playerId;
		static uint32 arrayOffset;
		__asm {
			MOV playerId, ECX
			PUSHAD
		}
//		if (AIScriptController[playerId].attackers[0] == 0) {
		if(Globals.attackers.attacks[playerId][0] == 0){
			__asm {
				POPAD
				JMP AIAddToAttack_NullJmp
			}
		}
		else {
//			arrayOffset = reinterpret_cast<uint32>(&AIScriptController[playerId].attackers[0]);
			arrayOffset = reinterpret_cast<uint32>(&Globals.attackers.attacks[playerId][0]);
			__asm {
				POPAD
				MOV ECX, arrayOffset
				JMP AIAddToAttack_Continue
			}
		}
	}
	const uint32 AIAddAttackTo_LastJmp = 0x00447282;
	void __declspec(naked) AI_AddToAttack_JmpWrapper2() {
		static uint32 playerMul;
		static uint32 player;
		static uint32 index;
		static uint32 getUnit;
		__asm {
			MOV playerMul, EDX
			MOV index, EAX
			PUSHAD
		}
		assert(index < ATTACK_LIMIT);
		player = playerMul / (sizeof(AI_Main) / 2);
//		getUnit = AIScriptController[player].attackers[index];
		getUnit = Globals.attackers.attacks[player][index];
		__asm {
			POPAD
			MOV ECX, getUnit
			JMP AIAddAttackTo_LastJmp
		}
	}
	const uint32 AIAttackManager_Jmp = 0x0043AC61;
	void __declspec(naked) AI_AttackManager_JmpWrapper() {
		static uint32 offset;
		static uint32 playerId;
		static uint32 arrayOffset;
		static uint16 unitId;
		__asm {
			PUSH EAX
			MOV EAX,[EBP+0x08]
			MOV playerId,EAX
			POP EAX
			PUSHAD
		}
/*		unitId = AIScriptController[playerId].attackers[0];
		arrayOffset = reinterpret_cast<uint32>(AIScriptController[playerId].attackers);*/
		unitId = Globals.attackers.attacks[playerId][0];
		arrayOffset = reinterpret_cast<uint32>(&Globals.attackers.attacks[playerId][0]);
		__asm {
			POPAD
			MOV AX,unitId
			MOV ECX, arrayOffset
			JMP AIAttackManager_Jmp
		}
	}
	static uint32 AI_HasUnitsForAttack_Jmp = 0x00437F36;
	void __declspec(naked) AI_HasUnitsForAttackForce_JmpWrapper() {
		static uint32 playerId;
		static uint32 arrayOffset;
		__asm {
			MOV playerId,EAX
			PUSHAD
		}
//		arrayOffset = reinterpret_cast<uint32>(AIScriptController[playerId].attackers);
		arrayOffset = reinterpret_cast<uint32>(&Globals.attackers.attacks[playerId][0]);
		__asm {
			POPAD
			MOV EAX,arrayOffset
			JMP AI_HasUnitsForAttack_Jmp
		}
	}



	void __declspec(naked) AI_RemoveFromAttack_Wrapper() { // 00447040
		static uint32 unitId;
		static uint32 player;
		__asm {
			MOV unitId, EAX
			MOV player, EDX
			PUSHAD
		}
		hooks::AI_RemoveFromAttack(unitId, player);
		__asm {
			POPAD
			RETN
		}
	}
	const uint32 Jmp_SendSuicide = 0x0043e1f9;
	void __declspec(naked) AI_SendSuicide_JmpWrapper() {
		static uint32 playerId;
		static uint32 index;
		static uint32 arrayOffset;
		static uint32 size2;
		__asm {
			MOV EDX,index
			PUSHAD
		}
		playerId = index / sizeof(AI_Main);
		arrayOffset = reinterpret_cast<uint32>(&Globals.attackers.attacks[playerId][0]);
		size2 = ATTACK_LIMIT / 2;
		__asm {
			POPAD
			MOV EDI,arrayOffset
			MOV ECX,size2
			JMP Jmp_SendSuicide
		}
	}

	const uint32 Jmp_Unk1 = 0x00448179;
	void __declspec(naked) AI_Unk1_JmpWrapper() {
		static uint32 playerId;
		static uint32 index;
		static uint32 arrayOffset;
		static uint32 size2;
		__asm {
			MOV EDX, index
			PUSHAD
		}
		playerId = index / sizeof(AI_Main);
		arrayOffset = reinterpret_cast<uint32>(&Globals.attackers.attacks[playerId][0]);
		size2 = ATTACK_LIMIT / 2;
		__asm {
			POPAD
			MOV EDI, arrayOffset
			MOV ECX, size2
			JMP Jmp_Unk1
		}
	}

	void __declspec(naked) AI_Unk2_Wrapper() {
		static uint32 playerId;
		static uint32 arrayOffset;
		__asm {
			MOV EAX,playerId
			PUSHAD
		}
		arrayOffset = reinterpret_cast<uint32>(&Globals.attackers.attacks[playerId][0]);
		__asm {
			POPAD
			MOV EAX,arrayOffset
			RETN
		}
	}



	void newAttackExtender() {
		jmpPatch(GetAttackUnit_Wrapper, 0x004171E5);//null int3 bytes
		jmpPatch(GetAttackLength_Wrapper, 0x00417268);//null int3 bytes
		jmpPatch(SetAttackUnit_Wrapper, 0x00417207);//null int3 bytes
		//
		uint32 attackSize = (ATTACK_LIMIT * 4) + 8;
		memoryPatch(0x0043E673 + 2, attackSize);
		memoryPatch(0x0043EEC3 + 2, attackSize);
		jmpPatch(AI_GetMissingAttackForceUnits_JmpWrapper, 0x0043805B, 5);
		//AI_AddToAttack
		jmpPatch(AI_AddToAttack_JmpWrapper1, 0x0044724F, 3);
		jmpPatch(AI_AddToAttack_JmpWrapper2, 0x0044727B, 2);
		jmpPatch(AI_AttackManager_JmpWrapper, 0x0043AC54, 2);
		jmpPatch(AI_HasUnitsForAttackForce_JmpWrapper, 0x00437F2B, 1);
		jmpPatch(AI_RemoveFromAttack_Wrapper, 0x00447040, 0);
		jmpPatch(AI_SendSuicide_JmpWrapper, 0x0043E1EE, 1);
		jmpPatch(AI_Unk1_JmpWrapper, 0x0044816E, 0);//most likely do not execute
		jmpPatch(AI_Unk2_Wrapper, 0x00447220, 0);//most likely do not execute


		//AI_RemoveFromAttack
	}
	void injectLimitHooks() {


//		hooks::injectCCMUHooks();
//		hooks::sfxExtender();
		newSfxExtender();
//		injectPortdataLimits();
//		hooks::injectTblExtender();
		insaneFlingyIdExtender();

//		newAttackExtender();//if extended attack config option is not enabled in aise, will cause aise crashes

		jmpPatch(preInitDataWrapper, 0x004DAF31);
		/*
		auto str = scbw::readSamaseFile("samase\\sfx_ext.txt");
		GPTP::logger << "Read Test Len: " << str.size() << '\n';*/
	}
}
