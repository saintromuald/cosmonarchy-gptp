#include "train_cmd_receive.h"
#include <SCBW/api.h>

namespace hooks
{
	void cmdrecv_train_fighter() // 004C1800
	{
		*unit_iterator_pos = 0;
		Unit* current_selection = get_active_player_next_selection();
		while (current_selection)
		{
			u32 unit_to_train;
			//the original code was messy, simplified/optimized version here
			if (current_selection->id == UnitId::ProtossSolarion)
			{
				unit_to_train = UnitId::ProtossSteward;
			}
			else
			{
				if (current_selection->id == UnitId::ProtossAccantor)
				{
					unit_to_train = UnitId::ProtossPlasmaShell;
				}
				else
				{
					unit_to_train = 0; // not translating it as marine as it's an error case
				}
			}

			if (current_selection->can_make_unit(unit_to_train, *ACTIVE_NATION_ID) == 1
				&& helper::has_money_can_make(current_selection, unit_to_train)) // add unit to build queue if successful
			{
				if (current_selection->id == UnitId::ProtossSolarion
					|| current_selection->id == UnitId::ProtossAccantor)
				{
					if (current_selection->secondary_order_state != 2)
					{
						current_selection->secondary_order_id = OrderId::TrainFighter;
						current_selection->secondary_order_pos.y = 0;
						current_selection->secondary_order_pos.x = 0;
						current_selection->current_build_unit = 0;
						current_selection->secondary_order_state = 0;
					}
				}
			}
			current_selection = get_active_player_next_selection();
		}
		scbw::refresh_console();
	}

	// Note: the default function isn't compatible with multiple selection
	void cmdrecv_train(u16 unit_id) // 004C1C20
	{
		Unit* builder;
		*unit_iterator_pos = 0;
		builder = get_active_player_next_selection();
		if (builder && get_active_player_next_selection() == 0)
		{
			// Originally also checked for a lardcoded ID distance, disabled for training units with high IDs
			if (builder->can_make_unit(unit_id, *ACTIVE_NATION_ID) == 1)
			{
				if (helper::has_money_can_make(builder, unit_id)) // add unit to build queue if successful
				{
					builder->set_secondary_order(OrderId::Train);
				}
				scbw::refresh_console();
			}
		}
	}
}