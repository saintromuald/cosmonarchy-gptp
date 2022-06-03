#include "attack_extender.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "logger.h"

namespace hooks
{	
	// hooked for attack extender
	// 0x0043E670 => train_attack_force(@eax *mut AiRegion)->u32;
	u32 AI_TrainAttackForce(AiRegion* region) // 0043E670
	{
		u32 result;
		u32 child;
		AiRegion* parentAi;
		u16 childAi;
		u32 childTown;
		u32 v6;
		u32 parentTown;
		u32 v8;
		u32 v9[63];
		u32 v10;
		u32 v11;
		u32 v12;

		parentAi = region;
		child = (u8)region->playerId;
		result = AI_GetMissingAttackForceUnits(region, v9, AIScriptController[child].AI_Flags.isUseMapSettings);
		v10 = result;
		if (AIScriptController[child].AI_Flags.isUseMapSettings
			|| (childTown = AIScriptController[child].previous_building_hit_second) != 0
			&& (v12 = *elapsedTimeSeconds - childTown >= 30, *elapsedTimeSeconds - childTown < 30))
		{
			v11 = 60;
		}
		else
		{
			v11 = 160;
		}
		v6 = 0;
		v12 = 0;
		if (result)
		{
			do
			{
				parentTown = v9[v6];
				if (parentTown < UnitId::None && !helper::ai_add_to_region_military(parentAi, parentTown, v11))
				{
					v8 = (u8)parentAi->playerId;
					AIScriptController[v8].AI_Flags.flag_0x40 = 1; // Might be wrong
					helper::ai_add_cost_to_needs(parentTown, v8, 1);
					helper::ai_add_spending_request(parentTown, v8, 1, 60, parentAi);
					helper::AI_AddBaseUnitSpendingRequest(parentTown, v8, parentAi, 60);
					v6 = v12;
				}
				result = v10;
				v12 = ++v6;
			}
			while (v6 < v10);
		}
		return result;
	}

	//	0x0043EEC0 => grouping_for_attack(*mut AiRegion);
	void AI_GroupingForAttack(AiRegion* region) // 0043EEC0
	{
		AiRegion* regionVar;
		Unit* attackTarget;
		AiRegion* targetRegion;
		u32 missingUnits;
		u32 attackUnit;
		u16 regionId;
		AiRegion* targetRegion2;
		AiRegion* targetRegion3;
		u32 attackWave[63];
		s32 a3;
		s32 a4;
		s32 a2;

		helper::ai_recalculate_region_strength(region);
		helper::AI_TargetEnemyInRegion(region);
		if (region->captainFlags & 0x20 && helper::AI_MoveMilitaryToDefend(region))
		{
			return;
		}
		helper::AI_GetRegionMilitaryTypes(region, &a2, &a3, &a4);
		regionVar = region;
		if (a2 && (*SAI_Paths)->regions[region->region].accessibilityFlags == 0x1FFD || helper::AI_CanTravelToRegion(region, region))
		{
			helper::ai_set_region_type(0, region);
			attackTarget = helper::AI_PickAttackTarget_NoBuildings(region);
			if (attackTarget)
			{
				targetRegion = &AiRegionCaptains[(u8)region->playerId][helper::get_region_id(
					attackTarget->sprite->position.x,
					attackTarget->sprite->position.y)];
				helper::ai_set_region_type(9, targetRegion);
				helper::AI_MoveMilitaryToOtherRegion(targetRegion, region, 0, 0, 1);
			}
			else
			{
				helper::AI_IterateTownSomething(region);
			}
			return;
		}
		if (helper::AI_AttackTimerDecrement((u8)region->playerId))
		{
LABEL_15:
			regionId = region->targetRegion;
			if (regionId)
			{
				targetRegion2 = &AiRegionCaptains[(u8)region->playerId][regionId];
				helper::ai_set_region_type(1, targetRegion2);
				targetRegion3 = targetRegion2;
				regionVar = region;
				helper::AI_MoveMilitaryToOtherRegion(targetRegion3, region, 0, 0, 1);
			}
			helper::ai_set_region_type(0, regionVar);
			return;
		}
		missingUnits = AI_GetMissingAttackForceUnits(region, attackWave, 1);
		attackUnit = 0;
		if (!missingUnits)
		{
LABEL_14:
			if (helper::AI_HasForcesMovingToAttack((u8)region->playerId))
			{
				return;
			}
			goto LABEL_15;
		}
		while ((u32)attackWave[attackUnit] >= 0xE4)
		{
			if (++attackUnit >= missingUnits)
			{
				goto LABEL_14;
			}
		}
	}

	//	0x00438050 => get_missing_attack_force_units_hook(*mut AiRegion,*mut u32,bool)->u32;
	u32 AI_GetMissingAttackForceUnits(AiRegion* region, u32* unitIds, u32 isCampaign) // 00438050
	{
		s16 parentAi;
		u16* childAi;
		u32 i;
		u32 v6;
		Unit* parentTown;
		s8 v8;
		u32 v9;
		u32 v10; // was unitIDs, not sure what it should be here
		s32 v11;
		u32 v12;
		s32 v13;
		UnitAi* v14;
		u32 v15;
		u32 v16;
		Unit* v18;
		MilitaryAi* military;

		childAi = AIScriptController[(u8)region->playerId].attackers;
		parentAi = *childAi;
		for (i = 0; *childAi; ++i)
		{
			unitIds[i] = parentAi - 1;
			++childAi;
			parentAi = *childAi;
		}
		v6 = (u8)region->playerId;
		parentTown = firstPlayerUnit->unit[v6];
		v18 = firstPlayerUnit->unit[v6];
		if (parentTown)
		{
			while (1)
			{
				if (parentTown->sprite)
				{
					v8 = parentTown->main_order_id;
					if (v8)
					{
						v9 = (u16)parentTown->id;
						if (!(units_dat::BaseProperty[v9] & UnitProperty::Subunit))
							break;
					}
				}
LABEL_37:
				v18 = parentTown->player_link.next;
				if (!v18)
					return i;
				parentTown = parentTown->player_link.next;
			}
			v10 = (u16)parentTown->id;
			if (v9 == UnitId::TerranPhalanxSiegeMode)
				v10 = UnitId::TerranPhalanxTankMode;
			v11 = 1;
			v12 = parentTown->status & 1;
			if (!v12 || v8 == OrderId::CompletingArchonSummon || v8 == OrderId::ResetCollision1)
			{
				if (isCampaign)
				{
LABEL_36:
					parentTown = v18;
					goto LABEL_37;
				}
				if (v10 == UnitId::ZergEgg || v10 == UnitId::ZergCocoon)
				{
					v10 = (u16)v18->build_queue[(u8)v18->build_queue_slot];
					v13 = units_dat::BaseProperty[v10];
					if (v13 & 0x400)
						v11 = 2;
				}
			}
			if (!(units_dat::ComputerAIInternal[v10] & 1))
			{
				v14 = (UnitAi*)v18->unit_ai;
				military = (MilitaryAi*)v18->unit_ai;
				if (!v12
					|| v8 == 0x6A // UNI_FIRST_BLDG
					|| v8 == UnitId::Special_MatureChrysalis
					|| v14 != 0 && v14->type == 4 && military->region == region)
				{
					if (v11)
					{
						v15 = i - 1;
						do
						{
							v16 = 0;
							if (i)
							{
								while (v10 != unitIds[v16])
								{
									if (++v16 >= i)
									{
										goto LABEL_35;
									}
								}
								unitIds[v16] = UnitId::None;
								if (v16 == v15)
								{
									--i;
									--v15;
								}
							}
LABEL_35:
							--v11;
						}
						while (v11);
					}
				}
			}
			goto LABEL_36;
		}
		return i;
	}

	//	0x00447230 => ai_add_to_attackforce(@edi u32,@eax u32,@ecx u32);
	void AI_AddToAttack(u32 result, u32 player, u32 unit_id) // 00447230
	{
		u32 counter;
		u32 index;
		u32 plOffset;
		u16* atkGroups;
		u16* atkFinal;

		counter = result;
		if (player < 8 && result < 63 && unit_id < UnitId::None)
		{
			plOffset = 628 * player;
			index = 0;
			if (AIScriptController[player].attackers[0])
			{
				atkGroups = AIScriptController[player].attackers;
				while (index < 63)
				{
					++atkGroups;
					++index;
					if (!*atkGroups)
					{
						goto LABEL_8;
					}
				}
			}
			else
			{
LABEL_8:
				if (counter)
				{
					atkFinal = &AIScriptController[0].attackers[plOffset + index];
					do
					{
						--counter;
						if (index >= 63)
						{
							break;
						}
						*atkFinal = unit_id + 1;
						++index;
						++atkFinal;
					}
					while (counter);
				}
			}
		}
	}

	//	0x0043ABB0 => ai_attack_to_hook(u32,i32,i32,bool,bool)->u32;
	bool ai_attack_manager(u32 player, s32 x1, s32 y1, u32 alywaysOverwriteLastAttack, u32 allowFallbackAirOnly) // 0043ABB0
	{
		u16 attackWave;
		u16* attackWave2;
		char regionsList[2500];
		u32 waveBool;

		if (AIScriptController[player].supplyCollection)
		{
			return 1;
		}
		if (AIScriptController[player].AI_AttackGroup)
		{
			return *((u8*)&AiRegionCaptains[player][(u16)AIScriptController[player].AI_AttackGroup] - 47) != 8;
		}
		memset(regionsList, 0, sizeof(regionsList));
		helper::PopulateRegionsWithOwn(regionsList, player);
		helper::PopulateRegionsWithEnemy(player, regionsList);
		while (helper::AI_GetNeighborRegionTypes(regionsList))
		{
			helper::AI_FinishRegionAccessibility(regionsList);
		}
		attackWave = AIScriptController[player].attackers[0];
		attackWave2 = AIScriptController[player].attackers;
		waveBool = attackWave != 0;
		if (attackWave)
		{
			while (units_dat::DestroyScore[2 * attackWave + 226] & 4)
			{
				attackWave = attackWave2[1];
				++attackWave2;
				if (!attackWave)
				{
					goto LABEL_12;
				}
			}
			waveBool = 0;
		}
LABEL_12:
		AIScriptController[player].AI_Flags.isSecureFinished = 1;
		if (!helper::AI_SetAttackGroupingRegion(waveBool, 0, player, regionsList, x1, y1, alywaysOverwriteLastAttack)
			&& !helper::AI_SetAttackGroupingRegion(waveBool, 1, player, regionsList, x1, y1, alywaysOverwriteLastAttack))
		{
			if (!allowFallbackAirOnly || waveBool)
			{
				return 1;
			}
			if (!helper::AI_SetAttackGroupingRegion(1, 0, player, regionsList, x1, y1, alywaysOverwriteLastAttack)
				&& !helper::AI_SetAttackGroupingRegion(1, 1, player, regionsList, x1, y1, alywaysOverwriteLastAttack))
			{
				AIScriptController[player].supplyCollection = 1;
				return 1;
			}
		}
		return 0;
	}

	//	0x00437F20 => has_units_for_attack_force_hook(@edx u32)->u32;
	s32 AI_HasUnitsForAttackForce(u32 player) // 00437F20
	{
		u16* child;
		u16 parentAi;
		u32 i;
		Unit* childTown;
		u32 v6;
		u8 parentTown;
		u32 v8;
		s32 v9;
		u32 v10;
		u32 v11;
		u32 v13[63];

		child = AIScriptController[player].attackers;
		parentAi = *child;
		for (i = 0; *child; ++i)
		{
			v13[i] = parentAi - 1;
			++child;
			parentAi = *child;
		}
		childTown = firstPlayerUnit->unit[player];
		if (childTown)
		{
			v6 = i - 1;
			do
			{
				if (childTown->sprite)
				{
					parentTown = childTown->main_order_id;
					if (parentTown)
					{
						v8 = (u16)childTown->id;
						if (!(units_dat::BaseProperty[v8] & 0x10))
						{
							v9 = (u16)childTown->id;
							if (v8 == 30)
							{
								v9 = 5;
							}
							if (!(units_dat::ComputerAIInternal[v9] & 1))
							{
								if (childTown->status & 1)
								{
									if (parentTown != 106 && parentTown != 150 && !orders_dat::UseWeaponTargeting[parentTown])
									{
										if (childTown->unit_ai)
										{
											v10 = 0;
											if (i)
											{
												while (v9 != v13[v10])
												{
													if (++v10 >= i)
													{
														goto LABEL_22;
													}
												}
												v13[v10] = 228;
												if (v10 == v6)
												{
													--i;
													--v6;
												}
											}
										}
									}
								}
							}
						}
					}
				}
LABEL_22:
				childTown = childTown->player_link.next;
			}
			while (childTown);
		}
		v11 = 0;
		if (!i)
		{
			return 1;
		}
		while (v13[v11] == 228)
		{
			if (++v11 >= i)
			{
				return 1;
			}
		}
		return 0;
	}

	// I guess this was supposed to be hooked, but wasn't - Pr0nogo
	void AI_RemoveFromAttack(s32 unit_id, s32 player) // 00447040
	{
		if (unit_id == UnitId::TerranPhalanxSiegeMode)
		{
			unit_id = UnitId::TerranPhalanxTankMode;
		}
		u32 v2 = unit_id + 1;
		u32 v3 = 0;
		u16* v4 = AIScriptController[player].attackers;
		while (*v4 != v2)
		{
			++v3;
			++v4;
			if (v3 >= 63)
			{
				return;
			}
		}
		AIScriptController[0].attackers[v3 + 628 * player] = 229;
	}
}