#include "train_cmd_receive.h"
#include <BW/api.h>

namespace hooks {
	void cmdrecv_train_fighter() { // 004C1800
		*g_unit_iterator_pos = 0;
		Unit* current_selection = get_active_player_next_selection();
		while (current_selection) {
			u32 unit_to_train;
			// The original code was messy, simplified/optimized version here. -GPTP
			if (current_selection->id == UnitId::ProtossSolarion) {
				unit_to_train = UnitId::ProtossSteward;
			} else {
				if (current_selection->id == UnitId::ProtossAccantorOld) {
					unit_to_train = UnitId::ProtossPlasmaShell;
				} else {
					unit_to_train = 0; // Not translating it as marine as it's an error case
				}
			}

			// Add unit to build queue if successful
			if (current_selection->can_make_unit(unit_to_train, *g_active_nation_id) == 1 && helper::has_money_can_make(current_selection, unit_to_train)) {
				if (current_selection->id == UnitId::ProtossSolarion || current_selection->id == UnitId::ProtossAccantorOld) {
					if (current_selection->secondary_order_state != 2) {
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
		bw::refresh_console();
	}

	// Note: the default function isn't compatible with multiple selection
	void cmdrecv_train(u16 unit_id) { // 004C1C20
		*g_unit_iterator_pos = 0;
		Unit* builder = get_active_player_next_selection();
		if (builder && get_active_player_next_selection() == 0) {
			// Originally also checked for a lardcoded ID distance, disabled for training units with high IDs. -Pr0nogo
			if (builder->can_make_unit(unit_id, *g_active_nation_id) == 1) {
				if (helper::has_money_can_make(builder, unit_id)) {
					builder->set_secondary_order(OrderId::TrainUnit);
				}
			}
			bw::refresh_console();
		}
	}
}