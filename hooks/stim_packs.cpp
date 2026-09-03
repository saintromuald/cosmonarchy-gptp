#include "stim_packs.h"
#include "../BW/api.h"
#include "BW/stat_txt_spec.h"

static void stimpack_proc(Unit* unit) {
    auto& unit_ext = unit->ext();
    unit->damage_hp(Terran::StimpackMinimumHP, 0, -1, true);
    switch (unit->id) {
        case UnitId::TerranMaverick:		unit_ext.timers[TimerStimpackDelay] = 1.5secs + 1; break;
        case UnitId::TerranMaverickUpstart: unit_ext.timers[TimerStimpackDelay] = 2;           break;
        default:							unit_ext.timers[TimerStimpackDelay] = 1.5secs + 1; break;
    }
    bw::decloak_passive_proc(unit);
    bw::play_sound(SoundId::StimpackInject01, unit->position);
    unit->sprite->create_top_overlay(ImageId::MaverickStimpackInject);
}

namespace hooks {
    // Local answer when using stimpacks (check units of selection, and can just display an error message without notifying others players
    // of the attempt if no unit can do the stimpack action). -GPTP
    void cmdact_stimpack() { // 004234D0
        bool able_to_stim = false;
        // Original code used pointer comparison with g_client_selection_group_end and incrementing a pointer starting at g_client_selection_group
        Unit* current_client_selection_unit;
        for (s32 i = 0; i < SelectionCap && !able_to_stim; i++) {
            current_client_selection_unit = g_client_selection_group->unit[i];
            if (current_client_selection_unit && current_client_selection_unit->hitpoints > Terran::StimpackMinimumHP) {
                able_to_stim = true;
            }
        }

        if (able_to_stim) {
            static u8 opcode[] = {0x36}; // packet opcode for stimpack is 0x36, no others args
            helper::queue_game_command(&opcode[0], 1);
        } else {
            helper::get_stat_txt_string(*g_active_portrait_unit, g_local_nation_id, "Not enough life remaining.");
        }
    }

    // Code used by the AI (no questions about sync between players in that case) -GPTP
    void use_stimpack_ai(Unit* unit) { // 004554A0
        auto& unit_ext = unit->ext();
        if (unit->hitpoints > Terran::StimpackMinimumHP && unit->stim_timer == 0 && unit_ext.timers[TimerStimpackDelay] <= 1) {
            stimpack_proc(unit);
        }
    }

    // Global answer when applying the validated action of stimpack
    // Check the active player selection and apply stimpack effect on each valid stimpack user -GPTP
    void cmdrecv_stimpack() { // 004C2F30
        Unit* unit;
        *g_unit_iterator_pos = 0;
        unit = get_active_player_next_selection();
        while (unit) {
            auto& unit_ext = unit->ext();
            if (unit->can_use_tech(TechId::Stimpacks, *g_active_nation_id) == 1 && unit->hitpoints > Terran::StimpackMinimumHP && unit_ext.timers[TimerStimpackDelay] <= 1) {
                stimpack_proc(unit);
            }
            unit = get_active_player_next_selection();
        }
    }
} // namespace hooks