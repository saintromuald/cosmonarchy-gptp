#pragma once
#include "SCBW\api.h"
#include "SCBW\scbwdata.h"
#include "SCBW\UnitFinder.h"
#include <vector>

namespace CFlags {
	enum {
		Invincible = 1 << 0,
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

scbw::UnitFinderCallbackMatchInterface unitRangeCount(CUnit* center, u32 context, std::vector<u16> allowedUnitIds);

bool rangeMatch(CUnit* unit, u32 r, u32 context, std::vector<u16> allowedIds);
CUnit* rangeMatchUnit(CUnit* unit, u32 r, u32 context, std::vector<u16> allowedIds);
CUnit* rangeMatchPosition(Point32 position, u32 r, u32 context, std::vector<u16> allowedIds);
u32 hydraUnitCount(CUnit* unitPtr, Point16 pos, s32 r, u32 flags, std::vector<u16> unitIds);
template<typename hydraLoop>
inline void hydraFinderLoop(CUnit* unitPtr, Point16 pos, s32 r, u32 flags, std::vector<u16> unitIds, hydraLoop l) {
	auto condition = unitRangeCount(unitPtr, flags, unitIds);
	scbw::UnitFinder unitsInSplash(pos.x - r, pos.y - r,
		pos.x + r, pos.y + r);
	for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++) {
		auto unit = unitsInSplash.getUnit(i);
		if (condition(unit)) {
			l(unit);
		}
	}
}
