#include "supply_hooks.h"
#include <BW/api.h>
#include <BW/structures.h>
#include <BW/bwdata.h>
#include <windows.h>
#include <globals.h>
#include <map>
#include "systems/triggers/trigger_system.h"
#include "config.h"
#include "player_color.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "hooks/interface/hud/hud.h"
#include "hooks/interface/mainmenu/glucustom.h"

namespace hooks {
u32 get_supplies_available(u8 player, s32 race) { // 00488900
    u32 result = g_supplies_max_terran[player];
    u32 suppliesAvailable = g_supplies_available_terran[player];
    if (suppliesAvailable < result) {
        result = suppliesAvailable;
    }
    return result;
}

s32 get_supplies_remaining(u8 player, u16 unit_id) { // 004889D0
    u32 supply_available = 0;
    u32 supply_used = 0;
    if (hooks::get_supplies_max(unit_id, player) >= g_supplies_available_terran[player]) {
        supply_used = g_supplies_used_terran[player];
        supply_available = g_supplies_available_terran[player];
    } else {
        supply_used = g_supplies_used_terran[player];
        supply_available = hooks::get_supplies_max(unit_id, player);
    }
    return supply_available - supply_used;
}

s32 get_supplies_max(u16 unit_id, u8 player) { // 00488990
    return g_supplies_max_terran[player];
}

s32 get_supplies_used(s32 race, u8 player) { // 004888C0
    return g_supplies_used_terran[player];
}

char initialize_unit_counts() { // 00488F90
    memset(g_unit_count_score, 0, 0x540u);

    // Let's reset what it originally did, since it cleared more than just g_all_unit_count_table
    memset(g_all_unit_count_table_original, 0, 0xAB00u);
        
    // And let's reset the new tables too
    memset(g_completed_unit_count_table,      0, UnitsIdLimit * PLAYER_COUNT * sizeof(u32));
    memset(g_all_unit_count_table,            0, UnitsIdLimit * PLAYER_COUNT * sizeof(u32));
    memset(g_unit_killed_table,               0, UnitsIdLimit * PLAYER_COUNT * sizeof(u32));
    memset(g_unit_deaths_table,               0, UnitsIdLimit * PLAYER_COUNT * sizeof(u32));
    memset(g_unit_availability->available[0], 1, UnitsIdLimit * PLAYER_COUNT * sizeof(u8));

    s32 result = 0;
    if (g_multiplayer_mode && game_data->teams_enabled) {
        s32 force_iterator = 1;
        do
        {
            result = unknown_57F41CB[(u8)force_iterator];
            u8 iterator_result = unknown_57F41CB[(u8)force_iterator];
            if ((s8)result != -1) {
                s32 supply_max1 = 0;
                s32 supply_max2 = 0;
                s32 supply_max3 = 0;

                if (400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 0) >= 400) {
                    supply_max1 = 400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 0);
                } else {
                    supply_max1 = 400;
                }

                supply_max2 = iterator_result;
                g_supplies_max_terran[supply_max2] = supply_max1;
                if (400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 1) >= 400) {
                    supply_max3 = 400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 1);
                } else {
                    supply_max3 = 400;
                }

                g_supplies_max_terran[supply_max2] += supply_max3;
                if (400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 2) >= 400) {
                    result = 400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 2);
                } else {
                    result = 400;
                }

                g_supplies_max_terran[supply_max2] += result;
            }
            ++force_iterator;
        }
        while (force_iterator <= 4);
    } else {
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

s32 get_supplies_temp() { // 004246D0
    Unit* activeUnit = *g_active_portrait_unit;
    GroupFlag groupFlags = units_dat::group_flags[activeUnit->id];
    g_supplies_available_temp = get_supplies_available(activeUnit->player, RaceId::Terran);
    u8 player = activeUnit->player;
    s32 result = g_supplies_used_terran[player];
    *g_supplies_used_temp = g_supplies_used_terran[player];
    return result;
}

bool redraw_supply_info() { // 00425230 // Guessed name
    if (!g_is_in_replay && (u8)(*g_active_portrait_unit)->player != g_local_nation_id) {
        return helper::unit_stat_cond_standard();
    }

    u8 activePlayer = (*g_active_portrait_unit)->player;
    GroupFlag groupFlags = units_dat::group_flags[(u16)(*g_active_portrait_unit)->id];
    char race = RaceId::Terran;

    if ((u16)g_supplies_available_temp == get_supplies_available(activePlayer, race)) {
        s32 suppliesVar = g_supplies_used_terran[activePlayer];
        if ((u16)g_supplies_used_temp == suppliesVar) {
            return false;
        }
    }
    return true;
}

void draw_supply_unit_status(BinDlg* dialog) {
    Unit* unit = *g_active_portrait_unit;
    if (g_is_in_replay || unit->player == g_local_nation_id) {
        get_supplies_temp();
        if (g_status_screen_func != 12) {
            helper::hide_optional_status_screen_controls(dialog);
            helper::show_control_group(dialog, -10, -13);
            g_status_screen_func = 12;
        }

        BinDlg* child = dialog->control_type != DialogControlType::DialogBoxDC ? dialog->parent->child_dialog : dialog->child_dialog;
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

void increment_unit_scores(Unit* unit, s32 count) { // 00488BF0
    if (unit->status & UnitStatus::IsHallucination) {
        return;
    }
    if (units_dat::base_property[unit->id] & UnitProperty::Subunit) {
        return;
    }
    g_all_unit_count_table[unit->id].player[unit->player] += count;
    if (unit->get_race() == RaceId::Zerg) {
        // Zerg special handling
        if (unit->id != UnitId::ZergEgg && unit->id != UnitId::ZergCocoon) {
            u8 supply = units_dat::supply_required[unit->id];
            if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg && !(unit->status & UnitStatus::Completed)) {
                g_race_supply[RaceId::Terran].used[unit->player] += count * supply * 2;
            } else {
                g_race_supply[RaceId::Terran].used[unit->player] += count * supply;
            }
        } else {
            s32 queue_unit_id = unit->build_queue[unit->build_queue_slot];
            u8 supply = units_dat::supply_required[queue_unit_id];
            if (units_dat::base_property[queue_unit_id] & UnitProperty::TwoUnitsIn1Egg) {
                supply *= 2;
            }
            g_race_supply[RaceId::Terran].used[unit->player] += count * supply;
        }
    } else {
        g_race_supply[RaceId::Terran].used[unit->player] += count * units_dat::supply_required[unit->id];
    }

    GroupFlag group_flags = units_dat::group_flags[unit->id];
    if (group_flags.is_factory) {
        g_factories_produced_owned_total[unit->player] += count;
    }
    if (group_flags.is_men) {
        g_units_produced_owned_total[unit->player] += count;
    }
    if (group_flags.is_building) {
        g_structures_produced_owned_total[unit->player] += count;
    }
    if (unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon) {
        g_units_produced_owned_total[unit->player] += count;
    }

    if (g_all_unit_count_table[unit->id].player[unit->player] < 0) {
        g_all_unit_count_table[unit->id].player[unit->player] = 0;
    }
}

void increment_unit_scores_ex(s32 count, Unit* unit, s32 increment_scores) { // 00488D50
    UnitExt* unit_ext = get_unit_ext(unit);
    if (unit->status & UnitStatus::IsHallucination) {
        return;
    }
    if (units_dat::base_property[unit->id] & UnitProperty::Subunit) {
        return;
    }
    u32 player = unit->player;
    u32 unit_id = unit->id;
    g_completed_unit_count_table[unit->id].player[unit->player] += count;

    u32 race = unit->get_race();
    if (race <= RaceId::Protoss) {
        g_race_supply[RaceId::Terran].provided[player] += count * unit->get_supply_provided();
    }
    if (units_dat::group_flags[unit_id].is_factory) {
        g_factories_owned[unit->player] += count;
    }
    if (units_dat::group_flags[unit_id].is_building) {
        g_buildings_owned[unit->player] += count;
    }
    if (units_dat::group_flags[unit_id].is_men) {
        g_units_produced[unit->player] += count;
    }
    if (increment_scores) {
        if (units_dat::group_flags[unit_id].is_men) {
            if (!bw::is_any_morph_result(unit_id))  {
                g_units_owned[unit->player] += count;
            }
            g_unit_score[unit->player] += count;
        } else if (units_dat::group_flags[unit_id].is_building)  {
            if (!bw::is_zerg_building_morph_result(unit_id)) {
                g_buildings_owned[unit->player] += count;
            }
            g_building_score[unit->player] += count * units_dat::build_score[unit_id];
        }
    }
    if (g_completed_unit_count_table[unit_id].player[unit->player] < 0)  {
        g_completed_unit_count_table[unit_id].player[unit->player] = 0;
    }
}

// Check if @p player has enough supplies to build @p unit_id.
bool has_supplies_for_unit(u8 player, u16 unit_id, bool can_show_error) {
    // Default StarCraft behavior
    s32 supply_cost = units_dat::supply_required[unit_id];
    if (units_dat::base_property[unit_id] & UnitProperty::TwoUnitsIn1Egg) {
        supply_cost *= 2;
    }
    if (unit_id == UnitId::ZergLakizilisk) {
        supply_cost -= units_dat::supply_required[UnitId::ZergHydralisk];
    }

    g_ai_supply_reserved[player] = supply_cost;
    // No supply cost check needed
    if (supply_cost == 0 || units_dat::base_property[unit_id] & UnitProperty::MorphFromOtherUnit) {
        return true;
    }
    u32 race = g_player_table[player].race;
    const u32 supply_used = g_race_supply[RaceId::Terran].used[player];
    // Must construct additional pylons
    if (supply_used + supply_cost > g_race_supply[RaceId::Terran].max[player]) {
        if (can_show_error) {
            helper::display_error_message_id_with_sfx(player, 847 + race, 1 + race);
        }
        return false;
    }
    // Supply limit exceeded
    if (supply_cost > bw::get_supply_remaining(player, RaceId::Terran)) {
        if (can_show_error) {
            helper::display_error_message_id_with_sfx(player, 844 + race, 153 + race);
        }
        return false;
    }
    return true;
}

const u32 update_units_ED127 = 0x004ED127;
void __declspec(naked) stat_res_refresh_production_arrays() {
    __asm {
        PUSHAD
        MOV		EAX, 0xE400E4//UnitId::None
        MOV		EDI, offset g_production
        //production.types
        MOV		ECX, 64//(PlayerLimit * RowSize * 2) / 4, or (sizeof(production.types) / 4)
        REP STOSD
        MOV		EAX, -1
        //production.timers
        MOV		ECX, 64//(PlayerLimit * RowSize * 2) / 4, or (sizeof(production.timers) / 4)
        REP STOSD
        XOR		EAX, EAX
        //production.counts
        MOV		ECX, 32//(PlayerLimit * RowSize) / 4, or (sizeof(production.counts) / 4)
        REP STOSD
        POPAD
        RETN
    }
}

void __declspec(naked) __fastcall stat_res_get_production(Unit* unit) {
    __asm {
        MOV		AL, [ECX + 0x4C]
        CMP		AL, 8						//	if (unit->player >= PlayerLimit)
        JB		offset _isactive			//		return;
        RETN

        _isactive:
        PUSH	EBX
        PUSH	EDI
        MOV		EDI, ECX
        MOV		EDX, [EDI + 0xEC]
        TEST	EDX, EDX
        JE		offset _checktime
        MOV		CL, [EDI + 0xA6]
        CMP		CL, 37
        JE		offset _checktime			//	if (unit->current_build_unit && unit->secondary_order_id != OrderId::BuildAddon)
        MOV		EDI, EDX					//		unit = unit->current_build_unit;

        _checktime:
        MOVZX	EBX, word ptr [EDI + 0xAC]	//	u16 remaining_build_time = unit->remaining_build_time;
        TEST	EBX, EBX
        JE		offset _ret
        MOV		CL, [EDI + 0xDC]
        TEST	CL, 1						//	if (!remaining_build_time || unit->status & UnitStatus::Completed)
        JNE		offset _ret					//		return;
        SHL		EBX, 16
        MOVZX	ECX, byte ptr [EDI + 0xA4]
        MOV		BX, [EDI + ECX + 0x98]		//	u16 unit_id = unit->build_queue[unit->build_queue_slot];
        CMP		BX, 228
        JNE		offset _hasqueue			//	if (unit_id == UnitId::None) {
        MOV		CL, [EDI + 0x4D]
        CMP		CL, 41						//		if (unit->main_order_id == OrderId:ZergBirth)
        JE		offset _ret					//			return;
        MOV		BX, [EDI + 0x64]			//		unit_id = unit->id;
                                            //	}
        _hasqueue:
        CMP		BX, 73						//	if (unit_id == UnitId::ProtossSteward)
        JE		offset _ret					//		return;
        MOVZX	EAX, AL						//	u32 player_id = (u8)unit->player;
        SHL		EAX, 4
        LEA		EDX, [EAX * 2 + g_production.types]
        XOR		ECX, ECX					//	u32 i_0 = 0;

        _loop:								//	do {
        CMP		[EDX + ECX * 2], BX			//		if (production.types[player_id][i_0] == unit_id)
        JE		offset _addCount			//			break;
        CMP		word ptr [EDX + ECX * 2], 228//		if (production.types[player_id][i_0] != UnitId::None) {
        JE		offset _addType				//			production.types[player_id][i_0] = unit_id;
        INC		ECX							//			break;
        CMP		ECX, RowSize				//		}
        JNE		offset _loop				//	} while (++i_0 != RowSize);
        JMP		offset _ret					//	if (i_0 >= 8) return;

        _addType:
        MOV		[EDX + ECX * 2], BX

        _addCount:
        INC		byte ptr [EAX + ECX + g_production.counts]//++production.counts[player_id][i_0];
        SHR		EBX, 16						//	(Restore remaining_build_time.)
        SHL		EAX, 1
        LEA		EDX, [EAX + ECX * 2 + g_production.timers]
        MOV		AX, [EDX]
        CMP		BX, AX
        JAE		offset _ret					//	if (remaining_build_time < production.timers[player_id][i_0])
        MOV		[EDX], BX					//		production.timers[player_id][i_0] = remaining_build_time;

        _ret:
        POP		EDI
        POP		EBX
        RETN
    }
}
} // namespace hooks