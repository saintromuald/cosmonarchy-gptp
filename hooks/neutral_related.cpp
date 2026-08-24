#include "neutral_related.h"
#include <BW/api.h>

namespace hooks {
	bool unit_is_not_neutral(ChkUnit* unit) { // 004CBE20
		if (game_data->starting_units == StartingUnits::MapUnits) {
			return false;
		}
		if (unit->player != 11) {
			return true;
		}
		if (!(units_dat::base_property[unit->unit_id] & UnitProperty::ResourceContainer)) {
			return true;
		}
		return false;
	}
} // namespace hooks