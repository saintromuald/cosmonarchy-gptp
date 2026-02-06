#include "CMDRECV_Build.h"
#include <BW/api.h>
#include "trigger_system/trigger_system.h"
#include "hooks\limits\sfx_extender.h"

namespace hooks {
	bool cmdrecv_place_building_allowed(Unit* builder, u8 order_id, u16 built_unit_id) { // 0048DBD0
		bool is_allowed = false;
		if (order_id >= OrderId::DroneStartBuild && order_id <= OrderId::CTFCOP2) {
			if (order_id == OrderId::DroneStartBuild
			||	order_id == OrderId::BuildTerran
			||	order_id == OrderId::BuildProtoss1
			||	order_id == OrderId::BuildProtoss2
			||	order_id == OrderId::PlaceAddon)
			{
				if (builder->can_make_unit(built_unit_id, *g_active_nation_id)) {
					is_allowed = (helper::order_is_allowed(builder, order_id, *g_active_nation_id) != 0);
				}
			} else if (order_id == OrderId::BuildNydusExit || order_id == OrderId::BuildingLand) {
				if (builder->id == built_unit_id) {
					is_allowed = (helper::order_is_allowed(builder, order_id, *g_active_nation_id) != 0);
				}
			} else if (order_id == OrderId::CTFCOP2) {
				if (*g_elapsed_time_seconds <= 600 && builder->id == built_unit_id) {
					is_allowed = (helper::order_is_allowed(builder, order_id, *g_active_nation_id) != 0);
				}
			}
		}
		return is_allowed;
	}

	void cmdrecv_place_building_normal(s16 x, s16 y, u32 order_id, u32 built_unit_id) { // 0048E010
		*g_unit_iterator_pos = 0;
		Unit* builder = get_active_player_next_selection();
		u32 can_build = helper::tile_location_is_unbuildable(builder, builder->player, x, y, built_unit_id, 1, 0, 0, 0);

		if (helper::place_building_message(can_build)) {
			s32 dimension_x = (s16)units_dat::building_dimensions[built_unit_id].x;
			if (dimension_x < 0) {
				dimension_x++;
			}
			dimension_x /= 2;
			dimension_x += (x * 32);
			s32 dimension_y = (s16)units_dat::building_dimensions[built_unit_id].y;
			if (dimension_y < 0) {
				dimension_y++;
			}
			dimension_y /= 2;
			dimension_y += (y * 32);
			helper::process_building_placement(builder, order_id, dimension_x, dimension_y, built_unit_id);
		}
	}

	void cmdrecv_place_addon(s16 x, s16 y, u32 order_id, u32 built_unit_id) { // 0048E0A0
		u16 addon_place_x = 0;
		u16 addon_place_y = 0;
		*g_unit_iterator_pos = 0;
		Unit* builder = get_active_player_next_selection();
		s32 can_build = helper::tile_location_is_unbuildable(builder, builder->player, x, y, built_unit_id, 1, 0, 0, 0);
		if (!can_build) {
			addon_place_x = (s16)units_dat::addon_placement[built_unit_id - UnitId::TerranMinistry].x;
			bw::adjust_addon_x(built_unit_id, builder->id, &addon_place_x);
			if (addon_place_x < 0) {
				addon_place_x += 31;
			}
			addon_place_x /= 32;
			addon_place_x = x - addon_place_x;
			addon_place_y = (s16)units_dat::addon_placement[built_unit_id - UnitId::TerranMinistry].y;
			bw::adjust_addon_y(built_unit_id, builder->id, &addon_place_y);
			if (addon_place_y < 0) {
				addon_place_y += 31;
			}
			addon_place_y /= 32;
			addon_place_y = y - addon_place_y;
			can_build = helper::tile_location_is_unbuildable(builder, builder->player, addon_place_x, addon_place_y, builder->id, 0, 0, 0, 0);
		}

		if (helper::place_building_message(can_build)) {
			s32 dim_x, dim_y;
			dim_x = (s16)units_dat::building_dimensions[builder->id].x;
			if (dim_x < 0) {
				dim_x++;
			}
			dim_x /= 2;
			addon_place_x *= 32;
			dim_x += addon_place_x;
			dim_y = (s16)units_dat::building_dimensions[builder->id].y;
			if (dim_y < 0) {
				dim_y++;
			}
			dim_y /= 2;
			addon_place_y *= 32;
			dim_y += addon_place_y;
			helper::process_building_placement(builder, order_id, dim_x, dim_y, built_unit_id);
		}
	}

	void cmdrecv_build(u8 order_id, s16 x, s16 y, u16 built_unit_id) { // 004C23C0
		*g_unit_iterator_pos = 0;
		Unit* builder = get_active_player_next_selection();
		if (builder) {
			if (!get_active_player_next_selection()) { // multiple selection not allowed 
				if (x < g_map_tile_size->width && y < g_map_tile_size->height && helper::CMDRECV_place_building_allowed(builder, order_id, built_unit_id)) {
					helper::CMDRECV_place_building(x, y, order_id, built_unit_id);
				}
			}
		}
	}

	// Hooked to try to allow for queueable build orders. -Pr0nogo
	// Not injected. Status unknown as of Feb '24.
	void process_building_placement(Unit* unit, u8 order_id, Point16 pos, u16 target_unit_id) { // 0048DE70
		if (order_id == OrderId::BuildingLand) {
			helper::issue_order_targeting_ground(unit, OrderId::BuildingLand, pos.x, pos.y);
			return;
		}
		if  (!UnitId::is_zerg_worker(static_cast<UnitId::Enum>(unit->id))
		|| ((unit->main_order_id != OrderId::DroneLand) || !(unit->status & (UnitStatus::CanNotReceiveOrders | UnitStatus::NoBrkCodeStart)))
		&&   unit->main_order_id != OrderId::DroneBuild) 
		{
			if ((unit->id != UnitId::ZergNydusCesiant || order_id != OrderId::BuildNydusExit) && order_id != OrderId::CTFCOP2) {
				g_ai_minerals_reserve[unit->player] = units_dat::mineral_cost[target_unit_id];
				g_ai_vespene_reserve[unit->player] = units_dat::vespene_cost[target_unit_id];
				if (g_game_minerals[unit->player] >= g_ai_minerals_reserve[unit->player]) {
					if (g_game_vespene[unit->player] >= g_ai_vespene_reserve[unit->player]) {
						helper::issue_order_targeting_ground(unit, order_id, pos.x, pos.y);
						if (units_dat::base_property[unit->id] & UnitProperty::Building && units_dat::base_property[target_unit_id] & UnitProperty::Addon) {
							helper::refund_all_queue_slots(unit);
							unit->building.addon_build_id = target_unit_id;
						} else {
							helper::refund_all_queue_slots(unit);
							helper::has_money_can_make(unit, target_unit_id);
						}
					} else {
						helper::display_error_message_id_with_sfx(unit->player, 851, bw::get_advisor_sound_from_event_type(unit->player, AdvisorEvent_NoVespene));
					}
				} else {
					helper::display_error_message_id_with_sfx(unit->player, 850, bw::get_advisor_sound_from_event_type(unit->player, AdvisorEvent_NoMinerals));
				}
			} else {
				helper::issue_order_targeting_ground(unit, order_id, pos.x, pos.y);
			}
		}
	}

	// Hooked to debug Pylon-based construction. -Pr0nogo
	u32 cmdrecv_place_building(u32 order_id, u16 built_unit_id, Point16 position) { // 0048E190
		if (units_dat::base_property[built_unit_id] & UnitProperty::Addon) {
			cmdrecv_place_addon(position.x, position.y, order_id, built_unit_id);
		} else {
			cmdrecv_place_building_normal(position.x, position.y, order_id, built_unit_id);
		}
		bw::refresh_console();
		return 0;
	}

	// Hooked to enable custom advisor lines. -Pr0nogo
	// Wound up going for a different attack, but this hook works, it's just not injected.
	b32 check_resources_for_building(u32 notify, u8 player) { // 0042D0C0 
		if (g_game_minerals[player] >= g_ai_minerals_reserve[player]) {
			if (g_game_vespene[player] >= g_ai_vespene_reserve[player]) {
				return true;
			} else {
				if (notify) {
					u32 sound_id = g_console_index + 150;
					if (bw::has_custom_advisor(player)) {
						sound_id = bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoVespene);
					}
					helper::display_error_message_id_with_sfx(player, 851, sound_id);
				}
				return false;
			}
		}
		if (notify) {
			u32 sound_id = g_console_index + 147;
			if (bw::has_custom_advisor(player)) {
				sound_id = bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoMinerals);
			}
			helper::display_error_message_id_with_sfx(player, 850, sound_id);
		}
		return false;
	}

	// Hooked to enable custom advisor lines. -Pr0nogo
	void display_error_message_id_with_sfx(u32 player, u32 message_id, u32 sound_id) { // 0048EE30
		if (player == g_local_nation_id && !g_is_in_replay) {
			sound_id = (u16)sound_id;
			if (sound_id && (*g_options_list).sfx) {
				u32 new_sound_id = bw::get_advisor_sound_from_message_id(player, message_id);
				if (new_sound_id) {
					sound_id = new_sound_id;
				}

				u32 volume = sfx_dat::volume[sound_id];
				u32 max_volume = 99;
				if (volume >= 99) {
					if (volume >= 0) {
						max_volume = sfx_dat::volume[sound_id];
					} else {
						max_volume = 0;
					}
				}
				if (g_use_quiet_sounds) {
					max_volume /= 5;
				}
				if (max_volume > 10) {
					u32 error_id = SoundId::None;
					switch (g_player_table[player].race) {
						case RaceId::Terran: {
							error_id = SoundId::ErrorTerran;
						} break;
						case RaceId::Protoss: {
							error_id = SoundId::ErrorProtoss;
						} break;
						case RaceId::Zerg: {
							error_id = SoundId::ErrorZerg;
						} break;
						case RaceId::Askosi: {
							error_id = SoundId::ErrorTerran;
						} break;
					}
					helper::play_sound_global(error_id);
					if (!bw::has_custom_advisor(player) || !g_globals->advisor_state.cooldown) {
						if (!g_globals->advisor_state.play_timer) {
							u32 timer = 6;
							switch (g_player_table[player].race) {
								case RaceId::Protoss:
								case RaceId::Zerg: {
									timer = 4;
								}
							}
							g_globals->advisor_state.play_timer = timer;
							g_globals->advisor_state.sound_id = sound_id;
							if (bw::has_custom_advisor(player)) {
								g_globals->advisor_state.cooldown = 54; // TODO: Replace with length of sound converted into w/e units
							}
						}
					}
				}
			}
			u32 final_message_id = message_id - 1;
			if ((u16)message_id) {
				char* message_string = (char*)g_stat_txt_buffer.get_string32(message_id);
				if (player == g_local_nation_id && g_active_player_id == g_local_human_id) {
					helper::set_unit_stat_txt_error_message(message_string);
				}
			}
		}
	}
}