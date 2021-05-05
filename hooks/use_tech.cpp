#include "use_tech.h"
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"
#include "hook_tools.h"
#include "SCBW/stat_txt_spec.h"

namespace hooks
{
	void BTNSACT_UseTech(u16 tech, GrpHead* head)
	{
		// replacement for proc 0x00423660
		bool TechCapableUnit = false;
		CUnit* firstInList = playersSelections->unit[*ACTIVE_NATION_ID][0];
		if (firstInList != nullptr)
		{
			// initial check for energy-based switch abilities
			if (playersSelections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::TerranAzazel &&
				scbw::get_generic_rust_value(firstInList, ValueId::SublimeShepherd) == 1)
			{
				TechCapableUnit = true;

			}
			if (playersSelections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::ProtossLanifect &&
				scbw::get_generic_rust_value(firstInList, ValueId::DisruptionWeb) == 1)
			{
				TechCapableUnit = true;

			}
			if (tech == TechId::LobotomyMine)
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
				if (playersSelections->unit[*ACTIVE_NATION_ID][0]->id == UnitId::TerranAzazel &&
					scbw::get_generic_rust_value(firstInList, ValueId::SublimeShepherd) == 1)
				{
					TechCapableUnit = true;
				}
				if (firstInList->_padding_0x132 & 0x8)
				{
					TechCapableUnit = true;
				}
			}
			if (!TechCapableUnit)
			{
				char* message = (char*)statTxtTblNew->getString(864);
				CUnit* portrait_unit = playersSelections->unit[*ACTIVE_NATION_ID][0];
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
			case TechId::EnsnaringBrood:
			{
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
				buf[0] = 0x99; // new general command
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = ValueId::EnsnaringBroodAbility;
				scbw::SendCommand(buf, 0xc);
			} break;
			case TechId::Parasite:
			{
				helper::issueTriTargetOrder(OrderId::CastParasite, 0, 0);
			} break;
			case TechId::Infestation:
			{
				helper::issueTriTargetOrder(OrderId::InfestMine3, OrderId::Move, OrderId::InfestMine2);
			} break;
			case TechId::ReverseThrust:
			{
				if (firstInList != nullptr)
				{
					s32 action = 1; //on
					if (scbw::get_generic_rust_value(firstInList, ValueId::ReverseThrust) == 1)
					{
						action = 0; //off
					}
					u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0}; // 12
					buf[0] = 0x99; // new general command
					buf[1] = NewGameCommand::GenericSwitchAbility;
					buf[2] = action;
					buf[3] = *ACTIVE_PLAYER_ID;
					buf[4] = ValueId::ReverseThrust;
					scbw::SendCommand(buf, 0xc);
				}
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
			case TechId::TertiaryEyes:
			{
				helper::issueTriTargetOrder(OrderId::Restoration, 0, 0);
			} break;
			case TechId::NaniteField:
			{
				s32 action = 1; // on
				if (scbw::get_generic_rust_value(firstInList, ValueId::NaniteField) == 1)
				{
					action = 0; // off
				}
				// set nanite field
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
				buf[0] = 0x99; // new general command
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = ValueId::NaniteField;
				scbw::SendCommand(buf, 0xc);
			} break;
			case TechId::RescindMindControl:
			{
				s32 action = 1; // on
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0 }; // 12
				buf[0] = 0x99; // new general command
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = ValueId::RescindMindControl;
				scbw::SendCommand(buf, 0xc);
			} break;
			case TechId::Observance:
			{
				helper::issueTriTargetOrder(OrderId::EmpShockwave, 0, 0);
			} break;
			case TechId::GodsMadeFlesh:
			{
				u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };
				buf[0] = 0x99; // new general command
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = ValueId::GodsMadeFleshCast;
				scbw::SendCommand(buf, 0xc);
			} break;
			case TechId::PhaseLink: // and Khaydarin Eclipse
			{
				auto firstInList = clientSelectionGroup->unit[0];
				if (firstInList != nullptr)
				{
					if (firstInList->id == UnitId::ProtossClarion)
					{
						s32 action = 0; // enable
						if (scbw::get_aise_value(firstInList, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
						{
							action = 1; // disable
						}
						u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0}; // 12
						buf[0] = 0x99; // new general command
						buf[1] = NewGameCommand::PhaseLink;
						buf[2] = action;
						buf[3] = *ACTIVE_PLAYER_ID;
						scbw::SendCommand(buf, 0xc);
					}
					else
					{ // exemplar
						s32 action = 1;//on
						if (scbw::get_generic_rust_value(firstInList, ValueId::ExemplarSwitch) == 1)
						{
							action = 0;//off
						}
						u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0}; // 12
						buf[0] = 0x99; // new general command
						buf[1] = NewGameCommand::GenericSwitchAbility;
						buf[2] = action;
						buf[3] = *ACTIVE_PLAYER_ID;
						buf[4] = ValueId::ExemplarSwitch;
						scbw::SendCommand(buf, 0xc);
						break;
					}
				}
			} break;
			case TechId::PlanetCracker:
			{
				auto firstInList = clientSelectionGroup->unit[0];
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0}; // 12
				buf[0] = 0x99; // new general command
				buf[1] = NewGameCommand::IssueCTFCop2; // used for Phase Link, Khaydarin Eclipse, and (in Overlord) Anti-Air Mode
				buf[2] = *ACTIVE_PLAYER_ID;
				scbw::SendCommand(buf, 0xc);
			} break;
			case TechId::SublimeShepherd:
			{
				s32 action = 1; // on
				if (scbw::get_generic_rust_value(firstInList, ValueId::SublimeShepherd) == 1)
				{
					action = 0; // off
				}
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0}; // 12
				buf[0] = 0x99; // new general command
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = ValueId::SublimeShepherd;
				scbw::SendCommand(buf, 0xc);
			} break;
			case TechId::LazarusAgent:
			{
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0}; // 12
				buf[0] = 0x99; // new general command
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = 1;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = ValueId::CastLazarusAgent;
				scbw::SendCommand(buf, 0xc);
			} break;
			case TechId::TempoChange:
			{
				s32 action = 1; // on
				if (scbw::get_generic_rust_value(firstInList, ValueId::TempoChange) == 1)
				{
					action = 0; // off
				}
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0}; // 12
				buf[0] = 0x99; // new general command
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = ValueId::TempoChange;
				scbw::SendCommand(buf, 0xc);
			} break;
			case TechId::Skyfall:
			{
				helper::issueTriTargetOrder(OrderId::Skyfall, 0, 0);
			} break;
			case TechId::EssenceDrain:
			{
				helper::issueTriTargetOrder(OrderId::CTFCOP1, 0, 0);
			} break;
			case TechId::PowerTransfer:
			{
				helper::issueTriTargetOrder(OrderId::Hover, 0, 0);
			} break;
			case TechId::DisruptionWeb:
			{
				s32 action = 1; // on
				if (scbw::get_generic_rust_value(firstInList, ValueId::DisruptionWeb) == 1)
				{
					action = 0; // off
				}
				u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0}; // 12
				buf[0] = 0x99; // new general command
				buf[1] = NewGameCommand::GenericSwitchAbility;
				buf[2] = action;
				buf[3] = *ACTIVE_PLAYER_ID;
				buf[4] = ValueId::DisruptionWeb;
				scbw::SendCommand(buf, 0xc);
			} break;
			case TechId::MindControl:
			{
				helper::issueTriTargetOrder(OrderId::CastMindControl, 0, 0);
			} break;
			case TechId::OpticalFlare:
			{
				helper::issueTriTargetOrder(OrderId::CastOpticalFlare, 0, 0);
			} break;
			case TechId::Maelstrom:
			{
				helper::issueTriTargetOrder(OrderId::CastMaelstrom, 0, 0);
			} break;
			default:
			{
			}
		};
	}
}
