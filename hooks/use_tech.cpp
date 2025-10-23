#include "use_tech.h"
#include "BW/bwdata.h"
#include "BW/api.h"
#include "hook_tools.h"
#include "BW/stat_txt_spec.h"
#include "hooks/interface/select_larva.h"
#include <hooks/limits/sfx_extender.h>

namespace hooks {
    static void select_larva_type(u32 buttonset, u32 ocsal_buttonset) {
        Unit* unit = *g_active_portrait_unit;
        if (!unit) {
            return;
        }
        if (unit->id != UnitId::ZergLarva && unit->id != UnitId::ZergOcsalLarva) {
            btnsact_select_larva();
            helper::update_hud_info();

            unit = *g_active_portrait_unit;
            if (!unit) {
                return;
            }
        }

        if (unit->id == UnitId::ZergLarva) {
            helper::BTNSACT_ChangeButtons(buttonset);
        } else if (unit->id == UnitId::ZergOcsalLarva) {
            helper::BTNSACT_ChangeButtons(ocsal_buttonset);
        }
    }

    static void queue_generic_ability(u8 rust_id, u8 value) {
        u8 buffer[12] = {}; // btw
        buffer[0] = 99;
        buffer[1] = NewGameCommand::GenericSwitchAbility;
        buffer[2] = value;
        buffer[3] = *g_active_player_id;
        buffer[4] = rust_id;
        helper::queue_game_command(buffer, sizeof(buffer));
    }

    static void queue_toggle_ability(u8 rust_id, Unit* unit) {
        u8 buffer[12] = {}; // btw
        buffer[0] = 99;
        buffer[1] = NewGameCommand::GenericSwitchAbility;
        buffer[2] = !bw::get_generic_rust_value(unit, rust_id);
        buffer[3] = *g_active_player_id;
        buffer[4] = rust_id;
        helper::queue_game_command(buffer, sizeof(buffer));
    }

    static void queue_generic_ability_simple() {
        queue_generic_ability(0, 0);
    }

    // Replacement for proc @ 00423660. -iquare
    void btnsact_use_tech(u16 tech, GrpHead* head) { // 00423F70
        bool tech_capable_unit = false;
        bool can_play_local_sound = (*g_active_portrait_unit)->player == g_local_nation_id && !g_is_in_replay;
        
        // Only taking the first unit has bad implications for toggles, since we toggle based on the first unit's state, which is not guaranteed
        // to be the same state as all other units in the selection.
        // We should think of a way to fix this! -Pr0nogo
        Unit* unit = players_selections->unit[*g_active_nation_id][0];
        if (unit) {
            // Initial check for energy-based switch abilities
            if (players_selections->unit[*g_active_nation_id][0]->id == UnitId::ProtossLanifect && bw::get_generic_rust_value(unit, RustId::ToggleDisruptionField) == 1) {
                tech_capable_unit = true;
            }
        }
        if (!tech_capable_unit) {
            if (helper::check_energy_for_tech_use(tech)) {
                tech_capable_unit = true;
            }
            if (!tech_capable_unit) {
                char* message = (char*)g_stat_txt_tbl_new->get_string(864);
                Unit* portrait_unit = players_selections->unit[*g_active_nation_id][0];
                helper::get_stat_txt_string(*g_active_portrait_unit, (*g_active_portrait_unit)->player, message);
                return;
            }
        }
        switch (tech) {
            case TechId::ControlledDemolition: {
                queue_generic_ability(RustId::CastControlledDemolition, 1);
            } break;
            case TechId::EmergencyDetonation: {
                queue_generic_ability(RustId::CastEmergencyDetonation, 1);
            } break;
            case TechId::ScannerSweep: {
                helper::issue_tri_target_order(OrderId::PlaceScanner, 0, 0);
            } break;
            case TechId::TalosSalvage: {
                helper::issue_tri_target_order(OrderId::TalosSalvage, 0, 0);
            } break;
            case TechId::ApostleLazarusAgent: {
                if (can_play_local_sound) {
                    u32 action_sound = bw::rand_between_local(SoundId::ApostleToggleLazarus01, SoundId::ApostleToggleLazarus03);
                    bw::play_action_sound(action_sound, *g_active_portrait_unit);
                }
                queue_generic_ability(RustId::ToggleLazarusAgent, 1);
            } break;
            case TechId::HypnagogueThroughTheLookingGlass: {
                helper::issue_tri_target_order(OrderId::CastThroughTheLookingGlass, 0, 0);
            } break;
            case TechId::SpiderMines: {
                helper::issue_tri_target_order(OrderId::PlaceMine, 0, 0);
            } break;
            case TechId::GoskaLobotomyGrenade: {
                helper::issue_tri_target_order(OrderId::Lockdown, 0, 0);
            } break;
            case TechId::MadrigalTempoChange: {
                queue_generic_ability(RustId::CastToggleTempoChange, 1);
            } break;
            case TechId::PaladinSkyfall: {
                helper::issue_tri_target_order(OrderId::PaladinSkyfall, 0, 0);
            } break;
            case TechId::DefensiveMatrix: {
                helper::issue_tri_target_order(OrderId::DefensiveMatrix, 0, 0);
            } break;
            case TechId::Irradiate: {
                helper::issue_tri_target_order(OrderId::Irradiate, 0, 0);
            } break;
            case TechId::YamatoGun: {
                helper::issue_tri_target_order(OrderId::FireYamatoGun1, OrderId::FireYamatoGun2, 0);
            } break;
            case TechId::FacadePryingEyes: {
                if (!unit->is_in_air()) {
                    queue_generic_ability(RustId::CastPryingEyes, 1);
                }
            } break;
            case TechId::Consume: {
                helper::issue_tri_target_order(OrderId::Consume, 0, 0);
            } break;
            case TechId::TargetGround: {
                helper::issue_tri_target_order(OrderId::WyvernLandGrab, 0, 0);
            } break;
            case TechId::MatralethParasite: {
                helper::issue_tri_target_order(OrderId::CastParasite, 0, 0);
            } break;
            case TechId::Infestation: {
                helper::issue_tri_target_order(OrderId::InfestMine3, OrderId::Move, OrderId::InfestMine2);
            } break;
            case TechId::ZergBlissfulEmbrace: {
                helper::issue_tri_target_order(OrderId::BlissfulEmbrace, 0, 0);
            } break;
            case TechId::CarapaceSwarm: {
                helper::issue_tri_target_order(OrderId::CarapaceSwarm, 0, 0);
            } break;
            case TechId::Plague: {
                helper::issue_tri_target_order(OrderId::Plague, 0, 0);
            } break;
            case TechId::PsionicStorm: {
                if (can_play_local_sound) {
                    u32 action_sound = bw::rand_between_local(SoundId::Protoss_TEMPLAR_PTePss01_WAV, SoundId::Protoss_TEMPLAR_PTePss02_WAV);
                    bw::play_action_sound(action_sound, *g_active_portrait_unit);
                }
                helper::issue_tri_target_order(OrderId::PsiStorm, 0, 0);
            } break;
            case TechId::Hallucination: {
                if (can_play_local_sound) {
                    u32 action_sound = SoundId::Protoss_TEMPLAR_PTePss03_WAV;
                    bw::play_action_sound(action_sound, *g_active_portrait_unit);
                }
                helper::issue_tri_target_order(OrderId::Hallucination1, 0, 0);
            } break;
            case TechId::MindTyrantMaledictus: {
                if (can_play_local_sound) {
                    u32 action_sound = bw::rand_between_local(SoundId::Protoss_DARCHON_PDaPss00_WAV, SoundId::Protoss_DARCHON_PDaPss01_WAV);
                    bw::play_action_sound(action_sound, *g_active_portrait_unit);
                }
                helper::issue_tri_target_order(OrderId::CastMaledictus, 0, 0);
            } break;
            case TechId::MindTyrantMaelstrom: {
                helper::issue_tri_target_order(OrderId::CastMaelstrom, 0, 0);
            } break;
            case TechId::LanifectDisruptionField: {
                queue_toggle_ability(RustId::ToggleDisruptionField, unit);
            } break;
            case TechId::DimensionalRecall: {
                if (unit->id == UnitId::ProtossDidact && can_play_local_sound) {
                    bw::play_action_sound(SoundId::Protoss_ARBITER_PAbPss00_WAV, *g_active_portrait_unit);
                }
                helper::issue_tri_target_order(OrderId::Teleport, 0, 0);
            } break;
            case TechId::StasisField: {
                helper::issue_tri_target_order(OrderId::StasisField, 0, 0);
            } break;
            case TechId::StarSovereignTheGriefOfAllGods: {
                u8 buffer[12] = {}; // btw
                buffer[0] = 99;
                buffer[1] = NewGameCommand::IssueCTFCop2;
                buffer[2] = *g_active_player_id;
                helper::queue_game_command(buffer, sizeof(buffer));
            } break;
            case TechId::LuminaryAuralAssault: {
                helper::issue_tri_target_order(OrderId::CastOpticalFlare, 0, 0);
            } break;
            case TechId::LarvalSubmenuSelectSwarm: {
                select_larva_type(UnitId::Buttons_LarvaSwarm, UnitId::Buttons_OcsalLarvaSwarm);
            } break;
            case TechId::LarvalSubmenuSelectAssault: {
                select_larva_type(UnitId::Buttons_LarvaAssault, UnitId::Buttons_OcsalLarvaAssault);
            } break;
            case TechId::LarvalSubmenuSelectControl: {
                select_larva_type(UnitId::Buttons_LarvaControl, UnitId::Buttons_OcsalLarvaControl);
            } break;
            case TechId::StroluumSkryspawn: {
                queue_generic_ability_simple();
            } break;
            case TechId::CancelNewAbility: {
                Unit* selected = *g_active_portrait_unit;
                if (selected) {
                    switch (selected->id) {
                        case UnitId::TerranApostle: {
                            queue_generic_ability(RustId::ToggleLazarusAgent, 0);
                        } break;
                        case UnitId::TerranWyvern: {
                            queue_generic_ability(RustId::CancelLandGrab, 1);
                        } break;
                        case UnitId::ProtossXenolith:
                        case UnitId::ProtossDidact: {
                            queue_generic_ability(RustId::CancelRecall, 1);
                        } break;
                        case UnitId::ProtossCantavis: {
                            queue_generic_ability(RustId::CancelPsiStorm, 1);
                        } break;
                    }
                    if (unit_id_can_do_controlled_demolition(selected->id)) {
                        queue_generic_ability(RustId::CancelControlledDemolition, 1);
                    }
                }
            } break;
        };
    }

    // Hooked to allow Hallucinations to use tech. -Pr0nogo
    s32 tech_use_allowed(Unit* unit, u16 tech_id, u32 player) { // 0046DD80
        *g_last_internal_error = 0;
        u32 result = 0;
        if (tech_id == TechId::None) {
            *g_last_internal_error = 17;
            return 0;
        }
        if (techdata_dat::requirement_offsets[tech_id] == RequirementOpcodes::EndOfSublist) {
            helper::parse_tech_dat_requirements();
        }
        if (player != unit->player) {
            *g_last_internal_error = 1;
            return 0;
        }
        if (!unit->is_completed()) {
            *g_last_internal_error = 20;
            return 0;
        }
        if (unit->is_frozen()) {
            *g_last_internal_error = 10;
            return 0;
        } 
        if (unit->is_hallucination() && tech_id != TechId::Burrowing) {
            *g_last_internal_error = 22;
            return 0;
        }
        u32 req_offset = techdata_dat::requirement_offsets[tech_id];
        if (!req_offset) {
            *g_last_internal_error = 23;
            return 0;
        }
        return helper::parse_requirement_opcodes(unit, tech_id, player, requirements::tech, req_offset);
    }
}