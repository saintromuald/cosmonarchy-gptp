#include "use_tech.h"
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"
#include "hook_tools.h"
#include "SCBW/stat_txt_spec.h"
#include "hooks/interface/select_larva.h"
#include <hooks/limits/sfx_extender.h>

namespace hooks
{
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
		buffer[2] = !scbw::get_generic_rust_value(unit, rust_id);
		buffer[3] = *g_active_player_id;
		buffer[4] = rust_id;
		helper::queue_game_command(buffer, sizeof(buffer));
	}

	void btnsact_use_tech(u16 tech, GrpHead* head) { // 00423F70
		// Replacement for proc @ 00423660. -iquare
		bool tech_capable_unit = false;
		Unit* unit = players_selections->unit[*g_active_nation_id][0];
		if (unit) {
			// Initial check for energy-based switch abilities
			if (players_selections->unit[*g_active_nation_id][0]->id == UnitId::ProtossLanifect
			&&  scbw::get_generic_rust_value(unit, RustId::ToggleDisruptionField) == 1)
			{
				tech_capable_unit = true;
			}
			if (tech == TechId::UnusedLobotomyMine) {
				tech_capable_unit = true;
			}
		}
		if (!tech_capable_unit) {
			if (helper::check_energy_for_tech_use(tech)) {
				tech_capable_unit = true;
			}
			if (unit) {
				/*
				if (players_selections->unit[*g_active_nation_id][0]->id == UnitId::TerranAzazel
					&& scbw::get_generic_rust_value(first_in_list, RustId::ToggleSublimeShepherd) == 1)
				{
					tech_capable_unit = true;
				}
				*/
			}
			if (!tech_capable_unit)
			{
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
			case TechId::Lockdown: {
				helper::issue_tri_target_order(OrderId::MagnaPulse, 0, 0);
			} break;
			case TechId::SavantOnhitToggle: {
				s32 action = !scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit);
				queue_generic_ability(RustId::ToggleSavantOnhit, action);

				u32 action_sound = action ? helper::get_random_sfx_between(SoundId::SavantSwitchPierce01, SoundId::SavantSwitchPierce03) : helper::get_random_sfx_between(SoundId::SavantSwitchSteal01, SoundId::SavantSwitchSteal03);
				helper::play_sound_from_unit(action_sound, *g_active_portrait_unit, g_options_list->flags & 1, 99);
				helper::do_unit_event_notification(*g_active_portrait_unit, 17, sfx_dat::portrait_duration[action_sound] + 1000 * (sfx_dat::sound_data[action_sound].duration_ms / 1000), 0, action_sound);
			} break;
			case TechId::ApostleLazarusAgent: {
				queue_toggle_ability(RustId::ToggleLazarusAgent, unit);
			} break;
			case TechId::HypnagogueThroughTheLookingGlass: {
				scbw::print("select target");
				helper::issue_tri_target_order(OrderId::CastThroughTheLookingGlass, 0, 0);
			} break;
			case TechId::SpiderMines: {
				helper::issue_tri_target_order(OrderId::PlaceMine, 0, 0);
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
			case TechId::Consume: {
				helper::issue_tri_target_order(OrderId::Consume, 0, 0);
			} break;
			case TechId::MatralethParasite: {
				helper::issue_tri_target_order(OrderId::CastParasite, 0, 0);
			} break;
			case TechId::Ensnare: { // TODO: Remove? -Pr0nogo
				helper::issue_tri_target_order(OrderId::CTFCOPInit, 0, 0);
			} break;
			case TechId::Infestation: {
				helper::issue_tri_target_order(OrderId::InfestMine3, OrderId::Move, OrderId::InfestMine2);
			} break;
			case TechId::AkistrokorBlissfulEmbrace: {
				helper::issue_tri_target_order(OrderId::CTFCOP1, 0, 0);
			} break;
			case TechId::CarapaceSwarm: {
				helper::issue_tri_target_order(OrderId::CarapaceSwarm, 0, 0);
			} break;
			case TechId::Plague: {
				helper::issue_tri_target_order(OrderId::Plague, 0, 0);
			} break;
			case TechId::PsionicStorm: {
				helper::issue_tri_target_order(OrderId::PsiStorm, 0, 0);
			} break;
			case TechId::Hallucination: {
				helper::issue_tri_target_order(OrderId::Hallucination1, 0, 0);
			} break;
			case TechId::Tyranny: {
				helper::issue_tri_target_order(OrderId::CastTyranny, 0, 0);
			} break;
			case TechId::MindTyrantMaelstrom: {
				helper::issue_tri_target_order(OrderId::CastMaelstrom, 0, 0);
			} break;
			case TechId::LanifectDisruptionField: {
				queue_toggle_ability(RustId::ToggleDisruptionField, unit);
			} break;
			case TechId::Recall: {
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
		//	case TechId::LarvalSubmenuSelectShared: {
		//		select_larva_type(UnitId::Buttons_LarvaShared);
		//	} break;
			case TechId::CancelNewAbility: {
				Unit* selected = *g_active_portrait_unit;
				if (selected) {
					switch (selected->id) {
						case UnitId::TerranWyvern: {
							queue_generic_ability(RustId::CancelLandGrab, 1);
						} break;
						case UnitId::ProtossDidact: {
							queue_generic_ability(RustId::CancelRecall, 1);
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
		if (!(unit->status & UnitStatus::Completed)) {
			*g_last_internal_error = 20;
			return 0;
		}
		if (unit->is_frozen()) {
			*g_last_internal_error = 10;
			return 0;
		} 
		if (unit->status & UnitStatus::IsHallucination && tech_id != TechId::Burrowing) {
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