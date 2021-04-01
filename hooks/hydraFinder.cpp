#include "hooks\hydraFinder.h"


bool rangeMatch(CUnit* unit, u32 r, u32 context, std::vector<u16> allowedIds) {
	scbw::UnitFinder unitsInSplash(unit->getX() - r, unit->getY() - r, unit->getX() + r, unit->getY() + r);
	auto a = scbw::UnitFinder::getNearestTarget(
		unit->getX() - r, unit->getY() - r,
		unit->getX() + r, unit->getY() + r,
		unit, unitRangeCount(unit, context, allowedIds)
	);
	return a != nullptr;
}

u32 hydraUnitCount(CUnit* unitPtr, Point16 pos, int r, u32 flags, std::vector<u16> unitIds)
{
	auto condition = unitRangeCount(unitPtr, flags, unitIds);
	scbw::UnitFinder unitsInSplash(pos.x - r, pos.y - r,
		pos.x + r, pos.y + r);
	auto count = 0;
	for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
		auto unit = unitsInSplash.getUnit(i);
		if (condition.match(unit)) {
			count++;
		}
	}
	return count;
}
CUnit* rangeMatchUnit(CUnit* unit, u32 r, u32 context, std::vector<u16> allowedIds) {
	scbw::UnitFinder unitsInSplash(unit->getX() - r, unit->getY() - r, unit->getX() + r, unit->getY() + r);
	auto a = scbw::UnitFinder::getNearestTarget(
		unit->getX() - r, unit->getY() - r,
		unit->getX() + r, unit->getY() + r,
		unit, unitRangeCount(unit, context, allowedIds)
	);
	return a;
}

CUnit* rangeMatchPosition(Point32 position, u32 r, u32 context, std::vector<u16> allowedIds) {
	scbw::UnitFinder unitsInSplash(position.x - r, position.y - r, position.x + r, position.x + r);
	auto a = scbw::UnitFinder::getNearestTarget(
		position.x - r, position.y - r,
		position.x + r, position.y + r,
		position, unitRangeCount(nullptr, context, allowedIds)
	);
	return a;
}
