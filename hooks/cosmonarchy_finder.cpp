#include "hooks\cosmonarchy_finder.h"


bool rangeMatch(Unit* unit, u32 r, u32 context, std::vector<u16> allowedIds)
{
	scbw::UnitFinder unitsInSplash(unit->get_x() - r, unit->get_y() - r, unit->get_x() + r, unit->get_y() + r);
	auto a = scbw::UnitFinder::get_nearest_target(
		unit->get_x() - r, unit->get_y() - r,
		unit->get_x() + r, unit->get_y() + r,
		unit, cosmonarchy_finder_match(unit, context, allowedIds)
	);
	return a != nullptr;
}

u32 cosmonarchy_unit_count(Unit* unitPtr, Point16 pos, s32 r, u32 flags, std::vector<u16> unitIds)
{
	auto condition = cosmonarchy_finder_match(unitPtr, flags, unitIds);
	scbw::UnitFinder unitsInSplash(pos.x - r, pos.y - r,
		pos.x + r, pos.y + r);
	auto count = 0;
	for (s32 i = 0; i < unitsInSplash.get_unit_count(); i++)
	{
		auto unit = unitsInSplash.get_unit(i);
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
	auto a = scbw::UnitFinder::get_nearest_target(
		unit->get_x() - r,
		unit->get_y() - r,
		unit->get_x() + r,
		unit->get_y() + r,
		unit,
		cosmonarchy_finder_match(unit, context, allowedIds)
	);
	return a;
}

Unit* rangeMatchPosition(Vector32 position, u32 r, u32 context, std::vector<u16> allowedIds)
{
	scbw::UnitFinder unitsInSplash(position.x - r, position.y - r, position.x + r, position.x + r);
	auto a = scbw::UnitFinder::get_nearest_target(
		position.x - r,
		position.y - r,
		position.x + r,
		position.y + r,
		position,
		cosmonarchy_finder_match(nullptr, context, allowedIds)
	);
	return a;
}

scbw::UnitFinder::CallbackMatch cosmonarchy_finder_match(Unit* center, u32 context, std::vector<u16> allowedUnitIds)
{
	scbw::UnitFinder::CallbackMatch callback = [=](Unit* target)
	{
		if (target == center && !(context & Flags::IncludingSelf))
		{
			return false;
		}
		if (target == center && context & Flags::NotSelf)
		{
			return false;
		}
		if (target->status & UnitStatus::Invincible && !(context & Flags::Invincible))
		{
			return false;
		}
		if (target->status & UnitStatus::InAir && context & Flags::NoAir)
		{
			return false;
		}
		if (!(target->status & UnitStatus::InAir) && context & Flags::NoGround)
		{
			return false;
		}
		if (context & Flags::Spellcaster)
		{
			if (!(units_dat::base_property[target->id] & UnitProperty::Spellcaster))
			{
				return false;
			}
		}
		if (context & Flags::HasEnergy)
		{
			if (target->energy < 256)
			{
				return false;
			}
		}
		if (context & Flags::Shielded)
		{
			if (!scbw::unit_has_shields(target))
			{
				return false;
			}
			if (target->shields < 256)
			{
				return false;
			}
		}
		if (context & Flags::Powered)
		{
			if (target->status & UnitStatus::Frozen)
			{
				return false;
			}
		}
		if (context & Flags::Completed)
		{
			if (!(target->status & UnitStatus::Completed))
			{
				return false;
			}
		}
		if (context & Flags::NoTrap)
		{
			if (unit_id_is_trap(target->id))
			{
				return false;
			}
		}
		if (context & Flags::NoHallucination)
		{
			if (target->status & UnitStatus::IsHallucination)
			{
				return false;
			}
		}
		if (center)
		{
			if (context & Flags::FrontalArc)
			{
				if (!scbw::unit_is_facing_position(center, target->position)) {
					return false;
				}
			}
			if (context & Flags::Own && target->player != center->player)
			{
				return false;
			}
			if (scbw::is_allied_to(target->player, center->player) && context & Flags::Enemy)
			{
				return false;
			}
			if (!scbw::is_allied_to(target->player, center->player) && context & Flags::Ally)
			{
				return false;
			}
		}
		if (allowedUnitIds.size() > 0)
		{
			bool good = false;
			for (auto a : allowedUnitIds)
			{
				if (target->id == a)
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
		if (units_dat::base_property[target->id] & UnitProperty::Organic && context & Flags::NoOrganic)
		{
			return false;
		}
		if (!(units_dat::base_property[target->id] & UnitProperty::Organic) && context & Flags::OnlyOrganic)
		{
			return false;
		}
		if (units_dat::base_property[target->id] & UnitProperty::Mechanical && context & Flags::NoMechanical)
		{
			return false;
		}
		if (!(units_dat::base_property[target->id] & UnitProperty::Mechanical) && context & Flags::OnlyMechanic)
		{
			return false;
		}
		if (units_dat::base_property[target->id] & UnitProperty::Building && context & Flags::NoBuilding)
		{
			return false;
		}
		if (!(scbw::is_enabled(target)) && context & Flags::Enabled)
		{
			return false;
		}
		if (context & Flags::CanAttackOrCast && !target->has_weapon() && !(units_dat::base_property[target->id] & UnitProperty::Spellcaster))
		{
			return false;
		}
		if (context & Flags::NotWorker && units_dat::base_property[target->id] & UnitProperty::Worker)
		{
			return false;
		}
		if (context & Flags::NoEgg && (target->id == UnitId::ZergEgg || target->id == UnitId::ZergCocoon))
		{
			return false;
		}
		if (context & Flags::NoLarva && (target->id == UnitId::ZergLarva || target->id == UnitId::ZergOcsalLarva))
		{
			return false;
		}
		if (context & Flags::NoHangar && (target->id == UnitId::ProtossPlasmaShell || target->id == UnitId::ProtossSteward))
		{
			return false;
		}
		return true;
	};
	return callback;
}