#include "create_init_units.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "ai_layouts.h"
#include "threshold.h"

u32 get_racial_townhall(u32 raceId)
{
	if (raceId == RaceId::Zerg)
	{
		return UnitId::ZergHachirosk;
	}
	else if (raceId == RaceId::Terran)
	{
		return UnitId::TerranCommandCenter;
	}
	else if (raceId == RaceId::Protoss)
	{
		return UnitId::ProtossNexus;
	}
	else
	{
		return UnitId::None;
	}
}

u32 get_racial_worker(u32 raceId)
{
	if (raceId == RaceId::Zerg)
	{
		return UnitId::ZergDroleth;
	}
	else if (raceId == RaceId::Terran)
	{
		return UnitId::TerranMason;
	}
	else if (raceId == RaceId::Protoss)
	{
		return UnitId::ProtossScribe;
	}
	else
	{
		return UnitId::None;
	}
}

namespace hooks
{
	void create_initial_melee_buildings(u8 raceId, u32 player)
	{
		u32 buildingId = get_racial_townhall(raceId);
		s32 buildingSizeX, buildingSizeY;
		Box16 searchBox;
		Unit* created_unit;

		buildingSizeX = units_dat::building_dimensions[buildingId].x;
		buildingSizeY = units_dat::building_dimensions[buildingId].y;
		if (buildingSizeX < 0)
		{
			buildingSizeX++;
		}
		if (buildingSizeY < 0)
		{
			buildingSizeY++;
		}

		searchBox.left = (g_start_positions[player].x - (s16)(buildingSizeX / 2)) & 0x0000FFE0;
		searchBox.top = (g_start_positions[player].y - (s16)(buildingSizeY / 2)) & 0x0000FFE0;
		searchBox.bottom = buildingSizeY / 2 + ((g_start_positions[player].y - (s16)(buildingSizeY / 2)) & 0x0000FFE0) + buildingSizeY / 2 - 1;
		searchBox.right = buildingSizeX / 2 + ((g_start_positions[player].x - (s16)(buildingSizeX / 2)) & 0x0000FFE0) + buildingSizeX / 2 - 1;

		// yes, the return value is unused for some reason
		helper::find_best_unit(&searchBox, 0x0049D640);
		created_unit = helper::create_unit(buildingId,
				((g_start_positions[player].x - (buildingSizeX / 2)) & 0x0000FFE0) + buildingSizeX / 2,
				((g_start_positions[player].y - (buildingSizeY / 2)) & 0x0000FFE0) + buildingSizeY / 2,
				player);

		if (created_unit != nullptr)
		{
			helper::update_unit_stats_and_finish_building(created_unit);
			if (helper::show_finished_unit(created_unit) != 0)
			{
				helper::update_unit_strength(created_unit);
			}
			if (helper::spreads_creep(created_unit->id, 1)
				|| units_dat::base_property[created_unit->id] & UnitProperty::CreepBuilding)
			{
				helper::apply_creep_at_location_from_unit_id(created_unit->sprite->position.x, created_unit->sprite->position.y, created_unit->id);
			}
		}
	}

	f64 Sign(f64 x) {
		return x < 0.0 ? -1.0 : x > 0.0 ? 1.0 : 0.0;
	}

	Point16 find_worker_spawn_positions(s32 townhallId, const u16& startX, const u16& startY, bool& horizontal) {
		s32 sum_x = 0;
		s32 sum_y = 0;
		u32 count = 0;

		scbw::UnitFinder unit_finder;
		unit_finder.search(
			startX - 10 * 32,
			startY - 10 * 32,
			startX + 10 * 32,
			startY + 10 * 32
		);
		u8 cliff_level = scbw::get_active_tile_at_coords(startX, startY)->ground_height;
		unit_finder.for_each([&](Unit* unit) {
			if ((unit->id == UnitId::ResourceMineralField
			|| unit->id == UnitId::ResourceMineralFieldType2
			|| unit->id == UnitId::ResourceMineralFieldType3)
			&& scbw::get_active_tile_at_coords(unit->position.x, unit->position.y)->ground_height == cliff_level) {
				Point16 pos = unit->position;
				sum_x += pos.x - startX;
				sum_y += pos.y - startY;
				count += 1;
			}
		});

		if (count == 0) { // Default to bottom horizontal
			count = 1;
			sum_x = 0;
			sum_y = 1;
		}

		f64 averageX = (f64)sum_x / count;
		f64 averageY = (f64)sum_y / count;
		f64 distance = pow(pow(averageX, 2) + pow(averageY, 2), 0.5);

		f64 offsetX = averageX / distance;
		f64 offsetY = averageY / distance;

		const f64 distanceMultiplier = 1.4;
		f64 width = distanceMultiplier;
		f64 height = distanceMultiplier;

		if (offsetX < 0) {
			width *= units_dat::unit_bounds[townhallId].left;
		} else {
			width *= units_dat::unit_bounds[townhallId].right;
		}

		if (offsetY < 0) {
			height *= units_dat::unit_bounds[townhallId].top;
		} else {
			height *= units_dat::unit_bounds[townhallId].bottom;
		}

		offsetX = offsetX * width;
		offsetY = offsetY * height;

		const f64 diagonalThreshold = 0.5; // 1 - full diagonal, 0 - straight line
		bool forceHorizontal = false;
		f64 max = std::max(abs(offsetX), abs(offsetY));
		f64 min = std::min(abs(offsetX), abs(offsetY));
		if (min != 0 && diagonalThreshold != 0) {
			forceHorizontal = max / min < 1 / diagonalThreshold;
		}

		if (forceHorizontal || abs(offsetX) < abs(offsetY)) {
			offsetX = -units_dat::unit_bounds[townhallId].left * distanceMultiplier;
			offsetY = Sign(offsetY) * height;
			horizontal = true;
		} else {
			offsetX = Sign(offsetX) * width;
			offsetY = units_dat::unit_bounds[townhallId].bottom * distanceMultiplier * 1.5;
			horizontal = false;
		}

		return Point16{(u16)(startX + offsetX), (u16)(startY + offsetY)};
	}

	void create_initial_melee_units()
	{
		u32 player;
		u8* raceIdOffset;

		player = 8;
		raceIdOffset = (u8*)0x0057F009;

		while (raceIdOffset != (u8*)0x0057EEE9)
		{
			raceIdOffset -= 0x24;
			player--;

			if (*(raceIdOffset - 1) == 2 || *(raceIdOffset - 1) == 1)
			{
				// 0 is use map settings, 1 is Workers, 2 is Workers + Main Building
				u8 starting_units = (u8)(*(u16*)(0x00596871));

				static u8* victory_conditions = (u8*)0x0059686D;
				static u8* tournament_mode = (u8*)0x00596877;
				static u8* user_select_slots = (u8*)0x0059BDA8;

				if (*victory_conditions == 0					// check triggers for conditions
					&& starting_units == 0 						// use map units
					&& *tournament_mode == 0					// tournament mode off
					&& player < 8
					&& (user_select_slots[player]) != 0		// race selected instead of using map settings?
					)
				{
					starting_units = 2;
				}

				if (starting_units == 2)
				{
					create_initial_melee_buildings(*raceIdOffset, player);
					if (*raceIdOffset == RaceId::Zerg)
					{
						create_initial_ovileth(player);
					}
				}

				if (starting_units == 1 || starting_units == 2)
				{
					u32 unit_id = get_racial_worker(*raceIdOffset);
					u8 townhallId = get_racial_townhall(*raceIdOffset);
					Unit* created_unit;

					if (unit_id != UnitId::None)
					{
						Point16 startPos = g_start_positions[player];
						bool horizontal = true;
						Point16 pos = find_worker_spawn_positions(townhallId, startPos.x, startPos.y, horizontal);

						for (s32 i = 0; i < 6; i++)
						{
							const s32 correctionOffset = 1;
							const s32 unitWidth = units_dat::unit_bounds[unit_id].left + units_dat::unit_bounds[unit_id].right + correctionOffset;
							const s32 unitHeight = units_dat::unit_bounds[unit_id].bottom + units_dat::unit_bounds[unit_id].top + correctionOffset;

							if (horizontal) pos.x += unitWidth;
							else pos.y -= unitHeight;

							created_unit = helper::create_unit(unit_id, pos.x, pos.y, player);
							scbw::teleport(created_unit, pos);

							if (created_unit != nullptr)
							{
								helper::update_unit_stats_and_finish_building(created_unit);
								if (helper::show_finished_unit(created_unit) != 0)
								{
									helper::update_unit_strength(created_unit);
								}
							}
						}
					}
				}
			}
		}
	}
		
	void create_initial_ovileth(u8 player)
	{
		return; // Ovileth no longer spawns

		static u16* const maxBoxRightValue = (u16*)0x00628450;
		static u16* const maxBoxBottomValue = (u16*)0x006284B4;
		s32 overlordStartX, overlordStartY;
		Unit* created_unit;
		if (g_start_positions[player].x < *maxBoxRightValue / 2)
		{
			overlordStartX = g_start_positions[player].x + units_dat::unit_bounds[UnitId::ZergHachirosk].right + units_dat::unit_bounds[UnitId::ZergHachirosk].left + 1;
		}
		else
		{
			overlordStartX = g_start_positions[player].x - (units_dat::unit_bounds[UnitId::ZergHachirosk].right + units_dat::unit_bounds[UnitId::ZergHachirosk].left + 1);
		}

		if (g_start_positions[player].y < *maxBoxBottomValue / 2)
		{
			overlordStartY = g_start_positions[player].y + units_dat::unit_bounds[UnitId::ZergHachirosk].bottom + units_dat::unit_bounds[UnitId::ZergHachirosk].top + 1;
		}
		else
		{
			overlordStartY = g_start_positions[player].y - (units_dat::unit_bounds[UnitId::ZergHachirosk].bottom + units_dat::unit_bounds[UnitId::ZergHachirosk].top + 1);
		}

		created_unit = helper::create_unit(UnitId::ZergOvileth, overlordStartX, overlordStartY, player);
		if (created_unit != nullptr)
		{
			helper::update_unit_stats_and_finish_building(created_unit);
			if (helper::show_finished_unit(created_unit) != 0)
			{
				helper::update_unit_strength(created_unit);
			}
		}
	}

	Unit* create_unit_xy(u16 player_id, u16 id, u32 x, u32 y, bool prevent_creep) {
		if (units_dat::staredit_avail_flags[id] & 0x200 && !*g_is_brood_war) {
			return 0;
		}
		Unit* unit = helper::create_unit(id, x, y, player_id);
		if (unit) {
			if (!prevent_creep && (helper::spreads_creep(unit->id, 1) || units_dat::base_property[id] & UnitProperty::CreepBuilding)) {
				helper::apply_creep_at_location_from_unit_id(unit->sprite->position.x, unit->sprite->position.y, id);
			}
			helper::update_unit_stats_and_finish_building(unit);
			if (helper::show_finished_unit(unit)) {
				helper::update_unit_strength_and_apply_default_orders(unit);
				helper::add_guard_ai_to_unit(unit);
				return unit;
			} else {
				const char* text  = "";
				if ( *g_network_table > 0x11u ) {
					text = (char *)g_network_table + g_network_table[18];
				}
				const char* text2 = helper::get_unit_name_stat_string(unit->id);
				char buffer[256] = {};
				sprintf_s(buffer, 256, text, text2, unit->sprite->position.x, unit->sprite->position.y);
				helper::unit_placement_error(buffer, 3);
				unit->user_action_flags |= 4u;
				helper::orders_self_destructing(unit);
			}
		} else {
			helper::display_last_network_error_for_player(player_id);
		}
		return 0;
	}

	Unit* create_preplaced_unit(CHKUnit* chkunit) {
		u32 flags = chkunit->valid_state_flags & chkunit->state_flags;
		if (init_threshold_spawn(chkunit, flags)) {
			return 0;
		}
		if (chk_cache_size < ChkCacheLimit) {
			chk_cache[chk_cache_size++] = *chkunit;
		}
		// Requires changes to player select so they actually spawn units
		//if (flags & ChkUnitFlags_Lifted) {
		//	if (!scbw::unit_id_race_check(chkunit->unit_id, g_player_table[chkunit->player].race)) {
		//		return 0;
		//	}
		//}
		if (!(flags & ChkUnitFlags_Cloaked) && chkunit->unit_id != UnitId::TownCenterMarker) {
			chkunit->state_flags &= ~ChkUnitFlags_Cloaked;
			chkunit->state_flags &= ~ChkUnitFlags_Burrowed;
			chkunit->state_flags &= ~ChkUnitFlags_Lifted;
			chkunit->state_flags &= ~ChkUnitFlags_Hallucinated;
			chkunit->state_flags &= ~ChkUnitFlags_Invincible;
			chkunit->hp = 100;
			chkunit->shields = 100;
			chkunit->energy = 100;
			Unit* unit = create_unit_xy(chkunit->player, chkunit->unit_id, chkunit->x, chkunit->y, false);
			if (unit) {
				helper::edit_unit_flags(unit, &chkunit->valid_state_flags);
				unit->remove_timer = 0;
			}
			return unit;
		}
		return 0;
	}
}