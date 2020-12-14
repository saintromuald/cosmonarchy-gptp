#include "hooks/limits/extender.h"
#include "hooks/limits/ccmu.h"
#include "hooks/limits/sfx_extender.h"
#include "hooks/limits/tbl_extender.h"
#include "hooks/limits/misc_extenders.h"
#include "hooks/limits/fake_unitid_extender.h"
#include "logger.h"
#include "globals.h"
namespace hooks {
	void loadAiseSamaseData() {
		hooks::sfxExtenderGetCount();
		hooks::unitsDatWrapperPatches();
	}
	const u32 jmp_004DAF36 = 0x004DAF36;
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
		static u32 flingyId;
		static u32 index;
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
	void set_flingyid_value(u32 index, u32 value) {
		fakeExtFlingyIds[index] = value;
	}
	void __declspec(naked) madnessSetFlingyId_Wrapper(){
		static u32 index;
		static u32 value;
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
	u32 buildingVisionSyncHook() {
		int result = 0;
		u8* visHash = (u8*)0x0063fd30;
		u32* bWantVisibilityHashUpdate = (u32*)0x00629d90;
		u8* regionVisionHash = (u8*)0x0063fe38;
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
		static u32 result;
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
	void dumbCodeInject(u32 address, u32* ids, std::vector<u8> code, u32 nop) {
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

		std::vector<u8> eax_eax = { 0x8b, 0x04, 0x85 };
		dumbCodeInject(0x00454317, fakeExtFlingyIds, eax_eax, 0);//+
		std::vector<u8> ecx_edi = { 0x8b, 0x0c, 0xbd };
		dumbCodeInject(0x00463A71, fakeExtFlingyIds, ecx_edi, 0);
		std::vector<u8> edx_ecx = { 0x8b, 0x14, 0x8d };
	//	dumbCodeInject(0x004683C6, fakeExtFlingyIds, edx_ecx, 2);//unit_morph - AI Wrapper
		std::vector<u8> edx_eax = { 0x8b, 0x14, 0x85 };
		//dumbCodeInject(0x0047B854, fakeExtFlingyIds, edx_eax, 2);
		//dumbCodeInject(0x0047B8A4, fakeExtFlingyIds, edx_eax, 2);
		//wrappers 3-5 if gptp speed hooks are enabled
		std::vector<u8> ecx_eax = { 0x8b, 0x0c, 0x85 };
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
		std::vector<u8> eax_ebx = { 0x8b, 0x04, 0x9d };
		dumbCodeInject(0x0049ED0B, fakeExtFlingyIds, eax_ebx, 0);//crashes hexrays
		std::vector<u8> eax_edx = { 0x8b, 0x04, 0x95 };
		dumbCodeInject(0x004E9A10, fakeExtFlingyIds, eax_edx, 0);
		
		
	}
	void injectLimitHooks() {

//		hooks::injectCCMUHooks();
//		hooks::sfxExtender();
		newSfxExtender();
//		injectPortdataLimits();
//		hooks::injectTblExtender();
		insaneFlingyIdExtender();
		jmpPatch(preInitDataWrapper, 0x004DAF31);
		/*
		auto str = scbw::readSamaseFile("samase\\sfx_ext.txt");
		GPTP::logger << "Read Test Len: " << str.size() << '\n';*/
	}
}
