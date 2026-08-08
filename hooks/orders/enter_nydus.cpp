#include "enter_nydus.h"
#include <BW/api.h>

namespace hooks {
    // Necessary to assign the Enter Nydus order to units that normally can't use it
    bool can_enter_nydus(Unit* unit, Unit* nydus) { // 004E8C20
        Unit* nydus_exit = nydus->building.nydus_exit;
        if (!nydus_exit) {
            return false;
        }
        if (!nydus->is_completed() || !nydus_exit->is_completed()) {
            return false;
        }
        if (nydus->is_in_transport() || nydus_exit->is_in_transport()) {
            return false;
        }
        if (!bw::is_allied_to(unit->player, nydus->player)) {
            return false;
        }
        if (unit->is_in_air() || unit->is_flyer()) {
            return nydus->id == UnitId::ProtossWarpGateTheFall;
        }
        if (unit->ext().units[UnitHiveTissue] == nydus) {
            return false;
        }
        return true;
    }

    // Perform the Nydus teleport effect on an unit that validated the conditions within the orders_enter_nydus function
    void enter_nydus_effect(Unit* unit, Unit* nydus) { // 004EA180
        v2u16 old_pos = { unit->position.x, unit->position.y };
        v2u16 aimed_pos = {(nydus->building.nydus_exit)->position.x, (nydus->building.nydus_exit)->position.y};
        v2u16 final_pos = {};
        bw::prepare_unit_move(unit, true);
        helper::move_unit(unit, aimed_pos.x, aimed_pos.y);
        if (bw::check_unit_collision_pos(unit, &aimed_pos, &final_pos, 0, false, 0)) {
            SoundId::Enum nydus_sound_id = SoundId::LegionnaireTeleport01;
            SoundId::Enum target_sound_id = SoundId::LegionnaireTeleport01;
            switch (nydus->get_race()) {
                case RaceId::Terran: {
                    nydus_sound_id = SoundId::TerranUnitDeployment01;
                    target_sound_id = SoundId::TerranUnitDeployment01;
                } break;
                case RaceId::Protoss: {
                    nydus_sound_id = SoundId::Protoss_ARBITER_PAbFol02_WAV;
                    target_sound_id = static_cast<SoundId::Enum>(bw::rand_between(SoundId::ProtossUnitDeployment01, SoundId::ProtossUnitDeployment03));
                } break;
                case RaceId::Zerg: {
                    nydus_sound_id = SoundId::Misc_IntoNydus_wav;
                    target_sound_id = SoundId::Misc_IntoNydus_wav;
                } break;
                case RaceId::Askosi: {
                    nydus_sound_id = SoundId::AskosiUnitDeployment01;
                    target_sound_id = SoundId::AskosiUnitDeployment01;
                } break;
            }
            bw::play_sound(nydus_sound_id, nydus->position);
            helper::move_unit(unit, final_pos.x, final_pos.y);
            helper::refresh_unit_after_move(unit);
            bw::play_sound(target_sound_id, unit->position);
            if (unit->order_queue_head) {
                unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
                prepare_for_next_order(unit);
            } else {
                if (unit->unit_ai) {
                    unit->order_computer(OrderId::ComputerAI);
                } else {
                    unit->order_computer(units_dat::return_to_idle_order[unit->id]);
                }
            }
        } else {
            // collision detected, restore old position
            helper::move_unit(unit, old_pos.x, old_pos.y);
            helper::refresh_unit_after_move(unit);
        }
    }

    void orders_enter_nydus(Unit* unit) { // 004EA3E0
        Unit* nydus = unit->order_target.unit;
        if (!nydus) {
            return;
        }
        if (!nydus->is_valid_nydus() || (!bw::is_allied_to(unit->player, nydus->player))) {
            if (unit->order_queue_head) {
                unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
                prepare_for_next_order(unit);
            } else {
                if (unit->unit_ai) {
                    unit->order_computer(OrderId::ComputerAI);
                } else {
                    unit->order_computer(units_dat::return_to_idle_order[unit->id]);
                }
            }
            return;
        }
        if (unit->main_order_state == 0) {
            if (helper::order_to_move_to_target(unit, nydus)) {
                unit->main_order_state = 1;
            }
        } else if (unit->main_order_state == 1) {
            u8 movable_state = unit->get_movable_state();
            if (movable_state == MovableState_ReachedDestination) {
                unit->main_order_state = 2;
            } else if (movable_state != MovableState_NotReachedDestination) { // (value is probably 2 == unmovable)
                unit->order_to_idle();
            }
        }
        if (unit->main_order_state == 2) {
            enter_nydus_effect(unit, nydus);
        }
    }
} // namespace hooks
