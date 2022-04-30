#include "use_tech.h"
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"
#include "hook_tools.h"
#include "SCBW/stat_txt_spec.h"

namespace hooks
{
	void BTNSACT_UseTech(u16 tech, GrpHead* head)
	{
		// replacement for proc 00423660
		bool TechCapableUnit = false;
		Unit* firstInList = playersSelections->unit[*ACTIVE_NATION_ID][0];
		if (firstInList != nullptr)
		{
			// Initial check for energy-based switch abilities
			// Not sure if Lanifect needs this anymore
			if (playersSelections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::ProtossLanifect
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
			if (helper::checkEnergyForTechUse(tech))
			{
				TechCapableUnit = true;
			}
			if (firstInList != nullptr)
			{
				/*
				if (playersSelections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::TerranAzazel
					&& scbw::get_generic_rust_value(firstInList, RustId::ToggleSublimeShepherd) == 1)
				{
					TechCapableUnit = true;
				}
				*/
			}
			if (!TechCapableUnit)
			{
				char* message = (char*)statTxtTblNew->getString(864);
				Unit* portrait_unit = playersSelections->unit[*ACTIVE_NATION_ID][0];
				helper::get_statTxt_Str_0(*activePortraitUnit, (*activePortraitUnit)->playerId, message);
				return;
			}
		}
		switch (tech)
		{
			case TechId::ScannerSweep:
			{
				helper::issueTriTargetOrder(OrderId::PlaceScanner, 0, 0);
			} break;
			case TechId::SpiderMines:
			{
				helper::issueTriTargetOrder(OrderId::PlaceMine, 0, 0);
			} break;
			case TechId::YamatoGun:
			{
				helper::issueTriTargetOrder(OrderId::FireYamatoGun1, OrderId::FireYamatoGun2, 0);
			} break;
			case TechId::Lockdown:
			{
				helper::issueTriTargetOrder(OrderId::MagnaPulse, 0, 0);
			} break;
			case TechId::Recall:
			{
				helper::issueTriTargetOrder(OrderId::Teleport, 0, 0);
			} break;
			case TechId::DefensiveMatrix:
			{
				helper::issueTriTargetOrder(OrderId::DefensiveMatrix, 0, 0);
			} break;
			case TechId::MatralethEnsnaringBrood:
			{
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastEnsnaringBrood;
				helper::QueueGameCommand(buf, 0xc);
			} break;
			case TechId::Parasite:
			{
				helper::issueTriTargetOrder(OrderId::CastParasite, 0, 0);
			} break;
			case TechId::Infestation:
			{
				helper::issueTriTargetOrder(OrderId::InfestMine3, OrderId::Move, OrderId::InfestMine2);
			} break;
			case TechId::Irradiate:
			{
				helper::issueTriTargetOrder(OrderId::Irradiate, 0, 0);
			} break;
			case TechId::Ensnare:
			{
				helper::issueTriTargetOrder(OrderId::CTFCOPInit, 0, 0);
			} break;
			case TechId::StasisField:
			{
				helper::issueTriTargetOrder(OrderId::StasisField, 0, 0);
			} break;
			case TechId::DarkSwarm:
			{
				helper::issueTriTargetOrder(OrderId::DarkSwarm, 0, 0);
			} break;
			case TechId::Plague:
			{
				helper::issueTriTargetOrder(OrderId::Plague, 0, 0);
			} break;
			case TechId::Consume:
			{
				helper::issueTriTargetOrder(OrderId::Consume, 0, 0);
			} break;
			case TechId::PsionicStorm:
			{
				helper::issueTriTargetOrder(OrderId::PsiStorm, 0, 0);
			} break;
			case TechId::Hallucination:
			{
				helper::issueTriTargetOrder(OrderId::Hallucination1, 0, 0);
			} break;
			case TechId::DemiurgeGodsMadeFlesh:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0 };
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastGodsMadeFlesh;
				helper::QueueGameCommand(buf, 0xc);
			} break;
			case TechId::SavantOnhitToggle:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0 };
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastSavantOnhitToggle;
				helper::QueueGameCommand(buf, 0xc);
			} break;
			case TechId::CancelNewAbility:
			{
				if (playersSelections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::ProtossMindTyrant)
				{
					s32 action = 1;
					u8 buf[0xc] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
					buf[0] = 0x99;
					buf[1] = NewGameCommand::GenericSwitchAbility;
					buf[2] = action;
					buf[3] = *ACTIVE_PLAYER_ID;
					buf[4] = RustId::CancelMindControl;
					helper::QueueGameCommand(buf, 0xc);
				}
				else if (playersSelections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::TerranWyvern)
				{
					s32 action = 1;
					u8 buf[0xc] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
					buf[0] = 0x99;
					buf[1] = NewGameCommand::GenericSwitchAbility;
					buf[2] = action;
					buf[3] = *ACTIVE_PLAYER_ID;
					buf[4] = RustId::CancelLandGrab;
					helper::QueueGameCommand(buf, 0xc);
				}
				else if (playersSelections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::ZergAkistrokor
					|| playersSelections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::ZergSovroleth)
				{
					s32 action = 1;
					u8 buf[0xc] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
					buf[0] = 0x99;
					buf[1] = NewGameCommand::GenericSwitchAbility;
					buf[2] = action;
					buf[3] = *ACTIVE_PLAYER_ID;
					buf[4] = RustId::CancelAttachment;
					helper::QueueGameCommand(buf, 0xc);
				}
			} break;
			case TechId::AnticthonFailureMode:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastFailureMode;
				helper::QueueGameCommand(buf, 0xc);
			} break;
			case TechId::ControlledDemolition:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastControlledDemolition;
				helper::QueueGameCommand(buf, 0xc);
			} break;
			case TechId::EmergencyDetonation:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastEmergencyDetonation;
				helper::QueueGameCommand(buf, 0xc);
			} break;
			case TechId::StarSovereignTheGriefOfAllGods:
			{
				auto firstInList = clientSelectionGroup->unit[0];
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
				buf[0] = 0x99;
				buf[1] = NewGameCommand::IssueCTFCop2;
				buf[2] = *ACTIVE_PLAYER_ID;
				helper::QueueGameCommand(buf, 0xc);
			} break;
			case TechId::ApostleLazarusAgent:
			{
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
				buf[0] = 0x99;
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = RustId::CastLazarusAgent;
				helper::QueueGameCommand(buf, 0xc);
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
				helper::QueueGameCommand(buf, 0xc);
			} break;
			case TechId::PaladinSkyfall:
			{
				helper::issueTriTargetOrder(OrderId::PaladinSkyfall, 0, 0);
			} break;
			case TechId::AkistrokorEnthrallingAssault:
			case TechId::SovrolethConjoiningRoot:
			{
				helper::issueTriTargetOrder(OrderId::CTFCOP1, 0, 0);
			} break;
			case TechId::SavantPowerSiphon:
			{
				helper::issueTriTargetOrder(OrderId::Hover, 0, 0);
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
				helper::QueueGameCommand(buf, 0xc);
			} break;
			case TechId::MindControl:
			{
				helper::issueTriTargetOrder(OrderId::CastMindControl, 0, 0);
			} break;
			case TechId::LuminaryAuralAssault:
			{
				helper::issueTriTargetOrder(OrderId::CastOpticalFlare, 0, 0);
			} break;
			case TechId::Maelstrom:
			{
				helper::issueTriTargetOrder(OrderId::CastMaelstrom, 0, 0);
			} break;
			case TechId::TalosSalvage:
			{
				helper::issueTriTargetOrder(OrderId::CastFeedback, 0, 0);
			} break;
			default:
			{
			}
		};
	}
}