#include "rally_point.h"
#include "../SCBW/api.h"
#include "globals.h"

namespace hooks
{
	// Orders newly-produced units to rally, based upon the properties of the building that produced it.
	// @param  unit      The unit that needs to receive rally orders.
	// @param  factory   The unit (building) that created the given unit.
	// Identical to sub_466F50 @ 00466F50

	void order_new_unit_to_rally(Unit* unit, Unit* factory) {
		// Egg rally
		if (unit) {
			if (unit->rally.unit != unit
			&&  unit->rally.unit != factory
			&&  unit->rally.pt.x != 0) {
				// Following should be allowed only on friendly units
				if (unit->rally.unit && scbw::is_allied_to((unit->rally.unit)->player, unit->player)) {
					unit->order_to(OrderId::Follow, unit->rally.unit);
				}  else {
					unit->order_to(OrderId::Move, unit->rally.pt.x, unit->rally.pt.y);
				}
				unit->rally = {};
				return;
			}
		}
		// Do nothing if the rally target is the factory itself or the rally target position is 0
		if (factory
		&&  factory->is_factory()
		&&  factory->rally.unit != unit
		&&  factory->rally.unit != factory
		&&  factory->rally.pt.x != 0) {
			// Following should be allowed only on friendly units
			if (factory->rally.unit && scbw::is_allied_to((factory->rally.unit)->player, unit->player)) {
				unit->order_to(OrderId::Follow, factory->rally.unit);
			} else {
				unit->order_to(OrderId::Move, factory->rally.pt.x, factory->rally.pt.y);
			}
		}
	}

	// Called when the player sets the rally point on the ground.
	void set_rally_position(Unit* unit, u16 x, u16 y) {
		unit->rally.unit = 0;
		unit->rally.pt.x = x;
		unit->rally.pt.y = y;
	}

	// Called when the player sets the rally point on a unit.
	void set_rally_unit(Unit* unit, Unit* target) {
		if (!target) {
			target = unit;
		}
		unit->rally.unit = target;
		unit->rally.pt.x = target->get_x();
		unit->rally.pt.y = target->get_y();
	}

	// Hooked to allow rally for morphing Larval Colonies - Pr0nogo
	// Possible downstream effect: need to clear rally for the creep colony if the morph cancelled?
	u32 btnscond_rally(u32 unused, u32 player, Unit* unit) // 00429520
	{
		*last_internal_error = 0;
		if (orders_dat::RequirementsOffset[OrderId::RallyPoint2] == -1)
		{
			helper::parse_orders_dat_requirements();
		}

		u32 result = 0;
		if (player == unit->player)
		{
			if (unit->is_frozen())
			{
				*last_internal_error = 10;
				result = 0;
			}
			else if (unit->status & UnitStatus::Burrowed && unit->id != UnitId::ZergLakizilisk) // ?
			{
				*last_internal_error = 11;
				result = 0;
			}
			else
			{
				u16 unit_id = unit->id;
				if (unit_id == UnitId::TerranMason && unit->main_order_id == OrderId::ConstructingBuilding
				  || scbw::unit_can_nuke(unit) && unit->main_order_id == OrderId::NukeTrack
				  || unit_id == UnitId::ProtossAugur && unit->main_order_id == OrderId::CompletingArchonSummon)
				{
					*last_internal_error = 8;
					result = 0;
				}
				else
				{
					result = orders_dat::RequirementsOffset[OrderId::RallyPoint2];
					if (orders_dat::RequirementsOffset[OrderId::RallyPoint2])
					{
						if (unit->id == UnitId::ZergCreepColony && unit->build_queue[unit->build_queue_slot] == UnitId::ZergLarvalColony) // lame
						{
							result = 1;
						}
						else
						{
							result = helper::parse_requirement_opcodes(unit, OrderId::RallyPoint2, player, (u32)orders_requirement_tree, orders_dat::RequirementsOffset[OrderId::RallyPoint2]);
						}
					}
					else
					{
						*last_internal_error = 23;
					}
				}
			}
		}
		else
		{
			*last_internal_error = 1;
			result = 0;
		}
		return result;
	}

	Order* remove_references(Unit* unit, Unit* dying) // 0049E590
	{
		if (unit->current_build_unit == dying)
		{
			unit->current_build_unit = 0;
		}
		if (unit->order_target.unit == dying)
		{
			unit->order_target.unit = 0;
		}
		if (unit->move_target.unit == dying)
		{
			unit->move_target.unit = 0;
		}
		if (unit->auto_target_unit == dying)
		{
			unit->auto_target_unit = 0;
		}
		if (unit->connected_unit == dying)
		{
			unit->connected_unit = 0;
		}
		if (unit->irradiate_caster == dying)
		{
			unit->irradiate_caster = 0;
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
		{
			if (unit->worker.target_resource.unit == dying)
			{
				unit->worker.target_resource.unit = 0;
			}
			if (unit->worker.is_harvesting)
			{
				if (unit->worker.harvest_target == dying)
				{
					unit->worker.is_harvesting = 0;
					unit->worker.harvest_target = 0;
				}
			}
		}
		else if (unit->status & UnitStatus::GroundedBuilding)
		{
			if (unit->building.addon == dying)
			{
				unit->building.addon = 0;
			}
		}
		if (unit->is_factory())
		{
			UnitExt* unitExt = globals.get_unit_ext(unit);
			if (unit->rally.unit == dying)
			{
				unit->rally.unit = 0;
				unit->rally.pt.x = dying->sprite->position.x;
				unit->rally.pt.y = dying->sprite->position.y;
			}
		}
		if (unit->subunit)
		{
			if (units_dat::BaseProperty[unit->subunit->id] & UnitProperty::Subunit)
			{
				remove_references(unit->subunit, dying);
			}
		}
		Order* result = unit->order_queue_head;
		while (result)
		{
			Order* next = result->next;
			if (result->target.unit == dying)
			{
				helper::remove_order_from_unit_queue(result, unit);
			}
			result = next;
		}
		return 0; // used to return result, but it will always be 0. The return value is never used anyways. -Veeq7
	}

	Unit* right_click_command(u32 x, u32 y, Unit *target, u32 fow_unit_id, char queued) // 004560D0
	{
		MovementGroup movement_group = {};
		movement_group.pos.x = x;
		movement_group.pos.y = y;
		helper::prepare_formation_movement(&movement_group, 1);
		if (target)
		{
			fow_unit_id = UnitId::None;
		}
		else if (fow_unit_id != UnitId::None)
		{
			u32 tileId = x / 32 + map_tile_size->width * (y / 32);
			if (!((1 << *ACTIVE_NATION_ID) & (*activeTileArrayInt)[tileId]))
			{
				target = helper::find_fow_unit(y, x, fow_unit_id);
				fow_unit_id = UnitId::None;
			}
		}
		*unit_iterator_pos = 0;
		Unit* result = helper::get_next_commanded_unit();
		u8 order_type;
		for (Unit* unit = result; result; unit = result)
		{
			u16 id = unit->id;
			if (id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
			{
				order_type = 3;
			}
			else
			{
				order_type = units_dat::RightClickAction[id];
				if (!order_type)
				{
					if (unit->status & UnitStatus::GroundedBuilding)
					{
						if (unit->is_factory())
						{
							order_type = 2;
						}
					}
				}
			}
			if (unit->status & UnitStatus::IsHallucination)
			{
				if (order_type == 4 || order_type == 5)
				{
					order_type = 1;
				}
			}
			unk_order_array[0] = OrderId::Nothing2;
			u8 rightOrder = helper::get_right_click_order(order_type, unit, x, y, &target, fow_unit_id);
			if (rightOrder != OrderId::Nothing2)
			{
				if (rightOrder == OrderId::RallyPoint1)
				{
					if (unit->is_factory())
					{
						Unit* rallyTarget = target;
						if (!rallyTarget)
						{
							rallyTarget = unit;
						}

						UnitExt* unitExt = globals.get_unit_ext(unit);
						unit->rally.unit = rallyTarget;
						unit->rally.pt.x = rallyTarget->sprite->position.x;
						unit->rally.pt.y = rallyTarget->sprite->position.y;
					}
				}
				else if (unit != target)
				{
					if (rightOrder == OrderId::RallyPoint2)
					{
						if (unit->is_factory() || unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon)
						{
							UnitExt* unitExt = globals.get_unit_ext(unit);
							unit->rally.unit = 0;
							unit->rally.pt.x = x;
							unit->rally.pt.y = y;
						}
					}
					else
					{
						if (rightOrder == OrderId::Attack1)
						{
							Unit* subunit = unit->subunit;
							if (subunit)
							{
								helper::targeted_order_unit(target, unit->subunit, queued != 0, units_dat::AttackUnitOrder[subunit->id]);
							}
							rightOrder = units_dat::AttackUnitOrder[unit->id];
						}
						if (helper::order_is_allowed(unit, rightOrder, *ACTIVE_NATION_ID) == 1)
						{
							if (target)
							{
								helper::targeted_order_unit(target, unit, queued != 0, rightOrder);
								if (unk_order_array[0] != OrderId::Nothing2)
								{
									helper::targeted_order_unit(target, unit, queued != 0, unk_order_array[0]);
								}
							}
							else
							{
								movement_group.pos.x = x;
								movement_group.pos.y = y;
								if (fow_unit_id == UnitId::None)
								{
									helper::get_formation_movement_target(unit, &movement_group);
								}
								else
								{
									movement_group.current_unit_target = movement_group.pos;
								}

								u16 target_x = movement_group.current_unit_target.x;
								u16 target_y = movement_group.current_unit_target.y;
								helper::targeted_order_ground(rightOrder, fow_unit_id, unit, queued != 0, target_x, target_y);
								if (unk_order_array[0] != OrderId::Nothing2)
								{
									helper::targeted_order_ground(unk_order_array[0], fow_unit_id, unit, queued != 0, target_x, target_y);
								}
							}
							unit->user_action_flags |= 2u;
						}
					}
				}
			}
			result = helper:: get_next_commanded_unit();
		}
		return 0;
	}

	bool unit_can_have_rally(Unit* unit) { // 0047B2E0
		return units_dat::GroupFlags[unit->id].is_factory
			|| unit->id == UnitId::ZergEgg
			|| unit->id == UnitId::ZergCocoon;
	}

	bool unit_can_rally(Unit* unit) {
		if (unit_can_have_rally(unit)) {
			if (units_dat::BaseProperty[unit->id] & UnitProperty::Building) {
				return unit->status & UnitStatus::GroundedBuilding;
			}
			return true;
		}
		return false;
	}

	// Original functonality for pack/unpack, required to be able to read/save maps and saves properly. DO NOT CHANGE!!!
	bool unit_can_have_rally_original(Unit* unit) {
		return unit->id == UnitId::TerranCommandCenter
			|| unit->id == UnitId::TerranBarracks
			|| unit->id == UnitId::TerranFulcrum
			|| unit->id == UnitId::TerranStarport
			|| unit->id == UnitId::ZergInfestedCommandCenter
			|| unit->id == UnitId::ZergHatchery
			|| unit->id == UnitId::ZergLair
			|| unit->id == UnitId::ZergHive
			|| unit->id == UnitId::ProtossNexus
			|| unit->id == UnitId::ProtossGateway
			|| unit->id == UnitId::ProtossStargate
			|| unit->id == UnitId::ProtossRoboticsFacility;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	u32 order_acquire_generic_right_click_unit_proc_2(Unit* unit, u32 x_pos, u16 y_pos, Unit** target, u16 fow_unit_id) { // 00455E00
		u32 result = OrderId::Nothing2;
		if (unit->status & UnitStatus::GroundedBuilding || unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon) {
			if (hooks::unit_can_rally(unit)) {
				result = OrderId::RallyPoint2 - (*target != 0);
			}
			return result;
		}

		Unit* target_unit = *target;
		if (!*target) {
			if ((unit->id == UnitId::ZergMatraleth) && fow_unit_id != UnitId::None)
			{
				return OrderId::Rightclickaction;
			}
			return OrderId::Move;
		}
		if (helper::unit_is_active_transport(unit) && helper::is_order_valid(OrderId::Pickup2, unit, 0, target_unit, 0)) {
			return OrderId::Pickup2;
		}
		if (helper::unit_is_active_transport(target_unit) && helper::can_enter_transport(target_unit, unit)) {
			return OrderId::EnterTransport;
		}
		if (target_unit->status & UnitStatus::Burrowed) {
			return OrderId::Move;
		}

		if (unit->id == UnitId::ZergMatraleth
		&& !(target_unit->status & UnitStatus::Invincible)
		&& helper::get_target_unit_ability_error_stat_string(target_unit, unit->player, 12) == 0)
		{
			return OrderId::InfestMine2;
		}
		if (unit->id == UnitId::TerranCleric) {
			return OrderId::HealMove;
		}
		return OrderId::Follow;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	void process_right_click_action(Unit* unit, u32 order_unit_id, u16 y_pos, u32 x_pos, Unit* target) { // 00455F20
		Box16 location = {};
		location.x = x_pos - 96;
		location.w = x_pos + 96;
		location.h = y_pos + 96;
		location.y = y_pos - 96;
		unk_order_array[0] = 23;
		*is_unit_in_point_x_pos = x_pos;
		*is_unit_in_point_y_pos = y_pos;
		u8 idle_order = helper::get_generic_unit_idle_order(order_unit_id, unit);
		Unit* unit_in_point = helper::iterate_units_at_location_target_proc_in_point(&location, target);
		u8 right_click_order = helper::get_right_click_order(idle_order, unit_in_point, x_pos, y_pos, &unit_in_point, UnitId::None);
		if (right_click_order == OrderId::Nothing2) {
			helper::set_resource_target(unit);
			u32 x_move_target = unit->move_target.pt.x;
			u32 y_move_target = unit->move_target.pt.y;
			if (x_move_target != unit->next_target_waypoint.x || y_move_target != unit->next_target_waypoint.y) {
				unit->next_target_waypoint.x = x_move_target;
				unit->next_target_waypoint.y = y_move_target;
			}
			if (unit->order_queue_head) {
				unit->user_action_flags |= 1u;
				helper::orders_nothing_2(unit);
			} else if (unit->unit_ai) {
				helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
			} else {
				helper::issue_order_targeting_nothing(unit, units_dat::ReturnToIdleOrder[unit->id]);
			}
		} else {
			if (right_click_order == 8) {
				right_click_order = units_dat::AttackUnitOrder[unit->id];
			}
			if (helper::order_allowed(unit, right_click_order, unit->player) == 1) {
				if (right_click_order == OrderId::RallyPoint1)
				{
					if (hooks::unit_can_rally(unit))
					{
						helper::set_rally_unit(unit_in_point, unit);
					}
				} else if (right_click_order == OrderId::RallyPoint2) {
					if (hooks::unit_can_rally(unit))
					{
						helper::set_rally_position(unit, x_pos, y_pos);
					}
				} else {
					if (unit_in_point) {
						helper::prepend_order_targeting_unit(unit_in_point, unit, right_click_order);
					} else {
						helper::prepend_order_targeting_ground(right_click_order, x_pos, y_pos, unit);
					}
					helper::to_idle(unit);
				}
			}
		}
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	u8 get_generic_unit_idle_order(u32 order_unit_id, Unit* unit) { // 00455760
		if (!unit) {
			return OrderId::Die;
		}

		u8 result = 0;
		if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed) {
			result = OrderId::PlayerGuard;
		} else {
			u8 right_click_order = units_dat::RightClickAction[unit->id];
			if (!right_click_order) {
				if (hooks::unit_can_rally(unit)) {
					right_click_order = OrderId::Guard;
				}
			}
			result = right_click_order;
		}
		if (unit->status & UnitStatus::IsHallucination) {
			if (result == OrderId::TurretGuard || result == OrderId::BunkerGuard) {
				result = OrderId::Stop;
			}
		}
		return result;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	void input_game_right_mouse_click(DialogEvent* dlg_event) { // 004564E0
		if (!helper::is_outside_of_game_screen(dlg_event->current.x, dlg_event->current.y)) {
			u8 local_group_owner = local_selection_group[0] ? local_selection_group[0]->player : 255;
			if (local_group_owner == *LOCAL_NATION_ID) {
				if (*active_portrait_unit) {
					if (helper::unit_is_owned_by_current_player(*active_portrait_unit)) {
						s32 selection_iterator = 0;
						for (u32 i = 0; i < SelectionCap; ++i) {
							Unit* selected_unit = *client_selection_group[selection_iterator].unit;
							if (selected_unit) {
								if (helper::accepts_right_click_commands(selected_unit)) {
									break;
								}
							}
							if (i == SelectionCap - 1) {
								return;
							}
						}
						if (selection_iterator < SelectionCap) {
							u32 find_x = *move_to_x + dlg_event->current.x;
							u32 find_y = *move_to_y + dlg_event->current.y;
							Unit* unit_at_point = helper::find_unit_at_point(find_x, find_y);
							if (*client_selection_count != 1
							|| *active_portrait_unit != unit_at_point
							|| unit_at_point && hooks::unit_can_rally(unit_at_point)) {
								if (unit_at_point) {
									goto exit;
								}
								for (u32 i = 0; i < SelectionCap; ++i) {
									Unit* unit = client_selection_group->unit[i];
									if (unit) {
										if (helper::can_move(unit)) {
											if (i < 12) {
											exit:
												Thingy* command_thingy = helper::show_command_response(find_x, find_y, unit_at_point);
												u16 unit_id = UnitId::None;
												if (command_thingy) {
													unit_id = command_thingy->unit_id;
												}
												helper::cmdact_right_click_order(unit_at_point, find_x, find_y, unit_id, IS_KEYCODE_USED[16]);
												if (helper::show_right_click_error_if_needed(unit_at_point)) {
													helper::play_worker_action_sound(*active_portrait_unit);
												}
											}
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	u32 can_do_targeted_order(Unit* unit, u8 order_id) { // 0049A410
		if (unit->status & UnitStatus::GroundedBuilding
		&& (order_id == OrderId::RallyPoint1
		||  order_id == OrderId::RallyPoint2
		||  order_id == OrderId::Rechargeshields2
		||  order_id == OrderId::Pickup3)) {
			return 1;
		}
		if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed) {
			return 1;
		}
		u8 right_click_order = units_dat::RightClickAction[unit->id];
		if (!right_click_order) {
			if (hooks::unit_can_rally(unit)) {
				right_click_order = 2;
			}
		}
		return right_click_order != 0;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	char get_right_click_action_order(Unit* unit) { // 004E5EA0
		if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed) {
			return 3;
		}

		char right_click_order = units_dat::RightClickAction[unit->id];
		if (!right_click_order) {
			if (hooks::unit_can_rally(unit)) {
				return 2;
			}
		}
		return right_click_order;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	s32 __stdcall btnscond_can_move_special_case(u32 buffer) { // 00428D40 - guessed arg name, unused
		UnitsSel selected_units = *client_selection_group;
		Unit* unit_from_selection = 0;
		for (u32 i = 0; i < SelectionCap; ++i) {
			unit_from_selection = selected_units.unit[i];
			if (unit_from_selection) {
				if (unit_from_selection->id == UnitId::ZergLakizilisk) {
					if (unit_from_selection->status & UnitStatus::Burrowed) {
						return 1;
					}
				}
				if (units_dat::RightClickAction[unit_from_selection->id]
				||  unit_from_selection->status & UnitStatus::GroundedBuilding && unit_can_rally(unit_from_selection)) {
					return 1;
				}
			}
		}
		return 0;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	s32 __stdcall btnscond_can_move(u32 buffer) { // 00428DA0
		Unit** selected_units = client_selection_group->unit;
		Unit* unit_from_selection = 0;
		char right_click_order = 0;
		bool another_pass = false;
		for (u32 i = 0; i < SelectionCap; ++i) {
			unit_from_selection = selected_units[i];
			if (!unit_from_selection) {
				continue;
			}
			if (unit_from_selection->id == UnitId::ZergLakizilisk) {
				if (unit_from_selection->status & UnitStatus::Burrowed) {
					continue;
				}
			}
			right_click_order = units_dat::RightClickAction[unit_from_selection->id];
			if (right_click_order) {
				if (right_click_order == OrderId::PlayerGuard) {
					continue;
				}
				return 1;
			}
			if (unit_from_selection->status & UnitStatus::GroundedBuilding) {
				if (unit_can_rally(unit_from_selection)) {
					return true;
				}
				continue;
			}
		}
		return 0;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	s32 __stdcall btnscond_can_attack(u32 buffer) // 00428F30
	{
		UnitsSel selected = *client_selection_group;
		u32 i = 0;
		while (1)
		{
			Unit* unit_from_selected = selected.unit[i];
			if (unit_from_selected)
			{
				if (unit_from_selected->id == UnitId::ZergLakizilisk && unit_from_selected->status & UnitStatus::Burrowed
				||  units_dat::RightClickAction[unit_from_selected->id]
				||  unit_from_selected->status & UnitStatus::GroundedBuilding && unit_can_rally(unit_from_selected))
				{
					if (helper::ai_unit_can_attack(unit_from_selected))
					{
						break;
					}
				}
			}
			++i;
			if (selected.unit[i] >= client_selection_group_end)
			{
				return 0;
			}
		}
		return 1;
	}
	
	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	s32 unit_can_move(Unit* unit) // 00455660
	{
		u32 result = 0;
		u16 right_click_order = units_dat::RightClickAction[unit->id];
		if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
		{
			right_click_order = OrderId::PlayerGuard;
		}
		else
		{
			if (!right_click_order)
			{
				if (unit->status & UnitStatus::GroundedBuilding)
				{
					if (unit_can_rally(unit))
					{
						right_click_order = 2;
					}
				}
			}
		}
		switch (right_click_order)
		{
			case 1:
			case 2:
			case 4:
			case 5:
			case 6:
			{
				result = 1;
			} break;
			default:
			{
				result = 0;
			} break;
		}
		return result;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	void drone_build_execute(Unit* unit) // 0045D2E0
	{
		Unit* unit_var = unit;
		helper::stop_creep_disappearing_at_building(unit);
		u32 unit_status = unit_var->status;
		if (!unit_can_rally(unit_var))
		{
			if (!(unit_status & UnitStatus::Frozen)
			&&   !unit_var->lockdown_timer
			&&   !unit_var->stasis_timer
			&&   !unit_var->maelstrom_timer
			||    units_dat::BaseProperty[unit_var->id] & UnitProperty::Building)
			{
				unit_var->current_button_set = 233;
			}
		}
		else if (!(unit_status & UnitStatus::Frozen)
			&& !unit_var->lockdown_timer
			&& !unit_var->stasis_timer
			&& !unit_var->maelstrom_timer
			|| units_dat::BaseProperty[unit_var->id] & UnitProperty::Building)
		{
			unit_var->current_button_set = 234;
		}

		helper::init_building(unit_var, 1);
		char main_order_id = unit_var->main_order_id;
		char action_flags = unit_var->user_action_flags | 1;
		unit_var->hitpoints = units_dat::MaxHitPoints[unit_var->id] / 10;
		unit_var->user_action_flags = action_flags;
		if (main_order_id)
		{
			while (1)
			{
				Order* order_queue_tail = unit_var->order_queue_tail;
				if (!order_queue_tail)
				{
					break;
				}
				u8 order_id = order_queue_tail->order_id;
				if (!orders_dat::CanBeInterrupted[order_id] && order_id != OrderId::ZergBirth)
				{
					break;
				}
				helper::remove_order_from_unit_queue(order_queue_tail, unit_var);
			}
			helper::perform_another_order(OrderId::ZergBuildSelf, 0, unit_var, UnitId::None, {}, 0);
		}
		helper::orders_nothing_2(unit_var);
		unit_var->build_queue[unit_var->build_queue_slot] = UnitId::None;
	}
}