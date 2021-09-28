#include "supply_hooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>
#include <globals.h>

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
                    dialogVar = 0;
                    break;
                }
            }
        }
        else
        {
            dialogVar = 0;
        }

        s32 activePlayer = *LOCAL_NATION_ID;
        s32 replay_switch_something = 0;
        if (*IS_IN_REPLAY)
        {

            if (*clientSelectionCount)
            {
                Unit* activeUnit = *activePortraitUnit;
                if (*clientSelectionCount != 1)
                {
                    activeUnit = clientSelectionGroup->unit[0];
                }
                activePlayer = activeUnit->playerId;
                if (activePlayer != 11)
                {
                    activePlayer = activePlayer;
                    *LOCAL_NATION_ID = activePlayer;
                    if (activePlayer != *unknown_player_5153F8)
                    {
                        replay_switch_something = 1;
                    }
                    *unknown_player_5153F8 = activePlayer;
                }
            }
            else
            {
                for (*unknown_player_5153F8 = -1; dialogVar; dialogVar = dialogVar->next)
                {
                    if (dialogVar->index > 5)
                    {
                        break;
                    }
                    helper::hideDialog(dialogVar);
                }
                return;
            }
        }

        s32 supplies = suppliesUsed_Terran[*LOCAL_NATION_ID];
        if (playerTable[activePlayer].race == RaceId::Zerg)
        {
            helper::showSupplyInfo(dialogVar, 4, supplies, getSuppliesAvailable(*LOCAL_NATION_ID, RaceId::Zerg));
            dialogVar = dialogVar->next;
        }
        else if (playerTable[activePlayer].race == RaceId::Terran)
        {
            helper::showSupplyInfo(dialogVar, 5, supplies, getSuppliesAvailable(*LOCAL_NATION_ID, RaceId::Terran));
            dialogVar = dialogVar->next;
        }
        else if (playerTable[activePlayer].race == RaceId::Protoss)
        {
            helper::showSupplyInfo(dialogVar, 6, supplies, getSuppliesAvailable(*LOCAL_NATION_ID, RaceId::Protoss));
            dialogVar = dialogVar->next;
        }

        helper::printStatResInfo(dialogVar, *consoleIndex + 1, (char**)unknown_68C22C, (char*)gameGas[*LOCAL_NATION_ID], replay_switch_something);
        BinDlg* v18 = dialogVar->next;
        helper::printStatResInfo(v18, 0, (char**)unknown_68C230, (char*)gameMinerals[*LOCAL_NATION_ID], replay_switch_something);
        BinDlg* v17 = v18->next;
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
            *LOCAL_NATION_ID = activePlayer;
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
        s32 playerId;

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
            playerId = RaceId::Terran;
            v5 = activeUnit->playerId;
            v6 = suppliesUsed_Terran[v5];
            v21 = (u8)playerId;
            v7 = (u8)playerId + 820;
            if ((u8)playerId == -821)
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
            v9 = (u8)playerId + 814;
            if ((u8)playerId == -815)
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
            sprintf_s(aKills1, 260, "%s %d", v10, (units_dat::SupplyProvided[v11] + 1) / 2);
            v12 = v21 + 66353;
            if ((u8)playerId == -818)
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
            v12 = playerId;
            v14 = getSuppliesAvailable((*activePortraitUnit)->playerId, v12);
            sprintf_s(unkDialogByte, 260, "%s %d", v13, (s32)(v14 + 1) / 2);
            playerId = (*activePortraitUnit)->playerId;
            v15 = units_dat::GroupFlags[helper::getLastQueueSlotType(*activePortraitUnit)];
            v16 = suppliesMax_Terran[(u8)playerId];
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

    void incrementUnitScores(Unit* unit, s32 count) // 00488BF0
    {
        if (unit->status & UnitStatus::IsHallucination)
        {
            return;
        }
        if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit)
        {
            return;
        }
        allUnitCountTable[unit->id].player[unit->playerId] += count;
        GroupFlag groupFlag = units_dat::GroupFlags[(u16)unit->id];
        
        if (unit->getRace() == RaceId::Zerg)
        {
            u8 supply;
            // zerg special handling
            if (unit->id != UnitId::ZergEgg && unit->id != UnitId::ZergCocoon)
            {
                supply = units_dat::SupplyRequired[unit->id];
                if (!(units_dat::BaseProperty[unit->id] & 0x400) || unit->status & 1)
                {
                    raceSupply[RaceId::Terran].used[unit->playerId] += count * supply;
                }
                else
                {
                    supply *= 2;
                    raceSupply[RaceId::Terran].used[unit->playerId] += count * supply;
                }
            }
            else
            {
                s32 queueUnit = unit->buildQueue[unit->buildQueueSlot];
                supply = units_dat::SupplyRequired[queueUnit];
                if (units_dat::BaseProperty[queueUnit] & UnitProperty::TwoUnitsIn1Egg)
                {
                    supply *= 2;
                }

                raceSupply[RaceId::Terran].used[unit->playerId] += count * supply;
            }
        }
        else
        {
            raceSupply[RaceId::Terran].used[unit->playerId] += count * units_dat::SupplyRequired[unit->id];
        }

        if (groupFlag.isFactory)
        {
            factoriesProducedOwnedTotal[unit->playerId] += count;
        }
        if (groupFlag.isMen)
        {
            unitsProducedOwnedTotal[unit->playerId] += count;
        }
        if (groupFlag.isBuilding)
        {
            structuresProducedOwnedTotal[unit->playerId] += count;
        }
        if (unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon)
        {
            unitsProducedOwnedTotal[unit->playerId] += count;
        }

        if (allUnitCountTable[unit->id].player[unit->playerId] < 0)
        {
            allUnitCountTable[unit->id].player[unit->playerId] = 0;
        }
    }

    void incrementUnitScoresEx(s32 count, Unit* unit, s32 increment_scores) // 00488D50
    {
        UnitExt* unitExt = globals.GetUnitExt(unit);

        if (unit->status & UnitStatus::IsHallucination)
        {
            return;
        }
        if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit)
        {
            return;
        }
        auto player = unit->playerId;
        auto unit_id = unit->id;
        completed_unit_counts[unit->id].player[unit->playerId] += count;

        s32 race = unit->getRace();
        s32 supply = units_dat::SupplyProvided[unit->id];
        if (scbw::isZergEggNew(unit, false, false))
        {
            if (unit->buildQueue[unit->buildQueueSlot % 5] == UnitId::ZergIroleth)
            {
                supply = units_dat::SupplyProvided[UnitId::ZergOvileth];
            }
            if (unit->buildQueue[unit->buildQueueSlot % 5] == UnitId::ZergOthstoleth)
            {
                supply = units_dat::SupplyProvided[UnitId::ZergIroleth];
            }
            if (unit->buildQueue[unit->buildQueueSlot % 5] == UnitId::ZergAlaszileth)
            {
                supply = units_dat::SupplyProvided[UnitId::ZergOthstoleth];
            }
        }

        if (*elapsedTimeFrames > 0)
        {
            u32 supply_new = unitExt->values[ValueSupplyProvided];
            if (supply_new)
            {
                supply = supply_new;
            }
            unitExt->values[ValueSupplyProvided] = supply;
            auto supply_overlord_modifier = unitExt->values[ValueOvilethSupply];
            if (supply_overlord_modifier > 0)
            {
                switch (supply_overlord_modifier)
                {
                    case 1:
                    {
                        supply = units_dat::SupplyProvided[UnitId::ZergIroleth] - units_dat::SupplyProvided[UnitId::ZergOvileth];
                    } break;
                    case 2:
                    {
                        supply = units_dat::SupplyProvided[UnitId::ZergOthstoleth] - units_dat::SupplyProvided[UnitId::ZergIroleth]; 
                    } break;
                    case 3:
                    {
                        supply = units_dat::SupplyProvided[UnitId::ZergAlaszileth] - units_dat::SupplyProvided[UnitId::ZergOthstoleth];
                    } break;
                    default:
                    {
                    } break;
                }
            }
            else if (unit->id == UnitId::ZergOthstoleth)
            {
                supply = units_dat::SupplyProvided[unit->id];
            }
            unitExt->values[ValueOvilethSupply] = 0;
        }
        if (race <= RaceId::Protoss)
        {
            raceSupply[RaceId::Terran].provided[player] += count * supply;
        }
        if (units_dat::GroupFlags[unit_id].isFactory)
        {
            factoriesOwned[unit->playerId] += count;
        }
        if (units_dat::GroupFlags[unit_id].isBuilding)
        {
            buildingsOwned[unit->playerId] += count;
        }
        if (units_dat::GroupFlags[unit_id].isMen)
        {
            unitsProduced[unit->playerId] += count;
        }
        if (increment_scores != 0)
        {
            if (units_dat::GroupFlags[unit_id].isMen)
            {
                if (!scbw::isAnyMorphMorphResult(unit_id))
                {
                    unitsOwned[unit->playerId] += count;
                }
                unitScore[unit->playerId] += count;
            }
            else if (units_dat::GroupFlags[unit_id].isBuilding)
            {
                if (!scbw::isZergBuildingMorphResult(unit_id))
                {
                    buildingsOwned[unit->playerId] += count;
                }
                buildingScore[unit->playerId] += count * units_dat::BuildScore[unit_id];
            }
        }
        if (completed_unit_counts[unit_id].player[unit->playerId] < 0)
        {
            completed_unit_counts[unit_id].player[unit->playerId] = 0;
        }
    }

    // Check if @p playerId has enough supplies to build @p unitId.
    bool hasSuppliesForUnitHook(u8 playerId, u16 unitId, bool canShowErrorMessage)
    {
        // Default StarCraft behavior
        s32 supplyCost = units_dat::SupplyRequired[unitId];
        if (units_dat::BaseProperty[unitId] & UnitProperty::TwoUnitsIn1Egg)
        {
            supplyCost *= 2;
        }
        if (unitId == UnitId::ZergLakizilisk)
        {
            supplyCost -= units_dat::SupplyRequired[UnitId::ZergHydralisk];
        }

        aiSupplyReserved[playerId] = supplyCost;
        // No supply cost check needed
        if (supplyCost == 0 || units_dat::BaseProperty[unitId] & UnitProperty::MorphFromOtherUnit)
        {
            return true;
        }
        u32 raceId = playerTable[playerId].race;
        const u32 supplyUsed = raceSupply[RaceId::Terran].used[playerId];
        // Must construct additional pylons
        if (supplyUsed + supplyCost > raceSupply[RaceId::Terran].max[playerId])
        {
            if (canShowErrorMessage)
            {
                scbw::showErrorMessageWithSfx(playerId, 847 + raceId, 1 + raceId);
            }
            return false;
        }
        // Supply limit exceeded
        if (supplyCost > scbw::getSupplyRemaining(playerId, RaceId::Terran))
        {
            if (canShowErrorMessage)
            {
                scbw::showErrorMessageWithSfx(playerId, 844 + raceId, 153 + raceId);
            }
            return false;
        }
        return true;
    }
}