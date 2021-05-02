#include "target_related.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <globals.h>
#include "byte_operations_utils.h"

namespace hooks
{
	bool isUnitTargetOutOfMaxRange(CUnit* a1, CUnit* a2)
	{ // 00476430
		CUnit* _unit; // edi
		CUnit* finalUnit; // esi
		unsigned __int8 _wpn; // bl
		__int16 _id; // ax
		s32 _maxRange; // eax
		s32 v8; // eax

		_unit = a1;
		if (!isTargetVisibleHook(a2, a1))
			return 1;
		finalUnit = _unit->subunit;
		if (!finalUnit || !(units_dat::GroupFlags[(unsigned __int16)finalUnit->id].isBuilding))
			finalUnit = _unit;
		if (a2->status & UnitStatus::InAir)
		{
			_wpn = units_dat::AirWeapon[(unsigned __int16)finalUnit->id];
		}
		else
		{
			_id = finalUnit->id;
			if (_id != UnitId::ZergLakizilisk || finalUnit->status & 0x10)
				_wpn = units_dat::GroundWeapon[_id];
			else
				_wpn = -126;
		}
		if (_wpn == -126)
			return 0;
		_maxRange = helper::getUnitMaxWeaponRange(_unit, _wpn);

		ByteOperations::SetLowByte(v8, helper::isTargetWithinMinMovementRange(a2, _maxRange, finalUnit));
		return v8 == 0;
	}

	s32 isReadyToAttack(CUnit* a1, unsigned __int8 a2)
	{ // 00476640
		CUnit* v2; // esi
		CUnit* v4; // ecx
		u32 v5; // eax
		u32 v6; // eax
		u32 v7; // edi
		__int16 v8; // cx

		v2 = a1;
		if (a1->movementFlags & 8)
			return 0;
		v4 = a1->orderTarget.unit;
		if (v4)
			v5 = helper::getDistanceToUnit(a1, v4);
		else
			v5 = helper::distanceFromDboxCorner(a1, a1->orderTarget.pt.x, a1->orderTarget.pt.y);
		v7 = v5;
		v6 = weapons_dat::MinRange[a2];
		if (v6 && v7 < v6 || v7 > helper::getUnitMaxWeaponRange(v2, a2) || v2->pathingFlags & 2)
			return 0;
		v8 = v2->nextTargetWaypoint.x;
		if ((v8 != v2->position.x || v2->nextTargetWaypoint.y != v2->position.y)
			&& !helper::isAngleInAttackAngle(v2->nextTargetWaypoint.y, v8, v2, a2))
		{
			v2->orderQueueTimer = 0;
			return 0;
		}
		return 1;
	}

	bool isUnitInWeaponRange(CUnit* a1, CUnit* a2)
	{ // 00476870
		CUnit* v2; // edi
		CUnit* v4; // esi
		unsigned __int8 v5; // bl
		unsigned __int16 v6; // ax
		u32 v7; // eax
		s32 v8; // eax

		v2 = a1;
		if (!a1)
		{
			v2 = a2->orderTarget.unit;
			if (!v2)
				return 1;
		}
		if (!isTargetVisibleHook(v2, a2))
			return 0;
		v4 = a2->subunit;
		if (!v4 || !(units_dat::GroupFlags[(unsigned __int16)v4->id].isBuilding))
			v4 = a2;
		if (v2->status & UnitStatus::InAir)
		{
			v5 = units_dat::AirWeapon[(unsigned __int16)v4->id];
		}
		else
		{
			v6 = v4->id;
			if (v6 != UnitId::ZergLakizilisk || v4->status & 0x10)
				v5 = units_dat::GroundWeapon[v6];
			else
				v5 = -126;
		}
		if (v5 >= (unsigned __int8)0x82)
			return 0;
		v7 = weapons_dat::MinRange[v5];
		if (v7)
		{
			if (helper::isTargetWithinMinRange(v4, v7, v2))
				return 0;
		}
		v8 = helper::getUnitMaxWeaponRange(a2, v5);
		return helper::isTargetWithinMinMovementRange(v2, v8, v4);
	}

	char canCastOrderedSpellNow(CUnit* a1)
	{ // 00491DB0
		CUnit* v1; // esi
		u32 v2; // eax
		CUnit* v3; // edi
		s32 v4; // ebx
		s32 v5; // eax
		s32 v6; // edi
		s32 v7; // ST14_4
		s32 v8; // ebx
		u32 v9; // ebx

		v1 = a1;
		v2 = isUnitVisibleHook(a1);
		if (v2)
		{
			v3 = v1->orderTarget.unit;
			if (v3)
			{
				v4 = helper::getUnitMaxWeaponRange(v1, orders_dat::OrderWeaponId[(unsigned __int8)v1->mainOrderId]);
				if (v1->movementFlags & 2)
				{
					if (!(v3->movementFlags & 2))
						goto LABEL_14;
					v5 = (unsigned __int8)(v3->velocityDirection1 - v1->velocityDirection1);
					if (v5 > 128)
						v5 = 256 - v5;
					if (v5 > 32)
						LABEL_14:
					v4 += helper::getFlingyHaltDistance((s32)v1) >> 8;
				}
				//ByteOperations::SetLowByte((UINT32)v2, (UINT8)isTargetWithinMinRange(v1, v4, v3)); // Needs fixing
			}
			else
			{
				v6 = v1->orderTarget.pt.y;
				v7 = v1->orderTarget.pt.x;
				v8 = helper::getUnitMaxWeaponRange(v1, orders_dat::OrderWeaponId[(unsigned __int8)v1->mainOrderId]);
				v9 = (helper::getFlingyHaltDistance((s32)v1) >> 8) + v8;
				v2 = v9 >= helper::getDistanceFast(v1->halt.x, v7 << 8, v1->halt.y, v6 << 8) >> 8;
			}
		}
		return v2;
	}

	void ordersWatchTarget(CUnit* a1)
	{ // 0047BAB0
		CUnit* v1; // esi
		CUnit* v2; // ebx
		s32 v3; // eax
		CUnit* v4; // ebx
		CUnit* v5; // edi
		unsigned __int8 v6; // bl
		unsigned __int16 v7; // ax
		u32 v8; // eax
		s32 v9; // eax
		s32 v10; // eax
		CUnit* target; // [esp+Ch] [ebp-4h]

		v1 = a1;
		v2 = a1->orderTarget.unit;
		if (!v2)
			goto LABEL_22;
		ByteOperations::SetLowByte(v3, helper::unitCanAttackTarget(v2, a1, 1));
		if (!v3)
			goto LABEL_22;
		v4 = v1->orderTarget.unit;
		target = v1->orderTarget.unit;
		if (v4)
		{
			if (!isTargetVisibleHook(v4, v1))
				goto LABEL_23;
			v5 = v1->subunit;
			if (!v5 || !(units_dat::GroupFlags[(unsigned __int16)v5->id].isBuilding))
				v5 = v1;
			if (v4->status & 4)
			{
				v6 = units_dat::AirWeapon[(unsigned __int16)v5->id];
			}
			else
			{
				v7 = v5->id;
				if (v7 != 103 || v5->status & 0x10)
					v6 = units_dat::GroundWeapon[v7];
				else
					v6 = 0x82;
			}
			if (v6 >= (unsigned __int8)0x82
				|| (v8 = weapons_dat::MinRange[v6]) != 0 && helper::isTargetWithinMinRange(v5, v8, target)
				|| (v9 = helper::getUnitMaxWeaponRange(v1, v6), ByteOperations::SetLowByte(v10, (helper::isTargetWithinMinMovementRange(target, v9, v5), !v10))))
			{
LABEL_23:
				if (helper::getRightClickActionOrder(v1) == 3)
					LABEL_22 :
					helper::unknownOrdersFunc((s32)v1, v1->orderTarget.pt.x, v1->orderTarget.pt.y);
			}
		}
	}

	void ordersStayInRange(CUnit* a1)
	{ // 0047C4F0
		CUnit* v1; // esi
		CUnit* v2; // ebx
		s32 v3; // eax
		CUnit* v4; // ebx
		CUnit* v5; // edi
		unsigned __int8 v6; // bl
		unsigned __int16 v7; // ax
		u32 v8; // eax
		s32 v9; // eax
		s32 v10; // eax
		CUnit* v11; // edi
		unsigned __int8 v12; // bl
		unsigned __int16 v13; // ax
		u32 v14; // eax
		s32 v15; // eax
		s32 v16; // eax
		__int16 v17; // ax
		s32 v18; // edi
		s32 v19; // ebx
		char v20; // al
		s32 v21; // ecx
		s32 v22; // eax
		CUnit* target; // [esp+8h] [ebp-4h]

		v1 = a1;
		v2 = a1->orderTarget.unit;
		if (v2)
		{
			a1->orderTarget.pt.x = v2->sprite->position.x;
			a1->orderTarget.pt.y = v2->sprite->position.y;
			ByteOperations::SetLowByte(v3, helper::unitCanAttackTarget(v2, a1, 1));
			if (!v3)
			{
				if (!(v1->status & 0x8000000) && !v1->pAI)
					helper::prependOrderTargetingGround(6u, v1->orderTarget.pt.x, v1->orderTarget.pt.y, (s32)v1);
				helper::toIdle(v1);
				return;
			}
			v4 = v1->orderTarget.unit;
			target = v1->orderTarget.unit;
			if (v1->mainOrderState)
			{
				if (!v4)
					goto LABEL_62;
				if (!isTargetVisibleHook(v4, v1))
					goto LABEL_63;
				v11 = v1->subunit;
				if (!v11 || !(units_dat::GroupFlags[(unsigned __int16)v11->id].isBuilding))
					v11 = v1;
				if (v4->status & 4)
				{
					v12 = units_dat::AirWeapon[(unsigned __int16)v11->id];
				}
				else
				{
					v13 = v11->id;
					if (v13 != UnitId::ZergLakizilisk || v11->status & 0x10)
						v12 = units_dat::GroundWeapon[v13];
					else
						v12 = 0x82;
				}
				if (v12 >= (unsigned __int8)0x82
					|| (v14 = weapons_dat::MinRange[v12]) != 0 && helper::isTargetWithinMinRange(v11, v14, target)
					|| (v15 = helper::getUnitMaxWeaponRange(v1, v12), ByteOperations::SetLowByte(v16, (helper::isTargetWithinMinMovementRange(target, v15, v11), !v16))))
				{
LABEL_63:
					if (!helper::moveTowards(v1->orderTarget.unit, v1))
						return;
				}
				else
				{
LABEL_62:
					helper::ordersHoldPositionSuicidal(v1);
				}
				goto LABEL_42;
			}
			if (!v4)
				goto LABEL_64;
			if (isTargetVisibleHook(v4, v1))
			{
				v5 = v1->subunit;
				if (!v5 || !(units_dat::GroupFlags[(unsigned __int16)v5->id].isBuilding))
					v5 = v1;
				if (v4->status & 4)
				{
					v6 = units_dat::AirWeapon[(unsigned __int16)v5->id];
				}
				else
				{
					v7 = v5->id;
					if (v7 != 103 || v5->status & 0x10)
						v6 = units_dat::GroundWeapon[v7];
					else
						v6 = 0x82;
				}
				if (v6 < (unsigned __int8)0x82)
				{
					v8 = weapons_dat::MinRange[v6];
					if (!v8 || !helper::isTargetWithinMinRange(v5, v8, target))
					{
						v9 = helper::getUnitMaxWeaponRange(v1, v6);
						ByteOperations::SetLowByte(v10, helper::isTargetWithinMinMovementRange(target, v9, v5));
						if (v10)
							goto LABEL_64;
					}
				}
			}
			if (helper::moveToTarget(v1->orderTarget.unit, v1))
			{
LABEL_64:
				v1->mainOrderState = 1;
LABEL_42:
				v17 = v1->id;
				if ((v17 == 3 || v17 == 17) && !(v1->movementFlags & 2))
				{
					v18 = v1->orderTarget.pt.y;
					v19 = v1->orderTarget.pt.x;
					v20 = helper::getDirectionFromPoints(v18, v1->sprite->position.x, v19, v1->sprite->position.y) - v1->currentDirection1;
					if (v20 >= 128 || v20 <= -128)
						v20 = -128;
					if (abs(v20) >= 32)
						helper::setSecondaryWaypoint(v1, v19, v18);
				}
				return;
			}
		}
		else
		{
			helper::setResourceTarget(a1);
			v21 = v1->moveTarget.pt.x;
			v22 = v1->moveTarget.pt.y;
			if (v21 != v1->nextTargetWaypoint.x || v22 != v1->nextTargetWaypoint.y)
			{
				v1->nextTargetWaypoint.x = v21;
				v1->nextTargetWaypoint.y = v22;
			}
			if (v1->orderQueueHead)
			{
				v1->userActionFlags |= 1u;
				helper::orders_Nothing2(v1);
			}
			else if (v1->pAI)
			{
				helper::orderComputerClear(v1, 0x9Cu);
			}
			else
			{
				helper::orderComputerClear(v1, units_dat::ReturnToIdleOrder[(unsigned __int16)v1->id]);
			}
		}
	}

	s32 genericRightClickUnitProc2(CUnit* unit, s32 a2, s16 a3, CUnit* target, s16 unitID)
	{ // 00455E00
		s32 result; // eax
		CUnit* targetVar; // esi

		if (unit->status & UnitStatus::GroundedBuilding)
		{
			if (unit->isFactory())
				result = OrderId::RallyPoint2 - (target != 0);
			else
				result = OrderId::Nothing2;
			return result;
		}
		targetVar = target;
		if (!target)
		{
			if ((unit->id == UnitId::ZergMatraleth) && unitID != UnitId::None)
				return OrderId::Rightclickaction;
			return OrderId::Move;
		}
		if (helper::unitIsActiveTransport(unit) && helper::isOrderValid(OrderId::Pickup2, unit, 0, targetVar, 0))
			return OrderId::Pickup2;
		if (helper::unitIsActiveTransport(targetVar) && helper::canEnterTransport(targetVar, unit))
			return OrderId::EnterTransport;
		if (targetVar->status & UnitStatus::Burrowed)
			return OrderId::Move;
		if (unit->id != UnitId::ZergMatraleth
			|| targetVar->status & UnitStatus::Invincible
			|| helper::getUnitAbilityErrorStatString(targetVar, unit->playerId, 12) != 0)
		{
			result = (u8)(unit->id != UnitId::TerranCleric ? 0 : 0x80) + OrderId::Follow;
		}
		else
		{
			result = OrderId::InfestMine2;
		}
		return result;
	}

	CUnit* CMDRECV_HoldPosition(s32 a1)
	{ // 004C20C0
		CUnit* result; // eax
		CUnit* i; // esi
		s16 unitId; // ax
		u8 orderId; // al

		*((s8*)&mapHeightPixels + 2) = 0;
		result = helper::getActivePlayerFirstSelection();
		for (i = result; result; i = result)
		{
			unitId = i->id;
			if (unitId != UnitId::ProtossSolarion)
			{
				if (unitId != UnitId::ProtossAccantor)
				{
					if (unitId != UnitId::ZergMatraleth)
					{
						if (unitId != UnitId::ZergSkryling && unitId != UnitId::ZergInfestedTerran)
						{
							orderId = unitId != UnitId::TerranCleric ? 107 : 178;
						}
						else
						{
							orderId = OrderId::HoldPosition4;
						}
					}
					else
					{
						orderId = OrderId::HoldPosition3;
					}
				}
				else
				{
					orderId = OrderId::ReaverHold;
				}
			}
			else
			{
				orderId = OrderId::HoldPosition1;
			}
			if (helper::orderAllowed(i, orderId, *ACTIVE_NATION_ID) == 1)
			{
				helper::targetedOrder(0, UnitId::None, i, 0, 1, *(s8*)(a1 + 1) != 0, orderId, 0, *orderType4Position, *orderType4Target);
			}
			result = helper::getActivePlayerFirstSelection();
		}
		return result;
	}

	u32 unitCanSeeCloakedTargetHook(CUnit* target, CUnit* unit)
	{ // 00401D60, hooked for cloak rewrite - Pr0nogo
		return target->status & 0x300 && !((1 << unit->playerId) & target->visibilityStatus);
	}

	// Hooked to help with cloak rewrite - Pr0nogo
	u32 isTargetVisibleHook(CUnit* target, CUnit* unit)
	{ // 004E5E30
		s32 playerUnit;
		playerUnit = 1 << unit->playerId;
		if (!target)
		{
			return playerUnit & ~(activeTileArray[unit->orderTarget.pt.x / 32 + mapTileSize->width * (unit->orderTarget.pt.y / 32)]->visibilityFlags);
		}

		u32 result;
		if (target->status & 0x300 && !((1 << unit->playerId) & target->visibilityStatus))
		{
			result = (u8)(playerUnit & target->sprite->visibilityFlags);
		}
		else
		{
			result = 0;
		}
		return result;
	}

	// Hooked to help with cloak rewrite - Pr0nogo
	u32 isUnitVisibleHook(CUnit* unit)
	{ // 004E5DB0
		CUnit* target; // eax
		s32 playerUnit; // ebx
		u32 result; // eax

		target = unit->orderTarget.unit;
		playerUnit = 1 << unit->playerId;
		if (!target)
		{
			return playerUnit & ~(activeTileArray[unit->orderTarget.pt.x / 32 + mapTileSize->width * (unit->orderTarget.pt.y / 32)]->visibilityFlags);
		}

		if (unit->status & 0x300)
		{
			result = (u8)(playerUnit & target->sprite->visibilityFlags);
		}
		else
		{
			result = 0;
		}
		return result;
	}

	void updateUnitVisibility(CUnit* unit)
	{
		if (*playerVisions & unit->visibilityStatus)
		{
			helper::makeSpriteVisible(unit->sprite);
			CUnit* subunit = unit->subunit;
			if (subunit)
			{
				helper::makeSpriteVisible(subunit->sprite);
			}
		}
		else
		{
			if (unit->sprite->flags & CSprite_Flags::Selected)
			{
				helper::updateBuildingLandUnitSelection(unit->visibilityStatus, unit);
			}
			helper::removeSelectionCircle(unit->sprite);
			helper::makeSpriteInvisible(unit->sprite);
			CUnit* subunit = unit->subunit;
			if (subunit)
			{
				helper::makeSpriteInvisible(subunit->sprite);
			}
		}
	}

	// Hooked to help with cloak rewrite - Pr0nogo
	void checkUnitVisibilityHook(CUnit* unit)
	{ // 00443390
		u32 detectingPlayers = helper::getDetectingPlayers(unit);
		if (unit->visibilityStatus != detectingPlayers)
		{
			if (unit->visibilityStatus == 0x80000000)
			{
				unit->visibilityStatus = ~(u8)detectingPlayers;
			}
			u32 v6 = unit->visibilityStatus ^ detectingPlayers;
			unit->visibilityStatus = detectingPlayers;

			u32 v5 = v6 & ~detectingPlayers;
			if (v5)
			{
				helper::iteratePlayerUnitsSomething(v5, unit);
			}

			if (*playerVisions & v6)
			{
				updateUnitVisibility(unit);
			}
		}
	}

	// Hooked to complete the cloak rewrite - Pr0nogo
	// was __thiscall, does that matter?
	s32 isUnitCloakingHook(CUnit* unit)
	{ // 004916E0
		if (unit->isCloaked)
		{
			return 0;
		}

		s32 redraw = 0;
		if (*firstInvisibleUnit == unit)
		{
			*firstInvisibleUnit = unit->burrow_link.next;
			redraw = 1;
		}

		CUnit* previousInvisibleUnit = unit->burrow_link.prev;
		if (previousInvisibleUnit)
		{
			previousInvisibleUnit->burrow_link.next = unit->burrow_link.next;
			redraw = 1;
		}

		CUnit* nextInvisibleUnit = unit->burrow_link.next;
		if (nextInvisibleUnit)
		{
			nextInvisibleUnit->burrow_link.prev = unit->burrow_link.prev;
			redraw = 1;
		}

		unit->status &= 0xFFFFF7FF;
		unit->burrow_link.prev = 0;
		unit->burrow_link.next = 0;

		if (redraw)
		{
			*forceButtonRefresh = 1;
			portraitRedraw[0] = (s8)1;
			*statusScreenRedraw = 1;
			*ctrlUnderMouse = 0;
			*ctrlUnderMouseValue = 0;
		}
		return 1;
	}
}