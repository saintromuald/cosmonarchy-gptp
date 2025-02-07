#include "psi_field.h"
#include "../BW/bwdata.h"
#include "BW/api.h"
#include "trigger_system/trigger_system.h"

namespace hooks {
    // Actual state check for whether a unit can generate a psi field.
    bool is_ready_to_make_psi_field(Unit* unit) {
        return unit_id_is_psi_provider(unit->id);
    }

    // Conditions on which the power field should be shown.
    bool unit_can_show_power_for(Unit* unit, u32 player) {
        if (player > PLAYABLE_PLAYER_COUNT) {
            return true;
        }
        if (bw::player_is_witness(player) || bw::in_replay()) {
            return true;
        }
        if (!bw::is_allied_to(unit->player, player)) {
            return false;
        }
        return true;
    }

    // Hooked to allow allies to benefit from power. -Veeq7
    bool unit_has_power(u32 unit_id, s32 x, s32 y, u8 player) { // 004936B0
        if (!(units_dat::base_property[unit_id] & UnitProperty::RequiredPsi)) {
            return true;
        }
        if (player == NeutralHostilePlayer && bw::get_game_kind() == GameKind::Uprising) {
            return true;
        }

        u32 w = *psi_field_grp_width;
        u32 h = *psi_field_grp_height;
        for (Unit* unit = *g_first_psi_field_provider; unit; unit = unit->ext().psi_link.next) {
            UnitExt* unit_ext = get_unit_ext(unit);
            if (unit->status & UnitStatus::InTransport) {
                continue;
            }
            if (!unit_can_show_power_for(unit, player)) {
                continue;
            }
            if (fabs(unit->sprite->position.x - x) >= w || fabs(unit->sprite->position.y - y) >= h) {
                continue;
            }
            if ((*psi_field_mask)[2 * w / 32 * ((y + h - unit->sprite->position.y) / 32) + (x + w - unit->sprite->position.x) / 32]) {
                return true;
            }
        }
        return false;
    }

    // Hooked to show allied power fields. -Veeq7
    b32 add_psi_field_sprite(Unit* unit) { // 00493BF0
        Sprite* sprite = helper::create_sprite(SpriteId::PsiFieldRightUpper, unit->sprite->position.x, unit->sprite->position.y, unit->player); // Originally player was 0
        UnitExt& unit_ext = unit->ext();
        unit_ext.power_aura = sprite;
        if (!sprite) {
            return false;
        }
        sprite->elevation = 1;
        if (!unit_can_show_power_for(unit, g_local_nation_id)) {
            unit_ext.power_aura->flags |= SpriteFlags::Hidden;
            helper::refresh_all_visible_images_at_screen_position(unit_ext.power_aura, 0);
            return true;
        }
        if (g_psi_aura_is_drawn) {
            helper::refresh_all_visible_images_at_screen_position(unit_ext.power_aura, *g_player_visions);
            return true;
        }
        if (!(unit->sprite->flags & SpriteFlags::Selected) ) {
            unit_ext.power_aura->flags |= SpriteFlags::Hidden;
            helper::refresh_all_visible_images_at_screen_position(unit_ext.power_aura, 0);
            return true;
        }
        helper::create_pylon_aura();
        return true;
    }

    // Hooked to show allied power fields -Veeq7
    void __cdecl create_pylon_aura() { // 00493640
        for (Unit* unit = *g_first_psi_field_provider; unit; unit = unit->ext().psi_link.next) {
            UnitExt& unit_ext = unit->ext();
            if (!g_is_in_replay && unit_can_show_power_for(unit, g_local_nation_id) && (unit_ext.power_aura || add_psi_field_sprite(unit))) {
                unit_ext.power_aura->flags &= ~SpriteFlags::Hidden;
                helper::refresh_all_visible_images_at_screen_position(unit_ext.power_aura, *g_player_visions);
            }
        }
    }

    // Hooked to move psi provider to unitext. -Pr0nogo
    void __cdecl hide_all_psi_fields() { // 00494100
        for (Unit* power_node = *g_first_psi_field_provider; power_node; power_node = power_node->ext().psi_link.next) {
            UnitExt& unit_ext = power_node->ext();
            if (unit_ext.power_aura || helper::add_psi_field_sprite(power_node)) {
                unit_ext.power_aura->flags |= SpriteFlags::Hidden;
                helper::refresh_all_visible_images_at_screen_pos(unit_ext.power_aura, 0);
            }
        }
        g_psi_aura_is_drawn = 0;
    }

    // Hooked to add more cases. -Pr0nogo
    void __cdecl create_all_power_auras() { // 00494030
        for (Unit* power_node = *g_first_visible_unit; power_node; power_node = power_node->link.next) {
            if (unit_id_is_power_generator(power_node->id)) {
                if (power_node->is_completed()) {
                    helper::orders_init_psi_provider(power_node);
                    if (g_player_table[power_node->player].type == PlayerType::Rescuable) {
                        u32 order_id = power_node->main_order_id;
                        power_node->user_action_flags |= UserActionFlags::UserActionFlags_IscriptGoToIdle;
                        if (order_id) {
                            while (1) {
                                Order* tail = power_node->order_queue_tail;
                                if (!tail) {
                                    break;
                                }
                                u32 tail_order_id = tail->order_id;
                                if (!orders_dat::can_be_interrupted[tail_order_id] && tail_order_id != OrderId::RescuePassive) {
                                    break;
                                }
                                helper::remove_order_from_unit_queue(tail, power_node);
                            }
                            helper::perform_another_order(OrderId::RescuePassive, 0, power_node, UnitId::None, {}, 0);
                        }
                        helper::do_next_queued_order(power_node);
                    }
                }
            }
        }
    }

    // Hooked to move psi provider to unitext. -Pr0nogo
    void orders_init_psi_provider(Unit* unit) { // 00493F70
        UnitExt& unit_ext = unit->ext();
        Unit* first_provider = *g_first_psi_field_provider;
        if (first_provider == unit || unit_ext.psi_link.next || unit_ext.psi_link.prev) {
            u32 order_id = units_dat::return_to_idle_order[unit->id];
            unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
            helper::order(1, unit, order_id, {}, 0, UnitId::None);
            helper::do_next_queued_order(unit);
            helper::force_order_done(unit);
        } else {
            unit_ext.psi_link.prev = nullptr;
            unit_ext.psi_link.next = first_provider;
            if (first_provider) {
                UnitExt& provider_ext = first_provider->ext();
                provider_ext.psi_link.prev = unit;
            }
            *g_first_psi_field_provider = unit;
            helper::add_psi_field_sprite(unit);
            u32 order_id = units_dat::return_to_idle_order[unit->id];
            g_can_update_powered_status = 1;
            helper::issue_order_targeting_nothing(unit, order_id);
            helper::force_order_done(unit);
        }
    }
}