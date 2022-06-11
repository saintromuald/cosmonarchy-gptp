#include "btns_cond.h"
#include "../../SCBW/api.h"

namespace hooks
{

#define BUTTON_ENABLED 1
#define BUTTON_HIDDEN 0
#define BUTTON_DISABLED -1

	/// These hooks are a selection of buttons conditions
	/// showing how they work, and allowing these cases
	/// to be modified for a specific unit or situation.
	/// No more buttons conditions will be hooked, so
	/// use these for anything you need.

	s32 BTNSCOND_NoNydusExit(Unit* unit)
	{
		return (unit->building.nydus_exit == nullptr) ? BUTTON_ENABLED : BUTTON_HIDDEN;
	}

	;

	s32 BTNSCOND_Movement(Unit* unit)
	{

		s32 return_value = BUTTON_ENABLED;

		for (s32 i = 0; return_value == BUTTON_ENABLED && i < SelectionCap; i++)
			if (client_selection_group->unit[i] != nullptr && client_selection_group->unit[i]->status & UnitStatus::Burrowed)
				return_value = BUTTON_HIDDEN;

		return return_value;

	}

	;

	s32 BTNSCOND_HasScarabs(Unit* unit)
	{

		s32 return_value = BUTTON_DISABLED;

		for (s32 i = 0; return_value == BUTTON_DISABLED && i < SelectionCap; i++)
			if (client_selection_group->unit[i] != nullptr &&
				(client_selection_group->unit[i]->id == UnitId::Hero_Warbringer || client_selection_group->unit[i]->id == UnitId::ProtossAccantor) &&
				client_selection_group->unit[i]->unit_with_hangar.in_hangar_count != 0
				)
				return_value = BUTTON_ENABLED;

		return return_value;

	}

#undef BUTTON_ENABLED
#undef BUTTON_HIDDEN
#undef BUTTON_DISABLED 

} //hooks