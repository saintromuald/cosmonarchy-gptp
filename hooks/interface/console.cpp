#include "console.h"
#include <Windows.h>
#include <BW/api.h>
#include <BW/structures.h>
#include <BW/bwdata.h>

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
		if (g_console_index) {
			if (g_console_index == 1) {
				helper::dialog_grp_construct(178, (s32)"Starcraft\\SWAR\\lang\\game.cpp", (s32)"dlgs\\terran.grp", (s32(__fastcall*)(s32, s32, s32, s32))helper::load_graphic);
				g_current_music_id = 4;
			} else if (g_console_index == 2) {
				helper::dialog_grp_construct(183, (s32)"Starcraft\\SWAR\\lang\\game.cpp", (s32)"dlgs\\protoss.grp", (s32(__fastcall*)(s32, s32, s32, s32))helper::load_graphic);
				g_current_music_id = 7;
			}
		} else {
			helper::dialog_grp_construct(173, (s32)"Starcraft\\SWAR\\lang\\game.cpp", (s32)"dlgs\\zerg.grp", (s32(__fastcall*)(s32, s32, s32, s32))helper::load_graphic);
			g_current_music_id = 1;
		}
		v0 = *campaign_index;
		if (campaign_index) {
			if (*campaign_index > (u16)0x1F) {
				conrace = g_campaign_first_mission_bw + g_console_index;
			} else {
				conrace = g_campaign_first_mission + g_console_index;
			}
			v0 = (*campaign_index - conrace) / 3;
			v4 = (*campaign_index - conrace) % 3;
			g_current_music_id += v4;
			return v0;
		}
		v1 = g_console_race_specific;
		if (g_console_race_specific == -1) {
			v2 = helper::rand_func();
			v3 = v2;
			v0 = v2 / 3;
			v1 = v3 % 3;
		}
		v4 = v1 + 1;
		g_console_race_specific = v4;
		if (v4 < 3) {
			g_current_music_id += v4;
			return v0;
		}
		g_console_race_specific = 0;
		return v0;
	}

	void set_text_string(BinDlg* dialog) // 004263E0
	{
		const char* string = "";
		Unit* active_unit = *g_active_portrait_unit;
		u32 active_id = active_unit->id;
		s32 max_hp_real = units_dat::max_hitpoints[active_id] >> 8;
		if ((max_hp_real || (max_hp_real = (active_unit->hitpoints + 255) >> 8) != 0) && max_hp_real > 9999
		||   units_dat::max_shield_points[active_id] > 9999
		||   active_unit->status & UnitStatus::IsHallucination)
		{
			string = 0;
		} else {
			s32 max_hp_raw = helper::get_max_hp(active_unit);
			u8 hp_state = 0;
			if (helper::unit_hp_below_33_percent(*g_active_portrait_unit))
			{
				hp_state = 6;
			} else {
				hp_state = helper::unit_hp_between_33_66_percent(active_unit) != 0 ? 3 : 7;
			}

			static char buffer[32] = {};
			if (bw::unit_has_shields(active_unit)) {
				s32 hp_plus_1 = (active_unit->hitpoints + 255) >> 8;
				u16 last_queue_id = helper::get_last_queue_slot_id(active_unit);
				sprintf_s(buffer, 32, "%d/%d %c%d/%d", active_unit->shields >> 8, units_dat::max_shield_points[last_queue_id], hp_state, hp_plus_1, max_hp_raw);
			} else {
				sprintf_s(buffer, 32, "%c%d/%d", hp_state, (active_unit->hitpoints + 255) >> 8, max_hp_raw);
			}
			string = buffer;
		}
		helper::add_text_to_dialog(dialog, -7, string);
		helper::stats_panel_draw_energy(dialog);
	}
}