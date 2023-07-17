#include "CMDRECV_Build.h"
#include <SCBW/api.h>

namespace hooks
{
	bool cmdrecv_place_building_allowed(Unit* builder, u8 order_id, u16 built_unit_id) // 0048DBD0
	{
		bool is_allowed = false;
		if (order_id >= OrderId::DroneStartBuild && order_id <= OrderId::CTFCOP2)
		{
			if (order_id == OrderId::DroneStartBuild
			||	order_id == OrderId::BuildTerran
			||	order_id == OrderId::BuildProtoss1
			||	order_id == OrderId::BuildProtoss2
			||	order_id == OrderId::PlaceAddon)
			{
				if (builder->can_make_unit(built_unit_id, *g_active_nation_id))
				{
					is_allowed = (helper::order_is_allowed(builder, order_id, *g_active_nation_id) != 0);
				}
			} else if (order_id == OrderId::Build5			// Build Nydus Exit
			||		   order_id == OrderId::BuildingLand)
			{
				if (builder->id == built_unit_id)
				{
					is_allowed = (helper::order_is_allowed(builder, order_id, *g_active_nation_id) != 0);
				}
			} else if (order_id == OrderId::CTFCOP2) // Create Flag Beacon
			{
				if (*g_elapsed_time_seconds <= 600 && builder->id == built_unit_id)
				{
					is_allowed = (helper::order_is_allowed(builder, order_id, *g_active_nation_id) != 0);
				}
			}
		}
		return is_allowed;
	}

	void cmdrecv_place_building_normal(s16 x, s16 y, u32 order_id, u32 built_unit_id) // 0048E010
	{
		*unit_iterator_pos = 0;
		Unit* builder = get_active_player_next_selection();
		u32 can_build = helper::can_build_on_tile_location(builder, builder->player, x, y, built_unit_id, 1, 0, 0, 0);

		if (helper::place_building_message(can_build))
		{
			s32 DimensionX, DimensionY;
			DimensionX = (s16)units_dat::building_dimensions[built_unit_id].x;
			if (DimensionX < 0)
			{
				DimensionX++;
			}
			DimensionX /= 2;
			DimensionX += (x * 32);
			DimensionY = (s16)units_dat::building_dimensions[built_unit_id].y;
			if (DimensionY < 0)
			{
				DimensionY++;
			}
			DimensionY /= 2;
			DimensionY += (y * 32);
			helper::process_building_placement(builder, order_id, DimensionX, DimensionY, built_unit_id);
		}
	}

	void cmdrecv_place_addon(s16 x, s16 y, u32 order_id, u32 built_unit_id) // 0048E0A0
	{
		u16 addonPlaceX = 0;
		u16 addonPlaceY = 0;
		*unit_iterator_pos = 0;
		Unit* builder = get_active_player_next_selection();
		s32 can_build = helper::can_build_on_tile_location(builder, builder->player, x, y, built_unit_id, 1, 0, 0, 0);
		if (!can_build)
		{
			addonPlaceX = (s16)units_dat::addon_placement[built_unit_id - UnitId::TerranMinistry].x;
			scbw::adjust_addon_x(built_unit_id, builder->id, &addonPlaceX);
			if (addonPlaceX < 0)
			{
				addonPlaceX += 31;
			}
			addonPlaceX /= 32;
			addonPlaceX = x - addonPlaceX;
			addonPlaceY = (s16)units_dat::addon_placement[built_unit_id - UnitId::TerranMinistry].y;
			scbw::adjust_addon_y(built_unit_id, builder->id, &addonPlaceY);
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
			DimensionX = (s16)units_dat::building_dimensions[builder->id].x;
			if (DimensionX < 0)
			{
				DimensionX++;
			}
			DimensionX /= 2;
			addonPlaceX *= 32;
			DimensionX += addonPlaceX;
			DimensionY = (s16)units_dat::building_dimensions[builder->id].y;
			if (DimensionY < 0)
			{
				DimensionY++;
			}
			DimensionY /= 2;
			addonPlaceY *= 32;
			DimensionY += addonPlaceY;
			helper::process_building_placement(builder, order_id, DimensionX, DimensionY, built_unit_id);
		}
	}

	void cmdrecv_build(u8 order_id, s16 x, s16 y, u16 built_unit_id) // 004C23C0
	{
		*unit_iterator_pos = 0;
		Unit* builder = get_active_player_next_selection();
		if (builder)
		{
			if (!get_active_player_next_selection()) // multiple selection not allowed
			{
				if (x < g_map_tile_size->width
				&&	y < g_map_tile_size->height
				&&	helper::CMDRECV_place_building_allowed(builder, order_id, built_unit_id))
				{
					helper::CMDRECV_place_building(x, y, order_id, built_unit_id);
				}
			}
		}
	}

	// Hooked to try to allow for queueable build orders. -Pr0nogo
	void process_building_placement(Unit* unit, u8 order_id, Point16 pos, u16 target_unit_id) // 0048DE70
	{
		if (order_id == OrderId::BuildingLand) {
			helper::issue_order_targeting_ground(unit, OrderId::BuildingLand, pos.x, pos.y);
		} else {
			if  (unit->id != UnitId::ZergDroleth && unit->id != UnitId::ZergGosvileth
			|| ((unit->main_order_id != OrderId::DroneLand) || !(unit->status & (UnitStatus::CanNotReceiveOrders|UnitStatus::NoBrkCodeStart)))
			&&   unit->main_order_id != OrderId::DroneBuild)
			{
				if ((unit->id != UnitId::ZergNydusCesiant || order_id != OrderId::Build5) && order_id != OrderId::CTFCOP2)
				{
					g_ai_minerals_reserve[unit->player] = units_dat::mineral_cost[target_unit_id];
					g_ai_vespene_reserve[unit->player] = units_dat::vespene_cost[target_unit_id];
					if (g_game_minerals[unit->player] >= g_ai_minerals_reserve[unit->player])
					{
						if (g_game_vespene[unit->player] >= g_ai_vespene_reserve[unit->player])
						{
							helper::issue_order_targeting_ground(unit, order_id, pos.x, pos.y);
							if (units_dat::base_property[unit->id] & UnitProperty::Building && units_dat::base_property[target_unit_id] & UnitProperty::Addon)
							{
								helper::refund_all_queue_slots(unit);
								unit->building.addon_build_id = target_unit_id;
							} else {
								helper::refund_all_queue_slots(unit);
								helper::has_money_can_make(unit, target_unit_id);
							}
						} else {
							helper::display_error_message_id_with_sfx(unit->player, 851, *g_console_index + 0x96);
						}
					} else {
						helper::display_error_message_id_with_sfx(unit->player, 850, *g_console_index + 0x93);
					}
				} else {
					helper::issue_order_targeting_ground(unit, order_id, pos.x, pos.y);
				}
			}
		}
	}

	// Hooked to debug Pylon-based construction. -Pr0nogo
	u32 cmdrecv_place_building(u32 order_id, u16 built_unit_id, Point16 position) // 0048E190
	{
		if (units_dat::base_property[built_unit_id] & UnitProperty::Addon)
		{
			cmdrecv_place_addon(position.x, position.y, order_id, built_unit_id);
		} else {
			cmdrecv_place_building_normal(position.x, position.y, order_id, built_unit_id);
		}
		scbw::refresh_console();
		return 0;
	}
}