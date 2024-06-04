#include "building_terran.h"
#include "building_modifier.h"
#include <BW/api.h>
#include <globals.h>
#include <hooks/limits/sfx_extender.h>
#include "trigger_system/trigger_system.h"

namespace hooks
{
	// The structure is being constructed
	void orders_terran_build_self(Unit* building) { // 00467760
		IscriptAnimation::Enum anim;

		switch (building->main_order_state) {
			case 0: {
				if (building->hitpoints < units_dat::max_hitpoints[building->id] * 0.2) {
					return;
				}

				for (Image* image = building->sprite->images.head; image; image = image->link.next) {
					image->play_iscript_animation(IscriptAnimation::SpecialState1);
				}
				++building->main_order_state;
			} break;
			case 1: {
				if (building->hitpoints < units_dat::max_hitpoints[building->id] * 0.4) {
					return;
				}

				for (Image* image = building->sprite->images.head; image; image = image->link.next) {
					image->play_iscript_animation(IscriptAnimation::SpecialState2);
				}
				++building->main_order_state;
			} break;
			case 2: {
				if (building->hitpoints < units_dat::max_hitpoints[building->id] * 0.6) {
					return;
				}

				helper::unit_completion_init(building, 0);
				for (Image* image = building->sprite->images.head; image; image = image->link.next) {
					image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
				}
				++building->main_order_state;
			} break;
			case 3: {
				if (building->hitpoints < units_dat::max_hitpoints[building->id] * 0.8) {
					building->order_computer(OrderId::Nothing2);
				}
			} break;
		}
	}

	// Resume construction on an existing structure
	void orders_scv_build_2(Unit* unit) { // 00467A70
		Unit* built_unit = unit->order_target.unit;
		BuildingModifier modifier = get_building_modifier(built_unit, unit, true);
		UnitExt* builder_ext = get_unit_ext(unit);

		if (built_unit && !(built_unit->status & UnitStatus::Completed)) {
			switch (unit->main_order_state) {
				case 0: {
					if (!helper::order_to_move_to_target(unit, built_unit)) {
						return;
					}
					unit->main_order_state = 2;
				} break;
				case 2: {
					u32 movable_state = unit->get_movable_state();
					if (movable_state == MovableState_UnmovableAtDestination) {
						unit->order_to_idle();
					} else if (movable_state != MovableState_NotReachedDestination) {
						bw::refresh_console();

						if (built_unit->connected_unit
						&&  built_unit->connected_unit != unit
						&& (built_unit->connected_unit)->order_target.unit == built_unit)
						{
							unit->order_to_idle(); // another worker is already on the job
						} else {
							builder_ext->values[ValueTerranConstruction] = built_unit->id;
							built_unit->connected_unit = unit;
							unit->status &= ~UnitStatus::IsNormal;
							unit->sprite->elevation = units_dat::elevation[unit->id] + 1;
							unit->order_simple(OrderId::ResetCollision1, false);
							unit->order_simple(units_dat::return_to_idle_order[unit->id], false);
							unit->secondary_order_pos = unit->position;
							// DF: Removed an unecessary delay for resuming construction progress. There might be a small chance for the worker to remain outside of the building's bounds if
							//     the randomized order_target is very near the same spot the SCV resumed construction, but I think the helper::TerrainCollision_? condition in State 4 should handle it.
							// if (helper::change_movement_target_move_target_2(unit, built_unit->sprite->position.x, built_unit->sprite->position.y))
							++unit->main_order_state;
						}
					}
					return;
				} break;
				case 3: {
					if (unit->get_movable_state() != MovableState_NotReachedDestination) {
						++unit->main_order_state;
					}
					return;
				} break;
				case 4: {
					s16 x_min = built_unit->sprite->position.x - units_dat::unit_bounds[built_unit->id].left + units_dat::unit_bounds[unit->id].left;
					s16 x_max = built_unit->sprite->position.x + units_dat::unit_bounds[built_unit->id].right - units_dat::unit_bounds[unit->id].right;
					s16 y_min = built_unit->sprite->position.y - units_dat::unit_bounds[built_unit->id].top + units_dat::unit_bounds[unit->id].top;
					s16 y_max = built_unit->sprite->position.y + units_dat::unit_bounds[built_unit->id].bottom - units_dat::unit_bounds[unit->id].bottom;

					if (x_min > x_max) {//If worker is wider than building, don't make him shuffle around horizontally. Might be safer to compare units_dat bounds, but this should be true for like 99% of cases.
						x_min = built_unit->sprite->position.x;
						x_max = built_unit->sprite->position.x;
					}

					if (y_min > y_max) {//If worker is taller than building, don't make him shuffle around vertically.
						y_min = built_unit->sprite->position.y;
						y_max = built_unit->sprite->position.y;
					}

					// DF: New, RNG-free pattern
					if (unit->position.y >= y_max) {
						unit->order_target.pt.x = x_min;
					} else {
						unit->order_target.pt.x = x_max;
					}
					if (unit->position.x <= x_min) {
						unit->order_target.pt.y = y_min;
					} else {
						unit->order_target.pt.y = y_max;
					}

					helper::fix_target_location(&unit->order_target.pt, unit->id);

					if (!helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y)) {
						return;
					}
				} // fall
				case 5: {
					unit->main_order_state = 6;
				} break;
				case 6: {
					if (unit->position == unit->next_target_waypoint) {
						helper::set_resource_target(unit);
						helper::set_secondary_waypoint(unit, (unit->order_target.unit)->sprite->position.x, (unit->order_target.unit)->sprite->position.y);
						s32 angle = bw::get_angle(unit->sprite->position.x, unit->sprite->position.y, unit->next_target_waypoint.x, unit->next_target_waypoint.y);
						unit->order_target.pt.x = (g_angle_distance[angle].x * 20) / 256 + unit->sprite->position.x;
						unit->order_target.pt.y = (g_angle_distance[angle].y * 20) / 256 + unit->sprite->position.y;
						++unit->main_order_state;
					}
				} break;
				case 7: {
					if (!helper::is_unit_positions_2_equal(unit)) {
						s32 angle = unit->current_direction_1 - bw::get_angle(unit->sprite->position.x, unit->sprite->position.y, unit->next_target_waypoint.x, unit->next_target_waypoint.y);
						if (angle < 0) {
							angle += 256;
						}
						if (angle > 128) {
							angle = 256 - angle;
						}
						if (angle > weapons_dat::attack_angle[unit->get_ground_weapon()]) {
							break;
						}
					}
					unit->main_order_timer = 30;
					for (Image* image = unit->sprite->images.head; image; image = image->link.next) {
						image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
					}
					++unit->main_order_state;
				} break;
				case 8: {
					if (!unit->main_order_timer) {
						for (Image* image = unit->sprite->images.head; image; image = image->link.next) {
							image->play_iscript_animation(IscriptAnimation::GndAttkToIdle);
						}

						unit->main_order_state = 4;
					}
				} break;
			}

			u32 hp_gain = built_unit->build_repair_hp_gain * modifier.multiplier;
			if (*g_cheat_state & CheatFlags::OperationCwal) {
				hp_gain *= 16;
			}

			helper::progress_build(built_unit, hp_gain, false);
			if (modifier.decrease) {
				if (built_unit->remaining_build_time > modifier.decrease) {
					built_unit->remaining_build_time -= modifier.decrease;
				} else {
					built_unit->remaining_build_time = 0;
					helper::set_unit_hp(built_unit, built_unit->get_max_hp_raw());
				}
			}

			if (!(built_unit->status & UnitStatus::Completed)) {
				return;
			}
		}
		
		for (Image* image = unit->sprite->images.head; image; image = image->link.next) {
			image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
		}

		if (built_unit) {
			built_unit->connected_unit = 0;
			helper::play_building_completed_sound(unit);
			if (unit_id_is_tech_tier_structure(built_unit->id)) {
				replay_witness_event(built_unit, SpectatorEvent_Tech);
			}
			builder_ext->values[ValueTerranConstruction] = UnitId::None;
			helper::update_unit_damage_overlay(built_unit); // New - fix bug where damaged structures don't create overlay until damaged or healed. -Pr0nogo

			if ((built_unit->status & UnitStatus::Completed
			&&   unit_id_is_capped_gas_structure(built_unit->id) && built_unit->player == unit->player) || unit_id_is_minerals(built_unit->id))
			{
				unit->order_to(OrderId::Harvest1, built_unit);
				return;
			}
		}

		if (unit->order_queue_head) {
			unit->user_action_flags |= 1;
			prepare_for_next_order(unit);
		} else if (unit->unit_ai) {
			unit->order_computer(OrderId::ComputerAI);
		} else {
			unit->order_computer(units_dat::return_to_idle_order[unit->id]);
		}
	}

	// Move to construct a structure that has not been started already
	void orders_scv_build(Unit* unit) { // 00467FD0
		if (unit->unit_ai) {
			unit->status |= UnitStatus::IsGathering;
		}

		if (unit->main_order_state) {
			if (unit->main_order_state == 1) {
				if (unit->sprite->position.x == unit->move_target.pt.x
					&& unit->sprite->position.y == unit->move_target.pt.y) {
					++g_unmovable_hits[10];
					g_unmovable_lastframe[10] = g_elapsed_time_frames;
					if (!(unit->status & UnitStatus::Unmovable)
					&&    helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 128)
					&&    helper::begin_building(unit))
					{
						UnitExt* builder_ext = get_unit_ext(unit);
						builder_ext->terran_build_pos = unit->position;

						Unit* built_unit = 0;
						unit->status &= ~UnitStatus::IsNormal;
						unit->sprite->elevation = units_dat::elevation[unit->id] + 1;

						if (unit->order_queue_head) {
							helper::prepend_order_targeting_nothing(unit, unit->order_queue_head, OrderId::ResetCollision1);
						} else {
							helper::add_order_targeting_nothing(unit, 0, OrderId::ResetCollision1);
							helper::add_order_targeting_nothing(unit, 0, units_dat::return_to_idle_order[unit->id]);
						}

						u16 gas_building_id = unit->build_queue[unit->build_queue_slot];
						if (gas_building_id == UnitId::TerranReservoir || gas_building_id == UnitId::TerranWellbore) {
							unit->secondary_order_pos = unit->position;
							built_unit = helper::begin_gas_building(unit, gas_building_id);
						} else {
							unit->secondary_order_pos = { 0, 0 };
							built_unit = helper::create_unit(unit->build_queue[unit->build_queue_slot], unit->order_target.pt.x, unit->order_target.pt.y, unit->player);
						}

						unit->build_queue[unit->build_queue_slot] = UnitId::None;

						if (built_unit) {
							spread_debuffs(unit, built_unit);
							builder_ext->values[ValueTerranConstruction] = built_unit->id;
							built_unit->create_origin = unit->id;
							built_unit->connected_unit = unit;
							built_unit->sprite->player = unit->sprite->player; // Player color inheritance.
							unit->main_order_id = OrderId::ConstructingBuilding;
							unit->main_order_state = 3;
							unit->order_target.unit = built_unit;

							if (g_is_placing_building) {
								if (!helper::can_still_place_building()) {
									helper::refresh_layers_3_and_4();
									helper::end_building_placement();
								}
							}

							bw::refresh_console();
							built_unit->order_computer(OrderId::BuildSelf1);
							helper::ai_training_overlord(built_unit, unit);

							if (unit_id_is_tech_tier_structure(built_unit->id)) {
								replay_witness_event(built_unit, SpectatorEvent_Tech);
							}
						} else {
							helper::refund_unit_train_cost(unit->build_queue[unit->build_queue_slot], unit->player);
							helper::display_last_network_error_for_player(unit->player);
							unit->order_computer(units_dat::return_to_idle_order[unit->id]);
						}
					} else {
						unit->order_to_idle();
					}
				}
			}
		} else {
			bw::refresh_console();

			if (unit->order_target.unit) {
				unit->order_target.pt.x = (unit->order_target.unit)->sprite->position.x;
				unit->order_target.pt.y = (unit->order_target.unit)->sprite->position.y;
			}

			bool move_target_result = helper::set_move_target_xy(unit, unit->order_target.pt.x, unit->order_target.pt.y);
			if ((unit->order_target.pt.x != unit->next_target_waypoint.x || unit->order_target.pt.y != unit->next_target_waypoint.y)
				&& !(unit->order_target.pt.x == 0 && unit->order_target.pt.y == 0)) {
				unit->next_target_waypoint.x = unit->order_target.pt.x;
				unit->next_target_waypoint.y = unit->order_target.pt.y;
			}

			if (move_target_result) {
				unit->main_order_state = 1;
			}
		}
	}

	char refresh_screen(s32 ecx0) { // 0048DDC0
		auto v6 = ecx0;
		auto result = g_is_placing_building;
		if (g_is_placing_building) {
			auto y = g_mouse->y;
			auto x = g_mouse->x;
			auto mouse_pos = Point16{ (u16)g_mouse->x, (u16)g_mouse->y };
			if (helper::init_placement_box_pos(&mouse_pos)) {
				*g_place_building_message_id = helper::update_placement_state_for_placement_box();
				helper::refresh_layers_3_and_4();
				Point16 buildPoint;
				buildPoint.x = x;
				buildPoint.y = y;
				helper::unk_building_placement_related(1, &buildPoint);
				if (*g_is_placing_addon) {
					u32 unit_id = *g_place_building_unit_id - UnitId::TerranMinistry;
					Point16 addon_point = { units_dat::addon_placement[unit_id].x , units_dat::addon_placement[unit_id].y };
					bw::adjust_addon_coordinates(*g_place_building_unit_id, 0, &addon_point.x, &addon_point.y);
					helper::unk_building_placement_related(0, &addon_point);
				}
				result = helper::refresh_layers_3_and_4();
			} else {
				helper::unk_building_placement_related_mouse(0, 1);
				result = helper::unk_building_placement_related_mouse(0, 0);
			}
		}
		return result;
	}

	// Utility structure, so setting command buffer is easier
#pragma pack(push, 1)
	struct BuildCommand {
		u8 id;
		u8 order_id;
		u16 tile_x;
		u16 tile_y;
		u16 unit_id;
	};
#pragma pack(pop)

	// Hooked to allow instant addon placement. -Pr0nogo
	void btnsact_place_addon(u16 unit_id, char* buffer) { // 00423D10
		bool errored = false;
		*g_unit_iterator_pos = 0;
		while (Unit* unit = helper::get_next_commanded_unit()) {
			u8 player = unit->player;
			g_ai_minerals_reserve[player] = units_dat::mineral_cost[unit_id];
			g_ai_vespene_reserve[player] = units_dat::vespene_cost[unit_id];
			if (helper::has_supplies_for_unit(player, unit_id, 1)) {
				if (g_game_minerals[player] >= g_ai_minerals_reserve[player]) {
					if (g_game_vespene[player] >= g_ai_vespene_reserve[player]) {
						helper::issue_place_building_order(buffer, unit_id, OrderId::PlaceAddon);
						// For MBS this would essentially require a custom game command, for now we just have to disable it :( -Veeq7
						/*
						if ((*g_is_holding_shift || unit->status & UnitStatus::InAir) && *g_client_selection_count <= 1) {
							helper::issue_place_building_order(buffer, unit_id, OrderId::PlaceAddon);
						} else if (!(unit->status & UnitStatus::InAir)) {
							BuildCommand command = {};
							command.id = 12;
							command.order_id = OrderId::PlaceAddon;
							command.tile_x = (unit->position.x - units_dat::addon_placement[unit_id].x) / 32;
							command.tile_y = (unit->position.y - units_dat::addon_placement[unit_id].y) / 32;
							command.unit_id = unit_id;
							helper::queue_game_command((u8*)&command, sizeof(command));
						}
						*/
					} else if (!errored) {
						errored = true;
						helper::display_error_message_id_with_sfx(player, 851, g_console_index[150]);
					}
				} else if (!errored) {
					errored = true;
					helper::display_error_message_id_with_sfx(player, 850, g_console_index[147]);
				}
			}
		}
	}

	// Hooked to play the correct new_sound when Taloses complete a structure. -Pr0nogo
	void play_building_complete_sound(Unit* unit) { // 0048F310
		if (!g_is_in_replay && unit->player == *g_local_nation_id) {
			u32 race = unit->get_race();
			switch (race) {
				case RaceId::Zerg: {
					u32 sound_id = 0;
					if (unit_id_is_tech_tier_structure(unit->id)) {
						sound_id = bw::get_advisor_sound_from_event_type(*g_local_nation_id, AdvisorEvent_TechUpComplete);
					}
					if (unit->create_origin == UnitId::ZergDroleth) {
						if (sound_id) {
							helper::play_sound_from_unit(sound_id, unit, (*g_options_list).flags & 1, 99);
						}
						helper::play_sound_from_unit(SoundId::Zerg_Bldg_ZChRdy00_WAV, unit, (*g_options_list).flags & 1, 99);
						helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[135] + 1000 * (building_complete_something[0] / 1000), 1, 135);
					} else if (unit->create_origin == UnitId::ZergGosvileth) {
						if (sound_id) {
							helper::play_sound_from_unit(sound_id, unit, (*g_options_list).flags & 1, 99);
						}
						helper::play_sound_from_unit(SoundId::Zerg_Bldg_ZChRdy00_WAV, unit, (*g_options_list).flags & 1, 99);
						helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[135] + 1000 * (building_complete_something[0] / 1000), 1, 135);
					} else {
						// Nydus exit and morphs
						if (sound_id) {
							helper::play_sound_from_unit(sound_id, unit, (*g_options_list).flags & 1, 99);
						}
						helper::play_sound_from_unit(SoundId::Zerg_Bldg_ZChRdy00_WAV, unit, (*g_options_list).flags & 1, 99);
						helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[135] + 1000 * (building_complete_something[0] / 1000), 1, 135);
					}
				} break;
				case RaceId::Terran: {
					UnitExt* builder_ext = get_unit_ext(unit);
					u32 sound_id = 0;
					if (units_dat::base_property[unit->id] & UnitProperty::Building) {
						sound_id = bw::get_advisor_sound_from_event_type(*g_local_nation_id, AdvisorEvent_AddonComplete);
						if (!sound_id) {
							sound_id = SoundId::Terran_Advisor_TAdUpd03_WAV;
						}
					} else if (unit->id == UnitId::TerranMason || unit->id == UnitId::TerranTalos) {
						sound_id = SoundId::Terran_SCV_TSCUpd00_WAV; // Job's finished
					}

					if (unit_id_is_tech_tier_structure(builder_ext->values[ValueTerranConstruction])) {
						sound_id = bw::get_advisor_sound_from_event_type(*g_local_nation_id, AdvisorEvent_TechUpComplete);
					}

					helper::play_sound_from_unit(sound_id, unit, g_options_list->flags & 1, 99);
					helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[sound_id] + 1000 * (sfx_dat::sound_data[sound_id].duration_ms / 1000), 1, sound_id);
				} break;
				case RaceId::Protoss:
				default: {
					u32 sound_id = bw::get_advisor_sound_from_event_type(*g_local_nation_id, AdvisorEvent_StructureComplete_P);
					helper::play_sound_from_unit(sound_id, unit, 1, 99);
					helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[sound_id] + 1000 * (building_complete_something[0] / 1000), 1, sound_id);
				} break;
			}
		}
	}

	// Hooked to clear placement area. -Pr0nogo
	// Not currently functional, so until we fix it, we won't have "tech center complete" for tech addons.
	void orders_place_addon(u32 unused, Unit* unit) { // 004E6880
		if (!unit->main_order_state) {
			if (!(units_dat::base_property[unit->building.addon_build_id] & UnitProperty::Addon)) {
				return;
			}
			if (unit->status & UnitStatus::GroundedBuilding) {
				if (helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 0)) {
					if (!helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y)) {
						return;
					}
					unit->main_order_state = 1;
					goto place_addon;
				}
				helper::issue_order_targeting_ground(unit, OrderId::Liftoff, unit->order_target.pt.x, unit->order_target.pt.y);
			} else {
				helper::issue_order_targeting_ground(unit, OrderId::BuildingLand, unit->order_target.pt.x, unit->order_target.pt.y);
			}
			helper::do_order(unit, OrderId::PlaceAddon, unit->order_target.pt.x, unit->order_target.pt.y, 0);
			return;
		}
		if (unit->main_order_state != 1) {
			return;
		}
	place_addon:
		if  (unit->move_target.pt.x == unit->position.x
		&&   unit->move_target.pt.y == unit->position.y)
		{
			// Get addon position - this is probably wrong right now, the original IDA code was a disaster.
			u32 addon_id = unit->building.addon_build_id;
			Point16 dimensions = units_dat::building_dimensions[addon_id];
			unit->order_target.pt.x += units_dat::addon_placement[addon_id].x + dimensions.x / 2;
			unit->order_target.pt.y += units_dat::addon_placement[addon_id].y + dimensions.y / 2;

			u32 player = unit->player;
			g_ai_minerals_reserve[player] = units_dat::mineral_cost[addon_id];
			g_ai_vespene_reserve[player] = units_dat::vespene_cost[addon_id];
			if (g_game_minerals[player] < g_ai_minerals_reserve[player])
			{
				helper::display_error_message_id_with_sfx(player, 850, *g_console_index + 147);
				helper::to_idle(unit);
				return;
			}
			if (g_game_vespene[player] < g_ai_vespene_reserve[player])
			{
				helper::display_error_message_id_with_sfx(player, 851, *g_console_index + 150);
				helper::to_idle(unit);
				return;
			}

			if (helper::can_build_on_tile_location(unit, unit->player, unit->order_target.pt.x / 32 - dimensions.x / 32 / 2, unit->order_target.pt.y / 32 - dimensions.y / 32 / 2, unit->building.addon_build_id, 0, 0, 1, 0))
			{
				helper::play_worker_error_sound(unit, 856); // Can't build there
				helper::to_idle(unit);
			} else {
				Unit* addon = helper::create_unit(unit->building.addon_build_id, unit->order_target.pt.x, unit->order_target.pt.y, unit->player);
				if (addon)
				{
					UnitExt* builder_ext = get_unit_ext(unit);
					builder_ext->values[ValueTerranConstruction] = unit->building.addon_build_id;
					helper::subtract_queue_cost(unit->player);
					helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
					helper::set_secondary_order(unit, OrderId::BuildAddon);
					unit->current_build_unit = addon;
					helper::issue_order_targeting_nothing(addon, OrderId::BuildSelf1);
					helper::ai_training_overlord(addon, unit);
					addon->connected_unit = unit;
					helper::set_unit_event_complete();
				} else {
					helper::display_last_network_error_for_player(unit->player);
				}
			}
		}
	}

	// Hooked to enable more refinery types. -Pr0nogo
	b32 unit_is_geyser(Unit* unit) // 004688B0
	{
		if (unit->status & UnitStatus::Completed) {
			if (unit_id_is_capped_gas_structure(unit->id)) {
				return 1;
			}
		}
		return 0;
	}

	// Hooked by mistake. -Pr0nogo
	b32 is_constructing_addon(Unit* unit) // 004E66B0
	{
		b32 result = 0;
		if (unit->secondary_order_id == OrderId::BuildAddon) {
			if (unit->status & UnitStatus::GroundedBuilding) {
				Unit* build_unit = unit->current_build_unit;
				if (build_unit) {
					if (!(build_unit->status & UnitStatus::Completed)) {
						result = 1;
					}
				}
			}
		}
		return result;
	}

	// Hooked to modify addon construction speed based on building modifier (for e.g. Crucible speed boosts) -Pr0nogo
	b32 building_addon(Unit* unit, u32 hp_gain, b32 is_incomplete) // 004679A0
	{
		helper::set_unit_hp(unit, hp_gain + unit->hitpoints);
		u32 build_time = unit->remaining_build_time;
		if (build_time) {
			BuildingModifier modifier = get_building_modifier(unit, unit->connected_unit, true);
			if (modifier.decrease) {
				if (unit->remaining_build_time > modifier.decrease) {
					unit->remaining_build_time -= modifier.decrease;
				} else {
					unit->remaining_build_time = 0;
					if (unit_needs_hp_train_workaround(unit)) {
						helper::set_unit_hp(unit, unit->get_max_hp_raw());
					}
				}
			} else {
				unit->remaining_build_time -= 1;
			}
			return true;
		}
		if (!is_incomplete) {
			helper::update_unit_stats_and_finish_building(unit);
			goto finish;
		}
		helper::update_unit_stats_and_finish_building(unit);

		if (helper::show_finished_unit(unit)) {
		finish:
			helper::update_unit_strength_and_apply_default_orders(unit);
			if (!(unit->status & UnitStatus::GroundedBuilding)) {
				helper::play_unit_ready_sound(unit);
				return true;
			}
			if (units_dat::group_flags[unit->id].is_terran) {
				Unit* addon = helper::connect_addon_check(unit);
				if (addon) {
					helper::complete_addon(unit, addon);
				}
			}
			return true;
		}
		return false;
	}

	// Hooked to stop giving addons to neutral. -Pr0nogo
	void disconnect_addon(Unit* unit) // 00464930
	{
		Unit* addon = unit->building.addon;
		unit->building.addon = 0;
		helper::play_addon_detach_sound(addon);

		UnitExt* addon_ext = get_unit_ext(addon);
		addon_ext->values[ValueAddonInactive] = 1;
		bw::refresh_console();

		Unit* active_unit = *g_active_iscript_unit;
		*g_active_iscript_unit = addon;
		for (Image* i = addon->sprite->images.head; i; i = i->link.next) {
			helper::play_image_iscript(i, IscriptAnimation::LiftOff);
		}
		*g_active_iscript_unit = active_unit;
	}

	// Hooked to stop giving addons to neutral. -Pr0nogo
	void complete_addon(Unit* addon, Unit* parent) // 00463D50
	{
		spread_debuffs(parent, addon);
		u32 player = parent->player;
		parent->building.addon = addon;
		helper::give_unit(addon, player, 0);
		helper::give_sprite(addon, parent->player);

		UnitExt* addon_ext = get_unit_ext(addon);
		addon_ext->values[ValueAddonInactive] = 0;

		Unit* active_unit = *g_active_iscript_unit;
		*g_active_iscript_unit = addon;
		for (Image* i = addon->sprite->images.head; i; i = i->link.next) {
			helper::play_image_iscript(i, IscriptAnimation::Landing);
		}
		*g_active_iscript_unit = active_unit;

		bool is_silo = parent->id == UnitId::TerranAnvil;
		if (is_silo) {
			Unit* missile = parent->building.silo.nuke;
			if (missile) {
				helper::give_unit(missile, parent->player, 0);
			}
		}
	}

	// Hooked to add custom advisor handling. -Pr0nogo
	void play_addon_detach_sound(Unit* unit) // 0048F8A0
	{
		if (!g_is_in_replay && unit->player == *g_local_nation_id) {
			u32 sound_id = bw::get_advisor_sound_from_event_type(unit->player, AdvisorEvent_AbandonAddon);
			helper::play_sound_from_unit(sound_id, unit, (g_options_list->flags >> 2) & 1, 99);
			helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[sound_id] + 1000 * (*g_audio_something / 1000), 1, 130);
		}
	}
}