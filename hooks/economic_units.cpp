#include "hooks\economic_units.h"


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
		case UnitId::scv://0x7
			order = OrderId::BuildTerran;
			break;
		case UnitId::drone://0x29
			order = OrderId::DroneStartBuild;
			break;
		case UnitId::probe://0x40
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
			id = UnitId::command_center;
			break;
		case RaceId::Protoss:
			id = UnitId::nexus;
			break;
		case RaceId::Zerg:
			id = UnitId::hatchery;
			break;
		default:
			break;
		}
		return id;
	}
	bool is_townhall(u32 unit_id) {
		switch (unit_id) {
		case UnitId::command_center:
		case UnitId::nexus:
		case UnitId::hatchery://lairs and hives are not required
			return true;
		default:
			break;
		};
		return false;
	}
	bool is_terran_supply(u16 unit_id) {
		if (unit_id == UnitId::TerranSupplyDepot) {
			return true;
		}
		return false;
	}
	u32 get_worker_from_race(u32 race) {
		u32 id = 0xe4;
		switch (race) {
		case RaceId::Terran:
			id = UnitId::scv;
			break;
		case RaceId::Protoss:
			id = UnitId::probe;
			break;
		case RaceId::Zerg:
			id = UnitId::drone;
			break;
		default:
			break;
		}
		return id;
	}
	u32 get_supply_structure_from_race(u32 race) {
		u32 id = 0xe4;
		switch (race) {
		case RaceId::Terran:
			id = UnitId::supply_depot;
			break;
		case RaceId::Protoss:
			id = UnitId::pylon;
			break;
		case RaceId::Zerg:
			id = UnitId::overlord;
			break;
		default:
			break;
		}
		return id;
	}
	void Ai_BuildSupplies(AiTown* a1) {
		// 1
		u8 race; // al
		unsigned __int8 v2; // bl
		int v3; // esi
		unsigned __int16 workerUnit; // ax
		int v6; // eax
		signed int v7; // edi
		int v8; // eax
		unsigned int v9; // eax
		int v10; // ebx
		char v11; // al
		unsigned int v12; // eax
		u8 v13; // cl
		int v14; // eax
		u16 v15; // ecx
		unsigned int v16; // ecx
		unsigned int v17; // edi
		int v18; // [esp+0h] [ebp-Ch]
		signed int v19; // [esp+4h] [ebp-8h]
		u32 supplyUnit; // [esp+8h] [ebp-4h]
		
		//copied from hexrays, will be adjusted later
		if (a1)
		{
			v2 = a1->player;
			v3 = v2;
			race = playerTable[v2].race;
			v18 = v2;
			supplyUnit = get_supply_structure_from_race(race);
			workerUnit = get_worker_from_race(race);
			v8 = getSuppliesRemaining(v2, workerUnit);
			v7 = v8;
			v19 = v8;
			v6 = Ai_HasBuildingsNeedingPower(v2);
			if (v6)
			{
				v10 = supplyUnit;
				v9 = Ai_CountEconomyUnits(v3, supplyUnit);//int player, u16 unitId
			LABEL_21:
				v17 = v9 + 1;
				if ((!v19 || !(AIScriptController[v3].AI_Flags.isFarmsNotimingOn) || Ai_HasBuildingsNeedingPower(v3))
					&& v17 > Ai_CountEconomyUnits(v3, v10))
				{
					Ai_AddTownSpendReq(v18, v19 > 8 ? 120 : 140, v10, (void*)a1);
				}
				//used variables: v17, v9, v3, v10, v18, v19, a1

				return;
			}
			if (v7 < 13)
			{
				u8 v6byte;
				if (units_dat::GroupFlags[supplyUnit].isZerg) {
					v6byte = RaceId::Zerg;
				}
				else if(units_dat::GroupFlags[supplyUnit].isTerran){
					v6byte = RaceId::Terran;
				}
				else if (units_dat::GroupFlags[supplyUnit].isProtoss) {//original code don't had explicit protoss check
					v6byte = RaceId::Protoss;
				}
				v14 = getSuppliesUsed(v6, v2);
				v13 = playerTable[v2].race;
				v12 = v14 + 13;
				v15 = get_townhall_from_race(v13);
				v16 = units_dat::SupplyProvided[v15];
				if (v12 > v16)
				{
					v10 = supplyUnit;
					v9 = (v12 - v16) / units_dat::SupplyProvided[supplyUnit];
					goto LABEL_21;
				}
			}
		}
	}
}