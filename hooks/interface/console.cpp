#include "console.h"
#include <Windows.h>
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace hooks {
	int16 loadRaceUI() { // 004EE2D0 -- https://pastebin.com/raw/N8H5XBKK
		int32 v0; // eax
		int32 v1; // edx
		int32 v2; // eax
		__int64 v3; // rtt
		int32 v4; // edx
		uint8 conrace; // cl

		helper::getRaceSoundList(1);
		if (consoleIndex) {
			if (*consoleIndex == 1) {
				helper::dialogGrpConstruct(
					178,
					(int)"Starcraft\\SWAR\\lang\\game.cpp",
					(int)"dlgs\\terran.grp",
					(int(__fastcall*)(int, int, int, int))helper::loadGraphic);
				*currentMusicId = 4;
			} else if (*consoleIndex == 2) {
				helper::dialogGrpConstruct(
					183,
					(int)"Starcraft\\SWAR\\lang\\game.cpp",
					(int)"dlgs\\protoss.grp",
					(int(__fastcall*)(int, int, int, int))helper::loadGraphic);
				*currentMusicId = 7;
			}
		} else {
			helper::dialogGrpConstruct(
				173,
				(int)"Starcraft\\SWAR\\lang\\game.cpp",
				(int)"dlgs\\zerg.grp",
				(int(__fastcall*)(int, int, int, int))helper::loadGraphic);
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
			v2 = helper::randFunc();
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
			if (helper::unitHPbelow33_percent(*activePortraitUnit))
				v5 = 6;
			else
				v5 = helper::unitHPbetween33_66_percent(v4, v1) != 0 ? 3 : 7;
			if (units_dat::ShieldsEnabled[v2]) {
				v7 = helper::getMaxHP(v1);
				v8 = (v1->hitPoints + 255) >> 8;
				v9 = helper::getLastQueueSlotType(v1);
				sprintf_s("", 32, "%d/%d %c%d/%d", v1->shields >> 8, units_dat::MaxShieldPoints[v9], v5, v8, v7);
			} else {
				v6 = helper::getMaxHP(v1);
				sprintf_s("", 32, "%c%d/%d", v5, (v1->hitPoints + 255) >> 8, v6);
			}
			v10 = "";
		}
		helper::addTextToDialog(a1, -7, v10);
		helper::drawEnergy(a1);
	}
}