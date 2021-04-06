#include "hooks\economic_units.h"

namespace {

	// 
	int refundAllQueueSlots(CUnit* a1);																			// 00466E80
	int Ai_HasBuildingsNeedingPower(uint32 player);																// 00433410
	int Ai_CountEconomyUnits(uint32 player, uint32 unit_id);															// 00433470
	int getSuppliesRemaining(uint8 player, uint16 unit_id);															// 004889D0
	int getSuppliesUsed(int byte_data, uint8 player);																// 004888C0

	// For AI_AddTownSpendingRequest
	int32 AI_AddCostToNeeds(int32 a1, int32 a2, int32 a3);																// 00447DC0
	int32 AI_AddSpendingRequest(int16 a1, int32 a2, char a3, char a4, void* a5);										// 00447980

	// For AI_AddSpendingRequest
	bool isAIAllowedToBuild(int32 a1, int32 a2);																	// 00447830

}

namespace hooks {

	uint32 get_order_from_unit_id(uint32 unit_id) {
		uint32 order = 0;
		switch (unit_id) {
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

	uint32 get_townhall_from_race(uint32 race) {
		uint32 id = 0xe4;
		switch (race) {
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

	bool unit_is_correct_townhall(uint32 unit_id, uint32 cmp_id, CUnit* unit) {
		//unit is unused. this is function is supposed to be used for custom faction workers
		switch (unit_id) {
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

	bool is_townhall(uint32 unit_id) {
		switch (unit_id) {
			case UnitId::TerranCommandCenter:
			case UnitId::ProtossNexus:
			case UnitId::ZergHatchery://lairs and hives are not required
				return true;
			default:
				break;
		};
		return false;
	}

	bool is_terran_supply(uint16 unit_id, CUnit* worker) {
		//worker is not used originally in regards of supply, but may be required for custom race support
		if (unit_id == UnitId::TerranSupplyDepot) {
			return true;
		}
		return false;
	}

	bool is_vespene_top_structure(uint16 unit_id) {
		switch (unit_id) {
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExtractor:
				return true;
			default:
				break;
		}
	}

	bool is_gasholder_structure(uint16 unit_id) {
		switch (unit_id) {
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExtractor:
				//		case UnitId::KelMorianGasPump:
				return true;
			default:
				break;
		}
	}

	uint32 get_worker_from_race(uint32 race) {
		uint32 id = 0xe4;
		switch (race) {
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

	uint32 get_supply_structure_from_race(uint32 race, uint8 player) {
		uint32 id = 0xe4;
		//player is not used originally but may be required for custom race support
		switch (race) {
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

	// Veeq7

	// Returns town race judging by main town, first checks if it has buildings assigned
	uint8 GetTownRace(AiTown* town) {
		if (!town) return RaceId::Neutral;
		if (!town->main_building) return RaceId::Neutral; // can't tell from main town, might want to check from buildings maybe
		return town->main_building->getRace();
	}

	// Checks if worker is valid (exists && correct race)
	bool IsValidWorker(const WorkerAi& worker, uint8 raceId) {
		if (!worker.parent) return false;
		if (worker.parent->getRace() != raceId) return false;
		return true;
	}

	// Checks if town has a free worker of given race
	// First checks SC's first free worker var; If invalid iterates over all free workers
	// This was needed since first free worker is sometimes null.
	bool HasFreeWorker(AiTown* town, uint8 raceId) {
		WorkerAi* first = town->free_workers->first_free_worker;
		if (first) {
			if (IsValidWorker(*first, raceId)) {
				return true;
			}
		}
		for (const WorkerAi& worker : town->free_workers->worker_ais) {
			if (IsValidWorker(worker, raceId)) return true;
		}
		return false;
	}

	// Checks if town is eligible for supply building (correct race && has correct free worker)
	bool IsTownEligible(AiTown* town, uint8 raceId) {
		if (!town) return false;
		if (town->in_battle) return false; // We probably don't want supplies to be built in attacked towns. We might want to ommit this condition if it's the only eligable town
		if (GetTownRace(town) != raceId) return false;
		if (town->main_building->remainingBuildTime != 0) return false; // town not finished, nullchecks are in the previous function
		if (raceId == RaceId::Zerg) { // Zerg don't need workers for supply building, we could check for larvae instead, but it should be fine
			return true;
		}
		return HasFreeWorker(town, raceId);
	}

	// First checks if main town is eligible for supply building
	// If not then iterates on every player's town and then checks if it's eligible for supply building
	AiTown* GetFirstEligibleTown(AiTown* mainTown, uint8 playerId, uint8 raceId) {
		if (mainTown) {
			if (IsTownEligible(mainTown, raceId)) {
				return mainTown;
			}
		}
		AiTown* aiTown = AiArray; // first
		if (!aiTown) return nullptr; // no AI towns
		do {
			if (aiTown->player == playerId) {
				if (IsTownEligible(aiTown, raceId)) {
					return aiTown;
				}
			}
		} while (aiTown = aiTown->link.next);
		//scbw::print("Can't find eligible town for supply building!");
		return nullptr; // couldn't find anything
	}

	// ---

	void Ai_BuildSupplies(AiTown* town) {
		// Tidomann
		uint8 race_;
		uint8 player_;
		bool hasTerran_ = false;
		bool hasZerg_ = false;
		bool hasProtoss_ = false;
		if (town) 		{
			player_ = town->player;
			race_ = playerTable[player_].race;
			//Race Checks, Allows main race to continue to function, but will consider other races if there is supply exists
			//and supply is provided
			switch (race_) {
				case RaceId::Terran:
					hasTerran_ = true;
					if (raceSupply[RaceId::Zerg].used[player_] != 0 && raceSupply[RaceId::Zerg].provided[player_] != 0) {
						hasZerg_ = true;
					}
					if (raceSupply[RaceId::Protoss].used[player_] != 0 && raceSupply[RaceId::Protoss].provided[player_] != 0) {
						hasProtoss_ = true;
					}
					break;
				case RaceId::Zerg:
					hasZerg_ = true;
					if (raceSupply[RaceId::Terran].used[player_] != 0 && raceSupply[RaceId::Terran].provided[player_] != 0) {
						hasTerran_ = true;
					}
					if (raceSupply[RaceId::Protoss].used[player_] != 0 && raceSupply[RaceId::Protoss].provided[player_] != 0) {
						hasProtoss_ = true;
					}
					break;
				case RaceId::Protoss:
					hasProtoss_ = true;
					if (raceSupply[RaceId::Terran].used[player_] != 0 && raceSupply[RaceId::Terran].provided[player_] != 0) {
						hasTerran_ = true;
					}
					if (raceSupply[RaceId::Zerg].used[player_] != 0 && raceSupply[RaceId::Zerg].provided[player_] != 0) {
						hasZerg_ = true;
					}
					break;
			}

			if (hasProtoss_) {
				if (checkSupplyNeed(player_, RaceId::Protoss)) {
					town = GetFirstEligibleTown(town, player_, RaceId::Protoss);
					if (town)
						AI_AddTownSpendingRequest(player_, scbw::getSupplyRemaining(player_, RaceId::Protoss) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Protoss, player_), (void*) town);
				}
			}
			if (hasZerg_) {
				if (checkSupplyNeed(player_, RaceId::Zerg)) {
					town = GetFirstEligibleTown(town, player_, RaceId::Zerg);
					if (town)
						AI_AddTownSpendingRequest(player_, scbw::getSupplyRemaining(player_, RaceId::Zerg) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Zerg, player_), (void*) town);
				}
			}
			if (hasTerran_) {
				if (checkSupplyNeed(player_, RaceId::Terran)) {
					town = GetFirstEligibleTown(town, player_, RaceId::Terran);
					if (town)
						AI_AddTownSpendingRequest(player_, scbw::getSupplyRemaining(player_, RaceId::Terran) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Terran, player_), (void*) town);
				}
			}
		}
	}

	bool checkSupplyNeed(uint8 player_, uint8 race_) {
		unsigned int checkEcoUnits_;
		int supplyRemaining_;
		uint32 workerUnit = get_worker_from_race(race_);
		int supplyUsedCompare_;
		uint8 townHallSupply_;
		uint32 supplyUnit_ = get_supply_structure_from_race(race_, player_);

		supplyRemaining_ = getSuppliesRemaining(player_, workerUnit);

		if (race_ == RaceId::Protoss) {
			if (Ai_HasBuildingsNeedingPower(player_)) {
				return true;
			}
		}
		if (supplyRemaining_ < 13) {
			supplyUsedCompare_ = getSuppliesUsed(race_, player_) + 13;
			townHallSupply_ = units_dat::SupplyProvided[get_townhall_from_race(race_)];
			if (supplyUsedCompare_ > townHallSupply_) {
				checkEcoUnits_ = (supplyUsedCompare_ - townHallSupply_) / units_dat::SupplyProvided[supplyUnit_];
				//farms_timing on, compare ecounits required vs ecounits count
				if (!(AIScriptController[player_].AI_Flags.isFarmsNotimingOn) && (checkEcoUnits_ + 1 > Ai_CountEconomyUnits(player_, supplyUnit_))) {
					return true;
				}
				//if max supply , and current supply units (including those under construction) will not satisfy requirement
				if ((!supplyRemaining_) && Ai_CountEconomyUnits(player_, supplyUnit_) * units_dat::SupplyProvided[supplyUnit_] + townHallSupply_ <= getSuppliesUsed(race_, player_)) {
					return true;
				}
			}
		}
		return false;
	}

	uint32 orders_isGeyserTopUnitHook(CUnit* unit) {
		if (unit->status & UnitStatus::Completed) {
			if (is_vespene_top_structure(unit->id)) {
				return true;
			}
		}
		return false;
	}

	int32 AI_AddTownSpendingRequest(int32 playerId, int32 priority, int32 unitId, void* param) { // 00448400
		int battleTime;
		int8 race;
		int result;
		int32 reqType;

		battleTime = AIScriptController[playerId].lastIndividualUpdateTime;
		if (!battleTime
			|| *elapsedTimeSeconds - battleTime >= 0x1E // in_battle flag? just a coincidence?
			|| ((race = playerTable[playerId].race, race != 1) ? (result = 8 * (race != 0) + 149) : (result = 110),
				unitId == result || priority == 140)) 		{
			if (units_dat::BaseProperty[unitId] & UnitProperty::Worker) {
				reqType = 4;
			} 			else {
				reqType = 2 * (units_dat::BaseProperty[unitId] & UnitProperty::Addon) | 3; // 3 = UnitProperty::Addon + UnitProperty::Building
			}
			AI_AddCostToNeeds(unitId, playerId, reqType);
			result = AI_AddSpendingRequest(unitId, playerId, reqType, priority, param);
		}
		return result;
	}
	/*
	int32 AI_AddSpendingRequest(int16 a1, int32 a2, char a3, char a4, void* a5) { // 00447980
		char v5; // bl
		int16 v6; // di
		int32 v7; // esi
		int32 result; // eax
		uint8 v9; // al
		AI_Main* v10; // ecx
		int32 v11; // esi
		int32 v12; // edx
		int32 v13; // edi
		int32 v14; // [esp+Ch] [ebp-8h]

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
			*((int32*)&v10->unknown_0x18 + 2 * v11) = v14;
			result = (v11 - 1) / 2;
			for (*((int32*)&v10->pTownMain + 2 * v11) = a5; v11 > 0; result = (result - 1) / 2)
			{
				if ((unsigned __int8)*(&v10->unknown_0x18 + 8 * v11) <= *(&v10->unknown_0x18 + 8 * result))
					break;
				v12 = *((int32*)&v10->unknown_0x18 + 2 * v11);
				v13 = *((int32*)&v10->pTownMain + 2 * v11);
				*((int32*)&v10->unknown_0x18 + 2 * v11) = *((int32*)&v10->unknown_0x18 + 2 * result);
				*((int32*)&v10->pTownMain + 2 * v11) = *((int32*)&v10->pTownMain + 2 * result);
				v11 = result;
				*((int32*)&v10->unknown_0x18 + 2 * result) = v12;
				*((int32*)&v10->pTownMain + 2 * result) = v13;
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

namespace {

	const uint32 Func_0x00433410 = 0x00433410;
	int Ai_HasBuildingsNeedingPower(uint32 player) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL Func_0x00433410
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_0x00433470 = 0x00433470;
	int Ai_CountEconomyUnits(uint32 player, uint32 unit_id) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, player
			PUSH unit_id
			CALL Func_0x00433470
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_0x004889D0 = 0x004889D0;
	int getSuppliesRemaining(uint8 player, uint16 unit_id) {
		static int result;
		__asm {
			PUSHAD
			MOV BL, player
			MOV DI, unit_id
			CALL Func_0x004889D0
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_4888c0 = 0x004888C0;
	int getSuppliesUsed(int byte_data, uint8 player) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV EAX, byte_data
			MOV CL, player
			CALL Func_4888c0
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_AddCostToNeeds = 0x00447DC0;
	int32 AI_AddCostToNeeds(int32 a1, int32 a2, int32 a3) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EBX, a2
			MOV EAX, a1
			CALL Func_AI_AddCostToNeeds
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_AddSpendingRequest = 0x00447980;
	int32 AI_AddSpendingRequest(int16 a1, int32 a2, char a3, char a4, void* a5) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			MOVZX CL, a3
			MOV EDX, a2
			MOVZX AX, a1
			CALL Func_AI_AddSpendingRequest
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For AI_AddSpendingRequest
	const uint32 Func_isAIAllowedToBuild = 0x00447830;
	bool isAIAllowedToBuild(int32 a1, int32 a2) {
		static int32 result;
		__asm {
			PUSHAD
			MOV ECX, a2
			MOV EAX, a1
			CALL Func_isAIAllowedToBuild
			MOV result, EAX
			POPAD
		}
		return (bool) result;
	}
}