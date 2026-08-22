//Contains hooks that control unit movement speed, acceleration, and turn speed.

#include "unit_speed.h"
#include "../BW/enumerations.h"
#include "../BW/bwdata.h"
#include "hooks/cosmonarchy_finder.h"
#include <BW/UnitFinder.h>
#include "BW/api.h"
#include "globals.h"
#include "interpolation.h"
#include "Utils/captivating_claws.h"

namespace hooks {
    fp32 get_speed_multiplier(Unit* unit) {
        auto& unit_ext = unit->ext();

        fp32 speed{1};
        // Set additives based on status effects
        s32 decrease = (unit->ensnare_timer ? unit_ext.values[ValueEnsnareFactor] : 0);

        if (decrease > 0) {
            for (s32 i = 0; i < decrease; i++) {
                speed *= 0.666fp;
            }
        }

        if (unit->stim_timer) {
            switch (unit->id) {
                case UnitId::TerranMaverick:        speed *= 1.75fp; break;
                case UnitId::TerranMaverickUpstart: speed *= 1.5fp;  break;
                default:                            speed *= 1.75fp; break;
            }
        }

        // Zerg - Adrenal Frenzy - speed
        if (unit_ext.timers[TimerZergAdrenalFrenzy] > 1) {
            speed *= 1.25fp;
        }

        // Protoss Cornerstone: Avenge - speed
        if (unit_ext.timers[TimerAvenge] > 1) {
            speed *= 1.33fp;
        }

        // Lobotomy Mine - slow
        if (unit_ext.timers[TimerLobotomySlow] > 1) {
            speed *= 0.666fp;
        }

        // Scrapworld: Implosion - slow
        if (unit_ext.timers[TimerScrapworldImplosion] > 1) {
            speed *= 0.666fp;
        }

        // Claymore - Kelvin Munitions - slow
        if (unit_ext.timers[TimerKelvinMunitions] > 1) {
            speed *= 0.666fp;
        }

        // Grandstand - Booming Blast - slow
        if (unit_ext.timers[TimerBoomingBlast] > 1) {
            speed *= 0.666fp;
        }

        // Gorgon - Petrification - slow
        if (unit_ext.timers[TimerPetrification]) {
            speed *= 1 - (0.10fp * unit_ext.values[ValuePetrificationStacks]);
        }

        // Vorvrokor - Captivating Claws - reduces movement speed by 25% per unique attacker
        if (captivating_claws_get_timer(unit) > 1) {
            speed *= 1 - 0.25fp * captivating_claws_get_stacks(unit);
        }

        // Sovroleth - Writhing Web - slow more based on proximal Sovroleths
        if (unit->id == UnitId::ZergSovroleth) {
            speed *= pow(0.95fp, unit_ext.values[ValueSovrolethLinkCount]);
        }

        // Keskathalor - Caustic Discharge - speed up
        if (unit_ext.timers[TimerCausticDischargeSpeed] > 1) {
            speed *= 1.5fp;
        }

        // Zoryusthaleth - Swarming Omen - 133% movement speed when moving towards Zoryusthaleth victims
        if (unit_ext.timers[TimerSwarmingOmenSearch] && unit_ext.timers[TimerSwarmingOmenSpeed] < 2) {
            for (Unit* target : g_unit_search.radial_ex(unit, Zoryusthaleth::SwarmingOmenRadius)) {
                UnitExt& target_ext = target->ext();
                if (!target_ext.timers[TimerSwarmingOmenDebuff]) {
                    continue;
                }
                if (!bw::unit_is_facing(unit, target->position)) {
                    continue;
                }
                unit_ext.timers[TimerSwarmingOmenSpeed] = 6;
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
        if (unit_ext.timers[TimerSwarmingOmenSpeed] > 1) {
            speed *= 1.33fp;
        }

        if (unit_ext.timers[TimerImpactDissipation] > 1 || unit_ext.timers[TimerImpactPrevarication] > 1) {
            speed *= 1.5fp;
        }

        if (unit_ext.timers[TimerImpactDissipation] > 1) {
            speed *= 1.33fp;
            bw::add_unit_overlay(unit, ImageId::ImpactDissipationSmall + bw::get_overlay_offset(unit->id, true));
        }

        // Tyranny of the Sun
        if (unit_ext.values[ValueMindControllerUnitId]) {
            auto targets = g_unit_search
                .radial_ex(unit, unit->get_sight_range() * 32, FilterEnemy | FilterVisible);
            for (auto target : targets) {
                speed *= 1.33fp;
                bw::set_unit_overlay(unit, {ImageId::DemiurgeWorshipOverlay_Small, ImageId::DemiurgeWorshipOverlay_XLarge}, true);
                // TODO: make this overlay work when not moving (probably move to game_hooks). -Pr0nogo
                // TODO: might need to remove the overlay externally
                break;
            }
        }

        if (unit_ext.timers[TimerSegmentedExpansion]) {
            speed /= 2;
        }

        if (unit_ext.units[UnitHiveTissue]) {
            speed *= 2;
        }

        if (unit_ext.values[ValueIsMalefactor] == 2) {
            speed *= 1.33fp;
        }

        // Crucible - speed up
        bool has_crucible = unit_ext.unit_vectors[UnitVectorCrucibles].size() != 0;
        speed *= 1 + (has_crucible * 0.25fp);

        // Vassal Terminal Surge: increase movespeed when any nearby Vassal dies
        if (unit_ext.timers[TimerTerminalSurge] > 1) {
            speed *= 1.25fp;
        }

        // Hierophant Signify: Cowardice
        if (unit_ext.values[ValueSignifyStacks]) {
            speed *= 0.5fp;
        }

        // Hijack - slow
        if (unit_ext.timers[TimerIsBeingHijacked] > 1) {
            speed *= 0.5fp;
        }

        // Mind Tyrant - Maelstrom - slow
        if (unit_ext.timers[TimerMindTyrantMaelstromSlow] > 1) {
            speed *= 0.33fp;
        }

        if (unit_ext.values[ValueReclaimed]) {
            speed *= 0.75fp;
        }

        // Infestation - Slow
        if (unit_ext.values[ValueInfested] && bw::get_game_kind() != GameKind::Embraced) {
            speed *= 0.66fp;
        }

        if (unit_ext.timers[TimerGoremastSlow]) {
            speed *= 0.66fp;
        }

        if (unit_ext.timers[TimerCrushingDelivery]) {
            speed *= 0.66fp;
        }

        // Shade - speed steroid
        if (unit_ext.values[ValueIsShade]) {
            speed *= 1.5fp;
        }

        // Vengeful March - fading speed steroid
        if (unit_ext.timers[TimerVengefulMarch]) {
            speed *= utils::map<fp32>(unit_ext.timers[TimerVengefulMarch], Protoss::VengefulMarchDuration, 0, 1.33fp, 1);
        }

        if (unit->id == UnitId::TerranEidolonNydus && !unit_ext.timers[TimerDecloak]) {
            speed *= 1.33fp;
        }

        if (unit_ext.timers[TimerForlornHaste]) {
            speed *= 2;
        }

        switch (unit->id) {
            case UnitId::TerranBlackjack: {
                // Wear and Tear: reduce movement speed based on missing health
                if (unit_ext.values[ValueWearAndTear]) {
                    speed *= 0.66fp;
                } else {
                    speed *= 1.2fp;
                }
            } break;
            case UnitId::TerranBlackjackGolden: {
                // Wear and Tear: reduce movement speed based on missing health
                if (unit_ext.values[ValueWearAndTear]) {
                    speed *= 0.66fp;
                } else {
                    speed *= 1.1fp;
                }
            } break;
            case UnitId::ZergUltrokor: {
                speed *= 1.2fp;
                // Ultrokor Kaiser Rampage: gains 200% movespeed on kill for 1 second
                if (unit_ext.timers[TimerKaiserRampage]) {
                    speed *= 2;
                }
            } break;
            case UnitId::ZergZoltorokor: {
                speed *= 1.25fp;
                if (unit_ext.timers[TimerKaiserRampage]) {
                    speed *= 2;
                }
            } break;

            // Deathless Procession
            case UnitId::ProtossAdherent: {
                speed *= 1.25fp;
                if (unit->shields > 256) {
                    speed *= 1.25fp;
                }
            } break;

            // Hardcode ideal movement speeds
            case UnitId::TerranArhaun:
            case UnitId::TerranRamesses:
            case UnitId::ProtossOptecton: {
                speed *= 0.8fp;
            } break;
            case UnitId::TerranSiren:
            case UnitId::TerranPhalanxTankMode:
            case UnitId::ProtossAccantor:
            case UnitId::ProtossIdolKhalendric:
            case UnitId::ZergBactalisk:
            case UnitId::ZergEvigrilisk: {
                speed *= 0.9fp;
            } break;
            case UnitId::ZergQuazilisk: {
                speed *= 1.05fp;
            } break;
            case UnitId::TerranMason:
            case UnitId::ProtossScribe:
            case UnitId::ZergDroleth:
            case UnitId::AskosiPilgrim:
            case UnitId::BloodsummonAcqui:
            case UnitId::TerranMaverick:
            case UnitId::TerranMaverickUpstart:
            case UnitId::TerranHotshot:
            case UnitId::TerranCleric:
            case UnitId::TerranClericFuturist:
            case UnitId::TerranEidolon:
            case UnitId::TerranApostle:
            case UnitId::TerranCutlass:
            case UnitId::TerranCataphract:
            case UnitId::ProtossDracadin:
            case UnitId::ZergQuazrokor:
            case UnitId::ZergZoryusthaleth:
            case UnitId::AskosiVicar:
            case UnitId::AskosiObject:
            case UnitId::AskosiThaumalogue: {
                speed *= 1.1fp;
            } break;
            case UnitId::TerranCamazotz:
            case UnitId::TerranCyclops:
            case UnitId::TerranDurendal: {
                speed *= 1.15fp;
            } break;
            case UnitId::TerranMaverickFuturist:
            case UnitId::TerranMadcap:
            case UnitId::TerranOlympian:
            case UnitId::TerranSevenstep:
            case UnitId::ProtossCabalist:
            case UnitId::ProtossVestige:
            case UnitId::ProtossSimulacrum:
            case UnitId::ProtossSimulacrumAstral:
            case UnitId::ZergAgrileth:
            case UnitId::ZergLiiralisk:
            case UnitId::ZergIzirokor:
            case UnitId::ZergAlzurokor:
            case UnitId::AskosiMissionary:
            case UnitId::AskosiCardinal:
            case UnitId::AskosiDoronin: {
                speed *= 1.2fp;
            } break;
            case UnitId::TerranSavantEntropyGauntlets:
            case UnitId::TerranHarakan:
            case UnitId::TerranNuriel:
            case UnitId::TerranHeracles:
            case UnitId::TerranMatadorTankMode:
            case UnitId::ProtossGolem:
            case UnitId::ProtossSycophant:
            case UnitId::ZergMalkothrokor:
            case UnitId::ZergZethrokor:
            case UnitId::ZergAxitrilisk:
            case UnitId::AskosiDyad: {
                speed *= 1.25fp;
            } break;
            case UnitId::TerranCyprian:
            case UnitId::ProtossSimulacrumKhalendric:
            case UnitId::ZergYblyleth:
            case UnitId::ZergVorvrokor:
            case UnitId::ZergGarvrokor:
            case UnitId::ZergCikralisk:
            case UnitId::ZergRozirokor:
            case UnitId::ZergInzorthrokor:
            case UnitId::ZergLakizilisk:
            case UnitId::ZergLakiziliskZorthos:
            case UnitId::ZergZobriolisk:
            case UnitId::AskosiProselyte:
            case UnitId::AskosiAvatar:
            case UnitId::AskosiInsurant: {
                speed *= 1.3fp;
            } break;
            case UnitId::TerranHarakanGolden:
            case UnitId::TerranStriga:
            case UnitId::ProtossZealot:
            case UnitId::ProtossCicatus: {
                speed *= 1.35fp;
            } break;
            case UnitId::TerranTiangou:
            case UnitId::ProtossZealotStarbound:
            case UnitId::ProtossAmaranth:
            case UnitId::ZergHydraliskOrkoti: {
                speed *= 1.4fp;
            } break;
            case UnitId::TerranTalos:
            case UnitId::ZergGosvileth: {
                speed *= 1.45fp;
            } break;
            case UnitId::ProtossLegionnaire:
            case UnitId::ProtossVagrantShade:
            case UnitId::ZergAxitriliskWeaponless: {
                speed *= 1.5fp;
            } break;
            case UnitId::ZergMortothrokor: {
                speed *= 1.6fp;
            } break;
            case UnitId::AskosiConstituent: {
                speed *= 1.65fp;
            } break;
            case UnitId::ZergHydralisk:
            case UnitId::ZergHydrokor: {
                speed *= 1.7fp;
            } break;
            case UnitId::TerranCannibal: {
                speed *= 1.75fp;
            } break;

            // Special overrides
            case UnitId::ProtossSteward: {
                speed = 1fp;
            }
        }

        if (speed < 0fp) {
            speed = 0fp;
        }
        if (unit_ext.values[ValueReassemblyActive]) {
            speed = 0fp;
        }
        return speed;
    }

    u32 get_modified_unit_speed(Unit* unit, u32 base_speed) { // 0047B5F0
        if (!unit) {
            return 1;
        }

        s32 increase = unit->status & UnitStatus::SpeedUpgrade ? 1 : 0;
        switch (unit->id) {
            case UnitId::ProtossPanoptus: {
                // Leftover movement speed code
                base_speed = 1707;
            } break;
            default: {
                // Flingy-related code
                if (increase > 0) {
                    for (s32 i = 0; i < increase; i++) {
                        base_speed = static_cast<u32>(base_speed * 1.6fp);
                    }
                    if (base_speed < 853) {
                        base_speed = 853;
                    }
                }
            }
        }
        return static_cast<u32>(base_speed * get_speed_multiplier(unit));
    }

    u32 get_modified_unit_acceleration(Unit* unit) {
        if (unit->id == UnitId::TerranDiadem && unit->status & UnitStatus::InAir) {
            return flingy_dat::acceleration[units_dat::graphic[unit->id]];
        }
        return u32(std::max(flingy_dat::acceleration[units_dat::graphic[unit->id]] * get_speed_multiplier(unit), 1.0fp));
    }

    u32 get_modified_unit_turn_speed(Unit* unit) {
        if (unit->id == UnitId::TerranDiadem) {
            if (unit->status & UnitStatus::InAir) {
                return 27;
            }
        }
        return u32(std::max(flingy_dat::turn_speed[units_dat::graphic[unit->id]] * get_speed_multiplier(unit), 1.0fp));
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
             	helper::iscript_opcode_cases(loop_image, g_temp_iscript_header, 1, (u32)&opcode_value);
                u32 opcode_value_final = opcode_value;

                u32 buff_or_debuff = 0;
                *g_active_iscript_unit = active_unit;
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