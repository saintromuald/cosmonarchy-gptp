#include "stim_packs.h"
#include "../SCBW/api.h"
#include "SCBW/stat_txt_spec.h"

namespace hooks
{

	const s32 MIN_HITPOINTS_FOR_STIMPACKS = 2560;

	// Equivalent to original code, only doubt is queue_game_command
	// Local answer when using stimpacks (check units of selection,
	// and can just display an error message without notifying others
	// players of the attempt if no unit can do the stimpack action)
	void CMDACT_Stimpack()
	{
		Unit* currentClientSelectionUnit;
		bool bFoundUnitAbleToStimpack = false;

		// original code used pointer comparison with clientSelectionGroupEnd
		// and incrementing a pointer starting at clientSelectionGroup
		for (s32 i = 0; i < SELECTION_ARRAY_LENGTH && !bFoundUnitAbleToStimpack; i++)
		{
			currentClientSelectionUnit = clientSelectionGroup->unit[i];
			if (currentClientSelectionUnit != nullptr &&
				currentClientSelectionUnit->hitPoints > MIN_HITPOINTS_FOR_STIMPACKS)
			{
				bFoundUnitAbleToStimpack = true;
			}
		}

		if (bFoundUnitAbleToStimpack)
		{
			static u8 opcode[] = {0x36};	// packet opcode for stimpack is 0x36, no others args
			helper::queue_game_command(&opcode[0], 1);
		}
		else
		{
			char* message = (char*)statTxtTblNew->getString(874); //not enough life remaining
			helper::get_statTxt_Str_0(*activePortraitUnit, *LOCAL_NATION_ID, message);
		}
	}

	// Identical to StimpackSFXRand @ 004554A0
	// code used by the AI (no questions about sync between players in that case)
	void useStimPacksAIHook(Unit* unit)
	{
		if (unit->hitPoints > MIN_HITPOINTS_FOR_STIMPACKS + StimpackOffset
			&& unit->stimTimer == 0)
		{
			helper::playSoundFromSource(helper::rand_between(SoundId::Terran_MARINE_TMaSti00_WAV, SoundId::Terran_MARINE_TMaSti01_WAV, 31), unit, 1, 0);
			unit->damageHp(MIN_HITPOINTS_FOR_STIMPACKS, nullptr, -1, true);
			if (unit->stimTimer < 37)
			{
				unit->stimTimer = 37;
				unit->update_speed();
			}
		}
	}

	// Identical to original code
	// Global answer when applying the validated action of stimpack
	// Check the active player selection and apply stimpack effect on
	// each valid stimpack user
	void CMDRECV_StimPack()
	{
		Unit* activePlayerCurrentSelection;
		*selectionIndexStart = 0;
		activePlayerCurrentSelection = getActivePlayerNextSelection();
		while (activePlayerCurrentSelection != nullptr)
		{
			if (activePlayerCurrentSelection->canUseTech(TechId::Stimpacks, *ACTIVE_NATION_ID) == 1
				&& activePlayerCurrentSelection->hitPoints > MIN_HITPOINTS_FOR_STIMPACKS)
			{
				s32 random_value;
				if (!*IS_IN_GAME_LOOP)
				{
					random_value = 0;
				}
				else
				{
					static u32* randomnessCounter_0051C68C = (u32*)0x0051C68C;
					static u32* randomnessCounter_0051CA18 = (u32*)0x0051CA18;
					*randomnessCounter_0051C68C = (*randomnessCounter_0051C68C) + 1;
					*randomnessCounter_0051CA18 = (*randomnessCounter_0051CA18) + 1;
					*lastRandomNumber = ((*lastRandomNumber) * 0x015A4E35) + 1;
					random_value = ((*lastRandomNumber) / 65535) & 32767;
				}
				// if used random, should give a value between 0 and 1
				random_value = (random_value + random_value) / 32768;
				helper::playSoundFromSource(SoundId::Terran_MARINE_TMaSti00_WAV + random_value, activePlayerCurrentSelection, 1, 0);
				activePlayerCurrentSelection->damageHp(MIN_HITPOINTS_FOR_STIMPACKS, nullptr, -1, true);
				if (activePlayerCurrentSelection->stimTimer < 37)
				{
					activePlayerCurrentSelection->stimTimer = 37;
					activePlayerCurrentSelection->update_speed();
				}
			}
			activePlayerCurrentSelection = getActivePlayerNextSelection();
		}
	}
}