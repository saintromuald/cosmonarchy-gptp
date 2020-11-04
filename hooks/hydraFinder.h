#pragma once
#include "SCBW\api.h"
#include "SCBW\scbwdata.h"
#include "SCBW\UnitFinder.h"
#include <vector>

namespace CFlags {
	enum {
		NoInvincible=1<<0,
		NoAir=1<<1,
		NoGround=1<<2,
		NoMechanical=1<<3,
		NoOrganic=1<<4,
		NoBuilding=1<<5,
		OnlyOrganic=1<<6,
		OnlyMechanic=1<<7,
		Enemy=1<<8,
		Ally=1<<9,
		IncludingSelf=1<<10,
		FrontalArc=1<<11,
	};
}

class unitRangeCount : public scbw::UnitFinderCallbackMatchInterface
{
private:
	CUnit* center;
	u32 context;//flags
	std::vector<u16> allowedUnitIds;
public:
	unitRangeCount(CUnit* center, u32 context, std::vector<u16> allowedUnitIds)
		: center(center), context(context), allowedUnitIds(allowedUnitIds) {}

	bool match(CUnit* unit) {
		if (unit == center && !(context & CFlags::IncludingSelf)) {
			return false;
		}
		if (unit->status & UnitStatus::Invincible && context & CFlags::NoInvincible) {
			return false;
		}
		if (unit->status & UnitStatus::InAir && context & CFlags::NoAir) {
			return false;
		}
		if (!(unit->status & UnitStatus::InAir) && context & CFlags::NoGround) {
			return 0;
		}

		
		if (center) {
			if (context & CFlags::FrontalArc) {
				u16 angle = scbw::getAngle(center->position.x, center->position.y,
					unit->position.x, unit->position.y);
				u16 unit_angle = center->velocityDirection1;
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
		return true;
	}
};
bool rangeMatch(CUnit* unit, u32 r, u32 context, std::vector<u16> allowedIds);
CUnit* rangeMatchUnit(CUnit* unit, u32 r, u32 context, std::vector<u16> allowedIds);
CUnit* rangeMatchPosition(Point32 position, u32 r, u32 context, std::vector<u16> allowedIds);