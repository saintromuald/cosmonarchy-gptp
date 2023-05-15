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

	void unburrow_proc(Unit* unit)
	{
		if (unit->main_order_id == OrderId::Burrow
		||  unit->main_order_id == OrderId::Burrowed
		||  unit->id == UnitId::ZergLakizilisk)
		{
			if (unit->id == UnitId::ZergLakizilisk)
			{
				unit->status |= UnitStatus::CanNotReceiveOrders;
			}
			unit->user_action_flags |= 1;
			if (unit->main_order_id != OrderId::Die)
			{
				bool stop_loop = false;
				while (unit->order_queue_tail && !stop_loop)
				{
					if (!orders_dat::can_be_interrupted[unit->order_queue_tail->order_id]
						&& unit->order_queue_tail->order_id != OrderId::Unburrow)
					{
						stop_loop = true;
					} else {
						helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
					}
				}
				unit->perform_another_order(OrderId::Unburrow, 0, 0, 0, UnitId::None);
			}
			prepare_for_next_order(unit);
			helper::force_order_done(unit);
		}
	}
}