#include "ai_orders.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>
/*
namespace {
	CUnit** getAllUnitsInBounds(Box32* a1);
	bool unitCanAttackTarget(CUnit* a1, CUnit* a2, int a3);
	void wpnHit(CUnit* a1, CUnit* a2, int a3, int a4);
	unsigned int getDistanceFast(int a1, int a2, int a3, int a4);
	byte getGroundWeapon(CUnit* a1);
}

namespace hooks {
	
	int askForHelp_AI(CUnit* a1, CUnit* a2, int a3) { // 0043ADA0
		CUnit* v3; // esi
		signed __int16 v4; // ax
		int v5; // eax
		int v6; // ecx
		CSprite* v7; // ecx
		CUnit** v8; // edi
		__int16 v9; // dx
		CUnit** v10; // eax
		bool v11; // eax
		CUnit* v12; // edi
		unsigned __int16 v13; // cx
		unsigned __int16 v14; // ax
		int v15; // eax
		int v16; // eax
		AI_Main* v17; // eax
		char v18; // al
		CUnit* v19; // esi
		AI_Main* v20; // ecx
		int v21; // edx
		CUnit* v22; // ebx
		CSprite* v23; // eax
		__int16 v24; // bx
		unsigned int v25; // eax
		int v26; // eax
		unsigned __int8 v27; // al
		int v28; // eax
		int result; // eax
		__int16 a1; // [esp+8h] [ebp-18h]
		__int16 v31; // [esp+Ah] [ebp-16h]
		unsigned int v32; // [esp+Ch] [ebp-14h]
		bool v33; // [esp+10h] [ebp-10h]
		int v34; // [esp+14h] [ebp-Ch]
		CUnit** v35; // [esp+18h] [ebp-8h]
		CUnit* x2; // [esp+1Ch] [ebp-4h]

		v3 = a1;
		int _id1 = a1->id;
		if (units_dat::GroupFlags[_id1].isBuilding)
			v4 = (AIScriptController[(unsigned __int8)a1->playerId].AI_Flags & 0x20) != 0 ? 288 : 576;	// 0x20 = CONTROLLER_IS_CAMPAIGN
		else
			v4 = (AIScriptController[(unsigned __int8)a1->playerId].AI_Flags & 0x20) != 0 ? 96 : 288;	// 0x20 = CONTROLLER_IS_CAMPAIGN
		v7 = a1->sprite;
		v9 = v7->position.y;
		v7 = (2^17-1) & v7->position.x | v7 & ((2^17-1) << 16);  // LOWORD(v7)
		v32 = ((2 ^ 17-1) & v4 + v9) << 16 | (v32 & (2 ^ 17-1)); // HIWORD(v32)
		v31 = v9 - v4;
		a1 = (__int16)v7 - v4;
		v32 = (2 ^ 17-1) & (v4 + (__int16)v7) | v32 & ((2^17-1) << 16); // LOWORD(v32)
		v10 = getAllUnitsInBounds((Box32*)&a1);
		v6 = (unsigned __int8)a1->playerId;
		v8 = v10;
		v5 = v6;
		v35 = v8;
		v34 = v6;
		v11 = !AIScriptController[v6].AI_DefenseBuild_GG
			&& !AIScriptController[v5].AI_DefenseBuild_AG
			&& !AIScriptController[v5].AI_DefenseBuild_GA
			&& !AIScriptController[v5].AI_DefenseBuild_AA;
		v12 = *v8;
		v33 = v11 == 0;
		if (v12)
		{
			while (1)
			{
				if (v12 != v3 && (unsigned __int8)v12->playerId == v6)
				{
					v13 = v12->id;
					if (!(units_dat::BaseProperty[v13] & UnitProperty::Worker)
						|| (v14 = a2->id, units_dat::BaseProperty[v14] & UnitProperty::Worker)
						|| !v33 && v14 == UnitId::ZergZergling)
					{
						if (v13 != UnitId::ProtossArbiter)
						{
							v15 = unitCanAttackTarget(a2, v12, 1) & (2^9-1) | v15 & ((2^25-1) << 8); // LOBYTE(v15)
							if (v15 || (v16 = (unitCanAttackTarget(v12, a2, 1) | v16 & ((2^25-1) << 8), v16) && !(v12->status & UnitStatus::Burrowed)) // LOBYTE(v16)
							{
								if (!a3)
								{
									v19 = a2;
									goto LABEL_32;
								}
								v17 = v12->CAIControl;
								if (v17)
								{
									if (v17->controlType == 4)
									{
										v18 = v17->cptn->captainType;
										if (v18 == 1 || v18 == 8 || v18 == 9 || v18 == 2)
										{
											v19 = a2;
											wpnHit(v12, a2, 0, 1);
										LABEL_32:
											v20 = v12->CAIControl;
											if (!v20)
												goto LABEL_47;
											if (v20->controlType != 1)
												goto LABEL_47;
											v21 = SLOWORD(v20->town);
											v23 = v19->sprite;
											v24 = v23->position.y;
											v23 = v23->position.x & (2^17-1) | v23 & ((2^17-1) << 16); // LOWORD(v23)
											x2 = (CUnit*)SHIWORD(v20->cptn);
											v25 = getDistanceFast((signed __int16)v23, (int)x2, v24, v21);
											v22 = v12->subunit;
											v32 = v25;
											x2 = v22;
											v26 = v22->isSubunit & (2^9-1) | v26 & ((2^25-1) << 8); // LOBYTE(v26)
											if (!v26)
											{
												x2 = v12;
												v22 = v12;
											}
											v27 = v12->status & UnitStatus::InAir ? units_dat::AirWeapon[(unsigned __int16)v22->id] : getGroundWeapon(v22);
											if (v27 != -126)
											{
												v28 = getUnitMaxWpnRange(x2, v27);
												if (v32 <= v28 + 192)
													LABEL_47:
												wpnHit(v12, v19, 0, 1);
											}
											goto LABEL_42;
										}
									}
								}
							}
						}
					}
				}
			LABEL_42:
				v12 = v35[1];
				++v35;
				if (!v12)
				{
					result = tempUnitsListArraysCountsListLastIndex[0] - 1;
					tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsListLastIndex[tempUnitsListArraysCountsListLastIndex[0]--];
					return result;
				}
				v6 = v34;
				v3 = a1;
			}
		}
		result = tempUnitsListArraysCountsListLastIndex[0] - 1;
		tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsListLastIndex[tempUnitsListArraysCountsListLastIndex[0]--];
		return result;
	}
}

namespace {
	const u32 Func_getAllUnitsInBounds = 0x0042FF80;
	CUnit** getAllUnitsInBounds(Box32* a1) {
		static Box32* a1;
		static CUnit** result;

		__asm {
			PUSHAD
			MOV a1, EAX
			CALL Func_getAllUnitsInBounds
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_unitCanAttackTarget = 0x00476730;
	bool unitCanAttackTarget(CUnit* a1, CUnit* a2, int a3) {
		static CUnit* a1;
		static CUnit* a2;
		static int a3;
		static bool result;

		__asm {
			PUSHAD
			PUSH a3
			MOV EBX, a1
			MOV ESI, a2
			CALL Func_unitCanAttackTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_wpnHit = 0x0043F320;
	void wpnHit(CUnit* a1, CUnit* a2, int a3, int a4) {
		static CUnit* a1;
		static CUnit* a2;
		static int a3;
		static int a4;
		
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_wpnHit
			POPAD
		}
	}

	const u32 Func_getDistanceFast = 0x0040C360;
	unsigned int getDistanceFast(int a1, int a2, int a3, int a4) {
		static int a1;
		static int a2;
		static int a3;
		static int a4;
		static unsigned int result;

		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_getDistanceFast
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_getGroundWeapon = 0x00475AD0;
	byte getGroundWeapon(CUnit* a1) {
		static CUnit* a1;
		static byte result;

		__asm {
			PUSHAD
			PUSH a1
			CALL Func_getGroundWeapon
			MOV result, EAX
			POPAD
		}
		return result;
	}
}*/