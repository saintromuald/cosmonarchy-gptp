#include "supply_hooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>
#include <globals.h>

namespace hooks
{
    // Until further notice, these hooks are all for the shared supply rewrite - Pr0nogo
    u32 get_supplies_available(u8 playerId, s32 race) // 00488900
    {
        u32 result = suppliesMax_Terran[playerId];
        u32 suppliesAvailable = suppliesAvailable_Terran[playerId];
        if (suppliesAvailable < result)
        {
            result = suppliesAvailable;
        }
        return result;
    }

    s32 get_supplies_remaining(u8 playerId, u16 unitId) // 004889D0
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

    s32 get_supplies_used(s32 race, u8 playerId) // 004888C0
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
        BinDlg* dialogVar = dialog->children_dialog;
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
                activePlayer = activeUnit->player;
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
                    helper::hide_dialog(dialogVar);
                }
                return;
            }
        }

        s32 workers = 0;
        s32 men = 0;
        for (Unit* unit = firstPlayerUnit->unit[*LOCAL_NATION_ID]; unit; unit = unit->player_link.next) {
            if (unit->status & UnitStatus::Completed) {
                if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker) {
                    workers += 2;
                } else if (units_dat::GroupFlags[unit->id].isMen) {
                    men += 2;
                }
            }
        }

        if (playerTable[activePlayer].race == RaceId::Zerg)
        {
            helper::showSupplyInfo(dialogVar, 4, workers, men);
            dialogVar = dialogVar->next;
        }
        else if (playerTable[activePlayer].race == RaceId::Terran)
        {
            helper::showSupplyInfo(dialogVar, 5, workers, men);
            dialogVar = dialogVar->next;
        }
        else if (playerTable[activePlayer].race == RaceId::Protoss)
        {
            helper::showSupplyInfo(dialogVar, 6, workers, men);
            dialogVar = dialogVar->next;
        }

        helper::printStatResInfo(dialogVar, *console_index + 1, (char**)unknown_68C22C, (char*)gameGas[*LOCAL_NATION_ID], replay_switch_something);
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
                helper::hide_dialog(v17);
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
        *suppliesAvailableTemp = get_supplies_available(activeUnit->player, RaceId::Terran);
        u8 playerId = activeUnit->player;
        s32 result = suppliesUsed_Terran[playerId];
        *suppliesUsedTemp = suppliesUsed_Terran[playerId];
        return result;
    }

    bool redrawSupplyInfo() // 00425230 // Guessed name
    {
        if (!IS_IN_REPLAY && (u8)(*activePortraitUnit)->player != *LOCAL_NATION_ID)
        {
            return helper::unitStatCond_Standard();
        }

        u8 activePlayer = (*activePortraitUnit)->player;
        GroupFlag groupFlags = units_dat::GroupFlags[(u16)(*activePortraitUnit)->id];
        char race = RaceId::Terran;

        if ((u16)suppliesAvailableTemp == get_supplies_available(activePlayer, race))
        {
            s32 suppliesVar = suppliesUsed_Terran[activePlayer];
            if ((u16)suppliesUsedTemp == suppliesVar)
            {
                return 0;
            }
        }
        return 1;
    }

	void drawSupplyUnitStatus(BinDlg* dialog) {
		Unit* unit = *activePortraitUnit;
		if (*IS_IN_REPLAY || unit->player == *LOCAL_NATION_ID) {
			getSuppliesTemp();
			if (*statusScreenFunc != 12) {
				helper::hide_optional_status_screen_controls(dialog);
				helper::show_control_group(dialog, -10, -13);
				*statusScreenFunc = 12;
			}

            /*
			u32 race = unit->get_race();
			u32 supply_used = suppliesUsed_Terran[unit->playerId];

			u32 supply_used_string_id = race + 820;
            char* supply_used_string = 0;
			if (race == -821) {
				supply_used_string = 0;
			} else if (supply_used_string_id < **statTxtBuffer) {
				supply_used_string = (char*)*statTxtBuffer + (*statTxtBuffer)[supply_used_string_id + 1];
			} else {
				supply_used_string = "";
			}
			sprintf_s(supply_used_buffer, 260, "%s %d", supply_used_string, (supply_used + 1) / 2);

			u32 supply_provided_string_id = race + 814;
            char* supply_provided_string = 0;
			if (race == -815) {
				supply_provided_string = 0;
			} else if (supply_provided_string_id < **statTxtBuffer) {
				supply_provided_string = (char*)*statTxtBuffer + (*statTxtBuffer)[supply_provided_string_id + 1];
			} else {
				supply_provided_string = "";
			}
			u32 last_queue_slot_type = helper::get_last_queue_slot_id(unit);
			sprintf_s(supply_provided_buffer, 260, "%s %d", supply_provided_string, (units_dat::SupplyProvided[last_queue_slot_type] + 1) / 2);
			
            u32 supply_available_string_id = race + 66353;
            char* supply_available_string = 0;
			if (race == -818) {
				supply_available_string = 0;
			} else if (supply_available_string_id < **statTxtBuffer) {
				supply_available_string = (char*)*statTxtBuffer + (*statTxtBuffer)[supply_available_string_id + 1];
			} else {
				supply_available_string = "";
			}
			u32 supply_available = get_supplies_available(unit->playerId, race);
			sprintf_s(supply_available_buffer, 260, "%s %d", supply_available_string, (supply_available + 1) / 2);

            u32 supply_max = suppliesMax_Terran[unit->playerId];
			u32 supply_max_string_id = race + 823;
            char* supply_max_string = 0;
			if (race == -824) {
				supply_max_string = 0;
			} else if (supply_max_string_id < **statTxtBuffer) {
				supply_max_string = (char*)*statTxtBuffer + (*statTxtBuffer)[supply_max_string_id + 1];
			} else {
				supply_max_string = "";
			}
			sprintf_s(supply_max_buffer, 260, "%s %d", supply_max_string, (supply_max + 1) >> 1);

			helper::add_text_to_dialog(dialog, -10, supply_used_buffer);
			helper::add_text_to_dialog(dialog, -11, supply_provided_buffer);
			helper::add_text_to_dialog(dialog, -12, supply_available_buffer);
			helper::add_text_to_dialog(dialog, -13, supply_max_buffer);
            */

            BinDlg* child = dialog->controlType ? dialog->parent->children_dialog : dialog->children_dialog;
            while (child) {
                helper::hide_dialog(child);
                child = child->next;
            }

            helper::drawArmor(dialog, unit->id, 0);
			if (units_dat::ShieldsEnabled[unit->id]) {
				helper::draw_shield_upgrade(dialog, 1);
			}
		} else {
			helper::unit_stat_act_standard(dialog);
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
        allUnitCountTable[unit->id].player[unit->player] += count;
        GroupFlag groupFlag = units_dat::GroupFlags[(u16)unit->id];
        
        if (unit->get_race() == RaceId::Zerg)
        {
            u8 supply;
            // zerg special handling
            if (unit->id != UnitId::ZergEgg && unit->id != UnitId::ZergCocoon)
            {
                supply = units_dat::SupplyRequired[unit->id];
                if (!(units_dat::BaseProperty[unit->id] & 0x400) || unit->status & 1)
                {
                    raceSupply[RaceId::Terran].used[unit->player] += count * supply;
                }
                else
                {
                    supply *= 2;
                    raceSupply[RaceId::Terran].used[unit->player] += count * supply;
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

                raceSupply[RaceId::Terran].used[unit->player] += count * supply;
            }
        }
        else
        {
            raceSupply[RaceId::Terran].used[unit->player] += count * units_dat::SupplyRequired[unit->id];
        }

        if (groupFlag.isFactory)
        {
            factoriesProducedOwnedTotal[unit->player] += count;
        }
        if (groupFlag.isMen)
        {
            unitsProducedOwnedTotal[unit->player] += count;
        }
        if (groupFlag.isBuilding)
        {
            structuresProducedOwnedTotal[unit->player] += count;
        }
        if (unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon)
        {
            unitsProducedOwnedTotal[unit->player] += count;
        }

        if (allUnitCountTable[unit->id].player[unit->player] < 0)
        {
            allUnitCountTable[unit->id].player[unit->player] = 0;
        }
    }

    void increment_unit_scores_ex(s32 count, Unit* unit, s32 increment_scores) // 00488D50
    {
        UnitExt* unitExt = globals.get_unit_ext(unit);

        if (unit->status & UnitStatus::IsHallucination)
        {
            return;
        }
        if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit)
        {
            return;
        }
        auto player = unit->player;
        auto unit_id = unit->id;
        completed_unit_counts[unit->id].player[unit->player] += count;

        s32 race = unit->get_race();
        s32 supply = unit->getSupplyProvided();
        if (race <= RaceId::Protoss)
        {
            raceSupply[RaceId::Terran].provided[player] += count * supply;
        }
        if (units_dat::GroupFlags[unit_id].isFactory)
        {
            factoriesOwned[unit->player] += count;
        }
        if (units_dat::GroupFlags[unit_id].isBuilding)
        {
            buildingsOwned[unit->player] += count;
        }
        if (units_dat::GroupFlags[unit_id].isMen)
        {
            unitsProduced[unit->player] += count;
        }
        if (increment_scores != 0)
        {
            if (units_dat::GroupFlags[unit_id].isMen)
            {
                if (!scbw::is_any_morph_result(unit_id))
                {
                    unitsOwned[unit->player] += count;
                }
                unitScore[unit->player] += count;
            }
            else if (units_dat::GroupFlags[unit_id].isBuilding)
            {
                if (!scbw::is_zerg_building_morph_result(unit_id))
                {
                    buildingsOwned[unit->player] += count;
                }
                buildingScore[unit->player] += count * units_dat::BuildScore[unit_id];
            }
        }
        if (completed_unit_counts[unit_id].player[unit->player] < 0)
        {
            completed_unit_counts[unit_id].player[unit->player] = 0;
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
                scbw::show_error_message_with_sfx(playerId, 847 + raceId, 1 + raceId);
            }
            return false;
        }
        // Supply limit exceeded
        if (supplyCost > scbw::get_supply_remaining(playerId, RaceId::Terran))
        {
            if (canShowErrorMessage)
            {
                scbw::show_error_message_with_sfx(playerId, 844 + raceId, 153 + raceId);
            }
            return false;
        }
        return true;
    }
}