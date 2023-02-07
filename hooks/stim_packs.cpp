#include "stim_packs.h"
#include "../SCBW/api.h"
#include "SCBW/stat_txt_spec.h"

namespace hooks
{
	const s32 MIN_HITPOINTS_FOR_STIMPACKS = 1280;

	// Equivalent to original code, only doubt is queue_game_command.
	// Local answer when using stimpacks (check units of selection, and can just display an error message without notifying others players
	// of the attempt if no unit can do the stimpack action). -GPTP
	void cmdact_stimpack() // 004234D0
	{
		Unit* current_client_selection_unit;
		bool able_to_stim = false;

		// Original code used pointer comparison with g_client_selection_group_end and incrementing a pointer starting at g_client_selection_group
		for (s32 i = 0; i < SelectionCap && !able_to_stim; i++)
		{
			current_client_selection_unit = g_client_selection_group->unit[i];
			if (current_client_selection_unit
			&&  current_client_selection_unit->hitpoints > MIN_HITPOINTS_FOR_STIMPACKS)
			{
				able_to_stim = true;
			}
		}

		if (able_to_stim)
		{
			static u8 opcode[] = {0x36}; // packet opcode for stimpack is 0x36, no others args
			helper::queue_game_command(&opcode[0], 1);
		}
		else
		{
			char* message = (char*)g_stat_txt_tbl_new->get_string(874); // not enough life remaining
			helper::get_stat_txt_string(*g_active_portrait_unit, *g_local_nation_id, message);
		}
	}

	// Identical to StimpackSFXRand @ 004554A0
	// code used by the AI (no questions about sync between players in that case)
	void use_stimpack_ai(Unit* unit) // 004554A0
	{
		if (unit->hitpoints > MIN_HITPOINTS_FOR_STIMPACKS
		&&  unit->stim_timer == 0)
		{
			helper::play_sound_from_unit(helper::rand_between(SoundId::Terran_MARINE_TMaSti00_WAV, SoundId::Terran_MARINE_TMaSti01_WAV, 31), unit, 1, 0);
			unit->damage_hp(MIN_HITPOINTS_FOR_STIMPACKS, 0, -1, true);
			if (unit->stim_timer < 37)
			{
				unit->stim_timer = 37;
				unit->update_speed();
			}
		}
	}

	// Identical to original code
	// Global answer when applying the validated action of stimpack
	// Check the active player selection and apply stimpack effect on each valid stimpack user
	void cmdrecv_stimpack() // 004C2F30
	{
		Unit* active_player_selection;
		*unit_iterator_pos = 0;
		active_player_selection = get_active_player_next_selection();
		while (active_player_selection != nullptr)
		{
			if (active_player_selection->can_use_tech(TechId::Stimpacks, *g_active_nation_id) == 1
			&&  active_player_selection->hitpoints > MIN_HITPOINTS_FOR_STIMPACKS)
			{
				s32 random_value;
				if (!*g_is_in_game_loop)
				{
					random_value = 0;
				}
				else
				{
					static u32* randomnessCounter_0051C68C = (u32*)0x0051C68C;
					static u32* randomnessCounter_0051CA18 = (u32*)0x0051CA18;
					*randomnessCounter_0051C68C = (*randomnessCounter_0051C68C) + 1;
					*randomnessCounter_0051CA18 = (*randomnessCounter_0051CA18) + 1;
					*g_last_random_number = ((*g_last_random_number) * 0x015A4E35) + 1;
					random_value = ((*g_last_random_number) / 65535) & 32767;
				}
				// if used random, should give a value between 0 and 1
				random_value = (random_value + random_value) / 32768;
				helper::play_sound_from_unit(SoundId::Terran_MARINE_TMaSti00_WAV + random_value, active_player_selection, 1, 0);
				active_player_selection->damage_hp(MIN_HITPOINTS_FOR_STIMPACKS, 0, -1, true);
				if (active_player_selection->stim_timer < 37)
				{
					active_player_selection->stim_timer = 37;
					active_player_selection->update_speed();
				}
			}
			active_player_selection = get_active_player_next_selection();
		}
	}
}