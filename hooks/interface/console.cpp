#include "console.h"
#include <Windows.h>
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace {
	// For loadRaceUI
	void getRaceSoundList(char a1); //0048FB40
	int dialogGrpConstruct(int a1, int a2, int a3, int (__fastcall *a4)(int, int, int, int)); // 004197B0
	void loadGraphic(char a1, int a2, int a3, char a4, int a5); // 004D2B30

	//For setTextStr
	bool unitHPbelow33_percent(CUnit* a1); // 004022C0
	bool unitHPbetween33_66_percent(int a1, CUnit* a2); // 00402270
	int getMaxHP(CUnit* a1); // 00401400
	unsigned __int16 getLastQueueSlotType(CUnit* a1); // 0047B270
	void addTextToDialog(int a1, __int16 a2, const char* a3); // 004258B0
	void drawEnergy(int a1); // 00425A30
}

namespace hooks {
	/*__int16 loadRaceUI() { // 004EE2D0 -- https://pastebin.com/raw/N8H5XBKK
		int v0; // eax
		int v1; // edx
		int v2; // eax
		__int64 v3; // rtt
		int v4; // edx
		unsigned __int8 conrace; // cl

		getRaceSoundList(1);
		if (consoleIndex)
		{
			if (consoleIndex == 1)
			{
				dialogGrpConstruct(
					178,
					(int)"Starcraft\\SWAR\\lang\\game.cpp",
					(int)"dlgs\\terran.grp",
					(int(__fastcall*)(int, int, int, int))loadGraphic);
				currentMusicId = 4;
			}
			else if (consoleIndex == 2)
			{
				dialogGrpConstruct(
					183,
					(int)"Starcraft\\SWAR\\lang\\game.cpp",
					(int)"dlgs\\protoss.grp",
					(int(__fastcall*)(int, int, int, int))loadGraphic);
				currentMusicId = 7;
			}
		}
		else
		{
			dialogGrpConstruct(
				173,
				(int)"Starcraft\\SWAR\\lang\\game.cpp",
				(int)"dlgs\\zerg.grp",
				(int(__fastcall*)(int, int, int, int))loadGraphic);
			currentMusicId = 1;
		}
		LOWORD(v0) = campaignIndex;
		if (campaignIndex)
		{
			if (campaignIndex > (unsigned __int16)0x1F)
				conrace = xCampaignFirstMission[consoleIndex];
			else
				conrace = campaignFirstMission[consoleIndex];
			v0 = (campaignIndex - conrace) / 3;
			v4 = (campaignIndex - conrace) % 3;
			currentMusicId += v4;
			return v0;
		}
		v1 = consoleRaceSpecific;
		if (consoleRaceSpecific == -1)
		{
			v2 = rand();
			v3 = v2;
			v0 = v2 / 3;
			v1 = v3 % 3;
		}
		v4 = v1 + 1;
		consoleRaceSpecific = v4;
		if (v4 < 3)
		{
			currentMusicId += v4;
			return v0;
		}
		consoleRaceSpecific = 0;
		return v0;
	}*/

	void setTextStr(int a1) { // 004263E0
		CUnit* v1; // esi
		unsigned __int16 v2; // edi
		int v3; // eax
		int v4; // ecx
		unsigned __int8 v5; // bl
		int v6; // eax
		int v7; // ST14_4
		int v8; // ST10_4
		unsigned __int16 v9; // ax
		const char* v10; // [esp-4h] [ebp-10h]

		v1 = *activePortraitUnit;
		v2 = v1->id;
		v3 = units_dat::MaxHitPoints[v2] >> 8;
		if ((v3 || (v3 = (v1->hitPoints + 255) >> 8) != 0) && v3 > 9999
			|| units_dat::MaxShieldPoints[v2] > 0x270Fu
			|| v1->status & UnitStatus::IsHallucination)
		{
			v10 = 0;
		}
		else
		{
			if (unitHPbelow33_percent(*activePortraitUnit))
				v5 = 6;
			else
				v5 = unitHPbetween33_66_percent(v4, v1) != 0 ? 3 : 7;
			if (units_dat::ShieldsEnabled[v2])
			{
				v7 = getMaxHP(v1);
				v8 = (v1->hitPoints + 255) >> 8;
				v9 = getLastQueueSlotType(v1);
				sprintf_s("", 32, "%d/%d %c%d/%d", v1->shields >> 8, units_dat::MaxShieldPoints[v9], v5, v8, v7);
			}
			else
			{
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
	/*
	const u32 Func_getRaceSoundList = 0x0048FB40;
	void getRaceSoundList(char a1) {

		__asm {
			PUSHAD
			CALL Func_getRaceSoundList
			POPAD
		}
	};

	const u32 Func_dialogGrpConstruct = 0x004197B0;
	int dialogGrpConstruct(int a1, int a2, int a3, int(__fastcall* a4)(int, int, int, int)) {

		static int result;
		__asm {
			PUSHAD
			MOV a1, EAX
			MOV a2, ECX
			CALL Func_dialogGrpConstruct
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_loadGraphic = 0x004D2B30;
	void loadGraphic(char a1, int a2, int a3, char a4, int a5) {

		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			MOV ESI, a3
			CALL Func_loadGraphic
			MOV EAX, result
			POPAD
		}
	};*/

	// For setTextStr

	const u32 Func_unitHPBelow33Percent = 0x004022C0;
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

	const u32 Func_unitHPBetween33_66Percent = 0x00402270;
	bool unitHPbetween33_66_percent(int a1, CUnit * a2) {
		static int result;

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

	const u32 Func_getMaxHP = 0x00401400;
	int getMaxHP(CUnit* a1) {
		static int result;

		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_getMaxHP
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_getLastQueueSlotType = 0x0047B270;
	unsigned __int16 getLastQueueSlotType(CUnit* a1) {
		static int result;

		__asm {
			POPAD
			MOV EDX, a1
			CALL Func_getLastQueueSlotType
			MOV result, EAX
			POPAD
		}
		return (unsigned __int16)result;

	}

	const u32 Func_addTextToDialog = 0x004258B0;
	void addTextToDialog(int a1, __int16 a2, const char* a3) {
		static int _a3 = (int)a3;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CX, a2
			PUSH _a3
			CALL Func_addTextToDialog
			POPAD
		}
	}

	const u32 Func_drawEnergy = 0x00425A30;
	void drawEnergy(int a1) {
		__asm {
			PUSHAD
			MOV EBX, a1
			CALL Func_drawEnergy
			POPAD
		}
	}
}