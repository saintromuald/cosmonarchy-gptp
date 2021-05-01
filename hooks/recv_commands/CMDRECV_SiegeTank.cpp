#include "CMDRECV_SiegeTank.h"
#include <SCBW/api.h>

namespace hooks {
	void CMDRECV_Siege(uint8 bCommandType) {
		CUnit* current_selection;

		*selectionIndexStart = 0;
		current_selection = getActivePlayerNextSelection();

		while (current_selection != nullptr) {

			if (
				(current_selection->id == UnitId::TerranPhalanxTankMode ||
					current_selection->id == UnitId::Hero_EdmundDukeTankMode) &&
				current_selection->mainOrderId != OrderId::SiegeMode
				) {
				if (current_selection->canUseTech(TechId::TankSiegeMode, *ACTIVE_NATION_ID) == 1) {

					if (bCommandType != 0)
						bCommandType = 1;
					else
						bCommandType = 0;

					function_004754F0(
						current_selection,
						UnitId::None,
						*(uint32*)(0x006D5C24),
						*(uint32*)(0x006D5C20),
						OrderId::SiegeMode,
						0,
						bCommandType,
						1
					);

				}
			}

			current_selection = getActivePlayerNextSelection();

		}

	}

	;

	void CMDRECV_Unsiege(uint8 bCommandType) {

		CUnit* current_selection;

		*selectionIndexStart = 0;
		current_selection = getActivePlayerNextSelection();

		while (current_selection != nullptr) {

			if (
				(current_selection->id == UnitId::TerranPhalanxSiegeMode ||
					current_selection->id == UnitId::Hero_EdmundDukeSiegeMode) &&
				current_selection->mainOrderId != OrderId::TankMode
				) {
				if (current_selection->canUseTech(TechId::TankSiegeMode, *ACTIVE_NATION_ID) == 1) {

					if (bCommandType != 0)
						bCommandType = 1;
					else
						bCommandType = 0;

					function_004754F0(
						current_selection,
						UnitId::None,
						*(uint32*)(0x006D5C24),
						*(uint32*)(0x006D5C20),
						OrderId::TankMode,
						0,
						bCommandType,
						1
					);

				}
			}

			current_selection = getActivePlayerNextSelection();

		}

	}
}