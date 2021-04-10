#include "hooks\hydraFinder.h"


bool rangeMatch(CUnit* unit, uint32 r, uint32 context, std::vector<uint16> allowedIds) {
	scbw::UnitFinder unitsInSplash(unit->getX() - r, unit->getY() - r, unit->getX() + r, unit->getY() + r);
	auto a = scbw::UnitFinder::getNearestTarget(
		unit->getX() - r, unit->getY() - r,
		unit->getX() + r, unit->getY() + r,
		unit, unitRangeCount(unit, context, allowedIds)
	);
	return a != nullptr;
}

uint32 hydraUnitCount(CUnit* unitPtr, Point16 pos, int r, uint32 flags, std::vector<uint16> unitIds) {
	auto condition = unitRangeCount(unitPtr, flags, unitIds);
	scbw::UnitFinder unitsInSplash(pos.x - r, pos.y - r,
		pos.x + r, pos.y + r);
	auto count = 0;
	for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
		auto unit = unitsInSplash.getUnit(i);
		if (condition(unit)) {
			count++;
		}
	}
	return count;
}
CUnit* rangeMatchUnit(CUnit* unit, uint32 r, uint32 context, std::vector<uint16> allowedIds) {
	scbw::UnitFinder unitsInSplash(unit->getX() - r, unit->getY() - r, unit->getX() + r, unit->getY() + r);
	auto a = scbw::UnitFinder::getNearestTarget(
		unit->getX() - r,
		unit->getY() - r,
		unit->getX() + r,
		unit->getY() + r,
		unit,
		unitRangeCount(unit, context, allowedIds)
	);
	return a;
}

CUnit* rangeMatchPosition(Point32 position, uint32 r, uint32 context, std::vector<uint16> allowedIds) {
	scbw::UnitFinder unitsInSplash(position.x - r, position.y - r, position.x + r, position.x + r);
	auto a = scbw::UnitFinder::getNearestTarget(
		position.x - r,
		position.y - r,
		position.x + r,
		position.y + r,
		position,
		unitRangeCount(nullptr, context, allowedIds)
	);
	return a;
}

scbw::UnitFinderCallbackMatchInterface unitRangeCount(CUnit* center, uint32 context, std::vector<uint16> allowedUnitIds) {
	scbw::UnitFinderCallbackMatchInterface callback = [=](CUnit* unit) {
		if (unit == center && !(context & CFlags::IncludingSelf)) {
			return false;
		}
		if (unit == center && context & CFlags::NotSelf) {
			return false;
		}
		if (unit->status & UnitStatus::Invincible && !(context & CFlags::Invincible)) {
			return false;
		}
		if (unit->status & UnitStatus::InAir && context & CFlags::NoAir) {
			return false;
		}
		if (!(unit->status & UnitStatus::InAir) && context & CFlags::NoGround) {
			return false;
		}
		if (context & CFlags::Spellcaster) {
			if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster)) {
				return false;
			}
		}
		if (context & CFlags::HasEnergy) {
			if (unit->energy < 256) {
				return false;
			}
		}
		if (context & CFlags::Shielded) {
			if (!units_dat::ShieldsEnabled[unit->id]) {
				return false;
			}
			if (unit->shields < 256) {
				return false;
			}
		}
		if (context & CFlags::Powered) {
			if (unit->status & UnitStatus::DoodadStatesThing) {
				return false;
			}
		}
		if (context & CFlags::Completed) {
			if (!(unit->status & UnitStatus::Completed)) {
				return false;
			}
		}
		if (center) {
			if (context & CFlags::FrontalArc) {
				uint16 angle = scbw::getAngle(center->position.x, center->position.y,
					unit->position.x, unit->position.y);
				uint16 unit_angle = center->velocityDirection1;
				int arc = (double)45 / 1.40625;
				//90 degrees
				int delta = unit_angle - angle;
				delta = abs((delta + 128) % 256 - 128);
				if (delta > arc) {
					return false;
				}
			}
			if (scbw::isAlliedTo(unit->playerId, center->playerId) && context & CFlags::Enemy) {
				return false;
			}
			if (!scbw::isAlliedTo(unit->playerId, center->playerId) && context & CFlags::Ally) {
				return false;
			}
		}
		if (allowedUnitIds.size() > 0) {
			bool good = false;
			for (auto a : allowedUnitIds) {
				if (unit->id == a) {
					good = true;
					break;
				}
			}
			if (!good) {
				return false;
			}
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic && context & CFlags::NoOrganic)
			return false;
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Organic) && context & CFlags::OnlyOrganic)
			return false;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical && context & CFlags::NoMechanical)
			return false;
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical) && context & CFlags::OnlyMechanic)
			return false;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building && context & CFlags::NoBuilding)
			return false;
		if (!(scbw::isEnabled(unit)) && context & CFlags::Enabled) {
			return false;
		}
		return true;
	};
	return callback;
}