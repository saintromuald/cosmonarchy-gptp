#include "larva_creep_spawn.h"
#include "building_making\building_modifier.h"
#include <BW/api.h>
#include "BW\UnitFinder.h"
#include "globals.h"

namespace hooks {
    helper::SetCreepRelationalData_callback run_creep_randomizer_ptr = &run_creep_randomizer;
    void create_building_larva(Unit* building) {
        u32 unit_id = unit_id_get_larva_type(building->id);
        if (unit_id == UnitId::None) {
            return;
        }

        Vec32 coords = {};
        if (helper::get_larva_spawn_pos(building, &coords)) {
            Unit* unit = helper::create_unit(unit_id, coords.x, coords.y, building->player);
            if (unit) {
                spread_debuffs(building, unit);
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
        if (!unit_id_spawns_larva_by_order(unit->id) && !unit_spawns_custom_larva(unit)) {
            return;
        }
        if (!unit_building_can_act(unit)) {
            return;
        }
        UnitExt& unit_ext = unit->ext();
        if (unit_ext.timers[TimerSpawnLarva] == 0 && unit->player != 11) {
            unit_ext.timers[TimerSpawnLarva] = 8;
            unit->count_connected_larva();
            if (*g_larva_counter < 3 && !unit->is_in_air()) {
                const auto modifier = cm::get_building_modifier(unit);
                if (unit->building.larva_timer > modifier.integer_step) {
                    unit->building.larva_timer -= modifier.integer_step;
                } else {
                    unit->building.larva_timer = 0;
                }

                if (!unit->building.larva_timer) {
                    unit->building.larva_timer = Zerg::LarvaSpawnRate;
                    u32 unit_id = unit->id;
                    if (unit->main_order_id == OrderId::MorphStructure) {
                        unit_id = unit->previous_unit_type;
                    }
                    create_building_larva(unit);
                }
            }
        }
    }

    void secondary_order_spread_creep_spawning_larva(Unit* unit) { // 004EA880
        // No longer checks unit ids here because we check in the function itself,
        // in order to prevent Cagrants regressed from Larvosks from spawning Larva. -Pr0nogo
        secondary_order_spawning_larva(unit);
        if (unit->building.creep_timer != 0) {
            if (unit->building.creep_timer >= 2) {
                unit->building.creep_timer -= 2;

                const auto modifier = cm::get_building_modifier(unit, { .already_ticked = true });
                if (unit->building.creep_timer > modifier.integer_step) {
                    unit->building.creep_timer -= modifier.integer_step;
                } else {
                    unit->building.creep_timer = 0;
                }
            } else {
                unit->building.creep_timer = 0;
            }
        } else {
            unit->building.creep_timer = 15;
            // look for a place to spawn creep on
            // return value == 0 if all potential tiles are already creep, 2 == potential tiles are occupied by another building, 1 == potential tiles are otherwise still available
            // Replace UnitId::ZergGathtelosk with unit->id if giving specific buildings different creep bounding rectangles in get_creep_region
            u8 did_spread_cagra = helper::spread_creep(unit->id, unit->sprite->position.x, unit->sprite->position.y);
            if (unit->id == UnitId::UnusedCaveIn && did_spread_cagra != 0) { // This was for Bactalisk spawner, old iquare cagra spread code. -Pr0nogo
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
                            ActiveTile* a = bw::get_active_tile_at_coords(i * 32, j * 32);
                            if (a->has_cagra || a->temporary_cagra || a->cagra_receding) {
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
                    did_spread_cagra = 0;
                }
            }
            if (did_spread_cagra == 0) {
                u8 order_id = unit_id_spawns_larva_by_order(unit->id) ? OrderId::SpawningLarva : OrderId::Nothing2;
                if (unit->secondary_order_id != order_id) {
                    unit->secondary_order_id = order_id;
                    unit->secondary_order_pos.y = 0;
                    unit->secondary_order_pos.x = 0;
                    unit->current_build_unit = 0;
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
            case UnitId::ZergOcsalCesiant: {
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
    bool run_creep_randomizer(u16* tile_backup, u8* creep_overlay_value, u32* return_value, u16* tile, u32 arg) { // 004140A0
        // Stores the existing tile to replace the creep tile when the creep is removed
        if (*tile_backup == 0) {
            *tile_backup = *tile;
        }

        u16 creep_tile = 0;
        if ((g_last_random_number % 100) >= 4) {
            creep_tile = g_last_random_number % 6; // normal tiles
        } else {
            creep_tile = (g_last_random_number % 7) + 6;
        }
        *tile = creep_tile + 0x10; // tile in creep group
        *return_value = 0;

        // Change random number; it's used weirdly here, original func can protentially call bw::random() twice. -Pr0nogo
        // TODO: Figure out whether this is actually necessary.
        bw::random();

        return true;
    }

    u32 btsncond_zerg_building_rally(u32 unused, u32 player, Unit* unit) { // 00429740
        if (unit_id_is_zerg_production(unit->id) || unit_id_is_zerg_production(unit->build_queue[unit->build_queue_slot])) {
            return helper::btnscond_rally(unused, player, unit);
        } else {
            return 0;
        }
    }

    s32 update_creep_disappearance_unit(Unit* unit) { // 0047DD60
        Box32 rect = {};
        bool does_spread = helper::get_creep_spread_area(unit->id, unit->position.x, unit->position.y, unit->status & UnitStatus::Completed, &rect);
        u32 pos_x = 32 * rect.left - unit->position.x + 16;
        u32 pos_y = 32 * rect.top - unit->position.y + 16;
        bool is_zerg_gas_cap = unit_id_is_capped_gas_node(unit->id) && unit->get_race() == RaceId::Zerg;
        u32 tile_flags = is_zerg_gas_cap ? 0x10000000 : 0x18000000;
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
            ||		  does_spread && 100 * pos_x * pos_x + (pos_y * pos_y << 8) > 10240000
            ||		  tile_flags & (*g_active_tile_array_int)[rect_left + rect_top * g_map_tile_size->width]
            ||		  helper::clip_creep_area_in_bounds(rect_left, rect_top, (helper::ClipCreepAreaInBounds_Callback*)0x0047D860)
            ||		  helper::mark_cagra_to_disappear(rect_left, rect_top))
            {
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

    // Hooked to make Larvae move towards the spawner's rally angle. -Pr0nogo
    void orders_larva_idle(Unit* unit) { // 004E9F90
        Unit* spawner = unit->connected_unit;
        if (spawner && !helper::is_target_within_min_range(unit, 10, spawner)) {
            helper::move_to_target(spawner, unit);
            u32 spawner_pos_x = spawner->position.x;
            u32 spawner_pos_y = spawner->position.y;
            if (spawner_pos_x != unit->next_target_waypoint.x || spawner_pos_y != unit->next_target_waypoint.y) {
                unit->next_target_waypoint.x = spawner_pos_x;
                unit->next_target_waypoint.y = spawner_pos_y;
            }
        }

        u16 unit_x = unit->position.x;
        u16 unit_y = unit->position.y;
        u16 move_x = unit->move_target.pt.x;
        u16 move_y = unit->move_target.pt.y;
        Unit* move_target = unit->move_target.unit;
        if (move_x == unit_x && move_y == unit_y
        ||  move_target && (move_x != move_target->position.x || move_y != move_target->position.y) && !helper::pixel_pos_has_cagra(move_x, move_y))
        {
            if (!spawner && !(bw::get_active_tile_at_coords(unit->position)->has_cagra)) { // Guessed flag
                helper::orders_self_destructing(unit);
                return;
            }

            u32 pos_x = unit->position.x + (bw::rand_between(0, 1) ? 10 : -10);
            u32 pos_y = unit->position.y + (bw::rand_between(0, 1) ? 10 : -10);
            if (pos_x < g_map_width_pixels && pos_y < g_map_height_pixels && helper::pixel_pos_has_cagra(pos_x, pos_y)) {
                if (!helper::chk_larva_stray(unit, pos_x, pos_y)) {
                    if (helper::chk_larva_stray(unit, unit->sprite->position.x, unit->sprite->position.y)) {
                        return;
                    }
                    helper::chk_larva_stray0(unit, &pos_x, &pos_y);
                }
                helper::change_movement_target_move_target_2(unit, pos_x, pos_y);
            }
        }
    }
}