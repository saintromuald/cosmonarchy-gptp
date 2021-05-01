#include "train_cmd_receive.h"
#include <SCBW/api.h>

namespace hooks {
	void CMDRECV_TrainFighter() {
		CUnit* current_selection;
		*selectionIndexStart = 0;
		current_selection = getActivePlayerNextSelection();

		while (current_selection != nullptr) {
			uint32 unitToBuild;
			//the original code was messy, simplified/optimized version here
			if (current_selection->id == UnitId::ProtossSolarion)
				unitToBuild = UnitId::ProtossSteward;
			else
				if (current_selection->id == UnitId::ProtossAccantor)
					unitToBuild = UnitId::ProtossPlasmaShell;
				else
					unitToBuild = 0; //not translating it as marine as it's an error case

			if (current_selection->canMakeUnit(unitToBuild, *ACTIVE_NATION_ID) == 1 &&
				HasMoneyCanMake(current_selection, unitToBuild) //add unit to build queue if successful
				) {
				if (current_selection->id == UnitId::ProtossSolarion ||
					current_selection->id == UnitId::ProtossAccantor
					)
				{
					if (current_selection->secondaryOrderState != 2) {
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
		scbw::refreshConsole();
	}

	//Note: the default function isn't compatible with multiple selection
	void CMDRECV_Train(uint16 wUnitType) {
		CUnit* builder;
		*selectionIndexStart = 0;
		builder = getActivePlayerNextSelection();
		if (builder != nullptr && getActivePlayerNextSelection() == nullptr) {
			if (builder->canMakeUnit(wUnitType, *ACTIVE_NATION_ID) == 1 /*&&
				wUnitType <= UnitId::Spell_DisruptionWeb*/ //id before buildings id //must be disabled for paladins
				) {
				if (HasMoneyCanMake(builder, wUnitType)) //add unit to build queue if successful
					builder->setSecondaryOrder(OrderId::Train);
				scbw::refreshConsole();
			}
		}
	}
}