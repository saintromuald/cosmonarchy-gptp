#include "glucustom.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace hooks
{
	void displayEstablishingShot() // 004D9200, hooked by X405
	{
		// This is the original original function which loads the establishing shots in campaigns:
		if (!*multiPlayerMode && !(*CHEAT_STATE & CheatFlags::NoGlues) && *campaignIndex)
		{
			for (s32 i = 0; i < 65; i++)
			{
				if (campaignMissions[i].missionIndex == *campaignIndex)
				{
					helper::loadInitCreditsBIN(campaignMissions[i].missionName);
				}
			}
		}

		// This addition enables establishing shots in custom scenarios:
		else if (*campaignIndex == 0 && current_map_filename)
		{
			helper::SFileOpenArchive(current_map_filename, 0, 0, mapArchiveHandle);
			void* handle;
			bool establishingShotExists = helper::SFileOpenFileEx(*mapArchiveHandle, "rez\\est.txt", 0, &handle);
			if (handle)
			{
				helper::SFileCloseFile(handle);
			}

			if (establishingShotExists)
			{
				helper::loadInitCreditsBIN("est");
			}

			if (*mapArchiveHandle)
			{
				helper::SFileCloseArchive(*mapArchiveHandle);
			}
		}
	}

	// Hooked to fix crash after removing game speed settings. -Pr0nogo
	void CMDACT_game_speed(BinDlg* control) // 00481EF0
	{
		BinDlg* control_var = control;
		if (*last_control_index == -2)
		{
			if (control->control_type)
			{
				control = control->parent;
			}
			BinDlg* child = control->children_dialog;
			if (child)
			{
				while (child->index != 1)
				{
					child = child->next;
					if (!child)
					{
						goto LABEL_7;
					}
				}
			}
			else
			{
			LABEL_7:
				child = 0;
			}
			if (child && !(child->flags & 2))
			{
				char min_value = child->scrollDialog0x36.scrollMinValue;
				u8 game_command = 15;
				helper::queue_game_command(&game_command, 2u);
			}
		}
		else
		{
			*cpu_throttle = *temp_cpu_throttle;
		}
		helper::process_options_change(control_var);
		char interacting_bin = (*bin_interact)-- - 1;
		if (*bin_interact)
		{
			*bin_interact = interacting_bin - 1;
			*last_bindlg_menu_function = helper::main_menu_options_custom_interact;
			helper::load_dialog("rez\\options.bin", helper::game_menu_dialog_interact);
		}
		else
		{
			helper::destroy_dialog(control_var);
		}
	}
}