#include "target_related.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "byte_operations_utils.h"

namespace {
	// for isUnitTargetOutOfMaxRange
	unsigned int isTargetVisible(CUnit* a1, CUnit* a2);
	int getUnitMaxWeaponRange(CUnit* a1, unsigned __int8 a2);
	bool isTargetWithinMinMovementRange(CUnit* a1, unsigned int a2, CUnit* a3);

	// for isReadyToAttack
	unsigned int getDistanceToUnit(CUnit* a1, CUnit* a2);
	unsigned int distanceFromDboxCorner(CUnit* a1, __int16 a2, __int16 a3);
	bool isAngleInAttackAngle(int a1, int a2, CUnit* a3, unsigned __int8 a4);

	// for canCastOrderedSpellNow
	int isUnitVisible(CUnit* a1);
	bool isTargetWithinMinRange(CUnit* a1, unsigned int a2, CUnit* a3);
	int getFlingyHaltDistance(int a1);
	unsigned int getDistanceFast(int a1, int a2, int a3, int a4);

	// for ordersWatchTarget
	bool unitCanAttackTarget(CUnit* a1, CUnit* a2, int a3);
	char getRightClickActionOrder(CUnit* a1);
	void unknownOrdersFunc(int a1, __int16 a2, __int16 a3);

	// for ordersStayInRange
	void prependOrderTargetingGround(__int8 a1, __int16 a2, __int16 a3, int a4);
	void toIdle(CUnit* a1);
	signed int moveTowards(CUnit* a1, CUnit* a2);
	int ordersHoldPositionSuicidal(CUnit* a1);
	signed int moveToTarget(CUnit* a1, CUnit* a2);
	int getDirectionFromPoints(int a1, int a2, int a3, int a4);
	CUnit* setSecondaryWaypoint(CUnit* a1, int a2, int a3);
	Point32 setResourceTarget(CUnit* a1);
	void ordersNothing2(CUnit* a1);
	void orderComputerClear(CUnit* a1, __int8 a2);

	// for genericRightClickUnitProc2
	bool unitIsActiveTransport(CUnit* unit);											// 004E6BA0
	int32 isOrderValid(char a1, CUnit* a2, int32 a3, CUnit* a4, int32 a5);					// 00474D90
	bool canEnterTransport(CUnit* a1, CUnit* a2);										// 004E6E00
	uint16 getUnitAbilityErrorStatString(CUnit* a1, char a2, int32 a3);						// 00491E80

	// for CMDRECV_HoldPosition
	CUnit* getActivePlayerFirstSelection();												// 0049A850
	int32 orderAllowed(CUnit* a1, uint16 a2, int32 a3);										// 0046DC20
	void targetedOrder(int16 a1, int32 a2, CUnit* a3, int16 a4, int32 a5, int32 a6, char a7, int32 a8, int32 a9, int32 a10);	// 004754F0
}

namespace hooks {
	bool isUnitTargetOutOfMaxRange(CUnit* a1, CUnit* a2) { // 00476430
		CUnit* _unit; // edi
		CUnit* finalUnit; // esi
		unsigned __int8 _wpn; // bl
		__int16 _id; // ax
		int _maxRange; // eax
		int v8; // eax

		_unit = a1;
		if (!isTargetVisible(a2, a1))
			return 1;
		finalUnit = _unit->subunit;
		if (!finalUnit || !(units_dat::GroupFlags[(unsigned __int16)finalUnit->id].isBuilding))
			finalUnit = _unit;
		if (a2->status & UnitStatus::InAir) {
			_wpn = units_dat::AirWeapon[(unsigned __int16)finalUnit->id];
		} else {
			_id = finalUnit->id;
			if (_id != UnitId::ZergLakizilisk || finalUnit->status & 0x10)
				_wpn = units_dat::GroundWeapon[_id];
			else
				_wpn = -126;
		}
		if (_wpn == -126)
			return 0;
		_maxRange = getUnitMaxWeaponRange(_unit, _wpn);

		ByteOperations::SetLowByte(v8, isTargetWithinMinMovementRange(a2, _maxRange, finalUnit));
		return v8 == 0;
	}

	signed int isReadyToAttack(CUnit* a1, unsigned __int8 a2) { // 00476640
		CUnit* v2; // esi
		CUnit* v4; // ecx
		unsigned int v5; // eax
		unsigned int v6; // eax
		unsigned int v7; // edi
		__int16 v8; // cx

		v2 = a1;
		if (a1->movementFlags & 8)
			return 0;
		v4 = a1->orderTarget.unit;
		if (v4)
			v5 = getDistanceToUnit(a1, v4);
		else
			v5 = distanceFromDboxCorner(a1, a1->orderTarget.pt.x, a1->orderTarget.pt.y);
		v7 = v5;
		v6 = weapons_dat::MinRange[a2];
		if (v6 && v7 < v6 || v7 > getUnitMaxWeaponRange(v2, a2) || v2->pathingFlags & 2)
			return 0;
		v8 = v2->nextTargetWaypoint.x;
		if ((v8 != v2->position.x || v2->nextTargetWaypoint.y != v2->position.y)
			&& !isAngleInAttackAngle(v2->nextTargetWaypoint.y, v8, v2, a2)) {
			v2->orderQueueTimer = 0;
			return 0;
		}
		return 1;
	}

	bool isUnitInWeaponRange(CUnit* a1, CUnit* a2) { // 00476870
		CUnit* v2; // edi
		CUnit* v4; // esi
		unsigned __int8 v5; // bl
		unsigned __int16 v6; // ax
		unsigned int v7; // eax
		int v8; // eax

		v2 = a1;
		if (!a1) {
			v2 = a2->orderTarget.unit;
			if (!v2)
				return 1;
		}
		if (!isTargetVisible(v2, a2))
			return 0;
		v4 = a2->subunit;
		if (!v4 || !(units_dat::GroupFlags[(unsigned __int16)v4->id].isBuilding))
			v4 = a2;
		if (v2->status & UnitStatus::InAir) {
			v5 = units_dat::AirWeapon[(unsigned __int16)v4->id];
		} else {
			v6 = v4->id;
			if (v6 != UnitId::ZergLakizilisk || v4->status & 0x10)
				v5 = units_dat::GroundWeapon[v6];
			else
				v5 = -126;
		}
		if (v5 >= (unsigned __int8)0x82)
			return 0;
		v7 = weapons_dat::MinRange[v5];
		if (v7) {
			if (isTargetWithinMinRange(v4, v7, v2))
				return 0;
		}
		v8 = getUnitMaxWeaponRange(a2, v5);
		return isTargetWithinMinMovementRange(v2, v8, v4);
	}

	char canCastOrderedSpellNow(CUnit* a1) { // 00491DB0
		CUnit* v1; // esi
		unsigned int v2; // eax
		CUnit* v3; // edi
		int v4; // ebx
		int v5; // eax
		int v6; // edi
		int v7; // ST14_4
		int v8; // ebx
		unsigned int v9; // ebx

		v1 = a1;
		v2 = isUnitVisible(a1);
		if (v2) {
			v3 = v1->orderTarget.unit;
			if (v3) {
				v4 = getUnitMaxWeaponRange(v1, orders_dat::OrderWeaponId[(unsigned __int8)v1->mainOrderId]);
				if (v1->movementFlags & 2) {
					if (!(v3->movementFlags & 2))
						goto LABEL_14;
					v5 = (unsigned __int8)(v3->velocityDirection1 - v1->velocityDirection1);
					if (v5 > 128)
						v5 = 256 - v5;
					if (v5 > 32)
						LABEL_14:
					v4 += getFlingyHaltDistance((int)v1) >> 8;
				}
				//ByteOperations::SetLowByte((UINT32)v2, (UINT8)isTargetWithinMinRange(v1, v4, v3)); // Needs fixing
			} else {
				v6 = v1->orderTarget.pt.y;
				v7 = v1->orderTarget.pt.x;
				v8 = getUnitMaxWeaponRange(v1, orders_dat::OrderWeaponId[(unsigned __int8)v1->mainOrderId]);
				v9 = (getFlingyHaltDistance((int)v1) >> 8) + v8;
				v2 = v9 >= getDistanceFast(v1->halt.x, v7 << 8, v1->halt.y, v6 << 8) >> 8;
			}
		}
		return v2;
	}

	void ordersWatchTarget(CUnit* a1) { // 0047BAB0
		CUnit* v1; // esi
		CUnit* v2; // ebx
		int v3; // eax
		CUnit* v4; // ebx
		CUnit* v5; // edi
		unsigned __int8 v6; // bl
		unsigned __int16 v7; // ax
		unsigned int v8; // eax
		int v9; // eax
		int v10; // eax
		CUnit* target; // [esp+Ch] [ebp-4h]

		v1 = a1;
		v2 = a1->orderTarget.unit;
		if (!v2)
			goto LABEL_22;
		ByteOperations::SetLowByte(v3, unitCanAttackTarget(v2, a1, 1));
		if (!v3)
			goto LABEL_22;
		v4 = v1->orderTarget.unit;
		target = v1->orderTarget.unit;
		if (v4) {
			if (!isTargetVisible(v4, v1))
				goto LABEL_23;
			v5 = v1->subunit;
			if (!v5 || !(units_dat::GroupFlags[(unsigned __int16)v5->id].isBuilding))
				v5 = v1;
			if (v4->status & 4) {
				v6 = units_dat::AirWeapon[(unsigned __int16)v5->id];
			} else {
				v7 = v5->id;
				if (v7 != 103 || v5->status & 0x10)
					v6 = units_dat::GroundWeapon[v7];
				else
					v6 = 0x82;
			}
			if (v6 >= (unsigned __int8)0x82
				|| (v8 = weapons_dat::MinRange[v6]) != 0 && isTargetWithinMinRange(v5, v8, target)
				|| (v9 = getUnitMaxWeaponRange(v1, v6), ByteOperations::SetLowByte(v10, (isTargetWithinMinMovementRange(target, v9, v5), !v10)))) {
			LABEL_23:
				if (getRightClickActionOrder(v1) == 3)
					LABEL_22 :
					unknownOrdersFunc((int)v1, v1->orderTarget.pt.x, v1->orderTarget.pt.y);
			}
		}
	}

	void ordersStayInRange(CUnit* a1) { // 0047C4F0
		CUnit* v1; // esi
		CUnit* v2; // ebx
		int v3; // eax
		CUnit* v4; // ebx
		CUnit* v5; // edi
		unsigned __int8 v6; // bl
		unsigned __int16 v7; // ax
		unsigned int v8; // eax
		int v9; // eax
		int v10; // eax
		CUnit* v11; // edi
		unsigned __int8 v12; // bl
		unsigned __int16 v13; // ax
		unsigned int v14; // eax
		int v15; // eax
		int v16; // eax
		__int16 v17; // ax
		int v18; // edi
		int v19; // ebx
		char v20; // al
		int v21; // ecx
		int v22; // eax
		CUnit* target; // [esp+8h] [ebp-4h]

		v1 = a1;
		v2 = a1->orderTarget.unit;
		if (v2) {
			a1->orderTarget.pt.x = v2->sprite->position.x;
			a1->orderTarget.pt.y = v2->sprite->position.y;
			ByteOperations::SetLowByte(v3, unitCanAttackTarget(v2, a1, 1));
			if (!v3) {
				if (!(v1->status & 0x8000000) && !v1->pAI)
					prependOrderTargetingGround(6u, v1->orderTarget.pt.x, v1->orderTarget.pt.y, (int)v1);
				toIdle(v1);
				return;
			}
			v4 = v1->orderTarget.unit;
			target = v1->orderTarget.unit;
			if (v1->mainOrderState) {
				if (!v4)
					goto LABEL_62;
				if (!isTargetVisible(v4, v1))
					goto LABEL_63;
				v11 = v1->subunit;
				if (!v11 || !(units_dat::GroupFlags[(unsigned __int16)v11->id].isBuilding))
					v11 = v1;
				if (v4->status & 4) {
					v12 = units_dat::AirWeapon[(unsigned __int16)v11->id];
				} else {
					v13 = v11->id;
					if (v13 != UnitId::ZergLakizilisk || v11->status & 0x10)
						v12 = units_dat::GroundWeapon[v13];
					else
						v12 = 0x82;
				}
				if (v12 >= (unsigned __int8)0x82
					|| (v14 = weapons_dat::MinRange[v12]) != 0 && isTargetWithinMinRange(v11, v14, target)
					|| (v15 = getUnitMaxWeaponRange(v1, v12), ByteOperations::SetLowByte(v16, (isTargetWithinMinMovementRange(target, v15, v11), !v16)))) {
				LABEL_63:
					if (!moveTowards(v1->orderTarget.unit, v1))
						return;
				} else {
				LABEL_62:
					ordersHoldPositionSuicidal(v1);
				}
				goto LABEL_42;
			}
			if (!v4)
				goto LABEL_64;
			if (isTargetVisible(v4, v1)) {
				v5 = v1->subunit;
				if (!v5 || !(units_dat::GroupFlags[(unsigned __int16)v5->id].isBuilding))
					v5 = v1;
				if (v4->status & 4) {
					v6 = units_dat::AirWeapon[(unsigned __int16)v5->id];
				} else {
					v7 = v5->id;
					if (v7 != 103 || v5->status & 0x10)
						v6 = units_dat::GroundWeapon[v7];
					else
						v6 = 0x82;
				}
				if (v6 < (unsigned __int8)0x82) {
					v8 = weapons_dat::MinRange[v6];
					if (!v8 || !isTargetWithinMinRange(v5, v8, target)) {
						v9 = getUnitMaxWeaponRange(v1, v6);
						ByteOperations::SetLowByte(v10, isTargetWithinMinMovementRange(target, v9, v5));
						if (v10)
							goto LABEL_64;
					}
				}
			}
			if (moveToTarget(v1->orderTarget.unit, v1)) {
			LABEL_64:
				v1->mainOrderState = 1;
			LABEL_42:
				v17 = v1->id;
				if ((v17 == 3 || v17 == 17) && !(v1->movementFlags & 2)) {
					v18 = v1->orderTarget.pt.y;
					v19 = v1->orderTarget.pt.x;
					v20 = getDirectionFromPoints(v18, v1->sprite->position.x, v19, v1->sprite->position.y) - v1->currentDirection1;
					if (v20 >= 128 || v20 <= -128)
						v20 = -128;
					if (abs(v20) >= 32)
						setSecondaryWaypoint(v1, v19, v18);
				}
				return;
			}
		} else {
			setResourceTarget(a1);
			v21 = v1->moveTarget.pt.x;
			v22 = v1->moveTarget.pt.y;
			if (v21 != v1->nextTargetWaypoint.x || v22 != v1->nextTargetWaypoint.y) {
				v1->nextTargetWaypoint.x = v21;
				v1->nextTargetWaypoint.y = v22;
			}
			if (v1->orderQueueHead) {
				v1->userActionFlags |= 1u;
				ordersNothing2(v1);
			} else if (v1->pAI) {
				orderComputerClear(v1, 0x9Cu);
			} else {
				orderComputerClear(v1, units_dat::ReturnToIdleOrder[(unsigned __int16)v1->id]);
			}
		}
	}

	int32 genericRightClickUnitProc2(CUnit* unit, int32 a2, int16 a3, CUnit* target, int16 unitID) { // 00455E00
		int32 result; // eax
		CUnit* targetVar; // esi

		if (unit->status & UnitStatus::GroundedBuilding) {
			if (unit->isFactory())
				result = OrderId::RallyPoint2 - (target != 0);
			else
				result = OrderId::Nothing2;
			return result;
		}
		targetVar = target;
		if (!target) {
			if ((unit->id == UnitId::ZergMatraleth) && unitID != UnitId::None)
				return OrderId::Rightclickaction;
			return OrderId::Move;
		}
		if (unitIsActiveTransport(unit) && isOrderValid(OrderId::Pickup2, unit, 0, targetVar, 0))
			return OrderId::Pickup2;
		if (unitIsActiveTransport(targetVar) && canEnterTransport(targetVar, unit))
			return OrderId::EnterTransport;
		if (targetVar->status & UnitStatus::Burrowed)
			return OrderId::Move;
		if (unit->id != UnitId::ZergMatraleth
			|| targetVar->status & UnitStatus::Invincible
			|| getUnitAbilityErrorStatString(targetVar, unit->playerId, 12) != 0) {
			result = (uint8)(unit->id != UnitId::TerranCleric ? 0 : 0x80) + OrderId::Follow;
		} else {
			result = OrderId::InfestMine2;
		}
		return result;
	}

	CUnit* CMDRECV_HoldPosition(int32 a1) { // 004C20C0
		CUnit* result; // eax
		CUnit* i; // esi
		int16 unitId; // ax
		uint8 orderId; // al

		*((int8*)&mapHeightPixels + 2) = 0;
		result = getActivePlayerFirstSelection();
		for (i = result; result; i = result) {
			unitId = i->id;
			if (unitId != UnitId::ProtossSolarion) {
				if (unitId != UnitId::ProtossAccantor) {
					if (unitId != UnitId::ZergMatraleth) {
						if (unitId != UnitId::ZergSkryling && unitId != UnitId::ZergInfestedTerran)
							orderId = unitId != UnitId::TerranCleric ? 107 : 178;
						else
							orderId = OrderId::HoldPosition4;
					} else {
						orderId = OrderId::HoldPosition3;
					}
				} else {
					orderId = OrderId::ReaverHold;
				}
			} else {
				orderId = OrderId::HoldPosition1;
			}
			if (orderAllowed(i, orderId, *ACTIVE_NATION_ID) == 1)
				targetedOrder(0, UnitId::None, i, 0, 1, *(int8*)(a1 + 1) != 0, orderId, 0, *orderType4Position, *orderType4Target);
			result = getActivePlayerFirstSelection();
		}
		return result;
	}

	uint32 unitCanSeeCloakedTarget(CUnit* target, CUnit* unit) { // 00401D60, hooked for cloak rewrite - Pr0nogo
		const bool isCloaked = target->status & 0x300 && !((1 << unit->playerId) & target->visibilityStatus); // original condition
		const bool isDecloaked = scbw::get_aise_value(unit, NULL, ValueId::PermanentCloakTimer, 0, 0) > 1;	  // temp decloak
		return isCloaked && !isDecloaked;
		//u32 flags = target->status; // BYTE1() & 3 == check if unit is cloaked/requires detect
		//return flags & 0x300 && !((1 << unit->playerId) & target->visibilityStatus);
	}
}

namespace {
	// for isUnitTargetOutOfMaxRange
	const uint32 Func_isTargetVisible = 0x004E5E30;
	unsigned int isTargetVisible(CUnit* a1, CUnit* a2) {
		static unsigned int result;

		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ESI, a2
			CALL Func_isTargetVisible
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_getUnitMaxWeaponRange = 0x00475870;
	int getUnitMaxWeaponRange(CUnit* a1, unsigned __int8 a2) {
		static int result;

		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_getUnitMaxWeaponRange
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_isTargetWithinMinMovementRange = 0x004763D0;
	bool isTargetWithinMinMovementRange(CUnit* a1, unsigned int a2, CUnit* a3) {
		static int result;
		static bool result2;

		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			MOV EDI, a3
			CALL Func_isTargetWithinMinMovementRange
			MOV result, EAX
			POPAD
		}
		return result2 = (bool)result;
	};

	// for isReadyToAttack
	const uint32 Func_getDistanceToUnit = 0x00430D30;
	unsigned int getDistanceToUnit(CUnit* a1, CUnit* a2) {
		static unsigned int result;

		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_getDistanceToUnit
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_distanceFromDboxCorner = 0x0043E50;
	unsigned int distanceFromDboxCorner(CUnit* a1, __int16 a2, __int16 a3) {
		static unsigned int result;

		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_distanceFromDboxCorner
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_isAngleInAttackAngle = 0x00475BE0;
	bool isAngleInAttackAngle(int a1, int a2, CUnit* a3, unsigned __int8 a4) {
		static int result;
		static bool result2;

		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV ESI, a3
			CALL Func_isAngleInAttackAngle
			MOV result, EAX
			POPAD
		}
		return result2 = (bool)result;
	};

	// for canCastOrderedSpellNow
	const uint32 Func_isUnitVisible = 0x004E5DB0;
	int isUnitVisible(CUnit* a1) {
		static int result;

		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_isUnitVisible
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_isTargetWithinMinRange = 0x00430F10;
	bool isTargetWithinMinRange(CUnit* a1, unsigned int a2, CUnit* a3) {
		static int result;
		static bool result2;

		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_isTargetWithinMinRange
			MOV result, EAX
			POPAD
		}
		return result = (bool)result;
	};

	const uint32 Func_getFlingyHaltDistaince = 0x00494F90;
	int getFlingyHaltDistance(int a1) {
		static int result;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_getFlingyHaltDistaince
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_getDistanceFast = 0x0040C360;
	unsigned int getDistanceFast(int a1, int a2, int a3, int a4) {
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
	};

	// for ordersWatchTarget
	const uint32 Func_unitCanAttackTarget = 0x00476730;
	bool unitCanAttackTarget(CUnit* a1, CUnit* a2, int a3) {
		static int result;
		static bool result2;

		__asm {
			PUSHAD
			PUSH a3
			MOV EBX, a1
			MOV ESI, a2
			CALL Func_unitCanAttackTarget
			MOV result, EAX
			POPAD
		}
		return result2 = (bool)result;
	};

	const uint32 Func_getRightClickActionOrder = 0x004E5EA0;
	char getRightClickActionOrder(CUnit* a1) {
		static int result;
		static char result2;

		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_getRightClickActionOrder
			MOV result, EAX
			POPAD
		}
		return result2 = (char)result;
	};

	const uint32 Func_unknownOrdersFunc = 0x00475350;
	void unknownOrdersFunc(int a1, __int16 a2, __int16 a3) {

		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EAX, a1
			CALL Func_unknownOrdersFunc
			POPAD
		}
	};

	// for ordersStayInRange
	const uint32 Func_prependOrderTargetingGround = 0x00474A20;
	void prependOrderTargetingGround(__int8 a1, __int16 a2, __int16 a3, int a4) {

		__asm {
			PUSHAD
			PUSH a4
			MOVZX CX, a3
			MOVZX DX, a2
			MOVZX AL, a1
			CALL Func_prependOrderTargetingGround
			POPAD
		}
	};

	const uint32 Func_toIdle = 0x004753A0;
	void toIdle(CUnit* a1) {

		__asm {
			PUSHAD
			PUSH a1
			CALL Func_toIdle
			POPAD
		}
	};

	const uint32 Func_moveTowards = 0x004EB900;
	signed int moveTowards(CUnit* a1, CUnit* a2) {
		static signed int result;

		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_moveTowards
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_ordersHoldPositionSuicidal = 0x004EB5B0;
	int ordersHoldPositionSuicidal(CUnit* a1) {
		static int result;

		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_ordersHoldPositionSuicidal
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_moveToTarget = 0x004EB720;
	signed int moveToTarget(CUnit* a1, CUnit* a2) {
		static signed int result;

		__asm {
			PUSHAD
			MOV EDI, a1
			MOV ESI, a2
			CALL Func_moveToTarget
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_getDirectionFromPoints = 0x00495300;
	int getDirectionFromPoints(int a1, int a2, int a3, int a4) {
		static int result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV EDX, a2
			MOV ECX, a3
			CALL Func_getDirectionFromPoints
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_setSecondaryWaypoint = 0x00494BB0;
	CUnit* setSecondaryWaypoint(CUnit* a1, int a2, int a3) {
		static CUnit* result;

		__asm {
			PUSHAD
			MOV ECX, a3
			MOV EDX, a2
			MOV EAX, a1
			CALL Func_setSecondaryWaypoint
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const uint32 Func_setResourceTarget = 0x004EB290;
	Point32 setResourceTarget(CUnit* a1) {
		static __int32 result;
		static Point32 resultPoint;

		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_setResourceTarget
			MOV result, EAX
			POPAD
		}
		return resultPoint = *(Point32*)&result;
	};

	const uint32 Func_ordersNothing2 = 0x00475000;
	void ordersNothing2(CUnit* a1) {

		__asm {
			PUSHAD
			PUSH a1
			CALL Func_ordersNothing2
			POPAD
		}
	};

	const uint32 Func_orderComputerClear = 0x00475310;
	void orderComputerClear(CUnit* a1, __int8 a2) {

		__asm {
			PUSHAD
			MOVZX CL, a2
			MOV ESI, a1
			CALL Func_orderComputerClear
			POPAD
		}
	}

	// for genericRightClickUnitProc2
	const uint32 Func_unitIsActiveTransport = 0x004E6BA0;
	bool unitIsActiveTransport(CUnit* unit) {
		static int32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_unitIsActiveTransport
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_isOrderValid = 0x00474D90;
	int32 isOrderValid(char a1, CUnit* a2, int32 a3, CUnit* a4, int32 a5) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH a5
			MOV ESI, a4
			MOV EDI, a3
			MOV EBX, a2
			MOVZX AL, a1
			CALL Func_isOrderValid
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_canEnterTransport = 0x004E6E00;
	bool canEnterTransport(CUnit* a1, CUnit* a2) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH a2
			MOV EAX, a1
			CALL Func_canEnterTransport
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_getUnitAbilityErrorStatString = 0x00491E80;
	uint16 getUnitAbilityErrorStatString(CUnit* a1, char a2, int32 a3) {
		static uint32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOVZX BL, a2
			MOV EAX, a1
			CALL Func_getUnitAbilityErrorStatString
			MOV result, EAX
			POPAD
		}
		return (uint16)result;
	}

	// for CMDRECV_HoldPosition
	const uint32 Func_getActivePlayerFirstSelection = 0x0049A850;
	CUnit* getActivePlayerFirstSelection() {
		static CUnit* result;
		__asm {
			PUSHAD
			CALL Func_getActivePlayerFirstSelection
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_orderAllowed = 0x0046DC20;
	int32 orderAllowed(CUnit* a1, uint16 a2, int32 a3) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOVZX BX, a2
			MOV EAX, a1
			CALL Func_orderAllowed
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_targetedOrder = 0x004754F0;
	void targetedOrder(int16 a1, int32 a2, CUnit* a3, int16 a4, int32 a5, int32 a6, char a7, int32 a8, int32 a9, int32 a10) {
		__asm {
			PUSHAD
			PUSH a10
			PUSH a9
			PUSH a8
			PUSH a7
			PUSH a6
			PUSH a5
			MOVZX AX, a4
			MOV ESI, a3
			MOV EBX, a2
			MOVZX DX, a1
			CALL Func_targetedOrder
			POPAD
		}
	}
}
