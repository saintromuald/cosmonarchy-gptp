#include "wireframe.h"
#include "../../SCBW/api.h"

namespace hooks {

	//was initially in a building_morph file
	//the rest of the function this fragment is from use
	//too many unknowns, thus this is only a fragment.
	//This allow to change the wireframe displayed for
	//the unit, the special case being that zerg morphing
	//buildings show the wireframe of the new building
	//before construction is complete
	uint16 statdata_UnitWireframeUpdate_Fragment(CUnit* unit) {

		uint16 unitId;
		bool bUseCurrentUnitId = true;

		if (!(unit->status & UnitStatus::Completed)) {

			unitId = unit->buildQueue[unit->buildQueueSlot];

			if (
				unitId == UnitId::ZergHive ||
				unitId == UnitId::ZergLair ||
				unitId == UnitId::ZergGeszithRoost ||
				unitId == UnitId::ZergSporeColony ||
				unitId == UnitId::ZergSunkenColony ||
				unitId == 191
				)
				bUseCurrentUnitId = false;

		}

		//5694F
		if (bUseCurrentUnitId)
			unitId = unit->id;

		return unitId;
	}
	uint16 getLastQueueSlotType(CUnit* unit) {
		uint16 unitId;
		if (unit->status & UnitStatus::Completed)
			unitId = unit->id;
		else {
			unitId = unit->buildQueue[unit->buildQueueSlot];
			if (!(scbw::isZergBuildingMorphResult))
				unitId = unit->id;
		}
		return unitId;
	};

	uint8 wireframeShields(CUnit* unit) {
		auto eax = unit->id;
		uint8 al = 0;
		if (units_dat::ShieldsEnabled[unit->id]) {
			auto lastSlot = scbw::getLastQueueSlotType(unit);
			uint32 maxShields = units_dat::MaxShieldPoints[lastSlot];
			if (lastSlot == unit->id) {
				maxShields = unit->maxShields();//for stasis, later
			}
			if (maxShields) {
				uint32 shields = (uint32)unit->shields >> 8;
				shields *= 7;
				shields /= maxShields;
				if (shields * 2 + 1 >= 40)
					return al; // safety check
				auto wireConst = wireframeConstants[shields * 2];
				auto wireConstShift = wireframeConstants[(shields * 2) + 1];
				auto buf1 = charBuffer[wireConst];//CL
				al = charBuffer[wireConstShift];//AL
				*off_50CE81 = buf1;
				*off_50CE82 = al;
			}
		} else {
			al = *byteShieldStr;
			*off_50CE81 = al;
			*off_50CE82 = al;
		}
		return al;
	}
} //hooks
