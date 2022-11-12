#include "medic_orders.h"
#include <SCBW/api.h>
#include "globals.h"
#include <math_utils.h>

namespace hooks
{
	void orders_MedicHeal2(Unit* unit)
	{
		if (!(unit->movement_flags & MovementFlags::Accelerating))
		{
			Image* current_image = unit->sprite->images.head;
			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
				current_image = current_image->link.next;
			}
		}
		helper::force_order_done(unit);
	}

	void orders_HealMove(Unit* unit)
	{
		bool bStopThere = false;
		Unit* target = unit->order_target.unit;
		Unit* oldTarget = unit->order_target.unit;

		if (target != nullptr)
		{
			if (target->id == UnitId::ZergNydusCesiant && target->building.nydus_exit)
			{
				unit->order_to(OrderId::EnterNydusCanal, target);
				return;
			}

			if (helper::ai_order_heal(unit, target) != 0)
			{
				unit->order_computer(OrderId::MedicHeal1);
				unit->order_target.unit = target;
				unit->order_target.pt.x = target->sprite->position.x;
				unit->order_target.pt.y = target->sprite->position.y;
				helper::move_to_target(target, unit);
				bStopThere = true;
			}
			else
			{ // 00463801
				unit->order_target.pt.x = target->sprite->position.x;
				unit->order_target.pt.y = target->sprite->position.y;
			}
		}

		if (!bStopThere)
		{ // 00463817
			if (unit->recent_order_timer == 0)
			{
				if (!helper::set_move_target_xy(unit, unit->order_target.pt.x, unit->order_target.pt.y))
				{
					bStopThere = true;
				}
			}

			// 00463842
			if (!bStopThere)
			{
				if (unit->move_target.pt.x == unit->sprite->position.x && unit->move_target.pt.y == unit->sprite->position.y)
				{
					unit->order_to_idle();
				}
				else if (unit->main_order_timer == 0)
				{
					Unit* newTarget;
					Point16 oldPos;
					unit->main_order_timer = 15;
					newTarget = helper::acquire_medic_heal_target(unit);
					if (newTarget != nullptr)
					{
						oldPos.x = unit->order_target.pt.x;
						oldPos.y = unit->order_target.pt.y;
						helper::order_interrupt(unit, unit->order_target.unit, unit->order_target.pt.x, unit->order_target.pt.y, OrderId::MedicHeal1);

						helper::move_to_target(newTarget, unit);
						helper::unit_order_move_to_target_unit(unit, newTarget);
						if (oldTarget != nullptr)
						{
							helper::update_target_something(unit, oldTarget, OrderId::HealMove);
						}
						else
						{
							helper::function_4748E0(unit, oldPos.x, oldPos.y, OrderId::HealMove);
						}
					}
				}
			}
		}
	}

	// Idle/AI order probably
	void orders_Medic(Unit* unit)
	{
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueReviveStarted]) {
			return;
		}

		bool bStopThere = false;
		if (unit->unit_ai != nullptr && helper::ai_cast_spell_behavior(unit, NULL) != 0)
		{
			bStopThere = true;
		}
		if (!bStopThere)
		{
			Unit* target = helper::acquire_medic_heal_target(unit);
			if (target != nullptr)
			{
				unit->user_action_flags |= 1;
				if (unit->main_order_id != OrderId::Die)
				{
					bool bEndLoop = unit->order_queue_tail == nullptr;
					while (!bEndLoop)
					{

						if (!orders_dat::can_be_interrupted[unit->order_queue_tail->order_id]
							&& unit->order_queue_tail->order_id != OrderId::MedicHeal1)
						{
							bEndLoop = true;
						}
						else
						{
							helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
							bEndLoop = (unit->order_queue_tail == nullptr);
						}
					}
					unit->perform_another_order(OrderId::MedicHeal1, 0, 0, nullptr, UnitId::None);
				}
				// 00463991
				prepare_for_next_order(unit);
				unit->order_target.unit = target;
				unit->order_target.pt.x = target->sprite->position.x;
				unit->order_target.pt.y = target->sprite->position.y;
				helper::move_to_target(target, unit);
			}
			else
			{ // 004639BD
				if (unit->unit_ai != nullptr)
				{
					if (*(u8*)((u32)unit->unit_ai + 8) != 1)
					{
						unit->order_computer(OrderId::ComputerAI);
					}
					else if (unit->sprite->position.x != *(u16*)((u32)unit->unit_ai + 0x12)
						|| unit->sprite->position.y != *(u16*)((u32)unit->unit_ai + 0x14))
					{
						unit->order_to(OrderId::ComputerReturn, *(u16*)((u32)unit->unit_ai + 0x12), *(u16*)((u32)unit->unit_ai + 0x14));
					}
				}
			}
		}
	}

	// void orders_MedicHoldPosition(Unit* unit)
	// in base_orders/stopholdpos_orders.cpp

	u32 doMedicHeal_Func(Unit* cleric, Unit* target) // 00463C40
	{
		if (cleric->can_use_tech(TechId::Healing, cleric->player))
		{
			if (target && helper::ai_order_heal(cleric, target))
			{
				Unit* acquired = target;
				if (!acquired)
				{
					acquired = helper::acquire_medic_heal_target(cleric);
				}
				if (acquired)
				{
					helper::unit_order_move_to_target_unit(cleric, acquired);
					if (!cleric->is_target_within_min_range(acquired, 30))
					{
						helper::move_to_target(acquired, cleric);
						return 3;
					}
					scbw::decloak_proc(cleric);
					target->heal_hp(ClericHealValue, cleric);
					return 1;
				}
			}
		}
		return 0;
	}

	void orders_MedicHeal1(Unit* unit)
	{
		Unit* target = unit->order_target.unit;
		u32 doMedicHeal_result = helper::do_medic_heal(unit, target);
		if (doMedicHeal_result == 0)
		{
			helper::set_resource_target(unit);
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
				unit->order_computer(units_dat::return_to_idle_order[unit->id]);
			}
		}
		else // 004641D3
		{
			if (unit->main_order_state == 0)
			{
				if (target != nullptr
					&& helper::move_towards(unit, target) != 0
					&& unit->get_movable_state() != 2) // unit not unmovable
				{
					if (unit->is_target_within_min_range(target, 30))
					{
						helper::set_resource_target(unit);
						unit->main_order_state = 1;
					}
				}
				else // 00464216
				{
					unit->order_to_idle();
				}
			}
			else // 0046421E
			{
				if (doMedicHeal_result == 1)
				{
					if (!(unit->order_signal & 1))
					{
						Image* current_image = unit->sprite->images.head;
						while (current_image != nullptr)
						{
							current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
							current_image = current_image->link.next;
						}
						if (unit->order_queue_head == nullptr
							|| unit->order_queue_head->order_id != OrderId::MedicHeal2)
						{
							helper::order_immediate(unit, OrderId::MedicHeal2);
						}
					}

					// 0046425B
					target = unit->order_target.unit;
					if (target != nullptr
						&& (target->position.x != unit->next_target_waypoint.x || target->position.y != unit->next_target_waypoint.y))
					{
						unit->next_target_waypoint.x = target->position.x;
						unit->next_target_waypoint.y = target->position.y;
					}
				}
				else // 00464283
				{
					unit->main_order_state = 0;
				}
			}
		}
	}

	bool AI_AcquireHealTarget(Unit* medic, Unit* target)
	{
		UnitExt* unit_ext = get_unit_ext(medic);
		if (unit_ext->values[ValueReviveStarted]) {
			return 0;
		}

		if (target == nullptr || target == medic || medic->status & UnitStatus::IsHallucination)
		{
			return 0;
		}
		UnitExt* targetExt = get_unit_ext(target);
		s32 type = target->id;
		s32 abilityFlags = units_dat::base_property[type];
		s32 enemyCheck;
		bool result;
		if (abilityFlags & UnitProperty::Building
			|| target->status & UnitStatus::InAir
			|| !scbw::is_allied_to(medic->player, target->player)
			|| target->player >= 8
			|| medic->player >= 8
			|| target->hitpoints >= units_dat::max_hitpoints[type]
			|| target->is_frozen()
			|| targetExt->values[TimerReviveFlagClearMax])
		{
			return 0;
		}
		return true;
	}
}