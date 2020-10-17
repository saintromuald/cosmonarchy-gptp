#include "hooks\hydraFinder.h"


bool rangeMatch(CUnit* unit, u32 r, u32 context, std::vector<u16> allowedIds) {
	scbw::UnitFinder unitsInSplash(unit->getX() - r, unit->getY() - r, unit->getX() + r, unit->getY() + r);
	auto a = scbw::UnitFinder::getNearestTarget(
		unit->getX() - r, unit->getY() - r,
		unit->getX() + r, unit->getY() + r,
		unit, unitRangeCount(unit, context, allowedIds)
	);
	return a != NULL;
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
		position, unitRangeCount(NULL, context, allowedIds)
	);
	return a;
}
