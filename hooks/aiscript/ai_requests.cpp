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
		TownReq* townUnits;
		if (playerTable[(u8)town->player].type == PlayerType::Computer)
		{
			townUnits = town->townUnits;
			bool setNeed = false;
			if (town->townUnits[0].flags_and_count & BuildCountFlag) // if build
			{
				for (s32 i = 0; i < 100; i++)
				{
					++townUnits;
					if (!(townUnits->flags_and_count & BuildCountFlag)) // if not build
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
				town->townUnits[iterate].flags_and_count = need | 0x2 * (method & 0x3 | 0x4 * amount);
				town->townUnits[iterate].id = id;
				town->townUnits[iterate].priority = priority;
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
				Unit* firstTownUnitVar = firstTown->firstTownUnit;
				if (firstTown->firstTownUnit)
				{
					do
					{
						Unit* targetUnit = firstTownUnitVar->moveTarget.unit; // This was null during a crash
						if (targetUnit == nullptr)
						{
							scbw::print("null unit in AI_progressBuild");
							return result;
						}
						s32 buildOrderId = (u8)targetUnit->mainOrderId;
						if (buildOrderId == OrderId::BuildTerran
						  || buildOrderId == OrderId::BuildProtoss1
						  || buildOrderId == OrderId::DroneLand
						  || buildOrderId == OrderId::DroneBuild
						  || buildOrderId == OrderId::DroneStartBuild)
						{
							u8* consoleVar = nullptr;
							s32 playerIdVar = (u8)playerId;
							u16 targetUnitQueue = targetUnit->buildQueue[(u8)targetUnit->buildQueueSlot % 5];
							s32 targetUnitQueue2 = targetUnitQueue;
							s32 targetQueueCostMinerals = units_dat::MineralCost[targetUnitQueue2];
							s32 targetQueueCostGas = units_dat::GasCost[targetUnitQueue2];
							AIMineralsReserve[playerIdVar] = targetQueueCostMinerals;
							AIGasReserve[playerIdVar] = targetQueueCostGas;
							if (helper::hasSuppliesForUnit(playerId, targetUnitQueue, 1))
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
						firstTownUnitVar = firstTownUnitVar->link.prev;
					}
					while (firstTownUnitVar);
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
				unitId = UnitId::TerranFactory;
			} break;
			case UnitId::TerranGuildhall:
			{
				unitId = UnitId::TerranCaptaincy;
			} break;
			case UnitId::TerranFutureStation:
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
					if (helper::getBunkerPlacementPosition(playerVar, &ownrId))
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

	char AI_PerformTask(Unit* unit) // 004361A0 -- hooked to fix AI Archon training -- Pr0nogo
	{
		switch (unit->id)
		{
			case UnitId::ZergOvileth:
				helper::AI_OverlordBehavior(unit);
				break;
			case UnitId::TerranComsatStation:
				helper::AI_CastSpellBehaviour(unit, 0);
				break;
			case UnitId::TerranNuclearSilo:
				helper::AI_NukeSiloBehavior(unit);
				break;
		}

		UnitAi* unitAIVar = (UnitAi*)unit->pAI;
		UnitAi* unitAIVar2 = (UnitAi*)unit->pAI;
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
			unitAIVar = unitAIVar2;
		}

		AI_Main* AI_Mainframe = &AIScriptController[player];
		s32 request_type;
		if (AI_Mainframe->request_count)
		{
			request_type = AI_Mainframe->requests->ty;
		}
		else
		{
			request_type = 0;
		}

		char result;
		switch (request_type)
		{
			case 0: // UnitMake
			{
				if (!helper::AI_UnitConstructionBehavior(player, unit, 50))
				{
					result = 1;
					break; // Was goto label 21
				}
				someBool = 1;
				return helper::AI_PlanDoMergeMorph(player, unit, someBool);
			}
			case 1: // UnitMergeMorph1
			case 2: // UnitMergeMorph2
			{
				return helper::AI_PlanDoMergeMorph(player, unit, someBool);
			}
			case 3: // ConstructMorph
			{
				return helper::AI_MorphBehavior(player, unit, unit->pAI);
			}
			case 4: // Train
			{
				helper::AI_TrainBehavior(unit, unit->pAI, player);
				result = 1;
			} break;
			case 5: // Upgrade
			{
				helper::AI_UpgradeBehavior(unit, player);
				result = 1;
			} break;
			case 6: // Research
			{
				helper::AI_ResearchBehavior(unit, player);
				result = 1;
			} break;
			case 7: // PlaceAddon
			{
				helper::AI_ConstructBehavior(unit, unit->pAI, player);
				result = 1;
			} break;
			case 8: // MakeDetector
			{
				result = helper::AI_MakeDetectorBehavior(unit, player);
			} break;
			default:
			{
				result = 1;
			} break;
		}
		return result;
	}
}