#include "ai_common.h"
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include <globals.h>
#include <cassert>

namespace AI
{
	// Equivalent to 00440E30 ai_target_unit_is_worth_hitting
	bool isTargetWorthHitting(Unit* target, Unit* attacker)
	{
		// If the target is hidden by the fog-of-war
		if (!target->sprite->isVisibleTo(attacker->player))
		{
			return false;
		}
		// If the target is not detectable
		if (target->status & 0x300 && !((1 << attacker->player) & target->visibility_status) && !target->is_visible_to(attacker->player))
		{
			return false;
		}
		if (!attacker->is_target_enemy(target))
		{
			return false;
		}
		if (target->id == UnitId::ProtossPlasmaShell
			|| target->id == UnitId::TerranSpiderMine
			|| target->id == UnitId::ProtossSteward
			|| target->id == UnitId::TerranLobotomyMine)
		{
			return false;
		}
		return true;
	}

	bool isTargetAttackingAlly(Unit* target, Unit* unit)
	{
		if (Unit* targetOfTarget = target->order_target.unit)
		{
			if (targetOfTarget->player < 8 && !unit->is_target_enemy(targetOfTarget))
			{
				return true;
			}
		}
		return false;
	}

	bool isUmsMode(u8 playerId)
	{
		assert(playerId < 8);
		return AIScriptController[playerId].AI_Flags.is_use_map_settings;
	}

	// Internal use only
	const u32 Func_GetRegionIdAtPosEx = 0x0049C9F0;
	u16 get_region_at_position_exact(s32 x, s32 y)
	{
		static u16 result;

		__asm {
			PUSHAD
			MOV EDI, x
			MOV ECX, y
			CALL Func_GetRegionIdAtPosEx
			MOV result, AX
			POPAD
		}

		return result;
	}

	//Based on code @ 0x00440BB0 (not a real independant function)
	bool unit_is_in_unsafe_region(Unit* unit)
	{
		u16 currentRegion = get_region_at_position_exact(unit->get_x(), unit->get_y());
		AiRegion* currentAiRegion = &AiRegionCaptains[unit->player][currentRegion];
		return currentAiRegion->state == 3
			|| currentAiRegion->airTarget
			|| currentAiRegion->groundTarget
			|| currentAiRegion->captainFlags & 0x20;
	}


	//-------- Get total unit stats in area --------//

	// 00440A60  unit_shields_clump_in_area_accumulator_proc
	// 00440AC0  unitEnergyClumpInAreaAccumulatorProc
	// 00440B20  getEnemyClumpValueProc
	// 00440CF0  AIEnemyOrganicUnitHPAccumulatorProc
	// 00440D60  AIAllyUnitHPAccumulatorProc
	// 00440DC0  AIEnemyUnitHPAccumulatorProc
	// 00440FF0  FindNearestEnemyAndClumpProc

	scbw::UnitFinder unitStatTotalFinder;

	// In original functions:
	// 0x00695604 would contains weaponId
	// 0x00695798 would contains totalEnemyLife
	s32 getTotalEnemyLifeInArea(s32 x, s32 y, s32 searchBounds, Unit* caster, u8 weaponId)
	{
		unitStatTotalFinder.search(x - searchBounds, y - searchBounds,
			x + searchBounds, y + searchBounds);
		s32 totalEnemyLife = 0;
		for (s32 i = 0; i < unitStatTotalFinder.get_unit_count(); ++i)
		{
			Unit* target = unitStatTotalFinder.get_unit(i);
			if ((target != caster) &&
				!(target->status & UnitStatus::Invincible) &&
				scbw::can_weapon_target_unit(weaponId, target, caster) &&
				caster->is_target_enemy(target))
			{
				// specific to AIEnemyUnitHPAccumulatorProc @ 0x00440DC0
				if (weaponId == WeaponId::Plague)
				{
					totalEnemyLife += target->get_current_hp_ingame();
				}
				else
				// specific to AIEnemyOrganicUnitHPAccumulatorProc @ 0x00440CF0 - used only by Maelstrom
				if (weaponId == WeaponId::Maelstrom)
				{
					if (units_dat::BaseProperty[target->id] & UnitProperty::Organic &&
						target->maelstrom_timer == 0)
					{
						totalEnemyLife += target->get_current_life_ingame();
					}
				}
				else
				{
					totalEnemyLife += target->get_current_life_ingame();
				}
			}
		}
		return totalEnemyLife;
	}

	// Loop content based on AIAllyUnitHPAccumulatorProc @ 0x00440D60
	// 0x00695798 would contains totalAllyLife
	s32 getTotalAllyLifeInArea(s32 x, s32 y, s32 searchBounds, Unit* caster, u8 weaponId)
	{
		unitStatTotalFinder.search(x - searchBounds, y - searchBounds,
			x + searchBounds, y + searchBounds);
		s32 totalAllyLife = 0;
		for (s32 i = 0; i < unitStatTotalFinder.get_unit_count(); ++i)
		{
			Unit* target = unitStatTotalFinder.get_unit(i);
			if (!(target->status & UnitStatus::Invincible) &&
				scbw::can_weapon_target_unit(weaponId, target, caster) &&
				!caster->is_target_enemy(target))
			{
				totalAllyLife += target->get_current_life_ingame();
			}
		}
		return totalAllyLife;
	}

	// Loop content based on unit_shields_clump_in_area_accumulator_proc @ 0x00440A60
	// 0x00695798 would contains totalEnemyShields
	s32 getTotalEnemyShieldsInArea(s32 x, s32 y, s32 searchBounds, Unit* caster)
	{
		unitStatTotalFinder.search(x - searchBounds, y - searchBounds,
			x + searchBounds, y + searchBounds);
		s32 totalEnemyShields = 0;
		for (s32 i = 0; i < unitStatTotalFinder.get_unit_count(); ++i)
		{
			Unit* target = unitStatTotalFinder.get_unit(i);
			if (!(target->status & UnitStatus::Invincible) &&
				caster->is_target_enemy(target) &&
				units_dat::ShieldsEnabled[target->id])
			{
				totalEnemyShields += target->getCurrentShieldsInGame();
			}
		}
		return totalEnemyShields;
	}

	// Loop content based on unitEnergyClumpInAreaAccumulatorProc @ 0x00440AC0
	// 0x00695798 would contains totalEnemyEnergy
	s32 getTotalEnemyEnergyInArea(s32 x, s32 y, s32 searchBounds, Unit* caster)
	{
		unitStatTotalFinder.search(x - searchBounds, y - searchBounds,
			x + searchBounds, y + searchBounds);
		s32 totalEnemyEnergy = 0;
		for (s32 i = 0; i < unitStatTotalFinder.get_unit_count(); ++i)
		{
			Unit* target = unitStatTotalFinder.get_unit(i);
			if (!(target->status & UnitStatus::Invincible) &&
				caster->is_target_enemy(target) &&
				target->is_valid_caster())
			{
				totalEnemyEnergy += (&target->energy)[1]; // unitPointer+0xA3 in original code
			}
		}
		return totalEnemyEnergy;
	}

	// Content based on	AI_NukeRequirementsProc @ 0x00441600 and
	//					getEnemyClumpValueProc	@ 0x00440B20
	// 0x00695798 would contains totalNukeTargetValue
	// Optimized to stop when reaching totalNukeTargetValue == 800 since that's what matters
	s32 getTotalEnemyNukeValueInArea(s32 x, s32 y, s32 searchBounds, Unit* caster)
	{
		unitStatTotalFinder.search(x - searchBounds, y - searchBounds,
			x + searchBounds, y + searchBounds);
		s32 totalNukeTargetValue = 0;
		for (s32 i = 0; i < unitStatTotalFinder.get_unit_count() && totalNukeTargetValue < 800; ++i)
		{
			Unit* target = unitStatTotalFinder.get_unit(i);
			if (!(target->status & UnitStatus::Invincible) &&
				caster->is_target_enemy(target))
			{
				if ((units_dat::BaseProperty[target->id] & UnitProperty::Worker) ||
					(!(target->status & UnitStatus::IsBuilding)))
				{
					totalNukeTargetValue += target->get_current_life_ingame();
				}
				if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
				{
					if (target->canDetect()
					||  target->id == UnitId::ZergSunkenColony
					||  target->id == UnitId::ZergLakizilisk)
					{
						totalNukeTargetValue = 800; // 0x0320
					}
				}
			}
		}
		return totalNukeTargetValue;
	}
}