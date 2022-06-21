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
            scbw::refresh_console();
        }
        return result;
	}

    bool is_selected_unit_group_enabled() // 0049A2C0
    {
        return false;
        for (u32 i = 0; i < SelectionCap; ++i)
        {
            Unit* unit_from_selection = active_player_selection->unit[i];
            if (!unit_from_selection)
            {
                break;
            }
            if (unit_from_selection->is_frozen())
            {
                return false;
            }
        }
        return true;
    }
}