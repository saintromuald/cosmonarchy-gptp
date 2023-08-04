#include "supply_hooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>
#include <globals.h>
#include "trigger_system/trigger_system.h"
#include "config.h"
#include "player_color.h"
#include "hooks/interface/uifeatures/ui_features.h"

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
		u32 supply_available = 0;
		u32 supply_used = 0;
		if (hooks::get_supplies_max(unit_id, player) >= g_supplies_available_terran[player])
		{
			supply_used = g_supplies_used_terran[player];
			supply_available = g_supplies_available_terran[player];
		}
		else
		{
			supply_used = g_supplies_used_terran[player];
			supply_available = hooks::get_supplies_max(unit_id, player);
		}
		if (*g_cheat_state & 0x20000) // Food for Thought
		{
			return g_supplies_max_terran[player] - supply_used;
		}
		return supply_available - supply_used;
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

		// Let's reset what it originally did, since it cleared more than just g_all_unit_count_table
		memset(g_all_unit_count_table_original, 0, 0xAB00u);
		
		// And let's reset the new tables too
		memset(g_completed_unit_count_table,      0, UnitsIdLimit * PLAYER_COUNT * sizeof(u32));
		memset(g_all_unit_count_table,            0, UnitsIdLimit * PLAYER_COUNT * sizeof(u32));
		memset(g_unit_killed_table,               0, UnitsIdLimit * PLAYER_COUNT * sizeof(u32));
		memset(g_unit_deaths_table,               0, UnitsIdLimit * PLAYER_COUNT * sizeof(u32));
		memset(g_unit_availability->available[0], 1, UnitsIdLimit * PLAYER_COUNT * sizeof(u8));

		s32 result = 0;
		if (*g_multiplayer_mode && game_data->teams_enabled)
		{
			s32 force_iterator = 1;
			do
			{
				result = unknown_57F41CB[(u8)force_iterator];
				u8 iterator_result = unknown_57F41CB[(u8)force_iterator];
				if ((s8)result != -1)
				{
					s32 supply_max1 = 0;
					s32 supply_max2 = 0;
					s32 supply_max3 = 0;

					if (400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 0) >= 400)
					{
						supply_max1 = 400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 0);
					}
					else
					{
						supply_max1 = 400;
					}

					supply_max2 = iterator_result;
					g_supplies_max_terran[supply_max2] = supply_max1;
					if (400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 1) >= 400)
					{
						supply_max3 = 400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 1);
					}
					else
					{
						supply_max3 = 400;
					}

					g_supplies_max_terran[supply_max2] += supply_max3;
					if (400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 2) >= 400)
					{
						result = 400 * (u8)helper::team_game_count_races(force_iterator, (char*)0x190, 2);
					}
					else
					{
						result = 400;
					}

					g_supplies_max_terran[supply_max2] += result;
				}
				++force_iterator;
			}
			while (force_iterator <= 4);
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
		if (!*g_is_in_replay && (u8)(*g_active_portrait_unit)->player != *g_local_nation_id)
		{
			return helper::unit_stat_cond_standard();
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
            factories_produced_owned_total[unit->player] += count;
        }
        if (group_flags.is_men) {
            units_produced_owned_total[unit->player] += count;
        }
        if (group_flags.is_building) {
            structures_produced_owned_total[unit->player] += count;
        }
        if (unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon) {
            units_produced_owned_total[unit->player] += count;
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
            factories_owned[unit->player] += count;
        }
        if (units_dat::group_flags[unit_id].is_building) {
            buildings_owned[unit->player] += count;
        }
        if (units_dat::group_flags[unit_id].is_men) {
            units_produced[unit->player] += count;
        }
        if (increment_scores) {
            if (units_dat::group_flags[unit_id].is_men) {
                if (!scbw::is_any_morph_result(unit_id))  {
                    units_owned[unit->player] += count;
                }
                unit_score[unit->player] += count;
            } else if (units_dat::group_flags[unit_id].is_building)  {
                if (!scbw::is_zerg_building_morph_result(unit_id)) {
                    buildings_owned[unit->player] += count;
                }
                building_score[unit->player] += count * units_dat::build_score[unit_id];
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
                scbw::show_error_message_with_sfx(player, 847 + race, 1 + race);
            }
            return false;
        }
        // Supply limit exceeded
        if (supply_cost > scbw::get_supply_remaining(player, RaceId::Terran)) {
            if (can_show_error) {
                scbw::show_error_message_with_sfx(player, 844 + race, 153 + race);
            }
            return false;
        }
        return true;
    }

	static void draw_player_stats(BinDlg** dialog_ptr, Point16* pos, u32 player, char* color, bool can_show_apm, bool show_name, bool show_color_box) {
		// Track worker/military counts. -Veeq7
		u32 workers = 0;
		u32 men = 0;
		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			if (unit->status & UnitStatus::Completed) {
				if (units_dat::base_property[unit->id] & UnitProperty::Worker) {
					workers += 2;
				} else if (units_dat::group_flags[unit->id].is_men && !scbw::unit_is_trap(unit) && unit->id != UnitId::ProtossSteward) {
					men += 2;
				}
			}
		}

		helper::show_supply_info(*dialog_ptr, g_player_table[player].race + 4, workers, men);
		*dialog_ptr = (*dialog_ptr)->next;
		helper::print_stat_res_info(*dialog_ptr, g_player_table[player].race + 1, &StatRes_accrue_gas[player], resources->gas[player], FALSE);
		*dialog_ptr = (*dialog_ptr)->next;
		helper::print_stat_res_info(*dialog_ptr, 0, &StatRes_accrue_minerals[player], resources->minerals[player], FALSE);
		*dialog_ptr = (*dialog_ptr)->next;

		Point16 local_pos = *pos;
		if (show_color_box) {
			local_pos.x -= 2;
			graphics::draw_statres_box(local_pos.x + 4, local_pos.y + 2, local_pos.x + 13, local_pos.y + 11, g_player_minimap_colors[player]);
		}
		local_pos.x -= 2;

		bool show_apm = can_show_apm && g_player_table[player].type == PlayerType::Human;
		if (show_name || show_apm) {
			char buffer[64] = {};
			u32 apm = player_apm_counter[player].action_count / (ActionSampleTime / 60.0);
			if (show_name && show_apm) {
				sprintf_s(buffer, "%s%s | %u", color, player_data.full_player_names[player].data, apm);
			} else if (show_apm) {
				sprintf_s(buffer, "%s%u", color, apm);
			} else {
				sprintf_s(buffer, "%s%s", color, player_data.full_player_names[player].data);
			}
			u32 width = font_base[graphics::FONT_MEDIUM]->getTextWidth(buffer);
			graphics::draw_statres_text(local_pos.x - width, local_pos.y, buffer, graphics::FONT_MEDIUM);
		}

		pos->y += 15;

		// keeping this 4th dialog box for later, if i ever manage to use the native DrawBox function for it instead of graphics::draw_filled_box. -DF
		helper::hide_dialog(*dialog_ptr);
		*dialog_ptr = (*dialog_ptr)->next;
	}

	// Hook edited by DarkenedFantasies for new replay UI. -Pr0nogo
	void stat_res_main_proc(BinDlg* parent_dialog) { // 004E5640
		if (parent_dialog->control_type) {
			parent_dialog = parent_dialog->parent;
		}

		BinDlg* dialog = parent_dialog->child_dialog;
		if (dialog) {
			while (dialog->index != 1) {
				dialog = dialog->next;

				if (!dialog) {
					dialog = 0;
					break;
				}
			}
		} else {
			dialog = 0;
		}

		// Replay/witness UI. -DF
		Point16 pos = { 420, 3 };
		if (*g_is_in_replay || globals.players[*g_local_nation_id].witness_mode) {
			graphics::reset_stat_res_graphics();

			bool can_show_apm = false;
			if (config.witness_show_apm) {
				for (u8 player = 0; player < PlayerLimit; ++player) {
					if (g_player_table[player].type == PlayerType::Human && !globals.players[player].witness_mode) {
						can_show_apm = true;
						break;
					}
				}
			}

			for (u8 player = 0; player < PlayerLimit; ++player) {
				if (g_player_table[player].type != PlayerType::Computer && g_player_table[player].type != PlayerType::Human) {
					continue;
				}
				if (globals.players[player].witness_mode) {
					continue;
				}
				draw_player_stats(&dialog, &pos, player, scbw::get_player_color_code(player, false).data, config.witness_show_apm, true, true);
			}

			while (dialog && dialog->index <= 32) {
				helper::hide_dialog(dialog);
				dialog = dialog->next;
			}
		} else { // Normal player UI. -Veeq7
			graphics::reset_stat_res_graphics();

			bool can_show_apm = config.show_apm;
			u32 local_player = *g_local_nation_id;
			for (u8 player = 0; player < PlayerLimit; ++player) {
				if (g_player_table[player].type == PlayerType::Human && !globals.players[player].witness_mode && player != local_player) {
					can_show_apm = true;
					break;
				}
			}

			draw_player_stats(&dialog, &pos, local_player, BWTC_GREEN, config.show_team_ui_self && config.show_team_ui_apm, config.show_team_ui_self && config.show_team_ui_names, config.show_team_ui_self);
			if (config.show_team_ui) {
				for (u8 player = 0; player < PlayerLimit; ++player) {
					if (g_player_table[player].type != PlayerType::Computer && g_player_table[player].type != PlayerType::Human) {
						continue;
					}
					if (globals.players[player].witness_mode) {
						continue;
					}
					if (player == local_player || !scbw::is_allied_to(player, local_player)) {
						continue;
					}
					draw_player_stats(&dialog, &pos, player, scbw::get_player_color_code(player, true).data, config.show_team_ui_apm, config.show_team_ui_names, true);
				}
			}

			if (dialog->next) {
				do {
					if (dialog->index > 31) {
						break;
					}

					helper::hide_dialog(dialog);
					dialog = dialog->next;
				} while (dialog);
			}
		}
	}
}