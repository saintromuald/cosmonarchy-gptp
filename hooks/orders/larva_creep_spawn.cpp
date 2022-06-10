#include "larva_creep_spawn.h"
#include "building_making\building_modifier.h"
#include <SCBW/api.h>
#include "SCBW\UnitFinder.h"
#include "globals.h"

namespace hooks
{
	helper::setCreepRelationalData_callback runCreepRandomizerPtr = &runCreepRandomizer;
	void function_0049D660(Unit* main_building)
	{
		Vector32 coords;
		// look for a location to spawn a larva at
		if (helper::getLarvaSpawnPos(main_building, &coords))
		{
			Unit* unit_created = helper::create_unit(UnitId::ZergLarva, coords.x, coords.y, main_building->player);
			if (unit_created)
			{
				helper::update_unit_stats_and_finish_building(unit_created);
				helper::update_unit_strength(unit_created);
				unit_created->connected_unit = main_building;

				// set main_order_state depending on the collision state with the connected building			
				helper::function_004E8DA0(unit_created);
				if (main_building->unit_ai != nullptr)
				{
					helper::ai_training_overlord(unit_created, main_building);
				}
			}
		}
	}

	// filter for larva in internal game searches, use the limit of 3 larvas
	bool function_004E8C80(Unit* unit, Unit* main_building)
	{
		// while bReturnValue == false, the calling function keep calling,
		// so it return true only when max amount of larvas was counted,
		// since the counting itself is made with LARVA_COUNTER here and
		// not in the calling function.
		bool bReturnValue;
		if (unit->id == UnitId::ZergLarva)
		{
			if (unit->sprite->flags & SpriteFlags::Hidden)
			{
				bReturnValue = false;
			}
			else
				if (unit->connected_unit != main_building)
				{
					bReturnValue = false;
				}
				else
				{
					(*larvaCounter)++;
					bReturnValue = (*larvaCounter > 3);
				}
		}
		else
		{
			bReturnValue = false;
		}

		return bReturnValue;
	}

	void secondaryOrd_SpawningLarva(Unit* unit)
	{
		if (unit->status & UnitStatus::IsHallucination) {
			return;
		}
		if (unit->order_queue_timer == 0 && unit->player != 11)
		{
			// 004EA79E
			*larvaCounter = 0;
			Box16 coords;
			coords.left = unit->sprite->position.x - 256;
			coords.top = unit->sprite->position.y - 256;
			coords.right = unit->sprite->position.x + 256;
			coords.bottom = unit->sprite->position.y + 256;

			// Count existing larvas in LARVA_COUNTER
			helper::IterateUnitsAtLocationTargetProc_LarvaCount(unit, &coords);
			if (*larvaCounter < 3 && !(unit->status & UnitStatus::InAir))
			{
				if (unit->building.larva_timer != 0)
				{
					unit->building.larva_timer--;

					BuildingModifier modifier = get_building_modifier(unit, 0, false);
					if (modifier.decrease) {
						if (unit->building.larva_timer > modifier.decrease) {
							unit->building.larva_timer -= modifier.decrease;
						} else {
							unit->building.larva_timer = 0;
						}
					}
				}

				if (unit->building.larva_timer == 0)
				{
					// 004EA80E:
					unit->building.larva_timer = LarvaSpawnRateHatchery;
					if (unit->id == UnitId::ZergLarvalColony)
					{
						unit->building.larva_timer /= LarvaFactorLarvalColony;
					}
					else if (unit->id == UnitId::ZergLair && unit->main_order_id != OrderId::Morph2
						|| unit->id == UnitId::ZergHive && unit->main_order_id == OrderId::Morph2)
					{
						unit->building.larva_timer /= LarvaFactorLair;
					}
					else if (unit->id == UnitId::ZergHive && unit->main_order_id != OrderId::Morph2
						|| unit->id == UnitId::ZergSire && unit->main_order_id == OrderId::Morph2)
					{
						unit->building.larva_timer /= LarvaFactorHive;
					}
					else if (unit->id == UnitId::ZergSire && unit->main_order_id != OrderId::Morph2)
					{
						unit->building.larva_timer /= LarvaFactorSire;
					}

					// if failed to find a spawn point for larva set the timer before retry to 3
					Vector32 coords_2;
					if (!helper::getLarvaSpawnPos(unit, &coords_2))
					{
						unit->building.larva_timer = 3;
					}
					else
					{
						// 004EA830:
						Unit* new_larva = helper::create_unit(UnitId::ZergLarva, coords_2.x, coords_2.y, unit->player);
						if (new_larva != nullptr)
						{
							// update various stats (set hp, set shield...) not finished on CreateUnit
							helper::update_unit_stats_and_finish_building(new_larva);
							helper::update_unit_strength(new_larva);

							new_larva->connected_unit = unit;
							// set main_order_state depending on the collision state with the connected building						
							helper::function_004E8DA0(new_larva);

							if (unit->unit_ai != nullptr)
							{
								helper::ai_training_overlord(new_larva, unit);
							}
						}
					}
				}
			}
		}
	}

	void secondaryOrd_SpreadCreepSpawningLarva(Unit* unit)
	{
		if (unit->id == UnitId::ZergHatchery
			|| unit->id == UnitId::ZergLair
			|| unit->id == UnitId::ZergHive
			|| unit->id == UnitId::ZergSire
			|| unit->id == UnitId::ZergLarvalColony)
		{
			secondaryOrd_SpawningLarva(unit);
		}
		if (unit->building.creep_timer != 0)
		{
			if (unit->building.creep_timer >= 2)
			{
				unit->building.creep_timer -= 2;

				BuildingModifier modifier = get_building_modifier(unit, 0, false);
				if (modifier.decrease) {
					if (unit->building.creep_timer > modifier.decrease) {
						unit->building.creep_timer -= modifier.decrease;
					} else {
						unit->building.creep_timer = 0;
					}
				}
			}
			else
			{
				unit->building.creep_timer = 0;
			}

		}
		else
		{
			// 004EA8B6:
			u8 result_00414680;

			unit->building.creep_timer = 15;

			// look for a place to spawn creep on
			// return value == 0 if all potential tiles are already creep, 2 == potential tiles are occupied by another building, 1 == potential tiles are otherwise still available
			// Replace UnitId::ZergHive with unit->id if giving specific buildings different creep bounding rectangles in getCreepRegion
			result_00414680 = helper::spreadCreep(unit->id, unit->sprite->position.x, unit->sprite->position.y);
			if (unit->id == 180 && result_00414680 != 0)
			{
				u32 current_tile_x = unit->sprite->position.x / 32;
				u32 current_tile_y = unit->sprite->position.y / 32;
				s32 width_x = 6;
				s32 width_y = 4;
				u8 mustspread = 0;
				s32 x1 = current_tile_x - width_x;
				s32 y1 = current_tile_y - width_x;
				s32 x2 = current_tile_x + width_x;
				s32 y2 = current_tile_y + width_y;

				if (x1 < 0)
				{
					x1 = 0;
				}
				if (y1 < 0)
				{
					y1 = 0;
				}
				if (x2 >= map_tile_size->width)
				{
					x2 = map_tile_size->width - 1;
				}
				if (y2 >= map_tile_size->height)
				{
					y2 = map_tile_size->height - 1;
				}

				for (s32 i = x1; i <= x2; i++)
				{
					for (s32 j = y1; j <= y2; j++)
					{
						s32 x = (i * 32) + 16;
						s32 y = (j * 32) + 16;
						s32 X = (x - (current_tile_x * 32));
						s32 Y = (y - (current_tile_y * 32));
						bool in_range = ((256 * X * X) + (100 * Y * Y)) < 1300000;
						// adjust value 1300000 to get slightly different radius

						if (in_range)
						{
							ActiveTile a = scbw::get_stupid_reference_to_active_tile_at(i * 32, j * 32);
							if (a.has_creep || a.temporary_creep || a.creep_receeding)
							{
								// why is there nothing here LMAO
							}
							else
							{
								// can spread
								if (!a.always_unbuildable && a.is_walkable)
								{

									mustspread = 1;
								}
							}
						}
					}
				}
				if (!mustspread)
				{
					result_00414680 = 0;
				}
			}
			if (result_00414680 == 0)
			{
				// 004EA8DC:

				bool isZergMainBuilding =
					unit->id == UnitId::ZergHatchery
					|| unit->id == UnitId::ZergLair
					|| unit->id == UnitId::ZergHive
					|| unit->id == UnitId::ZergSire
					|| unit->id == UnitId::ZergLarvalColony;

				u8 order_id;
				if (isZergMainBuilding)
				{
					order_id = OrderId::SpawningLarva;
				}
				else
				{
					order_id = OrderId::Nothing2;
				}
				if (unit->secondary_order_id != order_id)
				{
					unit->secondary_order_id = order_id;
					unit->secondary_order_pos.y = 0;
					unit->secondary_order_pos.x = 0;
					unit->current_build_unit = nullptr;
					unit->secondary_order_state = 0;
				}
			}
		}
	}

	// Returns the bounding rectangle (in tiles) of the maximum extent of creep around a source or the building footprint of a non-source creep building.
	// Return value is if the building is a creep source (spreads creep) or not
	bool getCreepRegion(Box32* rect, u16 unit_id, s32 x, s32 y, bool defRetVal)
	{
		// Default maximum bounding size
		const static s32 maxCreepAreaLeft = 320; // 10 tiles
		const static s32 maxCreepAreaRight = 320; // 10 tiles
		const static s32 maxCreepAreaUp = 200; // 6.25 tiles
		const static s32 maxCreepAreaDown = 200; // 6.25 tiles
		if (spreads_creep(unit_id, defRetVal))
		{
			rect->left = (x - maxCreepAreaLeft) / 32;
			rect->right = (x + maxCreepAreaRight) / 32;
			rect->top = (y - maxCreepAreaUp) / 32;
			rect->bottom = (y + maxCreepAreaDown) / 32;
			if (rect->left < 0)
			{
				rect->left = 0;
			}
			else if (rect->right >= *helper::mapTileBufWidth)
			{
				rect->right = *helper::mapTileBufWidth - 1;
			}
			if (rect->top < 0)
			{
				rect->top = 0;
			}
			else if (rect->bottom >= *helper::mapTileBufHeight)
			{
				rect->bottom = *helper::mapTileBufHeight - 1;
			}
			return true;
		}
		s32 w = units_dat::BuildingDimensions[unit_id].x / 32;
		s32 h = units_dat::BuildingDimensions[unit_id].y / 32;
		rect->left = x / 32 - w / 2;
		rect->top = y / 32 - h / 2;
		rect->right = rect->left + w - 1;
		rect->bottom = rect->top + h - 1;
		if (rect->left < 0)
		{
			rect->left = 0;
		}
		else if (rect->right >= *helper::mapTileBufWidth)
		{
			rect->right = *helper::mapTileBufWidth - 1;
		}
		if (rect->top < 0)
		{
			rect->top = 0;
		}
		else if (rect->bottom >= *helper::mapTileBufHeight)
		{
			rect->bottom = *helper::mapTileBufHeight - 1;
		}
		return false;
	}

	// Determines which unit IDs are a creep source (i.e. spreads creep, not just requires it)
	// Base buildings built by a drone return the value of "isComplete", which is an argument that is true if the unit status flag has "Completed" set -- the building only spawns creep once it has been built
	bool spreads_creep(u16 unit_id, bool isComplete)
	{
		switch (unit_id)
		{
			case UnitId::ZergHatchery:
			case UnitId::ZergCreepColony:
			{
				return isComplete;
			}
			case UnitId::ZergSire:
			case UnitId::ZergHive:
			case UnitId::ZergLair:
			case UnitId::ZergSporeColony:
			case UnitId::ZergSunkenColony:
			case UnitId::ZergLarvalColony:
			{
				return true;
			}
			default:
			{
				return false;
			}
		}
	}

	// x and y are relative positions where (0,0) is presumably the center of the building
	// !!! WARNING !!! Because of the super fun getUnitPointer call in this function, you probably never want to call this function directly
	bool inCreepArea(s32 x, s32 y)
	{
		// Default function:
		Unit* unit = helper::getUnitPointer();
		return (x * x * 100 + y * y * 256) <= 10240000;

		// TODO: Cleanup
		// Per-unit function example
//		Unit* unit = getUnitPointer();
		
		/*
		switch (deathTable[214].player[11]) {
		case 180:
			return (x*x * 100 + y*y * 256) <= 2560000;
		case UnitId::ZergCreepColony:
		case UnitId::ZergSporeColony:
		case UnitId::ZergSunkenColony:
			return (x*x * 100 + y*y * 256) <= 10240000;

		default: // hatchery/hive/lair
			return (x*x*100 + y*y*256) <= 10240000;
		}
		*/
	}

	// Callback to determine the creep tile that will replace the existing tile.
	bool runCreepRandomizer(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg)
	{
		// Stores the existing tile to replace the creep tile when the creep is removed
		if (*tileBackup == 0)
		{
			*tileBackup = *tile;
		}

		u16 creepTile;
		// use of std::rand() can cause desyncs
		if ((std::rand() % 100) >= 4)
		{
			creepTile = std::rand() % 6; // normal tiles
		}
		else
		{
			creepTile = (std::rand() % 7) + 6; // clutter tiles
		}
		*tile = creepTile + 0x10; // tile in creep group
		*retval = 0;
		return true;
	}

	u32 BTNSCOND_ZergBuildingRally(u32 unused, u32 player, Unit* unit) // 00429740
	{
		if (scbw::is_zerg_production(unit->id)
			|| scbw::is_zerg_production(unit->build_queue[unit->build_queue_slot]))
		{
			return helper::btnscond_rally(unused, player, unit);
		}
		else
		{
			return 0;
		}
	}

	/*
	char applyCreepAtLocationFromUnitType(s16 a1, s32 a2, s32 a3) // 004148F0
	{
		u32 v3; // ecx
		u32 v4; // ebx
		s32 v5; // edi
		u32 v6; // esi
		Point16 building_dimensions; // eax
		u32 v8; // eax
		s32 v9; // edi
		char result; // al
		u32 v11; // [esp+Ch] [ebp-10h]
		s32 v12; // [esp+10h] [ebp-Ch]
		u32 v13; // [esp+14h] [ebp-8h]
		s32 v14; // [esp+18h] [ebp-4h]

		building_dimensions = units_dat::BuildingDimensions[a1];
		v5 = (s16)v7 / 32;
		v4 = a2 / 32 - v5 / 2;
		v6 = a3 / 32 - (v7 >> 16) / 32 / 2;
		v3 = v6 + (v7 >> 16) / 32;
		v12 = a2 / 32 - v5 / 2;
		v11 = v3;
		if (v6 < v3)
		{
			v8 = v5 + v4;
			v13 = v5 + v4;
			do
			{
				v9 = v4;
				v14 = v4;
				if (v4 < v8)
				{
					do
					{
						if (!(*activeTileArray[(s16)v9 + map_tile_size->width * (s16)v6] & 0x20800000)
							&& ((s16)v6 + 1 == map_tile_size->height
								|| !(*activeTileArray[(s16)v9 + map_tile_size->width * (s16)(v6 + 1)] & 0x800000)))
						{
							modifyCreepState(
								v9,
								v6,
								(s32(__stdcall*)(s32, s32, s32, s32, s32))runCreepRandomizer,
								0);
							if (!(*(u32*)0x006D0C78) || !unkCreepBorders(v9, v6))
							{
								forEachAdjacentTile(unkCreepTile, 0, v9, v6);
								v9 = v14;
							}
						}
						v8 = v13;
						v14 = ++v9;
					} while (v9 < v13);
					v4 = v12;
					v3 = v11;
				}
				++v6;
			} while (v6 < v3);
		}
		do {
			result = unkSpreadCreep(a3, a2, a1);
		} while (result == 1);
		return result;
	}
	*/

	s32 UpdateCreepDisappearanceUnit(Unit* unit)
	{
		Box32 rect = {};
		bool does_spread = helper::get_creep_spread_area(unit->id, unit->position.x, unit->position.y, unit->status & UnitStatus::Completed, &rect);
		u32 pos_x = 32 * rect.left - unit->position.x + 16;
		u32 pos_y = 32 * rect.top - unit->position.y + 16;
		u32 tile_flags = unit->id == UnitId::ZergExcisant ? 0x10000000 : 0x18000000;
		if (rect.top > rect.bottom)
		{
			*dying_unit_creep_disappearance_count = 0;
			return 1;
		}
		else
		{
			u32 rect_top = rect.top;
			u32 rect_left = rect.left;
			while (true)
			{
				if (rect_left <= rect.right)
				{
					break;
				}
				++rect_top;
				pos_y += 32;
				if (rect_top > rect.bottom)
				{
					*dying_unit_creep_disappearance_count = 0;
					return 1;
				}
			}
			while (((*current_tiles)[rect_left + rect_top * *map_creep_width] & 0x7FF0) != 16
				|| does_spread && 100 * pos_x * pos_x + (pos_y * pos_y << 8) > 10240000
				|| tile_flags & (*activeTileArrayInt)[rect_left + rect_top * map_tile_size->width]
				|| helper::clip_creep_area_in_bounds(rect_left, rect_top, (helper::ClipCreepAreaInBounds_Callback*)0x0047D860)
				|| helper::mark_creep_to_disappear(rect_left, rect_top))
			{
				++rect_left;
				pos_x += 32;
				if (rect_left > rect.right)
				{
					rect_left = rect.left;
					++rect_top;
					pos_y += 32;
					if (rect_top > rect.bottom)
					{
						*dying_unit_creep_disappearance_count = 0;
						return 1;
					}
				}
			}
			return 0;
		}
		return 0;
	}
}