#include "neutral_related.h"
#include <SCBW/api.h>

namespace hooks
{
	bool unit_is_not_neutral(CHKUnit* unit) // 004CBE20
	{
		bool result = 0;
		if (gameData->starting_units)
		{
			if  (unit->player != 12
			|| ((unit->unit_id < UnitId::ResourceMineralField) || unit->unit_id > UnitId::ResourceMineralFieldType3)
			&&   unit->unit_id != UnitId::ResourceVespeneGeyser)
			{
				result = 1;
			}
		}
		return result;
	}
}