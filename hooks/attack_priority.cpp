//Note: Units with smaller attack priority ranks will be attacked first!

#include "attack_priority.h"
#include <SCBW/scbwdata.h>
#include <SCBW/api.h>
#include <cassert>
#include <algorithm>
#include <SCBW/UnitFinder.h>
#include <hooks/unit_stats/weapon_range.h>

const s32 ATTACK_PRIORITY_GROUP_SIZE = 16;
const s32 ATTACK_PRIORITY_LEVELS = 6;

struct AttackPriorityData
{
	Unit* findBestTarget(Unit* attacker) const;
	Unit* findRandomTarget() const;

	void addTarget(Unit* target, u32 attackPriority);
	void reset();

	Unit* targets[ATTACK_PRIORITY_LEVELS][ATTACK_PRIORITY_GROUP_SIZE];
	u32 targetCounts[ATTACK_PRIORITY_LEVELS];
};

// Global variables (bad practice, but it's faster)
AttackPriorityData attackPriorityData;
scbw::UnitFinder attackTargetFinder;

namespace
{
	bool isTargetPosWithinAttackAngle(s32 x, s32 y, Unit* unit, u8 weapon_id);
	bool cannotChaseTarget(Unit* unit, Unit* target);
	// Returns the minimum air/ground weapon distance of the @p unit, whichever is smaller.
	u32 get_minimum_range(Unit* unit);

	// Checks whether the @p target should be added to the attack priority group.
	// Based on function @ 0x00442DA0
	bool checkAttackableTarget(Unit* unit, Unit* target, u32 seekRange, u32 minRange = 0)
	{
		// Shouldn't be necessary
		if (!unit || !target)
		{
			return false;
		}

		// Don't attack if the target is the unit itself.
		if (unit == target)
		{
			return false;
		}

		// Don't attack if the target is covered by fog of war.
		if (!target->sprite->isVisibleTo(unit->player))
		{
			return false;
		}

		// Check if the target can be attacked.
		if (!unit->can_attack_target(target))
		{
			return false;
		}

		// Check if the target is an enemy
		bool is_enemy = unit->is_target_enemy(target);
		if (unit->is_blind)
		{
			is_enemy = true;
		}

		if (!is_enemy)
		{
			return false;
		}

		// Check if the target is within the minimum weapon distance
		if (minRange != 0)
		{
			if (unit->get_distance_to_target(target) <= minRange)
			{
				return false;
			}
		}

		Unit* actualUnit = unit->subunit->is_subunit() ? unit->subunit : unit;

		// Check if the target is within the seek distance
		if (!(unit->get_distance_to_target(target) <= seekRange))
		{
			return false;
		}

		// If the unit can't turn around, check if the target is within the attack angle.
		if (!(actualUnit->status & UnitStatus::CanTurnAroundToAttack))
		{
			if (!isTargetPosWithinAttackAngle(target->get_x(), target->get_y(),
				actualUnit, units_dat::GroundWeapon[actualUnit->id]))
			{
				return false;
			}
		}

		// If the target cannot be chased (i.e. ground vs air, is out of distance), don't bother
		if (cannotChaseTarget(unit, target))
		{
			return false;
		}

		return true;
	}	
}

namespace hooks
{
	// Calculates the attack priority of the @p target for the @p attacker.
	u32 getAttackPriorityHook(Unit* target, Unit* attacker)
	{
		Unit* actualTarget = target;

		if (target->id == UnitId::TerranBunker)
		{
			Unit* firstLoadedUnit = target->get_first_loaded_unit();
			if (firstLoadedUnit)
			{
				actualTarget = firstLoadedUnit;
			}
		}

		// Units that are not important
		if (scbw::is_zerg_egg(target->id))
		{
			return 5;
		}

		// Normal units
		u32 attackPriority = 4;

		// Workers
		if (units_dat::base_property[actualTarget->id] & UnitProperty::Worker)
		{
			attackPriority = 2;
		}

		//Units that can fight back
		else if (actualTarget->can_attack_target(attacker))
		{
			attackPriority = 0;
		}

		//Active combat units
		else if (actualTarget->has_weapon())
		{
			attackPriority = 2;
		}

		else if (actualTarget->status & UnitStatus::IsBuilding)
		{
			attackPriority = 3;
		}

		if (!(actualTarget->status & UnitStatus::Completed) || actualTarget != target)
		{
			attackPriority += 1;
		}

		// Units under Disruption Field are less important
		if (attackPriority == 0 && actualTarget->status & UnitStatus::CanNotAttack)
		{
			attackPriority = 1;
		}

		if (actualTarget->id == UnitId::ProtossPlasmaShell)
		{
			attackPriority = 2;
		}

		return attackPriority;
	}

	// Searches for the best attack target nearby for the @p unit.
	Unit* find_best_attack_target(Unit* unit)
	{
		attackPriorityData.reset();
		s32 seek_range = unit->get_seek_range() * 32;

		// Bunker bonus
		s32 offset = 0;
		if (unit->status & UnitStatus::InBuilding)
		{
			offset += 64;
		}
		// AI units have use default sight distance if it is bigger
		else if (unit->unit_ai)
		{
			seek_range = std::max(seek_range, units_dat::SightRange[unit->id] * 32);
		}

		s32 search_range = seek_range + offset + 64;
		u32 min_range = get_minimum_range(unit);
		attackTargetFinder.search(
			unit->get_x() - search_range, unit->get_y() - search_range,
			unit->get_x() + search_range, unit->get_y() + search_range);

		attackTargetFinder.for_each([&](Unit* target)
		{
			s32 target_seek_range = hooks::getSeekRangeHook(unit, target) * 32;
			if (checkAttackableTarget(unit, target, target_seek_range + offset, min_range)) {
				attackPriorityData.addTarget(target, getAttackPriorityHook(target, unit));
			}
		});
		return attackPriorityData.findBestTarget(unit);
	}

	// Searches for a random attack target nearby for the @p unit.
	Unit* findRandomAttackTargetHook(Unit* unit)
	{
		attackPriorityData.reset();
		s32 seek_range = unit->get_seek_range() * 32;

		// Bunker bonus
		u32 offset = 0;
		if (!unit->unit_ai && unit->status & UnitStatus::InBuilding)
		{
			offset += 64;
		}

		s32 search_range = seek_range + offset + 64;
		attackTargetFinder.search(
			unit->get_x() - search_range, unit->get_y() - search_range,
			unit->get_x() + search_range, unit->get_y() + search_range);

		attackTargetFinder.for_each([&](Unit* target)
		{
			s32 target_seek_range = hooks::getSeekRangeHook(unit, target) * 32;
			if (checkAttackableTarget(unit, target, target_seek_range + offset)) {
				attackPriorityData.addTarget(target, getAttackPriorityHook(target, unit));
			}
		});
		return attackPriorityData.findRandomTarget();
	}
}


// Returns a random target selected from the attack target groups.
// Based on 0x004401B0
Unit* AttackPriorityData::findRandomTarget() const
{
	s32 priority = 0;
	while (targetCounts[priority] == 0)
	{
		++priority;
		if (priority >= countof(targetCounts))
		{
			return nullptr;
		}
	}

	if (targetCounts[priority] == 1)
	{
		return targets[priority][0];
	}
	else
	{
		return targets[priority][scbw::random() % targetCounts[priority]];
	}
}

// Returns the best target selected from the attack target groups.
// Based on 0x004405E0
Unit* AttackPriorityData::findBestTarget(Unit* attacker) const
{
	s32 priority = 0;
	while (targetCounts[priority] == 0)
	{
		++priority;
		if (priority >= countof(targetCounts))
		{
			return nullptr;
		}
	}

	if (targetCounts[priority] == 1)
	{
		return targets[priority][0];
	}

	// Prepare for search
	Unit* bestTarget = targets[priority][0];

	if (attacker->unit_ai && attacker->id == UnitId::ZergSkryling)
	{
		// AI-controlled Scourges auto-target units with the most HP + shields
		u32 bestTargetLife = bestTarget->get_current_life_ingame();

		for (u32 i = 1; i < targetCounts[priority]; ++i)
		{
			Unit* currentTarget = targets[priority][i];
			const u32 currentTargetLife = currentTarget->get_current_life_ingame();

			if (currentTargetLife > bestTargetLife)
			{
				bestTarget = currentTarget;
				bestTargetLife = currentTargetLife;
			}
		}
	}
	else
	{
		// Find the nearest target (don't use unit box sizes)
		u32 bestTargetDistance = scbw::get_distance_fast(
			attacker->get_x(), attacker->get_y(),
			bestTarget->get_x(), bestTarget->get_y());

		for (u32 i = 1; i < targetCounts[priority]; ++i)
		{
			Unit* currentTarget = targets[priority][i];
			const u32 currentTargetDistance = scbw::get_distance_fast(
				attacker->get_x(), attacker->get_y(),
				currentTarget->get_x(), currentTarget->get_y());

			if (currentTargetDistance < bestTargetDistance)
			{
				bestTarget = currentTarget;
				bestTargetDistance = currentTargetDistance;
			}
		}
	}
	return bestTarget;
}


//-------- The following function definitions should not be touched. --------//

// Identical to function @ 0x00440160
void AttackPriorityData::addTarget(Unit* target, u32 attackPriority)
{
	assert(target);
	assert(attackPriority < countof(targetCounts));

	if (targetCounts[attackPriority] < ATTACK_PRIORITY_GROUP_SIZE)
	{
		targets[attackPriority][targetCounts[attackPriority]] = target;
		targetCounts[attackPriority] += 1;
	}
}

void AttackPriorityData::reset()
{
	for (s32 i = 0; i < countof(targetCounts); ++i)
		targetCounts[i] = 0;
}

namespace
{

	//Identical to function @ 0x00475BE0
	bool isTargetPosWithinAttackAngle(s32 x, s32 y, Unit* unit, u8 weapon_id)
	{
		assert(unit);

		s32 angle = scbw::get_angle(unit->get_x(), unit->get_y(), x, y);

		if (unit->id == UnitId::ZergLakizilisk)
		{
			unit->current_direction_1 = angle;
			return true;
		}

		s32 angleDiff = unit->current_direction_1 - angle;
		if (angleDiff < 0)
			angleDiff += 128;
		if (angleDiff > 128)
			angleDiff = 256 - angleDiff;
		return angleDiff <= weapons_dat::AttackAngle[weapon_id];
	}

	const u32 Func_CannotChaseTarget = 0x004A1140;
	bool cannotChaseTarget(Unit* unit, Unit* target)
	{
		assert(unit);
		assert(target);
		static u32 result;

		__asm {
			PUSHAD
			MOV ESI, target
			MOV ECX, unit
			CALL Func_CannotChaseTarget
			MOV result, EAX
			POPAD
		}

		return result != 0;
	}

	u32 get_minimum_range(Unit* unit) // 00440520
	{
		if (unit->id == UnitId::ProtossClarion && scbw::get_aise_value(unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
		{
			return 0;
		}
		u8 groundWeapon = unit->get_ground_weapon();
		if (groundWeapon == WeaponId::None && unit->subunit)
		{
			groundWeapon = unit->subunit->get_ground_weapon();
		}

		u8 airWeapon = unit->get_air_weapon();
		if (airWeapon == WeaponId::None && unit->subunit)
		{
			airWeapon = unit->subunit->get_air_weapon();
		}

		if (groundWeapon == WeaponId::None)
		{
			if (airWeapon == WeaponId::None)
			{
				return 0;
			}
			else
			{
				return weapons_dat::MinRange[airWeapon];
			}
		}
		else
		{
			if (airWeapon == WeaponId::None)
			{
				return weapons_dat::MinRange[groundWeapon];
			}
			else
			{
				return std::min(weapons_dat::MinRange[groundWeapon], weapons_dat::MinRange[airWeapon]);
			}
		}
	}

} //unnamed namespace
