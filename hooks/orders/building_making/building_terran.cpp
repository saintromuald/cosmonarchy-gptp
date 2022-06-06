#include "building_terran.h"
#include "building_modifier.h"
#include <SCBW/api.h>
#include <globals.h>

namespace hooks
{
	void orders_terran_build_self(Unit* building)
	{
		if (building->main_order_state == 0)
		{
			if (building->hitpoints > 20 * units_dat::MaxHitPoints[building->id] / 100)
				building->main_order_state = 1;
		}
		else if (building->main_order_state == 1)
		{
			Image* current_image = building->sprite->images.head;
			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
				current_image = current_image->link.next;
			}
			building->main_order_state = 2;
		}
		else if (building->main_order_state == 2)
		{
			if (building->hitpoints > 40 * units_dat::MaxHitPoints[building->id] / 100)
				building->main_order_state = 3;
		}
		else if (building->main_order_state == 3)
		{
			Image* current_image = building->sprite->images.head;
			while (current_image != nullptr)
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
			while (current_image != nullptr)
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

	// Go to build on an existing building
	void orders_scv_build_2(Unit* unit)
	{
		Unit* builtUnit = unit->order_target.unit;

		BuildingModifier modifier = get_building_modifier(builtUnit, unit, true);

		if (builtUnit != nullptr && !(builtUnit->status & UnitStatus::Completed))
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
					if (builtUnit->connected_unit != nullptr &&
						builtUnit->connected_unit != unit &&
						(builtUnit->connected_unit)->order_target.unit == builtUnit)
					{
						unit->order_to_idle(); // another SCV is already on the job
					}
					else // 00467AF9
					{
						builtUnit->connected_unit = unit;
						unit->status &= ~UnitStatus::IsNormal;	//remove UnitStatus::IsNormal if present
						unit->sprite->elevationLevel = units_dat::Elevation[unit->id] + 1;
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
					unit->order_target.pt.x = (angleDistance[angle].x * 20) / 256 + unit->sprite->position.x;
					unit->order_target.pt.y = (angleDistance[angle].y * 20) / 256 + unit->sprite->position.y;
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
					if (angle > weapons_dat::AttackAngle[unit->get_ground_weapon()])
					{
						jump_to_switch_1 = true;
					}
					else // 00467E4A
					{
						u8 rand_value = helper::randomize_short(10);
						Image* current_image = unit->sprite->images.head;
						unit->main_order_timer = (rand_value & 63) + 30;
						while (current_image != nullptr)
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
					while (current_image != nullptr)
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
					while (current_image != nullptr)
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
				UnitExt* unitExt = globals.get_unit_ext(builtUnit);
				if (*CHEAT_STATE & CheatFlags::OperationCwal)
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
					while (current_image != nullptr)
					{
						current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
						current_image = current_image->link.next;
					}
					if (builtUnit != nullptr)
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
						if (!jump_to_67F3C) // 00467F2C
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
					if (!bEndThere) // 00467F4C
					{
						if (!(unit->order_queue_head == nullptr))
						{
							unit->user_action_flags |= 1;
							prepare_for_next_order(unit);
						}
						else if (unit->unit_ai != nullptr) // 00467F6F
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
		else // 00467ED0
		{
			Image* current_image = unit->sprite->images.head;
			bool bEndThere = false;
			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
				current_image = current_image->link.next;
			}
			if (builtUnit != nullptr)
			{
				bool jump_to_67F3C = false;
				builtUnit->connected_unit = nullptr;
				helper::play_building_completed_sound(builtUnit);
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
				if (!jump_to_67F3C) // 00467F2C
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
			if (!bEndThere) // 00467F4C
			{
				if (!(unit->order_queue_head == nullptr))
				{
					unit->user_action_flags |= 1;
					prepare_for_next_order(unit);
				}
				else if (unit->unit_ai != nullptr) // 00467F6F
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

	// Go to build a not yet existing building
	void orders_scv_build(Unit* unit)
	{
		if (unit->unit_ai != nullptr)
		{
			unit->status |= UnitStatus::IsGathering;
		}
		if (unit->main_order_state == 1)
		{
			if (unit->sprite->position.x == unit->move_target.pt.x &&
				unit->sprite->position.y == unit->move_target.pt.y)
			{
				if (!(unit->status & UnitStatus::Unmovable)
					&& helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 128)
					&& helper::begin_building(unit))
				{
					Unit* builtUnit;
					unit->status &= ~UnitStatus::IsNormal;
					unit->sprite->elevationLevel = units_dat::Elevation[unit->id] + 1;
					if (unit->order_queue_head == nullptr) // 004680A5
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
						builtUnit = helper::create_unit(
							unit->build_queue[unit->build_queue_slot],
							unit->order_target.pt.x,
							unit->order_target.pt.y,
							unit->player);
					}
					u32 unit_id = unit->build_queue[unit->build_queue_slot];
					unit->build_queue[unit->build_queue_slot] = UnitId::None;
					if (builtUnit == nullptr)
					{
						helper::refund_unit_train_cost(unit_id, unit->player);
						helper::display_last_network_error_for_player(unit->player);
						unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
					}
					else // 00468131
					{
						builtUnit->connected_unit = unit;
						unit->main_order_id = OrderId::ConstructingBuilding;
						unit->main_order_state = 3;
						unit->order_target.unit = builtUnit;
						if (*is_place_building)
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
		else if (unit->main_order_state == 0) // 00468188
		{
			bool bMoveTargetResult;
			scbw::refresh_console();
			if (unit->order_target.unit != nullptr)
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
		auto result = *is_place_building;
		if (*is_place_building)
		{
			auto y = mouse->y;
			auto x = mouse->x;
			auto mouse_pos = Point16{(u16)mouse->x, (u16)mouse->y};
			if (helper::init_placement_box_pos(&mouse_pos))
			{
				*placeBuildingMessageId = helper::update_placement_state_for_placement_box();
				helper::refresh_layers_3_and_4();
				Point16 buildPoint;
				buildPoint.x = x;
				buildPoint.y = y;
				helper::unk_building_placement_related(1, &buildPoint);
				if (*isPlacingAddon)
				{
					auto v4 = *placeBuildingUnitId - UnitId::TerranCommandCenter;
					Point16 addon_point;
					addon_point.x = units_dat::AddonPlacement[v4].x;
					addon_point.y = units_dat::AddonPlacement[v4].y;
					scbw::adjust_addon_coordinates(*placeBuildingUnitId, 0, &addon_point.x, &addon_point.y);
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

	// Hooked to allow instant addon placement. -Pr0nogo
	void btnsact_place_addon(u8 unit_id, char* buffer) { // 00423D10
		bool errored = false;
		*unit_iterator_pos = 0;
		while (Unit* unit = helper::get_next_commanded_unit()) {
			u8 player = unit->player;
			ai_minerals_reserve[player] = units_dat::mineral_cost[unit_id];
			ai_gas_reserve[player] = units_dat::gas_cost[unit_id];
			if (helper::has_supplies_for_unit(player, unit_id, 1)) {
				if (game_minerals[player] >= ai_minerals_reserve[player]) {
					if (game_vespene[player] >= ai_gas_reserve[player]) {
						if ((*IS_HOLDING_SHIFT || unit->status & UnitStatus::InAir) && *clientSelectionCount <= 1) {
							helper::issue_place_building_order(buffer, unit_id, OrderId::PlaceAddon);
						} else if (!(unit->status & UnitStatus::InAir)) {
							u8 buffer[8] = {};
							buffer[0] = 12;
							buffer[1] = OrderId::PlaceAddon;
							u16* buffer_u16 = (u16*)(buffer + 2);
							buffer_u16[0] = unit->position.x;
							buffer_u16[1] = unit->position.y;
							buffer_u16[2] = unit_id;
							helper::queue_game_command(buffer, sizeof(buffer));
						}
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
}