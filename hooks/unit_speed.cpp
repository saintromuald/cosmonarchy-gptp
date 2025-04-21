//Contains hooks that control unit movement speed, acceleration, and turn speed.

#include "unit_speed.h"
#include "../BW/enumerations.h"
#include "../BW/bwdata.h"
#include "hooks/cosmonarchy_finder.h"
#include <BW/UnitFinder.h>
#include "BW/api.h"
#include "globals.h"
#include "math_utils.h"
#include "Utils/captivating_claws.h"

namespace hooks {
    f64 get_speed_multiplier(Unit* unit) {
        UnitExt& unit_ext = unit->ext();

        f64 speed = 1;
        // Set additives based on status effects
        s32 decrease = (unit->ensnare_timer ? unit_ext.values[ValueEnsnareFactor] : 0);

        if (decrease > 0) {
            for (s32 i = 0; i < decrease; i++) {
                speed *= 0.666;
            }
        }

        // Zerg - Adrenal Frenzy - speed
        if (unit_ext.timers[TimerZergAdrenalFrenzy] > 1) {
            speed *= 1.25;
        }

        // Protoss Cornerstone: Avenge - speed
        if (unit_ext.timers[TimerAvenge] > 1) {
            speed *= 1.33;
        }

        // Lobotomy Mine - slow
        if (unit_ext.timers[TimerLobotomySlow] > 1) {
            speed *= 0.666;
        }

        // Claymore - Kelvin Munitions - slow
        if (unit_ext.timers[TimerKelvinMunitions] > 1) {
            speed *= 0.666;
        }

        // Gorgon - Petrification - slow
        if (unit_ext.timers[TimerPetrification]) {
            speed *= 1 - (0.10 * unit_ext.values[ValuePetrificationStacks]);
        }

        // Vorvrokor - Captivating Claws - reduces movement speed by 25% per unique attacker
        if (captivating_claws_get_timer(unit) > 1) {
            speed *= 1 - 0.25 * captivating_claws_get_stacks(unit);
        }

        // Sovroleth - Writhing Web - slow more based on proximal Sovroleths
        if (unit->id == UnitId::ZergSovroleth) {
            speed *= pow(0.95, unit_ext.values[ValueSovrolethLinkCount]);
        }

        // Keskathalor - Caustic Discharge - speed up
        if (unit_ext.timers[TimerCausticDischargeSpeed] > 1) {
            speed *= 1.5;
        }

        // Zoryusthaleth - Swarming Omen - 133% movement speed when moving towards Zoryusthaleth victims
        if (unit_ext.timers[TimerSwarmingOmenSearch]) {
            for (Unit* target : g_unit_search.radial_ex(unit, Zoryusthaleth::SwarmingOmenRadius)) {
                UnitExt& target_ext = target->ext();
                if (!target_ext.timers[TimerSwarmingOmenDebuff]) {
                    continue;
                }
                if (bw::is_allied_to(target->player, unit->player)) {
                    continue;
                }
                if (!bw::unit_is_facing(unit, target->position)) {
                    continue;
                }
                speed *= 1.33;
                if (unit_ext.try_reset_timer(TimerSwarmingOmenVisuals, 12)) {
                    bw::add_unit_overlay(unit, ImageId::SwarmingOmenSmall + bw::get_overlay_offset(unit->id, true));
                }
                if (unit_ext.try_reset_timer(TimerSwarmingOmenAudio, 36)) {
                    u32 sound_id = bw::rand_between(SoundId::SwarmingOmenProc01, SoundId::SwarmingOmenProc02);
                    helper::play_sound_from_unit(sound_id, unit, 1, 0);
                }
                break;
            }
        }

        // Tyranny of the Sun
        if (unit_ext.values[ValueMindControllerUnitId]) {
            auto targets = g_unit_search
                .radial_ex(unit, unit->get_sight_range() * 32, FilterEnemy | FilterVisible);
            for (auto target : targets) {
                speed *= 1.33;
                bw::set_unit_overlay(unit, {ImageId::DemiurgeWorshipOverlay_Small, ImageId::DemiurgeWorshipOverlay_XLarge}, true);
                // TODO: make this overlay work when not moving (probably move to game_hooks). -Pr0nogo
                // TODO: might need to remove the overlay externally
                break;
            }
        }

        if (unit_ext.values[ValueIsMalefactor] == 2) {
            speed *= 1.33;
        }

        // Crucible - speed up
        bool has_crucible = unit_ext.unit_vectors[UnitVectorCrucibles].size() != 0;
        speed *= 1 + (has_crucible * 0.25);

        // Vassal Terminal Surge: increase movespeed when any nearby Vassal dies
        if (unit_ext.timers[TimerTerminalSurge] > 1) {
            speed *= 1.25;
        }

        // Hierophant Signify: Cowardice
        if (unit_ext.values[ValueSignifyStacks]) {
            speed *= 0.5;
        }

        // Mind Tyrant - Maelstrom - slow
        if (unit_ext.timers[TimerMindTyrantMaelstromSlow] > 1) {
            speed *= 0.5;
        }

        // Infestation - Slow
        if (unit_ext.values[ValueReclaimed] || unit_ext.values[ValueInfested]) {
            speed *= 0.75;
        }

        // Shade - speed steroid
        if (unit_ext.values[ValueIsShade]) {
            speed *= 1.5;
        }

        // Vengeful March - fading speed steroid
        if (unit_ext.timers[TimerVengefulMarch]) {
            speed *= MathUtils::Map(unit_ext.timers[TimerVengefulMarch], Protoss::VengefulMarchDuration, 0, 1.33, 1);
        }

        if (unit->id == UnitId::TerranEidolonNydus && bw::is_proximity_decloakable(unit)) {
            speed *= 1.33;
        }

        switch (unit->id) {
            case UnitId::TerranBlackjack:
            case UnitId::TerranBlackjackGolden: {
                // Blackjack Wear and Tear: reduce movement speed based on missing health
                if (unit_ext.values[ValueWearAndTear]) {
                    speed *= 0.66;
                } else {
                    speed *= 1.1;
                }
            } break;
            case UnitId::ZergUltrokor: {
                // Ultrakor Kaiser Rampage: gains 200% movespeed on kill for 1 second
                speed *= 1.2;
                if (unit_ext.timers[TimerKaiserRampage]) {
                    speed *= 2;
                }
            } break;

            // Deathless Procession
            case UnitId::ProtossAdherent: {
                speed *= 1.25;
                if (unit->shields > 256) {
                    speed *= 1.25;
                }
            } break;

            // Hardcode ideal movement speeds
            case UnitId::TerranSavantPowerSiphon:
            case UnitId::TerranRamesses:
            case UnitId::ProtossOptecton: {
                speed *= 0.8;
            } break;
            case UnitId::TerranSiren:
            case UnitId::TerranPhalanxTankMode:
            case UnitId::ProtossAccantor:
            case UnitId::ZergEvigrilisk: {
                speed *= 0.9;
            } break;
            case UnitId::TerranMason:
            case UnitId::ProtossScribe:
            case UnitId::ZergDroleth:
            case UnitId::TerranMaverick:
            case UnitId::TerranHotshot:
            case UnitId::TerranCleric:
            case UnitId::TerranClericFuturist:
            case UnitId::TerranEidolon:
            case UnitId::TerranMadcap:
            case UnitId::TerranApostle:
            case UnitId::TerranOlympian:
            case UnitId::TerranCataphract:
            case UnitId::ProtossDracadin:
            case UnitId::ZergQuazrokor:
            case UnitId::ZergZoryusthaleth: {
                speed *= 1.1;
            } break;
            case UnitId::TerranCyclops:
            case UnitId::TerranDurendal: {
                speed *= 1.15;
            } break;
            case UnitId::TerranMaverickFuturist:
            case UnitId::TerranCyprian:
            case UnitId::TerranSevenstep:
            case UnitId::ProtossAtreus:
            case UnitId::ProtossCabalist:
            case UnitId::ProtossSimulacrum:
            case UnitId::ZergLiiralisk:
            case UnitId::ZergIzirokor:
            case UnitId::ZergKagralisk: {
                speed *= 1.2;
            } break;
            case UnitId::TerranSavantEntropyGauntlets:
            case UnitId::TerranHarakan:
            case UnitId::TerranHeracles:
            case UnitId::TerranMatadorTankMode:
            case UnitId::ProtossGolem:
            case UnitId::ProtossSycophant:
            case UnitId::ZergZethrokor:
            case UnitId::ZergAxitrilisk: {
                speed *= 1.25;
            } break;
            case UnitId::ZergYblyleth:
            case UnitId::ZergInzorthrokor:
            case UnitId::ZergVorvrokor:
            case UnitId::ZergZobriolisk: {
                speed *= 1.3;
            } break;
            case UnitId::TerranHarakanGolden:
            case UnitId::TerranStriga:
            case UnitId::ProtossZealot:
            case UnitId::ProtossCicatus: {
                speed *= 1.35;
            } break;
            case UnitId::TerranTiangou:
            case UnitId::ProtossZealotStarbound:
            case UnitId::ProtossAmaranth:
            case UnitId::ZergHydraliskOrkoti: {
                speed *= 1.4;
            } break;
            case UnitId::ProtossLegionnaire:
            case UnitId::ProtossVagrantShade:
            case UnitId::ZergAxitriliskWeaponless: {
                speed *= 1.5;
            } break;
            case UnitId::ZergMortothrokor:
            case UnitId::ZergHydralisk:
            case UnitId::ZergLakizilisk: {
                speed *= 1.6;
            } break;

            // Special overrides
            case UnitId::ProtossSteward: {
                speed = 1;
            }
        }

        if (speed < 0) {
            speed = 0;
        }
        return speed;
    }

    u32 get_modified_unit_speed(Unit* unit, u32 base_speed) { // 0047B5F0
        if (!unit) {
            return 1;
        }

        s32 increase = (unit->stim_timer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
        switch (unit->id) {
            case UnitId::ProtossPanoptus: {
                // Leftover movement speed code
                base_speed = 1707;
            } break;
            default: {
                // Flingy-related code
                if (increase > 0) {
                    for (s32 i = 0; i < increase; i++) {
                        base_speed *= 1.6;
                    }
                    if (base_speed < 853) {
                        base_speed = 853;
                    }
                }
            }
        }
        return base_speed * get_speed_multiplier(unit);
    }

    u32 get_modified_unit_acceleration(Unit* unit) {
        if (unit->id == UnitId::TerranDiadem && unit->status & UnitStatus::InAir) {
            return flingy_dat::acceleration[units_dat::graphic[unit->id]];
        }
        return std::max(flingy_dat::acceleration[units_dat::graphic[unit->id]] * get_speed_multiplier(unit), 1.0);
    }

    u32 get_modified_unit_turn_speed(Unit* unit) {
        if (unit->id == UnitId::TerranDiadem) {
            if (unit->status & UnitStatus::InAir) {
                return 27;
            }
        }
        return std::max(flingy_dat::turn_speed[units_dat::graphic[unit->id]] * get_speed_multiplier(unit), 1.0);
    }

    // Hooked to investigate crashes. -Pr0nogo
    Unit* set_unit_movement_speed(Unit* unit, u32 speed) { // 004951C0
        /*
           Crash occurs on the very first ASM instruction because "unit" is null. Appears exceedingly rarely.
           Call stack from fagguto:
                0049794A @ sprite_to_iscript_loop (hooked in update_unit_state)
                0048C405 @ progress_bullet_frames (hooked but disabled in weapon_fire)
                012C0B93 @ some plugin or windows thing?
                004D9908 @ game_loop_top (not hooked)
                004D9A6F @ begin_game (not hooked)
                004E07FA @ preload_game (not hooked)
                004E0B20 @ win_main (cannot be hooked?)
                00404DA5 @ start (not hooked)
                761C344D 0018FF94 0001:0000344D C:\Windows\syswow64\kernel32.dll
                77999802 0018FFD4 0001:00029802 C:\Windows\SysWOW64\ntdll.dll
                779997D5 0018FFEC 0001:000297D5 C:\Windows\SysWOW64\ntdll.dll
        */
        if (unit) { // Nullcheck is new.
            unit->current_speed_2 = speed;
            if (unit->current_speed_1 != speed) {
                u32 direction = unit->current_direction_2;
                unit->current_speed_1 = speed;
                unit->current_velocity.x = (s32)(speed * g_angle_distance[direction].x) >> 8;
                unit->current_velocity.y = (s32)(speed * g_angle_distance[direction].y) >> 8;
            }
            return unit;
        }
    }

    // Hooked to fix the speed tooltip for iscript units. -Pr0nogo
    void update_iscript_speed(Unit* unit) { // 0042D8C0
        if (unit->flingy_movement_type != MovementType::MovementType_Iscript) {
            return;
        }
        Image* image = unit->sprite->main_graphic;
        *g_temp_iscript_header = image->iscript_header_offset;
        *g_iscript_speed_something = image->return_address;
        if (helper::set_temp_iscript_animation(IscriptAnimation::Walking)) {
            Unit* active_unit = *g_active_iscript_unit;
            u32 iteration_factor = 0;
            u32 opcode_value = 0;
            u32 tile_size = 32;
            bool tile_size_bool = 0;
            u32 base_speed = 0;
            do {
                Image* loop_image = unit->sprite->main_graphic;
                *g_active_iscript_unit = unit;
            //	helper::iscript_opcode_cases(loop_image, *g_temp_iscript_header, 1, (u32)&opcode_value); // This is probably wrong, compare to IDA (our helper has 1 less arg)
                u8 stim_timer = unit->stim_timer;
                u32 opcode_value_final = opcode_value;

                u32 buff_or_debuff = 0;
                *g_active_iscript_unit = active_unit;
                if (stim_timer) {
                    buff_or_debuff = 1;
                }
                if (unit->status & UnitStatus::SpeedUpgrade) {
                    ++buff_or_debuff;
                }
                if (unit->ensnare_timer) {
                    --buff_or_debuff;
                }

                if (buff_or_debuff <= 0) {
                    if (buff_or_debuff < 0) {
                        opcode_value_final = opcode_value - (opcode_value >> 2);
                    }
                } else {
                    opcode_value_final = 2 * opcode_value;
                }
                base_speed = opcode_value_final + iteration_factor;
                tile_size_bool = tile_size == 1;
                iteration_factor += opcode_value_final;
                opcode_value = 0;
                --tile_size;
            } while (!tile_size_bool);
            unit->flingy_top_speed = base_speed / 32;
        }
    }

    // Hooked to investigate melee edge unit performance. -Pr0nogo
    s32 get_flingy_halt_distance(Unit* unit) { // 00494F90
        s32 result = 0;
        s32 speed = unit->current_speed_2;
        if (!speed || unit->flingy_movement_type) {
            return result;
        }
        u32 flingy_id = unit->flingy_id;
        if (speed != flingy_dat::top_speed[flingy_id] || unit->flingy_acceleration != flingy_dat::acceleration[flingy_id]) {
            result = speed * speed / (2 * unit->flingy_acceleration);
        } else {
            result = flingy_dat::halt_distance[flingy_id] >> 2;
        }
        return result;
    }
}