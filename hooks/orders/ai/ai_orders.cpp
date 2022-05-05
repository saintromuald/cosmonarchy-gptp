#include "ai_orders.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "logger.h"

namespace hooks
{
	// Hooked to modify AI behavior such as cancelling structures, withdrawing hurt high-value targets, etc -- Pr0nogo
	// TODO: finish and cleanup
	void AI_UnitWasHit(Unit* self, Unit* attacker, s32 main_target_reactions, s32 dont_call_help) // 0043F320
	{
		AiRegion* v14;
		s32 v15;
		AiRegion* selfRegion;
		char v18;
		s16 v19;
		s16 child0;
		s32 child1;
		s32 child3;
		s32 child5;
		s32 child6;
		Unit* child7;
		__int64 parentAi3;
		Unit* parentAi4;
		Unit* parentAi6;
		Unit* parentAi7;
		s32 parentAi9;
		s32 childAi0;
		s32 childAi1;
		s32 childAi2;
		s32 childAi4;

		if (playerTable[self->playerId].type != PlayerType::Computer || self->playerId >= 8u)
		{
			return;
		}
		if (units_dat::BaseProperty[(u16)self->id] & UnitProperty::Building)
		{
			if (self->pAI)
			{
				AIScriptController[self->playerId].lastIndividualUpdateTime = *elapsedTimeSeconds;
				BuildingAi* selfBuildingAi = (BuildingAi*)self->pAI;
				if (selfBuildingAi)
				{
					selfBuildingAi->town->inBattle = 1;
				}
				if (main_target_reactions)
				{
					if (attacker)
					{
						if (units_dat::BaseProperty[(u16)self->id] & UnitProperty::Worker // cancel structure code
							&& !(self->mainOrderId) // Original function call checks if unit has a sprite??? 00475A50
							&& !(self->status & UnitStatus::Completed))
						{
							s32 quartTotalHP = self->getTotalHitPointsPlusShields() / 4;
							if (self->getCurrentLifeInGame() < quartTotalHP)
							{
								helper::AI_CancelStructure(self); // originally unused variable v8
								return;
							}
						}
					}
				}
			}
		}
		Unit* target = helper::AI_GetActualTarget(self, attacker);
		Unit* actual_target = target;
		if (target->status & UnitStatus::InBuilding)
		{
			actual_target = target->connectedUnit;
			if (!actual_target)
			{
				return;
			}
		}
		if (actual_target->status & UnitStatus::CloakingForFree)
		{
			if (!(actual_target->status & UnitStatus::CloakingForFree))
			{
				actual_target = helper::checkArbiterCloakingRange(self, UnitId::ProtossDidact);
				if (!actual_target)
				{
					return;
				}
			}
		}
		if (main_target_reactions)
		{
			helper::AI_OrderUnitCloaking(self);
			helper::AI_OrderLurkerBurrow(self);
		}
		UnitAi* selfUnitAi = (UnitAi*)self->pAI;
		parentAi9 = 0;
		childAi1 = 0;
		if (!selfUnitAi)
		{
			goto LABEL_37;
		}
		if (selfUnitAi->type == 4)
		{
			MilitaryAi* selfMilitaryAi = (MilitaryAi*)selfUnitAi;
			if (selfMilitaryAi->region->state == 1)
			{
				if (self->isUnitAINonBuilding())
				{
					helper::AI_AssignMilitary(2, selfMilitaryAi->region);
				}
			}
			childAi1 = helper::AI_IsUnderAttack(self, 0);
		}
		if (!main_target_reactions || !helper::unitIsActiveTransport(self) || !(self->status & UnitStatus::IsBuilding))
			goto LABEL_37;
		if (helper::isUnitLoaded(self))
		{
			if (self->mainOrderId != 111)
			{
				helper::issue_order_targeting_nothing(self, 0x6Fu);
			}
			return;
		}
		if (helper::AI_TryReturnHome(self, 0) || !helper::AI_TransportAction(self))
		{
			if (AIScriptController[(u8)self->playerId].AI_Flags.isUseMapSettings)
			{
				parentAi9 = 1;
			}
LABEL_37:
			v14 = &AiRegionCaptains[(u8)self->playerId][helper::SAI_GetRegionIdFromPxEx(self->sprite->position.x, self->sprite->position.y)];
			if (!parentAi9)
			{
				if (!dont_call_help)
				{
					helper::AI_AskForHelp(self, actual_target, childAi1);
				}
				if (!childAi1)
				{
					if (!self->pAI
						|| helper::AI_IsUnderAttack(self, 0)
						|| (childAi2 = 1, *(s8*)(v15 + 8) == 1)
						&& AIScriptController[(u8)self->playerId].AI_Flags.isUseMapSettings)
					{
						childAi2 = 0;
					}
					selfRegion = &AiRegionCaptains[(u8)self->playerId][helper::getRegionIdFromUnit(actual_target)];
					if (childAi2)
					{
						v18 = v14->state;
						if (v18 == 5 || v18 == 4)
						{
							helper::AI_AssignMilitary(6, v14);
						}
						if (v14->state)
						{
							if (!selfRegion->state && *elapsedTimeSeconds > 60)
								helper::AI_AssignMilitary(5, selfRegion);
						}
					}
					if (!dont_call_help)
					{
						selfRegion->enemyAirStrength = helper::getEnemyAirStrengthInRegion(selfRegion->region, (u8)self->playerId);
						selfRegion->enemyGroundStrength = helper::getEnemyStrengthInRegion(selfRegion->region, (u8)self->playerId, 0);
						v19 = selfRegion->neededAirStrength;
						if (v19 > v14->neededAirStrength)
						{
							v14->neededAirStrength = v19;
						}
						child0 = selfRegion->neededGroundStrength;
						if (child0 > v14->neededGroundStrength)
						{
							v14->neededGroundStrength = child0;
						}
					}
				}
			}
			v14->captainFlags |= 0x20u;
			if (self->status & UnitStatus::Completed)
			{
				child1 = (u8)self->mainOrderId;
				childAi0 = (u8)self->mainOrderId;
				if (child1 != OrderId::CompletingArchonSummon && child1 != OrderId::ResetCollision1 && child1 != OrderId::ConstructingBuilding)
				{
					if (main_target_reactions || !scbw::orderIsSpell(child1))
					{
						child3 = (u16)self->id;
						if (self->id || child1 != OrderId::EnterTransport)
						{
							childAi4 = helper::unitIsPerformingUnbreakableCode(self);
							if (childAi4 || !helper::AI_CastSpellBehavior(self, main_target_reactions))
							{
								if (child1 != OrderId::RechargeShields1 && child1 != OrderId::Move)
								{
									if (!main_target_reactions)
									{
										goto LABEL_126;
									}
									if (child3 != UnitId::ProtossSolarion
//										|| (u8)(LOBYTE(self->field_C8) + BYTE1(self->carrier.inHangarCount)) > 2u
										|| !helper::AI_TransportAction(self))
									{
										if (actual_target && actual_target->status & UnitStatus::IsBuilding // Attempt to return to base when below an HP threshold
											|| AIScriptController[(u8)self->playerId].AI_Flags.isUseMapSettings
											|| ((child5 = 3 * self->getTotalHitPointsPlusShields() / 4, self->getCurrentLifeInGame() > child5)
												|| child3 != UnitId::ProtossDidact && child3 != UnitId::TerranWraith && child3 != UnitId::ZergMutalisk
												|| !helper::AI_TransportAction(self))
											&& ((child6 = 2 * self->getTotalHitPointsPlusShields() / 3, self->getCurrentLifeInGame() > child6)
												|| child3 != UnitId::TerranMinotaur && child3 != UnitId::ProtossSolarion && child3 != UnitId::ProtossPanoptus
												|| !helper::AI_TransportAction(self)))
										{
											if (self->id != UnitId::ZergLakizilisk
												|| self->status & UnitStatus::Burrowed
												|| childAi0 == OrderId::Burrow
												|| childAi0 == OrderId::Unburrow)
											{
												child7 = actual_target;
											}
											else
											{
												child7 = actual_target;
												if (helper::AI_Flee(self, actual_target))
												{
													return;
												}
											}
											if (childAi4
												|| !(self->status & UnitStatus::IsBuilding)
												|| helper::unitHasGroundPath(self, child7->sprite->position.x, child7->sprite->position.y)
												|| !helper::AI_Flee(self, child7))
											{
LABEL_126:
												if (self->orderTarget.unit != actual_target)
												{ // Responding with worker
													if (!(units_dat::BaseProperty[(u16)self->id] & UnitProperty::Worker)
//														|| (child9 = actual_target->id, units_dat::BaseProperty[child9] & UnitProperty::Worker)
														|| !scbw::unitIsRangedAttacker(actual_target) // previously only checked for Zergling id
														|| helper::AI_HasNoDefenseBuild((u8)self->id))
													{
														if (main_target_reactions)
														{
															if (self->pAI)
															{
																if (helper::unitCanSeeCloakedTarget(actual_target, self))
																{
																	helper::AI_RespondToCloakedThreat(self, actual_target);
																}
															}
														}
														if (!childAi4)
														{
															if (helper::unitCanAttackTarget(actual_target, self, 1))
															{
																parentAi6 = self->subunit;
																parentAi7 = self->subunit;
																self->autoTargetUnit = actual_target;
																if (parentAi7->isSubunit())
																{
																	parentAi6->autoTargetUnit = actual_target;
																}
																helper::AI_UpdateAttackTarget(self, 0, 1, 0);
															}
															else if (self->pAI)
															{
																if (childAi0 != 159
//																	|| (parentAi3 = AI_PrepareToFlee(actual_target, self), (s32)parentAi3 == *(s32*)&self->sprite->position)
																	)
																{
																	if (main_target_reactions)
																	{
																		if (self->id != 34)
																		{
																			parentAi4 = self->orderTarget.unit;
																			if (!parentAi4 || !helper::unitCanAttackTarget(parentAi4, parentAi4, 1))
																			{
																				helper::AI_Flee(self, actual_target);
																			}
																		}
																	}
																}
																else
																{
//																	orderInterrupt(self, 6u, (Point32*)parentAi3, (Unit*)HIDWORD(parentAi3)); // point32 was dwPosition, I think that's the same thing
																}
															}
														}
													}
													else if (main_target_reactions)
													{
														helper::AI_Flee(self, actual_target);
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Hooked to help with AI recloaking units after the cloak rewrite - Pr0nogo
	void AI_OrderUnitCloaking(Unit* unit) // 0043B970
	{
		return;
		/*
		s32 unitStatus = unit->status;
		if (!(unitStatus & 0x100))
		{
			s8 techId;
			if (unit->id == UnitId::TerranEidolon)
			{
				techId = TechId::PersonnelCloaking;
			}
			else if (unit->id == UnitId::TerranWraith)
			{
				techId = TechId::CloakingField;
			}
			if (helper::techUseAllowed(unit, techId, unit->playerId) == 1)
			{
				helper::applyCloakingOrder(unit);
			}
		}
		*/
	}

	// Hooked to improve functionality - Pr0nogo
	void AI_SendSuicide(char randomSuicide, s32 player) // 0043E050
	{
		if (randomSuicide)
		{
			for (Unit* unit = firstPlayerUnit->unit[player]; unit; unit = unit->player_link.next)
			{
				if (unit->sprite)
				{
					if (unit->mainOrderId)
					{
						if (!unit->isSubunit()
							&& unit->id != UnitId::TerranNuclearMissile
							&& unit->id != UnitId::ZergLarva
							&& !(units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
							&& !(unit->sprite->flags & SpriteFlags::Hidden)
							&& !helper::isThreat(0, unit))
						{
							helper::prepSuicide(unit);
						}
					}
				}
			}
			return;
		}

		if (!AIScriptController[player].strategic_suicide)
		{
			AiRegion* region2 = 0;

			s32 regionIndex = 2500;
			for (Unit* unit = firstPlayerUnit->unit[player]; unit; unit = unit->player_link.next)
			{
				if (!unit->sprite)
				{
					continue;
				}
				if (!unit->mainOrderId)
				{
					continue;
				}
				GuardAi* guardAI = (GuardAi*)unit->pAI;
				if (!guardAI
				  || helper::unitIsActiveTransport(unit)
				  || unit->status & UnitStatus::InBuilding && !(AIScriptController[player].AI_Flags.isUseMapSettings))
				{
					continue;
				}
				s16 regionId = helper::SAI_GetRegionIdFromPxEx(unit->sprite->position.x, unit->sprite->position.y);
				AiRegion* region = &AiRegionCaptains[player][regionId];
				u8 aiType = guardAI->type;
				if (aiType == 1)
				{
					guardAI->parent->pAI = nullptr;
					helper::AI_AddMilitary(region, guardAI->parent, 1);
					guardAI->parent->pAI = nullptr;
				}
				else if (aiType != 4)
				{
					continue;
				}
				MilitaryAi* unitAI2 = (MilitaryAi*)unit->pAI;
				if (unitAI2)
				{
					if (unitAI2->type == 4)
					{
						MilitaryAi* militaryAI = unitAI2;
						u8 regionType = militaryAI->region->state;
						if (regionType == 1 || regionType == 8 || regionType == 9 || regionType == 2)
						{
							continue;
						}
					}
				}
				if (regionIndex != 2500)
				{
					helper::AI_RemoveUnitAI(unit, nullptr);
					helper::AI_AddMilitary(region2, unit, 1);
					continue;
				}
				memset(AIScriptController[player].attackers, 0, sizeof(AIScriptController[player].attackers));
				AIScriptController[player].AI_AttackGroup = 0;
				helper::AI_AttackClear(player);
				if (!helper::AI_AttackManager(player, unit->sprite->position.x, unit->sprite->position.y, 0, 1))
				{
					AIScriptController[player].strategic_suicide = 24;
					regionIndex = (u16)AIScriptController[player].AI_AttackGroup - 1;
					region2 = &AiRegionCaptains[player][regionIndex];
					helper::AI_RemoveUnitAI(unit, nullptr);
					helper::AI_AddMilitary(region2, unit, 1);
				}
			}
		}
	}

	// Hooked to investigate crashes - Pr0nogo
	u32 AI_RecalculateRegionStrength(AiRegion* region) // 0043A390
	{
		const u32 Func_CB_AICaptainStrengthIterator = 0x00438F60;
		AiTown* regionTown = region->town;
		region->regionAirStrength = 0;
		region->fullAirStrength = 0;
		region->regionGroundStrength = 0;
		region->fullGroundStrength = 0;
			
		for (AiTown* i = regionTown; i; i = i->link.prev)
		{
			Unit* townWorker = (Unit*)i->workers;
			if (!townWorker)
			{
				continue;
			}
			if (!(townWorker->status & UnitStatus::InBuilding))
			{
				s32 townUnitGroundStrength = townWorker->groundStrength;
				s16 townUnitAirStrength = townWorker->airStrength;
				region->fullAirStrength += townUnitAirStrength;
				region->fullGroundStrength += townUnitGroundStrength;
				Sprite* townUnitSprite = townWorker->sprite;
				if (!townUnitSprite)
				{
					continue;
				}
				s32 v6 = townUnitSprite->position.x;
				s32 regionTown6 = townUnitGroundStrength;
				if (helper::SAI_GetRegionIdFromPxEx(v6, townUnitSprite->position.y) == region->region)
				{
					region->regionAirStrength += townUnitAirStrength;
					region->regionGroundStrength += regionTown6;
				}
			}
		}

		if (AIScriptController[region->playerId].AI_Flags.isUseMapSettings)
		{
			s32 v8 = region->region << 6;
			s32 v9 = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterY + v8);
			s32 regionTown0 = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterX + v8) >> 8;
			v9 >>= 8;
			s16 a1 = regionTown0 - 256;
			s32 regionTown6 = (v9 + 256) << 16;
			s16 regionTown5 = v9 - 256;
			regionTown6 |= regionTown0 + 256;
			helper::iterateUnitsAtLocationTargetProc((Box16*)&a1, (Unit*(__fastcall*)(Unit*, Unit*))Func_CB_AICaptainStrengthIterator, (Unit*)region);
		}

		u32 regionAirStrength = region->enemyAirStrength;
		if (!(AIScriptController[region->playerId].AI_Flags.isUseMapSettings))
		{
			regionAirStrength += regionAirStrength >> 1;
		}
		if (regionAirStrength > region->neededAirStrength)
		{
			region->neededAirStrength = regionAirStrength;
		}

		u32 regionGroundStrength = region->enemyGroundStrength;
		if (!(AIScriptController[region->playerId].AI_Flags.isUseMapSettings))
		{
			regionGroundStrength += regionGroundStrength >> 1;
		}
		if (regionGroundStrength > region->neededGroundStrength)
		{
			region->neededGroundStrength = regionGroundStrength;
		}

		u16 result = AIScriptController[region->playerId].AI_MaxForce;
		if (AIScriptController[region->playerId].AI_MaxForce)
		{
			if (region->neededAirStrength > result)
			{
				region->neededAirStrength = result;
			}
			if (region->neededGroundStrength > result)
			{
				region->neededGroundStrength = result;
			}
		}
		return result;
	}	

	// Hooked to change functionality later - Pr0nogo
	Unit* getClosestEnemyStructure(s32 x, s32 y, s32 playerId) // 004A0F90
	{
		s16 regionId = helper::SAI_GetRegionIdFromPxEx(x, y);
		Unit* firstUnit = *firstVisibleUnit;
		s32 regionOffset = regionId << 6;
		s32 regionCenterX = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterX + regionOffset);
		s32 regionCenterY = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterY + regionOffset);

		Unit* result = nullptr;
		if (firstVisibleUnit)
		{
			// initialize vars used in loop
			Sprite* firstSprite;
			u32 fastDistance = 0;
			u32 maxDistance = 99999999;
			firstSprite = firstUnit->sprite;
			Unit* foundUnit = nullptr;
			u8 firstUnitPlayer;
			do
			{
				if (firstSprite)
				{
					if (firstUnit->mainOrderId)
					{
						if (firstUnit->status & UnitStatus::GroundedBuilding)
						{
							firstUnitPlayer = firstUnit->playerId;
							if (firstUnitPlayer == 11)
								firstUnitPlayer = firstSprite->playerId;
							if (!scbw::isAlliedTo(playerId, firstUnitPlayer))
							{
								fastDistance = helper::getDistanceFast(regionCenterX >> 8, firstUnit->sprite->position.x, regionCenterY >> 8, firstUnit->sprite->position.y);
								if (fastDistance < maxDistance)
								{
									maxDistance = fastDistance;
									foundUnit = firstUnit;
								}
							}
						}
					}
				}
				firstUnit = firstUnit->link.next;
			} while (firstUnit);
			result = foundUnit;
		}
		return result;
	}	

	s16 getRegionIdFromUnit(Unit* unit) // 0049CB40
	{
		if (unit == nullptr || unit->sprite == nullptr)
		{
			return 0;
		}
		return helper::SAI_GetRegionIdFromPxEx(unit->sprite->position.x, unit->sprite->position.y);
	}

	bool getBestSCVForRepairProc(Unit* repairman, Unit* repairTarget) // 004402B0
	{
		bool result = false;
		WorkerAi* repairmanAI = (WorkerAi*)repairman->pAI;
		if (repairman->playerId == repairTarget->playerId
			&& units_dat::BaseProperty[repairman->id] & UnitProperty::Worker
			&& !(repairman->status & UnitStatus::InTransport)
			&& !(repairman->status & UnitStatus::InBuilding) // new
			&& repairmanAI != nullptr
			&& repairmanAI->town != nullptr
			&& repairmanAI->town->building_SCV != nullptr) // new
		{
			result = repairmanAI->town->building_SCV == repairman;
		}
		return result;
	}

	// Hooked to add sprite nullcheck - Pr0nogo
	b32 AI_MilitaryShouldWaitForGroup(Unit* unit) // 0043AA10
	{
		// new start
		if (unit == nullptr)
		{
			return false;
		}
		if (unit->sprite == nullptr)
		{
			return false;
		}
		// new end

		MilitaryAi* unitAI = (MilitaryAi*)unit->pAI;
		if (unitAI == nullptr)
		{
			return false;
		}
		if (unitAI->type != AiType::Military)
		{
			return false;
		}
		if (unitAI->parent != unit)
		{
			return false;
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
		{
			return false;
		}
		if (unitAI->region->state == RegionState::Attacking)
		{
			return false;
		}

		AiTown* i = nullptr;
		AiTown* town = unitAI->region->town;
		for (i = town; town; i = town)
		{
			Unit* firstUnit = (Unit*)town->workers;
			if (firstUnit != nullptr && firstUnit->sprite != nullptr && firstUnit != unit)
			{
				u32 distance = helper::getDistanceFast(firstUnit->sprite->position.x, unit->sprite->position.x, firstUnit->sprite->position.y, unit->sprite->position.y);
				if (distance <= 0x180 && helper::AI_AreOnConnectedRegions(firstUnit, unit) && !(helper::AI_CheckMilitaryGrouping(firstUnit)) && distance > 0x100)
				{
					return 1;
				}
				town = i;
			}
			town = town->link.prev;
		}

		// Not sure if this is correct
		if (town)
		{
			return true;
		}
		return false;
	}

	// Hooked to add more functionality - Pr0nogo
	AiTown* deleteAITownIfNeeded(AiTown* town) // 004347E0
	{
		AiTown* result = helper::tryBeginDeleteTown(town);
		if (result)
		{
			helper::deleteAIScriptsUsingTown(town);
			for (Unit* i = *firstVisibleUnit; i; i = i->link.next)
			{
				if (i->sprite != nullptr)
				{
					if (i->mainOrderId)
					{
						if (units_dat::BaseProperty[i->id] & UnitProperty::ResourceContainer)
						{
							i->building.resource.resourceGroup = 0;
						}
					}
				}
			}

			u8 resourceArea = town->resarea;
			if (resourceArea)
			{
				baseArray[resourceArea].flags &= 0xFDu;
			}

			s32 player = (u8)town->player;
			AiTown* firstTown = AiActiveTownArray[player].first;
			AiExpandList* townList = &AiActiveTownArray[player];
			if (firstTown == town)
			{
				townList->first = town->link.prev;
			}

			AiTown* nextTown = town->link.next;
			if (nextTown)
			{
				nextTown->link.prev = town->link.prev;
			}

			if (town->link.prev)
			{
				town->link.prev->link.next = town->link.next;
			}

			result = townList->last;
			town->link.next = 0;
			town->link.prev = result[100].link.prev;
			AiTown* previousTown = result[100].link.prev;
			if (previousTown)
			{
				previousTown->link.next = town;
			}
			result[100].link.prev = town;
		}
		return result;
	}

	// Hooked because why not - Pr0nogo
	bool AI_CanMineExtra(Unit* unit) // 00447A70
	{
		if (!unit->pAI)
		{
			return 0;
		}
		s32 player = (u8)unit->playerId;
		if (AIScriptController[player].difficulty != 2)
		{
			return 0;
		}

		s32 unitId = UnitId::None;
		u8 raceId = playerTable[player].race;
		switch(raceId)
		{
			case RaceId::Terran:
			{
				unitId = UnitId::TerranMason;
			} break;
			case RaceId::Protoss:
			{
				unitId = UnitId::ProtossScribe;
			} break;
			case RaceId::Zerg:
			{
				unitId = UnitId::ZergDroleth;
			} break;
		}
		unitId = (u16)unitId;
		u32 completedCount = unitCountTable[(u16)unitId].player[(u8)player];

		// Originally had an inlined function that combined sieged and unsieged tanks for unit counts
		return completedCount >= 12;
	}

	// Hooked to fix crashes - Pr0nogo
	bool AI_IsBuildingSafe(Unit* unit) // 00438910
	{
		bool result = 0;
		if (unit != nullptr && unit->sprite != nullptr)
		{
			if (unit->mainOrderId)
			{
				s16 regionId = helper::SAI_GetRegionIdFromPxEx(unit->sprite->position.x, unit->sprite->position.y);
				AiRegion* playerRegions = AiRegionCaptains[unit->playerId];
				char regionState = playerRegions[regionId].state;
				AiRegion* region = &playerRegions[regionId];
				if (regionState != RegionState::EnemySighted && !region->airTarget && !region->groundTarget && !(region->captainFlags & 0x20))
				{
					result = 1;
				}
			}
		}
		return result;
	}

	// Hooked to fix crashes - Pr0nogo
	bool AI_AreOnConnectedRegions(Unit* unit1, Unit* unit2) // 00438E10
	{
		if (!unit1 || !unit2 || !unit1->sprite || !unit2->sprite)
		{
			return 0;
		}

		u16 regionId = helper::SAI_GetRegionIdFromPxEx(unit2->sprite->position.x, unit2->sprite->position.y);
		u16 regionId2 = helper::SAI_GetRegionIdFromPxEx(unit1->sprite->position.x, unit1->sprite->position.y);
		return helper::AI_RegionsAreConnected(unit2->playerId, regionId2, regionId);
	}

	// Hooked to change default behaviors - Pr0nogo
	s32 getBunkerPlacementPosition(u32 placementId, u32* regionId) // 004442E0
	{
		u32 region_max = *regionIdsCount;
		if (!regionIdsCount)
		{
			return 0;
		}

		s32 result;
		if (*regionIdsCount == 1)
		{
			*regionId = regionIds[0];
			result = 1;
		}
		else
		{
			u32 iterator1 = 0;
			Point16 regionPositions[24];
			u32 tileDistances[24];
			if (regionIdsCount)
			{
				memset(tileDistances, 99999999, *regionIdsCount);
				pathing* _paths = *SAI_Paths;
				do
				{
					u32 regionOffset = regionIds[iterator1] << 6;
					u32 region_x = *(u32*)((char*)&_paths->regions[0].regionCenterX + regionOffset) >> 8;
					u32 region_y = *(u32*)((char*)&_paths->regions[0].regionCenterY + regionOffset) >> 8;
					regionPositions[iterator1].x = region_x;
					regionPositions[iterator1++].y = region_y;
				} while (iterator1 < region_max);
			}

			Unit* i;
			for (i = *firstPlayerUnit[placementId].unit; i; i = i->player_link.next)
			{
				if (i->sprite)
				{
					if (i->mainOrderId)
					{
						if (i->id == UnitId::TerranBunker || i->id == UnitId::ZergCreepColony || i->id == UnitId::ZergSunkenColony)
						{
							u32 iterator2 = 0;
							if (region_max)
							{
								do
								{ // Distance from unit to region position
									u32 newTileDistance = helper::getTileDistance(i->sprite->position.x, i->sprite->position.y, regionPositions[iterator2].x, regionPositions[iterator2].y);
									if (newTileDistance < tileDistances[iterator2])
									{
										tileDistances[iterator2] = newTileDistance;
									}
									region_max = *regionIdsCount;
									++iterator2;
								} while (iterator2 < *regionIdsCount);
							}
						}
					}
				}
			}

			u32 bestTileDistance = tileDistances[0];
			u32 currentRegionId = 1;
			u32 bestRegionId;
			for (bestRegionId = 0; currentRegionId < region_max; ++currentRegionId)
			{
				if (tileDistances[currentRegionId] > bestTileDistance)
				{
					bestTileDistance = tileDistances[currentRegionId];
					bestRegionId = currentRegionId;
				}
			}
			*regionId = regionIds[bestRegionId];
			result = 1;
		}
		return result;
	}

	// Hooked to change default behaviors - Pr0nogo
	void AI_GetBuildLocation(u32 unitId, char* placementData, u32 playerId, Point16 desiredPos, s32 maxRange) // 004461B0
	{
		memset(placementData, 0, 0x1000u);
		helper::Ai_GetBuildingPlacementData(maxRange, unitId, placementData, playerId, desiredPos);
		helper::Ai_RemoveUnreachableRegions(placementData, desiredPos);
		if (unitId != UnitId::ProtossPylon)
		{
			helper::Ai_RemoveDangerousRegions(unitId, placementData, playerId, desiredPos);
		}
		helper::Ai_RemoveFarAwayTiles(placementData, desiredPos, maxRange);
		while (2)
		{
			u32 iterator1 = 0;
			do
			{
				if (placementData[iterator1])
				{
					helper::Ai_RemoveDifferentHeight(placementData, desiredPos); // Owned?
					helper::Ai_PlacementRelatedSomething(unitId, placementData, desiredPos);
					if (unitId == UnitId::TerranCommandCenter || unitId == UnitId::ZergHatchery || unitId == UnitId::ProtossNexus)
					{
						maxRange = 1;
					}
					else
					{
						maxRange = 0;
						helper::Ai_RemoveTilesIfUnfit(unitId, placementData, playerId, desiredPos);
					}
					helper::Ai_RemoveAddonPlaces(unitId, placementData, playerId, desiredPos);
					switch (unitId)
					{
						case UnitId::TerranWatchdog:
						case UnitId::ProtossPhotonCannon:
						{
							goto LABEL_17; 
						}
						case UnitId::TerranBunker:
						{
							if (!helper::getBunkerPlacementPosition(playerId, &maxRange))
							{
LABEL_17:
								helper::getTurretPlacementPosition(placementData, playerId, desiredPos, unitId, 0);
								return;
							}
						}
						case UnitId::ZergCreepColony:
						{
							if (!AIScriptController[playerId].AI_Flags.isSecureFinished) // SLOBYTE(AIScriptController[playerId].AI_Flags) < 0
							{
								goto LABEL_21;
							}

							if (!helper::getBunkerPlacementPosition(playerId, &maxRange))
							{
LABEL_21:
								helper::getTurretPlacementPosition(placementData, playerId, desiredPos, unitId, 0);
							}
						} break;
						case UnitId::ProtossPylon:
						{
							helper::getPylonPlacementPosition(placementData, playerId, desiredPos);
						} break;
						default:
						{
							if (!maxRange)
							{
								helper::getDefaultPlacementPosition(unitId, placementData, playerId, desiredPos);
							}
						} break;
					}
					return;
				}
				++iterator1;
			} while (iterator1 < 0x40);

			u32 iterator2 = 0;
			++iterator2;
			placementData += 64;
			if (iterator2 < 0x40)
			{
				continue;
			}
			break;
		}
	}

	// Hooked to change default behaviors - Pr0nogo
	u32 getTurretPlacementPosition(char* placementData, u32 playerId, Point16 desiredPos, u16 unitId, u32 unkFlag) // 00443AC0
	{
		// will do this later
		return 0;
	}

	// Hooked to nullcheck sprites - Pr0nogo
	bool AI_CheckMilitaryGrouping(Unit* unit) // 00439550
	{
		UnitAi* unitAi = (UnitAi*)unit->pAI;
		if (!unitAi || unitAi->type != 4 || units_dat::BaseProperty[unit->id] & 1)
		{
			return 1;
		}

		MilitaryAi* militaryAi = (MilitaryAi*)unit->pAI;
		if (militaryAi->parent != unit)
		{
			return 1;
		}

		AiRegion* region = militaryAi->region;
		Unit* slowestUnitVar = region->slowestUnit;
		if (!slowestUnitVar
		  || !slowestUnitVar->sprite // new
		  || slowestUnitVar->status & UnitStatus::InTransport
		  || slowestUnitVar == unit
		  || region->state == 2
		  || (u16)(&(*SAI_Paths)->regionCount + 32 * (helper::getRegionIdFromUnit(region->slowestUnit) + 4392)) < 60u
		  || !helper::AI_AreOnConnectedRegions(slowestUnitVar, unit)
		  || helper::getDistanceFast(region->slowestUnit->sprite->position.x, unit->sprite->position.x, region->slowestUnit->sprite->position.y, unit->sprite->position.y) <= 0x80)
		{
			return 1;
		}

		s32 x2;
		s32 y2;
		helper::getRegionCenter(region->region, &x2, &y2);
		return helper::getDistanceFast(unit->sprite->position.x, x2, unit->sprite->position.y, y2) <= 0x180;
	}

	// Hooked to fix AI height stupidity... or DUMBNESS??? -Pr0nogo
	u32 __stdcall AI_remove_different_height(char* placement_data, Point16 desired_pos) // 004436C0
	{
		return 0;

		/*
		u32 ground_height;
		char* placement_data_local;
		s32 pos_x_copy;
		u32 placement_data_iterator;
		u32 ground_height_2;
		bool iterate_done;
		u32 loop_var;
		char* placement_data_local_2;
		u32 result;
		char placement_data_buffer[4096];
		s32 pos_x;
		s32 times_to_iterate;
		s32 pos_y;

		memcpy(placement_data_buffer, placement_data, sizeof(placement_data_buffer));
		ground_height = helper::get_ground_height_at_px(desired_pos.y, desired_pos.x);
		pos_x = desired_pos.x - 1024;
		pos_y = desired_pos.y - 1024;
		placement_data_local = placement_data;
		times_to_iterate = 64;
		do
		{
			pos_x_copy = pos_x;
			placement_data_iterator = 0;
			do
			{
				if (placement_data_local[placement_data_iterator])
				{
					ground_height_2 = helper::get_ground_height_at_px(pos_y, pos_x_copy);
					if (ground_height != ground_height_2)
					{
						placement_data_local[placement_data_iterator] = 0;
					}
				}
				++placement_data_iterator;
				pos_x_copy += 32;
			} while (placement_data_iterator < 0x40);
			placement_data_local += 64;
			iterate_done = times_to_iterate == 1;
			pos_y += 32;
			--times_to_iterate;
		} while (!iterate_done);
		loop_var = 0;
		placement_data_local_2 = placement_data;
	LABEL_9:
		result = 0;
		while (!placement_data_local_2[result])
		{
			if (++result >= 0x40)
			{
				++loop_var;
				placement_data_local_2 += 64;
				if (loop_var < 0x40)
				{
					goto LABEL_9;
				}
				memcpy(placement_data, placement_data_buffer, 0x1000u);
				return result;
			}
		}
		return result;
	*/
	}

	// Hooked to investigate removing AI ally retaliation. (No cigar!) -Pr0nogo
	s32 attack_recent_attacker(Unit* unit) // 004770E0
	{
		Unit* target_unit = unit->autoTargetUnit;
		if (target_unit && !unit->pAI)
		{
			u8 target_player = 0;
			if (target_unit->playerId == 11) // Neutral
			{
				target_player = target_unit->sprite->playerId;
			}
			else
			{
				target_player = target_unit->playerId;
			}
			if (!scbw::isAlliedTo(unit->playerId, target_player))
			{
				if (helper::unit_can_attack_target(target_unit, unit, 1))
				{
					helper::order_to_attack_target(unit->autoTargetUnit, unit, 1, 0);
					unit->status |= UnitStatus::Disabled;
					return 1;
				}
			}
			unit->autoTargetUnit = 0;
		}
		return 0;
	}

	u32 AI_get_vespene_type(Unit* unit, u32 player_id, bool for_harvesting) { // 004326E0
		if (!unit || !unit->sprite || !unit->mainOrderId) {
			return 0;
		}
		if (!for_harvesting && unit->id == UnitId::ResourceVespeneGeyser) {
			return 1;
		}
		if (unit->id == UnitId::ResourceVespeneGeyser || unit->id == UnitId::TerranReservoir || unit->id == UnitId::ProtossAquifer || unit->id == UnitId::ZergExtractor) {
			return 2 + (unit->building.resource.resourceAmount != 0);
		}
		return 0;
	}

	Unit* get_ai_refinery(AiTown* town, u32 gas_state, u32 index, bool for_harvesting) { // 00432B90
		if (town->gas_buildings[index - 1] && AI_get_vespene_type(town->gas_buildings[index - 1], town->player, for_harvesting) == gas_state)
		{
			return town->gas_buildings[index - 1];
		}
		return 0;
	}

	// Hooked to help with AI implementation of the vespene overhaul. -Pr0nogo
	void AI_worker_controller(Unit* worker) // 00435210
	{
		WorkerAi* worker_ai = (WorkerAi*)worker->pAI;
		if (worker_ai)
		{
			if (worker_ai->wait_timer)
			{
				worker_ai->wait_timer--;
				return;
			}
			Point16 worker_pos = {};
			AiTown* town = worker_ai->town;
			worker_ai->last_update_second = *elapsedTimeSeconds;
			GroupFlag group_flags = units_dat::GroupFlags[worker->id];
			if ((group_flags.isZerg || group_flags.isProtoss || !group_flags.isTerran || !helper::AI_SCV_repair_controller(worker, town))
				&& !helper::AI_try_progress_spending_queue_worker(town, worker))
			{
				Unit* main_building = town->main_building;
				if (main_building)
				{
					Unit* town_gas = 0;
					Unit* town_gas_2 = 0;
					if (helper::unit_is_active_resource_depot(main_building))
					{
						if (worker_ai->target_resource == 1)
						{
							Unit* target_resource = worker->worker.targetResource.unit;
							if (target_resource && helper::unit_is_mineral_field(target_resource))
							{
								helper::AI_auto_harvest(target_resource, worker, 0);
							}
							else
							{
								Unit* town_mineral = town->mineral;
								if (!town_mineral || town_mineral->building.resource.resourceBelongsToAI)
								{
									worker_pos = worker->sprite->position;
									town_mineral = helper::harvest_next_nearest_resources_ex(town->worker_count <= 0xCu, worker, worker_pos);
									if (!town_mineral)
									{
										town_mineral = town->mineral;
									}
								}
								Unit* ai_refinery = get_ai_refinery(town, 3, 1, true);
								if (!ai_refinery)
								{
									ai_refinery = get_ai_refinery(town, 2, 1, true);
								}
								helper::AI_auto_harvest(town_mineral, worker, ai_refinery);
							}
							return;
						}
						if (worker_ai->target_resource == 2)
						{
							Unit* ai_refinery = get_ai_refinery(town, 3, 1, true);
							if (ai_refinery)
							{
								helper::AI_auto_harvest(ai_refinery, worker, town->mineral);
								return;
							}
							goto exit;
						}

						u32 gas_iterator = 0;
						u32 gas_type = 0;
						town_gas = town->gas_buildings[0];
						if (worker_ai->target_resource == 3)
						{
							if (town_gas)
							{
								if (AI_get_vespene_type(town_gas, town->player, true) == 3)
								{
									gas_iterator = 1;
								}
							}
							town_gas_2 = town->gas_buildings[1];
							gas_type = AI_get_vespene_type(town->gas_buildings[1], town->player, true);
							if (!town_gas_2 || gas_type != 3 || (++gas_iterator, gas_iterator != 2))
							{
								town_gas_2 = town->gas_buildings[2];
								gas_type = AI_get_vespene_type(town->gas_buildings[2], town->player, true);
								if (!town_gas_2 || gas_type != 3)
								{
									goto exit;
								}
								if (gas_iterator != 1)
								{
									goto exit;
								}
								if (town_gas_2)
								{
									helper::AI_auto_harvest(town_gas_2, worker, town->mineral);
									return;
								}
								goto exit;
							}
						}
						else
						{
							if (town_gas)
							{
								if (AI_get_vespene_type(town_gas, town->player) == 3)
								{
									gas_iterator = 1;
								}
							}
							town_gas_2 = town->gas_buildings[1];
							u32 type = AI_get_vespene_type(town->gas_buildings[1], town->player, true);
							if (town_gas_2 && type == 3) {
								++gas_iterator;
							}
							if (!town_gas_2 || type != 3 || gas_iterator != 2)
							{
								town_gas_2 = town->gas_buildings[2];
								type = AI_get_vespene_type(town->gas_buildings[2], town->player, true);
								if (!town_gas_2 || type != 3)
								{
									goto exit;
								}
								if (gas_iterator != 2)
								{
									goto exit;
								}
								if (town_gas_2)
								{
									helper::AI_auto_harvest(town_gas_2, worker, town->mineral);
									return;
								}
								goto exit;
							}
						}
						if (town_gas_2)
						{
							helper::AI_auto_harvest(town_gas_2, worker, town->mineral);
							return;
						}
					exit:
						Unit* ai_refinery = get_ai_refinery(town, 3, 1, true);
						if (!ai_refinery)
						{
							Unit* town_gas_final = town->gas_buildings[0];
							u32 gas_iterator_final = 0;
							if (town_gas_final)
							{
								if (AI_get_vespene_type(town_gas_final, town->player, true) == 3)
								{
									gas_iterator_final = 1;
								}
							}
							town_gas_final = town->gas_buildings[1];
							if ((!town_gas_final
								|| AI_get_vespene_type(town->gas_buildings[1], town->player, true) != 3
								|| (++gas_iterator_final, gas_iterator_final != 2))
								&& ((town_gas_final = town->gas_buildings[2]) == 0
									|| AI_get_vespene_type(town->gas_buildings[2], town->player, true) != 3
									|| gas_iterator_final != 1)
								|| (ai_refinery = town_gas_final) == 0)
							{
								town_gas_final = town->gas_buildings[0];
								gas_iterator_final = 0;
								if (town_gas_final)
								{
									if (AI_get_vespene_type(town_gas_final, town->player, true) == 3)
									{
										gas_iterator_final = 1;
									}
								}
								town_gas_final = town->gas_buildings[1];
								if ((!town_gas_final
									|| AI_get_vespene_type(town->gas_buildings[1], town->player, true) != 3
									|| (++gas_iterator_final, gas_iterator_final != 3))
									&& ((town_gas_final = town->gas_buildings[2]) == 0
										|| AI_get_vespene_type(town->gas_buildings[2], town->player, true) != 3
										|| gas_iterator_final != 2)
									|| (ai_refinery = town_gas_final) == 0)
								{
									ai_refinery = get_ai_refinery(town, 2, 1, true);
									if (!ai_refinery)
									{
										town_gas_final = town->gas_buildings[0];
										gas_iterator_final = 0;
										if (town_gas_final)
										{
											if (AI_get_vespene_type(town_gas_final, town->player, true) == 2)
											{
												gas_iterator_final = 1;
											}
										}
										town_gas_final = town->gas_buildings[1];
										if ((!town_gas_final
											|| AI_get_vespene_type(town->gas_buildings[1], town->player, true) != 2
											|| (++gas_iterator_final, gas_iterator_final != 2))
											&& ((town_gas_final = town->gas_buildings[2]) == 0
												|| AI_get_vespene_type(town->gas_buildings[2], town->player, true) != 2
												|| gas_iterator_final != 1)
											|| (ai_refinery = town_gas_final) == 0)
										{
											gas_iterator_final = 0;
											if (town_gas_final)
											{
												if (AI_get_vespene_type(town_gas_final, town->player, true) == 2)
												{
													gas_iterator_final = 1;
												}
											}
											Unit* town_gas_2_copy_4 = town->gas_buildings[1];
											if (town_gas_2_copy_4
												&& AI_get_vespene_type(town->gas_buildings[1], town->player, true) == 2
												&& (++gas_iterator_final, gas_iterator_final == 3)
												|| (town_gas_2_copy_4 = town->gas_buildings[2]) != 0
												&& AI_get_vespene_type(town->gas_buildings[2], town->player, true) == 2
												&& gas_iterator_final == 2)
											{
												ai_refinery = town_gas_2_copy_4;
											}
											else
											{
												ai_refinery = 0;
											}
										}
									}
								}
							}
						}
						helper::AI_auto_harvest(ai_refinery, worker, town->mineral);
					}
				}
			}
		}
	}

	// Hooked as part of the vespene overhaul. -Pr0nogo
	char AI_reassign_worker_resources(AiTown* town) // 00435900, unused int arg
	{
		u8 target_resource_type = 0;
		if (town->inited)
		{
			if (!town->resarea)
			{
				town->resarea = helper::get_closest_base_from_startloc(town->player, town->position.x, town->position.y);
				town->resource_units_not_set = 1;
			}

			u32 resarea = town->resarea;
			if (town->resource_units_not_set)
			{
				town->resource_units_not_set = 0;
				helper::get_resource_area_gas(resarea, &town->gas_buildings[0], &town->gas_buildings[1], &town->gas_buildings[2]);
				if (town->gas_buildings[0])
				{
					helper::AI_MakeGuard(town->player, town->gas_buildings[0]);
				}
				if (town->gas_buildings[1])
				{
					helper::AI_MakeGuard(town->player, town->gas_buildings[1]);
				}
				if (town->gas_buildings[2])
				{
					helper::AI_MakeGuard(town->player, town->gas_buildings[2]);
				}
			}
			if (!town->mineral)
			{
				Unit* resarea_minerals = helper::get_resource_area_mineral(town->resarea);
				town->mineral = resarea_minerals;
				if (resarea_minerals)
				{
					helper::AI_MakeGuard(town->player, resarea_minerals);
				}
			}

			if (resarea)
			{
				ResourceArea* ai_resarea = &baseArray[resarea];
				u32 new_worker_limit = 0;
				if (resarea && resarea < 250u && ai_resarea != 0)
				{
					new_worker_limit = 3 * ai_resarea->geyser_count + helper::get_optimal_mineral_workers(resarea, town->player);
				}
				town->worker_limit = new_worker_limit;
			}
			u8 mineral_bool = 1;
			u8 gas_bool = 2;
			if (get_ai_refinery(town, 3, 1, true) || get_ai_refinery(town, 2, 1, true))
			{
				if (!town->mineral)
				{
					mineral_bool = 2;
				}
			}
			else
			{
				gas_bool = 1;
			}

			u32 town_player = town->player;
			u8 minerals_percentage = 0;
			u8 gas_percentage = 0;
			helper::AI_resource_need_split(&gas_percentage, town_player, &minerals_percentage);
			u32 gas_count_1 = gas_percentage * town->worker_count / 100;
			u32 gas_count_2 = gas_percentage * town->worker_count / 100;

			u8 race = playerTable[town_player].race;
			u16 worker_id = UnitId::None;
			if (race == RaceId::Terran)
			{
				worker_id = UnitId::TerranMason;
			}
			else
			{
				worker_id = race != RaceId::Zerg ? UnitId::ProtossScribe : UnitId::ZergDroleth;
			}
			u32 worker_count = unitCountTable[worker_id].player[town_player];
			if (gas_count_1 <= 4 * baseArray[resarea].geyser_count)
			{
				if (gameGas[town_player] >= 800)
				{
					goto exit;
				}
				if (worker_count < 20)
				{
					if (worker_count < 15)
					{
						if (worker_count < 10)
						{
							if (worker_count < 5 || gas_count_1 >= 1)
							{
								goto exit;
							}
							gas_count_2 = 1;
						}
						else
						{
							if (gas_count_1 >= 2)
							{
								goto exit;
							}
							gas_count_2 = 2;
						}
					}
					else
					{
						if (gas_count_1 >= 3)
						{
							goto exit;
						}
						gas_count_2 = 3;
					}
				}
				else
				{
					if (gas_count_1 >= 4)
					{
						goto exit;
					}
					gas_count_2 = 4;
				}
			}
			else
			{
				gas_count_2 = 4 * baseArray[resarea].geyser_count;
			}
			gas_count_1 = gas_count_2;
		exit:
			u32 i = 0;
			for (WorkerAi* town_worker_ai = town->workers; town_worker_ai; town_worker_ai = town_worker_ai->link.prev)
			{
				Unit* town_unit = town_worker_ai->parent;
				u8 unit_order = town_unit->mainOrderId;
				if (unit_order == OrderId::ConstructingBuilding
					|| unit_order == OrderId::Pickup4
					|| unit_order == OrderId::DroneBuild
					|| town_unit->status & UnitStatus::InTransport)
				{
					town_worker_ai->last_update_second = *elapsedTimeSeconds;
				}
				else if (*elapsedTimeSeconds - town_worker_ai->last_update_second >= 60)
				{
					helper::issue_order_targeting_nothing(town_unit, OrderId::ComputerAI);
					town_worker_ai->wait_timer = 10;
					town_worker_ai->last_update_second = *elapsedTimeSeconds;
					town_worker_ai->reassign_count += 1;
					helper::AI_get_old_peons(town_worker_ai->town, 1u);
					gas_count_1 = gas_count_2;
				}
				u8 mineral_bool_2 = mineral_bool;
				u8 gas_bool_2 = gas_bool;

				target_resource_type = mineral_bool;
				if (gas_count_1)
				{
					--gas_count_1;
					target_resource_type = gas_bool;
					gas_count_2 = gas_count_1;
				}

				town_worker_ai->target_resource = target_resource_type;
				if (target_resource_type != 1 && ++i >= 4)
				{
					i = 0;
					if (gas_bool == target_resource_type)
					{
						gas_bool_2 = gas_bool + 1;
						gas_bool = (gas_bool + 1);
					}
					if (mineral_bool == target_resource_type)
					{
						mineral_bool_2 = mineral_bool + 1;
						mineral_bool = (mineral_bool + 1);
					}
					if (gas_bool_2 > 4)
					{
						gas_bool = 2;
					}
					if (mineral_bool_2 > 4)
					{
						mineral_bool = 2;
					}
				}
			}
		}
		return target_resource_type;
	}

	// Hooked as part of the vespene overhaul. -Pr0nogo
	void AI_auto_harvest(Unit* resource, Unit* worker, Unit* fallback) // 00433B90
	{
		if (worker->resourceType & ResourceType::Minerals)
		{
			worker->userActionFlags |= 1u;
			if (worker->mainOrderId)
			{
				while (1)
				{
					if (!worker->orderQueueTail)
					{
						break;
					}
					if (!orders_dat::CanBeInterrupted[worker->orderQueueTail->orderId]
					&&	worker->orderQueueTail->orderId != OrderId::ReturnMinerals)
					{
						break;
					}
					helper::remove_order_from_unit_queue(worker->orderQueueTail, worker);
				}
				helper::perform_another_order(OrderId::ReturnMinerals, 0, worker, 228, 0, 0);
			}
			helper::orders_nothing_2(worker);
		}
		else if (worker->resourceType & ResourceType::VespeneGas)
		{
			helper::issue_order_targeting_nothing(worker, OrderId::ReturnGas);
		}
		else
		{
			if (!resource || !(resource->status & UnitStatus::Completed))
			{
				resource = fallback;
			}
			if (resource && (resource->id == UnitId::ResourceVespeneGeyser || resource->status & UnitStatus::Completed))
			{
				helper::assign_order_with_target(resource, worker, OrderId::Harvest1);
				resource->building.resource.resourceBelongsToAI = 1;
			}
		}
	}
}