#include "selection_extender.h"
#include "SCBW/api.h"

namespace hooks {
	s32 select_dual_birth_units(Unit* unit) { // 0045D040
        u32 selection_index = 0;
        Unit* selection_array[SelectionCap];
        memcpy(selection_array, active_player_selection, sizeof(selection_array));
        s32 result = 0;
        while (selection_array[result])
        {
            if (++result >= SelectionCap)
            {
                return result;
            }
        }
        if (result < 12)
        {
            selection_index = result + 1;
            selection_array[result] = unit;
            helper::create_new_unit_selections_from_list((Unit**)selection_array, result + 1);
            helper::cmdact_select(selection_array, selection_index);
            result = 1;
            *client_selection_changed = 1;
            *force_button_refresh = 1;
            portrait_redraw[0] = 1;
            *status_screen_redraw = 1;
            *control_under_mouse = 0;
            *control_under_mouse_value = 0;
        }
        return result;
	}

    bool is_selected_unit_group_enabled() // 0049A2C0
    {
        bool result = 1;
        Unit** unit_selection = active_player_selection->unit;
        do
        {
            Unit* unit_from_selection = *unit_selection;
            if (!*unit_selection)
            {
                break;
            }
            if (!unit_from_selection->is_frozen()) // Originally also checked for DisabledDoodad, not sure if we need that too. -Pr0nogo
            {
                return 0;
            }
            ++unit_selection;
        } while (unit_selection < players_selections->unit[0]); // [0] is wrong, but I don't yet know how to translate this from IDA to GPTP. -Pr0nogo
        return result;
    }
}