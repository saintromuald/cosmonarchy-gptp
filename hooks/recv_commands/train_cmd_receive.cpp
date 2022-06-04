#include "train_cmd_receive.h"
#include <SCBW/api.h>

namespace hooks
{
	void CMDRECV_TrainFighter()
	{
		Unit* current_selection;
		*selectionIndexStart = 0;
		current_selection = getActivePlayerNextSelection();

		while (current_selection != nullptr)
		{
			u32 unitToBuild;
			//the original code was messy, simplified/optimized version here
			if (current_selection->id == UnitId::ProtossSolarion)
			{
				unitToBuild = UnitId::ProtossSteward;
			}
			else
			{
				if (current_selection->id == UnitId::ProtossAccantor)
				{
					unitToBuild = UnitId::ProtossPlasmaShell;
				}
				else
				{
					unitToBuild = 0; // not translating it as marine as it's an error case
				}
			}

			if (current_selection->can_make_unit(unitToBuild, *ACTIVE_NATION_ID) == 1
				&& helper::has_money_can_make(current_selection, unitToBuild)) // add unit to build queue if successful
			{
				if (current_selection->id == UnitId::ProtossSolarion
					|| current_selection->id == UnitId::ProtossAccantor)
				{
					if (current_selection->secondary_order_state != 2)
					{
						current_selection->secondary_order_id = OrderId::TrainFighter;
						current_selection->secondary_order_pos.y = 0;
						current_selection->secondary_order_pos.x = 0;
						current_selection->current_build_unit = nullptr;
						current_selection->secondary_order_state = 0;
					}
				}
			}
			current_selection = getActivePlayerNextSelection();
		}
		scbw::refresh_console();
	}

	// Note: the default function isn't compatible with multiple selection
	void CMDRECV_Train(u16 wUnitType)
	{
		Unit* builder;
		*selectionIndexStart = 0;
		builder = getActivePlayerNextSelection();
		if (builder != nullptr && getActivePlayerNextSelection() == nullptr)
		{
			// Originally also checked for a lardcoded ID range, disabled for training units with high IDs
			if (builder->can_make_unit(wUnitType, *ACTIVE_NATION_ID) == 1)
			{
				if (helper::has_money_can_make(builder, wUnitType)) // add unit to build queue if successful
				{
					builder->set_secondary_order(OrderId::Train);
				}
				scbw::refresh_console();
			}
		}
	}
}