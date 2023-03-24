#include "briefing.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>

namespace hooks
{
	// Hooked in case we need it later - Pr0nogo
	void update_mission_objectives() // 0046CD30 | doesn't work currently due to null check late in the function
	{
		BinDlg* ready_room_var = *g_ready_room_dialog;
		if ((*g_ready_room_dialog)->control_type)
		{
			ready_room_var = (*g_ready_room_dialog)->parent;
		}

		BinDlg* objective_dialog = 0;
		BinDlg* child_dialog = ready_room_var->child_dialog;
		if (child_dialog)
		{
			while (child_dialog->index != -11)
			{
				child_dialog = child_dialog->next;
				if (!child_dialog)
				{
					goto LABEL_6;
				}
			}
			objective_dialog = child_dialog;
		}
		else
		{
		LABEL_6:
			objective_dialog = 0;
		}

		const char* objective_text;
		if (g_map_string_tbl)
		{
			u16 map_string_id = *((s16*)&g_mission_objectives_string_index + 2 * *g_local_nation_id) - 1;
			if (*((s16*)&g_mission_objectives_string_index + 2 * *g_local_nation_id))
			{
				if (map_string_id < *(s16*)g_map_string_tbl)
				{
					objective_text = (char*)g_map_string_tbl + *((u16*)g_map_string_tbl + map_string_id + 1);
				}
				else
				{
					objective_text = "";
				}
			}
			else
			{
				objective_text = 0;
			}
		}
		else
		{
			objective_text = 0;
		}

		if (!objective_dialog)
		{
			return;
		}
		else
		{
			objective_dialog->psz_text = (char*)objective_text;
			helper::show_dialog(objective_dialog);
			s32 objective_flags = objective_dialog->flags;
			if (!(objective_flags & BinDlgFlags::Unknown0))
			{
				objective_dialog->flags = objective_flags | BinDlgFlags::Unknown0;
				helper::update_dialog(objective_dialog);
			}
		}
	}
}