#include "attack_priority.h"
#include <BW/bwdata.h>
#include <BW/api.h>
#include <cassert>
#include <algorithm>
#include <BW/UnitFinder.h>
#include <hooks/unit_stats/weapon_range.h>

// Note: Units with smaller attack priority ranks will be attacked first!
const s32 ATTACK_PRIORITY_GROUP_SIZE = 16;
const s32 ATTACK_PRIORITY_LEVELS = 6;

struct AttackPriorityData {
	Unit* find_best_target(Unit* attacker) const;
	Unit* find_random_target() const;

	void add_target(Unit* target, u32 attackPriority);
	void reset();

	Unit* targets[ATTACK_PRIORITY_LEVELS][ATTACK_PRIORITY_GROUP_SIZE];
	u32 target_counts[ATTACK_PRIORITY_LEVELS];
};

// Global variables (bad practice, but it's faster)
AttackPriorityData attack_priority_data;
bw::UnitFinder attack_target_finder;

namespace {
	bool is_target_pos_within_attack_angle(s32 x, s32 y, Unit* unit, u8 weapon_id);
	// Returns the minimum edge/inside weapon distance of the @p unit, whichever is smaller.
	u32 get_minimum_range(Unit* unit);

	// Checks whether the @p target should be added to the attack priority group.
	// Based on function @ 0x00442DA0
	bool check_attackable_target(Unit* unit, Unit* target, u32 seekRange, u32 minRange = 0) {
		// Shouldn't be necessary
		if (!unit || !target) {
			return false;
		}
		// Don't attack if the target is the unit itself.
		if (unit == target) {
			return false;
		}
		// Don't attack if the target is covered by fog of war.
		if (!target->sprite->is_visible_to(unit->player)) {
			return false;
		}
		// Check if the target can be attacked.
		if (!unit->can_attack_target(target)) {
			return false;
		}
		// Check if the target is an enemy
		bool is_enemy = unit->is_target_enemy(target);
		if (unit->is_blind) {
			is_enemy = true;
		}
		if (!is_enemy) {
			return false;
		}
		// Check if the target is within the minimum weapon distance
		if (minRange != 0) {
			if (unit->get_distance_to_target(target) <= minRange) {
				return false;
			}
		}
		Unit* final_unit = unit->subunit->is_subunit() ? unit->subunit : unit;
		// Check if the target is within the seek distance
		if (!(unit->get_distance_to_target(target) <= seekRange)) {
			return false;
		}
		if (unit_id_has_custom_air_weapon(unit->id) && target->is_in_air()) {
			return false;
		}
		// If the unit can't turn around, check if the target is within the attack angle.
		if (!(final_unit->status & UnitStatus::CanTurnAroundToAttack)) {
			if (!is_target_pos_within_attack_angle(target->get_x(), target->get_y(), final_unit, units_dat::ground_weapon[final_unit->id])) {
				return false;
			}
		}
		// If the target cannot be chased (i.e. inside vs edge, is out of distance), don't bother
		if (helper::cannot_chase_target(unit, target)) {
			return false;
		}
		if (!unit_should_be_autoattacked_by_player(target, unit->player)) {
			return false;
		}
		return true;
	}	
}

namespace hooks {
	// Calculates the attack priority of the @p target for the @p attacker.
	u32 get_attack_priority(Unit* attacker, Unit* target) { // 00442160
		Unit* actual_target = target;
		if (unit_id_is_garrison(target->id)) {
			Unit* first_loaded_unit = target->get_first_loaded_unit();
			if (first_loaded_unit) {
				actual_target = first_loaded_unit;
			}
		}

		// Units that are not important
		if (bw::is_zerg_egg(target->id) || unit_id_is_larva(target->id)) {
			return 5;
		}
		// Normal units
		u32 attack_priority = 4;
		// Workers
		if (actual_target->is_worker()) {
			attack_priority = 2;
		} else {
			if (actual_target->can_attack_target(attacker)) {
				attack_priority = 0;
			} else {
				if (helper::ai_unit_can_attack(actual_target)) {
					attack_priority = 2;
				} else if (actual_target->status & UnitStatus::Reacts) {
					attack_priority = 3;
				}
			}
		}

		if (!actual_target->is_completed() || actual_target != target) {
			attack_priority += 1;
		}
		// Units that cannot attack are less important
		if (attack_priority == 0 && actual_target->status & UnitStatus::CanNotAttack) {
			attack_priority = 1;
		}
		if (!unit_should_be_autoattacked_by_player(target, attacker->player)) {
			attack_priority = 0;
		}
		return attack_priority;
	}

	// Searches for the best attack target nearby for the @p unit.
	Unit* find_best_attack_target(Unit* unit) {
		attack_priority_data.reset();
		s32 seek_range = unit->get_seek_range() * 32;

		// Garrison bonus
		s32 offset = 0;
		if (unit->is_garrisoned()) {
			offset += 64;
		} else if (unit->unit_ai) { // CPU units use default sight distance if it is bigger
			seek_range = std::max(seek_range, units_dat::sight_range[unit->id] * 32);
		}

		s32 search_range = seek_range + offset + 64;
		u32 min_range = get_minimum_range(unit);
		attack_target_finder.search(unit->get_x() - search_range, unit->get_y() - search_range, unit->get_x() + search_range, unit->get_y() + search_range);
		attack_target_finder.for_each([&](Unit* target) {
			s32 target_seek_range = hooks::get_seek_range(unit, target) * 32;
			if (check_attackable_target(unit, target, target_seek_range + offset, min_range)) {
				attack_priority_data.add_target(target, get_attack_priority(target, unit));
			}
		});
		return attack_priority_data.find_best_target(unit);
	}

	// Searches for a random attack target nearby for the @p unit.
	Unit* find_random_attack_target(Unit* unit) {
		attack_priority_data.reset();
		s32 seek_range = unit->get_seek_range() * 32;

		// Bunker bonus
		u32 offset = 0;
		if (unit->is_garrisoned()) {
			offset += 64;
		}
		s32 search_range = seek_range + offset + 64;
		attack_target_finder.search(unit->get_x() - search_range, unit->get_y() - search_range, unit->get_x() + search_range, unit->get_y() + search_range);
		attack_target_finder.for_each([&](Unit* target) {
			s32 target_seek_range = hooks::get_seek_range(unit, target) * 32;
			if (check_attackable_target(unit, target, target_seek_range + offset)) {
				attack_priority_data.add_target(target, get_attack_priority(target, unit));
			}
		});
		return attack_priority_data.find_random_target();
	}
}

// Returns a random target selected from the attack target groups.
// Based on 0x004401B0
Unit* AttackPriorityData::find_random_target() const {
	s32 priority = 0;
	while (target_counts[priority] == 0) {
		++priority;
		if (priority >= countof(target_counts)) {
			return 0;
		}
	}
	if (target_counts[priority] == 1) {
		return targets[priority][0];
	}
	return targets[priority][bw::random() % target_counts[priority]];
}

// Returns the best target selected from the attack target groups.
// Based on 0x004405E0
Unit* AttackPriorityData::find_best_target(Unit* attacker) const {
	s32 priority = 0;
	while (target_counts[priority] == 0) {
		++priority;
		if (priority >= countof(target_counts)) {
			return 0;
		}
	}

	if (target_counts[priority] == 1) {
		return targets[priority][0];
	}

	// Prepare for search
	Unit* best_target = targets[priority][0];
	// Find the nearest target (don't use unit box sizes)
	u32 best_target_distance = bw::distance_between(attacker->get_x(), attacker->get_y(), best_target->get_x(), best_target->get_y());

	for (u32 i = 1; i < target_counts[priority]; ++i) {
		Unit* current_target = targets[priority][i];
		const u32 current_target_distance = bw::distance_between(attacker->get_x(), attacker->get_y(), current_target->get_x(), current_target->get_y());
		if (current_target_distance < best_target_distance) {
			best_target = current_target;
			best_target_distance = current_target_distance;
		}
	}
	return best_target;
}

//-------- The following function definitions should not be touched. --------//

// Identical to function @ 0x00440160
void AttackPriorityData::add_target(Unit* target, u32 attackPriority) {
	assert(target);
	assert(attackPriority < countof(target_counts));
	if (target_counts[attackPriority] < ATTACK_PRIORITY_GROUP_SIZE) {
		targets[attackPriority][target_counts[attackPriority]] = target;
		target_counts[attackPriority] += 1;
	}
}

void AttackPriorityData::reset() {
	for (s32 i = 0; i < countof(target_counts); ++i) {
		target_counts[i] = 0;
	}
}

namespace {
	bool is_target_pos_within_attack_angle(s32 x, s32 y, Unit* unit, u8 weapon_id) { // 00475BE0
		assert(unit);
		s32 angle = bw::get_angle(unit->get_x(), unit->get_y(), x, y);
		if (unit_id_is_burrowed_attacker(unit->id)) {
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

	u32 get_minimum_range(Unit* unit) { // 00440520
		u8 ground_weapon = unit->get_ground_weapon();
		if (ground_weapon == WeaponId::None && unit->subunit) {
			ground_weapon = unit->subunit->get_ground_weapon();
		}
		u8 air_weapon = unit->get_air_weapon();
		if (air_weapon == WeaponId::None && unit->subunit) {
			air_weapon = unit->subunit->get_air_weapon();
		}
		if (ground_weapon == WeaponId::None) {
			if (air_weapon == WeaponId::None) {
				return 0;
			}
			return weapons_dat::min_range[air_weapon];
		}
		if (air_weapon == WeaponId::None) {
			return weapons_dat::min_range[ground_weapon];
		}
		return std::min(weapons_dat::min_range[ground_weapon], weapons_dat::min_range[air_weapon]);
	}
}