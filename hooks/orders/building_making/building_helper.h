#pragma once
#include "SCBW/api.h"

namespace BuildingHelper {
	//Check if @p unit is warping in a Protoss building.
	//Identical to function @ 0x004E4C40
	bool isWarpingInProtossBuilding(const CUnit* probe) {
		return probe->mainOrderId == OrderId::BuildProtoss1
			&& probe->status & UnitStatus::GroundedBuilding
			&& probe->orderTarget.unit != nullptr
			&& !(probe->orderTarget.unit->status & UnitStatus::Completed);
	}

	//Check if @p building is in the process of constructing an add-on.
	//Identical to function @ 0x004E66B0
	bool isConstructingAddon(const CUnit* building) {
		return building->secondaryOrderId == OrderId::BuildAddon
			&& building->status & UnitStatus::GroundedBuilding
			&& building->currentBuildUnit != nullptr
			&& !(building->currentBuildUnit->status & UnitStatus::Completed);
	}

	//Return the building or add-on that the @p unit is constructing.
	//Identical to function @ 0x00466A30
	CUnit* getConstructRepairTarget(const CUnit* unit) {
		if (unit->mainOrderId == OrderId::BuildTerran
			|| unit->mainOrderId == OrderId::Repair1
			|| unit->mainOrderId == OrderId::ConstructingBuilding
			|| isWarpingInProtossBuilding(unit))
			return unit->orderTarget.unit;

		if (unit->secondaryOrderId == OrderId::Train
			|| unit->secondaryOrderId == OrderId::TrainFighter
			|| isConstructingAddon(unit))
			return unit->currentBuildUnit;

		return nullptr;
	}

	//Identical to function @ 0x00466B70
	int getNumberOfUnitTypeInBuildQueue(const CUnit* unit, uint16 unitId) {
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker) {
			if (getConstructRepairTarget(unit) == nullptr
				&& unit->buildQueue[unit->buildQueueSlot % 5] == unitId
				&& (unit->mainOrderId == OrderId::BuildTerran
					|| unit->mainOrderId == OrderId::BuildProtoss1
					|| unit->mainOrderId == OrderId::DroneLand
					|| unit->mainOrderId == OrderId::DroneBuild
					|| unit->mainOrderId == OrderId::DroneStartBuild))
				return 1;
			else
				return 0;
		}

		if (scbw::isZergEgg(unit->id, true, false)
			|| unit->mainOrderId == OrderId::Morph2
			|| unit->mainOrderId == OrderId::ZergBuildSelf)
			return (unit->buildQueue[unit->buildQueueSlot % 5] == unitId) ? 1 : 0;

		int unitCount = 0;
		for (int i = 0; i < 5; ++i) {
			if (unit->buildQueue[(unit->buildQueueSlot + i) % 5] == unitId)
				++unitCount;
		}
		return unitCount;
	}

	int getNumberOfUnitType(const CUnit* unit, uint16 unitId, bool ignoreIncomplete) {
		if (!ignoreIncomplete || unit->status & UnitStatus::Completed) {
			if (unit->id == unitId)
				return 1;

			if (!ignoreIncomplete)
				return getNumberOfUnitTypeInBuildQueue(unit, unitId);
		}

		return 0;
	}
}