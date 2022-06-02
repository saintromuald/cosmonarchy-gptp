#include "hooks\cosmonarchy_finder.h"


bool rangeMatch(Unit* unit, u32 r, u32 context, std::vector<u16> allowedIds)
{
	scbw::UnitFinder unitsInSplash(unit->get_x() - r, unit->get_y() - r, unit->get_x() + r, unit->get_y() + r);
	auto a = scbw::UnitFinder::getNearestTarget(
		unit->get_x() - r, unit->get_y() - r,
		unit->get_x() + r, unit->get_y() + r,
		unit, cosmonarchyFinderMatch(unit, context, allowedIds)
	);
	return a != nullptr;
}

u32 cosmonarchyUnitCount(Unit* unitPtr, Point16 pos, s32 r, u32 flags, std::vector<u16> unitIds)
{
	auto condition = cosmonarchyFinderMatch(unitPtr, flags, unitIds);
	scbw::UnitFinder unitsInSplash(pos.x - r, pos.y - r,
		pos.x + r, pos.y + r);
	auto count = 0;
	for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
	{
		auto unit = unitsInSplash.getUnit(i);
		if (condition(unit))
		{
			count++;
		}
	}
	return count;
}
Unit* rangeMatchUnit(Unit* unit, u32 r, u32 context, std::vector<u16> allowedIds)
{
	scbw::UnitFinder unitsInSplash(unit->get_x() - r, unit->get_y() - r, unit->get_x() + r, unit->get_y() + r);
	auto a = scbw::UnitFinder::getNearestTarget(
		unit->get_x() - r,
		unit->get_y() - r,
		unit->get_x() + r,
		unit->get_y() + r,
		unit,
		cosmonarchyFinderMatch(unit, context, allowedIds)
	);
	return a;
}

Unit* rangeMatchPosition(Vector32 position, u32 r, u32 context, std::vector<u16> allowedIds)
{
	scbw::UnitFinder unitsInSplash(position.x - r, position.y - r, position.x + r, position.x + r);
	auto a = scbw::UnitFinder::getNearestTarget(
		position.x - r,
		position.y - r,
		position.x + r,
		position.y + r,
		position,
		cosmonarchyFinderMatch(nullptr, context, allowedIds)
	);
	return a;
}

scbw::UnitFinderCallbackMatchInterface cosmonarchyFinderMatch(Unit* center, u32 context, std::vector<u16> allowedUnitIds)
{
	scbw::UnitFinderCallbackMatchInterface callback = [=](Unit* unit)
	{
		if (unit == center && !(context & Flags::IncludingSelf))
		{
			return false;
		}
		if (unit == center && context & Flags::NotSelf)
		{
			return false;
		}
		if (unit->status & UnitStatus::Invincible && !(context & Flags::Invincible))
		{
			return false;
		}
		if (unit->status & UnitStatus::InAir && context & Flags::NoAir)
		{
			return false;
		}
		if (!(unit->status & UnitStatus::InAir) && context & Flags::NoGround)
		{
			return false;
		}
		if (context & Flags::Spellcaster)
		{
			if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster))
			{
				return false;
			}
		}
		if (context & Flags::HasEnergy)
		{
			if (unit->energy < 256)
			{
				return false;
			}
		}
		if (context & Flags::Shielded)
		{
			if (!units_dat::ShieldsEnabled[unit->id])
			{
				return false;
			}
			if (unit->shields < 256)
			{
				return false;
			}
		}
		if (context & Flags::Powered)
		{
			if (unit->status & UnitStatus::Frozen)
			{
				return false;
			}
		}
		if (context & Flags::Completed)
		{
			if (!(unit->status & UnitStatus::Completed))
			{
				return false;
			}
		}
		if (context & Flags::NoTrap)
		{
			if (scbw::unit_is_trap(unit))
			{
				return false;
			}
		}
		if (context & Flags::NoHallucination)
		{
			if (unit->status & UnitStatus::IsHallucination)
			{
				return false;
			}
		}
		if (center)
		{
			if (context & Flags::FrontalArc)
			{
				u16 angle = scbw::get_angle(center->position.x, center->position.y,
					unit->position.x, unit->position.y);
				u16 unit_angle = center->velocityDirection1;
				s32 arc = (f64)45 / 1.40625;
				//90 degrees
				s32 delta = unit_angle - angle;
				delta = abs((delta + 128) % 256 - 128);
				if (delta > arc)
				{
					return false;
				}
			}
			if (context & Flags::Own && unit->player != center->player)
			{
				return false;
			}
			if (scbw::is_allied_to(unit->player, center->player) && context & Flags::Enemy)
			{
				return false;
			}
			if (!scbw::is_allied_to(unit->player, center->player) && context & Flags::Ally)
			{
				return false;
			}
		}
		if (allowedUnitIds.size() > 0)
		{
			bool good = false;
			for (auto a : allowedUnitIds)
			{
				if (unit->id == a)
				{
					good = true;
					break;
				}
			}
			if (!good)
			{
				return false;
			}
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic && context & Flags::NoOrganic)
		{
			return false;
		}
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Organic) && context & Flags::OnlyOrganic)
		{
			return false;
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical && context & Flags::NoMechanical)
		{
			return false;
		}
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical) && context & Flags::OnlyMechanic)
		{
			return false;
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building && context & Flags::NoBuilding)
		{
			return false;
		}
		if (!(scbw::is_enabled(unit)) && context & Flags::Enabled)
		{
			return false;
		}
		if (context & Flags::CanAttackOrCast && !unit->has_weapon() && !(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster))
		{
			return false;
		}
		if (context & Flags::NotWorker && units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
		{
			return false;
		}
		if (context & Flags::NoEgg && (unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon))
		{
			return false;
		}
		if (context & Flags::NoLarva && unit->id == UnitId::ZergLarva)
		{
			return false;
		}
		if (context & Flags::NoHangar && (unit->id == UnitId::ProtossPlasmaShell || unit->id == UnitId::ProtossSteward))
		{
			return false;
		}
		return true;
	};
	return callback;
}