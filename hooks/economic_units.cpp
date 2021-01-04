#include "hooks\economic_units.h"

namespace {

	// For Ai_TryProgressSpendingQueue_Worker
	signed int isUnitTypeRaceUnitRace(unsigned __int16 a1, CUnit* a2); // 00432480
	signed int techtreeAllowed(unsigned __int16 a1, CUnit* a2, int a3); // 0046E1C0
	bool hasResourcesForUnit(unsigned __int16 a1, int a2); // 004487B0
	int getPlayerDefaultRefineryUnitType(int a1); // 004320D0
	signed int getBunkerPosition(int a1, Point32* a2); // 004376F0
	__int16 getRegionIdFromPixels(signed int a1, signed int a2); // 0049C9F0
	void doAreaFixupsForEnemy(int a1, unsigned __int16 a2, unsigned int a3); // 00444EB0
	signed int AiManageBuildingPlacement(CUnit* a1, unsigned int a2, Point32* a3, Point32* a4, signed int a5); // 004464C0
	int AiFinishUnit(CUnit* a1); // 0000433640
	void AiAssignMilitary(int a1, AiCaptain* a2); // 004390A0
	signed int getTurretPosition(int a1, Point32* a2); // 00437670
	void getSecondHatcheryBuildLocation(int a1, __int16 a2, __int16 a3); // 00444630
	CUnit* getAiRefinery(AiTown* a1, int a2, int a3); // 00432B90
	__int16 AiReserve(int a1, int a2); // 00447D00
	AI_Main* AiPopSpendingRequest(int a1); // 004476D0
	void orderTarget(CUnit* a1, unsigned __int8 a2, __int16 a3, __int16 a4); // 00475260
	int refundAllQueueSlots(CUnit* a1); // 00466E80
	bool hasMoneyCanMake(CUnit* a1, unsigned __int16 a2); // 00467250
}

const u32 Func_0x00433410 = 0x00433410;
int Ai_HasBuildingsNeedingPower(u32 player) {
	static int result;
	__asm {
		PUSHAD
		MOV EAX,player
		CALL Func_0x00433410
		MOV result,EAX
		POPAD
	}
	return result;
}

const u32 Func_0x00433470 = 0x00433470;
int Ai_CountEconomyUnits(u32 player, u32 unit_id) {
	static int result;
	__asm {
		PUSHAD
		MOV EAX,player
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
		MOV BL,player
		MOV DI,unit_id
		CALL Func_0x004889D0
		MOV result,EAX
		POPAD
	}
	return result;
}

const u32 Func_00448400 = 0x00448400;
void Ai_AddTownSpendReq(u32 player, u32 priority, u32 unitId, void* param) {
	__asm {
		PUSHAD
		MOV EAX, player
		MOV ESI, unitId
		MOV EDI, priority
		PUSH param
		CALL Func_00448400
		POPAD
	}
}
//v6byte (eax), player (cl)

const u32 Func_4888c0 = 0x004888c0;
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
					Ai_AddTownSpendReq(player_, getSuppliesRemaining(player_, get_townhall_from_race(RaceId::Protoss)) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Protoss, player_), (void*)town);
				}
			}
			if (hasZerg_) {
				if (checkSupplyNeed(player_, RaceId::Zerg)) {
					Ai_AddTownSpendReq(player_, getSuppliesRemaining(player_, get_townhall_from_race(RaceId::Zerg)) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Zerg, player_), (void*)town);
				}
			}
			if (hasTerran_) {
				if (checkSupplyNeed(player_, RaceId::Terran)) {
					Ai_AddTownSpendReq(player_, getSuppliesRemaining(player_, get_townhall_from_race(RaceId::Terran)) > 8 ? 120 : 140, get_supply_structure_from_race(RaceId::Terran, player_), (void*)town);
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
				if ((!supplyRemaining_) && Ai_CountEconomyUnits(player_, supplyUnit_) * units_dat::SupplyProvided[supplyUnit_] + townHallSupply_ < getSuppliesUsed(race_, player_)) {
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
	
/*	signed int Ai_TryProgressSpendingQueue_Worker(AiTown* a1, CUnit* a2) {
		int v2; // eax
		CUnit* pUnit; // edi
		UnitAi* aiCtrl; // edx
		int ownr; // ecx
		unsigned int _buildType; // ebx
		unsigned __int16 buildType; // ax
		int v9; // edx
		AiTown* _pTown; // esi
		CUnit* v11; // eax
		CSprite* v12; // eax
		__int16 v13; // dx
		__int16 _ptwnPos_Y; // dx
		AiTown* __pTown; // esi
		unsigned __int16 rgnIdAtPos; // ax
		int v17; // edi
		AiCaptain* v18; // esi
		pathing* v19; // edx
		int v20; // ecx
		__int16 v21; // dx
		__int16 v22; // ax
		__int16 v23; // cx
		unsigned __int16 v24; // ax
		signed int v25; // eax
		int v26; // eax
		int v27; // esi
		unsigned __int8 v28; // al
		unsigned int v29; // [esp-18h] [ebp-28h]
		AiTown* v30; // [esp-14h] [ebp-24h]
		signed int v31; // [esp-Ch] [ebp-1Ch]
		int nTownOwner; // [esp+4h] [ebp-Ch]
		unsigned int __buildType; // [esp+8h] [ebp-8h]
		Point32 _pos; // [esp+Ch] [ebp-4h]

		pUnit = a2;
		v2 = (unsigned __int8)a2->playerId;
		if (!AIScriptController[v2].unknown_0x210 || AIScriptController[v2].newBuildType != 3) // Construct morph
			return 0;
		aiCtrl = (AI_Main*)&a2->pAI;
		ownr = (unsigned __int8)a2->playerId;
		if (AIScriptController[ownr].pTownMain != aiCtrl->pTownMain
			&& AIScriptController[ownr].nextBuildType != UnitId::TerranSupplyDepot)
		{
			return 0;
		}
		if (aiCtrl->unkA >= 3u)
			return 0;
		buildType = AIScriptController[v2].nextBuildType;
		_buildType = buildType;
		__buildType = buildType;
		if (!isUnitTypeRaceUnitRace(buildType, a2)
			|| techtreeAllowed(_buildType, pUnit, (unsigned __int8)pUnit->playerId) != 1
			|| !hasResourcesForUnit(_buildType, (unsigned __int8)pUnit->playerId))
		{
			return 0;
		}
		_pTown = a1;
		nTownOwner = (unsigned __int8)a1->player;
		if (_buildType != getPlayerDefaultRefineryUnitType(nTownOwner))
		{
			if (_buildType == UnitId::TerranBunker)
			{
				if (!getBunkerPosition(v9, &_pos))
				{
					_ptwnPos_Y = a1->position.y;
					_pos.x = a1->position.x;
					_pos.y = _ptwnPos_Y;
				}
				rgnIdAtPos = getRegionIdFromPixels(_pos.x, _pos.y);
				__pTown = a1;
				doAreaFixupsForEnemy((unsigned __int8)a1->player, rgnIdAtPos, 1u);
				pUnit = a2;
				if (!AIScriptController[(unsigned __int8)a2->playerId].builtSomething)
				{
					if (!AiManageBuildingPlacement(a2, UnitId::TerranBunker, _pos, &_pos, 64))
					{
						AIScriptController[(unsigned __int8)pUnit->playerId].builtSomething = 1;
						return AiFinishUnit(pUnit);
					}
					v17 = getRegionIdFromPixels(_pos.x, _pos.y);
					v18 = &AiRegionCaptains[(unsigned __int8)__pTown->player][v17];
					if (!v18->captainType)
					{
						AiAssignMilitary(4, v18);
						v19 = SAI_Paths;
						v20 = (unsigned __int16)v17;
						pUnit = a2;
						v18->captainFlags |= 0x41u;
						LOWORD(_buildType) = __buildType;
						if (v19->regions[v20].accessabilityFlags != 0x1FFD)
							v18->field_C = 1000;
						goto LABEL_44;
					}
					pUnit = a2;
				LABEL_43:
					LOWORD(_buildType) = __buildType;
				LABEL_44:
					_pTown = a1;
					goto LABEL_45;
				}
			}
			else if (_buildType == UnitId::TerranMissileTurret)
			{
				if (!getTurretPosition(v9, (Point32*)&a2))
				{
					v21 = a1->position.y;
					LOWORD(a2) = a1->position.x;
					HIWORD(a2) = v21;
				}
				if (!AIScriptController[(unsigned __int8)pUnit->playerId].builtSomething)
				{
					if (!AiManageBuildingPlacement(pUnit, UnitId::TerranMissileTurret, (Point32*)a2, &_pos, 40))
					{
						AIScriptController[(unsigned __int8)pUnit->playerId].builtSomething = 1;
						return AiFinishUnit(pUnit);
					}
					goto LABEL_43;
				}
			}
			else
			{
				if (_buildType == UnitId::ZergHatchery && _pTown->field_24)
				{
					getSecondHatcheryBuildLocation((unsigned __int8)_pTown->resource_area, (__int16)&a1, (__int16)&a1 + 1);
					if (AIScriptController[(unsigned __int8)pUnit->playerId].builtSomething)
						return AiFinishUnit(pUnit);
					v31 = 64;
					v30 = a1;
					v29 = UnitId::ZergHatchery;
				}
				else
				{
					v22 = _pTown->position.x;
					v23 = _pTown->position.y;
					_pos.x = _pTown->position.x;
					_pos.y = v23;
					if (_buildType == UnitId::ZergCreepColony || _buildType == UnitId::ProtossPhotonCannon)
					{
						v24 = getRegionIdFromPixels(v22, v23);
						doAreaFixupsForEnemy(nTownOwner, v24, 4u);
						pUnit = a2;
						_buildType = __buildType;
						_pTown = a1;
					}
					v25 = 64;
					if (AIScriptController[(unsigned __int8)pUnit->playerId].builtSomething)
						return AiFinishUnit(pUnit);
					if ((__int16)_buildType == UnitId::TerranMissileTurret)
						v25 = 40;
					v31 = v25;
					v30 = (AiTown*)&_pos;
					v29 = _buildType;
				}
				if (AiManageBuildingPlacement(pUnit, v29, (Point32*)&v30, &_pos, v31))
					goto LABEL_45;
				AIScriptController[(unsigned __int8)pUnit->playerId].builtSomething = 1;
			}
			return AiFinishUnit(pUnit);
		}
		v11 = getAiRefinery(_pTown, 1, 1);
		if (!v11)
			return 0;
		v12 = v11->sprite;
		v13 = v12->position.x;
		LOWORD(v12) = v12->position.y;
		_pos.x = v13;
		_pos.y = (signed __int16)v12;
	LABEL_45:
		v27 = (unsigned __int8)_pTown->player;
		AiReserve(v27, 1);
		AiPopSpendingRequest(v27);
		v26 = (unsigned __int16)pUnit->id;
		switch (v26)
		{
		case UnitId::TerranSCV:
			v28 = OrderId::BuildTerran;
			goto doBuild;
		case UnitId::ZergDrone:
			v28 = OrderId::DroneBuild;
			goto doBuild;
		case UnitId::ProtossProbe:
			v28 = OrderId::BuildProtoss1;
		doBuild:
			orderTarget(pUnit, v28, _pos.x, _pos.y);
			refundAllQueueSlots(pUnit);
			hasMoneyCanMake(pUnit, _buildType);
			return 1;
		}
		return 0;
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