#include "CMDRECV_Build.h"
#include <SCBW/api.h>

namespace hooks
{
	bool cmdrecv_place_building_allowed(Unit* builder, u8 order_id, u16 builtUnitId)
	{
		bool bReturnValue = false;
		if (order_id >= OrderId::DroneStartBuild && order_id <= OrderId::CTFCOP2)
		{
			if (order_id == OrderId::DroneStartBuild
			||	order_id == OrderId::BuildTerran
			||	order_id == OrderId::BuildProtoss1
			||	order_id == OrderId::BuildProtoss2
			||	order_id == OrderId::PlaceAddon)
			{
				if (builder->can_make_unit(builtUnitId, *ACTIVE_NATION_ID))
				{
					bReturnValue = (helper::order_is_allowed(builder, order_id, *ACTIVE_NATION_ID) != 0);
				}
			}
			else if (order_id == OrderId::Build5			// Build Nydus Exit
			||		 order_id == OrderId::BuildingLand)
			{
				if (builder->id == builtUnitId)
				{
					bReturnValue = (helper::order_is_allowed(builder, order_id, *ACTIVE_NATION_ID) != 0);
				}
			}
			else if (order_id == OrderId::CTFCOP2) // Create Flag Beacon
			{
				if (*elapsed_time_seconds <= 600 && builder->id == builtUnitId)
				{
					bReturnValue = (helper::order_is_allowed(builder, order_id, *ACTIVE_NATION_ID) != 0);
				}
			}
		}
		return bReturnValue;
	}

	void cmdrecv_place_building_normal(s16 x, s16 y, u32 order_id, u32 builtUnitId)
	{
		Unit* builder;
		u32 result_function_00473FB0;
		*unit_iterator_pos = 0;
		builder = get_active_player_next_selection();
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
			helper::function_0048DE70(builder, order_id, DimensionX, DimensionY, builtUnitId);
		}
	}

	void cmdrecv_place_addon(s16 x, s16 y, u32 order_id, u32 builtUnitId)
	{
		u16 addonPlaceX = 0;
		u16 addonPlaceY = 0;
		*unit_iterator_pos = 0;
		Unit* builder = get_active_player_next_selection();
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
			helper::function_0048DE70(builder, order_id, DimensionX, DimensionY, builtUnitId);
		}
	}

	void cmdrecv_build(u8 order_id, s16 x, s16 y, u16 builtUnitId)
	{
		Unit* builder = get_active_player_next_selection();
		*unit_iterator_pos = 0;
		if (builder)
		{
			if (!get_active_player_next_selection()) // multiple selection not allowed
			{
				if (x < map_tile_size->width
				&&	y < map_tile_size->height
				&&	helper::CMDRECV_place_building_allowed(builder, order_id, builtUnitId))
				{
					helper::CMDRECV_place_building(x, y, order_id, builtUnitId);
				}
			}
		}
	}

	// Hooked to try to allow for queueable build orders. -Pr0nogo
	void process_building_placement(Unit* unit, u8 order_id, Point16 pos, u16 target_unit_id) // 0048DE70
	{
		if (order_id == OrderId::BuildingLand)
		{
			helper::issue_order_targeting_ground(unit, OrderId::BuildingLand, pos.x, pos.y);
		}
		else
		{
			if  (unit->id != UnitId::ZergDroleth && unit->id != UnitId::ZergGosvileth
			|| ((unit->main_order_id != OrderId::DroneLand) || !(unit->status & 0x3000)) // CanNotReceiveOrders or NoBrkCodeStart
			&&   unit->main_order_id != OrderId::DroneBuild)
			{
				if ((unit->id != UnitId::ZergNydusCanal || order_id != OrderId::Build5) && order_id != OrderId::CTFCOP2)
				{
					ai_minerals_reserve[unit->player] = units_dat::mineral_cost[target_unit_id];
					ai_gas_reserve[unit->player] = units_dat::gas_cost[target_unit_id];
					if (helper::has_supplies_for_unit(unit->player, target_unit_id, 1))
					{
						u8 console_index_var = *console_index; // aids
						if (game_minerals[unit->player] >= ai_minerals_reserve[unit->player])
						{
							if (game_vespene[unit->player] >= ai_gas_reserve[unit->player])
							{
								helper::issue_order_targeting_ground(unit, order_id, pos.x, pos.y);
								if (units_dat::BaseProperty[unit->id] & 1 && units_dat::BaseProperty[target_unit_id] & 1)
								{
									helper::refund_all_queue_slots(unit);
									unit->building.addon_build_id = target_unit_id; // guessed
								}
								else
								{
									helper::refund_all_queue_slots(unit);
									helper::has_money_can_make(unit, target_unit_id);
								}
							}
							else
							{
								helper::display_error_message_id_with_sfx(unit->player, 851, console_index_var + 0x96);
							}
						}
						else
						{
							helper::display_error_message_id_with_sfx(unit->player, 850, console_index_var + 0x93);
						}
					}
				}
				else
				{
					helper::issue_order_targeting_ground(unit, order_id, pos.x, pos.y);
				}
			}
		}
	}
}