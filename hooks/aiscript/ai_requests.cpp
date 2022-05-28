#include "ai_requests.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "logger.h"

namespace hooks
{
	// Don't know what this was hooked for; tidied May 2021 - Pr0nogo
	void AI_BuildTechUpgrade(AiTown* town, char method, char amount, s16 id, char priority, bool need) // 00431F90
	{
		s32 iterate = 0;
		constexpr s32 BuildCountFlag = 0xF8;
		TownReq* town_units;
		if (playerTable[(u8)town->player].type == PlayerType::Computer)
		{
			town_units = town->town_units;
			bool setNeed = false;
			if (town->town_units[0].flags_and_count & BuildCountFlag) // if build
			{
				for (s32 i = 0; i < 100; i++)
				{
					++town_units;
					if (!(town_units->flags_and_count & BuildCountFlag)) // if not build
					{
						setNeed = true;
						iterate = i;
						break;
					}
				}
			}
			else
			{
				setNeed = true;
			}
			if (setNeed)
			{
				town->town_units[iterate].flags_and_count = need | 0x2 * (method & 0x3 | 0x4 * amount);
				town->town_units[iterate].id = id;
				town->town_units[iterate].priority = priority;
				// build orders have 0xf8 flag, tech has 0x4, upgrade has 0x2, if needed (player_need) has 0x1
			}
		}
	}

	// Hooked to investigate crashes - Pr0nogo
	AiTown* AI_ProgressBuild(s32 playerId, s32* minerals, s32* gas) // 00434480
	{
		AiTown* result = AiActiveTownArray[playerId].first;
		AiTown* firstTown = AiActiveTownArray[playerId].first;
		if (result)
		{
			do
			{
				WorkerAi* firstWorker = firstTown->workers;
				if (firstTown->workers)
				{
					do
					{
						Unit* worker = firstWorker->parent; // This was null during a crash
						if (!worker)
						{
							scbw::print("null unit in AI_progressBuild");
							return result;
						}
						s32 buildOrderId = (u8)worker->mainOrderId;
						if (buildOrderId == OrderId::BuildTerran
						  || buildOrderId == OrderId::BuildProtoss1
						  || buildOrderId == OrderId::DroneLand
						  || buildOrderId == OrderId::DroneBuild
						  || buildOrderId == OrderId::DroneStartBuild)
						{
							u8* consoleVar = nullptr;
							s32 playerIdVar = (u8)playerId;
							u16 targetUnitQueue = worker->buildQueue[(u8)worker->buildQueueSlot % 5];
							s32 targetUnitQueue2 = targetUnitQueue;
							s32 targetQueueCostMinerals = units_dat::MineralCost[targetUnitQueue2];
							s32 targetQueueCostGas = units_dat::GasCost[targetUnitQueue2];
							AIMineralsReserve[playerIdVar] = targetQueueCostMinerals;
							AIGasReserve[playerIdVar] = targetQueueCostGas;
							if (helper::has_supplies_for_unit(targetUnitQueue, playerId, 1))
							{
								if (gameMinerals[(u8)playerId] >= (s32)AIMineralsReserve[(u8)playerId])
								{
									if (gameGas[(u8)playerId] >= (s32)AIGasReserve[(u8)playerId])
									{
										*minerals -= AIMineralsReserve[playerId];
										*gas -= AIGasReserve[playerId];
									}
									else
									{
										consoleVar = consoleIndex;
										helper::displayErrorMessageIDWithSFX((u8)playerId, 851, *consoleVar + 150);
									}
								}
								else
								{
									consoleVar = consoleIndex;
									helper::displayErrorMessageIDWithSFX((u8)playerId, 850, *consoleVar + 147);
								}
							}
						}
						firstWorker = firstWorker->link.prev;
					}
					while (firstWorker);
				}
				result = firstTown->link.prev;
				firstTown = firstTown->link.prev;
			}
			while (firstTown);
		}
		return result;
	}

	// We all know what this is for - Pr0nogo
	// TODO: Finish and clean up
	s32 AIManageBuildingPlacement(Unit* builder, u32 unitId, Point16 tryPosition, Point16* outPosition, s32 areaTiles) // 004465C0 - 3 nops
	{
		s32 playerVar;
		Unit* builderVar;
		char v7;
		s16 v8;
		s32 v9;
		s32 v10;
		s16 v12;
		s32 v13;
		Unit* disabledUnitVar;
		Sprite* disabledUnitSprite;
		s16 _x;
		s16 _y;
		s32 v18;
		s64 v19;
		u32 bestDistance;
		s32 posy;
		s32 v22;
		s32 posx;
		u32 currentDistance;
		bool v25;
		s16 v26;
		bool v27;
		s32 v28;
		s8 a1[4096];
		s32 v30;
		s32 v31;
		s32 v32;
		POINT _tileX;
		u32 bestFallbackDistance;
		s16* v35;
		s32* v36;
		s32 ownrId;
		s8* v38;
		Point16 __tileX;
		s16 v40;
		s32 v41;
		Point16 unitTypea;
		Point16 desiredFallbackPosition;

		_tileX.x = tryPosition.x / 32;
		__tileX.x = tryPosition.x / 32;
		builderVar = builder;
		playerVar = (u8)builder->playerId;
		_tileX.y = (*(s32*)&tryPosition >> 16) / 32;
		__tileX.y = _tileX.y;
		v7 = units_dat::BaseProperty[(u16)unitId];
		ownrId = playerVar;
		v40 = 0;
		v41 = 0;
		if (v7 & 2)
		{
			v9 = 4 * (u16)builderVar->id;
			v10 = *(s16*)((char*)&units_dat::BuildingDimensions[0].x + v9);
			*(Point16*)&v40 = __tileX;
			v9 = *(s16*)((char*)&units_dat::BuildingDimensions[0].y + v9) / -2 / 32 + __tileX.y;
			v36 = &units_dat::MaxHitPoints[(u16)unitId + 122];
			v8 = *(s16*)v36 / 32 + __tileX.x - v10 / 2 / 32;
			v35 = (s16*)((char*)&units_dat::MaxHitPoints[(u16)unitId + 122] + 2);
			v41 = *v35 / 32 + v9;
			v40 = v8;
			if (!helper::canBuildOnTileLocation(builderVar, ownrId, *(s32*)&v40, v41, unitId, 0, 0, 1, 0))
			{
				v12 = *(s16*)v36 / -32 + v8;
				v41 = *v35 / -32 + v41;
				v13 = units_dat::BuildingDimensions[(u16)builderVar->id].x;
				v40 = v12;
				outPosition->x = 32 * v12 + v13 / 2;
				outPosition->y = 32 * v41 + units_dat::BuildingDimensions[(u16)builderVar->id].y / 2;
				return 1;
			}
			playerVar = ownrId;
		}
		ownrId = 2500;
		v38 = (s8*)1;
		switch ((u16)unitId)
		{
			// Town halls
			case UnitId::TerranCommandCenter:
			case UnitId::ZergHatchery:
			case UnitId::ProtossNexus:
			{
				goto LABEL_10;
			}
			// Addons
			case UnitId::TerranComsatStation:
			case UnitId::TerranNuclearSilo:
			case UnitId::TerranQuarry:
			{
				unitId = UnitId::TerranCommandCenter;
			} break;
			case UnitId::TerranControlTower:
			case UnitId::TerranMunitionsBay:
			{
				unitId = UnitId::TerranStarport;
			} break;
			case UnitId::TerranMedbay:
			case UnitId::TerranCovertOps:
			{
				unitId = UnitId::TerranBarracks;
			} break;
			case UnitId::TerranMachineShop:
			case UnitId::TerranSalvoYard:
			{
				unitId = UnitId::TerranFulcrum;
			} break;
			case UnitId::TerranGuildhall:
			{
				unitId = UnitId::TerranCaptaincy;
			} break;
			case UnitId::TerranBioticBase:
			case UnitId::TerranTinkerersTower:
			{
				unitId = UnitId::TerranScrapyard;
			} break;
			// Misc special rules
			case UnitId::TerranBunker:
			{
				goto LABEL_9;
			}
			case UnitId::ZergCreepColony:
			{
//				if (AIScriptController[playerVar].AI_Flags >= 0) // used SLOBYTE, probably checking "creep" asc3 command
				{
LABEL_9:
					if (helper::get_bunker_placement_position(playerVar, &ownrId))
					{
LABEL_10:
						v38 = 0;
					}
				}
			} break;
			case UnitId::ProtossPylon:
			{
				disabledUnitVar = helper::getDisabledUnitInRange(playerVar, tryPosition);
				disabledUnitArray[9] = disabledUnitVar;
				if (disabledUnitVar)
				{
					disabledUnitSprite = disabledUnitVar->sprite;
					_x = disabledUnitSprite->position.x;
					_y = disabledUnitSprite->position.y;
					_tileX.x = _x / 32;
					__tileX.x = _x / 32;
					tryPosition.x = _x;
					tryPosition.y = _y;
					_tileX.y = _y / 32;
					__tileX.y = _y / 32;
					v38 = 0;
				}				
			} break;
			default:
			{
			} break;
		}
		v18 = (u8)builderVar->playerId;
		*AIBuilderUnknown = builderVar;
		v32 = (u16)unitId;
		helper::AIGetBuildLocation((u16)unitId, (char*)a1, v18, tryPosition, areaTiles);
		*AIBuilderUnknown = 0;
		if (v38)
		{
			helper::AIPlacementDataSomething(a1, tryPosition);
		}
		unitTypea = __tileX;
		if (ownrId != 2500)
		{
			v19 = (s16)((*SAI_Paths)->regions[(u16)ownrId].regionCenterX >> 8);
//			LOWORD(unitTypea) = (((v19) & 0x1F) + (s32)v19) >> 5;
//			HIWORD(unitTypea) = (s16)((*SAI_Paths)->regions[(u16)ownrId].regionCenterY >> 8) / 32;
		}
		v30 = __tileX.y - 32;
		bestDistance = 99999999;
		bestFallbackDistance = 99999999;
		desiredFallbackPosition.x = 0;
		desiredFallbackPosition.y = 0;
		v36 = 0;
		v31 = __tileX.x - 32;
		while (1)
		{
			posy = v30;
			v38 = a1;
			v35 = (s16*)64;
			do
			{
				posx = v31;
				v22 = 0;
				__tileX.x = 0;
				__tileX.y = 0;
				do
				{
					if (v38[v22])
					{
//						currentDistance = helper::getTileDistance(32 * (s16)unitTypea, 32 * SHIWORD(unitTypea), 32 * posx, 32 * posy);
						if (currentDistance < bestDistance)
						{
							if (currentDistance <= areaTiles)
							{
								bestDistance = currentDistance;
								v40 = posx;
								v41 = posy;
							}
							else if (currentDistance < bestFallbackDistance)
							{
								bestFallbackDistance = currentDistance;
								desiredFallbackPosition.x = posx;
								desiredFallbackPosition.y = posy;
							}
						}
					}
					v22 = *(s32*)&__tileX + 1;
					++posx;
					++* (s32*)&__tileX;
				} while (*(u32*)&__tileX < 0x40);
				++posy;
				v25 = v35 == (s16*)1;
				v38 += 64;
				v35 = (s16*)((char*)v35 - 1);
			} while (!v25);

			if (bestDistance != 99999999)
			{
				break;
			}
			bestDistance = bestFallbackDistance;
			v26 = desiredFallbackPosition.x;
			*(Point16*)&v40 = desiredFallbackPosition;
			if (bestFallbackDistance != 99999999)
			{
				goto LABEL_36;
			}
			if (ownrId != 2500)
			{
				v27 = (u32)v36 + 1 < 2;
//				LOWORD(unitTypea) = _tileX.x;
//				HIWORD(unitTypea) = _tileX.y;
				v36 = (s32*)((char*)v36 + 1);
				if (v27)
				{
					continue;
				}
			}
			return 0;
		}
		v26 = v40;
LABEL_36:
		v28 = v32;
		outPosition->x = 32 * v26 + units_dat::BuildingDimensions[v32].x / 2;
		outPosition->y = 32 * v41 + units_dat::BuildingDimensions[v28].y / 2;
		return 1;
	}

	enum RequestType
	{
		RequestType_UnitMake = 0,
		RequestType_UnitMergeMorph1 = 1,
		RequestType_UnitMergeMorph2 = 2,
		RequestType_ConstructMorph = 3,
		RequestType_Train = 4,
		RequestType_Upgrade = 5,
		RequestType_Research = 6,
		RequestType_PlaceAddon = 7,
		RequestType_MakeDetector = 8,
	};

	char AI_PerformTask(Unit* unit) // 004361A0 -- hooked to fix AI Archon training -- Pr0nogo
	{
		switch (unit->id)
		{
			case UnitId::ZergOvileth:
			{
				helper::AI_OverlordBehavior(unit);
			} break;
			case UnitId::TerranComsatStation:
			{
				helper::AI_CastSpellBehaviour(unit, 0);
			} break;
			case UnitId::TerranNuclearSilo:
			{
				helper::AI_NukeSiloBehavior(unit);
			} break;
		}

		UnitAi* unitAIVar = (UnitAi*)unit->unit_ai;
		if (!unitAIVar || unitAIVar->type != AiType::Building)
		{
			return 1;
		}

		s32 player = unit->playerId;
		s32 someBool = 0;
		if (gameMinerals[player] > 1500 && gameGas[player] > 1500)
		{
			if (!helper::unitIsTrainingOrMorphing(unit))
			{
				if (helper::AI_UnitConstructionBehavior(player, unit, 160))
				{
					someBool = 1;
				}
			}
		}

		AI_Main* AI_Mainframe = &AIScriptController[player];
		s32 request_type;
		if (AI_Mainframe->request_count)
		{
			request_type = AI_Mainframe->requests->type;
			if (AI_Mainframe->requests->id == UnitId::ProtossArchon || AI_Mainframe->requests->id == UnitId::ProtossMindTyrant)
			{
				// Override archon and mind tyrant type
				request_type = RequestType_Train;
			}
		}
		else
		{
			request_type = 0;
		}

		switch (request_type)
		{
			case RequestType_UnitMake:
			{
				if (!helper::AI_UnitConstructionBehavior(player, unit, 50))
				{
					return 1;
				}
				someBool = 1;
				return helper::AI_PlanDoMergeMorph(player, unit, someBool);
			}
			case RequestType_UnitMergeMorph1:
			case RequestType_UnitMergeMorph2:
			{
				return helper::AI_PlanDoMergeMorph(player, unit, someBool);
			}
			case RequestType_ConstructMorph:
			{
				return helper::AI_MorphBehavior(player, unit, unit->unit_ai);
			}
			case RequestType_Train:
			{
				helper::AI_TrainBehavior(unit, unit->unit_ai, player);
				return 1;
			} break;
			case RequestType_Upgrade:
			{
				helper::AI_UpgradeBehavior(unit, player);
				return 1;
			} break;
			case RequestType_Research:
			{
				helper::AI_ResearchBehavior(unit, player);
				return 1;
			} break;
			case RequestType_PlaceAddon:
			{
				helper::AI_ConstructBehavior(unit, unit->unit_ai, player);
				return 1;
			} break;
			case RequestType_MakeDetector:
			{
				return helper::AI_MakeDetectorBehavior(unit, player);
			} break;
		}
		return 1;
	}

	char AI_PlanDoMergeMorph(s32 player, Unit* unit, s32 boolSomething) // 00435F10
	{
		bool error;
		AiTown* mainTown2;
		char requestedBuildId;
		AI_Main* mainTown;
		s16 mainTownTown;
		s16 unitAIQueues;
		s16 unitId;

		Unit* unitCopy = unit;
		s32 nextBuildType = AIScriptController[player].requests->type;
		Unit* dummy = nullptr;
		switch (nextBuildType)
		{
			case UnitId::ZergGeszithalor:
			case UnitId::ZergVithrilisk:
				unitCopy = helper::AI_GetMutaliskForMorph(player);
				if (!unitCopy)
				{
					helper::AI_Reserve(player, 0);
					helper::AI_PopSpendingRequest(player);
					return 0;
				}
				break;
			case UnitId::ProtossArchon:
				if (helper::AI_GetHighTemplarsForSummon(player, &unit, &dummy))
				{
					helper::assign_order_with_target(dummy, unit, OrderId::WarpingArchon);
					helper::assign_order_with_target(unit, dummy, OrderId::WarpingArchon);
					helper::AI_Reserve(player, 1);
					helper::AI_PopSpendingRequest(player);
					return 0;
				}
			case UnitId::ProtossMindTyrant:
				if (helper::AI_GetDarkTemplarsForSummon(player, &dummy, &unit))
				{
					helper::assign_order_with_target(dummy, unit, OrderId::WarpingDarkArchon);
					helper::assign_order_with_target(unit, dummy, OrderId::WarpingDarkArchon);
					helper::AI_Reserve(player, 1);
					helper::AI_PopSpendingRequest(player);
					return 0;
				}
				helper::AI_Reserve(player, 0);
				helper::AI_PopSpendingRequest(player);
				return 0;
			case UnitId::ZergLakizilisk:
				unitCopy = helper::AI_GetHydraliskForMorph(player);
				if (!unitCopy)
				{
					helper::AI_Reserve(player, 0);
					helper::AI_PopSpendingRequest(player);
					return 0;
				}
				break;
		}

		if (!helper::isUnitTypeRaceUnitRace(nextBuildType, unitCopy) || helper::createUnitAllowed(nextBuildType, unitCopy, player) != 1)
		{
			if (*lastInternalError == 2
			  || *lastInternalError == 23
			  || *lastInternalError == 21
			  || *lastInternalError == 8
			  || *lastInternalError == 7)
			{
				helper::AI_Reserve(player, 0);
				helper::AI_PopSpendingRequest(player);
				if (!boolSomething)
				{
					helper::AI_PerformTask(unit);
				}
				return 0;
			}
			error = *lastInternalError == 13;
			if (error)
			{
				helper::AI_Reserve(player, 0);
				helper::AI_PopSpendingRequest(player);
				if (!boolSomething)
				{
					helper::AI_PerformTask(unit);
				}
				return 0;
			}
		}

		if (!helper::hasResourcesForUnit(nextBuildType, player))
		{
			error = (AIScriptController[player].AI_Flags.isUseMapSettings) == 0;
			if (error)
			{
				helper::AI_Reserve(player, 0);
				helper::AI_PopSpendingRequest(player);
				if (!boolSomething)
				{
					helper::AI_PerformTask(unit);
				}
			}
			return 0;
		}

		if (helper::isAIAllowedToBuild(player, nextBuildType))
		{
			helper::AI_Reserve(player, 0);
			helper::AI_PopSpendingRequest(player);
			return 0;
		}

		if (unit == unitCopy)
		{
			requestedBuildId = helper::AI_GetRequestedBuildType(player);
			if (!helper::AI_TrainUnit(unitCopy, nextBuildType, requestedBuildId, AIScriptController[player].requests->val))
			{
				return 0;
			}
		}
		else
		{
			if (!helper::HasMoneyCanMake(unitCopy, nextBuildType))
			{
				return 0;
			}
			helper::AI_RemoveUnitAI(unitCopy, 0);
			if (helper::AI_GetRequestedBuildType(player) == 2)
			{
				/*
				BuildingAi* unitAI = (BuildingAi*)AIScriptController[player].requests->val;
				mainTownTown = unitAI->town;
				unitAIQueues = (s16)unitAI->trainQueueValues;
				unitAI->parent = unitCopy;
				unitAI->region = mainTownTown;
				unitAI->town = unitAIQueues;
				unitCopy->pAI = unitAI;
				*/
			}
			else
			{
				helper::AI_AddMilitary((AiRegion*)AIScriptController[player].requests->val, unitCopy, 1);
			}
		}

		helper::AI_Reserve(player, 1);
		helper::AI_PopSpendingRequest(player);
		unitId = unitCopy->id;
		if (unitId == UnitId::ZergLarva || unitId == UnitId::ZergMutalisk || unitId == UnitId::ZergHydralisk)
		{
			helper::issue_order_targeting_nothing(unitCopy, OrderId::Morph1);
			return 1;
		}
		else
		{
			helper::setSecondaryOrder(unitCopy, OrderId::Train);
			return 1;
		}
	}

	// Hooked to add more transport calls and hopefully improve AI defense logic. -Pr0nogo
	void ai_train_defensive_military(BYTE player_id) // 0043F990
	{
		u32 region_count;
		s32 v2;
		u8 v3;
		AiRegion* player_regions;
		AiRegion* military_region;
		u16 defense_region_id;
		u32 i;
		AiRegion* defense_region;
		u32 region_air_strength;
		s32 defense_region_id_copy;
		u32 j;
		u32 region_gnd_strength;
		u16 ai_flags;
		u8 v15;
		bool finished_with_regions;
		s16 transport_unit_id_1;
		u32 witness_count;
		s32 witness_request_quantity;
		u8 witness_supply_cost;
		s32 witness_gas_cost;
		u32 transport_1_count;
		u32 transport_1_quantity;
		s32 transport_1_priority;
		u8 transport_supply_cost;
		s32 v29;
		s32 transport_1_gas_cost;
		s32 region_count_zeroed;
		u32 transport_1_request_count;
		AiRegion* v33;
		s32 v34;
		u16 transport_unit_id_2;

		region_count = (*SAI_Paths)->regionCount;
		v33 = 0;
		if (region_count > 1)
		{
			v2 = 52;
			v34 = 52;
			region_count_zeroed = region_count - 1;
			do
			{
				player_regions = AiRegionCaptains[player_id];
				v3 = *(&player_regions->state + v2);
				military_region = (player_regions + v2);
				if (v3 > 3u)
				{
					if (v3 == 8)
					{
						helper::ai_region_train_attack_force(military_region);
					}
					else
					{
						if (v3 == 5 || v3 == 4)
						{
							v33 = military_region;
						}
						if (v3 < 7u)
						{
							defense_region_id = military_region->region;
						}
						else
						{
							defense_region_id = military_region->targetRegion;
						}
						region_air_strength = military_region->fullAirStrength;
						defense_region_id_copy = defense_region_id;
						i = player_regions[defense_region_id_copy].neededAirStrength;
						defense_region = &player_regions[defense_region_id_copy];
						if (i)
						{
							for (i = i; i > region_air_strength; i = defense_region->neededAirStrength)
							{
								region_air_strength += helper::ai_defend_region(military_region, defense_region_id, 1, i - region_air_strength);
							}
						}
						j = defense_region->neededGroundStrength;
						region_gnd_strength = military_region->fullGroundStrength;
						if (j)
						{
							for (j = j; j > region_gnd_strength; j = defense_region->neededGroundStrength)
							{
								region_gnd_strength += helper::ai_defend_region(military_region, defense_region_id, defense_region->enemyAirStrength != 0, j - region_gnd_strength);
							}
						}
						if (AIScriptController[player_id].AI_Flags.flag_0x40)
						{
							v15 = AIScriptController[player_id].defense_priority_base;
							if (v15)
							{
								AIScriptController[player_id].defense_priority_base = v15 - 1;
							}
							AIScriptController[player_id].AI_Flags.flag_0x40 = false;
						}
					}
				}
				v2 = v34 + 52;
				finished_with_regions = region_count_zeroed == 1;
				v34 += 52;
				--region_count_zeroed;
			} while (!finished_with_regions);
		}
		if (!(AIScriptController[player_id].AI_Flags.isTransportsOff))
		{
			u32 player_race = playerTable[player_id].race;
			if (player_race == RaceId::Terran)
			{
				if (!unitCountTable[UnitId::TerranPhalanxTankModeTurret].player[player_id + 12]) // what
				{
					return;
				}
				transport_unit_id_1 = UnitId::TerranTrojan;
				transport_unit_id_2 = UnitId::TerranTrojan;
			}
			else
			{
				if (player_race != RaceId::Protoss)
				{
					return;
				}
				witness_count = allUnitCountTable[UnitId::ProtossWitness].player[player_id];
				if (witness_count < 2)
				{
					witness_request_quantity = 2 - witness_count;
					do
					{
						witness_gas_cost = units_dat::GasCost[UnitId::ProtossWitness];
						witness_supply_cost = units_dat::SupplyRequired[UnitId::ProtossWitness];
						if (units_dat::BaseProperty[UnitId::ProtossWitness] & UnitProperty::TwoUnitsIn1Egg)
						{
							witness_supply_cost = 2 * units_dat::SupplyRequired[UnitId::ProtossWitness];
						}
						AIScriptController[player_id].mineralCollection += units_dat::MineralCost[UnitId::ProtossWitness];
						AIScriptController[player_id].gasCollection += witness_gas_cost;
						AIScriptController[player_id].supplyCollection += witness_supply_cost;
						helper::ai_add_spending_request(UnitId::ProtossWitness, player_id, 8, 80, 0);
						--witness_request_quantity;
					} while (witness_request_quantity);
				}
				transport_unit_id_2 = UnitId::ProtossEnvoy;
				transport_unit_id_1 = UnitId::ProtossEnvoy;
			}
			if (v33)
			{
				transport_1_count = allUnitCountTable[0].player[player_id + 12 * transport_unit_id_1];
				transport_1_quantity = (AIScriptController[player_id].AI_Flags.isUseMapSettings) != 0 ? 2 : 5;
				transport_1_priority = allUnitCountTable[0].player[player_id + 12 * transport_unit_id_1] != 0 ? 50 : 80;
				if (transport_1_count < transport_1_quantity)
				{
					transport_1_request_count = transport_1_quantity - transport_1_count;
					while (1)
					{
						AIScriptController[player_id].AI_Flags.flag_0x40 = true;
						transport_1_gas_cost = units_dat::GasCost[transport_unit_id_1];
						transport_supply_cost = units_dat::SupplyRequired[transport_unit_id_1];
						if (units_dat::BaseProperty[transport_unit_id_1] & UnitProperty::TwoUnitsIn1Egg)
						{
							transport_supply_cost *= 2;
						}
						v29 = AIScriptController[player_id].gasCollection;
						AIScriptController[player_id].mineralCollection += units_dat::MineralCost[transport_unit_id_1];
						AIScriptController[player_id].gasCollection = transport_1_gas_cost + v29;
						AIScriptController[player_id].supplyCollection += transport_supply_cost;
						helper::ai_add_spending_request(transport_unit_id_1, player_id, 1, transport_1_priority, v33);
						helper::ai_add_base_unit_spending_request(transport_unit_id_2, player_id, v33, transport_1_priority);
						if (!--transport_1_request_count)
						{
							break;
						}
						transport_unit_id_1 = transport_unit_id_2;
					}
				}
			}
		}
	}
	
	void AI_Build(u32 unit_id, bool need, u32 count, AiTown* town, u8 priority) // 004323D0
	{
		if (count <= 30 && unit_id < UnitId::None)
		{
			helper::Ai_AddBuildRequest(town, 0, count, unit_id, priority, need);
		}
	}

	// Hooked to improve logic. -Pr0nogo
	char AI_build_gas_building(AiTown* town) // 00433660
	{
		char result = 0;
		u32 player = town->player;
		u8 mineral_percent;
		u8 gas_percent;
		helper::AI_resource_need_split(&gas_percent, player, &mineral_percent);
		if (gas_percent || town->mineral)
		{
			Unit* refinery;
			u32 priority;
			if (helper::get_AI_refinery(town, 3, 1))
			{
				priority = 80;
				refinery = helper::get_AI_refinery(town, 1, 1);
				if (!refinery)
				{
					return 0;
				}

				u8 worker_count = town->worker_count;
				if (worker_count < town->worker_limit && worker_count < 0xFu)
				{
					return worker_count;
				}

				if (gameMinerals[player] < 500)
				{
					return player;
				}
			}
			else
			{
				refinery = helper::get_AI_refinery(town, 1, 1);
				if (!refinery)
				{
					return 0;
				}
				priority = 90;
			}
			u8 race = playerTable[player].race;
			u16 refinery_id = UnitId::TerranReservoir;
			if (race == RaceId::Zerg)
			{
				refinery_id = UnitId::ZergExcisant;
			}
			else if (race == RaceId::Protoss)
			{
				refinery_id = UnitId::ProtossAquifer;
			}
			result = helper::AI_add_town_spending_request(player, priority, refinery_id, town);
		}
		return result;
	}

	// Hooked to allow AI to train guards while under attack. -Pr0nogo
	void ai_request_guard(GuardAi* guard_ai, u32 player_id) // 00448630
	{
		u32 result = 0;
		u32 priority = 70;
		u32 last_hit = AIScriptController[player_id].previous_building_hit_second;
		//if (last_hit && *elapsedTimeSeconds - last_hit < 30) {
		//	return;
		//}
		s32 unit_id = guard_ai->unit_id;
		if (unit_id == UnitId::TerranPhalanxSiegeMode)
		{
			unit_id = UnitId::TerranPhalanxTankMode;
		}
		helper::ai_add_cost_to_needs(unit_id, player_id, 1);
		helper::ai_add_spending_request(unit_id, player_id, 2, priority, guard_ai);
		helper::ai_add_base_unit_spending_request(unit_id, player_id, scbw::get_ai_region(player_id, scbw::get_region_id(guard_ai->home)), priority);
	}

	// Hooked to change attack_add priorities. -Pr0nogo
	void ai_region_train_attack_force(AiRegion* region) { // 0043E670
		u32 attack_wave[63] = {};
		u32 priority = 80;
		u32 missing_attack_force = helper::get_missing_attack_force_units(region, attack_wave, AIScriptController[region->playerId].AI_Flags.isUseMapSettings);
		for (u32 i = 0; i < missing_attack_force; ++i) {
			u32 unit_id = attack_wave[i];
			if (unit_id < UnitId::None && !helper::ai_add_to_region_military(region, unit_id, priority))
			{
				AIScriptController[region->playerId].AI_Flags.flag_0x40 = true;
				helper::ai_add_cost_to_needs(unit_id, region->playerId, 1);
				helper::ai_add_spending_request(unit_id, region->playerId, 1, 60, region);
				helper::ai_add_base_unit_spending_request(unit_id, region->playerId, region, 60);
			}
		}
	}

	// Hooked to allow AI to build while under attack. -Pr0nogo
	void ai_add_town_spending_request(u32 player, u32 priority, u32 unit_id, AiRegion* region) { // 00448400
		u32 last_hit = AIScriptController[player].previous_building_hit_second;
		u32 race = playerTable[player].race;
		//if (last_hit && *elapsedTimeSeconds - last_hit < 30) {
		//	return;
		//}
		u32 type = 4;
		if (units_dat::BaseProperty[unit_id] & UnitProperty::Worker) {
			helper::ai_add_cost_to_needs(unit_id, player, type);
		} else {
			u32 type = 3;
			if (units_dat::BaseProperty[unit_id] & UnitProperty::Addon) {
				type = 7;
			}
			helper::ai_add_cost_to_needs(unit_id, player, type);
		}
		helper::ai_add_spending_request(unit_id, player, type, priority, region);
	}
}