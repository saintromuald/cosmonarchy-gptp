#include "supply_hooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>

namespace hooks
{
    // Until further notice, these hooks are all for the shared supply rewrite - Pr0nogo
    u32 getSuppliesAvailable(u8 playerId, s32 race) // 00488900
    {
        u32 result = suppliesMax_Terran[playerId];
        u32 suppliesAvailable = suppliesAvailable_Terran[playerId];
        if (suppliesAvailable < result)
        {
            result = suppliesAvailable;
        }
        return result;
    }

    s32 getSuppliesRemaining(u8 playerId, u16 unitId) // 004889D0
    {
        u32 supplyAvailable = 0;
        u32 supplyUsed = 0;
        if (hooks::getSuppliesMax(unitId, playerId) >= suppliesAvailable_Terran[playerId])
        {
            supplyUsed = suppliesUsed_Terran[playerId];
            supplyAvailable = suppliesAvailable_Terran[playerId];
        }
        else
        {
            supplyUsed = suppliesUsed_Terran[playerId];
            supplyAvailable = hooks::getSuppliesMax(unitId, playerId);
        }
        if (*CHEAT_STATE & 0x20000)
        {
            return suppliesMax_Terran[playerId] - supplyUsed;
        }
        return supplyAvailable - supplyUsed;
    }

    s32 getSuppliesMax(u16 unitId, u8 playerId) // 00488990
    {
        return suppliesMax_Terran[playerId];
    }

    s32 getSuppliesUsed(s32 race, u8 playerId) // 004888C0
    {
        return suppliesUsed_Terran[playerId];
    }

    char initializeUnitCounts() // 00488F90
    {
        memset(unitCountScore, 0, 0x540u);
        memset(allUnitCountTable, 0, 0xAB00u);

        s32 result = 0;
        if (multiPlayerMode && gameData->teamsEnabled)
        {
            s32 forceIterator = 1;
            do
            {
                result = unknown_57F41CB[(u8)forceIterator];
                u8 iteratorResult = unknown_57F41CB[(u8)forceIterator];
                if ((s8)result != -1)
                {
                    s32 supplyMax1 = 0;
                    s32 supplyMax2 = 0;
                    s32 supplyMax3 = 0;

                    if (400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 0) >= 400)
                    {
                        supplyMax1 = 400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 0);
                    }
                    else
                    {
                        supplyMax1 = 400;
                    }

                    supplyMax2 = iteratorResult;
                    suppliesMax_Terran[supplyMax2] = supplyMax1;
                    if (400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 1) >= 400)
                    {
                        supplyMax3 = 400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 1);
                    }
                    else
                    {
                        supplyMax3 = 400;
                    }

                    suppliesMax_Terran[supplyMax2] += supplyMax3;
                    if (400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 2) >= 400)
                    {
                        result = 400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 2);
                    }
                    else
                    {
                        result = 400;
                    }

                    suppliesMax_Terran[supplyMax2] += result;
                }
                ++forceIterator;
            }
            while (forceIterator <= 4);
        }
        else
        {
            result = (s32)suppliesMax_Terran;
            do
            {
                *(s32*)(result - 144) = 400;
                *(s32*)result = 400;
                *(s32*)(result + 144) = 400;
                result += 4;
            }
            while (result < (s32)suppliesAvailable_Protoss);
        }
        return result;
    }

    void StatRes_MainProc(BinDlg* dialog) // 004E5640
    {
        u8 v2;
        s32 v3;
        Unit* activeUnit;
        s8 activePlayer;
        u16 v6;
        u8 v7 = 0;
        s32 v8;
        u16 v9;
        u8 v10;
        s32 v11;
        s32 v12;
        s32 v13;
        u16 v14;
        s32 v15 = 0;
        s32 v16;
        BinDlg* v17;
        BinDlg* v18;
        s32 v19;

        if (dialog->controlType)
        {
            dialog = dialog->parent;
        }
        BinDlg* dialogVar = dialog->childrenDlg;
        if (dialogVar)
        {
            while (dialogVar->index != 1)
            {
                dialogVar = dialogVar->next;
                if (!dialogVar)
                {
                    goto LABEL_6;
                }
            }
        }
        else
        {
LABEL_6:
            dialogVar = 0;
        }
        v2 = *LOCAL_NATION_ID;
        v3 = 0;
        v19 = *LOCAL_NATION_ID;
        if (!*IS_IN_REPLAY)
        {
            goto LABEL_15;
        }
        if (*clientSelectionCount)
        {
            activeUnit = *activePortraitUnit;
            if (*clientSelectionCount != 1)
            {
                activeUnit = clientSelectionGroup->unit[0];
            }
            activePlayer = activeUnit->playerId;
            if (activePlayer != 11)
            {
                v2 = activePlayer;
                *LOCAL_NATION_ID = activePlayer;
                if (activePlayer != *unknown_player_5153F8)
                {
                    v3 = 1;
                }
                *unknown_player_5153F8 = activePlayer;
LABEL_15:
                //v8 = suppliesUsed_Zerg[v2];
                //dialog = nullptr; // was lobyte = 0
                //v6 = getSuppliesAvailable(v2, (s32)dialog);
                //if (v6)
                //{
                //    helper::showSupplyInfo(dialogVar, 4, v8, v6);
                //    dialogVar = dialogVar->next;
                //    v7 = *LOCAL_NATION_ID;
                //}
                v12 = v7;
                v13 = suppliesUsed_Terran[v7];
                v12 = 1;
                v9 = getSuppliesAvailable(v7, v12);
                if (v9)
                {
                    helper::showSupplyInfo(dialogVar, 5, v13, v9);
                    dialogVar = dialogVar->next;
                    v10 = *LOCAL_NATION_ID;
                }
                //v16 = suppliesUsed_Protoss[v10];
                //v11 = 2;
                //v14 = getSuppliesAvailable(v10, v11);
                //if (v14)
                //{
                //    helper::showSupplyInfo(dialogVar, 6, v16, v14);
                //    dialogVar = dialogVar->next;
                //    v15 = *LOCAL_NATION_ID;
                //}
                helper::printStatResInfo(dialogVar, *consoleIndex + 1, (char**)&(*unknown_68C22C), (char*)gameGas[v15], v3);
                v18 = dialogVar->next;
                helper::printStatResInfo(v18, 0, (char**)&(*unknown_68C22C), (char*)gameMinerals[*LOCAL_NATION_ID], v3);
                v17 = v18->next;
                if (v18->next)
                {
                    do
                    {
                        if (v17->index > 5)
                        {
                            break;
                        }
                        helper::hideDialog(v17);
                        v17 = v17->next;
                    }
                    while (v17);
                }
                if (*IS_IN_REPLAY)
                {
                    *LOCAL_NATION_ID = v19;
                }
                return;
            }
        }
        for (*unknown_player_5153F8 = -1; dialogVar; dialogVar = dialogVar->next)
        {
            if (dialogVar->index > 5)
            {
                break;
            }
            helper::hideDialog(dialogVar);
        }
    }

    s32 getSuppliesTemp() // 004246D0
    {
        Unit* activeUnit = *activePortraitUnit;
        GroupFlag groupFlags = units_dat::GroupFlags[activeUnit->id];
        *suppliesAvailableTemp = getSuppliesAvailable(activeUnit->playerId, RaceId::Terran);
        u8 playerId = activeUnit->playerId;
        s32 result = suppliesUsed_Terran[playerId];
        *suppliesUsedTemp = suppliesUsed_Terran[playerId];
        return result;
    }

    bool redrawSupplyInfo() // 00425230 // Guessed name
    {
        if (!IS_IN_REPLAY && (u8)(*activePortraitUnit)->playerId != *LOCAL_NATION_ID)
        {
            return helper::unitStatCond_Standard();
        }

        u8 activePlayer = (*activePortraitUnit)->playerId;
        GroupFlag groupFlags = units_dat::GroupFlags[(u16)(*activePortraitUnit)->id];
        char race = RaceId::Terran;

        if ((u16)suppliesAvailableTemp == getSuppliesAvailable(activePlayer, race))
        {
            s32 suppliesVar = suppliesUsed_Terran[activePlayer];
            if ((u16)suppliesUsedTemp == suppliesVar)
            {
                return 0;
            }
        }
        return 1;
    }

    void drawSupplyUnitStatus(BinDlg* dialog, s32 unused) // 00427540
    {
        u8 v5;
        s32 v6;
        WORD v7;
        const char* v8;
        WORD v9;
        const char* v10;
        u16 v11;
        s32 v12;
        const char* v13;
        u32 v14;
        GroupFlag v15;
        s32 v16;
        WORD v17;
        const char* v18;
        u16 v19;
        BinDlg* v20;
        s32 v21;
        s32 race;

        BinDlg* dialogVar = dialog;
        Unit* activeUnit = *activePortraitUnit;
        if (*IS_IN_REPLAY || (u8)(*activePortraitUnit)->playerId == *LOCAL_NATION_ID)
        {
            getSuppliesTemp();
            if (*statusScreenFunc != 12)
            {
                helper::hideOptionalStatusScreenControls(dialogVar);
                helper::showControlGroup(dialogVar, -10, -13);
                activeUnit = *activePortraitUnit;
                *statusScreenFunc = 12;
            }
            GroupFlag groupFlag = units_dat::GroupFlags[(u16)activeUnit->id];
            race = RaceId::Terran;
            v5 = activeUnit->playerId;
            v6 = suppliesUsed_Terran[v5];
            v21 = (u8)race;
            v7 = (u8)race + 820;
            if ((u8)race == -821)
            {
                v8 = 0;
            }
            else if (v7 < *statTxtTbl)
            {
                v8 = (char*)statTxtTbl + statTxtTbl[v7 + 1];
            }
            else
            {
                v8 = "";
            }
            sprintf_s(unknown_struct_6CA9F8, 260, "%s %d", v8, (v6 + 1) / 2);
            v9 = (u8)race + 814;
            if ((u8)race == -815)
            {
                v10 = 0;
            }
            else if (v9 < *statTxtTbl)
            {
                v10 = (char*)statTxtTbl + statTxtTbl[v9 + 1];
            }
            else
            {
                v10 = "";
            }
            v11 = helper::getLastQueueSlotType(*activePortraitUnit);
            //sprintf_s("", 260, "%s %d", v10, (units_dat::SupplyProvided[v11] + 1) / 2);
            v12 = v21 + 66353;
            if ((u8)race == -818)
            {
                v13 = 0;
            }
            else if ((u16)v12 < *statTxtTbl)
            {
                v12 = (u16)v12;
                v13 = (char*)statTxtTbl + statTxtTbl[(u16)v12 + 1];
            }
            else
            {
                v13 = "";
            }
            v12 = race;
            v14 = getSuppliesAvailable((*activePortraitUnit)->playerId, v12);
            sprintf_s(unkDialogByte, 260, "%s %d", v13, (s32)(v14 + 1) / 2);
            race = (*activePortraitUnit)->playerId;
            v15 = units_dat::GroupFlags[helper::getLastQueueSlotType(*activePortraitUnit)];
            v16 = suppliesMax_Terran[(u8)race];
            v17 = v21 + 823;
            if ((s16)v21 == -824)
            {
                v18 = 0;
            }
            else if (v17 < *statTxtTbl)
            {
                v18 = (char*)statTxtTbl + statTxtTbl[v17 + 1];
            }
            else
            {
                v18 = "";
            }
            sprintf_s(unknown_struct_6CAB08, 260, "%s %d", v18, (u32)(v16 + 1) >> 1);
            helper::addTextToDialog(dialogVar, -10, unknown_struct_6CA9F8);
            helper::addTextToDialog(dialogVar, -11, "");
            helper::addTextToDialog(dialogVar, -12, unkDialogByte);
            helper::addTextToDialog(dialogVar, -13, unknown_struct_6CAB08);
            v19 = (*activePortraitUnit)->id;
            if (units_dat::ShieldsEnabled[v19])
            {
                helper::drawShieldUpgrade(dialogVar, 0);
            }
            else if (v19 != UnitId::ZergOvileth)
            {
                if (dialogVar->controlType)
                    dialogVar = *(BinDlg**)&dialogVar->parent;
                v20 = *(BinDlg**)&dialogVar->childrenDlg;
                if (v20)
                {
                    while (v20->index != 9)
                    {
                        v20 = v20->next;
                        if (!v20)
                        {
                            goto LABEL_55;
                        }
                    }
                }
                else
                {
LABEL_55:
                    v20 = 0;
                }
                helper::hideDialog(v20);
            }
            else
            {
                helper::drawArmor(dialogVar, v19, 0);
            }
        }
        else
        {
            helper::unitStatAct_Standard(dialog);
        }
    }
}