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
	Unit* find_best_target(Unit* attacker) const;
	Unit* find_random_target() const;

	void add_target(Unit* target, u32 attackPriority);
	void reset();

	Unit* targets[ATTACK_PRIORITY_LEVELS][ATTACK_PRIORITY_GROUP_SIZE];
	u32 target_counts[ATTACK_PRIORITY_LEVELS];
};

// Global variables (bad practice, but it's faster)
AttackPriorityData attackPriorityData;
scbw::UnitFinder attackTargetFinder;

namespace
{
	bool is_target_pos_within_attack_angle(s32 x, s32 y, Unit* unit, u8 weapon_id);
	bool cannot_chase_target(Unit* unit, Unit* target);
	// Returns the minimum air/ground weapon distance of the @p unit, whichever is smaller.
	u32 get_minimum_range(Unit* unit);

	// Checks whether the @p target should be added to the attack priority group.
	// Based on function @ 0x00442DA0
	bool check_attackable_target(Unit* unit, Unit* target, u32 seekRange, u32 minRange = 0)
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
		if (!target->sprite->is_visible_to(unit->player))
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
			if (!is_target_pos_within_attack_angle(target->get_x(), target->get_y(),
				actualUnit, units_dat::ground_weapon[actualUnit->id]))
			{
				return false;
			}
		}

		// If the target cannot be chased (i.e. ground vs air, is out of distance), don't bother
		if (cannot_chase_target(unit, target))
		{
			return false;
		}

		return true;
	}	
}

namespace hooks
{
	// Calculates the attack priority of the @p target for the @p attacker.
	u32 get_attack_priority(Unit* target, Unit* attacker)
	{
		Unit* actual_target = target;

		if (unit_id_is_bunker(target->id))
		{
			Unit* first_loaded_unit = target->get_first_loaded_unit();
			if (first_loaded_unit)
			{
				actual_target = first_loaded_unit;
			}
		}

		// Units that are not important
		if (scbw::is_zerg_egg(target->id))
		{
			return 5;
		}

		// Normal units
		u32 attack_priority = 4;

		// Workers
		if (units_dat::base_property[actual_target->id] & UnitProperty::Worker)
		{
			attack_priority = 2;
		}

		// Units that can fight back
		else if (actual_target->can_attack_target(attacker))
		{
			attack_priority = 0;
		}

		// Active combat units
		else if (actual_target->has_weapon())
		{
			attack_priority = 2;
		}

		else if (actual_target->status & UnitStatus::Reacts)
		{
			attack_priority = 3;
		}

		if (!(actual_target->status & UnitStatus::Completed) || actual_target != target)
		{
			attack_priority += 1;
		}

		// Units under Disruption Field are less important
		if (attack_priority == 0 && actual_target->status & UnitStatus::CanNotAttack)
		{
			attack_priority = 1;
		}

		if (actual_target->id == UnitId::ProtossPlasmaShell)
		{
			attack_priority = 2;
		}

		return attack_priority;
	}

	// Searches for the best attack target nearby for the @p unit.
	Unit* find_best_attack_target(Unit* unit) {
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
			seek_range = std::max(seek_range, units_dat::sight_range[unit->id] * 32);
		}

		s32 search_range = seek_range + offset + 64;
		u32 min_range = get_minimum_range(unit);
		attackTargetFinder.search(
			unit->get_x() - search_range, unit->get_y() - search_range,
			unit->get_x() + search_range, unit->get_y() + search_range);

		attackTargetFinder.for_each([&](Unit* target) {
			s32 target_seek_range = hooks::get_seek_range(unit, target) * 32;
			if (check_attackable_target(unit, target, target_seek_range + offset, min_range)) {
				attackPriorityData.add_target(target, get_attack_priority(target, unit));
			}
		});
		return attackPriorityData.find_best_target(unit);
	}

	// Searches for a random attack target nearby for the @p unit.
	Unit* find_random_attack_target(Unit* unit)
	{
		attackPriorityData.reset();
		s32 seek_range = unit->get_seek_range() * 32;

		// Bunker bonus
		u32 offset = 0;
		if (unit->status & UnitStatus::InBuilding)
		{
			offset += 64;
		}

		s32 search_range = seek_range + offset + 64;
		attackTargetFinder.search(
			unit->get_x() - search_range, unit->get_y() - search_range,
			unit->get_x() + search_range, unit->get_y() + search_range);

		attackTargetFinder.for_each([&](Unit* target)
		{
			s32 target_seek_range = hooks::get_seek_range(unit, target) * 32;
			if (check_attackable_target(unit, target, target_seek_range + offset)) {
				attackPriorityData.add_target(target, get_attack_priority(target, unit));
			}
		});
		return attackPriorityData.find_random_target();
	}
}

// Returns a random target selected from the attack target groups.
// Based on 0x004401B0
Unit* AttackPriorityData::find_random_target() const
{
	s32 priority = 0;
	while (target_counts[priority] == 0)
	{
		++priority;
		if (priority >= countof(target_counts))
		{
			return nullptr;
		}
	}

	if (target_counts[priority] == 1)
	{
		return targets[priority][0];
	}
	else
	{
		return targets[priority][scbw::random() % target_counts[priority]];
	}
}

// Returns the best target selected from the attack target groups.
// Based on 0x004405E0
Unit* AttackPriorityData::find_best_target(Unit* attacker) const
{
	s32 priority = 0;
	while (target_counts[priority] == 0)
	{
		++priority;
		if (priority >= countof(target_counts))
		{
			return nullptr;
		}
	}

	if (target_counts[priority] == 1)
	{
		return targets[priority][0];
	}

	// Prepare for search
	Unit* best_target = targets[priority][0];

	// AI auto-targeted units with high shields/hp when using Skrylings, but we don't have them anymore, and we already handle that in tactical_ai! -Pr0nogo
	
	// Find the nearest target (don't use unit box sizes)
	u32 best_target_distance = scbw::get_distance_fast(attacker->get_x(), attacker->get_y(), best_target->get_x(), best_target->get_y());

	for (u32 i = 1; i < target_counts[priority]; ++i)
	{
		Unit* current_target = targets[priority][i];
		const u32 current_target_distance = scbw::get_distance_fast(attacker->get_x(), attacker->get_y(), current_target->get_x(), current_target->get_y());

		if (current_target_distance < best_target_distance)
		{
			best_target = current_target;
			best_target_distance = current_target_distance;
		}
	}
	return best_target;
}

//-------- The following function definitions should not be touched. --------//

// Identical to function @ 0x00440160
void AttackPriorityData::add_target(Unit* target, u32 attackPriority)
{
	assert(target);
	assert(attackPriority < countof(target_counts));

	if (target_counts[attackPriority] < ATTACK_PRIORITY_GROUP_SIZE)
	{
		targets[attackPriority][target_counts[attackPriority]] = target;
		target_counts[attackPriority] += 1;
	}
}

void AttackPriorityData::reset()
{
	for (s32 i = 0; i < countof(target_counts); ++i) {
		target_counts[i] = 0;
	}
}

namespace
{
	bool is_target_pos_within_attack_angle(s32 x, s32 y, Unit* unit, u8 weapon_id) // 00475BE0
	{
		assert(unit);

		s32 angle = scbw::get_angle(unit->get_x(), unit->get_y(), x, y);

		if (unit->id == UnitId::ZergLakizilisk)
		{
			unit->current_direction_1 = angle;
			return true;
		}

		s32 angle_difference = unit->current_direction_1 - angle;
		if (angle_difference < 0) {
			angle_difference += 128;
		}
		if (angle_difference > 128) {
			angle_difference = 256 - angle_difference;
		}
		return angle_difference <= weapons_dat::attack_angle[weapon_id];
	}

	const u32 Func_CannotChaseTarget = 0x004A1140;
	bool cannot_chase_target(Unit* unit, Unit* target)
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
		u8 ground_weapon = unit->get_ground_weapon();
		if (ground_weapon == WeaponId::None && unit->subunit)
		{
			ground_weapon = unit->subunit->get_ground_weapon();
		}

		u8 air_weapon = unit->get_air_weapon();
		if (air_weapon == WeaponId::None && unit->subunit)
		{
			air_weapon = unit->subunit->get_air_weapon();
		}

		if (ground_weapon == WeaponId::None)
		{
			if (air_weapon == WeaponId::None)
			{
				return 0;
			} else {
				return weapons_dat::min_range[air_weapon];
			}
		} else {
			if (air_weapon == WeaponId::None)
			{
				return weapons_dat::min_range[ground_weapon];
			} else {
				return std::min(weapons_dat::min_range[ground_weapon], weapons_dat::min_range[air_weapon]);
			}
		}
	}
}