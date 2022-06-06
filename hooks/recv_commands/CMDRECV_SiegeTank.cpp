#include "CMDRECV_SiegeTank.h"
#include <SCBW/api.h>

namespace hooks
{
	void CMDRECV_Siege(u8 command_type) {
		*unit_iterator_pos = 0;
		Unit* unit = get_active_player_next_selection();
		while (unit) {
			if (unit->id == UnitId::TerranPhalanxTankMode && unit->main_order_id != OrderId::SiegeMode) {
				if (unit->canUseTech(TechId::TankSiegeMode, *ACTIVE_NATION_ID) == 1) {
					if (command_type) {
						unit->issue_queued_order_target(OrderId::SiegeMode, 0, false);
					} else {
						unit->order_to(OrderId::SiegeMode);
					}
				}
			}
			unit = get_active_player_next_selection();
		}
	}

	void CMDRECV_Unsiege(u8 command_type) {
		*unit_iterator_pos = 0;
		Unit* unit = get_active_player_next_selection();
		while (unit) {
			if (unit->id == UnitId::TerranPhalanxSiegeMode && unit->main_order_id != OrderId::TankMode) {
				if (unit->canUseTech(TechId::TankSiegeMode, *ACTIVE_NATION_ID) == 1) {
					if (command_type) {
						unit->issue_queued_order_target(OrderId::TankMode, 0, false);
					} else {
						unit->order_to(OrderId::TankMode);
					}
				}
			}
			unit = get_active_player_next_selection();
		}
	}
}