#include "use_tech.h"
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"
#include "hook_tools.h"
#include "SCBW/stat_txt_spec.h"
#include "hooks/interface/select_larva.h"

namespace hooks
{
	enum LarvaType {
		LarvaType_Swarm,
		LarvaType_Assault,
		LarvaType_Control,
	};

	static void select_larva_type(u32 type) {
		BTSNACT_select_larva();
		helper::update_hud_info();
		if (*active_portrait_unit && (*active_portrait_unit)->id == UnitId::ZergLarva) {
			helper::BTNSACT_ChangeButtons(206 + type);
		}
	}

	void btnsact_use_tech(u16 tech, GrpHead* head) // 00423F70
	{
		// Replacement for proc @ 00423660. -iquare
		bool TechCapableUnit = false;
		Unit* firstInList = players_selections->unit[*ACTIVE_NATION_ID][0];
		if (firstInList != nullptr)
		{
			// Initial check for energy-based switch abilities
			// Not sure if Lanifect needs this anymore
			if (players_selections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::ProtossLanifect
				&& scbw::get_generic_rust_value(firstInList, RustId::ToggleDisruptionField) == 1)
			{
				TechCapableUnit = true;
			}
			if (tech == TechId::EidolonLobotomyMine)
			{
				TechCapableUnit = true;
			}
		}
		if (!TechCapableUnit)
		{
			if (helper::check_energy_for_tech_use(tech))
			{
				TechCapableUnit = true;
			}
			if (firstInList != nullptr)
			{
				/*
				if (players_selections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::TerranAzazel
					&& scbw::get_generic_rust_value(firstInList, RustId::ToggleSublimeShepherd) == 1)
				{
					TechCapableUnit = true;
				}
				*/
			}
			if (!TechCapableUnit)
			{
				char* message = (char*)statTxtTblNew->get_string(864);
				Unit* portrait_unit = players_selections->unit[*ACTIVE_NATION_ID][0];
				helper::get_stat_txt_string(*active_portrait_unit, (*active_portrait_unit)->player, message);
				return;
			}
		}
		switch (tech)
		{
			case TechId::ScannerSweep:
			{
				helper::issue_tri_target_order(OrderId::PlaceScanner, 0, 0);
			} break;
			case TechId::SpiderMines:
			{
				helper::issue_tri_target_order(OrderId::PlaceMine, 0, 0);
			} break;
			case TechId::YamatoGun:
			{
				helper::issue_tri_target_order(OrderId::FireYamatoGun1, OrderId::FireYamatoGun2, 0);
			} break;
			case TechId::Lockdown:
			{
				helper::issue_tri_target_order(OrderId::MagnaPulse, 0, 0);
			} break;
			case TechId::Recall:
			{
				helper::issue_tri_target_order(OrderId::Teleport, 0, 0);
			} break;
			case TechId::DefensiveMatrix:
			{
				helper::issue_tri_target_order(OrderId::DefensiveMatrix, 0, 0);
			} break;
			case TechId::MatralethEnsnaringBrood:
			{
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastEnsnaringBrood;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::Parasite:
			{
				helper::issue_tri_target_order(OrderId::CastParasite, 0, 0);
			} break;
			case TechId::Infestation:
			{
				helper::issue_tri_target_order(OrderId::InfestMine3, OrderId::Move, OrderId::InfestMine2);
			} break;
			case TechId::Irradiate:
			{
				helper::issue_tri_target_order(OrderId::Irradiate, 0, 0);
			} break;
			case TechId::Ensnare:
			{
				helper::issue_tri_target_order(OrderId::CTFCOPInit, 0, 0);
			} break;
			case TechId::StasisField:
			{
				helper::issue_tri_target_order(OrderId::StasisField, 0, 0);
			} break;
			case TechId::DarkSwarm:
			{
				helper::issue_tri_target_order(OrderId::DarkSwarm, 0, 0);
			} break;
			case TechId::Plague:
			{
				helper::issue_tri_target_order(OrderId::Plague, 0, 0);
			} break;
			case TechId::Consume:
			{
				helper::issue_tri_target_order(OrderId::Consume, 0, 0);
			} break;
			case TechId::PsionicStorm:
			{
				helper::issue_tri_target_order(OrderId::PsiStorm, 0, 0);
			} break;
			case TechId::Hallucination:
			{
				helper::issue_tri_target_order(OrderId::Hallucination1, 0, 0);
			} break;
			case TechId::DemiurgeGodsMadeFlesh:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0 };
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastGodsMadeFlesh;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::CancelNewAbility:
			{
				if (players_selections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::TerranWyvern)
				{
					s32 action = 1;
					u8 buf[0xc] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
					buf[0] = 0x99;
					buf[1] = NewGameCommand::GenericSwitchAbility;
					buf[2] = action;
					buf[3] = *ACTIVE_PLAYER_ID;
					buf[4] = RustId::CancelLandGrab;
					helper::queue_game_command(buf, 0xc);
				}
				else if (players_selections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::ZergAkistrokor
					|| players_selections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::ZergSovroleth)
				{
					s32 action = 1;
					u8 buf[0xc] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
					buf[0] = 0x99;
					buf[1] = NewGameCommand::GenericSwitchAbility;
					buf[2] = action;
					buf[3] = *ACTIVE_PLAYER_ID;
					buf[4] = RustId::CancelAttachment;
					helper::queue_game_command(buf, 0xc);
				}
			} break;
			case TechId::ControlledDemolition:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastControlledDemolition;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::EmergencyDetonation:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastEmergencyDetonation;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::StarSovereignTheGriefOfAllGods:
			{
				auto firstInList = client_selection_group->unit[0];
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
				buf[0] = 0x99;
				buf[1] = NewGameCommand::IssueCTFCop2;
				buf[2] = *ACTIVE_PLAYER_ID;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::ApostleLazarusAgent:
			{
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastLazarusAgent;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::SavantOnhitToggle:
			{
				s32 action = 1;
				if (scbw::get_generic_rust_value(firstInList, RustId::ToggleSavantOnhit)) {
					action = 0;
				}
				u8 buf[0xC] = {};
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::ToggleSavantOnhit;
				helper::queue_game_command(buf, 0xC);
			} break;
			case TechId::MadrigalTempoChange:
			{
				s32 action = 1;
				if (scbw::get_generic_rust_value(firstInList, RustId::ToggleTempoChange) == 1)
				{
					action = 0;
				}
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::ToggleTempoChange;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::PaladinSkyfall:
			{
				helper::issue_tri_target_order(OrderId::PaladinSkyfall, 0, 0);
			} break;
			case TechId::AkistrokorBlissfulEmbrace:
			{
				helper::issue_tri_target_order(OrderId::CTFCOP1, 0, 0);
			} break;
			case TechId::DisruptionWeb:
			{
				s32 action = 1;
				if (scbw::get_generic_rust_value(firstInList, RustId::ToggleDisruptionField) == 1)
				{
					action = 0;
				}
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::ToggleDisruptionField;
				helper::queue_game_command(buf, 0xc);
			} break;
			case TechId::MindControl:
			{
				helper::issue_tri_target_order(OrderId::CastMindControl, 0, 0);
			} break;
			case TechId::LuminaryAuralAssault:
			{
				helper::issue_tri_target_order(OrderId::CastOpticalFlare, 0, 0);
			} break;
			case TechId::Maelstrom:
			{
				helper::issue_tri_target_order(OrderId::CastMaelstrom, 0, 0);
			} break;
			case TechId::TalosSalvage:
			{
				helper::issue_tri_target_order(OrderId::TalosSalvage, 0, 0);
			} break;
			case TechId::LarvalSubmenuSelectSwarm:
			{
				select_larva_type(LarvaType_Swarm);
			} break;
			case TechId::LarvalSubmenuSelectAssault:
			{
				select_larva_type(LarvaType_Assault);
			} break;
			case TechId::LarvalSubmenuSelectControl:
			{
				select_larva_type(LarvaType_Control);
			} break;
			default:
			{
			}
		};
	}

	// Hooked to allow Hallucinations to use tech. -Pr0nogo
	u32 tech_use_allowed(Unit* unit, u16 tech_id, u32 player) { // 0046DD80
		*last_internal_error = 0;
		u32 result = 0;
		if (tech_id >= TechId::None) {
			*last_internal_error = 17;
			return 0;
		}
		if (techdata_dat::requirement_offsets[tech_id] == RequirementOpcodes::EndOfSublist) {
			//helper::parse_tech_dat_requirements();
		}
		if (player != unit->player) {
			*last_internal_error = 1;
			return 0;
		}
		if (!(unit->status & UnitStatus::Completed)) {
			*last_internal_error = 20;
			return 0;
		}
		if (unit->is_frozen()) {
			*last_internal_error = 10;
			return 0;
		} 
		if (unit->status & UnitStatus::IsHallucination && tech_id != TechId::Burrowing) {
			*last_internal_error = 22;
			return 0;
		}
		u32 req_offset = techdata_dat::requirement_offsets[tech_id];
		if (!req_offset) {
			*last_internal_error = 23;
			return 0;
		}
		return helper::parse_requirement_opcodes(unit, tech_id, player, (u32)requirements::tech, req_offset);
	}
}