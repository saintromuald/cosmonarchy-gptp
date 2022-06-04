#pragma once
#include "SCBW/api.h"

namespace BuildingHelper
{
	// Check if @p unit is warping in a Protoss building.
	// Identical to function @ 0x004E4C40
	bool is_warping_in_protoss_building(const Unit* probe)
	{
		return probe->main_order_id == OrderId::BuildProtoss1
			&& probe->status & UnitStatus::GroundedBuilding
			&& probe->order_target.unit != nullptr
			&& !(probe->order_target.unit->status & UnitStatus::Completed);
	}

	// Check if @p building is in the process of constructing an add-on.
	// Return the building or add-on that the @p unit is constructing.
	// Identical to function @ 0x00466A30
	Unit* get_construct_repair_target(const Unit* unit)
	{
		if (unit->main_order_id == OrderId::BuildTerran
			|| unit->main_order_id == OrderId::Repair1
			|| unit->main_order_id == OrderId::ConstructingBuilding
			|| is_warping_in_protoss_building(unit))
		{
			return unit->order_target.unit;
		}
		if (unit->secondary_order_id == OrderId::Train
			|| unit->secondary_order_id == OrderId::TrainFighter
			|| helper::is_constructing_addon(unit))
		{
			return unit->current_build_unit;
		}
		return nullptr;
	}

	// Identical to function @ 0x00466B70
	s32 get_unit_id_count_in_build_queue(const Unit* unit, u16 unit_id)
	{
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
		{
			if (get_construct_repair_target(unit) == nullptr
				&& unit->build_queue[unit->build_queue_slot % 5] == unit_id
				&& (unit->main_order_id == OrderId::BuildTerran
					|| unit->main_order_id == OrderId::BuildProtoss1
					|| unit->main_order_id == OrderId::DroneLand
					|| unit->main_order_id == OrderId::DroneBuild
					|| unit->main_order_id == OrderId::DroneStartBuild))
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		if (scbw::is_zerg_egg(unit->id, true, false)
			|| unit->main_order_id == OrderId::Morph2
			|| unit->main_order_id == OrderId::ZergBuildSelf)
			return (unit->build_queue[unit->build_queue_slot % 5] == unit_id) ? 1 : 0;

		s32 unitCount = 0;
		for (s32 i = 0; i < 5; ++i)
		{
			if (unit->build_queue[(unit->build_queue_slot + i) % 5] == unit_id)
				++unitCount;
		}
		return unitCount;
	}

	s32 get_unit_id_count(const Unit* unit, u16 unit_id, bool ignoreIncomplete)
	{
		if (!ignoreIncomplete || unit->status & UnitStatus::Completed)
		{
			if (unit->id == unit_id)
			{
				return 1;
			}
			if (!ignoreIncomplete)
			{
				return get_unit_id_count_in_build_queue(unit, unit_id);
			}
		}
		return 0;
	}
}