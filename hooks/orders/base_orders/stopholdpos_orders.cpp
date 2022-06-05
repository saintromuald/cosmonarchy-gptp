#include "stopholdpos_orders.h"
#include <SCBW/api.h>

namespace hooks
{
	void orders_MedicHoldPosition(Unit* unit)
	{
		u32 switchValue;
		if (unit->main_order_state == 0)
		{
			//change move target destination to current position
			helper::set_resource_target(unit);
			if (
				unit->next_target_waypoint.x != unit->move_target.pt.x ||
				unit->next_target_waypoint.y != unit->move_target.pt.y
				)
			{
				unit->next_target_waypoint.x = unit->move_target.pt.x;
				unit->next_target_waypoint.y = unit->move_target.pt.y;
			}
			//6407F:
			unit->status |= UnitStatus::HoldingPosition;
			if (unit->subunit != nullptr)
				unit->subunit->status |= UnitStatus::HoldingPosition;
			unit->main_order_state = 1;
			unit->sprite->play_iscript_animation(IscriptAnimation::WalkingToIdle, true);
		}
		//640AF

		//try healing current target, return a value depending
		//on what happened
		switchValue = helper::doMedicHeal(unit, unit->order_target.unit);

		if (switchValue == 3) //640C6
			unit->order_target.unit = nullptr;

		if (switchValue == 0 || switchValue == 2 || switchValue == 3) //640CD
			unit->sprite->play_iscript_animation(IscriptAnimation::WalkingToIdle, true);

		//640D9, continued into for values 0,2,3, and jumped into for values
		//above 3, thus only switchValue of 1 don't go through it
		if (switchValue != 1)
		{

			//if timer is 0, seek a target in range
			if (unit->main_order_timer == 0)
			{

				unit->main_order_timer = 15;
				unit->order_target.unit = helper::MedicHeal_TargetAcquire(unit);

				if (unit->order_target.unit != nullptr)
				{
					unit->order_target.pt.x = (unit->order_target.unit)->sprite->position.x;
					unit->order_target.pt.y = (unit->order_target.unit)->sprite->position.y;
				}

			}

		}
		else
		{ //64109 (switchValue == 1)

			if (!(unit->order_signal & 1))
			{

				Image* current_image = unit->sprite->images.head;

				while (current_image != nullptr)
				{
					current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
					current_image = current_image->link.next;
				}

				if (
					unit->order_queue_head == nullptr ||
					unit->order_queue_head->orderId != OrderId::MedicHeal2
					) 	//6413D
					helper::order_immediate(unit, OrderId::MedicHeal2); //return to idle after heal

			}

			//64146
			if (
				(unit->order_target.unit)->sprite->position.x != unit->next_target_waypoint.x ||
				(unit->order_target.unit)->sprite->position.y != unit->next_target_waypoint.y
				)
			{
				unit->next_target_waypoint.x = (unit->order_target.unit)->sprite->position.x;
				unit->next_target_waypoint.y = (unit->order_target.unit)->sprite->position.y;
			}

		}

	} //void orders_MedicHoldPosition(Unit* unit)

	;

	void orders_ReaverStop(Unit* unit)
	{

		Unit* out_hangar_child = unit->unit_with_hangar.out_hangar_child;
		Point16 pos;

		//change move target destination to current position
		helper::set_resource_target(unit);

		if (
			unit->move_target.pt.x != unit->next_target_waypoint.x ||
			unit->move_target.pt.y != unit->next_target_waypoint.y
			)
		{
			unit->next_target_waypoint.x = unit->move_target.pt.x;
			unit->next_target_waypoint.y = unit->move_target.pt.y;
		}

		//65500
		while (out_hangar_child != nullptr)
		{

			out_hangar_child->user_action_flags |= 1;

			if (out_hangar_child->main_order_id != OrderId::Die)
			{

				bool bNotRemovableOrder = false;

				while (out_hangar_child->order_queue_tail != nullptr && !bNotRemovableOrder)
				{

					if (
						!orders_dat::CanBeInterrupted[out_hangar_child->order_queue_tail->orderId] &&
						out_hangar_child->order_queue_tail->orderId != OrderId::SelfDestrucing
						)
						bNotRemovableOrder = true;
					else
						helper::remove_order_from_unit_queue(out_hangar_child->order_queue_tail, out_hangar_child);

				}

				//65539
				out_hangar_child->perform_another_order(
					OrderId::SelfDestrucing,
					0,
					0,
					nullptr,
					UnitId::None
				);

			}

			//65554
			prepare_for_next_order(out_hangar_child);
			out_hangar_child = out_hangar_child->hangar_unit.hangar_link.prev;

		} //while(out_hangar_child != nullptr)

		//65565

		unit->user_action_flags |= 1;

		if (unit->order_target.unit != nullptr)
		{
			pos.x = (unit->order_target.unit)->sprite->position.x;
			pos.y = (unit->order_target.unit)->sprite->position.y;
		}
		else
		{
			pos.x = 0;
			pos.y = 0;
		}

		//65598
		if (unit->main_order_id != OrderId::Die)
		{

			bool bNotRemovableOrder = false;

			while (unit->order_queue_tail != nullptr && !bNotRemovableOrder)
			{

				if (
					!orders_dat::CanBeInterrupted[unit->order_queue_tail->orderId] &&
					unit->order_queue_tail->orderId != OrderId::Reaver
					)
					bNotRemovableOrder = true;
				else
					helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);

			}

			//655C3
			unit->perform_another_order(
				OrderId::Reaver,
				pos.x,
				pos.y,
				unit->order_target.unit,
				UnitId::None
			);

		}

		//655DB
		prepare_for_next_order(unit);

	} //void orders_ReaverStop(Unit* unit)

	;

	void orders_Guard(Unit* unit)
	{

		unit->main_order_timer = (u8)helper::rand_between(0, 15, 29);

		if (playerTable[unit->player].type == PlayerType::Computer)
		{

			//since details about pAI structure are unknown, have to use this
			u8* const pAI_subparameter = (u8*)(((s32)(unit->unit_ai)) + 8);

			unit->main_order_id = OrderId::ComputerAI;

			if (unit->unit_ai == nullptr)
			{

				if (units_dat::BaseProperty[unit->id] & UnitProperty::Hero)
					unit->main_order_id = OrderId::PlayerGuard;

			}
			else //75BD6
				if (*pAI_subparameter == 1)
					unit->main_order_id = OrderId::GuardPost;

		}
		else
			unit->main_order_id = OrderId::PlayerGuard;

	} //void orders_Guard(Unit* unit)

	;

	//NOT HOOKED (since most of the action happen in another function)
	//Just here to show that the true code is in function_004774A0, though
	//a few things happen before
	void orders_TurretGuard(Unit* unit)
	{

		if (
			unit->subunit->next_target_waypoint.x != unit->next_target_waypoint.x ||
			unit->subunit->next_target_waypoint.y != unit->next_target_waypoint.y
			)
		{
			unit->next_target_waypoint.x = unit->subunit->next_target_waypoint.x;
			unit->next_target_waypoint.y = unit->subunit->next_target_waypoint.y;
		}

		orders_player_guard(unit);

	} //void orders_TurretGuard(Unit* unit)

	;

	//Guard related function, may be a poorly identified official guard order
	void orders_player_guard(Unit* unit)
	{

		//if unit->auto_target_unit, attack this target
		//then disable the unit...I think
		//That's why if it return true, nothing else is done
		//since has already attacked
		if (!helper::attack_recent_attacker(unit)) {
			if (unit->main_order_timer == 0) {
				//unit->main_order_timer = 15;
				if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit) {
					if (unit->subunit->next_target_waypoint.x != unit->next_target_waypoint.x ||
						unit->subunit->next_target_waypoint.y != unit->next_target_waypoint.y) {
						unit->next_target_waypoint.x = unit->subunit->next_target_waypoint.x;
						unit->next_target_waypoint.y = unit->subunit->next_target_waypoint.y;
					}
				}

				if (unit->get_seek_range() != 0) {
					Unit* target = helper::find_best_attack_target(unit);
					if (target) {
						// attack target I think, also used by attack_recent_attacker
						helper::order_to_attack_target(unit, target, 1, 0);
					}
				}
			}
		}
	}
}