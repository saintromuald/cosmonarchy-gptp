#pragma once
#include "SCBW\api.h"
#include "SCBW\scbwdata.h"
#include "SCBW\UnitFinder.h"
#include <vector>

namespace CFlags {
	enum {
		NoInvincible = 1 << 0,
		NoAir = 1 << 1,
		NoGround = 1 << 2,
		NoMechanical = 1 << 3,
		NoOrganic = 1 << 4,
		NoBuilding = 1 << 5,
		OnlyOrganic = 1 << 6,
		OnlyMechanic = 1 << 7,
		Enemy = 1 << 8,
		Ally = 1 << 9,
		IncludingSelf = 1 << 10,
		FrontalArc = 1 << 11,
		Spellcaster = 1 << 12,
		HasEnergy = 1 << 13,
		NotSelf = 1 << 14,
		Shielded = 1 << 15,
		Enabled = 1 << 16,
		Powered = 1 << 17,
		Completed = 1 << 18,
	};
}

scbw::UnitFinderCallbackMatchInterface unitRangeCount(CUnit* center, uint32 context, std::vector<uint16> allowedUnitIds);

bool rangeMatch(CUnit* unit, uint32 r, uint32 context, std::vector<uint16> allowedIds);
CUnit* rangeMatchUnit(CUnit* unit, uint32 r, uint32 context, std::vector<uint16> allowedIds);
CUnit* rangeMatchPosition(Point32 position, uint32 r, uint32 context, std::vector<uint16> allowedIds);
uint32 hydraUnitCount(CUnit* unitPtr, Point16 pos, int r, uint32 flags, std::vector<uint16> unitIds);
template<typename hydraLoop>
inline void hydraFinderLoop(CUnit* unitPtr, Point16 pos, int r, uint32 flags, std::vector<uint16> unitIds, hydraLoop l) {
	auto condition = unitRangeCount(unitPtr, flags, unitIds);
	scbw::UnitFinder unitsInSplash(pos.x - r, pos.y - r,
		pos.x + r, pos.y + r);
	for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
		auto unit = unitsInSplash.getUnit(i);
		if (condition(unit)) {
			l(unit);
		}
	}
}
