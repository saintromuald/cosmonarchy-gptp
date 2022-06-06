#include "load_unload_orders.h"
#include <SCBW/api.h>
#include <logger.h>
#include "globals.h"
#include "hooks\limits\ccmu.h"
#define WRITE_TO_LOG(x)GPTP::logger<<x<<std::endl

namespace hooks
{
	// unit is a transport (overlord, shuttle, dropship)
	// this is the initial pickup order
	void orders_Pickup1(Unit* unit)
	{
		Unit* unitToLoad = unit->order_target.unit;
		if (unitToLoad != nullptr
			&& scbw::can_be_entered_by(unit, unitToLoad))
		{
			helper::prepend_order_targeting_unit(unit, unitToLoad, OrderId::Pickup2);
			prepare_for_next_order(unit);
		}
		else if (unit->unit_ai != nullptr)
		{
			Point16 pos = {0,0};
			unit->user_action_flags |= 1;
			if (unit->main_order_id != OrderId::Die)
			{
				bool bStopLoop = (unit->order_queue_tail == nullptr);
				while (!bStopLoop)
				{
					u8 orderId = (u8)unit->order_queue_tail->orderId;
					if (orders_dat::CanBeInterrupted[orderId] == 0
						&& orderId != OrderId::ComputerAI)
					{
						bStopLoop = true;
					}

					if (!bStopLoop)
					{
						helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
						bStopLoop = (unit->order_queue_tail == nullptr);
					}
				}
				unit->perform_another_order(OrderId::ComputerAI, 0, 0, nullptr, UnitId::None);
			}
			prepare_for_next_order(unit);
		}
	}

	// unit is a non-mobile transport (bunker)
	void orders_Pickup3_0(Unit* unit)
	{
		Unit* unitToLoad = unit->order_target.unit;
		if (unitToLoad == nullptr)
		{
			if (unit->order_queue_head != nullptr)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			}
			else
			{
				if (unit->unit_ai != nullptr)
				{
					unit->order_computer(OrderId::ComputerAI);
				}
				else
				{
					unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
		else
		{ // 004E73F4
			if (scbw::can_be_entered_by(unit, unitToLoad))
			{
				unitToLoad->order_to(OrderId::EnterTransport, unit);
			}
			unit->order_to_idle();
		}
	}

	// unit is a transport (overlord,shuttle,dropship)
	// this is used after orders_Pickup1
	void orders_Pickup2(Unit* unit)
	{
		Unit* unitToLoad = unit->order_target.unit;
		if (unitToLoad == nullptr)
		{
			if (unit->order_queue_head != nullptr)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			}
			else
			{
				if (unit->unit_ai != nullptr)
				{
					unit->order_computer(OrderId::ComputerAI);
				}
				else
				{
					unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
		else
		{ // 004E761F
			if (unitToLoad->unit_ai == nullptr
				|| unitToLoad->main_order_id == OrderId::Pickup4)
			{ // 004E762F
				if (unit->main_order_state == 0)
				{
					if (scbw::can_be_entered_by(unit, unitToLoad))
					{
						if (unit->main_order_timer == 0)
						{
							unit->main_order_timer = 7;
							if (!unit->is_target_within_min_range(unitToLoad, 1))
							{
								helper::order_to_move_to_target(unit, unitToLoad);
							}
							else
							{
								helper::make_to_hold_position(unit);
								if (unitToLoad->main_order_id != OrderId::EnterTransport
									&& unitToLoad->main_order_id != OrderId::Pickup4)
								{
									unitToLoad->order_to(OrderId::EnterTransport, unit);
								}
								if (unit->unit_ai != nullptr)
								{
									unit->main_order_state = 1;
								}
								else
								{
									helper::prepend_order_targeting_unit(unit, unitToLoad, OrderId::Pickup1);
									prepare_for_next_order(unit);
								}
							}
						}
					}
					else
					{
						unit->order_to_idle();
					}
				}
			}
			else // 004E7642
			{
				unit->order_to_idle();
			}
		}
	}

	// unit is a transport
	// a big part of the code could not be tested since different ground height test never occured during testing
	void orders_MoveUnload(Unit* unit)
	{
		u32 distance;
		if (unit->main_order_state == 0 &&
			unit->order_target.unit != nullptr)
		{
			unit->main_order_state = 1;
			unit->order_target.pt.x = (unit->order_target.unit)->sprite->position.x;
			unit->order_target.pt.y = (unit->order_target.unit)->sprite->position.y;
			unit->order_target.unit = nullptr;
		}

		// 004E7739
		distance = scbw::get_distance_fast(unit->order_target.pt.x * 256, unit->order_target.pt.y * 256, unit->halt.x, unit->halt.y);
		distance &= 0xFFFFFF00;
		if (distance <= 0x1000)
		{
			if (unit->order_queue_head != nullptr)
			{
				helper::insert_order_before(unit, unit->order_queue_head, OrderId::Unload, 0, 0, nullptr, UnitId::None);
				prepare_for_next_order(unit);
			}
			else
			{
				unit->perform_another_order(OrderId::Unload, 0, 0, nullptr, UnitId::None);
				prepare_for_next_order(unit);
			}
		}
		else
		{ // 004E77B4
			helper::set_move_target_xy(unit, unit->order_target.pt.x, unit->order_target.pt.y);
			if (unit->order_target.pt.x != unit->next_target_waypoint.x
				|| unit->order_target.pt.y != unit->next_target_waypoint.y)
			{
				unit->next_target_waypoint.x = unit->order_target.pt.x;
				unit->next_target_waypoint.y = unit->order_target.pt.y;
			}
			if (unit->unit_ai != nullptr)
			{
				u32 groundHeightAtPxPos1 = scbw::get_ground_height_at_pos(unit->sprite->position.x, unit->sprite->position.y);
				if (groundHeightAtPxPos1 != scbw::get_ground_height_at_pos(unit->order_target.pt.x, unit->order_target.pt.y))
				{
					distance = scbw::get_distance_fast(unit->order_target.pt.x * 256, unit->order_target.pt.y * 256, unit->halt.x, unit->halt.y);
					distance &= 0xFFFFFF00;
					if (distance <= 0xCA000
						&& helper::ai_are_on_connected_regions(unit, unit->order_target.pt.x, unit->order_target.pt.y) != 0
						&& helper::get_first_loaded_worker(unit) == 0)
					{
						s32 index = 0;
						bool bStopLoop = false;
						bool bStopAll = false;
						bool bIsDistanceGreaterThanHaltDistance = helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 512);
						Point16 pos = {0,0}; // unsure about this start value
						while (!bStopLoop)
						{
							Unit* unitToUnload = helper::get_loaded_unit_from_index(unit, index);
							if (unitToUnload != nullptr)
							{
								if (unitToUnload->id == UnitId::ProtossAccantor && bIsDistanceGreaterThanHaltDistance)
								{
									bStopLoop = true;
								}
								else if (helper::canUnload(unit, unitToUnload, &pos) == 0)
								{
									bStopAll = true;
									bStopLoop = true;
								}
							}
							// 004E78BC
							if (!bStopAll)
							{
								index++;
								bStopLoop = (index < 8);
							}
						}
						if (!bStopAll)
						{
							helper::order_immediate(unit, OrderId::Unload);
							prepare_for_next_order(unit);
						}
					}
				}
			}
		}
	}

	// unit is a unit to pick up by transport
	// COULD NOT BE TESTED (tried bunker and all and nothing called this function)
	void orders_Pickup4_0(Unit* unit)
	{
		scbw::print("hello");
		Unit* unitVar = unit;
		bool bIsNeutral = unit->player == 11;
		char unitStatus = unit->status;
		if (bIsNeutral)
		{
			scbw::print("is_neutral");
			u8 idleOrder;
			if (unitStatus & 0x40)
			{
				idleOrder = OrderId::Nothing2;
			}
			else
			{
				idleOrder = units_dat::ReturnToIdleOrder[(u16)unitVar->id];
			}
			unitVar->user_action_flags |= 1u;
			helper::order(1, unitVar, idleOrder, 0, 0, 228);
			helper::orders_nothing_2(unitVar);
			return;
		}
		if (!(unitStatus & 0x40) && !helper::ai_update_attack_target(unitVar, 0, 0, 1))
		{
			scbw::print("order_target");
			Unit* order_target = unitVar->order_target.unit;
			Unit** orderTargetOffset = &unitVar->order_target.unit;
			if (order_target)
			{
				if (!helper::unit_is_active_transport(order_target) || order_target->order_target.unit != unitVar)
				{
					scbw::print("goto label16");
					goto LABEL_16;
				}
			}
			else
			{
				if (!helper::ai_find_transport(unitVar, &unitVar->order_target.unit))
				{
					scbw::print("toIdle / return");
					helper::to_idle(unitVar);
					return;
				}
				order_target = *orderTargetOffset;
				if (!*orderTargetOffset)
				{
					scbw::print("return");
					return;
				}
			}
			if (!canEnterTransport(order_target, unitVar))
			{
			LABEL_16:
				scbw::print("can't enter transport");
				*orderTargetOffset = 0;
				return;
			}
			if (helper::is_target_within_min_range(unitVar, 1u, order_target))
			{
				scbw::print("target within min range");
				helper::load_unit_proc(order_target, unitVar);
				helper::issue_order_targeting_ground(order_target, OrderId::MoveUnload, unitVar->order_target.pt.x, unitVar->order_target.pt.y);
				if (order_target->unit_ai)
				{
					scbw::print("has AI");
					helper::do_order(order_target, OrderId::Move, order_target->sprite->position.x, order_target->sprite->position.y, 0);
				}
			}
			else
			{
				scbw::print("target not within min range");
				helper::prepend_order_targeting_unit(unitVar, order_target, 0x5Eu);
				helper::orders_nothing_2(order_target);
				if (helper::ai_are_on_connected_regions(unitVar, order_target->sprite->position.x, order_target->sprite->position.y))
				{
					scbw::print("set waypoint");
					helper::set_secondary_waypoint(unitVar, order_target->sprite->position.x, order_target->sprite->position.y);
					helper::moveTowards(order_target, unitVar);
				}
			}
		}

		/*
		if (unit->player == 11)
		{
			unit->user_action_flags |= 1;
			if (unit->status & UnitStatus::InTransport)
			{
				unit->order(OrderId::Nothing2, 0, 0, nullptr, UnitId::None, 1);
			}
			else
			{
				unit->order(units_dat::ReturnToIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, 1);
			}
			prepare_for_next_order(unit);
		}
		else if (!(unit->status & UnitStatus::InTransport)
				&& helper::ai_update_attack_target(unit, 1, 0, 0) == 0)
		{ // 004E7BEB
			Unit* target = unit->order_target.unit;
			bool jump_to_E7C2B = false;
			bool jump_to_E7C37 = false;
			bool jump_to_E7C44 = false;
			bool bEndThere = false;
			if (target == nullptr)
			{
				if (helper::ai_find_transport(unit, &target) == 0)
				{
					unit->order_to_idle();
					bEndThere = true;
				}
				else
				if (target != nullptr)
				{
					jump_to_E7C2B = true;
				}
			}
			if (!bEndThere)
			{
				if (!jump_to_E7C2B)
				{ // 004E7C1B
					if (target != nullptr
						&& (!helper::unit_is_active_transport(target) || target->order_target.unit != unit))
					{
						jump_to_E7C37 = true;
					}
					else
					{
						jump_to_E7C2B = true;
					}
				}
				if (jump_to_E7C2B)
				{
					if (scbw::can_be_entered_by(target, unit))
					{
						jump_to_E7C44 = true;
					}
					else
					{
						jump_to_E7C37 = true;
					}
				}
				if (jump_to_E7C37)
				{
					unit->order_target.unit = nullptr;
				}
				if (jump_to_E7C44)
				{
					if (target != nullptr)
					{
						if (unit->is_target_within_min_range(target, 1))
						{ // 004E7C52
							helper::load_unit_proc(target, unit);
							target->order_to(OrderId::MoveUnload, unit->order_target.pt.x, unit->order_target.pt.y);
							if (target->pAI != nullptr)
							{
								helper::do_order(target, OrderId::Move, target->sprite->position.x, target->sprite->position.y, false);
							}
						}
						else
						{ // 004E7C94
							helper::prepend_order_targeting_unit(target, unit, OrderId::Pickup2);
							prepare_for_next_order(target);
							if (helper::ai_are_on_connected_regions(unit, target->sprite->position.x, target->sprite->position.y) != 0)
							{
								helper::set_secondary_waypoint(unit, target->sprite->position.x, target->sprite->position.y);
								helper::moveTowards(unit, target);
							}
						}
					}
				}
			}
		}
		*/
	}

	// unit is a unit aiming to enter a transport
	void orders_EnterTransport(Unit* unit)
	{
		Unit* target = unit->order_target.unit;
		if (target != nullptr
			&& !(target->status & UnitStatus::IsHallucination)
			&& units_dat::SpaceProvided[target->id] != 0
			&& scbw::can_be_entered_by(target, unit))
		{ // 004E7D51

			if (unit->main_order_state == 0
				&& target->main_order_id != OrderId::Pickup2)
			{
				if (target->status & UnitStatus::IsBuilding)
				{
					helper::prepend_order_targeting_unit(target, unit, OrderId::Pickup2);
					prepare_for_next_order(target);
				}
				unit->main_order_state = 1;
			}
			if (target->sprite->position.x != unit->next_target_waypoint.x ||
				target->sprite->position.y != unit->next_target_waypoint.y)
			{
				unit->next_target_waypoint.x = target->sprite->position.x;
				unit->next_target_waypoint.y = target->sprite->position.y;
			}
			helper::moveTowards(unit, target);
			if (unit->is_target_within_min_range(target, 1))
			{
				helper::load_unit_proc(target, unit);
			}
		}
		else
		{
			if (unit->order_queue_head != nullptr)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			}
			else if (unit->unit_ai != nullptr)
			{
				unit->order_computer(OrderId::ComputerAI);
			}
			else
			{
				unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
	}

	void orders_Unload(Unit* unit)
	{
		static Unit* unitTable_0059CB58 = (Unit*)(0x0059CB58);
		static Unit* unitTable_0059CB64 = (Unit*)(0x0059CB64);
		if (globals.extended_unit_table == 1)
		{
			const u32 index_start = reinterpret_cast<u32>(&new_unit_table.units[0]) - 336;
			unitTable_0059CB58 = (Unit*)index_start;
			unitTable_0059CB64 = (Unit*)(index_start + 12);
		}
		if (unit->status & UnitStatus::GroundedBuilding
			|| unit->main_order_timer == 0)
		{
			s32 counter = 0;
			u32 result_E7F70 = 0;
			bool bEndLoop = false;
			bool bFoundUnitToUnload = false;
			u32* loadedUnitOffset;
			bool jump_to_E8191 = false;
			bool jump_to_E8195 = false;
			bool jump_to_E81AD = false;
			while (!bEndLoop)
			{
				loadedUnitOffset = (u32*)&unit->loaded_unit[counter];
				if (*loadedUnitOffset != 0
					&& unitTable_0059CB64[unit->loaded_unit[counter].index].link.prev != nullptr) // sprite check
				{
					Unit* unitToUnload = &(unitTable_0059CB58[unit->loaded_unit[counter].index]);
					if (unitToUnload->main_order_id != 0 || unitToUnload->main_order_state != 1)
					{
						if (unit->loaded_unit[counter].unit_id == unitToUnload->target_order_special)
						{
							bFoundUnitToUnload = true;
							result_E7F70 = helper::unload_unit_proc(unitToUnload);
							if (!(unit->status & UnitStatus::GroundedBuilding))
							{
								bEndLoop = true;
							}
						}
					}
				}

				if (!bEndLoop)
				{
					counter++;
					if (counter >= 8)
					{
						bEndLoop = true;
					}
				}
			}

			if (!bFoundUnitToUnload)
			{
				jump_to_E8191 = true;
			}

			if (!jump_to_E8191)
			{
				if (result_E7F70 != 0)
				{
					jump_to_E8195 = true;
				}
				else if (unit->unit_ai == nullptr
						|| unit->status & UnitStatus::GroundedBuilding)
				{
					jump_to_E81AD = true;
				}
				else
				{
					helper::ai_unload_failure(unit);
				}
			}
			if (jump_to_E8191)
			{
				if (result_E7F70 == 0)
				{
					jump_to_E81AD = true;
				}
				else
				{
					jump_to_E8195 = true;
				}
			}
			if (jump_to_E8195)
			{
				if (unit->status & UnitStatus::GroundedBuilding
					|| !unit->has_loaded_unit())
					jump_to_E81AD = true;
			}
			if (jump_to_E81AD)
			{
				if (unit->order_queue_head != nullptr)
				{
					unit->user_action_flags |= 1;
					prepare_for_next_order(unit);
				}
				else if (unit->unit_ai != nullptr)
				{ // 004E81DE
					unit->user_action_flags |= 1;
					if (unit->main_order_id == OrderId::Die)
					{
						prepare_for_next_order(unit);
					}
					else
					{
						bEndLoop = (unit->order_queue_tail == nullptr);
						while (!bEndLoop)
						{
							if (orders_dat::CanBeInterrupted[(unit->order_queue_tail)->orderId]
								|| unit->order_queue_tail->orderId == OrderId::ComputerAI)
							{
								helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
							}
							else
							{
								bEndLoop = true;
							}
							if (!bEndLoop)
							{
								bEndLoop = (unit->order_queue_tail == nullptr);
							}
						}
						unit->perform_another_order(OrderId::ComputerAI, 0, 0, nullptr, UnitId::None);
						prepare_for_next_order(unit);
					}
				}
				else
				{ // 004E823A
					unit->user_action_flags |= 1;
					unit->order(units_dat::ReturnToIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, true);
					prepare_for_next_order(unit);
				}
			}
		}
	}

	bool canEnterTransport(Unit* transport, Unit* unit)
	{ // 004E6E00
		if (!(transport->status & UnitStatus::Completed))
		{
			return 0;
		}
		if (transport->status & UnitStatus::Frozen)
		{
			return 0;
		}
		if (transport->is_frozen())
		{
			return 0;
		}
		if (unit->status & UnitStatus::Burrowed) // originally also checked for playerIds being the same
		{
			return 0;
		}
		if (!(scbw::is_allied_to(transport->player, unit->player)))
		{
			return 0;
		}
		if (scbw::transport_is_full(transport))
		{
			return 0;
		}
		u32 spaceProvided = units_dat::SpaceProvided[transport->id];
		u32 index = 0;
		s32 spaceRequired = 0;
		Unit* loaded_unit;
		if (spaceProvided)
		{
			do
			{
				loaded_unit = helper::get_loaded_unit_from_index(transport, index);
				if (loaded_unit)
				{
					spaceRequired += units_dat::SpaceRequired[loaded_unit->id];
				}
				++index;
			} while (index < spaceProvided);
			unit = unit;
		}
		return spaceProvided - spaceRequired >= units_dat::SpaceRequired[unit->id];
	}
}