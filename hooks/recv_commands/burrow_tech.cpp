#include "burrow_tech.h"
#include <SCBW/api.h>

namespace hooks
{
	void cmdrecv_unburrow()
	{
		Unit* selected_unit;
		*unit_iterator_pos = 0;
		selected_unit = get_active_player_next_selection();
		while (selected_unit)
		{
			if (units_dat::base_property[selected_unit->id] & UnitProperty::Burrowable
			&&  selected_unit->can_use_tech(TechId::Burrowing, *g_active_nation_id) == 1)
			{
				helper::unburrow(selected_unit);
			}
			selected_unit = get_active_player_next_selection();
		}
	}

	void cmdrecv_burrow(u8 command_type)
	{
		Unit* selected_unit;
		*unit_iterator_pos = 0;
		selected_unit = get_active_player_next_selection();
		while (selected_unit)
		{
			if  (units_dat::base_property[selected_unit->id] & UnitProperty::Burrowable
			&&   selected_unit->can_use_tech(TechId::Burrowing, *g_active_nation_id) == 1
			&& !(selected_unit->status & UnitStatus::Burrowed)
			&&   selected_unit->main_order_id != OrderId::Burrow)
			{
				if (command_type != 0)
				{
					command_type = 1;
				} else {
					command_type = 0;
				}
				helper::targeted_order(selected_unit, UnitId::None, *g_order_type_4_target, *g_order_type_4_position, OrderId::Burrow, 0, command_type, 1);
			}
			selected_unit = get_active_player_next_selection();
		}
	}

	// Hooked to fix Lakizilisks entering an unburrow loop. -Pr0nogo
	void unburrow(Unit* unit) // 004E97C0
	{
		if (unit->main_order_id == OrderId::Burrow || unit->main_order_id == OrderId::Burrowed
		|| (unit->id == UnitId::ZergLakizilisk && unit->main_order_id == OrderId::PlayerGuard))
		{
			if (unit->id == UnitId::ZergLakizilisk)
			{
				unit->status |= UnitStatus::CanNotReceiveOrders;
			}
			unit->user_action_flags |= UserActionFlags::UserActionFlags_IscriptGoToIdle;
			if (unit->main_order_id)
			{
				while (1)
				{
					Order* last_order = unit->order_queue_tail;
					if (!last_order)
					{
						break;
					}
					if (!orders_dat::can_be_interrupted[last_order->order_id] && last_order->order_id != OrderId::Burrow)
					{
						break;
					}
					helper::remove_order_from_unit_queue(last_order, unit);
				}
				helper::perform_another_order(OrderId::Unburrow, 0, unit, UnitId::None, {}, 0);
			}
			helper::do_next_queued_order(unit);
			helper::force_order_done(unit);
		}
	}
}