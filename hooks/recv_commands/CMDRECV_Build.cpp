#include "CMDRECV_Build.h"
#include <SCBW/api.h>

namespace hooks
{
	bool cmdrecv_place_building_allowed(Unit* builder, u8 orderId, u16 builtUnitId)
	{
		bool bReturnValue = false;
		if (orderId >= OrderId::DroneStartBuild && orderId <= OrderId::CTFCOP2)
		{
			if (orderId == OrderId::DroneStartBuild
			||	orderId == OrderId::BuildTerran
			||	orderId == OrderId::BuildProtoss1
			||	orderId == OrderId::BuildProtoss2
			||	orderId == OrderId::PlaceAddon)
			{
				if (builder->can_make_unit(builtUnitId, *ACTIVE_NATION_ID))
				{
					bReturnValue = (helper::order_is_allowed(builder, orderId, *ACTIVE_NATION_ID) != 0);
				}
			}
			else if (orderId == OrderId::Build5			// Build Nydus Exit
			||		 orderId == OrderId::BuildingLand)
			{
				if (builder->id == builtUnitId)
				{
					bReturnValue = (helper::order_is_allowed(builder, orderId, *ACTIVE_NATION_ID) != 0);
				}
			}
			else if (orderId == OrderId::CTFCOP2) // Create Flag Beacon
			{
				if (*elapsedTimeSeconds <= 600 && builder->id == builtUnitId)
				{
					bReturnValue = (helper::order_is_allowed(builder, orderId, *ACTIVE_NATION_ID) != 0);
				}
			}
		}
		return bReturnValue;
	}

	void cmdrecv_place_building_normal(s16 x, s16 y, u32 orderId, u32 builtUnitId)
	{
		Unit* builder;
		u32 result_function_00473FB0;
		*selectionIndexStart = 0;
		builder = getActivePlayerNextSelection();
		result_function_00473FB0 = helper::can_build_on_tile_location(builder, builder->player, x, y, builtUnitId, 1, 0, 0, 0);

		if (helper::place_building_message(result_function_00473FB0))
		{
			s32 DimensionX, DimensionY;
			DimensionX = (s16)units_dat::BuildingDimensions[builtUnitId].x;
			if (DimensionX < 0)
			{
				DimensionX++;
			}
			DimensionX /= 2;
			DimensionX += (x * 32);
			DimensionY = (s16)units_dat::BuildingDimensions[builtUnitId].y;
			if (DimensionY < 0)
			{
				DimensionY++;
			}
			DimensionY /= 2;
			DimensionY += (y * 32);
			helper::function_0048DE70(builder, orderId, DimensionX, DimensionY, builtUnitId);
		}
	}

	void cmdrecv_place_building_addon(s16 x, s16 y, u32 orderId, u32 builtUnitId)
	{
		u16 addonPlaceX = 0;
		u16 addonPlaceY = 0;
		*selectionIndexStart = 0;
		Unit* builder = getActivePlayerNextSelection();
		s32 can_build = helper::can_build_on_tile_location(builder, builder->player, x, y, builtUnitId, 1, 0, 0, 0);
		if (!can_build)
		{
			addonPlaceX = (s16)units_dat::AddonPlacement[builtUnitId - UnitId::TerranCommandCenter].x;
			/*if (builtUnitId == UnitId::TerranFutureStation) {
				if (builder->id == UnitId::academy || builder->id == UnitId::armory) {
					addonPlaceX = 96;
				}
				else if (builder->id == UnitId::TerranDrydock) {
					addonPlaceX = 96;
				}
			}
			else if (builtUnitId == UnitId::TerranQuarry) {
				addonPlaceX = 128;
			}*/
			scbw::adjust_addon_x(builtUnitId, builder->id, &addonPlaceX);
			if (addonPlaceX < 0)
			{
				addonPlaceX += 31;
			}
			addonPlaceX /= 32;
			addonPlaceX = x - addonPlaceX;
			addonPlaceY = (s16)units_dat::AddonPlacement[builtUnitId - UnitId::TerranCommandCenter].y;
			/*if (builtUnitId == UnitId::TerranFutureStation) {
				if (builder->id == UnitId::academy || builder->id == UnitId::armory) {
					addonPlaceY = 0;
				}
				else if (builder->id == UnitId::TerranDrydock) {
					addonPlaceY = 48;
				}
			}
			else if (builtUnitId == UnitId::TerranQuarry) {
				addonPlaceY = 32;
			}*/
			scbw::adjust_addon_y(builtUnitId, builder->id, &addonPlaceY);
			if (addonPlaceY < 0)
			{
				addonPlaceY += 31;
			}
			addonPlaceY /= 32;
			addonPlaceY = y - addonPlaceY;
			can_build = helper::can_build_on_tile_location(builder, builder->player, addonPlaceX, addonPlaceY, builder->id, 0, 0, 0, 0);
		}

		if (helper::place_building_message(can_build))
		{
			s32 DimensionX, DimensionY;
			DimensionX = (s16)units_dat::BuildingDimensions[builder->id].x;
			if (DimensionX < 0)
			{
				DimensionX++;
			}
			DimensionX /= 2;
			addonPlaceX *= 32;
			DimensionX += addonPlaceX;
			DimensionY = (s16)units_dat::BuildingDimensions[builder->id].y;
			if (DimensionY < 0)
			{
				DimensionY++;
			}
			DimensionY /= 2;
			addonPlaceY *= 32;
			DimensionY += addonPlaceY;
			helper::function_0048DE70(builder, orderId, DimensionX, DimensionY, builtUnitId);
		}
	}

	void CMDRECV_Build(u8 orderId, s16 x, s16 y, u16 builtUnitId)
	{
		Unit* builder;
		*selectionIndexStart = 0;
		builder = getActivePlayerNextSelection();
		if (builder != nullptr)
		{
			if (getActivePlayerNextSelection() == nullptr) // multiple selection not allowed
			{
				if (x < mapTileSize->width
				&&	y < mapTileSize->height
				&&	helper::CMDRECV_place_building_allowed(builder, orderId, builtUnitId))
				{
					helper::CMDRECV_place_building(x, y, orderId, builtUnitId);
				}
			}
		}
	}
}