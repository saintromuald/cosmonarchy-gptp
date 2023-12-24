#include "CMDRECV_Stop.h"
#include <SCBW/api.h>
#include <globals.h>

namespace hooks
{
	// The functions seems similar but there are some differences,
	// like the u16_ offset, the function to perform the order in
	// cmdrecv_stop, and the orders id

	void cmdrecv_reaver_stop()
	{
		static u32* u32_0066FF60 = (u32*)0x0066FF60;
		*g_unit_iterator_pos = 0;
		Unit* current_unit = get_active_player_next_selection();
		while (current_unit != nullptr)
		{
			*u32_0066FF60 = 0x00;
			if (orders_dat::req_offset[OrderId::ReaverStop] == 0xFFFF)
			{
				helper::parse_orders_dat_requirements();
			}

			if (current_unit->player != *g_active_nation_id) {
				*u32_0066FF60 = 0x01;
			} else if (!(current_unit->status & UnitStatus::Completed)) {
				*u32_0066FF60 = 0x14;
			} else if (current_unit->is_frozen()) {
				*u32_0066FF60 = 0x0A;
			} else if (current_unit->status & UnitStatus::Burrowed && !unit_id_is_burrowed_attacker(current_unit->id)) {
				*u32_0066FF60 = 0x0B;
			} else if ((current_unit->id == UnitId::TerranMason || current_unit->id == UnitId::TerranTalos) && current_unit->main_order_id == OrderId::ConstructingBuilding) {
				*u32_0066FF60 = 0x08;
			} else if (scbw::unit_can_nuke(current_unit) && current_unit->main_order_id == OrderId::NukeTrack) {
				*u32_0066FF60 = 0x08;
			} else if ((current_unit->id == UnitId::ProtossArchon || current_unit->id == UnitId::ProtossAugur) && current_unit->main_order_id == OrderId::CompletingArchonSummon) {
				*u32_0066FF60 = 0x08;
			} else if (current_unit->id == UnitId::ProtossMindTyrant && current_unit->main_order_id == OrderId::CompletingArchonSummon) {
				*u32_0066FF60 = 0x08;
			} else if (orders_dat::req_offset[OrderId::ReaverStop] == 0) {
				*u32_0066FF60 = 0x17;
			} else if (helper::parse_requirement_opcodes(current_unit, OrderId::ReaverStop, *g_active_nation_id, g_orders_requirement_tree, orders_dat::req_offset[OrderId::ReaverStop]) == 1) {
				current_unit->user_action_flags |= 1;
				if (current_unit->main_order_id != OrderId::Die)
				{
					bool bContinueLoop = true;
					while (bContinueLoop)
					{
						if (current_unit->order_queue_tail == nullptr) {
							bContinueLoop = false;
						} else if (orders_dat::can_be_interrupted[current_unit->order_queue_tail->order_id] || current_unit->order_queue_tail->order_id == OrderId::ReaverStop) {
							helper::remove_order_from_unit_queue(current_unit->order_queue_tail, current_unit);
						} {
							bContinueLoop = false;
						}
					}
					current_unit->perform_another_order(OrderId::ReaverStop, 0, 0, nullptr, UnitId::None, 0);
				}
				prepare_for_next_order(current_unit);
			}
			current_unit = get_active_player_next_selection();
		}
	}

	void cmdrecv_carrier_stop()
	{
		static u32* u32_0066FF60 = (u32*)0x0066FF60;
		*g_unit_iterator_pos = 0;
		Unit* current_unit = get_active_player_next_selection();
		while (current_unit != nullptr)
		{
			*u32_0066FF60 = 0x00;
			if (orders_dat::req_offset[OrderId::CarrierStop] == 0xFFFF)
			{
				helper::parse_orders_dat_requirements();
			}

			if (current_unit->player != *g_active_nation_id) {
				*u32_0066FF60 = 0x01;
			} else if (!(current_unit->status & UnitStatus::Completed)) {
				*u32_0066FF60 = 0x14;
			} else if (current_unit->is_frozen()) {
				*u32_0066FF60 = 0x0A;
			} else if (current_unit->status & UnitStatus::Burrowed && !unit_id_is_burrowed_attacker(current_unit->id)) {
				*u32_0066FF60 = 0x0B;
			} else if (current_unit->id == UnitId::TerranMason || current_unit->id == UnitId::TerranTalos && current_unit->main_order_id == OrderId::ConstructingBuilding) {
				*u32_0066FF60 = 0x08;
			} else if (scbw::unit_can_nuke(current_unit) && current_unit->main_order_id == OrderId::NukeTrack) {
				*u32_0066FF60 = 0x08;
			} else if ((current_unit->id == UnitId::ProtossArchon || current_unit->id == UnitId::ProtossAugur) && current_unit->main_order_id == OrderId::CompletingArchonSummon) {
				*u32_0066FF60 = 0x08;
			} else if (current_unit->id == UnitId::ProtossMindTyrant && current_unit->main_order_id == OrderId::CompletingArchonSummon) {
				*u32_0066FF60 = 0x08;
			} else if (orders_dat::req_offset[OrderId::CarrierStop] == 0) {
				*u32_0066FF60 = 0x17;
			} else if (helper::parse_requirement_opcodes(current_unit, OrderId::CarrierStop, *g_active_nation_id, g_orders_requirement_tree, orders_dat::req_offset[OrderId::CarrierStop]) == 1) {
				current_unit->user_action_flags |= 1;
				if (current_unit->main_order_id != OrderId::Die)
				{
					bool bContinueLoop = true;
					while (bContinueLoop)
					{
						if (current_unit->order_queue_tail == nullptr)
						{
							bContinueLoop = false;
						} else if (orders_dat::can_be_interrupted[current_unit->order_queue_tail->order_id] || current_unit->order_queue_tail->order_id == OrderId::CarrierStop) {
							helper::remove_order_from_unit_queue(current_unit->order_queue_tail, current_unit);
						} else {
							bContinueLoop = false;
						}
					}
					current_unit->perform_another_order(OrderId::CarrierStop, 0, 0, nullptr, UnitId::None, 0);
				}
				prepare_for_next_order(current_unit);
			}
			current_unit = get_active_player_next_selection();
		}
	}

	void cmdrecv_stop(u8 bCommandType)
	{
		static u32* u32_0066FF60 = (u32*)0x0066FF60;
		*g_unit_iterator_pos = 0;
		Unit* current_unit = get_active_player_next_selection();
		while (current_unit != nullptr)
		{
			*u32_0066FF60 = 0x00;
			if (orders_dat::req_offset[OrderId::Stop] == 0xFFFF)
			{
				helper::parse_orders_dat_requirements();
			}

			if (current_unit->player != *g_active_nation_id) {
				*u32_0066FF60 = 0x01;
			} else if (!(current_unit->status & UnitStatus::Completed)) {
				*u32_0066FF60 = 0x14;
			} else if (current_unit->is_frozen()) {
				*u32_0066FF60 = 0x0A;
			} else if (current_unit->status & UnitStatus::Burrowed && !unit_id_is_burrowed_attacker(current_unit->id)) {
				*u32_0066FF60 = 0x0B;
			} else if (scbw::unit_can_nuke(current_unit) && current_unit->main_order_id == OrderId::NukeTrack) {
				*u32_0066FF60 = 0x08;
			} else if ((current_unit->id == UnitId::ProtossArchon || current_unit->id == UnitId::ProtossAugur) && current_unit->main_order_id == OrderId::CompletingArchonSummon) {
				*u32_0066FF60 = 0x08;
			} else if (current_unit->id == UnitId::ProtossMindTyrant && current_unit->main_order_id == OrderId::CompletingArchonSummon) {
				*u32_0066FF60 = 0x08;
			} else if (orders_dat::req_offset[OrderId::Stop] == 0) {
				*u32_0066FF60 = 0x17;
			} else if (helper::parse_requirement_opcodes(current_unit, OrderId::Stop, *g_active_nation_id, g_orders_requirement_tree, orders_dat::req_offset[OrderId::Stop]) == 1) {
				helper::targeted_order(current_unit, UnitId::None, *g_order_type_4_target, *g_order_type_4_position, OrderId::Stop, 0, bCommandType != 0, 1); // bCommandType is 1 if order is queued
			}

			if (current_unit->id == UnitId::TerranWyvern)
			{
				UnitExt* unit_ext = get_unit_ext(current_unit);
				unit_ext->units[UnitWyvernTargetLock] = 0;
			}				
			current_unit = get_active_player_next_selection();
		}
	}
}