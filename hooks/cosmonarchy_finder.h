#pragma once
#include "BW\api.h"
#include "BW\bwdata.h"
#include "BW\UnitFinder.h"
#include <vector>

namespace Flags {
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
		NoTrap = 1 << 19,
		NoHallucination = 1 << 20,
		CanAttackOrCast = 1 << 21,
		Own = 1 << 22,
		NotWorker = 1 << 23,
		NoEgg = 1 << 24,
		NoLarva = 1 << 25,
		NoHangar = 1 << 26,
	};
}

bw::UnitFinder::CallbackMatch cosmonarchy_finder_match(Unit* center, u32 context, std::vector<u16> allowedUnitIds);

bool range_match(Unit* unit, u32 r, u32 context, std::vector<u16> allowedIds);
Unit* range_match_unit(Unit* unit, u32 r, u32 context, std::vector<u16> allowedIds);
Unit* range_match_position(Vec32 position, u32 r, u32 context, std::vector<u16> allowedIds);
u32 cosmonarchy_unit_count(Unit* unitPtr, Point16 pos, s32 r, u32 flags, std::vector<u16> unitIds);
template<typename cosmonarchyLoop>
inline void cosmonarchy_finder_loop(Unit* unitPtr, Point16 pos, s32 r, u32 flags, std::vector<u16> unitIds, cosmonarchyLoop l) {
	auto condition = cosmonarchy_finder_match(unitPtr, flags, unitIds);
	bw::UnitFinder units_in_splash(pos.x - r, pos.y - r, pos.x + r, pos.y + r);
	for (s32 i = 0; i < units_in_splash.get_unit_count(); i++) {
		auto unit = units_in_splash.get_unit(i);
		if (condition(unit)) {
			l(unit);
		}
	}
}
