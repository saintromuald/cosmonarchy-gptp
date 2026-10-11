#include "recall_spell.h"
#include <BW/api.h>
#include "BW/stat_txt_spec.h"

namespace hooks {
    // Function performed on each target of Dimensional Recall by iterate_units_at_location_target_proc_recall_targets
    // that was validated through spells_recall_action_on_target_in_range.
    // Do the actual unit movement. -GPTP
    void spells_recall_action_on_valid_target_for_recall(Unit* target, Unit* caster) { // 004942D0
        v2u16 previous_pos = {};
        v2u16 new_pos = {};
        previous_pos.x = target->sprite->position.x;
        previous_pos.y = target->sprite->position.y;

        auto& caster_ext = caster->ext();
        if (!caster_ext.values[ValueDimensionalRecalling]) {
            return;
        }

        // Identical code to helper::move_unit(), but for some reason, is using another function to do exactly the same thing
        helper::move_unit_duplicate(target, caster->sprite->position.x, caster->sprite->position.y);

        if (!bw::check_unit_collision_pos(target, &(caster->sprite->position), &new_pos, 0, false, 0)) {
            // Collision detected, cancel the movement
            helper::move_unit(target, previous_pos.x, previous_pos.y);
        } else {
            // Finish the teleporting (and cut the recalled unit from a possible unit targeted by an order)
            bw::prepare_unit_move(target, false);
            target->order_target.unit = 0;
            helper::move_unit(target, new_pos.x, new_pos.y);
            helper::refresh_unit_after_move(target);

            // Stop some currently active orders
            if (target->id != UnitId::ZergCocoon) {
                target->order_computer(units_dat::return_to_idle_order[target->id]);
            }

            // Originally created a DimensionalRecall thingy over the unit

            // Disconnect a nuke from a nuker
            if (bw::unit_can_nuke(target)) {
                if (target->connected_unit && target->connected_unit->id == UnitId::TerranMorningstar) {
                    target->connected_unit->connected_unit = 0;
                    target->connected_unit = 0;
                }
            }
            g_units_recalled = 1;
        }
    }

    // Function performed on each target of DimensionalRecall by iterate_units_at_location_target_proc_recall_targets to choose if the unit
    // can have the DimensionalRecall effect cast on it or no, and if yes, calling another function to do the Dimensional Recall effect.
    void spells_recall_action_on_target_in_range(Unit* target, Unit* caster) { // 00494400
        if (target->sprite->is_hidden() || !bw::is_allied_to(target->player, caster->player) || target == caster) {
            return;
        }
        if (target->is_building()) {
            return;
        }
        if (unit_id_is_larva(target->id) || unit_id_is_zerg_egg(target->id)) {
            return;
        }
        if (target->is_burrowed()) {
            return;
        }
        if (target->is_invincible()) {
            return;
        }
        spells_recall_action_on_valid_target_for_recall(target, caster);
    }

    void tick_recall_channel_anim(Unit* unit) {
        auto& unit_ext = unit->ext();
        u32 index = unit_ext.values[ValueDidactChannelIndex] % 8;
        unit_ext.values[ValueDidactChannelIndex] += 1;

        v2u16 source_origin = {unit->position.x, unit->position.y};
        v2u16 target_origin = unit_ext.recall_pos;

        v2u16 source = source_origin;
        v2u16 target = target_origin;
        source.y -= 1 * 32;
        target.y -= 1 * 32;

        source = source.rotate(source_origin, fp32(index * (360 / 6)));
        target = target.rotate(target_origin, fp32(index * (360 / 6)));

        bw::create_and_init_thingy(SpriteId::Recall_Field, source.x, source.y, unit->player, unit->sprite->elevation - 1);
        bw::create_and_init_thingy(SpriteId::Recall_Field, target.x, target.y, unit->player, unit->sprite->elevation + 1);

        u32 sound_id_source = SoundId::Protoss_ARBITER_PAbFol00_WAV;
        u32 sound_id_target = SoundId::Protoss_ARBITER_PAbFol01_WAV;
        if (bw::random() % 2 == 0) {
            sound_id_source = SoundId::Protoss_ARBITER_PAbFol01_WAV;
            sound_id_target = SoundId::Protoss_ARBITER_PAbFol00_WAV;
        }
        helper::play_sound_at_pos(sound_id_source, source);
        helper::play_sound_at_pos(sound_id_target, target);

        unit->order_queue_timer = 12;
    }

    void orders_recall(Unit* unit) { // 00494470
        bw::decloak_passive_proc(unit);
        auto& unit_ext = unit->ext();
        // Unit has started casting Dimensional Recall
        if (unit->main_order_state == 1) {
            // Execute the recall effect when timer reach 0
            if (unit->main_order_timer == 0) {
                if (!unit_ext.values[ValueDimensionalRecalling]) {
                    unit_ext.values[ValueDimensionalRecalling] = true;
                    bw::set_generic_rust_value(unit, RustId::CancelRecall, 0);
                }

                unit->spend_energy(5 * 256 / 24);
                if (unit->energy == 0 || bw::get_generic_rust_value(unit, RustId::CancelRecall)) {
                    bw::set_generic_rust_value(unit, RustId::CancelRecall, 0);
                    unit_ext.values[ValueDidactChannelIndex] = 0;
                    unit_ext.values[ValueDimensionalRecalling] = false;

                    unit->status &= ~UnitStatus::CanNotReceiveOrders;
                    unit->order_to_idle();
                }

                if (unit_ext.timers[TimerRecallEffect] == 0) {
                    unit_ext.timers[TimerRecallEffect] = 8;

                    static b32* const recall_successful = (b32*)(0x0063FF50);
                    *recall_successful = false;
                    if (!unit_ext.timers[TimerRecallInterval]) {
                        if (Unit* target = g_unit_search.radial(unit->order_target.pt, Protoss::DimensionalRecallRange).closest_to(unit->order_target.pt)) {
                            unit_ext.timers[TimerRecallInterval] = Protoss::DimensionalRecallInterval;
                            spells_recall_action_on_target_in_range(target, unit);
                        }
                    }
                    if (*recall_successful) {
                        u32 sound_id_source = SoundId::Protoss_ARBITER_PAbFol02_WAV;
                        u32 sound_id_target = SoundId::Protoss_ARBITER_PAbFol03_WAV;
                        if (bw::random() % 2 == 0) {
                            sound_id_source = SoundId::Protoss_ARBITER_PAbFol03_WAV;
                            sound_id_target = SoundId::Protoss_ARBITER_PAbFol02_WAV;
                        }
                        helper::play_sound_at_pos(sound_id_source, unit->position);
                        helper::play_sound_at_pos(sound_id_target, unit->order_target.pt);
                    } else {
                        u32 sound_id_source = SoundId::Protoss_ARBITER_PAbFol00_WAV;
                        u32 sound_id_target = SoundId::Protoss_ARBITER_PAbFol01_WAV;
                        if (bw::random() % 2 == 0) {
                            sound_id_source = SoundId::Protoss_ARBITER_PAbFol01_WAV;
                            sound_id_target = SoundId::Protoss_ARBITER_PAbFol00_WAV;
                        }
                        helper::play_sound_at_pos(sound_id_source, unit->position);
                        helper::play_sound_at_pos(sound_id_target, unit->order_target.pt);
                    }

                    bw::create_and_init_thingy(SpriteId::RecallFieldShorter, unit->position.x, unit->position.y, unit->player, unit->sprite->elevation - 1);
                    bw::create_and_init_thingy(SpriteId::RecallFieldShorter, unit->order_target.pt.x, unit->order_target.pt.y, unit->player, unit->sprite->elevation + 1);
                }
            } else {
                unit_ext.energy_regen_timer = 0; // Prevent energy regen during channel
                tick_recall_channel_anim(unit);
            }
        } else {
            u32 energy_cost = techdata_dat::energy_cost[TechId::DimensionalRecall] * 256;
            if (!unit->has_energy(energy_cost)) {
                if (unit->player == g_local_nation_id && g_active_player_id == g_local_human_id) {
                    helper::set_unit_stat_txt_error_message("Not enough energy");
                }
                unit->order_to_idle();
            } else {
                unit->spend_energy(energy_cost);
                if (unit->order_target.unit) {
                    unit->order_target.pt.x = unit->order_target.unit->position.x;
                    unit->order_target.pt.y = unit->order_target.unit->position.y;
                }
                unit->status |= UnitStatus::CanNotReceiveOrders;
                unit->main_order_state = 1;
                unit->main_order_timer = 4 * 24;
                helper::make_to_hold_position(unit);
                unit_ext.values[ValueDidactChannelIndex] = 0;
                unit_ext.values[ValueDimensionalRecalling] = false;
                unit_ext.recall_pos = unit->order_target.pt;
                tick_recall_channel_anim(unit);
            }
        }
    }
}
