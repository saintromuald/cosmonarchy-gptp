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
				&& helper::HasMoneyCanMake(current_selection, unitToBuild)) // add unit to build queue if successful
			{
				if (current_selection->id == UnitId::ProtossSolarion
					|| current_selection->id == UnitId::ProtossAccantor)
				{
					if (current_selection->secondaryOrderState != 2)
					{
						current_selection->secondaryOrderId = OrderId::TrainFighter;
						current_selection->secondaryOrderPos.y = 0;
						current_selection->secondaryOrderPos.x = 0;
						current_selection->currentBuildUnit = nullptr;
						current_selection->secondaryOrderState = 0;
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
				if (helper::HasMoneyCanMake(builder, wUnitType)) // add unit to build queue if successful
				{
					builder->setSecondaryOrder(OrderId::Train);
				}
				scbw::refresh_console();
			}
		}
	}
}