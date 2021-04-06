#include "btns_cond.h"
#include "../../SCBW/api.h"

namespace hooks {

#define BUTTON_ENABLED 1
#define BUTTON_HIDDEN 0
#define BUTTON_DISABLED -1

	/// These hooks are a selection of buttons conditions
	/// showing how they work, and allowing these cases
	/// to be modified for a specific unit or situation.
	/// No more buttons conditions will be hooked, so
	/// use these for anything you need.

	int32 BTNSCOND_NoNydusExit(CUnit* unit) {
		return (unit->building.nydusExit == nullptr) ? BUTTON_ENABLED : BUTTON_HIDDEN;
	}

	;

	int32 BTNSCOND_Movement(CUnit* unit) {

		int32 return_value = BUTTON_ENABLED;

		for (int i = 0; return_value == BUTTON_ENABLED && i < SELECTION_ARRAY_LENGTH; i++)
			if (clientSelectionGroup->unit[i] != nullptr && clientSelectionGroup->unit[i]->status & UnitStatus::Burrowed)
				return_value = BUTTON_HIDDEN;

		return return_value;

	}

	;

	int32 BTNSCOND_HasScarabs(CUnit* unit) {

		int32 return_value = BUTTON_DISABLED;

		for (int i = 0; return_value == BUTTON_DISABLED && i < SELECTION_ARRAY_LENGTH; i++)
			if (clientSelectionGroup->unit[i] != nullptr &&
				(clientSelectionGroup->unit[i]->id == UnitId::Hero_Warbringer || clientSelectionGroup->unit[i]->id == UnitId::ProtossAccantor) &&
				clientSelectionGroup->unit[i]->carrier.inHangarCount != 0
				)
				return_value = BUTTON_ENABLED;

		return return_value;

	}

#undef BUTTON_ENABLED
#undef BUTTON_HIDDEN
#undef BUTTON_DISABLED 

} //hooks