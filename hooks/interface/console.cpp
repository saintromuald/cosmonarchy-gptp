#include "console.h"
#include <Windows.h>
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace {
	void getRaceSoundList(char a1); //0048FB40
	int dialogGrpConstruct(int a1, int a2, int a3, int (__fastcall *a4)(int, int, int, int)); // 004197B0
	void loadGraphic(char a1, int a2, int a3, char a4, int a5); // 004D2B30
}

namespace hooks {
	__int16 loadRaceUI() { // 004EE2D0 -- https://pastebin.com/raw/N8H5XBKK
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
	}
}

namespace {
	const u32 Func_getRaceSoundList = 0x0048FB40;
	void getRaceSoundList(char a1) {
		static char a1;
		__asm {
			PUSHAD
			CALL Func_getRaceSoundList
			POPAD
		}
	};

	const u32 Func_dialogGrpConstruct = 0x004197B0;
	int dialogGrpConstruct(int a1, int a2, int a3, int(__fastcall* a4)(int, int, int, int)) {
		static int a1;
		static int a2;
		static int a3;
		static int(__fastcall * a4)(int, int, int, int);
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
		static char a1;
		static int a2;
		static int a3;
		static char a4;
		static int a5;
		__asm {
			PUSHAD
			MOV a1, ECX
			MOV a2, EDX
			MOV a3, ESI
			CALL Func_loadGraphic
			MOV EAX, result
			POPAD
		}
	};
}