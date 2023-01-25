#include "use_tech.h"
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"
#include "hook_tools.h"
#include "SCBW/stat_txt_spec.h"
#include "hooks/interface/select_larva.h"

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

	void btnsact_use_tech(u16 tech, GrpHead* head) { // 00423F70
		// Replacement for proc @ 00423660. -iquare
		bool TechCapableUnit = false;
		Unit* firstInList = players_selections->unit[*g_active_nation_id][0];
		if (firstInList) {
			// Initial check for energy-based switch abilities
			// Not sure if Lanifect needs this anymore
			if (players_selections->unit[*g_active_nation_id][0]->id == UnitId::ProtossLanifect
				&& scbw::get_generic_rust_value(firstInList, RustId::ToggleDisruptionField) == 1) {
				TechCapableUnit = true;
			}
			if (tech == TechId::EidolonLobotomyMine) {
				TechCapableUnit = true;
			}
		}
		if (!TechCapableUnit) {
			if (helper::check_energy_for_tech_use(tech)) {
				TechCapableUnit = true;
			}
			if (firstInList) {
				/*
				if (players_selections->unit[*g_active_nation_id][0]->id == UnitId::TerranAzazel
					&& scbw::get_generic_rust_value(firstInList, RustId::ToggleSublimeShepherd) == 1)
				{
					TechCapableUnit = true;
				}
				*/
			}
			if (!TechCapableUnit)
			{
				char* message = (char*)g_stat_txt_tbl_new->get_string(864);
				Unit* portrait_unit = players_selections->unit[*g_active_nation_id][0];
				helper::get_stat_txt_string(*g_active_portrait_unit, (*g_active_portrait_unit)->player, message);
				return;
			}
		}
		switch (tech) {
			case TechId::ControlledDemolition:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *g_active_player_id;
				buf[4] = RustId::CastControlledDemolition;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::EmergencyDetonation:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *g_active_player_id;
				buf[4] = RustId::CastEmergencyDetonation;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::ScannerSweep:
			{
				helper::issue_tri_target_order(OrderId::PlaceScanner, 0, 0);
			} break;
			case TechId::TalosSalvage:
			{
				helper::issue_tri_target_order(OrderId::TalosSalvage, 0, 0);
			} break;
			case TechId::Lockdown:
			{
				helper::issue_tri_target_order(OrderId::MagnaPulse, 0, 0);
			} break;
			case TechId::SavantOnhitToggle:
			{
				s32 action = 1;
				if (scbw::get_generic_rust_value(firstInList, RustId::ToggleSavantOnhit)) {
					action = 0;
				}
				u8 buf[0xC] = {};
				buf[0] = 99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *g_active_player_id;
				buf[4] = RustId::ToggleSavantOnhit;
				helper::queue_game_command(buf, 0xC);
			} break;
			case TechId::ApostleLazarusAgent:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *g_active_player_id;
				buf[4] = RustId::CastLazarusAgent;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::HypnagogueThroughTheLookingGlass:
			{
				scbw::print("issue order");
				helper::issue_tri_target_order(OrderId::CastThroughTheLookingGlass, 0, 0);
			} break;
			case TechId::SpiderMines:
			{
				helper::issue_tri_target_order(OrderId::PlaceMine, 0, 0);
			} break;
			case TechId::MadrigalTempoChange:
			{
				s32 action = 1;
				if (scbw::get_generic_rust_value(firstInList, RustId::ToggleTempoChange) == 1)
				{
					action = 0;
				}
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *g_active_player_id;
				buf[4] = RustId::ToggleTempoChange;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::PaladinSkyfall:
			{
				helper::issue_tri_target_order(OrderId::PaladinSkyfall, 0, 0);
			} break;
			case TechId::DefensiveMatrix:
			{
				helper::issue_tri_target_order(OrderId::DefensiveMatrix, 0, 0);
			} break;
			case TechId::Irradiate:
			{
				helper::issue_tri_target_order(OrderId::Irradiate, 0, 0);
			} break;
			case TechId::YamatoGun:
			{
				helper::issue_tri_target_order(OrderId::FireYamatoGun1, OrderId::FireYamatoGun2, 0);
			} break;
			case TechId::Consume:
			{
				helper::issue_tri_target_order(OrderId::Consume, 0, 0);
			} break;
			case TechId::MatralethEnsnaringBrood:
			{
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
				buf[0] = 99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *g_active_player_id;
				buf[4] = RustId::CastEnsnaringBrood;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::MatralethParasite:
			{
				helper::issue_tri_target_order(OrderId::CastParasite, 0, 0);
			} break;
			case TechId::Ensnare: // TODO: Remove? -Pr0nogo
			{
				helper::issue_tri_target_order(OrderId::CTFCOPInit, 0, 0);
			} break;
			case TechId::Infestation:
			{
				helper::issue_tri_target_order(OrderId::InfestMine3, OrderId::Move, OrderId::InfestMine2);
			} break;
			case TechId::AkistrokorBlissfulEmbrace:
			{
				helper::issue_tri_target_order(OrderId::CTFCOP1, 0, 0);
			} break;
			case TechId::DarkSwarm:
			{
				helper::issue_tri_target_order(OrderId::DarkSwarm, 0, 0);
			} break;
			case TechId::Plague:
			{
				helper::issue_tri_target_order(OrderId::Plague, 0, 0);
			} break;
			case TechId::PsionicStorm:
			{
				helper::issue_tri_target_order(OrderId::PsiStorm, 0, 0);
			} break;
			case TechId::Hallucination:
			{
				helper::issue_tri_target_order(OrderId::Hallucination1, 0, 0);
			} break;
			case TechId::MindControl:
			{
				helper::issue_tri_target_order(OrderId::CastMindControl, 0, 0);
			} break;
			case TechId::MindTyrantMaelstrom:
			{
				helper::issue_tri_target_order(OrderId::CastMaelstrom, 0, 0);
			} break;
			case TechId::DemiurgeGodsMadeFlesh:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0 };
				buf[0] = 99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *g_active_player_id;
				buf[4] = RustId::CastGodsMadeFlesh;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::LanifectDisruptionField:
			{
				s32 action = 1;
				if (scbw::get_generic_rust_value(firstInList, RustId::ToggleDisruptionField) == 1)
				{
					action = 0;
				}
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *g_active_player_id;
				buf[4] = RustId::ToggleDisruptionField;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::Recall:
			{
				helper::issue_tri_target_order(OrderId::Teleport, 0, 0);
			} break;
			case TechId::StasisField:
			{
				helper::issue_tri_target_order(OrderId::StasisField, 0, 0);
			} break;
			case TechId::StarSovereignTheGriefOfAllGods:
			{
				auto firstInList = g_client_selection_group->unit[0];
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 99;
				buf[1] = NewGameCommand::IssueCTFCop2;
				buf[2] = *g_active_player_id;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::LuminaryAuralAssault:
			{
				helper::issue_tri_target_order(OrderId::CastOpticalFlare, 0, 0);
			} break;
			case TechId::LarvalSubmenuSelectSwarm:
			{
				select_larva_type(UnitId::Buttons_LarvaSwarm, UnitId::Buttons_OcsalLarvaSwarm);
			} break;
			case TechId::LarvalSubmenuSelectAssault:
			{
				select_larva_type(UnitId::Buttons_LarvaAssault, UnitId::Buttons_OcsalLarvaAssault);
			} break;
			case TechId::LarvalSubmenuSelectControl:
			{
				select_larva_type(UnitId::Buttons_LarvaControl, UnitId::Buttons_OcsalLarvaControl);
			} break;
		//	case TechId::LarvalSubmenuSelectShared:
		//	{
		//		select_larva_type(UnitId::Buttons_LarvaShared);
		//	} break;
			case TechId::CancelNewAbility:
			{
				Unit* selected = players_selections->unit[*g_active_nation_id][0];
				if (selected)
				{
					switch (selected->id)
					{
						case UnitId::TerranWyvern:
						{
							s32 action = 1;
							u8 buf[0xc] = { 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0 };
							buf[0] = 99;
							buf[1] = NewGameCommand::GenericSwitchAbility;
							buf[2] = action;
							buf[3] = *g_active_player_id;
							buf[4] = RustId::CancelLandGrab;
							helper::queue_game_command(buf, 0xc);
						} break;
					}
					UnitExt* selected_ext = get_unit_ext(selected);
					if (selected_ext->timers[TimerControlledDemolition])
					{
						selected_ext->timers[TimerControlledDemolition] = 0;
						Image* overlay = selected->get_overlay(ImageId::ControlledDemolition);
						if (overlay)
						{
							overlay->play_iscript_animation(IscriptAnimation::Death);
						}
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