#include "resarea.h"
#include "BW/bwhelper.h"
#include "ai_layouts.h"
#include "storm/storm.h"

v2u16* ext_start_locs;

namespace hooks {
	u32 __fastcall get_unit_hp_or_resarea(Unit* unit, ChkUnitSettings* settings) {
		u32 max_hp = units_dat::max_hitpoints[unit->id];
		u32 hp_percent = settings->hitpoints;
		if (!settings->state_flags & ChkUnitFlags_Burrowed || !(units_dat::base_property[unit->id] & UnitProperty::ResourceContainer)) {
			if (hp_percent < 100) {
				return max_hp - (max_hp / 100 * (100 - hp_percent));
			}
		} else {
			unit->building.temp_custom_resarea_id = hp_percent | 256;
		}
		return max_hp;
	}

	b32 __fastcall encode_start_location(ChkUnit* chk_unit, v2u16* position) {
		if (chk_unit->unit_id != UnitId::Special_StartLocation) {
			return false;
		}
		const u32 player = chk_unit->player;
		v2u16 unit_pos = {chk_unit->x, chk_unit->y};
		if (player < PLAYABLE_PLAYER_COUNT) {
			position[player] = unit_pos;
			if (g_is_in_replay ? (s16)g_move_to_tile_x == -1 : player == g_local_nation_id) {
				s16 pos = ((s16)unit_pos.x >> 5) - 10;
				g_move_to_tile_x = pos >= 0 ? pos : 0;
				pos = ((s16)unit_pos.y >> 5) - 6;
				g_move_to_tile_y = pos >= 0 ? pos : 0;
			}
			return true;
		}
		if ((unit_pos.x || unit_pos.y)
		&& (ext_start_locs || (ext_start_locs = (v2u16*)SMemAlloc((256 - PLAYABLE_PLAYER_COUNT) * sizeof(v2u16), __FILE__, __LINE__, 0)))) {
			ext_start_locs[player - PLAYABLE_PLAYER_COUNT] = unit_pos;
		}
		return true;
	}

	void create_resareas() {
		memset(g_resource_areas, 0, sizeof(ResourceAreaArray));
		u32 resarea_count = 0;
		for (u32 player_id = 0; player_id < PLAYABLE_PLAYER_COUNT; ++player_id) { // Create startloc resareas.
			if (g_start_positions[player_id].x || g_start_positions[player_id].y) {
				ResourceArea* resarea = &g_resource_areas->areas[++resarea_count];
				resarea->position = g_start_positions[player_id];
				resarea->is_start_location = true;
				u8 player_type = g_player_table[player_id].type - 1;
				resarea->flags |= player_type <= 1; // PlayerType::Computer || PlayerType::Human
			}
		}

		g_resource_areas->count = resarea_count;
		u8 res_type_list[UnitLimit];
		u8* res_type_list_curr = res_type_list;
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) { // Assign to startloc resareas within range.
			u32 type = 0;
			if (unit->sprite && unit->main_order_id && (units_dat::base_property[unit->id] & UnitProperty::ResourceContainer)) {
				type = !(unit->id >= UnitId::ResourceMineralField && unit->id <= UnitId::ResourceMineralFieldType3);
				if (u32 id = helper::get_resarea_id(unit)) {
					unit->building.resource.resarea = id;
					ResourceArea* resarea = &g_resource_areas->areas[id];
					++(&resarea->mineral_field_count)[type];
					(&resarea->total_mineral_count)[type] += unit->building.resource.resource_amount;
					type |= 2;
				}
				++type;
			}
			*res_type_list_curr = type;
			++res_type_list_curr;
		}

		helper::assign_to_nearest_resarea(2, res_type_list);
		helper::assign_to_nearest_resarea(1, res_type_list);
		if ((resarea_count != g_resource_areas->count)) {
			return;
		}
		if (ext_start_locs) { // Adjust resarea positions using P9+ start locs.
			for (s32 id = resarea_count - PLAYABLE_PLAYER_COUNT; id >= 0; --id) {
				v2u16 position = ext_start_locs[id];

				if (position.x || position.y) {
					g_resource_areas->areas[id + PLAYABLE_PLAYER_COUNT + 1].position = position;
				}
			}

			SMemFree(ext_start_locs, __FILE__, __LINE__, 0);
			ext_start_locs = nullptr;
		}

		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) { // Assign map-defined resarea IDs.
			if (unit->sprite && unit->main_order_id && (units_dat::base_property[unit->id] & UnitProperty::ResourceContainer)) {
				u32 id_new = unit->building.temp_custom_resarea_id;

				if (!id_new) {
					continue;
				}
				unit->building.temp_custom_resarea_id = 0;
				id_new = (u8)id_new;
				u8 id_old = unit->building.resource.resarea;

				if (id_new != id_old && id_new <= resarea_count) {
					unit->building.resource.resarea = id_new;
					u32 type = !(unit->id >= UnitId::ResourceMineralField && unit->id <= UnitId::ResourceMineralFieldType3);
					u32 resource_amount = unit->building.resource.resource_amount;

					if (id_new) {
						ResourceArea* resarea = &g_resource_areas->areas[id_new];
						++(&resarea->mineral_field_count)[type];
						(&resarea->total_mineral_count)[type] += resource_amount;
					}

					if (id_old) {
						ResourceArea* resarea = &g_resource_areas->areas[id_old];
						--(&resarea->mineral_field_count)[type];
						(&resarea->total_mineral_count)[type] -= resource_amount;
					}
				}
			}
		}

		for (s32 id = resarea_count; id >= 0; --id) { // Cull tail-end resareas that are empty.
			ResourceArea* resarea = &g_resource_areas->areas[id];
			if (resarea->mineral_field_count || resarea->geyser_count || resarea->is_start_location) {
				break;
			}
			resarea->position = {};
			g_resource_areas->count = --resarea_count;
		}
	}
}