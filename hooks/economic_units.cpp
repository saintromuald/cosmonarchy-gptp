#include "hooks\economic_units.h"

namespace {

	signed int isUnitTypeRaceUnitRace(unsigned __int16 a1, CUnit* a2);											// 00432480
	signed int techtreeAllowed(unsigned __int16 a1, CUnit* a2, int a3);											// 0046E1C0
	bool hasResourcesForUnit(unsigned __int16 a1, int a2);														// 004487B0
	int getPlayerDefaultRefineryUnitType(int a1);																// 004320D0
	signed int getBunkerPosition(int a1, Point32* a2);															// 004376F0
	__int16 getRegionIdFromPixels(signed int a1, signed int a2);												// 0049C9F0
	void doAreaFixupsForEnemy(int a1, unsigned __int16 a2, unsigned int a3);									// 00444EB0
	signed int AiManageBuildingPlacement(CUnit* a1, unsigned int a2, Point32* a3, Point32* a4, signed int a5);	// 004464C0
	int AiFinishUnit(CUnit* a1);																				// 00433640
	void AiAssignMilitary(int a1, AiCaptain* a2);																// 004390A0
	signed int getTurretPosition(int a1, Point32* a2);															// 00437670
	void getSecondHatcheryBuildLocation(int a1, __int16 a2, __int16 a3);										// 00444630
	CUnit* getAiRefinery(AiTown* a1, int a2, int a3);															// 00432B90
	__int16 AiReserve(int a1, int a2);																			// 00447D00
	AI_Main* AiPopSpendingRequest(int a1);																		// 004476D0
	void orderTarget(CUnit* a1, unsigned __int8 a2, __int16 a3, __int16 a4);									// 00475260
	int refundAllQueueSlots(CUnit* a1);																			// 00466E80
	bool hasMoneyCanMake(CUnit* a1, unsigned __int16 a2);														// 00467250
	int Ai_HasBuildingsNeedingPower(u32 player);																// 00433410
	int Ai_CountEconomyUnits(u32 player, u32 unit_id);															// 00433470
	int getSuppliesRemaining(u8 player, u16 unit_id);															// 004889D0
	int getSuppliesUsed(int byte_data, u8 player);																// 004888C0
	s32 AI_AddCostToNeeds(s32 a1, s32 a2, s32 a3);																// 00447DC0
	s32 AI_AddSpendingRequest(s16 a1, s32 a2, char a3, char a4, void* a5);										// 00447980
}

namespace hooks {

	u32 get_order_from_unit_id(u32 unit_id) {
		u32 order = 0;
		switch (unit_id) {
		case UnitId::TerranSCV://0x7
/*		case UnitId::mole:
		case UnitId::engineer:*/
			order = OrderId::BuildTerran;
			break;
		case UnitId::ZergDrone://0x29
			order = OrderId::DroneStartBuild;
			break;
		case UnitId::ProtossProbe://0x40
			order = OrderId::BuildProtoss1;
			break;
		default:
			break;
		};
		return order;
	}

	u32 get_townhall_from_race(u32 race) {
		u32 id = 0xe4;
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

	bool unit_is_correct_townhall(u32 unit_id, u32 cmp_id, CUnit* unit) {
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

	bool is_townhall(u32 unit_id) {
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

	bool is_terran_supply(u16 unit_id, CUnit* worker) {
		//worker is not used originally in regards of supply, but may be required for custom race support
		if (unit_id == UnitId::TerranSupplyDepot) {
			return true;
		}
		return false;
	}

	bool is_vespene_top_structure(u16 unit_id) {
		switch (unit_id) {
		case UnitId::TerranRefinery:
		case UnitId::ProtossAssimilator:
		case UnitId::ZergExtractor:
			return true;
		default:
			break;
		}
	}

	bool is_gasholder_structure(u16 unit_id){
		switch (unit_id) {
		case UnitId::TerranRefinery:
		case UnitId::ProtossAssimilator:
		case UnitId::ZergExtractor:
//		case UnitId::KelMorianGasPump:
			return true;
		default:
			break;
		}
	}

	u32 get_worker_from_race(u32 race) {
		u32 id = 0xe4;
		switch (race) {
		case RaceId::Terran:
			id = UnitId::TerranSCV;
			break;
		case RaceId::Protoss:
			id = UnitId::ProtossProbe;
			break;
		case RaceId::Zerg:
			id = UnitId::ZergDrone;
			break;
		default:
			id = UnitId::ZergDrone;
			break;
		}
		return id;
	}

	u32 get_supply_structure_from_race(u32 race, u8 player) {
		u32 id = 0xe4;
		//player is not used originally but may be required for custom race support
		switch (race) {
		case RaceId::Terran:
			id = UnitId::TerranSupplyDepot;
			break;
		case RaceId::Protoss:
			id = UnitId::ProtossPylon;
			break;
		case RaceId::Zerg:
			id = UnitId::ZergOverlord;
			break;
		default:
			id = UnitId::ZergOverlord;
			break;
		}
		return id;
	}

	void Ai_BuildSupplies(AiTown* town) {
		// Tidomann
		u8 race_;
		u8 player_;
		bool hasTerran_ = false;
		bool hasZerg_ = false;
		bool hasProtoss_ = false;
		if (town)
		{
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
					AI_AddTownSpendingRequest(player_, scbw::getSupplyRemaining(player_, RaceId::Protoss) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Protoss, player_), (void*)town);
				}
			}
			if (hasZerg_) {
				if (checkSupplyNeed(player_, RaceId::Zerg)) {
					AI_AddTownSpendingRequest(player_, scbw::getSupplyRemaining(player_, RaceId::Zerg) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Zerg, player_), (void*)town);
				}
			}
			if (hasTerran_) {
				if (checkSupplyNeed(player_, RaceId::Terran)) {
					AI_AddTownSpendingRequest(player_, scbw::getSupplyRemaining(player_, RaceId::Terran) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Terran, player_), (void*)town);
				}
			}
		}
	}

	bool checkSupplyNeed(u8 player_, u8 race_) {
		unsigned int checkEcoUnits_;
		int supplyRemaining_;
		u32 workerUnit = get_worker_from_race(race_);
		int supplyUsedCompare_;
		u8 townHallSupply_;
		u32 supplyUnit_ = get_supply_structure_from_race(race_, player_);

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

	u32 orders_isGeyserTopUnitHook(CUnit* unit) {
		if (unit->status & UnitStatus::Completed) {
			if (is_vespene_top_structure(unit->id)) {
				return true;
			}
		}
		return false;
	}

	s32 AI_AddTownSpendingRequest(s32 playerId, s32 priority, s32 unitId, void* param) { // 00448400
		int battleTime;
		s8 race;
		int result;
		s32 reqType;

		battleTime = AIScriptController[playerId].lastIndividualUpdateTime;
		if (!battleTime
			|| *elapsedTimeSeconds - battleTime >= 0x1E // in_battle flag? just a coincidence?
			|| ((race = playerTable[playerId].race, race != 1) ? (result = 8 * (race != 0) + 149) : (result = 110),
				unitId == result || priority == 140))
		{
			if (units_dat::BaseProperty[unitId] & UnitProperty::Worker)
				reqType = 4;
			else
				reqType = 2 * (units_dat::BaseProperty[unitId] & UnitProperty::Addon) | 3; // 3 = UnitProperty::Addon + UnitProperty::Building
			AI_AddCostToNeeds(unitId, playerId, reqType);
			result = AI_AddSpendingRequest(unitId, playerId, reqType, priority, param);
		}
		return result;
	}

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

	const u32 Func_0x00433410 = 0x00433410;
	int Ai_HasBuildingsNeedingPower(u32 player) {
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

	const u32 Func_0x00433470 = 0x00433470;
	int Ai_CountEconomyUnits(u32 player, u32 unit_id) {
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

	const u32 Func_0x004889D0 = 0x004889D0;
	int getSuppliesRemaining(u8 player, u16 unit_id) {
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

	const u32 Func_4888c0 = 0x004888C0;
	int getSuppliesUsed(int byte_data, u8 player) {
		static u32 result;
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

	const u32 Func_AI_AddCostToNeeds = 0x00447DC0;
	s32 AI_AddCostToNeeds(s32 a1, s32 a2, s32 a3) {
		static s32 result;
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

	const u32 Func_AI_AddSpendingRequest = 0x00447980;
	s32 AI_AddSpendingRequest(s16 a1, s32 a2, char a3, char a4, void* a5) {
		static s32 result;
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
}