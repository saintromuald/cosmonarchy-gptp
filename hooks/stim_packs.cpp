#include "stim_packs.h"
#include "../SCBW/api.h"
#include "SCBW/stat_txt_spec.h"

namespace hooks
{
	const s32 MIN_HITPOINTS_FOR_STIMPACKS = 1280;

	// Local answer when using stimpacks (check units of selection, and can just display an error message without notifying others players
	// of the attempt if no unit can do the stimpack action). -GPTP
	void cmdact_stimpack() // 004234D0
	{
		Unit* current_client_selection_unit;
		bool able_to_stim = false;

		// Original code used pointer comparison with g_client_selection_group_end and incrementing a pointer starting at g_client_selection_group
		for (s32 i = 0; i < SelectionCap && !able_to_stim; i++) {
			current_client_selection_unit = g_client_selection_group->unit[i];
			if (current_client_selection_unit && current_client_selection_unit->hitpoints > MIN_HITPOINTS_FOR_STIMPACKS) {
				able_to_stim = true;
			}
		}

		if (able_to_stim) {
			static u8 opcode[] = {0x36}; // packet opcode for stimpack is 0x36, no others args
			helper::queue_game_command(&opcode[0], 1);
		} else {
			char* message = (char*)g_stat_txt_tbl_new->get_string(874); // not enough life remaining
			helper::get_stat_txt_string(*g_active_portrait_unit, *g_local_nation_id, message);
		}
	}

	// Code used by the AI (no questions about sync between players in that case) -GPTP
	void use_stimpack_ai(Unit* unit) // 004554A0
	{
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit->hitpoints > MIN_HITPOINTS_FOR_STIMPACKS && unit->stim_timer == 0 && unit_ext->timers[TimerStimpackDelay] <= 1) {
			helper::play_sound_from_unit(SoundId::StimpackInject01, unit, 1, 0);
			unit->damage_hp(MIN_HITPOINTS_FOR_STIMPACKS, 0, -1, true);
			unit_ext->timers[TimerStimpackDelay] = 3 * 24 + 1;
		}
	}

	// Global answer when applying the validated action of stimpack
	// Check the active player selection and apply stimpack effect on each valid stimpack user -GPTP
	void cmdrecv_stimpack() // 004C2F30
	{
		Unit* active_player_selection;
		*g_unit_iterator_pos = 0;
		active_player_selection = get_active_player_next_selection();
		while (active_player_selection) {
			if (active_player_selection->can_use_tech(TechId::Stimpacks, *g_active_nation_id) == 1 && active_player_selection->hitpoints > MIN_HITPOINTS_FOR_STIMPACKS)
			{
				helper::play_sound_from_unit(SoundId::StimpackInject01, active_player_selection, 1, 0);
				active_player_selection->damage_hp(MIN_HITPOINTS_FOR_STIMPACKS, 0, -1, true);
				UnitExt* unit_ext = get_unit_ext(active_player_selection);
				unit_ext->timers[TimerStimpackDelay] = 2 * 24 + 1;
			}
			active_player_selection = get_active_player_next_selection();
		}
	}
}