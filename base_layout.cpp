#include "base_layout.h"

static std::unordered_map<u32, std::vector<BaseLayout>> layouts;

void init_base_layouts()
{
	layouts.clear();

	for (u32 i = 0; i < 256; ++i)
	{
		Location location = locationTable[i];

		char buffer[64];
		const char* name = mapStringTbl->getString(location.stringId);

		strcpy(buffer, name);

		char* prefix = buffer;
		char* player_str = strtok(prefix, "_");
		char* res_area_str = strtok(player_str, "_");

		if (strcmp(prefix, "Town") != 0)
		{
			continue;
		}

		u8 player_id = atoi(player_str);
		u8 res_area = atoi(res_area_str);

		UnitArray arr = unit_search_rect(location.dimensions);

		for (u32 j = 0; j < arr.size; ++j)
		{
			Unit* unit = arr.units[j];

			BaseLayout layout;
			layout.player_id = player_id;
			layout.unit_id = unit->id;
			layout.position = unit->position;
			layout.priority = 100.0 * unit->hitPoints / unit->getMaxHpRaw();
			layout.active = false;
			layout.town = nullptr;
			layouts[res_area].push_back(layout);

			unit->remove(); // TODO: find a silent remove function
		}
	}
}

void update_base_layouts()
{
	// Set New Layouts
	for (u32 i = 0; i < 100; ++i)
	{
		AiTown* town = &AiArray[i];
		if (town->inited && !town->resource_area_not_set)
		{
			for (BaseLayout& layout : layouts[town->resource_area])
			{
				if (units_dat::BaseProperty[layout.unit_id] & UnitProperty::Building)
				{
					scbw::sendGPTP_aise_cmd((Unit*)&layout, (Unit*)town, GptpId::SetBaseLayout, 0, 0, 0);
				}
				else
				{
					scbw::sendGPTP_aise_cmd((Unit*)&layout, (Unit*)town, GptpId::SetGuard, 0, 0, 0);
				}
				layout.active = true;
				layout.town = town; // hopefully town id can be handled from aise
			}
		}
	}

	// Clear layouts
	for (auto& pair : layouts)
	{
		for (BaseLayout& layout : pair.second)
		{
			if (layout.active && !layout.town->inited)
			{
				if (units_dat::BaseProperty[layout.unit_id] & UnitProperty::Building)
				{
					scbw::sendGPTP_aise_cmd((Unit*)&layout, 0, GptpId::ResetBaseLayout, 0, 0, 0);
				}
				else
				{
					scbw::sendGPTP_aise_cmd((Unit*)&layout, 0, GptpId::ResetGuard, 0, 0, 0);
				}
				layout.active = false;
				layout.town = nullptr;
			}
		}
	}
}
