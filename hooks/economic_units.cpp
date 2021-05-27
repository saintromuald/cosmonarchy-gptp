#include "hooks\economic_units.h"

namespace hooks
{

	u32 get_order_from_unit_id(u32 unit_id)
	{
		u32 order = 0;
		switch (unit_id)
		{
			case UnitId::TerranMason://0x7
	/*		case UnitId::mole:
			case UnitId::engineer:*/
				order = OrderId::BuildTerran;
				break;
			case UnitId::ZergDroleth://0x29
				order = OrderId::DroneStartBuild;
				break;
			case UnitId::ProtossScribe://0x40
				order = OrderId::BuildProtoss1;
				break;
			default:
				break;
		};
		return order;
	}

	u32 get_townhall_from_race(u32 race)
	{
		u32 id = 0xe4;
		switch (race)
		{
			case RaceId::Terran:
				id = UnitId::TerranCommandCenter;
				break;
			case RaceId::Protoss:
				id = UnitId::ProtossNexus;
				break;
			case RaceId::Zerg:
				id = UnitId::ZergHatchery;
				break;
			default:
				id = UnitId::ZergHatchery;
				break;
		}
		return id;
	}

	bool unit_is_correct_townhall(u32 unit_id, u32 cmp_id, Unit* unit)
	{
		//unit is unused. this is function is supposed to be used for custom faction worker
		switch (unit_id)
		{
			case UnitId::TerranCommandCenter:
				//		case UnitId::KelmorianTacticalCenter:
				//		case UnitId::UmojanGarrison:
				//		case UnitId::KelmorianMiningStation:		
			case UnitId::ProtossNexus:
			case UnitId::ZergHatchery:
				return true;
			default:
				break;
		}
		return false;
		//return (unit_id == cmp_id);
	}

	bool is_townhall(u32 unit_id)
	{
		switch (unit_id)
		{
			case UnitId::TerranCommandCenter:
			case UnitId::ProtossNexus:
			case UnitId::ZergHatchery://lairs and hives are not required
				return true;
			default:
				break;
		};
		return false;
	}

	bool is_terran_supply(u16 unit_id, Unit* worker)
	{
		//worker is not used originally in regards of supply, but may be required for custom race support
		if (unit_id == UnitId::TerranSupplyDepot)
		{
			return true;
		}
		return false;
	}

	bool is_vespene_top_structure(u16 unit_id)
	{
		switch (unit_id)
		{
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExtractor:
				return true;
			default:
				break;
		}
	}

	bool is_gasholder_structure(u16 unit_id)
	{
		switch (unit_id)
		{
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExtractor:
				//		case UnitId::KelMorianGasPump:
				return true;
			default:
				break;
		}
	}

	u32 get_worker_from_race(u32 race)
	{
		u32 id = 0xe4;
		switch (race)
		{
			case RaceId::Terran:
				id = UnitId::TerranMason;
				break;
			case RaceId::Protoss:
				id = UnitId::ProtossScribe;
				break;
			case RaceId::Zerg:
				id = UnitId::ZergDroleth;
				break;
			default:
				id = UnitId::ZergDroleth;
				break;
		}
		return id;
	}

	u32 get_supply_structure_from_race(u32 race, u8 player)
	{
		u32 id = 0xe4;
		//player is not used originally but may be required for custom race support
		switch (race)
		{
			case RaceId::Terran:
				id = UnitId::TerranSupplyDepot;
				break;
			case RaceId::Protoss:
				id = UnitId::ProtossPylon;
				break;
			case RaceId::Zerg:
				id = UnitId::ZergOvileth;
				break;
			default:
				id = UnitId::ZergOvileth;
				break;
		}
		return id;
	}

	// Returns town race judging by main town, first checks if it has buildings assigned
	u8 GetTownRace(AiTown* town)
	{
		if (!town) return RaceId::Neutral;
		if (!town->main_building) return RaceId::Neutral; // can't tell from main town, might want to check from buildings maybe
		return town->main_building->getRace();
	}

	// Checks if worker is valid (exists && correct race)
	bool IsValidWorker(const WorkerAi& worker, u8 raceId)
	{
		if (!worker.parent) return false;
		if (worker.parent->getRace() != raceId) return false;
		return true;
	}

	// Checks if town has a free worker of given race
	// First checks SC's first free worker var; If invalid iterates over all free worker
	// This was needed since first free worker is sometimes null.
	bool HasFreeWorker(AiTown* town, u8 raceId)
	{
		WorkerAi* first = town->free_workers->first_free_worker;
		if (first)
		{
			if (IsValidWorker(*first, raceId))
			{
				return true;
			}
		}
		for (const WorkerAi& worker : town->free_workers->worker_ais)
		{
			if (IsValidWorker(worker, raceId)) return true;
		}
		return false;
	}

	// Checks if town is eligible for supply building (correct race && has correct free worker)
	bool IsTownEligible(AiTown* town, u8 raceId)
	{
		if (!town) return false;
		if (town->in_battle) return false; // We probably don't want supplies to be built in attacked towns. We might want to ommit this condition if it's the only eligable town
		if (GetTownRace(town) != raceId) return false;
		if (town->main_building->remainingBuildTime != 0) return false; // town not finished, nullchecks are in the previous function
		if (raceId == RaceId::Zerg)
		{ // Zerg don't need worker for supply building, we could check for larvae instead, but it should be fine
			return true;
		}
		return HasFreeWorker(town, raceId);
	}

	// First checks if main town is eligible for supply building
	// If not then iterates on every player's town and then checks if it's eligible for supply building
	AiTown* GetFirstEligibleTown(AiTown* mainTown, u8 playerId, u8 raceId)
	{
		if (mainTown)
		{
			if (IsTownEligible(mainTown, raceId))
			{
				return mainTown;
			}
		}
		AiTown* aiTown = AiArray; // first
		if (!aiTown) return nullptr; // no AI towns
		do
		{
			if (aiTown->player == playerId)
			{
				if (IsTownEligible(aiTown, raceId))
				{
					return aiTown;
				}
			}
		}
		while (aiTown = aiTown->link.next);
		//scbw::printString("Can't find eligible town for supply building!");
		return nullptr; // couldn't find anything
	}

	void Ai_BuildSupplies(AiTown* mainTown)
	{
		u8 player = mainTown->player;
		u8 race = playerTable[player].race;
		if (mainTown)
		{
			if (checkSupplyNeed(player, RaceId::Terran))
			{
				AiTown* town = GetFirstEligibleTown(mainTown, player, RaceId::Terran);
				if (town)
				{
					AI_AddTownSpendingRequest(player, scbw::getSupplyRemaining(player, RaceId::Terran) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Terran, player), (void*)town);
				}
			}
			if (checkSupplyNeed(player, RaceId::Protoss))
			{
				AiTown* town = GetFirstEligibleTown(mainTown, player, RaceId::Protoss);
				if (town)
				{
					AI_AddTownSpendingRequest(player, scbw::getSupplyRemaining(player, RaceId::Protoss) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Protoss, player), (void*)town);
				}
			}
			if (checkSupplyNeed(player, RaceId::Zerg))
			{
				AiTown* town = GetFirstEligibleTown(mainTown, player, RaceId::Zerg);
				if (town)
				{
					AI_AddTownSpendingRequest(player, scbw::getSupplyRemaining(player, RaceId::Zerg) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Zerg, player), (void*)town);
				}
			}
		}
	}

	bool checkSupplyNeed(u8 player, u8 race)
	{
		u32 checkEcoUnits;
		s32 supplyRemaining;
		u32 workerUnit = get_worker_from_race(race);
		s32 supplyUsedCompare;
		u8 townHallSupply;
		u32 supplyUnit = get_supply_structure_from_race(race, player);

		supplyRemaining = helper::getSuppliesRemaining(player, workerUnit);

		if (race == RaceId::Protoss)
		{
			if (helper::Ai_HasBuildingsNeedingPower(player))
			{
				return true;
			}
		}
		if (supplyRemaining < 13)
		{
			supplyUsedCompare = helper::getSuppliesUsed(race, player) + 13;
			townHallSupply = units_dat::SupplyProvided[get_townhall_from_race(race)];
			if (supplyUsedCompare > townHallSupply)
			{
				checkEcoUnits = (supplyUsedCompare - townHallSupply) / units_dat::SupplyProvided[supplyUnit];
				//farms_timing on, compare ecounits required vs ecounits count
				if (!(AIScriptController[player].AI_Flags.isFarmsNotimingOn) && (checkEcoUnits + 1 > helper::Ai_CountEconomyUnits(player, supplyUnit)))
				{
					return true;
				}
				//if max supply, and current supply units (including those under construction) will not satisfy requirement
				if ((!supplyRemaining) && helper::Ai_CountEconomyUnits(player, supplyUnit) * units_dat::SupplyProvided[supplyUnit] + townHallSupply <= helper::getSuppliesUsed(race, player))
				{
					return true;
				}
			}
		}
		return false;
	}

	u32 orders_isGeyserTopUnitHook(Unit* unit)
	{
		if (unit->status & UnitStatus::Completed)
		{
			if (is_vespene_top_structure(unit->id))
			{
				return true;
			}
		}
		return false;
	}

	s32 AI_AddTownSpendingRequest(s32 playerId, s32 priority, s32 unitId, void* param)
	{ // 00448400
		s32 battleTime;
		s8 race;
		s32 result;
		s32 reqType;

		battleTime = AIScriptController[playerId].lastIndividualUpdateTime;
		if (!battleTime
			|| *elapsedTimeSeconds - battleTime >= 0x1E // in_battle flag? just a coincidence?
			|| ((race = playerTable[playerId].race, race != 1) ? (result = 8 * (race != 0) + 149) : (result = 110),
				unitId == result || priority == 140))
		{
			if (units_dat::BaseProperty[unitId] & UnitProperty::Worker)
			{
				reqType = 4;
			}
			else
			{
				reqType = 2 * (units_dat::BaseProperty[unitId] & UnitProperty::Addon) | 3; // 3 = UnitProperty::Addon + UnitProperty::Building
			}
			helper::AI_AddCostToNeeds(unitId, playerId, reqType);
			result = helper::AI_AddSpendingRequest(unitId, playerId, reqType, priority, param);
		}
		return result;
	}
	/*
	s32 AI_AddSpendingRequest(s16 a1, s32 a2, char a3, char a4, void* a5) { // 00447980
		char v5; // bl
		s16 v6; // di
		s32 v7; // esi
		s32 result; // eax
		u8 v9; // al
		AI_Main* v10; // ecx
		s32 v11; // esi
		s32 v12; // edx
		s32 v13; // edi
		s32 v14; // [esp+Ch] [ebp-8h]

		v6 = a1;
		v5 = a3;
		v7 = a2;
		switch (a3)
		{
		case 1:
		case 2:
		case 4:
		case 8:
			result = isAIAllowedToBuild(a2, a1);
			if (!result)
				goto LABEL_3;
			break;
		default:
		LABEL_3:
			v10 = &AIScriptController[v7];
			v9 = v10->waitForceCount;
			LOBYTE(v14) = a4;
			BYTE1(v14) = v5;
			HIWORD(v14) = v6;
			if (v9 == 63)
			{
				v11 = 62;
			}
			else
			{
				v11 = v9;
				v10->waitForceCount = v9 + 1;
			}
			*((s32*)&v10->unknown_0x18 + 2 * v11) = v14;
			result = (v11 - 1) / 2;
			for (*((s32*)&v10->pTownMain + 2 * v11) = a5; v11 > 0; result = (result - 1) / 2)
			{
				if ((unsigned __int8)*(&v10->unknown_0x18 + 8 * v11) <= *(&v10->unknown_0x18 + 8 * result))
					break;
				v12 = *((s32*)&v10->unknown_0x18 + 2 * v11);
				v13 = *((s32*)&v10->pTownMain + 2 * v11);
				*((s32*)&v10->unknown_0x18 + 2 * v11) = *((s32*)&v10->unknown_0x18 + 2 * result);
				*((s32*)&v10->pTownMain + 2 * v11) = *((s32*)&v10->pTownMain + 2 * result);
				v11 = result;
				*((s32*)&v10->unknown_0x18 + 2 * result) = v12;
				*((s32*)&v10->pTownMain + 2 * result) = v13;
			}
			break;
		}
		return result;
	}*/

	/*   TODO: Worker-related hooks
	Line 78684: 00434C36  |.  83F8 29       CMP EAX,29
	Line 80051: 00435A88  |.  83C0 29       ADD EAX,29
	Line 81026: 0043655E  |.  83C1 29       ADD ECX,29
	Line 107167: 00447AAE  |.  83C0 29       ADD EAX,29
	Line 108426: 004488FF  |.  83C0 29       ADD EAX,29
	Line 211601: 0048DE9C  |.  66:83FA 29    CMP DX,29
	Line 240222: 004A134E   .  66:837E 64 29 CMP WORD PTR DS:[ESI+64],29
	Line 285431: 004BFDB6  |.  C645 FC 29    MOV BYTE PTR SS:[EBP-4],29
	Line 347662: 004E9EC0   >  66:83F9 29    CMP CX,29
	*/
}