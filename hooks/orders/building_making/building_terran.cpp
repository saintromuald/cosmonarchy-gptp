#include "building_terran.h"
#include "building_modifier.h"
#include <SCBW/api.h>
#include <globals.h>
#include <hooks/limits/sfx_extender.h>

namespace hooks
{
	// The structure is being constructed
	void orders_terran_build_self(Unit* building) // 00467760
	{
		if (building->main_order_state == 0)
		{
			if (building->hitpoints > 20 * units_dat::MaxHitPoints[building->id] / 100)
				building->main_order_state = 1;
		}
		else if (building->main_order_state == 1)
		{
			Image* current_image = building->sprite->images.head;
			while (current_image)
			{
				current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
				current_image = current_image->link.next;
			}
			building->main_order_state = 2;
		}
		else if (building->main_order_state == 2)
		{
			if (building->hitpoints > 40 * units_dat::MaxHitPoints[building->id] / 100)
			{
				building->main_order_state = 3;
			}
		}
		else if (building->main_order_state == 3)
		{
			Image* current_image = building->sprite->images.head;
			while (current_image)
			{
				current_image->play_iscript_animation(IscriptAnimation::SpecialState2);
				current_image = current_image->link.next;
			}
			building->main_order_state = 4;
		}
		else if (building->main_order_state == 4)
		{
			if (building->hitpoints > 60 * units_dat::MaxHitPoints[building->id] / 100)
			{
				building->main_order_state = 5;
			}
		}
		else if (building->main_order_state == 5)
		{
			helper::init_building(building, 0);
			Image* current_image = building->sprite->images.head;
			while (current_image)
			{
				current_image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
				current_image = current_image->link.next;
			}
			building->main_order_state = 6;
		}
		else if (building->main_order_state == 6)
		{
			if (building->hitpoints > 80 * units_dat::MaxHitPoints[building->id] / 100)
			{
				building->order_computer(OrderId::Nothing2);
			}
		}
	}

	// Resume construction on an existing structure
	void orders_scv_build_2(Unit* unit) // 00467A70
	{
		Unit* builtUnit = unit->order_target.unit;
		BuildingModifier modifier = get_building_modifier(builtUnit, unit, true);
		if (builtUnit && !(builtUnit->status & UnitStatus::Completed))
		{
			bool jump_to_switch_1 = false;
			if (unit->main_order_state > 8 ||
				unit->main_order_state == 1)
			{
				jump_to_switch_1 = true;
			}
			if (unit->main_order_state == 0)
			{
				if (helper::order_to_move_to_target(unit, builtUnit))
					unit->main_order_state = 2;
			}
			if (unit->main_order_state == 2)
			{
				u32 movableState = unit->get_movable_state();
				if (movableState == MovableState::UnmovableAtDestination)
				{
					unit->order_to_idle();
				}
				else if (movableState == MovableState::ReachedDestination)
				{
					scbw::refresh_console();
					if (builtUnit->connected_unit &&
						builtUnit->connected_unit != unit &&
						(builtUnit->connected_unit)->order_target.unit == builtUnit)
					{
						unit->order_to_idle(); // another worker is already on the job
					}
					else
					{
						builtUnit->connected_unit = unit;
						unit->status &= ~UnitStatus::IsNormal;	//remove UnitStatus::IsNormal if present
						unit->sprite->elevation_level = units_dat::Elevation[unit->id] + 1;
						unit->order_simple(OrderId::ResetCollision1, false);
						unit->order_simple(units_dat::ReturnToIdleOrder[unit->id], false);
						// DF: Removed an unecessary delay for resuming construction progress. There might be a small chance for the SCV to remain outside of the building's bounds if
						//    the randomized order_target is very near the same spot the SCV resumed construction, but I think the helper::TerrainCollision_? condition in State 4 should handle it.
						//if (helper::ChangeMovementTargetMoveTarget2(unit, builtUnit->sprite->position.x, builtUnit->sprite->position.y))
						unit->main_order_state = 3;
					}
				}
			}
			else if (unit->main_order_state == 3)
			{
				if (unit->get_movable_state() != MovableState::NotReachedDestination)
				{
					unit->main_order_state = 4;
				}
			}
			else if (unit->main_order_state == 4)
			{
				s16 x_min, x_max, y_min, y_max;
				x_min = builtUnit->sprite->position.x - units_dat::UnitBounds[builtUnit->id].left + units_dat::UnitBounds[unit->id].left;
				x_max = builtUnit->sprite->position.x + units_dat::UnitBounds[builtUnit->id].right - units_dat::UnitBounds[unit->id].right;
				y_min = builtUnit->sprite->position.y - units_dat::UnitBounds[builtUnit->id].top + units_dat::UnitBounds[unit->id].top;
				y_max = builtUnit->sprite->position.y + units_dat::UnitBounds[builtUnit->id].bottom - units_dat::UnitBounds[unit->id].bottom;
				if (x_min > x_max) // DF: If worker is wider than building, don't make him shuffle around horizontally. Might be safer to compare units_dat bounds, but this should be true for like 99% of cases.
				{
					x_min = builtUnit->sprite->position.x;
					x_max = builtUnit->sprite->position.y;
				}
				if (y_min > y_max) // DF: If worker is taller than building, don't make him shuffle around vertically.
				{
					y_min = 0;
					y_max = 0;
				}

				unit->order_target.pt.x = helper::rand_between(x_min, x_max, 9); // DF: Can't tell what effect the third value has on the randomization.
				unit->order_target.pt.y = helper::rand_between(y_min, y_max, 9);
				helper::fix_target_location(&unit->order_target.pt,unit->id);
				if (helper::unk_terrain_collision_2(unit, unit->order_target.pt.x - unit->sprite->position.x, unit->order_target.pt.y - unit->sprite->position.y) != 0)
				{
					unit->order_target.pt.x = helper::rand_between(x_min, x_max, 9);
					unit->order_target.pt.y = helper::rand_between(y_min, y_max, 9);
					helper::fix_target_location(&unit->order_target.pt, unit->id);
				}

				if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y) != 0)
				{
					unit->main_order_state = 5;
					jump_to_switch_1 = true;
				}
			}
			else if (unit->main_order_state == 5)
			{
				unit->main_order_state = 6;
				jump_to_switch_1 = true;
			}
			else if (unit->main_order_state == 6)
			{
				if (!helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 20))
				{
					jump_to_switch_1 = true;
				}
				else
				{
					s32 angle;
					helper::set_resource_target(unit);
					helper::set_secondary_waypoint(unit, (unit->order_target.unit)->sprite->position.x, (unit->order_target.unit)->sprite->position.y);
					angle = scbw::get_angle(unit->sprite->position.x, unit->sprite->position.y, unit->next_target_waypoint.x, unit->next_target_waypoint.y);
					unit->order_target.pt.x = (g_angle_distance[angle].x * 20) / 256 + unit->sprite->position.x;
					unit->order_target.pt.y = (g_angle_distance[angle].y * 20) / 256 + unit->sprite->position.y;
					unit->main_order_state = 7;
					jump_to_switch_1 = true;
				}
			}
			else if (unit->main_order_state == 7)
			{
				if (!helper::is_unit_positions_2_equal(unit))
				{
					s32 angle = scbw::get_angle(unit->sprite->position.x, unit->sprite->position.y, unit->next_target_waypoint.x, unit->next_target_waypoint.y);
					angle = unit->current_direction_1 - angle;
					if (angle < 0)
					{
						angle += 256;
					}
					if (angle > weapons_dat::attack_angle[unit->get_ground_weapon()])
					{
						jump_to_switch_1 = true;
					}
					else
					{
						u8 rand_value = helper::randomize_short(10);
						Image* current_image = unit->sprite->images.head;
						unit->main_order_timer = (rand_value & 63) + 30;
						while (current_image)
						{
							current_image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
							current_image = current_image->link.next;
						}
						unit->main_order_state = 8;
						jump_to_switch_1 = true;
					}
				}
				else
				{
					u8 rand_value = helper::randomize_short(10);
					Image* current_image = unit->sprite->images.head;
					unit->main_order_timer = (rand_value & 63) + 30;
					while (current_image)
					{
						current_image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
						current_image = current_image->link.next;
					}
					unit->main_order_state = 8;
					jump_to_switch_1 = true;
				}
			}
			else if (unit->main_order_state == 8)
			{
				if (unit->main_order_timer != 0)
				{
					jump_to_switch_1 = true;
				}
				else
				{
					Image* current_image = unit->sprite->images.head;
					while (current_image)
					{
						current_image->play_iscript_animation(IscriptAnimation::GndAttkToIdle);
						current_image = current_image->link.next;
					}
					unit->main_order_state = 4;
					jump_to_switch_1 = true;
				}
			}

			if (jump_to_switch_1)
			{
				u32 hpGain = builtUnit->build_repair_hp_gain * modifier.multiplier;
				UnitExt* unitExt = get_unit_ext(builtUnit);
				if (*g_cheat_state & CheatFlags::OperationCwal)
				{
					hpGain *= 16;
				}
				helper::progress_build(builtUnit, hpGain, false);
				if (modifier.decrease) {
					if (builtUnit->remaining_build_time > modifier.decrease) {
						builtUnit->remaining_build_time -= modifier.decrease;
					} else {
						builtUnit->remaining_build_time = 0;
					}
				}
				if (builtUnit->status & UnitStatus::Completed)
				{
					Image* current_image = unit->sprite->images.head;
					bool bEndThere = false;
					while (current_image)
					{
						current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
						current_image = current_image->link.next;
					}
					if (builtUnit)
					{
						bool jump_to_67F3C = false;
						builtUnit->connected_unit = nullptr;
						helper::play_building_completed_sound(unit);

						if (builtUnit->status & UnitStatus::Completed)
						{
							if (builtUnit->id == UnitId::TerranReservoir
								|| builtUnit->id == UnitId::ProtossAquifer
								|| builtUnit->id == UnitId::ZergExcisant)
							{
								if (builtUnit->player == unit->player)
								{
									jump_to_67F3C = true;
								}
							}
						}
						if (!jump_to_67F3C)
						{
							if (builtUnit->id >= UnitId::ResourceMineralField &&
								builtUnit->id <= UnitId::ResourceMineralFieldType3)
							{
								jump_to_67F3C = true;
							}
						}
						if (jump_to_67F3C)
						{
							unit->order_to(OrderId::Harvest1, builtUnit);
							bEndThere = true;
						}
					}
					if (!bEndThere)
					{
						if (unit->order_queue_head)
						{
							unit->user_action_flags |= 1;
							prepare_for_next_order(unit);
						}
						else if (unit->unit_ai)
						{
							unit->order_computer(OrderId::ComputerAI);
						}
						else
						{
							unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
						}
					}
				}
			}
		}
		else
		{
			Image* current_image = unit->sprite->images.head;
			bool bEndThere = false;
			while (current_image)
			{
				current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
				current_image = current_image->link.next;
			}
			if (builtUnit)
			{
				bool jump_to_67F3C = false;
				builtUnit->connected_unit = nullptr;
				helper::play_building_completed_sound(builtUnit);
				if (builtUnit->status & UnitStatus::Completed)
				{
					if (scbw::unit_id_is_capped_gas_structure(builtUnit->id))
					{
						if (builtUnit->player == unit->player)
						{
							jump_to_67F3C = true;
						}
					}
				}
				if (!jump_to_67F3C)
				{
					if (builtUnit->id >= UnitId::ResourceMineralField &&
						builtUnit->id <= UnitId::ResourceMineralFieldType3)
					{
						jump_to_67F3C = true;
					}
				}
				if (jump_to_67F3C)
				{
					unit->order_to(OrderId::Harvest1, builtUnit);
					bEndThere = true;
				}
			}
			if (!bEndThere)
			{
				if (unit->order_queue_head)
				{
					unit->user_action_flags |= 1;
					prepare_for_next_order(unit);
				}
				else if (unit->unit_ai)
				{
					unit->order_computer(OrderId::ComputerAI);
				}
				else
				{
					unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
	}

	// Move to construct a structure that has not been started already
	void orders_scv_build(Unit* unit) // 00467FD0
	{
		if (unit->unit_ai)
		{
			unit->status |= UnitStatus::IsGathering;
		}
		if (unit->main_order_state == 1)
		{
			if (unit->sprite->position.x == unit->move_target.pt.x
			&&	unit->sprite->position.y == unit->move_target.pt.y)
			{
				if (!(unit->status & UnitStatus::Unmovable)
				&&	  helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 128)
				&&	  helper::begin_building(unit))
				{
					Unit* builtUnit;
					unit->status &= ~UnitStatus::IsNormal;
					unit->sprite->elevation_level = units_dat::Elevation[unit->id] + 1;
					if (!unit->order_queue_head)
					{
						helper::add_order_targeting_nothing(unit, nullptr, OrderId::ResetCollision1);
						helper::add_order_targeting_nothing(unit, nullptr, units_dat::ReturnToIdleOrder[unit->id]);
					}
					else
					{
						helper::prepend_order_targeting_nothing(unit, unit->order_queue_head, OrderId::ResetCollision1);
					}
					if (unit->build_queue[unit->build_queue_slot] == UnitId::TerranReservoir)
					{
						builtUnit = helper::begin_gas_building(unit, UnitId::TerranReservoir);
					}
					else
					{
						builtUnit = helper::create_unit(unit->build_queue[unit->build_queue_slot], unit->order_target.pt.x, unit->order_target.pt.y, unit->player);
					}
					if (builtUnit) {
						builtUnit->create_origin = unit->id;
					}
					u32 unit_id = unit->build_queue[unit->build_queue_slot];
					unit->build_queue[unit->build_queue_slot] = UnitId::None;
					if (!builtUnit)
					{
						helper::refund_unit_train_cost(unit_id, unit->player);
						helper::display_last_network_error_for_player(unit->player);
						unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
					}
					else
					{
						builtUnit->connected_unit = unit;
						builtUnit->sprite->player = unit->sprite->player;
						unit->main_order_id = OrderId::ConstructingBuilding;
						unit->main_order_state = 3;
						unit->order_target.unit = builtUnit;
						if (*g_is_placing_building)
						{
							if (!helper::can_still_place_building())
							{
								helper::refresh_layers_3_and_4();
								helper::end_building_placement();
							}
						}
						scbw::refresh_console();
						builtUnit->order_computer(OrderId::BuildSelf1);
						helper::ai_training_overlord(builtUnit, unit);
					}
				}
				else
				{
					unit->order_to_idle();
				}
			}
		}
		else if (unit->main_order_state == 0)
		{
			bool bMoveTargetResult;
			scbw::refresh_console();
			if (unit->order_target.unit)
			{
				unit->order_target.pt.x = (unit->order_target.unit)->sprite->position.x;
				unit->order_target.pt.y = (unit->order_target.unit)->sprite->position.y;
			}
			bMoveTargetResult = helper::set_move_target_xy(unit, unit->order_target.pt.x, unit->order_target.pt.y);
			if ((unit->order_target.pt.x != unit->next_target_waypoint.x ||
				unit->order_target.pt.y != unit->next_target_waypoint.y)
				&& !(unit->order_target.pt.x == 0 && unit->order_target.pt.y == 0))
			{
				unit->next_target_waypoint.x = unit->order_target.pt.x;
				unit->next_target_waypoint.y = unit->order_target.pt.y;
			}
			if (bMoveTargetResult)
			{
				unit->main_order_state = 1;
			}
		}
	}

	char refresh_screen(s32 ecx0) // 0048DDC0
	{
		auto v6 = ecx0;
		auto result = *g_is_placing_building;
		if (*g_is_placing_building)
		{
			auto y = mouse->y;
			auto x = mouse->x;
			auto mouse_pos = Point16{(u16)mouse->x, (u16)mouse->y};
			if (helper::init_placement_box_pos(&mouse_pos))
			{
				*g_place_building_message_id = helper::update_placement_state_for_placement_box();
				helper::refresh_layers_3_and_4();
				Point16 buildPoint;
				buildPoint.x = x;
				buildPoint.y = y;
				helper::unk_building_placement_related(1, &buildPoint);
				if (*is_placing_addon)
				{
					auto v4 = *place_building_unit_id - UnitId::TerranCommandCenter;
					Point16 addon_point;
					addon_point.x = units_dat::AddonPlacement[v4].x;
					addon_point.y = units_dat::AddonPlacement[v4].y;
					scbw::adjust_addon_coordinates(*place_building_unit_id, 0, &addon_point.x, &addon_point.y);
					helper::unk_building_placement_related(0, &addon_point);
				}
				result = helper::refresh_layers_3_and_4();
			}
			else
			{
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
	void btnsact_place_addon(u8 unit_id, char* buffer) { // 00423D10
		bool errored = false;
		*unit_iterator_pos = 0;
		while (Unit* unit = helper::get_next_commanded_unit()) {
			u8 player = unit->player;
			g_ai_minerals_reserve[player] = units_dat::mineral_cost[unit_id];
			g_ai_gas_reserve[player] = units_dat::gas_cost[unit_id];
			if (helper::has_supplies_for_unit(player, unit_id, 1)) {
				if (g_game_minerals[player] >= g_ai_minerals_reserve[player]) {
					if (g_game_vespene[player] >= g_ai_gas_reserve[player]) {
						helper::issue_place_building_order(buffer, unit_id, OrderId::PlaceAddon);

						// For MBS this would essentially require custom game command, for now we just have to disable it :( -Veeq7
						/*
						if ((*g_is_holding_shift || unit->status & UnitStatus::InAir) && *g_client_selection_count <= 1) {
							helper::issue_place_building_order(buffer, unit_id, OrderId::PlaceAddon);
						} else if (!(unit->status & UnitStatus::InAir)) {
							BuildCommand command = {};
							command.id = 12;
							command.order_id = OrderId::PlaceAddon;
							command.tile_x = (unit->position.x - units_dat::AddonPlacement[unit_id].x) / 32;
							command.tile_y = (unit->position.y - units_dat::AddonPlacement[unit_id].y) / 32;
							command.unit_id = unit_id;
							helper::queue_game_command((u8*)&command, sizeof(command));
						}
						*/
					} else if (!errored) {
						errored = true;
						helper::display_error_message_id_with_sfx(player, 851, console_index[150]);
					}
				} else if (!errored) {
					errored = true;
					helper::display_error_message_id_with_sfx(player, 850, console_index[147]);
				}
			}
		}
	}

	// Hooked to play the correct new_sound when Taloses complete a structure. -Pr0nogo
	void play_building_complete_sound(Unit* unit) { // 0048F310
		if (!*g_is_in_replay && unit->player == *g_local_nation_id) {
			u32 race = unit->get_race();
			switch (race) {
				case RaceId::Zerg: {
					if (unit->create_origin == UnitId::ZergDroleth) {
						helper::play_sound_from_unit(135, unit, (*g_options_list).flags & 1, 99);
						helper::do_unit_event_notification(unit, 17, sfx_dat::PortraitDuration[135] + 1000 * (building_complete_something[0] / 1000), 1, 135);
					} else if (unit->create_origin == UnitId::ZergGosvileth) {
						helper::play_sound_from_unit(135, unit, (*g_options_list).flags & 1, 99);
						helper::do_unit_event_notification(unit, 17, sfx_dat::PortraitDuration[135] + 1000 * (building_complete_something[0] / 1000), 1, 135);
					} else {
						// Nydus exit and morphs?
						helper::play_sound_from_unit(135, unit, (*g_options_list).flags & 1, 99);
						helper::do_unit_event_notification(unit, 17, sfx_dat::PortraitDuration[135] + 1000 * (building_complete_something[0] / 1000), 1, 135);
					}
				} break;
				case RaceId::Terran: {
					u32 sound_id = 126;
					if (unit->id == UnitId::TerranMason || unit->id == UnitId::TerranTalos)
					{
						sound_id = 136;
					}
					helper::play_sound_from_unit(sound_id, unit, g_options_list->flags & 1, 99);
					helper::do_unit_event_notification(unit, 17, sfx_dat::PortraitDuration[sound_id] + 1000 * (SFXDAT_SoundData[sound_id].duration_ms / 1000), 1, sound_id);
				} break;
				case RaceId::Protoss:
				default: {
					helper::do_unit_event_notification(unit, 17, sfx_dat::PortraitDuration[135] + 1000 * (building_complete_something[0] / 1000), 1, 135);
				} break;
			}
		}
	}
}