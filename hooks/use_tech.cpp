#include "use_tech.h"
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"
#include "hook_tools.h"
#include "SCBW/stat_txt_spec.h"

extern const u32 Func_TriTargetOrder = 0x0048CAF0;
const void IssueTriTargetOrder(u8 order1, u8 order2, u8 order3)
{
    if (order2 == 0)
    {
        _asm
        {
            PUSHAD
            MOV DL, order1
            MOV CL, DL
            MOV AL, DL
            CALL Func_TriTargetOrder
            POPAD
        }
    }
    else
    {
        if (order3 == 0)
        {
            _asm
            {
                PUSHAD
                MOV CL, order1
                MOV DL, order2
                MOV AL, CL
                CALL Func_TriTargetOrder
                POPAD
            }
        }
        else
        {
            _asm
            {
                PUSHAD
                MOV DL, order1
                MOV CL, order2
                MOV AL, order3
                CALL Func_TriTargetOrder
                POPAD
            }
        }
    }
}

const s32 techUseFunc = 0x00423660;
s32 Ui_CheckEnergyForTechUse(s32 tech_id)
{
    static s32 result;
    __asm {
        PUSHAD
        MOV EAX, tech_id
        CALL techUseFunc
        MOV result, EAX
        POPAD
    }
    return result;
}

namespace hooks
{
    void BTNSACT_UseTech(u16 tech, GrpHead* head)
    {
        //replacement for proc 0x00423660
        bool TechCapableUnit = false;
        /*CUnit* currentClientSelectionUnit;

        char buffer[32];
        sprintf(buffer,"Tech: %d, energy test, cost: %d ",tech,techdata_dat::EnergyCost[tech]);
        scbw::print(buffer);
        */


        /*		for (s32 i = 0; i < SELECTION_ARRAY_LENGTH && !TechCapableUnit; i++)
                {
                    currentClientSelectionUnit = clientSelectionGroup->unit[i];
                    if (currentClientSelectionUnit != nullptr)
                    {
                        if (techdata_dat::EnergyCost[tech] * 256 <= currentClientSelectionUnit->energy)
                        {
                            TechCapableUnit = true;
                        }
                    }
                }*/

                /*
            for (s32 i = 0; i < SELECTION_ARRAY_LENGTH && !TechCapableUnit; i++)
            {
                currentClientSelectionUnit = playersSelections->unit[*ACTIVE_NATION_ID][i];
                //if still desync, try ACTIVE_PLAYER_ID
                if (currentClientSelectionUnit != nullptr)
                {
                    if (techdata_dat::EnergyCost[tech] * 256 <= currentClientSelectionUnit->energy)
                    {
                        TechCapableUnit = true;
                    }
                }
            }*/
        CUnit* firstInList = playersSelections->unit[*ACTIVE_NATION_ID][0];
        if (firstInList != nullptr)
        {
            //initial check for energy-based switch abilities
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
            if (Ui_CheckEnergyForTechUse(tech))
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
                IssueTriTargetOrder(OrderId::PlaceScanner, 0, 0);
                break;
            case TechId::SpiderMines:
                IssueTriTargetOrder(OrderId::PlaceMine, 0, 0);
                break;
            case TechId::YamatoGun:
                IssueTriTargetOrder(OrderId::FireYamatoGun1, OrderId::FireYamatoGun2, 0);
                break;
            case TechId::Lockdown:
                IssueTriTargetOrder(OrderId::MagnaPulse, 0, 0);
                break;
            case TechId::Recall:
                IssueTriTargetOrder(OrderId::Teleport, 0, 0);
                break;
            case TechId::DefensiveMatrix:
                IssueTriTargetOrder(OrderId::DefensiveMatrix, 0, 0);
                break;
                /*		case TechId::SpawnBroodlings:
                            IssueTriTargetOrder(OrderId::SummonBroodlings, 0, 0);
                            break;*/

            case TechId::EnsnaringBrood:
            {
                u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
                buf[0] = 0x99;//new general command
                buf[1] = NewGameCommand::GenericSwitchAbility;
                buf[2] = 1;
                buf[3] = *ACTIVE_PLAYER_ID;
                buf[4] = ValueId::EnsnaringBroodAbility;
                scbw::SendCommand(buf, 0xc);
                //in gameplay sense it is not used as switch ability, 
                //but it uses switch ability framework to not bother with rust
                break;
            }
            /*
            case TechId::SwarmingOmen:
            {
                u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};
                buf[0] = 0x99;//new general command
                buf[1] = NewGameCommand::GenericSwitchAbility;
                buf[2] = 1;
                buf[3] = *ACTIVE_PLAYER_ID;
                buf[4] = ValueId::SwarmingOmenAbility;
                scbw::SendCommand(buf, 0xc);
                //copied from Ensnaring Brood
                break;
            }
            */
            case TechId::Parasite:
                IssueTriTargetOrder(OrderId::CastParasite, 0, 0);
                break;
            case TechId::Infestation:
                IssueTriTargetOrder(OrderId::InfestMine3, OrderId::Move, OrderId::InfestMine2);
                break;

            case TechId::EMPShockwave:
                //			IssueTriTargetOrder(OrderId::EmpShockwave, 0, 0);
                            //reverse thrust switch
                if (firstInList != nullptr)
                {
                    s32 action = 1;//on
                    if (scbw::get_generic_rust_value(firstInList, ValueId::ReverseThrust) == 1)
                    {
                        action = 0;//off
                    }
                    u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
                    buf[0] = 0x99;//new general command
                    buf[1] = NewGameCommand::GenericSwitchAbility;
                    buf[2] = action;
                    buf[3] = *ACTIVE_PLAYER_ID;
                    buf[4] = ValueId::ReverseThrust;
                    scbw::SendCommand(buf, 0xc);
                }
                break;
            case TechId::Irradiate:
                IssueTriTargetOrder(OrderId::Irradiate, 0, 0);
                break;
            case TechId::Ensnare:
                //			IssueTriTargetOrder(OrderId::Ensnare, 0, 0);
                            //land grab
                IssueTriTargetOrder(OrderId::CTFCOPInit, 0, 0);
                break;
            case TechId::StasisField:
                IssueTriTargetOrder(OrderId::StasisField, 0, 0);
                break;
            case TechId::DarkSwarm:
                IssueTriTargetOrder(OrderId::DarkSwarm, 0, 0);
                break;
            case TechId::Plague:
                IssueTriTargetOrder(OrderId::Plague, 0, 0);
                break;
            case TechId::Consume:
                IssueTriTargetOrder(OrderId::Consume, 0, 0);
                break;
            case TechId::PsionicStorm:
                IssueTriTargetOrder(OrderId::PsiStorm, 0, 0);
                break;
            case TechId::Hallucination:
                IssueTriTargetOrder(OrderId::Hallucination1, 0, 0);
                break;
            case TechId::Restoration:
            {
                //			IssueTriTargetOrder(OrderId::Restoration, 0, 0);
                //0 - deploy
                //1 - stop
    /*			if (firstInList != nullptr) {
                    u8 action = 0;
                    if (firstInList->_padding_0x132 & 0x8) {
                        action = 1;
                    }
                    //set medstims
                    u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };//12
                    buf[0] = 0x99;//new general command
                    buf[1] = NewGameCommand::ShamanAction;
                    buf[2] = action;
                    buf[3] = *ACTIVE_PLAYER_ID;
                    scbw::SendCommand(buf, 0xc);
                    //disable nanite field
                    u8 bufNanite[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };//12
                    bufNanite[0] = 0x99;//new general command
                    bufNanite[1] = NewGameCommand::GenericSwitchAbility;
                    bufNanite[2] = 0;
                    bufNanite[3] = *ACTIVE_PLAYER_ID;
                    bufNanite[4] = ValueId::NaniteField;
                    scbw::SendCommand(bufNanite, 0xc);
                }
                break;*/
            }
            case TechId::NaniteField:
            {
                s32 action = 1;//on
                if (scbw::get_generic_rust_value(firstInList, ValueId::NaniteField) == 1)
                {
                    action = 0;//off
                }
                //set nanite field
                u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
                buf[0] = 0x99;//new general command
                buf[1] = NewGameCommand::GenericSwitchAbility;
                buf[2] = action;
                buf[3] = *ACTIVE_PLAYER_ID;
                buf[4] = ValueId::NaniteField;
                scbw::SendCommand(buf, 0xc);
                break;
            }
            case TechId::Reconstitution:// also rescind mind control
            {
                auto zoryusthaleth = true;
                if (firstInList->id == UnitId::ProtossMindTyrant)
                {
                    zoryusthaleth = false;
                }

                s32 action = 1; // on
                /*
                if (zoryusthaleth) {
                    if (scbw::get_generic_rust_value(firstInList, ValueId::Reconstitution) == 1) {
                        action = 0; // off
                    }
                    u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
                    buf[0] = 0x99;//new general command
                    buf[1] = NewGameCommand::GenericSwitchAbility;
                    buf[2] = action;
                    buf[3] = *ACTIVE_PLAYER_ID;
                    buf[4] = ValueId::Reconstitution;
                    scbw::SendCommand(buf, 0xc);
                } else {
                */
                u8 buf[0xc] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0}; // 12
                buf[0] = 0x99; // new general command
                buf[1] = NewGameCommand::GenericSwitchAbility;
                buf[2] = action;
                buf[3] = *ACTIVE_PLAYER_ID;
                buf[4] = ValueId::RescindMindControl;
                scbw::SendCommand(buf, 0xc);
                //}
                break;
            }
            /*		case TechId::UnusedTech26://Cyprian Safety Off/Safety On
                    {
                        //0 - off (enable splash mode)
                        //1 - on (disable splash mode)
                        if (firstInList != nullptr) {
                            s32 action = 0;//enable

                            if (scbw::get_aise_value(firstInList, nullptr, AiseId::SendCyprianValue, 0, 0) == 1) {
                                action = 1;//disable
                            }
                            u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };//12
                            buf[0] = 0x99;//new general command
                            buf[1] = NewGameCommand::CyprianAction;
                            buf[2] = action;
                            buf[3] = *ACTIVE_PLAYER_ID;
                            scbw::SendCommand(buf, 0xc);
                        }
                        break;
                    }*/
                    /*		case TechId::UnusedTech38:
                            {
                                //Equip - Empower hero with relic
                                scbw::print("Try use Tech38");
                                if (firstInList != nullptr) {
                                    if (firstInList->id == 175) {//reliquary
                                        u8 relic_type = 0;
                                        u32 button_number = head->frameCount;
                                        u32 relic = 0;
                                        relic = scbw::get_aise_value(firstInList, nullptr, AiseId::GetRelicByIndex, button_number, 0);
                                        char buf[64];
                                        sprintf(buf, "Relic id: %d", relic);
                                        scbw::print(buf);

                                        if (relic != 0) {
                                            if (relic == 202) {
                                                scbw::print("Blind Judge");
                                            }
                                            scbw::sendGPTP_aise_cmd(firstInList, nullptr, GptpId::EmpowerHero, relic, 0, 0);
                                            if (scbw::get_aise_value(firstInList, nullptr, AiseId::HasRelic, UpgradeId::BlindJudge, 0) == 1) {
                                                scbw::print("Found relic");
                                            }
                                            else {
                                                scbw::print("No relic!");
                                            }
                                            scbw::refreshConsole();
                                        }
                                    }
                                    else {
                                        char buf[64];
                                        sprintf(buf, "Incorrect reliquary id %d", firstInList->id);
                                        scbw::print(buf);
                                    }
                                }
                                else {
                                    scbw::print("No selected units");
                                }
                                break;
                            }*/
            case TechId::UnusedTech39: // observance
                IssueTriTargetOrder(OrderId::EmpShockwave, 0, 0);

                /*			if (firstInList != nullptr) {
                                s32 action = 0;//enable
                                if (scbw::get_generic_value(firstInList, ValueId::Observance) == 1) {
                                    action = 1;//disable
                                }
                                u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };//12
                                buf[0] = 0x99;//new general command
                                buf[1] = NewGameCommand::Observance;
                                buf[2] = action;
                                buf[3] = *ACTIVE_PLAYER_ID;
                                scbw::SendCommand(buf, 0xc);
                            }*/
                break;
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
                /*
                u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };//12
                buf[0] = 0x99;//new general command
                buf[1] = NewGameCommand::IssueCTFCop2;//used for Phase Link, Khaydarin Eclipse and (in Overlord) Anti-Air Mode
                buf[2] = *ACTIVE_PLAYER_ID;
                scbw::SendCommand(buf, 0xc);
                break;
                */

                break;
            }
            case TechId::PlanetCracker:
            {
                auto firstInList = clientSelectionGroup->unit[0];
                u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
                buf[0] = 0x99;//new general command
                buf[1] = NewGameCommand::IssueCTFCop2;//used for Phase Link, Khaydarin Eclipse, and (in Overlord) Anti-Air Mode
                buf[2] = *ACTIVE_PLAYER_ID;
                scbw::SendCommand(buf, 0xc);
                break;
                /*u8 buf[0xc] = { 0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0 };//12
                buf[0] = 0x99;//new general command
                buf[1] = NewGameCommand::GenericSwitchAbility;
                buf[2] = 1;
                buf[3] = *ACTIVE_PLAYER_ID;
                buf[4] = ValueId::CastPlanetCracker;
                scbw::SendCommand(buf, 0xc);
                break;*/
            }
            case TechId::UnusedTech40: // sublime shepherd
            {

                s32 action = 1;//on
                if (scbw::get_generic_rust_value(firstInList, ValueId::SublimeShepherd) == 1)
                {
                    action = 0;//off
                }
                u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
                buf[0] = 0x99;//new general command
                buf[1] = NewGameCommand::GenericSwitchAbility;
                buf[2] = action;
                buf[3] = *ACTIVE_PLAYER_ID;
                buf[4] = ValueId::SublimeShepherd;
                scbw::SendCommand(buf, 0xc);
                break;
            }
            case TechId::LazarusAgent:
            {
                u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
                buf[0] = 0x99;//new general command
                buf[1] = NewGameCommand::GenericSwitchAbility;
                buf[2] = 1;
                buf[3] = *ACTIVE_PLAYER_ID;
                buf[4] = ValueId::CastLazarusAgent;
                scbw::SendCommand(buf, 0xc);
                //in gameplay sense it is not used as switch ability, 
                //but it uses switch ability framework to not bother with rust
                break;
            }
            case TechId::TempoChange:
            {

                s32 action = 1;//on
                if (scbw::get_generic_rust_value(firstInList, ValueId::TempoChange) == 1)
                {
                    action = 0;//off
                }
                u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
                buf[0] = 0x99;//new general command
                buf[1] = NewGameCommand::GenericSwitchAbility;
                buf[2] = action;
                buf[3] = *ACTIVE_PLAYER_ID;
                buf[4] = ValueId::TempoChange;
                scbw::SendCommand(buf, 0xc);
                break;
            }
            case TechId::Skyfall:
                IssueTriTargetOrder(OrderId::Skyfall, 0, 0);
                break;
            case TechId::EssenceDrain:
                IssueTriTargetOrder(OrderId::CTFCOP1, 0, 0);
                break;
            case TechId::PowerTransfer:
                IssueTriTargetOrder(OrderId::Hover, 0, 0);
                break;
            case TechId::DisruptionWeb:
            {
                s32 action = 1;//on
                if (scbw::get_generic_rust_value(firstInList, ValueId::DisruptionWeb) == 1)
                {
                    action = 0;//off
                }
                u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
                buf[0] = 0x99;//new general command
                buf[1] = NewGameCommand::GenericSwitchAbility;
                buf[2] = action;
                buf[3] = *ACTIVE_PLAYER_ID;
                buf[4] = ValueId::DisruptionWeb;
                scbw::SendCommand(buf, 0xc);
                //			IssueTriTargetOrder(OrderId::CastDisruptionWeb, 0, 0);
                break;
            }
            case TechId::MindControl:
                IssueTriTargetOrder(OrderId::CastMindControl, 0, 0);
                break;
                /*		case TechId::Feedback:
                            IssueTriTargetOrder(OrderId::CastFeedback, 0, 0);
                            break;*/
            case TechId::OpticalFlare:
                IssueTriTargetOrder(OrderId::CastOpticalFlare, 0, 0);
                break;
            case TechId::Maelstrom:
                IssueTriTargetOrder(OrderId::CastMaelstrom, 0, 0);
                break;
            default:
                break;
        };
    }
}
