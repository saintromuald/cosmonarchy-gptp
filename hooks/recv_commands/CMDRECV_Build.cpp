#include "CMDRECV_Build.h"
#include <SCBW/api.h>

namespace hooks {
	bool CMDRECV_PlaceBuildingAllowed(CUnit* builder, uint8 orderId, uint16 builtUnitId) {
		bool bReturnValue = false;
		if (orderId >= OrderId::DroneStartBuild && orderId <= OrderId::CTFCOP2) {
			if (
				orderId == OrderId::DroneStartBuild ||
				orderId == OrderId::BuildTerran ||
				orderId == OrderId::BuildProtoss1 ||
				orderId == OrderId::BuildProtoss2 ||
				orderId == OrderId::PlaceAddon
				) {
				if (builder->canMakeUnit(builtUnitId, *ACTIVE_NATION_ID))
					bReturnValue = (OrderAllowed(builder, orderId, *ACTIVE_NATION_ID) != 0);
			} else
				if (
					orderId == OrderId::Build5 ||		//Build Nydus Exit
					orderId == OrderId::BuildingLand
					) {
					if (builder->id == builtUnitId)
						bReturnValue = (OrderAllowed(builder, orderId, *ACTIVE_NATION_ID) != 0);
				} else
					if (orderId == OrderId::CTFCOP2) {	//Create flag beacon
						if (*elapsedTimeSeconds <= 600 && builder->id == builtUnitId)
							bReturnValue = (OrderAllowed(builder, orderId, *ACTIVE_NATION_ID) != 0);
					}
		}
		return bReturnValue;
	}

	void cmdRECV_PlaceBuildingNormal(int16 x, int16 y, uint32 orderId, uint32 builtUnitId) {

		CUnit* builder;
		uint32 result_function_00473FB0;

		*selectionIndexStart = 0;
		builder = getActivePlayerNextSelection();

		result_function_00473FB0 =
			function_00473FB0(
				builder,
				builder->playerId,
				x,
				y,
				builtUnitId,
				1,
				0,
				0,
				0
			);

		if (placeBuildingMsg(result_function_00473FB0)) {

			int32 DimensionX, DimensionY;

			DimensionX = (int16)units_dat::BuildingDimensions[builtUnitId].x;

			if (DimensionX < 0)
				DimensionX++;

			DimensionX /= 2;

			DimensionX += (x * 32);

			DimensionY = (int16)units_dat::BuildingDimensions[builtUnitId].y;

			if (DimensionY < 0)
				DimensionY++;

			DimensionY /= 2;

			DimensionY += (y * 32);

			function_0048DE70(builder, orderId, DimensionX, DimensionY, builtUnitId);

		}

	}

	;

	void cmdRECV_PlaceBuildingAddon(int16 x, int16 y, uint32 orderId, uint32 builtUnitId) {

		CUnit* builder;
		uint32 result_function_00473FB0;

		uint16 addonPlaceX = 0, addonPlaceY = 0;

		*selectionIndexStart = 0;
		builder = getActivePlayerNextSelection();

		result_function_00473FB0 =
			function_00473FB0(
				builder,
				builder->playerId,
				x,
				y,
				builtUnitId,
				1,
				0,
				0,
				0
			);

		if (result_function_00473FB0 == 0) {

			addonPlaceX = (int16)units_dat::AddonPlacement[builtUnitId - UnitId::TerranCommandCenter].x;
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
			scbw::adjustAddonX(builtUnitId, builder->id, addonPlaceX);

			if (addonPlaceX < 0)
				addonPlaceX += 31;

			addonPlaceX /= 32;

			addonPlaceX = x - addonPlaceX;

			addonPlaceY = (int16)units_dat::AddonPlacement[builtUnitId - UnitId::TerranCommandCenter].y;
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
			scbw::adjustAddonY(builtUnitId, builder->id, addonPlaceY);
			if (addonPlaceY < 0)
				addonPlaceY += 31;

			addonPlaceY /= 32;

			addonPlaceY = y - addonPlaceY;

			result_function_00473FB0 =
				function_00473FB0(
					builder,
					builder->playerId,
					addonPlaceX,
					addonPlaceY,
					builder->id,
					0,
					0,
					0,
					0
				);

		}

		if (placeBuildingMsg(result_function_00473FB0)) {

			int32 DimensionX, DimensionY;

			DimensionX = (int16)units_dat::BuildingDimensions[builder->id].x;

			if (DimensionX < 0)
				DimensionX++;

			DimensionX /= 2;

			addonPlaceX *= 32;

			DimensionX += addonPlaceX;

			DimensionY = (int16)units_dat::BuildingDimensions[builder->id].y;

			if (DimensionY < 0)
				DimensionY++;

			DimensionY /= 2;

			addonPlaceY *= 32;

			DimensionY += addonPlaceY;

			function_0048DE70(builder, orderId, DimensionX, DimensionY, builtUnitId);

		}

	}

	;

	void CMDRECV_Build(uint8 orderId, int16 x, int16 y, uint16 builtUnitId) {

		CUnit* builder;

		*selectionIndexStart = 0;
		builder = getActivePlayerNextSelection();

		if (builder != nullptr) {

			if (getActivePlayerNextSelection() == nullptr) { //multiple selection not allowed

				if (
					x < mapTileSize->width &&
					y < mapTileSize->height &&
					CMDRECV_PlaceBuildingAllowedHelper(builder, orderId, builtUnitId)
					)
					cmdRECV_PlaceBuilding(x, y, orderId, builtUnitId);
			}
		}
	}
}