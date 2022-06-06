#include "console.h"
#include <Windows.h>
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace hooks
{
	s16 loadRaceUI()
	{ // 004EE2D0 -- https://pastebin.com/raw/N8H5XBKK
		s32 v0; // eax
		s32 v1; // edx
		s32 v2; // eax
		__int64 v3; // rtt
		s32 v4; // edx
		u8 conrace; // cl

		helper::get_race_sound_list(1);
		if (console_index)
		{
			if (*console_index == 1)
			{
				helper::dialog_grp_construct(
					178,
					(s32)"Starcraft\\SWAR\\lang\\game.cpp",
					(s32)"dlgs\\terran.grp",
					(s32(__fastcall*)(s32, s32, s32, s32))helper::load_graphic);
				*current_music_id = 4;
			}
			else if (*console_index == 2)
			{
				helper::dialog_grp_construct(
					183,
					(s32)"Starcraft\\SWAR\\lang\\game.cpp",
					(s32)"dlgs\\protoss.grp",
					(s32(__fastcall*)(s32, s32, s32, s32))helper::load_graphic);
				*current_music_id = 7;
			}
		}
		else
		{
			helper::dialog_grp_construct(
				173,
				(s32)"Starcraft\\SWAR\\lang\\game.cpp",
				(s32)"dlgs\\zerg.grp",
				(s32(__fastcall*)(s32, s32, s32, s32))helper::load_graphic);
			*current_music_id = 1;
		}
		v0 = *campaignIndex;
		if (campaignIndex)
		{
			if (*campaignIndex > (u16)0x1F)
			{
				conrace = xCampaignFirstMission[*console_index];
			}
			else
			{
				conrace = campaignFirstMission[*console_index];
			}
			v0 = (*campaignIndex - conrace) / 3;
			v4 = (*campaignIndex - conrace) % 3;
			*current_music_id += v4;
			return v0;
		}
		v1 = *console_race_specific;
		if (*console_race_specific == -1)
		{
			v2 = helper::rand_func();
			v3 = v2;
			v0 = v2 / 3;
			v1 = v3 % 3;
		}
		v4 = v1 + 1;
		*console_race_specific = v4;
		if (v4 < 3)
		{
			*current_music_id += v4;
			return v0;
		}
		*console_race_specific = 0;
		return v0;
	}

	void set_text_string(BinDlg* a1)
	{ // 004263E0
		Unit* v1; // esi
		u16 v2; // edi
		s32 v3; // eax
		s32 v4; // ecx
		u8 v5; // bl
		s32 v6; // eax
		s32 v7; // ST14_4
		s32 v8; // ST10_4
		u16 v9; // ax
		const char* v10; // [esp-4h] [ebp-10h]

		v1 = *active_portrait_unit;
		v2 = v1->id;
		v3 = units_dat::MaxHitPoints[v2] >> 8;
		if ((v3 || (v3 = (v1->hitpoints + 255) >> 8) != 0) && v3 > 9999
			|| units_dat::MaxShieldPoints[v2] > 0x270Fu
			|| v1->status & UnitStatus::IsHallucination)
		{
			v10 = 0;
		}
		else
		{
			if (helper::unit_hp_below_33_percent(*active_portrait_unit))
				v5 = 6;
			else
				v5 = helper::unit_hp_between_33_66_percent(v4, v1) != 0 ? 3 : 7;
			if (units_dat::ShieldsEnabled[v2])
			{
				v7 = helper::get_max_hp(v1);
				v8 = (v1->hitpoints + 255) >> 8;
				v9 = helper::get_last_queue_slot_id(v1);
				sprintf_s("", 32, "%d/%d %c%d/%d", v1->shields >> 8, units_dat::MaxShieldPoints[v9], v5, v8, v7);
			}
			else
			{
				v6 = helper::get_max_hp(v1);
				sprintf_s("", 32, "%c%d/%d", v5, (v1->hitpoints + 255) >> 8, v6);
			}
			v10 = "";
		}
		helper::add_text_to_dialog(a1, -7, v10);
		helper::draw_energy(a1);
	}
}