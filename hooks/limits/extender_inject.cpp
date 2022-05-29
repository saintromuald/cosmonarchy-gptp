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
namespace hooks
{
	void loadAiseSamaseData()
	{
		hooks::sfxExtenderGetCount();
		hooks::unitsDatWrapperPatches();
	}

	void AI_RemoveFromAttack(s32 unitId_, u32 player)
	{
		s32 unitId = unitId_;
		if (unitId == UnitId::TerranPhalanxSiegeMode)
		{
			unitId = UnitId::TerranPhalanxTankMode;
		}
		unitId++;
		for (s32 i = 0; i < ATTACK_LIMIT; i++)
		{
			if (globals.attackers.attacks[player][i] == unitId)
			{
				globals.attackers.attacks[player][i] = UnitId::None + 1;
				return;
			}
		}
	}

	const u32 jmp_004DAF36 = 0x004DAF36;
	void __declspec(naked) preInitDataWrapper()
	{
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

	void __declspec(naked) FakeUnitFlingyIdState_Wrapper()
	{
		//		jump_patch(FakeUnitFlingyIdState_Wrapper, 0x004AAF23);
		static u32 flingyId;
		static u32 index;
		__asm {
			MOV index, ECX
			PUSHAD
		}
		flingyId = fakeExtFlingyIds[index];
		__asm {
			POPAD
			MOV EAX, flingyId
			RETN
		}
	}

	void fraudFlingyIdCopy()
	{
		for (s32 i = 0; i < 228; i++)
		{
			fakeExtFlingyIds[i] = units_dat::Graphic[i];
		}
	}

	void __declspec(naked) psychosisFlingyId_Wrapper()
	{
		__asm {
			PUSHAD
		}
		fraudFlingyIdCopy();
		__asm {
			POPAD
			RETN
		}
	}

	void set_flingyid_value(u32 index, u32 value)
	{
		fakeExtFlingyIds[index] = value;
	}

	void __declspec(naked) madnessSetFlingyId_Wrapper()
	{
		static u32 index;
		static u32 value;
		__asm {
			MOV index, EAX
			MOV value, ECX
			PUSHAD
		}
		set_flingyid_value(index, value);
		__asm {
			POPAD
			RETN
		}
	}

	u32 buildingVisionSyncHook()
	{
		s32 result = 0;
		u8* visHash = (u8*)0x0063fd30;
		u32* bWantVisibilityHashUpdate = (u32*)0x00629d90;
		u8* regionVisionHash = (u8*)0x0063fe38;
		s32* nCurrentHashOffset = (s32*)0x0063fd28;
		//
		memset(visHash, 0, 64);
		*bWantVisibilityHashUpdate = 0;
		*regionVisionHash = 0;
		*nCurrentHashOffset = 0;
		s32 i = 0;
		do
		{
			if (!(units_dat::BaseProperty[i] & UnitProperty::Building))
			{
				if (units_dat::StarEditAvailFlags[i] & 0x1)
				{
					auto flingy_id = fakeExtFlingyIds[i];
					auto sprite_id = scbw::getFlingyData(FlingyDatEntries::SpriteId, flingy_id);
					scbw::setSpriteData(SpritesDatEntries::IncludeInVision, sprite_id, 1);
				}
			}
			i++;
		}
		while (i < 228);
		return i;
	}

	void __declspec(naked) buildingVisionSyncHook_Wrapper()
	{
		static u32 result;
		__asm {
			PUSHAD
		}
		result = buildingVisionSyncHook();
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void dumbCodeInject(u32 address, u32* ids, std::vector<u8> code, u32 nop)
	{
		memoryPatch(address + code.size(), ids);
		for (s32 i = 0; i < code.size(); i++)
		{
			memoryPatch(address + i, code[i]);
		}
		nops(address + code.size() + 4, nop);
	}

	void insaneFlingyIdExtender()
	{
		jump_patch(FakeUnitFlingyIdState_Wrapper, 0x004AAFE3);
		jump_patch(psychosisFlingyId_Wrapper, 0x004AB045);
		jump_patch(madnessSetFlingyId_Wrapper, 0x004AB147);

		std::vector<u8> eax_eax = {0x8b, 0x04, 0x85};
		dumbCodeInject(0x00454317, fakeExtFlingyIds, eax_eax, 0); // +
		std::vector<u8> ecx_edi = {0x8b, 0x0c, 0xbd};
		dumbCodeInject(0x00463A71, fakeExtFlingyIds, ecx_edi, 0);
		std::vector<u8> edx_ecx = {0x8b, 0x14, 0x8d};
		//		dumbCodeInject(0x004683C6, fakeExtFlingyIds, edx_ecx, 2); // unit_morph - AI Wrapper
		std::vector<u8> edx_eax = {0x8b, 0x14, 0x85};
		//		dumbCodeInject(0x0047B854, fakeExtFlingyIds, edx_eax, 2);
		//		dumbCodeInject(0x0047B8A4, fakeExtFlingyIds, edx_eax, 2);
		// 
				// wrappers 3-5 if gptp speed hooks are enabled
		std::vector<u8> ecx_eax = {0x8b, 0x0c, 0x85};
		dumbCodeInject(0x0047B8F4, fakeExtFlingyIds, ecx_eax, 0);
		dumbCodeInject(0x0048D756, fakeExtFlingyIds, ecx_eax, 0);
		dumbCodeInject(0x0048DA8A, fakeExtFlingyIds, ecx_eax, 0);

		//building visibility
//		jump_patch(buildingVisionSyncHook_Wrapper, 0x00497110);

		dumbCodeInject(0x00497148, fakeExtFlingyIds, edx_eax, 0);
		dumbCodeInject(0x0049716F, fakeExtFlingyIds + 1, edx_eax, 0);
		dumbCodeInject(0x00497196, fakeExtFlingyIds + 2, edx_eax, 0);
		dumbCodeInject(0x004971BD, fakeExtFlingyIds + 3, edx_eax, 0);
		dumbCodeInject(0x004971E4, fakeExtFlingyIds + 4, edx_eax, 0);
		dumbCodeInject(0x0049720B, fakeExtFlingyIds + 5, edx_eax, 0);

		std::vector<u8> eax_ebx = {0x8b, 0x04, 0x9d};
		dumbCodeInject(0x0049ED0B, fakeExtFlingyIds, eax_ebx, 0); // crashes hexrays
		std::vector<u8> eax_edx = {0x8b, 0x04, 0x95};
		dumbCodeInject(0x004E9A10, fakeExtFlingyIds, eax_edx, 0);
	}

	void __declspec(naked) GetAttackUnit_Wrapper()
	{
		static u32 unitId;
		static u32 playerId;
		static u32 index;
		__asm {
			MOV playerId, ECX
			MOV index, EBX
			PUSHAD
		}
		assert(playerId < 8);
		assert(index < ATTACK_LIMIT);
		//		unitId = AIScriptController[playerId].attackers[index]; // is actually "unitId+1"
		unitId = globals.attackers.attacks[playerId][index];
		__asm {
			POPAD
			MOV EAX, unitId
			RETN
		}
	}

	void __declspec(naked) GetAttackLength_Wrapper()
	{
		__asm {
			PUSHAD
		}
		__asm {
			POPAD
			MOV EAX, 0x3F
			RETN
		}
	}

	void __declspec(naked) SetAttackUnit_Wrapper()
	{
		static u32 unitId;
		static u32 index;
		static u32 playerId;
		__asm {
			MOV unitId, ECX
			MOV index, EBX
			MOV playerId, EDX
			PUSHAD
		}
		assert(playerId < 8);
		assert(index < ATTACK_LIMIT);//change later
//		AIScriptController[playerId].attackers[index] = unitId;//is actually "unitId+1"
		globals.attackers.attacks[playerId][index] = unitId;
		__asm {
			POPAD
			RETN
		}
	}

	const u32 GetMissingJmpWrapper = 0x00438067;
	void __declspec(naked) AI_GetMissingAttackForceUnits_JmpWrapper()
	{
		static u32 playerId;
		static u16* arrayStart;
		__asm {
			MOV playerId, ECX
			PUSHAD
		}
		//		arrayStart = &AIScriptController[playerId].attackers[0];
		arrayStart = &globals.attackers.attacks[playerId][0];
		__asm {
			POPAD
			MOV ECX, arrayStart
			JMP GetMissingJmpWrapper
		}
	}

	const u32 AIAddToAttack_NullJmp = 0x00447274;
	const u32 AIAddToAttack_Continue = 0x00447265;
	void __declspec(naked) AI_AddToAttack_JmpWrapper1()
	{
		static u32 playerId;
		static u32 arrayOffset;
		__asm {
			MOV playerId, ECX
			PUSHAD
		}
		//		if (AIScriptController[playerId].attackers[0] == 0) {
		if (globals.attackers.attacks[playerId][0] == 0)
		{
			__asm {
				POPAD
				JMP AIAddToAttack_NullJmp
			}
		}
		else
		{
			//			arrayOffset = reinterpret_cast<u32>(&AIScriptController[playerId].attackers[0]);
			arrayOffset = reinterpret_cast<u32>(&globals.attackers.attacks[playerId][0]);
			__asm {
				POPAD
				MOV ECX, arrayOffset
				JMP AIAddToAttack_Continue
			}
		}
	}

	const u32 AIAddAttackTo_LastJmp = 0x00447282;
	void __declspec(naked) AI_AddToAttack_JmpWrapper2()
	{
		static u32 playerMul;
		static u32 player;
		static u32 index;
		static u32 getUnit;
		__asm {
			MOV playerMul, EDX
			MOV index, EAX
			PUSHAD
		}
		assert(index < ATTACK_LIMIT);
		player = playerMul / (sizeof(AI_Main) / 2);
		//		getUnit = AIScriptController[player].attackers[index];
		getUnit = globals.attackers.attacks[player][index];
		__asm {
			POPAD
			MOV ECX, getUnit
			JMP AIAddAttackTo_LastJmp
		}
	}

	const u32 AIAttackManager_Jmp = 0x0043AC61;
	void __declspec(naked) AI_AttackManager_JmpWrapper()
	{
		static u32 offset;
		static u32 playerId;
		static u32 arrayOffset;
		static u16 unitId;
		__asm {
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV playerId, EAX
			POP EAX
			PUSHAD
		}
		/*
		unitId = AIScriptController[playerId].attackers[0];
		arrayOffset = reinterpret_cast<u32>(AIScriptController[playerId].attackers);
		*/
		unitId = globals.attackers.attacks[playerId][0];
		arrayOffset = reinterpret_cast<u32>(&globals.attackers.attacks[playerId][0]);
		__asm {
			POPAD
			MOV AX, unitId
			MOV ECX, arrayOffset
			JMP AIAttackManager_Jmp
		}
	}

	static u32 AI_HasUnitsForAttack_Jmp = 0x00437F36;
	void __declspec(naked) AI_HasUnitsForAttackForce_JmpWrapper()
	{
		static u32 playerId;
		static u32 arrayOffset;
		__asm {
			MOV playerId, EAX
			PUSHAD
		}
		//		arrayOffset = reinterpret_cast<u32>(AIScriptController[playerId].attackers);
		arrayOffset = reinterpret_cast<u32>(&globals.attackers.attacks[playerId][0]);
		__asm {
			POPAD
			MOV EAX, arrayOffset
			JMP AI_HasUnitsForAttack_Jmp
		}
	}

	void __declspec(naked) AI_RemoveFromAttack_Wrapper()
	{ // 00447040
		static u32 unitId;
		static u32 player;
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

	const u32 Jmp_SendSuicide = 0x0043e1f9;
	void __declspec(naked) AI_SendSuicide_JmpWrapper()
	{
		static u32 playerId;
		static u32 index;
		static u32 arrayOffset;
		static u32 size2;
		__asm {
			MOV EDX, index
			PUSHAD
		}
		playerId = index / sizeof(AI_Main);
		arrayOffset = reinterpret_cast<u32>(&globals.attackers.attacks[playerId][0]);
		size2 = ATTACK_LIMIT / 2;
		__asm {
			POPAD
			MOV EDI, arrayOffset
			MOV ECX, size2
			JMP Jmp_SendSuicide
		}
	}

	const u32 Jmp_Unk1 = 0x00448179;
	void __declspec(naked) AI_Unk1_JmpWrapper()
	{
		static u32 playerId;
		static u32 index;
		static u32 arrayOffset;
		static u32 size2;
		__asm {
			MOV EDX, index
			PUSHAD
		}
		playerId = index / sizeof(AI_Main);
		arrayOffset = reinterpret_cast<u32>(&globals.attackers.attacks[playerId][0]);
		size2 = ATTACK_LIMIT / 2;
		__asm {
			POPAD
			MOV EDI, arrayOffset
			MOV ECX, size2
			JMP Jmp_Unk1
		}
	}

	void __declspec(naked) AI_Unk2_Wrapper()
	{
		static u32 playerId;
		static u32 arrayOffset;
		__asm {
			MOV EAX, playerId
			PUSHAD
		}
		arrayOffset = reinterpret_cast<u32>(&globals.attackers.attacks[playerId][0]);
		__asm {
			POPAD
			MOV EAX, arrayOffset
			RETN
		}
	}

	void newAttackExtender()
	{
		jump_patch(GetAttackUnit_Wrapper, 0x004171E5); // null int3 bytes
		jump_patch(GetAttackLength_Wrapper, 0x00417268); // null int3 bytes
		jump_patch(SetAttackUnit_Wrapper, 0x00417207); // null int3 bytes

		u32 attackSize = (ATTACK_LIMIT * 4) + 8;
		memoryPatch(0x0043E673 + 2, attackSize);
		memoryPatch(0x0043EEC3 + 2, attackSize);
		jump_patch(AI_GetMissingAttackForceUnits_JmpWrapper, 0x0043805B, 5);

		//AI_AddToAttack
		jump_patch(AI_AddToAttack_JmpWrapper1, 0x0044724F, 3);
		jump_patch(AI_AddToAttack_JmpWrapper2, 0x0044727B, 2);
		jump_patch(AI_AttackManager_JmpWrapper, 0x0043AC54, 2);
		jump_patch(AI_HasUnitsForAttackForce_JmpWrapper, 0x00437F2B, 1);
		jump_patch(AI_RemoveFromAttack_Wrapper, 0x00447040, 0);
		jump_patch(AI_SendSuicide_JmpWrapper, 0x0043E1EE, 1);
		jump_patch(AI_Unk1_JmpWrapper, 0x0044816E, 0); // most likely do not execute
		jump_patch(AI_Unk2_Wrapper, 0x00447220, 0); // most likely do not execute
	}

	void injectLimitHooks()
	{

//		hooks::injectCCMUHooks();
//		hooks::sfxExtender();
		newSfxExtender();
//		injectPortdataLimits();
//		hooks::injectTblExtender();
		insaneFlingyIdExtender();

//		newAttackExtender(); // if extended attack config option is not enabled in aise, will cause aise crashes

		jump_patch(preInitDataWrapper, 0x004DAF31);
		/*
		auto str = scbw::readSamaseFile("samase\\sfx_ext.txt");
		GPTP::logger << "Read Test Len: " << str.size() << '\n';
		*/
	}
}
