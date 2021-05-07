//Note: Units with smaller attack priority ranks will be attacked first!

#include "attack_priority.h"
#include <SCBW/scbwdata.h>
#include <SCBW/api.h>
#include <cassert>
#include <algorithm>
#include <SCBW/UnitFinder.h>


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

//Global variables (bad practice, but it's faster)
AttackPriorityData attackPriorityData;
scbw::UnitFinder attackTargetFinder;

//Helper function declarations
namespace
{
	bool isTargetPosWithinAttackAngle(s32 x, s32 y, Unit* unit, u8 weaponId);
	bool cannotChaseTarget(Unit* unit, Unit* target);
	//Returns the minimum air/ground weapon range of the @p unit, whichever is smaller.
	u32 getMinimumRange(Unit* unit);

	//Checks whether the @p target should be added to the attack priority group.
	//Based on function @ 0x00442DA0
	bool checkAttackableTarget(Unit* unit, Unit* target, u32 seekRange, u32 minRange = 0)
	{
		//Default StarCraft behavior

		//Don't attack if the target is the unit itself.
		if (unit == target)
			return false;

		//Don't attack if the target is covered by fog of war.
		if (!target->sprite->isVisibleTo(unit->playerId))
			return false;

		//Check if the target can be attacked.
		if (!unit->canAttackTarget(target))
			return false;
		if (unit->id == UnitId::civilian)
		{
			if (unit->_padding_0x132 & 0x8)
			{
				return false;
			}
		}
		//Check if the target is an enemy
		bool is_enemy = unit->isTargetEnemy(target);
		if (unit->isBlind)
		{
			is_enemy = true;
		}
		if (!is_enemy)
			return false;

		//Check if the target is within the minimum weapon range
		if (minRange != 0)
		{
			if (unit->getDistanceToTarget(target) <= minRange)
				return false;
		}

		//Check if the target is within the seek range
		if (!(unit->getDistanceToTarget(target) <= seekRange))
			return false;

		Unit* actualUnit = unit->subunit->isSubunit() ? unit->subunit : unit;

		//If the unit can't turn around, check if the target is within the attack angle.
		if (!(actualUnit->status & UnitStatus::CanTurnAroundToAttack))
		{
			if (!isTargetPosWithinAttackAngle(target->getX(), target->getY(),
				actualUnit, units_dat::GroundWeapon[actualUnit->id]))
			{
				return false;
			}
		}

		//If the target cannot be chased (i.e. ground vs air, is out of range),
		//don't bother
		if (cannotChaseTarget(unit, target))
			return false;

		return true;
	}

} //unnamed namespace

namespace hooks
{

	//Calculates the attack priority of the @p target for the @p attacker.
	u32 getAttackPriorityHook(Unit* target, Unit* attacker)
	{
		//Default StarCraft behavior

		Unit* actualTarget = target;

		if (target->id == UnitId::bunker)
		{
			Unit* firstLoadedUnit = target->getFirstLoadedUnit();
			if (firstLoadedUnit)
				actualTarget = firstLoadedUnit;
		}


		//Units that are not important
		if (scbw::isZergEgg(target->id))
			return 5;

		//Normal units
		u32 attackPriority = 4;

		//Workers
		if (units_dat::BaseProperty[actualTarget->id] & UnitProperty::Worker)
			attackPriority = 2;

		//Units that can fight back
		else if (actualTarget->canAttackTarget(attacker))
			attackPriority = 0;

		//Active combat units
		else if (actualTarget->hasWeapon())
			attackPriority = 2;

		else if (actualTarget->status & UnitStatus::IsBuilding)
			attackPriority = 3;

		if (!(actualTarget->status & UnitStatus::Completed) || actualTarget != target)
			attackPriority += 1;

		//Units under Disruption Web are less important
		if (attackPriority == 0 && actualTarget->status & UnitStatus::CanNotAttack)
			attackPriority = 1;
		if (actualTarget->id == UnitId::scarab)
		{
			attackPriority = 2;
		}
		return attackPriority;
	}

	//Searches for the best attack target nearby for the @p unit.
	Unit* findBestAttackTargetHook(Unit* unit)
	{
		//Default StarCraft behavior

		attackPriorityData.reset();
		s32 seekRange = unit->getSeekRange() * 32;

		//Bunker bonus
		if (unit->status & UnitStatus::InBuilding)
			seekRange += 64;
		//AI units have use default sight range if it is bigger
		else if (unit->pAI)
			seekRange = std::max(seekRange, units_dat::SightRange[unit->id] * 32);

		s32 searchRange = seekRange + 64;
		u32 minRange = getMinimumRange(unit);
		if (unit->id == UnitId::civilian)
		{
			if (unit->_padding_0x132 & 0x8)
			{
				searchRange = 0;
			}
		}
		attackTargetFinder.search(
			unit->getX() - searchRange, unit->getY() - searchRange,
			unit->getX() + searchRange, unit->getY() + searchRange);

		attackTargetFinder.forEach([&](Unit* target)
		{
			if (checkAttackableTarget(unit, target, seekRange, minRange))
				attackPriorityData.addTarget(target, getAttackPriorityHook(target, unit));
		});

		return attackPriorityData.findBestTarget(unit);

	}

	//Searches for a random attack target nearby for the @p unit.
	Unit* findRandomAttackTargetHook(Unit* unit)
	{
		//Default StarCraft behavior

		attackPriorityData.reset();

		s32 seekRange = unit->getSeekRange() * 32;

		//Bunker bonus
		if (unit->status & UnitStatus::InBuilding)
			seekRange += 64;

		s32 searchRange = seekRange + 64;

		attackTargetFinder.search(
			unit->getX() - searchRange, unit->getY() - searchRange,
			unit->getX() + searchRange, unit->getY() + searchRange);

		attackTargetFinder.forEach([&](Unit* target)
		{
			if (checkAttackableTarget(unit, target, seekRange))
				attackPriorityData.addTarget(target, getAttackPriorityHook(target, unit));
		});

		return attackPriorityData.findRandomTarget();
	}

} //hooks


//Returns a random target selected from the attack target groups.
//Based on 0x004401B0
Unit* AttackPriorityData::findRandomTarget() const
{
	//Default StarCraft behavior

	s32 priority = 0;
	while (targetCounts[priority] == 0)
	{
		++priority;
		if (priority >= countof(targetCounts))
			return nullptr;
	}

	if (targetCounts[priority] == 1)
		return targets[priority][0];
	else
		return targets[priority][scbw::random() % targetCounts[priority]];
}

//Returns the best target selected from the attack target groups.
//Based on 0x004405E0
Unit* AttackPriorityData::findBestTarget(Unit* attacker) const
{
	//Default StarCraft behavior

	s32 priority = 0;
	while (targetCounts[priority] == 0)
	{
		++priority;
		if (priority >= countof(targetCounts))
			return nullptr;
	}

	if (targetCounts[priority] == 1)
		return targets[priority][0];

	//Prepare for search
	Unit* bestTarget = targets[priority][0];

	if (attacker->pAI && attacker->id == UnitId::scourge)
	{
		//AI-controlled Scourges auto-target units with the most HP + shields
		u32 bestTargetLife = bestTarget->getCurrentLifeInGame();

		for (u32 i = 1; i < targetCounts[priority]; ++i)
		{
			Unit* currentTarget = targets[priority][i];
			const u32 currentTargetLife = currentTarget->getCurrentLifeInGame();

			if (currentTargetLife > bestTargetLife)
			{
				bestTarget = currentTarget;
				bestTargetLife = currentTargetLife;
			}
		}
	}
	else
	{
		//Find the nearest target (don't use unit box sizes)
		u32 bestTargetDistance = scbw::getDistanceFast(
			attacker->getX(), attacker->getY(),
			bestTarget->getX(), bestTarget->getY());

		for (u32 i = 1; i < targetCounts[priority]; ++i)
		{
			Unit* currentTarget = targets[priority][i];
			const u32 currentTargetDistance = scbw::getDistanceFast(
				attacker->getX(), attacker->getY(),
				currentTarget->getX(), currentTarget->getY());

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

//Identical to function @ 0x00440160
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
	bool isTargetPosWithinAttackAngle(s32 x, s32 y, Unit* unit, u8 weaponId)
	{
		assert(unit);

		s32 angle = scbw::getAngle(unit->getX(), unit->getY(), x, y);

		if (unit->id == UnitId::lurker)
		{
			unit->currentDirection1 = angle;
			return true;
		}

		s32 angleDiff = unit->currentDirection1 - angle;
		if (angleDiff < 0)
			angleDiff += 128;
		if (angleDiff > 128)
			angleDiff = 256 - angleDiff;
		return angleDiff <= weapons_dat::AttackAngle[weaponId];
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

	//Identical to function @ 0x00440520
	u32 getMinimumRange(Unit* unit)
	{
		if (unit->id == UnitId::ProtossClarion && scbw::get_aise_value(unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
		{
			return 0;
		}
		u8 groundWeapon = unit->getGroundWeapon();
		if (groundWeapon == WeaponId::None && unit->subunit)
			groundWeapon = unit->subunit->getGroundWeapon();

		u8 airWeapon = unit->getAirWeapon();
		if (airWeapon == WeaponId::None && unit->subunit)
			airWeapon = unit->subunit->getAirWeapon();

		if (groundWeapon == WeaponId::None)
		{
			if (airWeapon == WeaponId::None)
				return 0;
			else
				return weapons_dat::MinRange[airWeapon];
		}
		else
		{
			if (airWeapon == WeaponId::None)
				return weapons_dat::MinRange[groundWeapon];
			else
				return std::min(weapons_dat::MinRange[groundWeapon], weapons_dat::MinRange[airWeapon]);
		}
	}

} //unnamed namespace
