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

char stat_res_resources_buffer[ResSize][16];
char stat_res_production_buffer[PlayerLimit][RowSize][4];
u32 stat_res_displayed_gas[PlayerLimit];
u32 stat_res_displayed_minerals[PlayerLimit];
ProductionArrays production;
graphics::Font* font_stat_res;
extern u8 tFontGame_ImageData[];

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
                helper::display_error_message_id_with_sfx(player, 847 + race, 1 + race);
            }
            return false;
        }
        // Supply limit exceeded
        if (supply_cost > scbw::get_supply_remaining(player, RaceId::Terran)) {
            if (can_show_error) {
				helper::display_error_message_id_with_sfx(player, 844 + race, 153 + race);
            }
            return false;
        }
        return true;
    }

	void show_supply_info(BinDlg* dialog, u16 graphic, u16 workers, u16 men) {
		if (!(dialog->flags & BinDlgFlags::Visible))
			helper::show_dialog(dialog);

		u32 user_value = (men << 16) | workers;

		if (dialog->non_user_value != user_value || dialog->graphic != graphic || !dialog->text) {
			dialog->non_user_value = user_value;
			dialog->graphic = graphic;
			char* buffer = &stat_res_resources_buffer[dialog->index - 1][0];
			sprintf_s(buffer, 16, "%d/%d",
				workers,
				men);
			dialog->text = buffer;
			u32 len = 0;
			while (buffer[++len]);
			dialog->unk_1c = (u32)font_base[len < 8 ? 1 : 0];

			if (!(dialog->flags & 1)) {
				dialog->flags |= 1;
				helper::update_dialog(dialog);
			}
		}
	}

	void print_stat_res_info(BinDlg* dialog, u16 graphic, u32* display_amount, u32 resource_amount, bool no_step_animation) {
		if (!(dialog->flags & BinDlgFlags::Visible))
			helper::show_dialog(dialog);

		if (!dialog->text)
			goto text_needs_update;

		if (resource_amount != *display_amount)
			goto uservalue_needs_update;

		if (dialog->non_user_value != resource_amount || dialog->graphic != graphic)
			goto dialog_needs_update;

		return;

	text_needs_update:
		if (resource_amount != *display_amount) {
		uservalue_needs_update:
			s32 difference = resource_amount - *display_amount;

			if (no_step_animation) {
				*display_amount += difference;
			} else {
				*display_amount += (((difference > 0) * 2) - 1) + (difference / 16);
			}
		}

	dialog_needs_update:
		dialog->non_user_value = *display_amount;
		dialog->graphic = graphic;
		char* buffer = &stat_res_resources_buffer[dialog->index - 1][0];
		sprintf_s(buffer, 16, "%d", *display_amount);
		dialog->text = buffer;
		u32 len = 0;
		while (buffer[++len]);
		dialog->unk_1c = (u32)font_base[len < 8 ? 1 : 0];

		if (!(dialog->flags & 1)) {
			dialog->flags |= 1;
			helper::update_dialog(dialog);
		}
	}

	void print_player_apm(BinDlg* dialog, u32 user_value) {
		if (!(dialog->flags & BinDlgFlags::Visible))
			helper::show_dialog(dialog);

		if (!dialog->text || dialog->non_user_value != user_value) {
			dialog->non_user_value = user_value;
			char* buffer = &stat_res_resources_buffer[dialog->index - 1][0];

			if ((s16)user_value >= 0) {
				sprintf_s(buffer, 16, "%d", (u16)user_value);
			} else {
				sprintf_s(buffer, 16, "CPU");
			}

			dialog->text = buffer;
			u32 len = 0;
			while (buffer[++len]);
			dialog->unk_1c = (u32)font_base[len < 5 ? 1 : 0];

			if (!(dialog->flags & 1)) {
				dialog->flags |= 1;
				helper::update_dialog(dialog);
			}
		}
	}

	u32 show_production_info(BinDlg* dialog, u8 player, u16 x_offset) {
		if (!dialog)
			return 0;

		u16 row_size = dialog->index | RowSize;
		u32 user_offset = x_offset << 8;
		u32 i_0;

		for (i_0 = 0; i_0 < RowSize; ++i_0) {
			u32 remaining_time = (u16)production.timers[player][i_0];

			if (remaining_time == 65535)
				break;

			if (!(dialog->flags & BinDlgFlags::Visible))
				helper::show_dialog(dialog);

			x_offset -= 15;
			u16 icon_graphic = production.types[player][i_0] + 12;
			u8 user_count = production.counts[player][i_0];
			u32 user_value = user_offset | user_count;

			if (dialog->non_user_value != user_value || dialog->graphic != icon_graphic) {
				dialog->non_user_value = user_value;
				dialog->graphic = icon_graphic;
				char* buffer = &stat_res_production_buffer[player][i_0][0];
				sprintf_s(buffer, 4, "%d", user_count);
				dialog->text = buffer;
				dialog->bounds.left = x_offset;
				dialog->bounds.right = x_offset + 14;

				if (!(dialog->flags & 1)) {
					dialog->flags |= 1;
					helper::update_dialog(dialog);
				}
			}

			u32 time_cost = (u16)units_dat::time_cost[production.types[player][i_0]];

			if (time_cost) {
				u32 x = (u16)dialog->bounds.left;
				u32 y = (u16)dialog->bounds.bottom;
				u32 progress = ((time_cost - remaining_time) * 15) / time_cost;

				if (progress) {
					graphics::add_statres_bar(x, --progress + x, y, 117);
				}
			}
			
			dialog = dialog->next;

			if (!dialog)
				goto loc_ret;
		}

		do {
			if (dialog->flags & BinDlgFlags::Visible)
				helper::hide_dialog(dialog);

			dialog = dialog->next;
		} while (dialog && dialog->index < row_size);

	loc_ret:
		return x_offset - (i_0 ? 2 : 0);
	}

	void print_player_name(BinDlg* dialog, u32 user_value) {
		if (!(dialog->flags & BinDlgFlags::Visible))
			helper::show_dialog(dialog);

		if (!dialog->text || dialog->non_user_value != user_value) {
			dialog->non_user_value = user_value;
			dialog->bounds.right = (u16)user_value;
			dialog->text = player_data.full_player_names[user_value >> 24].data;

			if (!(dialog->flags & 1)) {
				dialog->flags |= 1;
				helper::update_dialog(dialog);
			}
		}
	}

	static void draw_player_stats(BinDlg** dialog_ptr, Point16* pos, u32 player, bool use_player_color, bool can_show_apm, bool show_production, bool show_name, bool show_color_box, bool count_in_production) {
		// Track worker/military counts. -Veeq7
		u32 workers = 0;
		u32 men = 0;

		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			if (count_in_production || unit->status & UnitStatus::Completed) {
				u32 unit_id = scbw::is_zerg_egg(unit->id, true, false) ? unit->build_queue[unit->build_queue_slot] : unit->id;

				if (units_dat::base_property[unit_id] & UnitProperty::Worker) {
					++workers;
				} else if (units_dat::group_flags[unit_id].is_men && !skip_military_tracking_by_unit_id(unit_id) && !(units_dat::base_property[unit_id] & UnitProperty::Subunit)) {
					++men;
				}
			}
		}

		BinDlg* dialog = *dialog_ptr;
		BinDlg* dialog_name = dialog->initialVersion0x36.unk_3e;
		BinDlg* dialog_production = dialog->child_dialog;

		show_supply_info(dialog, g_player_table[player].race + 4, workers, men);
		dialog = dialog->next;
		print_stat_res_info(dialog, g_player_table[player].race + 1, &stat_res_displayed_gas[player], resources->gas[player], FALSE);
		dialog = dialog->next;
		print_stat_res_info(dialog, 0, &stat_res_displayed_minerals[player], resources->minerals[player], FALSE);
		dialog = dialog->next;
		Point16 local_pos = *pos;

		u32 user_value = (player << 24) | (use_player_color << 16);

		if (can_show_apm) {
			u16 display_value = g_player_table[player].type == PlayerType::Human ? player_apm_counter[player].action_count / (ActionSampleTime / 60.0) : 0x8000;
			print_player_apm(dialog, user_value | display_value);
			local_pos.x -= 33;
		} else if (dialog->flags & BinDlgFlags::Visible) {
			helper::hide_dialog(dialog);
		}

		if (show_color_box) {
			local_pos.x -= 12;
			graphics::add_statres_box(local_pos.x + 2, local_pos.y + 2, local_pos.x + 11, local_pos.y + 11, g_player_minimap_colors[player]);
		}

		if (show_production) {
			local_pos.x = show_production_info(dialog_production, player, local_pos.x);
		}

		if (show_name) {
			print_player_name(dialog_name, user_value | local_pos.x);
		} else if (dialog_name->flags & BinDlgFlags::Visible) {
			helper::hide_dialog(dialog_name);
		}

		pos->y += 15;
		*dialog_ptr = dialog->next;
	}

	// Hook edited by DarkenedFantasies for new replay UI. -Pr0nogo
	void __fastcall stat_res_main_proc(BinDlg* dialog) { // 004E5640
		if (dialog->control_type) {
			dialog = dialog->parent;
		}

		dialog = dialog->child_dialog;

		do {// This while loop is not necessary with a properly organized bin file, but keep it for safety. -DF
			if (!dialog)
				return;// The original function would set dialog to null and break out of the loop. Surely that can't be good. -DF

			if (dialog->index != 1) {
				dialog = dialog->next;
				continue;
			}

			break;
		} while (true);

		graphics::reset_stat_res_graphics();
		u32 local_player = *g_local_nation_id;
		Point16 pos = { 434, 3 };//x is dialog->bounds.left of the third row element (from the right), minus 2. -DF

		if (*g_is_in_replay || globals.players[local_player].witness_mode) {
			/*bool can_show_apm = false;

			if (config.witness_show_apm) {
				for (u32 player = 0; player < PlayerLimit; ++player) {
					if (g_player_table[player].type == PlayerType::Human && !globals.players[player].witness_mode) {
						can_show_apm = true;
						break;
					}
				}
			}*/

			for (u32 player = 0; player < PlayerLimit; ++player) {
				if ((g_player_table[player].type == PlayerType::Computer || g_player_table[player].type == PlayerType::Human)
				&& !globals.players[player].witness_mode) {
					draw_player_stats(&dialog, &pos, player,
						true,						//use_player_color
						config.witness_show_apm,	//can_show_apm
						true,						//show_production
						true,						//show_name
						true,						//show_color_box
						*g_is_holding_alt);			//count_in_production
				}
			}
		} else { // Normal player UI. -Veeq7
			/*bool can_show_apm = config.show_apm;

			for (u32 player = 0; player < PlayerLimit; ++player) {
				if (g_player_table[player].type == PlayerType::Human && player != local_player && !globals.players[player].witness_mode) {
					can_show_apm = true;
					break;
				}
			}*/

			draw_player_stats(&dialog, &pos, local_player,
				false,													//use_player_color
				config.show_team_ui_self && config.show_team_ui_apm,	//can_show_apm
				false,													//show_production
				config.show_team_ui_self && config.show_team_ui_names,	//show_name
				config.show_team_ui_self,								//show_color_box
				!*g_is_holding_alt);									//count_in_production

			if (config.show_team_ui) {
				for (u32 player = 0; player < PlayerLimit; ++player) {
					if ((g_player_table[player].type == PlayerType::Computer || g_player_table[player].type == PlayerType::Human)
					&& player != local_player
					&& !globals.players[player].witness_mode
					&& scbw::is_allied_to(player, local_player)) {
						draw_player_stats(&dialog, &pos, player,
							true,						//use_player_color
							config.show_team_ui_apm,	//can_show_apm
							false,						//show_production
							config.show_team_ui_names,	//show_name
							true,						//show_color_box
							!*g_is_holding_alt);		//count_in_production
					}
				}
			}
		}

		while (dialog->index <= ResSize) {
			if (dialog->flags & BinDlgFlags::Visible) {
				helper::hide_dialog(dialog);
				BinDlg* child = dialog->initialVersion0x36.unk_3e;//dialog_name

				if (child && child->flags & BinDlgFlags::Visible)
					helper::hide_dialog(child);

				child = dialog->child_dialog;//dialog_production

				if (child) {
					for (u32 i_0 = RowSize; i_0; --i_0) {
						if (child->flags & BinDlgFlags::Visible)
							helper::hide_dialog(child);

						child = child->next;
					}
				}
			}

			dialog = dialog->next;
		}
	}/*

	bool __fastcall stat_res_text_interact(BinDlg* dialog, DialogEvent* dlg_event) {
		if (dlg_event->number_order != 14) {
			return false;
		}

		switch (dialog->non_user_value) {
			case 0:
			case 6:
			case 8:
			case 10:
			case 13:
			case 14:
				return helper::generic_common_interact(dialog, dlg_event);
			default:
				return false;
		}
	}*/

	void __fastcall stat_res_text_update(BinDlg* dialog, int dummy0, int dummy1, int dummy2) {
		Bounds icon_area = {};
		FxnDrawIconData icon_data = {};
		GrpHead* icons_grp = *g_icons_grp;// For some reason, this exact order of initialization is critical for bw_print_icon() to work.

		if (dialog->graphic >= icons_grp->frame_count) {
			dialog->graphic = 0;
		}

		GrpFrame* cur_frame = &icons_grp->frames[dialog->graphic];
		s16 graphic_width = (u8)cur_frame->w;
		s16 graphic_height = (u8)cur_frame->h;
		s32 icon_index = cur_frame->data_offset;
		
		if (icon_index < 0) {
			icon_index &= 0x7FFFFFFF;
			graphic_width += 256;
		}

		icon_data.stat_icon = (GrpHead*)((char*)icons_grp + icon_index);
		icon_data.w = graphic_width;
		icon_area.width = graphic_width;
		icon_area.right = graphic_width - 1;
		icon_data.h = graphic_height;
		icon_area.height = graphic_height;
		icon_area.bottom = graphic_height - 1;
		icon_area.left = 0;
		icon_area.top = 0;

		s16 dialog_top = dialog->bounds.top;
		s16 dialog_height = dialog->bounds.bottom - dialog_top + 1;
		s16 dialog_left = dialog->bounds.left;
		helper::bw_print_icon(&icon_area, &icon_data, dialog_left, dialog_top + (dialog_height - graphic_height) / 2);

		if (dialog->text) {
			if (dialog->index < 64) {
				helper::set_font_handle((u32*)dialog->unk_1c);
				*g_print_xy_alignment = 0x21;
				helper::get_color_handle(GameTextColor::Green);
				dialog_left += graphic_width + 2;
			} else {
				helper::set_font_handle((u32*)font_stat_res);
				*g_print_xy_alignment = 0x44;
				*g_font_spacing = 0;
				helper::get_color_handle(GameTextColor::White);
			}

			*g_draw_text = { dialog->bounds.left, dialog->bounds.top, dialog->bounds.right, dialog->bounds.bottom };
			helper::bw_print_xy(dialog->text, dialog_left, dialog_top);
			helper::set_font_handle(NULL);
		}
	}

	void __fastcall stat_res_colortext_update(BinDlg* dialog, int dummy0, int dummy1, int dummy2) {
		if (dialog->text) {
			helper::set_font_handle((u32*)dialog->unk_1c);
			*g_draw_text = { dialog->bounds.left, dialog->bounds.top, dialog->bounds.right, dialog->bounds.bottom };
			*g_print_xy_alignment = (s16)dialog->index < 0 ? 0x24 : 0x21;
			*g_font_spacing = 1;

			if (dialog->non_user_value & 0x10000) {//use_player_color
				*g_saved_font_color = *g_current_font_color;
				*g_current_font_color = (u32)&tFontGame_ImageData[304];
				u32 player = dialog->non_user_value >> 24;
				u32 i_0 = 0;

				do {
					tFontGame_ImageData[305 + i_0] = g_player_in_game_colors[player * ColorSetSize + i_0 * 2];
				} while (++i_0 < 4);
			} else {
				helper::get_color_handle(GameTextColor::Green);
			}

			helper::bw_print_xy(dialog->text, dialog->bounds.left, dialog->bounds.top);
			helper::set_font_handle(NULL);
		}
	}

	void init_stat_res_font(char* font_name) {
		const u32 load_font = 0x0041FD10;

		__asm {
			PUSH	0x004DAB00//AllocateFont
			MOV		EAX, font_name
			CALL	load_font
			MOV		font_stat_res, EAX
		}
	}
	
	bool __fastcall stat_res_custom_ctrl(BinDlg* dialog, DialogEvent* dlg_event) {
		if (dlg_event->number_order != 14) {
			return false;
		}

		if (!dlg_event->user) {
			dialog->graphic = 0;
			dialog->user = 0;
			dialog->fxn_interact = (void*)0x00418E60;//stat_res_text_interact
			dialog->fxn_update = &stat_res_text_update;
		}

		if (dlg_event->number_order != 14) {
			return false;
		}

		switch (dlg_event->user) {
		case 0:
		case 6:
		case 8:
		case 10:
		case 13:
		case 14:
			return helper::generic_common_interact(dialog, dlg_event);
		default:
			return false;
		}
	}

	inline int esi() { __asm MOV EAX, ESI }

	void stat_res_register_user_dialog_action(BinDlg* dialog) {
		dialog = (BinDlg*)esi();

		if (dialog) {
			BinDlg* parent_offset = dialog + ResSize;

			do {
				u16 control_type = dialog->control_type;

				if (!control_type) {
					dialog = dialog->child_dialog;
					continue;
				}

				s16 index = dialog->index;

				if (index > 0) {
					if (index & 0xFFC3) {
						dialog->fxn_interact = &stat_res_custom_ctrl;

						//Custom pointers to that row's player name and production dialogs, written to the bin file manually ("unknown3" in PyBIN's text export).
						//It's not necessary to do it this way, but it's faster instead of stepping through the dialog->next pointers to get to them.
						//It's however unconventional for control types 1+ to have BinDlg pointers, hence the need to initialize it here.
						u32 child_index = dialog->non_user_value;

						if (child_index) {
							dialog->initialVersion0x36.unk_3e = child_index + parent_offset;
							dialog->initialVersion0x36.unk_3e->unk_1c = (u32)font_base[1];
							dialog->initialVersion0x36.unk_3e->fxn_update = &stat_res_colortext_update;
							dialog->child_dialog = (RowSize * --child_index) + parent_offset + PlayerLimit + 1;
						}
					} else {
						dialog->fxn_update = &stat_res_colortext_update;
					}
				}

				dialog = dialog->next;
			} while (dialog);
		}

		if (!font_stat_res) {
			init_stat_res_font("font_statres");
		}
	}

	const u32 update_units_ED127 = 0x004ED127;
	void __declspec(naked) stat_res_refresh_production_arrays() {
		__asm {
			PUSHAD
			MOV		EAX, 0xE400E4//UnitId::None
			MOV		EDI, offset production
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
			LEA		EDX, [EAX * 2 + production.types]
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
			INC		byte ptr [EAX + ECX + production.counts]//++production.counts[player_id][i_0];
			SHR		EBX, 16						//	(Restore remaining_build_time.)
			SHL		EAX, 1
			LEA		EDX, [EAX + ECX * 2 + production.timers]
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
}