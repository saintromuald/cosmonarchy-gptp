#include "placement.h"
#include "data_ext.h"
#include <hooks/limits/sfx_extender.h>

namespace hooks {
    s32 init_placement_box_pos(s32 unused, Point16* placement_pos) { // 0048D660
        if (g_mouse->x < 0 || g_mouse->x >= 640 ||
            g_mouse->y < 0 || g_mouse->y >= 400) {
            return 0;
        }

        Vec32 pos = *g_mouse;
        Point16 dims = units_dat::building_dimensions[g_place_building_unit_id];
        pos.x -= dims.x / 2;
        pos.y -= dims.y / 2;

        g_place_building_x = (g_move_to_x + pos.x + 16) / 32;
        g_place_building_y = (g_move_to_y + pos.y + 16) / 32;
        placement_pos->x = g_place_building_x * 32 - g_move_to_x;
        placement_pos->y = g_place_building_y * 32 - g_move_to_y;
        return 1;
    }

    u8 draw_placement_box(Point16 unused) { // 0048DDC0
        Point16 placement_pos = unused;
        u8 result = g_is_placing_building;
        if (!result) {
            return result;
        }
        unused.x = g_mouse->y;
        if (helper::init_placement_box_pos(&placement_pos)) {
            g_place_building_message_id = helper::update_placement_state_for_placement_box();
            helper::refresh_layers_3_and_4();
            helper::unk_building_placement_related(1, &placement_pos);
            if (g_is_placing_addon) {
                placement_pos.x -= units_dat::addon_placement[g_place_building_unit_id].x;
                placement_pos.y -= units_dat::addon_placement[g_place_building_unit_id].y;
                helper::unk_building_placement_related(0, &placement_pos);
            }
            return helper::refresh_layers_3_and_4();
        }
        helper::unk_building_placement_related_mouse(0, 1);
        return helper::unk_building_placement_related_mouse(0, 0);
    }

    // Not enabled.
    s32 update_placement_state_for_placement_box() { // 0048DCE0
        if (!*g_active_portrait_unit) {
            return 0;
        }

        u32 source = g_place_building_base_type;
        u32 target = g_place_building_unit_id;
        u16 addon_x = (g_place_building_x - units_dat::addon_placement[2 * g_place_building_unit_id - 212].x / 32);
        u16 addon_y = (g_place_building_y - units_dat::addon_placement[2 * g_place_building_unit_id - 211].y / 32);
        bw::adjust_addon_coordinates(source, target, &addon_x, &addon_y);

        s32 can_build = helper::tile_location_is_unbuildable(*g_active_portrait_unit, g_placing_player, g_place_building_x, g_place_building_y, g_place_building_unit_id, 1, 1, 0, 0);
        s32 result = helper::tile_location_is_unbuildable(*g_active_portrait_unit, g_placing_player, addon_x, addon_y, g_place_building_base_type, 0, 1, 0, 0);
        // ^ this call might be needed, even if the result isn't returned, untested
        if (!g_is_placing_addon || can_build) {
            result = can_build;
        }
        return result;
    }

    // Hooked to enable unit mover for human players. - Pr0nogo
    s32 is_tile_blocked_by(Unit** unit_array, Unit* builder, s32 x_tile, s32 y_tile, s8 dont_ignore_reacting, s8 also_invisible) { // 00473300
        for (Unit** it = unit_array; *it; ++it) {
            Unit* other_unit = *it;
            bool needs_detection = false;
            if (builder) {
                if (bw::is_allied_to(other_unit->player, builder->player) && !unit_id_blocks_construction((UnitId::Enum)other_unit->id)) {
                    continue;
                }
                if (other_unit->status & (UnitStatus::RequiresDetection | UnitStatus::Cloaked)) {
                    if (!(other_unit->detection_status & (1 << builder->player))) {
                        needs_detection = true;
                    }
                }
            }
            if ((!dont_ignore_reacting && other_unit->status & UnitStatus::Reacts)
                || (needs_detection && !also_invisible)
                || other_unit == builder
                || other_unit->status & (UnitStatus::GroundedBuilding | UnitStatus::InAir | UnitStatus::Burrowed)
                || unit_id_is_spell(other_unit->id)
                || other_unit->sprite->flags & SpriteFlags::Hidden)
            {
                continue;
            }

            if (g_unit_ordering_x[other_unit->finder_index.left].position   < 32 * x_tile + 32
            &&  g_unit_ordering_x[other_unit->finder_index.right].position  > 32 * x_tile
            &&  g_unit_ordering_y[other_unit->finder_index.top].position    < 32 * y_tile + 32
            &&  g_unit_ordering_y[other_unit->finder_index.bottom].position > 32 * y_tile)
            {
                return needs_detection ? 1 : 4;
            }
        }
        return 0;
    }

    // Hooked to allow building on allied creep. -Pr0nogo
    int update_creep_building_placement_state(u8 player, u16 x_tile, u16 y_tile, Point16 size, u32 placement_state_entry, u32 check_temporary_creep, u32 check_creep) { // 00473010
        u32 human_only_player_bit = 0;
        if (player == 0xff) return 0; // First time calling bug fix -Jesse
        if (g_player_table[player].type == PlayerType::Human) {
            human_only_player_bit = 1 << player;
        }

        u8 result = 0;
        for (u32 y = 0; y < size.y; ++y) {
            for (u32 x = 0; x < size.x; ++x) {
                ActiveTile* tile = bw::get_active_tile_at_tile(x_tile + x, y_tile + y);

                char state = 0;
                if (tile->explored_flags & human_only_player_bit) {
                    state = 10;
                    result = 10;
                }
                bool creep = !tile->has_cagra;
                if (check_temporary_creep) {
                    creep = !tile->temporary_cagra;
                }
                if ((check_temporary_creep || check_creep || !(tile->visibility_flags & human_only_player_bit)) && creep) {
                    state = 5;
                    if (result < 5) {
                        result = 5;
                    }
                }
                g_building_placement_state[6 * (y + 8 * placement_state_entry) + x] = state;
            }
        }
        return result;
    }

    // Hooked to allow building on allied creep. -Pr0nogo
    u32 update_building_placement_state_map_tile_flags(u16 unit_id, u16 tile_x, u16 tile_y, u8 player, Point16 size, u32 placement_state_entry, u32 check_creep) { // 00473A10
        u32 human_only_player_bit = 0;
        if (player == 0xff) return 0; // First time calling bug fix -Jesse
        if (g_player_table[player].type == PlayerType::Human) {
            human_only_player_bit = 1 << player;
        }

        u8 result = 0;
        for (u32 y = 0; y < size.y; ++y) {
            for (u32 x = 0; x < size.x; ++x) {
                ActiveTile* tile = bw::get_active_tile_at_tile(tile_x + x, tile_y + y);
                TileExt* tile_ext = get_tile_ext_at_tile(tile_x + x, tile_y + y);

                u32 flags = 0x20800000; // cliff_edge, always_unbuildable
                if (!units_dat::group_flags[unit_id].is_zerg && !can_place_on_creep_tile(tile_ext, player)) {
                    flags |= check_creep ? 0x40000000 : 0x400000;
                }

                bool buildable = false;
                if (!check_creep && tile->visibility_flags & human_only_player_bit || !(tile->flags & flags) || tile->flags < 0) {
                    buildable = true;
                }

                u8 state = 0;
                if (tile->explored_flags & human_only_player_bit) {
                    //state = 10;

                    if (units_dat::base_property[unit_id] & UnitProperty::ResourceDepot && helper::find_blocking_fow_resource(x + tile_x, y + tile_y, 3)) {
                        state = 2;
                    }

                    u32 flags = g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[player];
                    if (!(flags & VespeneDeaths_DisableQuasifog)) {
                        if (!buildable) {
                            state = 4;
                        }
                    }
                } else if (buildable) { // originally also checked for if the unit id was any racial beacon
                    if (units_dat::base_property[unit_id] & UnitProperty::ResourceDepot && helper::place_building_resource_restriction_check(check_creep, x + tile_x, y + tile_y)) {
                        state = 2;
                    }
                } else {
                    state = 4;
                }
                g_building_placement_state[6 * (y + (8 * placement_state_entry)) + x] = state;
                if (result < state) {
                    result = state;
                }
            }
        }
        return result;
    }

    // Hooked to port unit mover logic from aise to GPTP. -Pr0nogo
    s32 start_building(Unit* builder) { // 00467030
        u16 building_id = builder->build_queue[builder->build_queue_slot];
        u32 player = builder->player;
        g_ai_minerals_reserve[player] = units_dat::mineral_cost[building_id];
        g_ai_vespene_reserve[player] = units_dat::vespene_cost[building_id];

        // Originally had code checking for supplies.
        if (g_game_minerals[player] < g_ai_minerals_reserve[player]) {
            helper::display_error_message_id_with_sfx(player, 850, (s32)g_console_index + 147); // Not enough minerals...mine more minerals.<0>
            return false;
        }
        if (g_game_vespene[player] < g_ai_vespene_reserve[player]) {
            helper::display_error_message_id_with_sfx(player, 851, (s32)g_console_index + 150); // Not enough Vespene gases....harvest more gas.<0>
            return false;
        }
        if (helper::tile_location_is_unbuildable(builder, player, (builder->order_target.pt.x - units_dat::building_dimensions[building_id].x / 2) / 32, (builder->order_target.pt.y - units_dat::building_dimensions[building_id].y / 2) / 32, building_id, 0, 0, 1, 0)) {
            helper::play_worker_error_sound(builder, 856); // You can't build there.<0>
            return false;
        } else {
            player = builder->player;
            g_game_minerals[player] -= g_ai_minerals_reserve[player];
            g_game_vespene[player] -= g_ai_vespene_reserve[player];
            bw::prepare_building_area(builder->order_target.pt, building_id);
            return true;
        }
    }

    // Hooked to enable more refinery types. -Pr0nogo
    void place_building_refinery_check(Unit* result) { // 0048E1E0
        if (unit_id_is_capped_gas_node(g_place_building_unit_id)) {
            if (helper::can_draw_placement_rect_maybe(g_place_building_unit_id, result)) {
                g_can_draw_placement_preview = 1;
            }
        }
    }

    // Hooked to enable more refinery types. -Pr0nogo
    void place_building_refinery_proc() { // 0048E430
        Unit* geyser = first_player_unit->unit[11];
        if (geyser) {
            u16 id = g_place_building_unit_id;
            do {
                if (geyser->id == UnitId::ResourceVespeneGeyser && unit_id_is_geyser_refinery(id) && id != UnitId::ZergEzigrisant) {
                    if (helper::can_draw_placement_rect_maybe(id, geyser)) {
                        g_can_draw_placement_preview = 1;
                    }
                }
                if (geyser->id == UnitId::ResourceVespeneRidge && unit_id_is_ridge_refinery(id)) {
                    if (helper::can_draw_placement_rect_maybe(id, geyser)) {
                        g_can_draw_placement_preview = 1;
                    }
                }
                geyser = geyser->player_link.next;
            } while (geyser);
        }
    }

    // Local function to explore Dedications. -Pr0nogo
    void place_custom_building_proc() {
        u32 player = g_local_nation_id;
        Unit* parent = 0;
        switch (g_place_building_unit_id) {
        case UnitId::ProtossPylon: {
            parent = first_player_unit->unit[player];
            if (parent) {
                UnitExt* parent_ext = get_unit_ext(parent);
                u16 id = g_place_building_unit_id;
                do {
                    switch (parent->id) {
                        case UnitId::ProtossGateway: {
                            if (helper::can_draw_placement_rect_maybe(id, parent)) {
                                g_can_draw_placement_preview = 1;
                            }
                        } break;
                    }
                } while (parent);
            }
        } break;
        }
    }

    // Hooked to enable more refinery types. -Pr0nogo
    void place_building_proc() { // 0048E6E0
        if (!(g_is_placing_addon)) {
            if (g_can_draw_placement_preview) {
                helper::prepare_placement_preview();
                g_can_draw_placement_preview = 0;
            }

            if (!unit_id_is_capped_gas_node(g_place_building_unit_id) && g_place_building_unit_id != UnitId::ZergEzigrisant) {
                if (units_dat::group_flags[g_place_building_unit_id].is_terran) {
                    helper::place_building_normal_proc();
                }
            } else {
                helper::place_building_refinery_proc();
            }
        }
    }

    // Hooked to enable more refinery types. -Pr0nogo
    b32 unit_id_is_refinery(u16 unit_id) { // 0047AFE0
        return unit_id_is_capped_gas_node(unit_id);
    }

    // Hooked to enable more refinery types. -Pr0nogo
    void issue_place_building_order(u16 unit_id, u8 order_id) { // 0048E730
        g_is_placing_building = 1;
        g_buttonset_special_buttonset_id = UnitId::Buttons_CancelPlaceBuilding;
        helper::update_current_buttonset();
        g_place_building_message_id = 0;
        *g_place_building_order_id = order_id;
        helper::prepare_placement_box(1, unit_id);

        if (units_dat::base_property[unit_id] & UnitProperty::Addon) {
            helper::prepare_placement_box(0, (*g_active_portrait_unit)->id);
            g_is_placing_addon = 1;
        } else if (!unit_id_is_capped_gas_node(g_place_building_unit_id)) {
            if (units_dat::group_flags[g_place_building_unit_id].is_terran) {
                helper::place_building_normal_proc();
            }
        } else {
            helper::place_building_refinery_proc();
        }

        const char* string = (char*)g_network_table_DF->get_string(64); // Select Location
        helper::send_message_something(2, string);
        helper::set_ingame_input_procs();
        Unit* active_unit = *g_active_portrait_unit;
        if (!active_unit) {
            g_placing_player = -1;
            helper::refresh_screen();
            g_minimap_cursor = 0;
            return;
        }

        if (units_dat::base_property[unit_id] & UnitProperty::RequiredPsi || unit_id_is_psi_provider(unit_id)) {
            helper::create_pylon_aura();
            active_unit = *g_active_portrait_unit;
        }

        g_minimap_cursor = 0;
        helper::refresh_screen();
        g_placing_player = active_unit->player;
    }

    // Not enabled.
    // Hooked to add support for custom races. -Pr0nogo
    void play_building_placement_sound(Unit* unit) { // 0048F630
        if (!(g_is_in_replay) && unit->player == g_local_nation_id) {
            u8 race = unit->get_race();
            u32 sound_id = race + 141;
            if (helper::play_sound_from_unit(sound_id, unit, 1, 99)) {
                helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[sound_id] + 1000 * (sfx_dat::sound_data[sound_id].duration_ms / 1000), 1, sound_id);
            }
        }
    }

    // Hooked to improve functionality, and later add handling for new races. -Pr0nogo
    void set_worker_error(Unit* unit, u32 error_string) { // 0048F7B0
        if (!(g_is_in_replay) && unit->player == g_local_nation_id) {
            u32 sound_id = SoundId::None;
            switch (unit->get_race()) {
                case RaceId::Terran:  sound_id = bw::get_advisor_sound_from_event_type(unit->player, AdvisorEvent_NoBuildTerran);  break;
                case RaceId::Protoss: sound_id = bw::get_advisor_sound_from_event_type(unit->player, AdvisorEvent_NoBuildProtoss); break;
                case RaceId::Zerg:    sound_id = bw::get_advisor_sound_from_event_type(unit->player, AdvisorEvent_NoBuildZerg);    break;
            }

            if (helper::play_sound_from_unit(sound_id, unit, 1, 99)) {
                helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[sound_id] + 1000 * (sfx_dat::sound_data[sound_id].duration_ms / 1000), 1, sound_id);
            }

            if (error_string) {
                const char* string = 0;
                if ((u16)(error_string - 1) < g_stat_txt_buffer.get_string_count32()) {
                    string = g_stat_txt_buffer.get_string32(error_string);
                    if (!string) {
                        return;
                    }
                } else {
                    string = "";
                }

                if (unit->player == g_local_nation_id && g_active_player_id == g_local_human_id) {
                    helper::set_unit_stat_txt_error_message(string);
                }
            }
        }
    }

    // Hooked to improve functionality, and later add handling for new races. -Pr0nogo
    void play_worker_error_sound(Unit* unit, u32 error_text_index) { // 0048F6C0
        if (!(g_is_in_replay) && unit->player == g_local_nation_id) {
            u32 sound_id = bw::get_advisor_sound_from_event_type(unit->player, AdvisorEvent_PlacementInterrupted);

            if (helper::play_sound_from_unit(sound_id, unit, 1, 99)) {
                helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[sound_id] + 1000 * (sfx_dat::sound_data[sound_id].duration_ms / 1000), 1, sound_id);
            }

            if (unit->player == g_local_nation_id && g_active_player_id == g_local_human_id) {
                char* string = (char*)g_stat_txt_buffer.get_string32(856);
                helper::set_unit_stat_txt_error_message(string);
            }
        }
    }

    // Hooked to add handling for new gas nodes. -Pr0nogo
    Thingy* __stdcall get_placement_restrictions_from_sprite_id(u32 tile_x, u32 tile_y, u32 radius) { // 00487B00
        if (!*g_first_fow_sprite) {
            return 0;
        }

        Thingy* fow_sprite_1 = *g_first_fow_sprite;
        Thingy* fow_sprite_2 = *g_first_fow_sprite;
        Sprite* sprite = fow_sprite_1->sprite;
        while (2) {
            u32 unit_id = UnitId::None;
            u32 sprite_id = fow_sprite_1->sprite->id;
            switch (sprite_id) {
                case SpriteId::Extractor:
                case SpriteId::Ezigrisant:
                case SpriteId::Assimilator:
                case SpriteId::Refinery:
                case SpriteId::Wellbore:
                case SpriteId::VespeneGeyser: {
                    unit_id = UnitId::ResourceVespeneGeyser;
                } break;
                case SpriteId::MineralFieldType1:
                case SpriteId::MineralFieldType2:
                case SpriteId::MineralFieldType3: {
                    unit_id = sprite_id - 103;
                } break;
                case SpriteId::Sieve:
                case SpriteId::Cedent:
                case SpriteId::Eskigant:
                case SpriteId::VespeneRidge: {
                    unit_id = UnitId::ResourceVespeneRidge;
                } break;
                default: {
                    goto LABEL_8;
                } break;
            }

            if (radius + tile_x - (sprite->position.x - units_dat::building_dimensions[unit_id].x / 2) / 32 >= (2 * radius + units_dat::building_dimensions[unit_id].x / 32)
            ||  radius + tile_y - (sprite->position.y - units_dat::building_dimensions[unit_id].y / 2) / 32 >= (2 * radius + units_dat::building_dimensions[unit_id].y / 32))
            {
                fow_sprite_1 = fow_sprite_2;
            LABEL_8:
                fow_sprite_1 = fow_sprite_1->next;
                fow_sprite_2 = fow_sprite_1;
                if (!fow_sprite_1) {
                    return 0;
                }
                continue;
            }
            return fow_sprite_2;
        }
    }

    // Hooked to turn stat_txt.tbl into a .txt. -Pr0nogo
    s32 place_building_message(s32 placement_state) { // 0048D930
        if (!placement_state) {
            return true;
        }
        if (bw::in_replay_or_witness()) {
            return false;
        }
        const char* string = g_stat_txt_buffer.get_string32(g_string_id[placement_state]);
        if (!string) {
            return false;
        }
        if (*g_active_nation_id == g_local_nation_id && g_active_player_id == g_local_human_id) {
            helper::set_unit_stat_txt_error_message(string);
        }
        return false;
    }

    // Hooked to turn stat_txt.tbl into a .txt.
    // Not working yet, not injected. Will revisit in another PR. -Pr0nogo
    void input_place_building_click_notify(u8* order_id, u16* building_id, bool is_valid) { // 0048E4E0
        if (order_id) {
            *order_id = *g_place_building_order_id;
        }
        if (!is_valid) {
            if (g_place_building_message_id) {
                if (*order_id != OrderId::BuildingLand) {
                    helper::set_worker_error(*g_active_portrait_unit, g_string_id[g_place_building_message_id]);
                    if (building_id) {
                        *building_id = UnitId::None;
                    }
                    return;
                }
                if (*g_active_nation_id == g_local_nation_id && g_active_player_id == g_local_human_id) {
                    helper::set_unit_stat_txt_error_message(g_stat_txt_buffer.get_string32(863)); // You can't land there.
                }
                helper::play_building_placement_sound(*g_active_portrait_unit);
                if (building_id) {
                    *building_id = UnitId::None;
                }
                return;
            }
            helper::play_building_placement_sound_0(g_place_building_unit_id);
        }
        if (building_id) {
            *building_id = g_place_building_unit_id;
        }
        helper::refresh_layers_3_and_4();
        helper::end_building_placement();
    }

    // Hooked to allow ridge capping. -Pr0nogo
    // We might be able to use a town center variant of this function (if it exists) to fix the bug that greenlights quasifog placement too close to resources.
    s32 vespene_geyser_placebox(u32 tile_x, u32 tile_y, u8 player, u32 check_vision, u32 unit_id) { // 00473DB0
        u32 node_id = unit_id_is_ridge_refinery(unit_id) ? UnitId::ResourceVespeneRidge : UnitId::ResourceVespeneGeyser;
        Point16 node_dims = units_dat::building_dimensions[node_id];
        u32 x_pos = 32 * tile_x + node_dims.x / 2;
        u32 y_pos = 32 * tile_y + node_dims.y / 2;
        if (g_player_table[player].type != PlayerType::Human)  {
            return helper::find_unit_at_tile(x_pos, y_pos, node_id) != 0 ? (s32)Placeability::Valid : (s32)Placeability::InvalidExplored;
        }
        u32 node_width = node_dims.x / 32;
        u32 node_height = node_dims.y / 32;
        if (helper::tiles_in_rect_are_not_explored_for(node_width, node_height, tile_x, tile_y, player)) {
            return (s32)Placeability::InvalidUnexplored;
        }
        if (!helper::tiles_in_rect_are_not_visible_for(node_width, node_height, tile_x, tile_y, player)) {
            return helper::find_unit_at_tile(x_pos, y_pos, node_id) != 0 ? (s32)Placeability::Valid : (s32)Placeability::InvalidExplored;
        }

        u32 node_sprite_id = node_id == UnitId::ResourceVespeneRidge ? SpriteId::VespeneRidge : SpriteId::VespeneGeyser;
        Thingy* parent = helper::get_placement_parent(node_sprite_id, x_pos, y_pos, node_dims);
        Sprite* parent_sprite = 0;
        if (parent) {
            parent_sprite = parent->sprite;
        }
        if (!check_vision || (parent_sprite && parent_sprite->position.x == x_pos && parent_sprite->position.y == y_pos)) {
            return (s32)Placeability::Valid;
        }
        return (s32)Placeability::InvalidExplored;
    }
}