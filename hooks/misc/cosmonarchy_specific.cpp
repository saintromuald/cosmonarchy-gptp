#include "hooks/misc/cosmonarchy_specific.h"
#include "globals.h"
#include <cassert>
#include "hooks/orders/unit_making/unit_train.h"
#include "stopwatch/Stopwatch.h"
#include "logger.h"
#include "BW/bwdata.h"
#include "hooks/unit_stats/max_energy.h"
#include <array>

namespace hooks {

bool debug_damage_overlay(Sprite* sprite) {
    if (!sprite->main_graphic) {
        return false;
    }
    const LO_Header* overlay = lo_files->damage_overlays[sprite->main_graphic->id];
    if (!overlay) {
        return false;
    }
    for (Image* image = sprite->images.head; image; image = image->link.next) {
        if (image->id >= ImageId::FirstMinorDamageOverlay && image->id <= ImageId::LastMinorDamageOverlay) {
            s32 variation = image->id - ImageId::FirstMinorDamageOverlay;
            if (sprite->main_graphic->frame_index >= overlay->frame_count) {
                bw::printf("CreateDamageOverlay Bug: incorrect frame %d, image id %d", sprite->main_graphic->frame_index, sprite->main_graphic->id);
                {
                    return false;
                }
            }
        }
    }
    std::array<s32, 22> results;
    auto result_pos = results.begin();
    if (sprite->main_graphic->frame_index >= overlay->frame_count) {
        bw::printf("CreateDamageOverlay Bug: incorrect frame %d, image id %d", sprite->main_graphic->frame_index, sprite->main_graphic->id);
        return false;
    }
    return true;
}

void create_damage_overlay(Sprite* sprite) {
    if (!sprite->main_graphic) {
        return;
    }
    const LO_Header* overlay = lo_files->damage_overlays[sprite->main_graphic->id];
    if (!overlay) {
        return; // return if there's no overlay defined
    }
    for (Image* image = sprite->images.head; image; image = image->link.next) {
        if (image->id >= ImageId::FirstMinorDamageOverlay && image->id <= ImageId::LastMinorDamageOverlay) {
            s32 variation = image->id - ImageId::FirstMinorDamageOverlay;
            image->free();
            if (sprite->main_graphic->frame_index >= overlay->frame_count) {
                bw::printf("CreateDamageOverlay Bug: incorrect frame %d, image id %d", sprite->main_graphic->frame_index, sprite->main_graphic->id);
                {
                    return;
                }
            }
            Point8 offset = overlay->get_offset(sprite->main_graphic->frame_index, variation);
            sprite->create_top_overlay(ImageId::FirstMajorDamageOverlay + variation, offset.x, offset.y, 0);
        }
    }
    std::array<s32, 22> results;
    auto result_pos = results.begin();
    if (sprite->main_graphic->frame_index >= overlay->frame_count) {
        return;
    }
    for (s32 i = 0; i < 22; i++) {
        Point8 offset = overlay->get_offset(sprite->main_graphic->frame_index, i);
        if (offset.x == 127 && offset.y == 127) {
            continue;
        }
        bw::printf("Overlay: %d", i);
        u32 overlay_id = ImageId::FirstMajorDamageOverlay + i;
        bool found = false;
        for (Image* image = sprite->images.head; image; image = image->link.next) {
            if (image->id == overlay_id) {
                found = true;
                break;
            }
        }
        if (!found) {
            *result_pos++ = i;
        }
    }
    if (result_pos != results.begin()) {
        u32 rand_amount = bw::rand_between(0, result_pos - results.begin() - 1);
        s32 variation = results[rand_amount];
        Point8 offset = overlay->get_offset(sprite->main_graphic->frame_index, variation);
        sprite->create_top_overlay(ImageId::FirstMinorDamageOverlay + variation, offset.x, offset.y, 0);
    }
}

u32 real_top_speed(Flingy* flingy, u32 flingy_id) {
    u32 speed = flingy_dat::top_speed[flingy_id];
    return speed;
}

u16 real_acceleration(Flingy* flingy, u32 flingy_id) {
    u16 acceleration = flingy_dat::acceleration[flingy_id];
    return acceleration;
}

bool is_high_templar_conduit(u16 unit_id) {
    switch (unit_id) {
        case UnitId::ProtossAugur: {
            return true;
        }
        default: {
            break;
        }
    }
    return false;
}

u32 weapon_flingy(u32 weapon_id, u8 player_id) {
    return weapons_dat::flingy_id[weapon_id];
}

u32 get_flingy_halt_distance(Flingy* flingy) {
    u32 speed = flingy->current_speed_2;
    if (speed == 0 || flingy->flingy_movement_type != 0) {
        return 0;
    }

    u32 flingy_id = flingy->flingy_id;
    if (speed != real_top_speed(flingy, flingy_id) || flingy->flingy_acceleration != real_acceleration(flingy, flingy_id)) {
        return speed * speed / (2 * (u16) flingy->flingy_acceleration);
    } else {
        return flingy_dat::halt_distance[flingy_id];
    }
}

bool is_unused_order(u8 order_id) {
    switch (order_id) {
        case OrderId::GuardianAspect:
        case OrderId::PaladinSkyfall: // OrderId::AttackTile
        case OrderId::Hover:
        case OrderId::Warpin:
        case OrderId::CTFCOP2: {
            return true;
        }
        default:
        {
        } break;
    }
    return false;
}

u32 extend_tech_target_check(u32 orig_result, Unit* caster, Unit* target, u8 techId) {
    if (orig_result == 0) { // Valid target 
        bw::print("Can cast");
        if (techId == TechId::Lockdown) {
            bw::printf("Lockdown %d %d", caster->order_target.pt.x, caster->order_target.pt.y);
        }
    }
    return orig_result;
}

bool can_attack_target_end_chunk(Unit* source, u32 target_flags) {
    if (target_flags & UnitStatus::InAir) {
        if (source->get_air_weapon() != WeaponId::None) {
            return true;
        }
    } else {
        if (source->get_ground_weapon() != WeaponId::None) {
            return true;
        }
    }
    return false;
}

s32 game_loop() // 004D94B0 - hooked for lag testing - Pr0nogo
{
    bool v0;
    win32::Stopwatch sw;
    sw.Start();
    f64 aiLoop, unitLoop, imageThingyLoop, tileLoop;

    bw::set_ingame_loop_state(1);
    helper::initialize_randomizer_info();
    helper::ai_loop();
    aiLoop = sw.ElapsedMilliseconds();
    sw.Stop();

    if (!visionUpdateCount) {
        *visionUpdateCount = 100;
    }
    v0 = *visionUpdateCount == 1;
    *visionUpdateCount -= 1;
    *g_vision_updated = v0;
    sw.Start();
    if (v0) {
        helper::update_active_tile_info();
        helper::remove_fow_cheat();
    }
    sw.Stop();
    tileLoop = sw.ElapsedMilliseconds();
    sw.Start();
    helper::update_units();
    sw.Stop();
    sw.Start();
    unitLoop = sw.ElapsedMilliseconds();
    helper::image_drawing_bullet_drawing();
    helper::update_images();
    helper::update_thingies();
    sw.Stop();
    imageThingyLoop = sw.ElapsedMilliseconds();
    //	GPTP::logger << "aiLoop: " << aiLoop << ", unitLoop: " << unitLoop << ", " << "tile loop: " << tileLoop << ", thingy/bullet/imageLoop: " << imageThingyLoop << '\n';
    return bw::set_ingame_loop_state(0);
}

// 004A09D0 - hooked to add ccmu cases (protect against garbage pointers) - Pr0nogo
// Now used to set starting energy to a new amount - Veeq7
Unit* create_unit(u16 unit_id, s32 x, s32 y, s32 player) {
    Vec32 attack_frame = {};
    u32 facing_angle = bw::rand_between(0, 31);

    Unit* unit = helper::unit_constructor(x, y, unit_id, player, facing_angle);
    if (!unit) {
        return 0;
    }

    if (units_dat::base_property[unit_id] & UnitProperty::Spellcaster) {
        unit->energy = hooks::get_unit_max_energy(unit); // New starting amount
    } else {
        unit->energy = 0;
    }

    if (units_dat::base_property[unit_id] & UnitProperty::Addon) {
        bw::play_sound(SoundId::TerranConstructionStart01, unit->position);
    }

    if (unit->is_grounded_building()) {
        helper::visible_units_add(unit);
    } else {
        helper::list_push_back_hidden_unit_entry(unit);
    }

    if (unit_id_is_broodspawn(unit->id)) {
        unit->remove_timer = Broodspawn::LifeTimer;
    }

    if (units_dat::subunit[unit_id] == UnitId::None) {
        unit->subunit = 0;
        return unit;
    }

    Unit* subunit = helper::unit_constructor(x, y, units_dat::subunit[unit_id], player, facing_angle);
    if (!subunit) {
        helper::unit_destructor(unit);
        return 0;
    }

    unit->subunit = subunit;
    subunit->subunit = unit;
    helper::get_image_attack_frame(&attack_frame, unit->sprite, 0, 2);
    helper::set_sprite_main_image_offset(subunit->sprite, attack_frame.x, attack_frame.y);

    // New
    if (subunit) {
        helper::turn_unit(subunit, 96);
    }

    if (game_kind_doubles_units() && unit_should_give_gas_in_massacre_mode(unit)) {
        bw::add_massacre_resource(unit->player);
        UnitExt* unit_ext = get_unit_ext(unit);
        unit_ext->values[ValueCancellingZergStructure] = 0;
    }
    return unit;
}

// Hooked to set starting energy to a new amount - Veeq7
void transform_unit_main(Unit* unit, u32 unit_id) { // 0049F170
    u32 hp = unit->hitpoints;
    u32 order_type = unit->main_order_id;
    u32 order_state = unit->main_order_state;
    Target order_target = unit->order_target;
    unit->previous_unit_type = unit->id;
    unit->previous_hitpoints = hp / 256;
    if (!unit->previous_hitpoints) {
        unit->previous_hitpoints = 1;
    }
    Sprite* sprite = unit->sprite;
    u32 player_id = sprite->player;
    u32 x = sprite->position.x;
    u32 y = sprite->position.y;
    if (sprite->images.head) {
        helper::sprite_destructor(sprite);
        unit->sprite = 0;
    }
    if (unit->path) {
        helper::reassign_path(unit);
    }
    helper::initialize_unit(unit, y, unit_id, x, unit->player);
    if (units_dat::base_property[unit_id] & UnitProperty::Spellcaster) {
        unit->energy = hooks::get_unit_max_energy(unit); // New starting amount
    } else {
        unit->energy = 0;
    }
    unit->status = unit->status & ~UnitStatus::Frozen;

    // NOTE: When Zerg eggs and cocoons finish morphing, the unit they morph into gains proportional HP to the
    //		 HP of the egg or cocoon, i.e. missing HP % on the predecessor will also be missing for the birthed
    //		 unit, but only for a moment before their HP is correctly set in unit_morph.cpp. This is a lard bug
    //		 and we could write code here to always set birthing Zerg units to full HP, but I do not believe it
    //		 is actually necessary. -Pr0nogo
    u32 new_hp = (hp / 256) * (units_dat::max_hitpoints[unit->id] / 256);
    u32 max_hp = units_dat::max_hitpoints[unit->previous_unit_type] / 256;
    u32 final_hp = 1;
    if (max_hp) {
        final_hp = (new_hp / max_hp);
    }
    unit->hitpoints = final_hp * 256;
    if (!unit->hitpoints) {
        unit->hitpoints = 1;
    }
    unit->air_strength = helper::get_unit_strength(unit, max_hp, false);
    unit->ground_strength = helper::get_unit_strength(unit, max_hp, true);
    unit->status &= ~(UnitStatus::CanNotReceiveOrders | UnitStatus::NoBrkCodeStart);
    if (!orders_dat::can_be_interrupted[order_type]) {
        unit->status |= UnitStatus::CanNotReceiveOrders;
    }

    sprite = unit->sprite; // Reassigning since it's different after calling initialize_unit. -Pr0nogo
    if (sprite->player != player_id) {
        sprite->player = player_id;
        for (Image* image = sprite->images.head; image; image = image->link.next) {
            image->flags |= ImageFlags::Redraw;
        }
    }
    if (unit->status & UnitStatus::GroundedBuilding) {
        unit->build_queue[unit->build_queue_slot] = UnitId::None;
    }
    unit->main_order_id = order_type;
    unit->main_order_state = order_state;
    unit->order_target = order_target;

    u32 temp_unit_id = unit_id;
    if (!unit->is_completed()) {
        temp_unit_id = UnitId::None;
    }
    if (!unit->is_frozen() || unit->is_building() || temp_unit_id == UnitId::None) {
        unit->current_buttonset = temp_unit_id;
    }
    bw::refresh_console();
}

// Hooked to add nullcheck - Pr0nogo
u8 ignore_iscript_and_go_to_idle(Unit* unit) { // 00476550
    if (!unit) { // New
        return 0;
    }
    if (!AssertSprite(unit->sprite)) { // New
        return 0;
    }

    unit->status &= ~UnitStatus::NoBrkCodeStart;
    Unit* active_entity = *g_active_iscript_entity;
    Unit* active_subunit = *g_active_iscript_unit;
    unit->sprite->flags &= ~SpriteFlags::IscriptCode;
    *g_active_iscript_unit = unit;
    *g_active_iscript_entity = unit;
    helper::play_idle_iscript(unit->sprite);
    *g_active_iscript_unit = active_subunit;
    *g_active_iscript_entity = active_entity;
    unit->movement_flags &= ~MovementFlags_StartingAttack;
    return unit->movement_flags;
}

// Hooked to investigate crashes. -Pr0nogo
void turn_unit(Unit* unit, char direction) { // 00495F20
    unit->velocity_direction_1 = direction;
    unit->current_direction_1 = direction;
    unit->current_direction_2 = direction;
    unit->current_velocity.x = unit->current_speed_1 * g_angle_distance[direction].x >> 8;
    unit->current_velocity.y = unit->current_speed_1 * g_angle_distance[direction].y >> 8;
    Sprite* sprite = unit->sprite;
    u32 pos_x = sprite->position.x;
    u32 pos_y = sprite->position.y;
    if (pos_x != unit->next_target_waypoint.x || pos_y != unit->next_target_waypoint.y) {
        unit->next_target_waypoint.x = pos_x;
        unit->next_target_waypoint.y = pos_y;
    }
    for (Image* i = sprite->images.head; i; i = i->link.next) {
        helper::set_image_direction(i, direction);
    }
}

// Hooked to troubleshoot loading issues. -Pr0nogo
void init_damage_overlay_counts() { // 004D5C20
    u32 image_id = ImagesIdLimit;
    while (image_id) {
        --image_id;
        g_building_overlay_state_max[image_id] = 0;
        LO_Header* lo_header = lo_files->damage_overlays[image_id];
        if (lo_header) {
            u32 max_overlay_state = 0;
            u32 offset = 22;
            while (offset) {
                offset--;
                Point8 overlay_frame = lo_header->get_offset(0, offset);
                u32 width = overlay_frame.x;
                u32 height = overlay_frame.y;
                if (width != 127 || height != 127) {
                    ++max_overlay_state;
                }
            }
            if (max_overlay_state) {
                g_building_overlay_state_max[image_id] = 2 * max_overlay_state;
            }
        }
    }
}

// Hooked to investigate the sprite desync. -Pr0nogo
void __cdecl create_unit_building_sprite_validity_array() { // 00497110
    memset(g_current_visibility_hash, 0, 256);
    *g_want_visibility_hash_update = 0;
    *g_region_vision_hash = 0;
    *g_current_hash_offset = 0;
    memset(g_is_valid_sprite, 0, SpritesIdLimit); // Resetting validity array to zero (new) -Veeq7
    /*
    for (u32 i = 0; i < UnitsIdLimit; ++i) {
        if (!(units_dat::base_property[i] & UnitProperty::Building) && units_dat::staredit_avail_flags[i] & 1) {
            g_is_valid_sprite[flingy_dat::sprite_id[units_dat::graphic[i]]] = true;
        }
    }
    */
}

// Hooked to help with corpse-related abilities. -Pr0nogo
Thingy* create_thingy(u32 sprite_id, Point16 position, u8 player) { // 00488210
    Thingy* thingy = *g_first_free_thingy;
    if (!thingy) {
        return 0;
    }

    *g_first_free_thingy = thingy->next;
    if (*g_last_free_thingy == thingy) {
        *g_last_free_thingy = thingy->prev;
    }
    if (thingy->prev) {
        thingy->prev->next = thingy->next;
    }
    if (thingy->next) {
        thingy->next->prev = thingy->prev;
    }

    thingy->prev = 0;
    thingy->next = 0;
    thingy->unit_id = 1;
    Sprite* sprite = helper::create_sprite(sprite_id, position.x, position.y, player);
    thingy->sprite = sprite;
    if (sprite) {
        Thingy* first_thingy = *g_first_thingy;
        if (first_thingy) {
            if (*g_last_thingy == first_thingy) {
                *g_last_thingy = thingy;
            }
            thingy->prev = first_thingy;
            thingy->next = first_thingy->next;
            if (first_thingy->next) {
                first_thingy->next->prev = thingy;
            }
            first_thingy->next = thingy;

            reset_thingy_ext(thingy);
            return thingy;
        }

        *g_first_thingy = thingy;
        *g_last_thingy = thingy;

        reset_thingy_ext(thingy);
        return thingy;
    }

    Thingy* first_free_thingy = *g_first_free_thingy;
    if (first_free_thingy) {
        if (*g_last_free_thingy == first_free_thingy) {
            *g_last_free_thingy = thingy;
        }
        thingy->prev = first_free_thingy;
        thingy->next = first_free_thingy->next;
        if (first_free_thingy->next) {
            first_free_thingy->next->prev = thingy;
        }
        first_free_thingy->next = thingy;
        return 0;
    }

    *g_first_free_thingy = thingy;
    *g_last_free_thingy = thingy;
    return 0;
}

}