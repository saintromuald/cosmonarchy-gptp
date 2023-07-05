#include "larva_creep_spawn.h"
#include "building_making\building_modifier.h"
#include <SCBW/api.h>
#include "SCBW\UnitFinder.h"
#include "globals.h"

namespace hooks
{
	helper::SetCreepRelationalData_callback run_creep_randomizer_ptr = &run_creep_randomizer;
	void create_building_larva(Unit* building) {
		u32 unit_id = unit_id_get_larva_type(building->id);
		if (unit_id == UnitId::None) {
			return;
		}

		Vector32 coords = {};
		if (helper::get_larva_spawn_pos(building, &coords)) {
			Unit* unit = helper::create_unit(unit_id, coords.x, coords.y, building->player);
			if (unit) {
				helper::update_unit_stats_and_finish_building(unit);
				helper::update_unit_strength(unit);
				unit->connected_unit = building;
				unit->sprite->player = building->sprite->player;

				// set main_order_state depending on the collision state with the connected building			
				helper::set_larva_order_state(unit);
				if (building->unit_ai) {
					helper::ai_training_overlord(unit, building);
				}
			}
		} else {
			building->building.larva_timer = 3;
		}
	}

	// filter for larva in internal game searches, use the limit of 3 larvas
	bool iterate_units_at_location_target_proc_larva_count(Unit* unit, Unit* main_building) {
		// While return_value == false, the calling function keep calling, so it return true only when max amount of larvas was counted,
		// since the counting itself is made with LARVA_COUNTER here and not in the calling function.
		if (unit->id == UnitId::ZergLarva || unit->id == UnitId::ZergOcsalLarva) {
			if (unit->sprite->flags & SpriteFlags::Hidden) {
				return false;
			}
			if (unit->connected_unit != main_building) {
				return false;
			}
			(*g_larva_counter)++;
			return *g_larva_counter > 3;
		}
		return false;
	}

	void secondary_order_spawning_larva(Unit* unit) {
		if (unit->status & UnitStatus::IsHallucination) {
			return;
		}
		if (unit->order_queue_timer == 0 && unit->player != 11) {
			*g_larva_counter = 0;
			Box16 coords = {};
			coords.left = unit->sprite->position.x - 256;
			coords.top = unit->sprite->position.y - 256;
			coords.right = unit->sprite->position.x + 256;
			coords.bottom = unit->sprite->position.y + 256;

			// Count existing larvas in LARVA_COUNTER
			helper::iterate_units_at_location_target_proc_larva_count(unit, &coords);
			if (*g_larva_counter < 3 && !(unit->status & UnitStatus::InAir)) {
				if (unit->building.larva_timer != 0) {
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

				if (unit->building.larva_timer == 0) {
					unit->building.larva_timer = LarvaSpawnRate;

					u32 unit_id = unit->id;
					if (unit->main_order_id == OrderId::Morph2) {
						unit_id = unit->previous_unit_type;
					}
					create_building_larva(unit);
				}
			}
		}
	}

	void secondary_order_spread_creep_spawning_larva(Unit* unit) {
		if (unit->id == UnitId::ZergHachirosk
		||  unit->id == UnitId::ZergCaphrolosk
		||  unit->id == UnitId::ZergGathtelosk
		||  unit->id == UnitId::ZergLarvoskCesiant
		||  unit->id == UnitId::ZergOcsalCesiant)
		{
			secondary_order_spawning_larva(unit);
		}
		if (unit->building.creep_timer != 0) {
			if (unit->building.creep_timer >= 2) {
				unit->building.creep_timer -= 2;

				BuildingModifier modifier = get_building_modifier(unit, 0, false);
				if (modifier.decrease) {
					if (unit->building.creep_timer > modifier.decrease) {
						unit->building.creep_timer -= modifier.decrease;
					} else {
						unit->building.creep_timer = 0;
					}
				}
			} else {
				unit->building.creep_timer = 0;
			}

		} else {
			u8 result_00414680;
			unit->building.creep_timer = 15;
			// look for a place to spawn creep on
			// return value == 0 if all potential tiles are already creep, 2 == potential tiles are occupied by another building, 1 == potential tiles are otherwise still available
			// Replace UnitId::ZergGathtelosk with unit->id if giving specific buildings different creep bounding rectangles in get_creep_region
			result_00414680 = helper::spread_creep(unit->id, unit->sprite->position.x, unit->sprite->position.y);
			if (unit->id == 180 && result_00414680 != 0) {
				u32 current_tile_x = unit->sprite->position.x / 32;
				u32 current_tile_y = unit->sprite->position.y / 32;
				s32 width_x = 6;
				s32 width_y = 4;
				u8 mustspread = 0;
				s32 x1 = current_tile_x - width_x;
				s32 y1 = current_tile_y - width_x;
				s32 x2 = current_tile_x + width_x;
				s32 y2 = current_tile_y + width_y;

				if (x1 < 0) {
					x1 = 0;
				}
				if (y1 < 0) {
					y1 = 0;
				}
				if (x2 >= g_map_tile_size->width) {
					x2 = g_map_tile_size->width - 1;
				}
				if (y2 >= g_map_tile_size->height) {
					y2 = g_map_tile_size->height - 1;
				}

				for (s32 i = x1; i <= x2; i++) {
					for (s32 j = y1; j <= y2; j++) {
						s32 x = (i * 32) + 16;
						s32 y = (j * 32) + 16;
						s32 X = (x - (current_tile_x * 32));
						s32 Y = (y - (current_tile_y * 32));
						bool in_range = ((256 * X * X) + (100 * Y * Y)) < 1300000;
						// adjust value 1300000 to get slightly different radius

						if (in_range) {
							ActiveTile* a = scbw::get_active_tile_at_coords(i * 32, j * 32);
							if (a->has_creep || a->temporary_creep || a->creep_receding) {
								// why is there nothing here LMAO
							} else {
								// can spread
								if (!a->always_unbuildable && a->is_walkable) {

									mustspread = 1;
								}
							}
						}
					}
				}
				if (!mustspread) {
					result_00414680 = 0;
				}
			}
			if (result_00414680 == 0) {
				bool can_spawn_larva = unit->id == UnitId::ZergHachirosk
									|| unit->id == UnitId::ZergCaphrolosk
									|| unit->id == UnitId::ZergGathtelosk
									|| unit->id == UnitId::ZergLarvoskCesiant
									|| unit->id == UnitId::ZergOcsalCesiant;

				u8 order_id = can_spawn_larva ? OrderId::SpawningLarva : OrderId::Nothing2;
				if (unit->secondary_order_id != order_id) {
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
	bool get_creep_region(Box32* rect, u16 unit_id, s32 x, s32 y, bool defRetVal) {
		// Default maximum bounding count
		const static s32 maxCreepAreaLeft = 320; // 10 tiles
		const static s32 maxCreepAreaRight = 320; // 10 tiles
		const static s32 maxCreepAreaUp = 200; // 6.25 tiles
		const static s32 maxCreepAreaDown = 200; // 6.25 tiles
		if (spreads_creep(unit_id, defRetVal)) {
			rect->left = (x - maxCreepAreaLeft) / 32;
			rect->right = (x + maxCreepAreaRight) / 32;
			rect->top = (y - maxCreepAreaUp) / 32;
			rect->bottom = (y + maxCreepAreaDown) / 32;
			if (rect->left < 0) {
				rect->left = 0;
			} else if (rect->right >= *helper::mapTileBufWidth) {
				rect->right = *helper::mapTileBufWidth - 1;
			}
			if (rect->top < 0) {
				rect->top = 0;
			} else if (rect->bottom >= *helper::mapTileBufHeight) {
				rect->bottom = *helper::mapTileBufHeight - 1;
			}
			return true;
		}
		s32 w = units_dat::building_dimensions[unit_id].x / 32;
		s32 h = units_dat::building_dimensions[unit_id].y / 32;
		rect->left = x / 32 - w / 2;
		rect->top = y / 32 - h / 2;
		rect->right = rect->left + w - 1;
		rect->bottom = rect->top + h - 1;
		if (rect->left < 0) {
			rect->left = 0;
		} else if (rect->right >= *helper::mapTileBufWidth) {
			rect->right = *helper::mapTileBufWidth - 1;
		}
		if (rect->top < 0) {
			rect->top = 0;
		} else if (rect->bottom >= *helper::mapTileBufHeight) {
			rect->bottom = *helper::mapTileBufHeight - 1;
		}
		return false;
	}

	// Determines which unit IDs are a creep source (i.e. spreads creep, not just requires it)
	// Base buildings built by a drone return the value of "is_complete", which is an argument that is true if the unit status flag has "Completed" set -- the building only spawns creep once it has been built
	bool spreads_creep(u16 unit_id, bool is_complete) {
		switch (unit_id) {
			case UnitId::ZergHachirosk:
			case UnitId::ZergCagrantCesiant: {
				return is_complete;
			}
			case UnitId::ZergCaphrolosk:
			case UnitId::ZergGathtelosk:
			case UnitId::ZergSpraithCesiant:
			case UnitId::ZergSurkithCesiant:
			case UnitId::ZergLarvoskCesiant:
			case UnitId::ZergSkortrithCesiant:
			case UnitId::ZergSpirtithCesiant:
			case UnitId::ZergNydusCesiant:
			case UnitId::ZergKazitrithCesiant:
			case UnitId::ZergOcsalCesiant:
			{
				return true;
			}
		}
		return false;
	}

	// x and y are relative positions where (0,0) is presumably the center of the building
	bool in_creep_area(s32 x, s32 y) {
		return (x * x * 100 + y * y * 256) <= 10240000;
	}

	// Callback to determine the creep tile that will replace the existing tile.
	bool run_creep_randomizer(u16* tile_backup, u8* creep_overlay_value, u32* return_value, u16* tile, u32 arg) {
		// Stores the existing tile to replace the creep tile when the creep is removed
		if (*tile_backup == 0) {
			*tile_backup = *tile;
		}

		u16 creep_tile = 0;
		// use of std::rand() can cause desyncs
		if ((std::rand() % 100) >= 4) {
			creep_tile = std::rand() % 6; // normal tiles
		} else {
			creep_tile = (std::rand() % 7) + 6; // clutter tiles
		}
		*tile = creep_tile + 0x10; // tile in creep group
		*return_value = 0;
		return true;
	}

	u32 btsncond_zerg_building_rally(u32 unused, u32 player, Unit* unit) { // 00429740
		if (unit_id_is_zerg_production(unit->id)
			|| unit_id_is_zerg_production(unit->build_queue[unit->build_queue_slot])) {
			return helper::btnscond_rally(unused, player, unit);
		} else {
			return 0;
		}
	}

	/*
	char apply_creep_at_location_from_unit_id(s16 a1, s32 a2, s32 a3) // 004148F0
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

		building_dimensions = units_dat::building_dimensions[a1];
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
						if (!(*activeTileArray[(s16)v9 + g_map_tile_size->width * (s16)v6] & 0x20800000)
							&& ((s16)v6 + 1 == g_map_tile_size->height
								|| !(*activeTileArray[(s16)v9 + g_map_tile_size->width * (s16)(v6 + 1)] & 0x800000)))
						{
							modify_creep_state(
								v9,
								v6,
								(s32(__stdcall*)(s32, s32, s32, s32, s32))run_creep_randomizer,
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

	s32 update_creep_disappearance_unit(Unit* unit) {
		Box32 rect = {};
		bool does_spread = helper::get_creep_spread_area(unit->id, unit->position.x, unit->position.y, unit->status & UnitStatus::Completed, &rect);
		u32 pos_x = 32 * rect.left - unit->position.x + 16;
		u32 pos_y = 32 * rect.top - unit->position.y + 16;
		u32 tile_flags = unit->id == UnitId::ZergExcisant ? 0x10000000 : 0x18000000;
		if (rect.top > rect.bottom) {
			*g_dying_unit_creep_disappearance_count = 0;
			return 1;
		} else {
			u32 rect_top = rect.top;
			u32 rect_left = rect.left;
			while (true) {
				if (rect_left <= rect.right) {
					break;
				}
				++rect_top;
				pos_y += 32;
				if (rect_top > rect.bottom) {
					*g_dying_unit_creep_disappearance_count = 0;
					return 1;
				}
			}
			while (((*g_current_tiles)[rect_left + rect_top * *g_map_creep_width] & 0x7FF0) != 16
				|| does_spread && 100 * pos_x * pos_x + (pos_y * pos_y << 8) > 10240000
				|| tile_flags & (*g_active_tile_array_int)[rect_left + rect_top * g_map_tile_size->width]
				|| helper::clip_creep_area_in_bounds(rect_left, rect_top, (helper::ClipCreepAreaInBounds_Callback*)0x0047D860)
				|| helper::mark_creep_to_disappear(rect_left, rect_top)) {
				++rect_left;
				pos_x += 32;
				if (rect_left > rect.right) {
					rect_left = rect.left;
					++rect_top;
					pos_y += 32;
					if (rect_top > rect.bottom) {
						*g_dying_unit_creep_disappearance_count = 0;
						return 1;
					}
				}
			}
			return 0;
		}
		return 0;
	}
}