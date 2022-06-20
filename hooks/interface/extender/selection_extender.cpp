#include "selection_extender.h"
#include "SCBW/api.h"

namespace hooks {
	s32 select_dual_birth_units(Unit* unit) { // 0045D040
        u32 selection_index = 0;
        Unit* selection_array[SELECTION_LIMIT];
        memcpy(selection_array, active_player_selection, sizeof(selection_array));
        s32 result = 0;
        while (selection_array[result])
        {
            if (++result >= SELECTION_LIMIT)
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
}