#include "console.h"
#include <Windows.h>
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace {
	// For loadRaceUI
	void getRaceSoundList(int8 a1); //0048FB40
	int32 dialogGrpConstruct(int32 a1, int32 a2, int32 a3, int32(__fastcall* a4)(int32, int32, int32, int32)); // 004197B0
	void loadGraphic(int8 a1, int32 a2, int32 a3, int8 a4, int32 a5); // 004D2B30
	int32 randFunc();

	//For setTextStr
	bool unitHPbelow33_percent(CUnit* a1); // 004022C0
	bool unitHPbetween33_66_percent(int32 a1, CUnit* a2); // 00402270
	int32 getMaxHP(CUnit* a1); // 00401400
	uint16 getLastQueueSlotType(CUnit* a1); // 0047B270
	void addTextToDialog(int32 a1, int16 a2, const char* a3); // 004258B0
	void drawEnergy(int32 a1); // 00425A30
}

namespace hooks {
	int16 loadRaceUI() { // 004EE2D0 -- https://pastebin.com/raw/N8H5XBKK
		int32 v0; // eax
		int32 v1; // edx
		int32 v2; // eax
		__int64 v3; // rtt
		int32 v4; // edx
		uint8 conrace; // cl

		getRaceSoundList(1);
		if (consoleIndex) {
			if (*consoleIndex == 1) {
				dialogGrpConstruct(
					178,
					(int)"Starcraft\\SWAR\\lang\\game.cpp",
					(int)"dlgs\\terran.grp",
					(int(__fastcall*)(int, int, int, int))loadGraphic);
				*currentMusicId = 4;
			} else if (*consoleIndex == 2) {
				dialogGrpConstruct(
					183,
					(int)"Starcraft\\SWAR\\lang\\game.cpp",
					(int)"dlgs\\protoss.grp",
					(int(__fastcall*)(int, int, int, int))loadGraphic);
				*currentMusicId = 7;
			}
		} else {
			dialogGrpConstruct(
				173,
				(int)"Starcraft\\SWAR\\lang\\game.cpp",
				(int)"dlgs\\zerg.grp",
				(int(__fastcall*)(int, int, int, int))loadGraphic);
			*currentMusicId = 1;
		}
		v0 = *campaignIndex;
		if (campaignIndex) {
			if (*campaignIndex > (uint16)0x1F) {
				conrace = xCampaignFirstMission[*consoleIndex];
			} else {
				conrace = campaignFirstMission[*consoleIndex];
			}
			v0 = (*campaignIndex - conrace) / 3;
			v4 = (*campaignIndex - conrace) % 3;
			*currentMusicId += v4;
			return v0;
		}
		v1 = *consoleRaceSpecific;
		if (*consoleRaceSpecific == -1) {
			v2 = randFunc();
			v3 = v2;
			v0 = v2 / 3;
			v1 = v3 % 3;
		}
		v4 = v1 + 1;
		*consoleRaceSpecific = v4;
		if (v4 < 3) {
			*currentMusicId += v4;
			return v0;
		}
		*consoleRaceSpecific = 0;
		return v0;
	}

	void setTextStr(int32 a1) { // 004263E0
		CUnit* v1; // esi
		uint16 v2; // edi
		int32 v3; // eax
		int32 v4; // ecx
		uint8 v5; // bl
		int32 v6; // eax
		int32 v7; // ST14_4
		int32 v8; // ST10_4
		uint16 v9; // ax
		const char* v10; // [esp-4h] [ebp-10h]

		v1 = *activePortraitUnit;
		v2 = v1->id;
		v3 = units_dat::MaxHitPoints[v2] >> 8;
		if ((v3 || (v3 = (v1->hitPoints + 255) >> 8) != 0) && v3 > 9999
			|| units_dat::MaxShieldPoints[v2] > 0x270Fu
			|| v1->status & UnitStatus::IsHallucination) {
			v10 = 0;
		} else {
			if (unitHPbelow33_percent(*activePortraitUnit))
				v5 = 6;
			else
				v5 = unitHPbetween33_66_percent(v4, v1) != 0 ? 3 : 7;
			if (units_dat::ShieldsEnabled[v2]) {
				v7 = getMaxHP(v1);
				v8 = (v1->hitPoints + 255) >> 8;
				v9 = getLastQueueSlotType(v1);
				sprintf_s("", 32, "%d/%d %c%d/%d", v1->shields >> 8, units_dat::MaxShieldPoints[v9], v5, v8, v7);
			} else {
				v6 = getMaxHP(v1);
				sprintf_s("", 32, "%c%d/%d", v5, (v1->hitPoints + 255) >> 8, v6);
			}
			v10 = "";
		}
		addTextToDialog(a1, -7, v10);
		drawEnergy(a1);
	}
}

namespace {

	// For loadRaceUI
	const uint32 Func_getRaceSoundList = 0x0048FB40;
	void getRaceSoundList(int8 a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_getRaceSoundList
			POPAD
		}
	}

	const uint32 Func_dialogGrpConstruct = 0x004197B0;
	int32 dialogGrpConstruct(int32 a1, int32 a2, int32 a3, int32(__fastcall* a4)(int32, int32, int32, int32)) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			MOV a1, EAX
			MOV a2, ECX
			CALL Func_dialogGrpConstruct
			MOV EAX, result
			POPAD
		}
		return result;
	}

	const uint32 Func_loadGraphic = 0x004D2B30;
	void loadGraphic(int8 a1, int32 a2, int32 a3, int8 a4, int32 a5) {
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			MOVZX ECX, a1
			MOV EDX, a2
			MOV ESI, a3
			CALL Func_loadGraphic
			POPAD
		}
	}

	const uint32 Func_randFunc = 0x0040C7BD;
	int randFunc() {
		static int result;
		__asm {
			PUSHAD
			CALL Func_randFunc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For setTextStr

	const uint32 Func_unitHPBelow33Percent = 0x004022C0;
	bool unitHPbelow33_percent(CUnit* a1) {
		static int result;

		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitHPBelow33Percent
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_unitHPBetween33_66Percent = 0x00402270;
	bool unitHPbetween33_66_percent(int32 a1, CUnit* a2) {
		static int32 result;

		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL Func_unitHPBetween33_66Percent
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_getMaxHP = 0x00401400;
	int32 getMaxHP(CUnit* a1) {
		static int32 result;

		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_getMaxHP
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_getLastQueueSlotType = 0x0047B270;
	uint16 getLastQueueSlotType(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EDX, a1
			CALL Func_getLastQueueSlotType
			MOV result, EAX
			POPAD
		}
		return (uint16)result;

	}

	const uint32 Func_addTextToDialog = 0x004258B0;
	void addTextToDialog(int32 a1, int16 a2, const char* a3) {
		static int32 _a3 = (int)a3;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CX, a2
			PUSH _a3
			CALL Func_addTextToDialog
			POPAD
		}
	}

	const uint32 Func_drawEnergy = 0x00425A30;
	void drawEnergy(int32 a1) {
		__asm {
			PUSHAD
			MOV EBX, a1
			CALL Func_drawEnergy
			POPAD
		}
	}
}