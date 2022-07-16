#include "supply_hooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>
#include <globals.h>

namespace hooks
{
    // Until further notice, these hooks are all for the shared supply rewrite - Pr0nogo
    u32 get_supplies_available(u8 player, s32 race) // 00488900
    {
        u32 result = g_supplies_max_terran[player];
        u32 suppliesAvailable = g_supplies_available_terran[player];
        if (suppliesAvailable < result)
        {
            result = suppliesAvailable;
        }
        return result;
    }

    s32 get_supplies_remaining(u8 player, u16 unit_id) // 004889D0
    {
        u32 supplyAvailable = 0;
        u32 supplyUsed = 0;
        if (hooks::get_supplies_max(unit_id, player) >= g_supplies_available_terran[player])
        {
            supplyUsed = g_supplies_used_terran[player];
            supplyAvailable = g_supplies_available_terran[player];
        }
        else
        {
            supplyUsed = g_supplies_used_terran[player];
            supplyAvailable = hooks::get_supplies_max(unit_id, player);
        }
        if (*g_cheat_state & 0x20000)
        {
            return g_supplies_max_terran[player] - supplyUsed;
        }
        return supplyAvailable - supplyUsed;
    }

    s32 get_supplies_max(u16 unit_id, u8 player) // 00488990
    {
        return g_supplies_max_terran[player];
    }

    s32 get_supplies_used(s32 race, u8 player) // 004888C0
    {
        return g_supplies_used_terran[player];
    }

    char initialize_unit_counts() // 00488F90
    {
        memset(g_unit_count_score, 0, 0x540u);
        memset(g_all_unit_count_table, 0, 0xAB00u);

        s32 result = 0;
        if (multiplayer_mode && game_data->teams_enabled)
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
                    g_supplies_max_terran[supplyMax2] = supplyMax1;
                    if (400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 1) >= 400)
                    {
                        supplyMax3 = 400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 1);
                    }
                    else
                    {
                        supplyMax3 = 400;
                    }

                    g_supplies_max_terran[supplyMax2] += supplyMax3;
                    if (400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 2) >= 400)
                    {
                        result = 400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 2);
                    }
                    else
                    {
                        result = 400;
                    }

                    g_supplies_max_terran[supplyMax2] += result;
                }
                ++forceIterator;
            }
            while (forceIterator <= 4);
        }
        else
        {
            result = (s32)g_supplies_max_terran;
            do
            {
                *(s32*)(result - 144) = 400;
                *(s32*)result = 400;
                *(s32*)(result + 144) = 400;
                result += 4;
            }
            while (result < (s32)g_supplies_available_protoss);
        }
        return result;
    }

    void stat_res_main_proc(BinDlg* dialog) // 004E5640
    {
        if (dialog->control_type)
        {
            dialog = dialog->parent;
        }
        BinDlg* dialogVar = dialog->child_dialog;
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

        s32 activePlayer = *g_local_nation_id;
        s32 replay_switch_something = 0;
        if (*g_is_in_replay)
        {

            if (*g_client_selection_count)
            {
                Unit* activeUnit = *g_active_portrait_unit;
                if (*g_client_selection_count != 1)
                {
                    activeUnit = g_client_selection_group->unit[0];
                }
                activePlayer = activeUnit->player;
                if (activePlayer != 11)
                {
                    activePlayer = activePlayer;
                    *g_local_nation_id = activePlayer;
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
        for (Unit* unit = first_player_unit->unit[*g_local_nation_id]; unit; unit = unit->player_link.next) {
            if (unit->status & UnitStatus::Completed) {
                if (units_dat::base_property[unit->id] & UnitProperty::Worker) {
                    workers += 2;
                } else if (units_dat::group_flags[unit->id].is_men) {
                    men += 2;
                }
            }
        }

        if (g_player_table[activePlayer].race == RaceId::Zerg)
        {
            helper::showSupplyInfo(dialogVar, 4, workers, men);
            dialogVar = dialogVar->next;
        }
        else if (g_player_table[activePlayer].race == RaceId::Terran)
        {
            helper::showSupplyInfo(dialogVar, 5, workers, men);
            dialogVar = dialogVar->next;
        }
        else if (g_player_table[activePlayer].race == RaceId::Protoss)
        {
            helper::showSupplyInfo(dialogVar, 6, workers, men);
            dialogVar = dialogVar->next;
        }

        helper::printStatResInfo(dialogVar, *console_index + 1, (char**)unknown_68C22C, (char*)g_game_vespene[*g_local_nation_id], replay_switch_something);
        BinDlg* v18 = dialogVar->next;
        helper::printStatResInfo(v18, 0, (char**)unknown_68C230, (char*)g_game_minerals[*g_local_nation_id], replay_switch_something);
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
        if (*g_is_in_replay)
        {
            *g_local_nation_id = activePlayer;
        }
    }

    s32 get_supplies_temp() // 004246D0
    {
        Unit* activeUnit = *g_active_portrait_unit;
        GroupFlag groupFlags = units_dat::group_flags[activeUnit->id];
        *g_supplies_available_temp = get_supplies_available(activeUnit->player, RaceId::Terran);
        u8 player = activeUnit->player;
        s32 result = g_supplies_used_terran[player];
        *g_supplies_used_temp = g_supplies_used_terran[player];
        return result;
    }

    bool redraw_supply_info() // 00425230 // Guessed name
    {
        if (!g_is_in_replay && (u8)(*g_active_portrait_unit)->player != *g_local_nation_id)
        {
            return helper::unitStatCond_Standard();
        }

        u8 activePlayer = (*g_active_portrait_unit)->player;
        GroupFlag groupFlags = units_dat::group_flags[(u16)(*g_active_portrait_unit)->id];
        char race = RaceId::Terran;

        if ((u16)g_supplies_available_temp == get_supplies_available(activePlayer, race))
        {
            s32 suppliesVar = g_supplies_used_terran[activePlayer];
            if ((u16)g_supplies_used_temp == suppliesVar)
            {
                return 0;
            }
        }
        return 1;
    }

	void draw_supply_unit_status(BinDlg* dialog) {
		Unit* unit = *g_active_portrait_unit;
		if (*g_is_in_replay || unit->player == *g_local_nation_id) {
			get_supplies_temp();
			if (*g_status_screen_func != 12) {
				helper::hide_optional_status_screen_controls(dialog);
				helper::show_control_group(dialog, -10, -13);
				*g_status_screen_func = 12;
			}

            /*
			u32 race = unit->get_race();
			u32 supply_used = g_supplies_used_terran[unit->player];

			u32 supply_used_string_id = race + 820;
            char* supply_used_string = 0;
			if (race == -821) {
				supply_used_string = 0;
			} else if (supply_used_string_id < **g_stat_txt_buffer) {
				supply_used_string = (char*)*g_stat_txt_buffer + (*g_stat_txt_buffer)[supply_used_string_id + 1];
			} else {
				supply_used_string = "";
			}
			sprintf_s(g_supply_used_buffer, 260, "%s %d", supply_used_string, (supply_used + 1) / 2);

			u32 supply_provided_string_id = race + 814;
            char* supply_provided_string = 0;
			if (race == -815) {
				supply_provided_string = 0;
			} else if (supply_provided_string_id < **g_stat_txt_buffer) {
				supply_provided_string = (char*)*g_stat_txt_buffer + (*g_stat_txt_buffer)[supply_provided_string_id + 1];
			} else {
				supply_provided_string = "";
			}
			u32 last_queue_slot_type = helper::get_last_queue_slot_id(unit);
			sprintf_s(g_supply_provided_buffer, 260, "%s %d", supply_provided_string, (units_dat::supply_provided[last_queue_slot_type] + 1) / 2);
			
            u32 supply_available_string_id = race + 66353;
            char* supply_available_string = 0;
			if (race == -818) {
				supply_available_string = 0;
			} else if (supply_available_string_id < **g_stat_txt_buffer) {
				supply_available_string = (char*)*g_stat_txt_buffer + (*g_stat_txt_buffer)[supply_available_string_id + 1];
			} else {
				supply_available_string = "";
			}
			u32 supply_available = get_supplies_available(unit->player, race);
			sprintf_s(g_supply_available_buffer, 260, "%s %d", supply_available_string, (supply_available + 1) / 2);

            u32 supply_max = g_supplies_max_terran[unit->player];
			u32 supply_max_string_id = race + 823;
            char* supply_max_string = 0;
			if (race == -824) {
				supply_max_string = 0;
			} else if (supply_max_string_id < **g_stat_txt_buffer) {
				supply_max_string = (char*)*g_stat_txt_buffer + (*g_stat_txt_buffer)[supply_max_string_id + 1];
			} else {
				supply_max_string = "";
			}
			sprintf_s(g_supply_max_buffer, 260, "%s %d", supply_max_string, (supply_max + 1) >> 1);

			helper::add_text_to_dialog(dialog, -10, g_supply_used_buffer);
			helper::add_text_to_dialog(dialog, -11, g_supply_provided_buffer);
			helper::add_text_to_dialog(dialog, -12, g_supply_available_buffer);
			helper::add_text_to_dialog(dialog, -13, g_supply_max_buffer);
            */

            BinDlg* child = dialog->control_type ? dialog->parent->child_dialog : dialog->child_dialog;
            while (child) {
                helper::hide_dialog(child);
                child = child->next;
            }

            helper::draw_armor(dialog, unit->id, 0);
			if (units_dat::shields_enabled[unit->id]) {
				helper::draw_shield_upgrade(dialog, 1);
			}
		} else {
			helper::unit_stat_act_standard(dialog);
		}
	}

    void increment_unit_scores(Unit* unit, s32 count) // 00488BF0
    {
        if (unit->status & UnitStatus::IsHallucination)
        {
            return;
        }
        if (units_dat::base_property[unit->id] & UnitProperty::Subunit)
        {
            return;
        }
        g_all_unit_count_table[unit->id].player[unit->player] += count;
        GroupFlag groupFlag = units_dat::group_flags[(u16)unit->id];
        
        if (unit->get_race() == RaceId::Zerg)
        {
            u8 supply;
            // zerg special handling
            if (unit->id != UnitId::ZergEgg && unit->id != UnitId::ZergCocoon)
            {
                supply = units_dat::supply_required[unit->id];
                if (!(units_dat::base_property[unit->id] & 0x400) || unit->status & 1)
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
                s32 queueUnit = unit->build_queue[unit->build_queue_slot];
                supply = units_dat::supply_required[queueUnit];
                if (units_dat::base_property[queueUnit] & UnitProperty::TwoUnitsIn1Egg)
                {
                    supply *= 2;
                }

                raceSupply[RaceId::Terran].used[unit->player] += count * supply;
            }
        }
        else
        {
            raceSupply[RaceId::Terran].used[unit->player] += count * units_dat::supply_required[unit->id];
        }

        if (groupFlag.is_factory)
        {
            factories_produced_owned_total[unit->player] += count;
        }
        if (groupFlag.is_men)
        {
            units_produced_owned_total[unit->player] += count;
        }
        if (groupFlag.is_building)
        {
            structures_produced_owned_total[unit->player] += count;
        }
        if (unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon)
        {
            units_produced_owned_total[unit->player] += count;
        }

        if (g_all_unit_count_table[unit->id].player[unit->player] < 0)
        {
            g_all_unit_count_table[unit->id].player[unit->player] = 0;
        }
    }

    void increment_unit_scores_ex(s32 count, Unit* unit, s32 increment_scores) // 00488D50
    {
        UnitExt* unitExt = get_unit_ext(unit);

        if (unit->status & UnitStatus::IsHallucination)
        {
            return;
        }
        if (units_dat::base_property[unit->id] & UnitProperty::Subunit)
        {
            return;
        }
        auto player = unit->player;
        auto unit_id = unit->id;
        g_completed_unit_counts[unit->id].player[unit->player] += count;

        s32 race = unit->get_race();
        s32 supply = unit->get_supply_provided();
        if (race <= RaceId::Protoss)
        {
            raceSupply[RaceId::Terran].provided[player] += count * supply;
        }
        if (units_dat::group_flags[unit_id].is_factory)
        {
            factories_owned[unit->player] += count;
        }
        if (units_dat::group_flags[unit_id].is_building)
        {
            buildings_owned[unit->player] += count;
        }
        if (units_dat::group_flags[unit_id].is_men)
        {
            units_produced[unit->player] += count;
        }
        if (increment_scores != 0)
        {
            if (units_dat::group_flags[unit_id].is_men)
            {
                if (!scbw::is_any_morph_result(unit_id))
                {
                    units_owned[unit->player] += count;
                }
                unit_score[unit->player] += count;
            }
            else if (units_dat::group_flags[unit_id].is_building)
            {
                if (!scbw::is_zerg_building_morph_result(unit_id))
                {
                    buildings_owned[unit->player] += count;
                }
                building_score[unit->player] += count * units_dat::build_score[unit_id];
            }
        }
        if (g_completed_unit_counts[unit_id].player[unit->player] < 0)
        {
            g_completed_unit_counts[unit_id].player[unit->player] = 0;
        }
    }

    // Check if @p player has enough supplies to build @p unit_id.
    bool has_supplies_for_unit(u8 player, u16 unit_id, bool canShowErrorMessage)
    {
        // Default StarCraft behavior
        s32 supplyCost = units_dat::supply_required[unit_id];
        if (units_dat::base_property[unit_id] & UnitProperty::TwoUnitsIn1Egg)
        {
            supplyCost *= 2;
        }
        if (unit_id == UnitId::ZergLakizilisk)
        {
            supplyCost -= units_dat::supply_required[UnitId::ZergHydralisk];
        }

        g_ai_supply_reserved[player] = supplyCost;
        // No supply cost check needed
        if (supplyCost == 0 || units_dat::base_property[unit_id] & UnitProperty::MorphFromOtherUnit)
        {
            return true;
        }
        u32 raceId = g_player_table[player].race;
        const u32 supplyUsed = raceSupply[RaceId::Terran].used[player];
        // Must construct additional pylons
        if (supplyUsed + supplyCost > raceSupply[RaceId::Terran].max[player])
        {
            if (canShowErrorMessage)
            {
                scbw::show_error_message_with_sfx(player, 847 + raceId, 1 + raceId);
            }
            return false;
        }
        // Supply limit exceeded
        if (supplyCost > scbw::get_supply_remaining(player, RaceId::Terran))
        {
            if (canShowErrorMessage)
            {
                scbw::show_error_message_with_sfx(player, 844 + raceId, 153 + raceId);
            }
            return false;
        }
        return true;
    }
}