#include "create_init_units.h"
#include <BW/api.h>
#include <BW/UnitFinder.h>
#include "ai_layouts.h"
#include "threshold.h"
#include "hooks/game_hooks.h"
#include "trigger_system/trigger_system.h"
#include "Game.h"

namespace hooks {
    void create_initial_melee_buildings(u8 race_id, u32 player) { // 0049D7C0
        u32 building_id = get_racial_town_center_id(race_id);
        GameKind game_kind = bw::get_game_kind();
        bool is_computer_player = g_player_table[player].type == PlayerType::Computer || g_player_table[player].type == PlayerType::ComputerSlot;
        bool is_omnipresence = game_kind == GameKind::Omnipresence;
        bool is_duopoly = game_kind == GameKind::Duopoly;

        if (!is_computer_player && (is_omnipresence || is_duopoly)) {
            building_id = UnitId::Omnivale;
        }
        s32 building_size_x, building_size_y;
        Box16 search_box;
        Unit* created_unit;

        building_size_x = units_dat::building_dimensions[building_id].x;
        building_size_y = units_dat::building_dimensions[building_id].y;
        if (building_size_x < 0) {
            building_size_x++;
        }
        if (building_size_y < 0) {
            building_size_y++;
        }

        search_box.left = (g_start_positions[player].x - (s16)(building_size_x / 2)) & 0xFFE0;
        search_box.top = (g_start_positions[player].y - (s16)(building_size_y / 2)) & 0xFFE0;
        search_box.bottom = building_size_y / 2 + ((g_start_positions[player].y - (s16)(building_size_y / 2)) & 0xFFE0) + building_size_y / 2 - 1;
        search_box.right = building_size_x / 2 + ((g_start_positions[player].x - (s16)(building_size_x / 2)) & 0xFFE0) + building_size_x / 2 - 1;

        // yes, the return value is unused for some reason
        helper::find_best_unit(&search_box, 0x0049D640);
        created_unit = helper::create_unit(building_id,
                ((g_start_positions[player].x - (building_size_x / 2)) & 0xFFE0) + building_size_x / 2,
                ((g_start_positions[player].y - (building_size_y / 2)) & 0xFFE0) + building_size_y / 2,
                player);

        if (created_unit) {
            helper::update_unit_stats_and_finish_building(created_unit);
            if (helper::show_finished_unit(created_unit) != 0) {
                helper::update_unit_strength(created_unit);
            }
            if (helper::spreads_creep(created_unit->id, 1) || units_dat::base_property[created_unit->id] & UnitProperty::CreepBuilding) {
                helper::apply_creep_at_location_from_unit_id(created_unit->sprite->position.x, created_unit->sprite->position.y, created_unit->id);
            }
        }
    }

    static fp32 sign(fp32 x) {
        return x < 0.0fp ? -1.0fp : x > 0.0fp ? 1.0fp : 0.0fp;
    }

    static Point16 find_worker_spawn_positions(s32 townhallId, const u16& startX, const u16& startY, bool& horizontal) {
        s32 sum_x = 0;
        s32 sum_y = 0;
        u32 count = 0;

        bw::UnitFinder unit_finder;
        unit_finder.search(
            startX - 10 * 32,
            startY - 10 * 32,
            startX + 10 * 32,
            startY + 10 * 32
        );
        u8 cliff_level = bw::get_active_tile_at_coords(startX, startY)->ground_height;
        unit_finder.for_each([&](Unit* unit) {
            if (unit_id_is_minerals(unit->id) && bw::get_active_tile_at_coords(unit->position.x, unit->position.y)->ground_height == cliff_level)
            {
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

        const auto averageX = fp32(sum_x) / count;
        const auto averageY = fp32(sum_y) / count;
        const auto distance = pow(pow(averageX, 2) + pow(averageY, 2), 0.5fp);

        auto offset_x = averageX / distance;
        auto offset_y = averageY / distance;

        const fp32 distance_multiplier{1.4};
        auto width = distance_multiplier;
        auto height = distance_multiplier;

        if (offset_x < 0fp) {
            width *= units_dat::unit_bounds[townhallId].left;
        } else {
            width *= units_dat::unit_bounds[townhallId].right;
        }

        if (offset_y < 0fp) {
            height *= units_dat::unit_bounds[townhallId].top;
        } else {
            height *= units_dat::unit_bounds[townhallId].bottom;
        }

        offset_x = offset_x * width;
        offset_y = offset_y * height;

        const fp32 diagonal_threshold{0.5}; // 1 - full diagonal, 0 - straight line
        bool force_horizontal = false;
        const auto max = std::max(abs(offset_x), abs(offset_y));
        const auto min = std::min(abs(offset_x), abs(offset_y));
        if (min != 0fp && diagonal_threshold != 0fp) {
            force_horizontal = max / min < 1 / diagonal_threshold;
        }

        if (force_horizontal || abs(offset_x) < abs(offset_y)) {
            offset_x = -units_dat::unit_bounds[townhallId].left * distance_multiplier;
            offset_y = sign(offset_y) * height;
            horizontal = true;
        } else {
            offset_x = sign(offset_x) * width;
            offset_y = units_dat::unit_bounds[townhallId].bottom * distance_multiplier * 1.5fp;
            horizontal = false;
        }

        return Point16{(u16)(startX + offset_x), (u16)(startY + offset_y)};
    }

    void create_initial_melee_units() {
        u32 player = 8;
        u8* race_id_offset = (u8*)0x0057F009;

        while (race_id_offset != (u8*)0x0057EEE9) {
            race_id_offset -= 0x24;
            player--;

            if (*(race_id_offset - 1) == 2 || *(race_id_offset - 1) == 1) {
                // 0 is use map settings, 1 is Workers, 2 is Workers + Main Building
                StartingUnits starting_units = (StartingUnits)g_starting_units;
                auto race_id = g_player_table[player].race;

                if (g_victory_conditions == 0 && starting_units == StartingUnits::MapUnits && g_tournament_mode == 0 && player < PlayerLimit && (g_player_force[player]) != 0) {
                    // race selected instead of using map settings?
                    starting_units = StartingUnits::WorkersTownCenter;
                }

                if (g_globals->players[player].no_starting_units == true) {
                    starting_units = StartingUnits::MapUnits;
                }

                if (starting_units == StartingUnits::WorkersTownCenter) {
                    create_initial_melee_buildings(race_id, player);
                    if (race_id == RaceId::Zerg) {
                        create_initial_ovileth(player);
                    }
                }

                if (starting_units == StartingUnits::WorkersOnly || starting_units == StartingUnits::WorkersTownCenter) {
                    GameKind game_kind = bw::get_game_kind();
                    bool is_computer_player = g_player_table[player].type == PlayerType::Computer || g_player_table[player].type == PlayerType::ComputerSlot;
                    bool is_omnipresence = game_kind == GameKind::Omnipresence;
                    bool is_duopoly = game_kind == GameKind::Duopoly;
                    u32 worker_id = get_racial_worker_id(race_id);
                    u8 townhall_id = get_racial_town_center_id(race_id);
                    if (!is_computer_player && (is_omnipresence || is_duopoly)) {
                        townhall_id = UnitId::Omnivale;
                    }

                    if (worker_id != UnitId::None) {
                        Point16 start_pos = g_start_positions[player];
                        bool horizontal = true;
                        Point16 pos = find_worker_spawn_positions(townhall_id, start_pos.x, start_pos.y, horizontal);

                        s32 dx = 0;
                        s32 dy = 0;
                        if (horizontal) {
                            dx = units_dat::unit_bounds[worker_id].left + units_dat::unit_bounds[worker_id].right + 1;
                        } else {
                            dy = units_dat::unit_bounds[worker_id].bottom + units_dat::unit_bounds[worker_id].top + 1;
                        }

                        for (s32 worker_count = 0; worker_count < 6; worker_count++) {
                            pos.x += dx;
                            pos.y -= dy;

                            if (is_omnipresence && !is_computer_player) {
                                switch (worker_count) {
                                    case 0:
                                    case 1: {
                                        worker_id = UnitId::TerranMason;
                                    } break;
                                    case 2:
                                    case 3: {
                                        worker_id = UnitId::ProtossScribe;
                                    } break;
                                    case 4:
                                    case 5: {
                                        worker_id = UnitId::ZergDroleth;
                                    } break;
                                }
                            }

                            if (is_duopoly && !is_computer_player) {
                                switch (g_player_table[player].race) {
                                    case RaceId::Terran: {
                                        switch (worker_count) {
                                            case 0:
                                            case 1:
                                            case 2: {
                                                worker_id = UnitId::TerranMason;
                                            } break;
                                            case 3:
                                            case 4:
                                            case 5: {
                                                worker_id = UnitId::ProtossScribe;
                                            } break;
                                        }
                                    } break;
                                    case RaceId::Protoss: {
                                        switch (worker_count) {
                                            case 0:
                                            case 1:
                                            case 2: {
                                                worker_id = UnitId::ProtossScribe;
                                            } break;
                                            case 3:
                                            case 4:
                                            case 5: {
                                                worker_id = UnitId::ZergDroleth;
                                            } break;
                                        }
                                    } break;
                                    case RaceId::Zerg: {
                                        switch (worker_count) {
                                            case 0:
                                            case 1:
                                            case 2: {
                                                worker_id = UnitId::TerranMason;
                                            } break;
                                            case 3:
                                            case 4:
                                            case 5: {
                                                worker_id = UnitId::ZergDroleth;
                                            } break;
                                        }
                                    } break;
                                }
                            }

                            Unit* created_unit = helper::create_unit(worker_id, pos.x, pos.y, player);
                            if (created_unit) {
                                bw::teleport_to_point(created_unit, pos);
                                helper::update_unit_stats_and_finish_building(created_unit);
                                if (helper::show_finished_unit(created_unit) != 0) {
                                    helper::update_unit_strength(created_unit);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
        
    void create_initial_ovileth(u8 player) {
        return; // Ovileth no longer spawns

        static u16* const max_box_right_value = (u16*)0x00628450;
        static u16* const max_box_bottom_value = (u16*)0x006284B4;
        s32 ovileth_start_x, ovileth_start_y;
        Unit* created_unit;
        if (g_start_positions[player].x < *max_box_right_value / 2) {
            ovileth_start_x = g_start_positions[player].x + units_dat::unit_bounds[UnitId::ZergHachirosk].right + units_dat::unit_bounds[UnitId::ZergHachirosk].left + 1;
        } else {
            ovileth_start_x = g_start_positions[player].x - (units_dat::unit_bounds[UnitId::ZergHachirosk].right + units_dat::unit_bounds[UnitId::ZergHachirosk].left + 1);
        }

        if (g_start_positions[player].y < *max_box_bottom_value / 2) {
            ovileth_start_y = g_start_positions[player].y + units_dat::unit_bounds[UnitId::ZergHachirosk].bottom + units_dat::unit_bounds[UnitId::ZergHachirosk].top + 1;
        } else {
            ovileth_start_y = g_start_positions[player].y - (units_dat::unit_bounds[UnitId::ZergHachirosk].bottom + units_dat::unit_bounds[UnitId::ZergHachirosk].top + 1);
        }

        created_unit = helper::create_unit(UnitId::ZergOvileth, ovileth_start_x, ovileth_start_y, player);
        if (created_unit) {
            helper::update_unit_stats_and_finish_building(created_unit);
            if (helper::show_finished_unit(created_unit) != 0) {
                helper::update_unit_strength(created_unit);
            }
        }
    }

    Unit* create_unit_xy(u16 player_id, u16 id, u32 x, u32 y, bool prevent_creep) {
        if (units_dat::staredit_avail_flags[id] & 0x200 && !g_is_brood_war) {
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
                if ( g_network_table_DF->get_string_count() > 17) {
                    text = g_network_table_DF->get_string(18);
                }
                const char* text2 = helper::get_unit_name_stat_string(unit->id);
                char buffer[256] = {};
                sprintf_s(buffer, 256, text, text2, unit->sprite->position.x, unit->sprite->position.y);
                helper::unit_placement_error(buffer, 3);
                unit->user_action_flags |= UserActionFlags_SilentRemove;
                helper::orders_self_destructing(unit);
            }
        } else {
            helper::display_last_network_error_for_player(player_id);
        }
        return 0;
    }

    Unit* create_preplaced_unit(CHKUnit* chkunit) { // 004CD740
        u32 flags = chkunit->valid_state_flags & chkunit->state_flags;

        if (init_threshold_spawn(chkunit, flags)) {
            return 0;
        }

        if (!(flags & ChkUnitFlags_Cloaked) && chkunit->player == NeutralPlayer) {
            if (chkunit->unit_id == UnitId::TerranSupplyDepot) {
                g_game->m_collision_set_tiles.emplace_back(chkunit->x / 32, chkunit->y / 32);
                return 0;
            }
            if (chkunit->unit_id == UnitId::TerranSpiderMine) {
                g_game->m_collision_fill_tiles.emplace_back(chkunit->x / 32, chkunit->y / 32);
                return 0;
            }
        }

        // Hacky workaround for allowing user select + no starting units.
        if (chkunit->player < PlayerLimit) {
            switch (chkunit->unit_id) {
                case UnitId::VolatileEgg: {
                    if (flags & ChkUnitFlags_Cloaked) {
                        g_globals->players[chkunit->player].no_starting_units = true;
                        return 0;
                    }
                } break;
            }
        }

        g_chk_cache.push_back(*chkunit);

        // Spawn 'lifted' chkunits, only if the player's race matches the unit's race.
        if (flags & ChkUnitFlags_Lifted) {
        	if (!unit_id_is_race(chkunit->unit_id, g_player_table[chkunit->player].race)) {
        		return 0;
        	}
        }
        if (flags & ChkUnitFlags_Cloaked || chkunit->unit_id == UnitId::TownCenterMarker) {
            return 0;
        }
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
            hooks::init_unit(unit);
        }
        return unit;
    }

    u32 create_initial_team_melee_units(s32 player) { // 0049D8E0
        create_initial_melee_units();
        return 1; // unused
    }

    // Hooked to add more cases. -Pr0nogo
    bool chkunit_invalid_for_melee(CHKUnit* chkunit) { // 004CBE20
        if ((StartingUnits)g_starting_units != StartingUnits::MapUnits) {
            if (chkunit->player == 11) {
                return false;
            }
            if (unit_id_is_resource_node(chkunit->unit_id, false) || chkunit->unit_id == UnitId::VolatileEgg) {
                return false;
            }
        }
        return true;
    }

    // Hooked to troubleshoot preplaced Volatile Eggs not appearing. Ended up not being useful. -Pr0nogo
    u32 get_player_force(u8 player) { // 004A8B90
        if (player < PlayerLimit) {
            return g_player_force[player];
        }
        return 0;
    }
}