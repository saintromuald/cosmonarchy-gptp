#include "CMDRECV_SiegeTank.h"
#include <SCBW/api.h>

namespace hooks
{
	void CMDRECV_Siege(u8 is_queued) {
		*g_unit_iterator_pos = 0;
		Unit* unit = get_active_player_next_selection();
		while (unit) {
			if (unit_id_is_tank_mode(unit->id) && unit->main_order_id != OrderId::SiegeMode) {
				if (unit->can_use_tech(TechId::TankSiegeMode, *g_active_nation_id) == 1) {
					if (is_queued) {
						unit->issue_queued_order_target(OrderId::SiegeMode, 0, false);
					} else {
						unit->order_to(OrderId::SiegeMode);
					}
				}
			}
			unit = get_active_player_next_selection();
		}
	}

	void CMDRECV_Unsiege(u8 is_queued) {
		*g_unit_iterator_pos = 0;
		Unit* unit = get_active_player_next_selection();
		while (unit) {
			if (unit_id_is_siege_mode(unit->id) && unit->main_order_id != OrderId::TankMode) {
				if (unit->can_use_tech(TechId::TankSiegeMode, *g_active_nation_id) == 1) {
					if (is_queued) {
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