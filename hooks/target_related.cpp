#include "target_related.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <hooks/unit_stats/weapon_range.h>
#include <globals.h>
#include "byte_operations_utils.h"

namespace hooks
{
	bool isUnitTargetOutOfMaxRange(Unit* a1, Unit* a2) // 00476430
	{
		Unit* _unit;
		Unit* finalUnit;
		unsigned __int8 _wpn;
		__int16 _id;
		s32 _maxRange;
		s32 v8;

		_unit = a1;
		if (!isTargetVisibleHook(a2, a1))
		{
			return 1;
		}
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
			{
				_wpn = units_dat::GroundWeapon[_id];
			}
			else
			{
				_wpn = -126;
			}
		}
		if (_wpn == -126)
		{
			return 0;
		}

		_maxRange = helper::get_unit_max_weapon_range(_unit, _wpn);
		ByteOperations::SetLowByte(v8, helper::is_target_within_min_movement_range(a2, finalUnit, _maxRange));
		return v8 == 0;
	}

	s32 isReadyToAttack(Unit* a1, unsigned __int8 a2) // 00476640
	{
		Unit* v2; // esi
		Unit* v4; // ecx
		u32 v5; // eax
		u32 v6; // eax
		u32 v7; // edi
		__int16 v8; // cx

		v2 = a1;
		if (a1->movement_flags & 8)
			return 0;
		v4 = a1->order_target.unit;
		if (v4)
			v5 = helper::getDistanceToUnit(a1, v4);
		else
			v5 = helper::distanceFromDboxCorner(a1, a1->order_target.pt.x, a1->order_target.pt.y);
		v7 = v5;
		v6 = weapons_dat::MinRange[a2];
		if (v6 && v7 < v6 || v7 > helper::get_unit_max_weapon_range(v2, a2) || v2->pathing_flags & 2)
			return 0;
		v8 = v2->next_target_waypoint.x;
		if ((v8 != v2->position.x || v2->next_target_waypoint.y != v2->position.y)
			&& !helper::isAngleInAttackAngle(v2->next_target_waypoint.y, v8, v2, a2))
		{
			v2->order_queue_timer = 0;
			return 0;
		}
		return 1;
	}

	bool isUnitInWeaponRange(Unit* a1, Unit* a2) // 00476870
	{
		Unit* v2; // edi
		Unit* v4; // esi
		unsigned __int8 v5; // bl
		unsigned __int16 v6; // ax
		u32 v7; // eax
		s32 v8; // eax

		v2 = a1;
		if (!a1)
		{
			v2 = a2->order_target.unit;
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
			if (helper::is_target_within_min_range(v4, v7, v2))
				return 0;
		}
		v8 = helper::get_unit_max_weapon_range(a2, v5);
		return helper::is_target_within_min_movement_range(v2, v4, v8);
	}

	char canCastOrderedSpellNow(Unit* a1) // 00491DB0
	{
		Unit* v1; // esi
		u32 v2; // eax
		Unit* v3; // edi
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
			v3 = v1->order_target.unit;
			if (v3)
			{
				v4 = helper::get_unit_max_weapon_range(v1, orders_dat::OrderWeaponId[(unsigned __int8)v1->main_order_id]);
				if (v1->movement_flags & 2)
				{
					if (!(v3->movement_flags & 2))
						goto LABEL_14;
					v5 = (unsigned __int8)(v3->velocity_direction_1 - v1->velocity_direction_1);
					if (v5 > 128)
						v5 = 256 - v5;
					if (v5 > 32)
						LABEL_14:
					v4 += helper::getFlingyHaltDistance((s32)v1) >> 8;
				}
				//ByteOperations::SetLowByte((UINT32)v2, (UINT8)is_target_within_min_range(v1, v4, v3)); // Needs fixing
			}
			else
			{
				v6 = v1->order_target.pt.y;
				v7 = v1->order_target.pt.x;
				v8 = helper::get_unit_max_weapon_range(v1, orders_dat::OrderWeaponId[(unsigned __int8)v1->main_order_id]);
				v9 = (helper::getFlingyHaltDistance((s32)v1) >> 8) + v8;
				v2 = v9 >= helper::get_distance_fast(v1->halt.x, v7 << 8, v1->halt.y, v6 << 8) >> 8;
			}
		}
		return v2;
	}

	void ordersWatchTarget(Unit* a1) // 0047BAB0
	{
		Unit* v1; // esi
		Unit* v2; // ebx
		s32 v3; // eax
		Unit* v4; // ebx
		Unit* v5; // edi
		unsigned __int8 v6; // bl
		unsigned __int16 v7; // ax
		u32 v8; // eax
		s32 v9; // eax
		s32 v10; // eax
		Unit* target; // [esp+Ch] [ebp-4h]

		v1 = a1;
		v2 = a1->order_target.unit;
		if (!v2)
		{
			goto LABEL_22;
		}
		ByteOperations::SetLowByte(v3, helper::unit_can_attack_target(v2, a1, 1));
		if (!v3)
		{
			goto LABEL_22;
		}
		v4 = v1->order_target.unit;
		target = v1->order_target.unit;
		if (v4)
		{
			if (!isTargetVisibleHook(v4, v1))
			{
				goto LABEL_23;
			}
			v5 = v1->subunit;
			if (!v5 || !(units_dat::GroupFlags[(unsigned __int16)v5->id].isBuilding))
			{
				v5 = v1;
			}
			if (v4->status & 4)
			{
				v6 = units_dat::AirWeapon[(unsigned __int16)v5->id];
			}
			else
			{
				v7 = v5->id;
				if (v7 != 103 || v5->status & 0x10)
				{
					v6 = units_dat::GroundWeapon[v7];
				}
				else
				{
					v6 = 0x82;
				}
			}
			if (v6 >= (unsigned __int8)0x82
				|| (v8 = weapons_dat::MinRange[v6]) != 0 && helper::is_target_within_min_range(v5, v8, target)
				|| (v9 = helper::get_unit_max_weapon_range(v1, v6), ByteOperations::SetLowByte(v10, (helper::is_target_within_min_movement_range(target, v5, v9), !v10))))
			{
LABEL_23:
				if (helper::get_right_click_action_order(v1) == 3)
LABEL_22:
				{
					helper::unknownOrdersFunc((s32)v1, v1->order_target.pt.x, v1->order_target.pt.y);
				}
			}
		}
	}

	void ordersStayInRange(Unit* a1) // 0047C4F0
	{
		Unit* v1; // esi
		Unit* v2; // ebx
		s32 v3; // eax
		Unit* v4; // ebx
		Unit* v5; // edi
		unsigned __int8 v6; // bl
		unsigned __int16 v7; // ax
		u32 v8; // eax
		s32 v9; // eax
		s32 v10; // eax
		Unit* v11; // edi
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
		Unit* target; // [esp+8h] [ebp-4h]

		v1 = a1;
		v2 = a1->order_target.unit;
		if (v2)
		{
			a1->order_target.pt.x = v2->sprite->position.x;
			a1->order_target.pt.y = v2->sprite->position.y;
			ByteOperations::SetLowByte(v3, helper::unit_can_attack_target(v2, a1, 1));
			if (!v3)
			{
				if (!(v1->status & 0x8000000) && !v1->unit_ai)
					helper::prependOrderTargetingGround(6u, v1->order_target.pt.x, v1->order_target.pt.y, v1);
				helper::to_idle(v1);
				return;
			}
			v4 = v1->order_target.unit;
			target = v1->order_target.unit;
			if (v1->main_order_state)
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
					|| (v14 = weapons_dat::MinRange[v12]) != 0 && helper::is_target_within_min_range(v11, v14, target)
					|| (v15 = helper::get_unit_max_weapon_range(v1, v12), ByteOperations::SetLowByte(v16, (helper::is_target_within_min_movement_range(target, v11, v15), !v16))))
				{
LABEL_63:
					if (!helper::moveTowards(v1->order_target.unit, v1))
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
					if (!v8 || !helper::is_target_within_min_range(v5, v8, target))
					{
						v9 = helper::get_unit_max_weapon_range(v1, v6);
						ByteOperations::SetLowByte(v10, helper::is_target_within_min_movement_range(target, v5, v9));
						if (v10)
							goto LABEL_64;
					}
				}
			}
			if (helper::move_to_target(v1->order_target.unit, v1))
			{
LABEL_64:
				v1->main_order_state = 1;
LABEL_42:
				v17 = v1->id;
				if ((v17 == 3 || v17 == 17) && !(v1->movement_flags & 2))
				{
					v18 = v1->order_target.pt.y;
					v19 = v1->order_target.pt.x;
					v20 = helper::getDirectionFromPoints(v18, v1->sprite->position.x, v19, v1->sprite->position.y) - v1->current_direction_1;
					if (v20 >= 128 || v20 <= -128)
						v20 = -128;
					if (abs(v20) >= 32)
					{
						helper::set_secondary_waypoint(v1, v19, v18);
					}
				}
				return;
			}
		}
		else
		{
			helper::setResourceTarget(a1);
			v21 = v1->move_target.pt.x;
			v22 = v1->move_target.pt.y;
			if (v21 != v1->next_target_waypoint.x || v22 != v1->next_target_waypoint.y)
			{
				v1->next_target_waypoint.x = v21;
				v1->next_target_waypoint.y = v22;
			}
			if (v1->order_queue_head)
			{
				v1->user_action_flags |= 1u;
				helper::orders_nothing_2(v1);
			}
			else if (v1->unit_ai)
			{
				helper::issue_order_targeting_nothing(v1, 0x9Cu);
			}
			else
			{
				helper::issue_order_targeting_nothing(v1, units_dat::ReturnToIdleOrder[(unsigned __int16)v1->id]);
			}
		}
	}

	s32 genericRightClickUnitProc2(Unit* unit, s32 a2, s16 a3, Unit* target, s16 unitID) // 00455E00
	{
		s32 result;
		Unit* targetVar;

		if (unit->status & UnitStatus::GroundedBuilding)
		{
			if (unit->is_factory())
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
		if (helper::unit_is_active_transport(unit) && helper::isOrderValid(OrderId::Pickup2, unit, 0, targetVar, 0))
			return OrderId::Pickup2;
		if (helper::unit_is_active_transport(targetVar) && helper::canEnterTransport(targetVar, unit))
			return OrderId::EnterTransport;
		if (targetVar->status & UnitStatus::Burrowed)
			return OrderId::Move;
		if (unit->id != UnitId::ZergMatraleth
			|| targetVar->status & UnitStatus::Invincible
			|| helper::getUnitAbilityErrorStatString(targetVar, unit->player, 12) != 0)
		{
			result = (u8)(unit->id != UnitId::TerranCleric ? 0 : 0x80) + OrderId::Follow;
		}
		else
		{
			result = OrderId::InfestMine2;
		}
		return result;
	}

	Unit* CMDRECV_HoldPosition(s32 a1) // 004C20C0
	{
		Unit* result;
		Unit* i;
		s16 unit_id;
		u8 orderId;

		*((s8*)&map_height_pixels + 2) = 0;
		result = helper::get_active_player_first_selection();
		for (i = result; result; i = result)
		{
			unit_id = i->id;
			if (unit_id != UnitId::ProtossSolarion)
			{
				if (unit_id != UnitId::ProtossAccantor)
				{
					if (unit_id != UnitId::ZergMatraleth)
					{
						if (unit_id != UnitId::ZergSkryling && unit_id != UnitId::ZergInfestedTerran)
						{
							orderId = unit_id != UnitId::TerranCleric ? 107 : 178;
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
			result = helper::get_active_player_first_selection();
		}
		return result;
	}

	u32 unitCanSeeCloakedTargetHook(Unit* target, Unit* unit) // 00401D60, hooked for cloak rewrite - Pr0nogo
	{
		return target->status & 0x300 && !((1 << unit->player) & target->visibility_status);
	}

	// Hooked to help with cloak rewrite - Pr0nogo
	u32 isTargetVisibleHook(Unit* target, Unit* unit) // 004E5E30
	{
		s32 playerUnit;
		playerUnit = 1 << unit->player;
		if (!target)
		{
			return playerUnit & ~(activeTileArray[unit->order_target.pt.x / 32 + mapTileSize->width * (unit->order_target.pt.y / 32)]->visibility_flags);
		}

		u32 result;
		if (target->status & 0x300 && !((1 << unit->player) & target->visibility_status))
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
	u32 isUnitVisibleHook(Unit* unit) // 004E5DB0
	{
		Unit* target;
		s32 playerUnit;
		u32 result;

		target = unit->order_target.unit;
		playerUnit = 1 << unit->player;
		if (!target)
		{
			return playerUnit & ~(activeTileArray[unit->order_target.pt.x / 32 + mapTileSize->width * (unit->order_target.pt.y / 32)]->visibility_flags);
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

	void updateUnitVisibility(Unit* unit)
	{
		if (*playerVisions & unit->visibility_status)
		{
			helper::makeSpriteVisible(unit->sprite);
			Unit* subunit = unit->subunit;
			if (subunit)
			{
				helper::makeSpriteVisible(subunit->sprite);
			}
		}
		else
		{
			if (unit->sprite->flags & SpriteFlags::Selected)
			{
				helper::updateBuildingLandUnitSelection(unit->visibility_status, unit);
			}
			helper::removeSelectionCircle(unit->sprite);
			helper::makeSpriteInvisible(unit->sprite);
			Unit* subunit = unit->subunit;
			if (subunit)
			{
				helper::makeSpriteInvisible(subunit->sprite);
			}
		}
	}

	// Hooked to help with cloak rewrite - Pr0nogo
	void checkUnitVisibilityHook(Unit* unit) // 00443390
	{
		u32 detectingPlayers = helper::get_detecting_players(unit);
		if (unit->visibility_status != detectingPlayers)
		{
			if (unit->visibility_status == 0x80000000)
			{
				unit->visibility_status = ~(u8)detectingPlayers;
			}
			u32 v6 = unit->visibility_status ^ detectingPlayers;
			unit->visibility_status = detectingPlayers;

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
	s32 isUnitCloakingHook(Unit* unit) // 004916E0
	{
		if (unit->is_cloaked)
		{
			return 0;
		}

		s32 redraw = 0;
		if (*first_invisible_unit == unit)
		{
			*first_invisible_unit = unit->burrow_link.next;
			redraw = 1;
		}

		Unit* previousInvisibleUnit = unit->burrow_link.prev;
		if (previousInvisibleUnit)
		{
			previousInvisibleUnit->burrow_link.next = unit->burrow_link.next;
			redraw = 1;
		}

		Unit* nextInvisibleUnit = unit->burrow_link.next;
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
			*force_button_refresh = 1;
			portrait_redraw[0] = (s8)1;
			*status_screen_redraw = 1;
			*control_under_mouse = 0;
			*control_under_mouse_value = 0;
		}
		return 1;
	}

	bool unit_can_attack_target(Unit* target, Unit* unit, int checkVisibility)
	{
		if (unit->is_frozen()
			|| !target
			|| !target->sprite // new
			|| target->status & UnitStatus::Invincible
			|| target->sprite->flags & SpriteFlags::Hidden
			|| checkVisibility && helper::unit_can_see_cloaked_target(target, unit))
		{
			return 0;
		}

		Unit* unitWithWeapon = unit->subunit;
		if (!helper::unitIsSubunit(unit->subunit))
		{
			unitWithWeapon = unit;
		}

		if (helper::unitIsCarrier(unit))
		{
			return 1;
		}

		if (helper::unitIsReaver(unit))
		{
			if (!(target->status & UnitStatus::InAir))
			{
				if (helper::unitHasPathToUnit(unit, target))
				{
					return 1;
				}
			}
			return 0;
		}

		// Originally had code related to infestation

		// Originally had code to disable AI Didact attacks

		// GAME RULE: Ground titanic units can be targeted by air-only weapons
		// Doesn't work currently.
		/*
		if (units_dat::SizeType[target->id] == UnitSize::Titanic)
		{
			return units_dat::AirWeapon[unitWithWeapon->id] != WeaponId::None
				|| units_dat::GroundWeapon[unitWithWeapon->id] != WeaponId::None;
		}
		*/

		u32 weapon = target->status & UnitStatus::InAir ? units_dat::AirWeapon[unitWithWeapon->id] : units_dat::GroundWeapon[unitWithWeapon->id];

		if (unit->status & UnitStatus::GroundedBuilding || unit->status & UnitStatus::Burrowed || unit->id == UnitId::TerranPhalanxSiegeMode) {
			if (!unit->is_target_within_min_range(target, hooks::getMaxWeaponRangeHook(unit, weapon, target))) {
				return false;
			}
		}

		return weapon != WeaponId::None;
	}

	s32 orderAttackBehavior(Unit* unit) // 00478540
	{
		s32 result;
		Unit *v1;
		Unit *v2;
		DWORD v3;
		s32 v4;
		Unit *v5;
		s32 v6;
		s32 v7;
		BOOL v8;
		Unit *v9;
		s32 v10;
		Unit *v11;
		s16 v13;
		s16 v14;
		Unit *v15;
		Unit *v16;
		u8 v17;
		u16 v18;
		u32 v19;
		s32 v20;
		s32 v21;
		s32 v22;
		DWORD v23;
		Unit *v24;
		bool v25;
		Unit *target;
		Unit *targeta;

		v2 = unit;
		v3 = unit->status;
		v1 = v2->order_target.unit;
		if (unit->is_frozen())
		{
			goto LABEL_23;
		}
		if (!v1)
		{
			goto LABEL_23;
		}
		target = (Unit*)v1->status;
		if (v1->status & UnitStatus::Invincible)
		{
			goto LABEL_23;
		}
		if ( v1->sprite->flags & 0x20 )
		{
			goto LABEL_23;
		}
		if (helper::unit_can_see_cloaked_target(v1, v2))
		{
			goto LABEL_23;
		}
		v5 = v2->subunit;
		if (!helper::unitIsSubunit(v2->subunit))
		{
			v5 = v2;
		}
		if (v2->id == UnitId::ProtossSolarion)
		{
			goto LABEL_34;
		}
		if (v2->id == UnitId::ProtossAccantor)
		{
			if (!(target->status & UnitStatus::InAir))
			{
				v8 = helper::unitHasPathToUnit(v2, v1);
				goto LABEL_22;
			}
LABEL_23:
			v11 = v2->order_target.unit;
			if ( !v11 || v2->status & UnitStatus::HoldingPosition )
			{
				helper::setResourceTarget(v2);
				if (v2->order_queue_head)
				{
					v2->user_action_flags |= 1u;
					helper::orders_nothing_2(v2);
					result = 0;
				}
				else
				{
					if (v2->unit_ai)
					{
						v2->order_computer(OrderId::ComputerAI);
					}
					else
					{
						v2->order_computer(units_dat::ReturnToIdleOrder[v2->id]);
					}
					result = 0;
				}
			}
			else
			{
				if ( !v2->unit_ai )
				{
					helper::prependOrderTargetingGround(6u, v11->sprite->position.x, v11->sprite->position.y, v2);
				}
				helper::to_idle(v2);
				result = 0;
			}
			return result;
		}
		if (helper::unit_can_infest(v2))
		{
			v8 = helper::canBeInfested(v9, v1) != 0;
		}
		else
		{
			if (target->status & UnitStatus::InAir || (units_dat::SizeType[target->id] == UnitSize::Titanic && units_dat::AirWeapon[v5->id] != -126))
			{
				v8 = units_dat::AirWeapon[v5->id] != WeaponId::None;
			}
			else
			{
				v8 = units_dat::GroundWeapon[v5->id] != WeaponId::None;
			}
		}
LABEL_22:
		if (!v8)
		{
			goto LABEL_23;
		}
LABEL_34:
		v15 = v2->order_target.unit;
		v14 = v15->sprite->position.x;
		v2->order_target.pt.x = v14;
		v13 = v15->sprite->position.y;
		v2->order_target.pt.y = v13;
		targeta = v15;
		if (v14 != v2->next_target_waypoint.x || v13 != v2->next_target_waypoint.y)
		{
			v2->next_target_waypoint.x = v14;
			v2->next_target_waypoint.y = v13;
		}
		if (!v2->main_order_state)
		{
			if (!v15)
			{
				goto LABEL_76;
			}
			if (helper::is_target_visible(v15, v2))
			{
				v16 = v2->subunit;
				if (!v16 || !(units_dat::BaseProperty[v16->id] & UnitProperty::Subunit))
				{
					v16 = v2;
				}
				if (v15->status & UnitStatus::InAir || (units_dat::SizeType[v15->id] == UnitSize::Titanic && units_dat::AirWeapon[v16->id] != -126))
				{
					v17 = units_dat::AirWeapon[v16->id];
				}
				else
				{
					v18 = v16->id;
					if (v18 != UnitId::ZergLakizilisk || v16->status & UnitStatus::Burrowed)
					{
						v17 = units_dat::GroundWeapon[v18];
					}
					else
					{
						v17 = -126;
					}
				}
				if (v17 < WeaponId::None)
				{
					v19 = weapons_dat::MinRange[v17];
					if (!v19 || !helper::is_target_within_min_range(v16, v19, targeta))
					{
						v20 = helper::get_unit_max_weapon_range(v2, v17);
						v21 = helper::is_target_within_min_movement_range(targeta, v16, v20);
						if (v21)
						{
							goto LABEL_76;
						}
					}
				}
			}
			if (!helper::move_to_target(v2->order_target.unit, v2))
			{
				return 0;
			}
LABEL_76:
			v2->main_order_state = 1;
			return 1;
		}
		if (helper::is_unit_target_out_of_max_range(v2, v15))
		{
			if (v2->unit_ai)
			{
				if (helper::isUnreachable(v2->order_target.unit, v2) || helper::is_unit_movable(v2) == 2)
				{
					if (helper::ai_update_attack_target(v2, 0, 1, 0))
					{
						return 0;
					}
					goto LABEL_65;
				}
			}
			else
			{
				v23 = v2->status;
				if (v23 & 0x8000000 && !(v23 & 0xC) && v2->order_target.unit->status & 4 || helper::is_unit_movable(v2) == 2)
				{
LABEL_65:
					helper::make_to_hold_position(v2);
					helper::to_idle(v2);
					return 0;
				}
			}
			v25 = helper::hasGroundWeaponGraphic(v2) == 0;
			v24 = v2->order_target.unit;
			if (!v25)
			{
				helper::moveTowards(v2, v24);
				return 0;
			}
			helper::moveForMeleeRange(v24, v2);
			return 0;
		}
		if (v2->id == UnitId::ZergSkryling)
		{
			helper::moveForMeleeRange(v2->order_target.unit, v2);
		}
		else
		{
			helper::setResourceTarget(v2);
		}
		return 1;
	}

	// Hooked to help with the Titanic size game rule - Pr0nogo
	void AI_AttackUnit(Unit* unit) // 0043FFD0
	{
		if (!unit)
		{
			return;
		}

		u16 unit_id = unit->id;
		if (!unit_id || unit_id == 32)
		{
			Unit* unitTarget = unit->order_target.unit;
			if (unitTarget)
			{
				s32 cooldownVar = 0;
				if (unit->stim_timer)
				{
					cooldownVar = 1;
				}
				if (unit->status & UnitStatus::SpeedUpgrade)
				{
					++cooldownVar;
				}
				if (unit->ensnare_timer)
				{
					--cooldownVar;
				}
				if (cooldownVar <= 0)
				{
					s32 unitHP = units_dat::MaxHitPoints[unit_id] >> 8;
					if (!unitHP)
					{
						unitHP = (unit->hitpoints + 255) >> 8;
						if (!unitHP)
						{
							unitHP = 1;
						}
					}
					if ((unit->hitpoints + 255) >> 8 >= unitHP / 2)
					{
						if (unit_can_attack_target(unit, unitTarget, 1))
						{
							unitTarget = unit->order_target.unit;
							if (!unitTarget)
							{
								if (helper::techUseAllowed(unit, 0, unit->player) == 1
								  && helper::get_current_hp_plus_shields(unitTarget) >= 100)
								{
									helper::stimpackProc(unit);
								}
								return;
							}
							if (helper::is_target_visible(unitTarget, unit))
							{
								Unit* subunit = unit->subunit;
								if (!subunit || !(units_dat::BaseProperty[subunit->id] & UnitProperty::Subunit))
								{
									subunit = unit;
								}

								u8 airWeapon;
								if (unitTarget->status & 4)
								{
									airWeapon = units_dat::AirWeapon[subunit->id];
								}
								else
								{
									u16 subunitId = subunit->id;
									if (subunitId != UnitId::ZergLakizilisk || subunit->status & 0x10)
									{
										airWeapon = units_dat::GroundWeapon[subunitId];
									}
									else
									{
										airWeapon = WeaponId::None;
									}
								}
								if (airWeapon < WeaponId::None)
								{
									u32 airWeaponMinRange = weapons_dat::MinRange[airWeapon];
									if (!airWeaponMinRange || !(helper::is_target_within_min_range(subunit, airWeaponMinRange, unitTarget)))
									{
										s32 airWeaponMaxRange = weapons_dat::MaxRange[airWeapon];
										if (helper::is_target_within_min_movement_range(unitTarget, subunit, airWeaponMaxRange))
										{
											if (helper::techUseAllowed(unit, 0, unit->player) == 1
											  && helper::get_current_hp_plus_shields(unitTarget) >= 100)
											{
												helper::stimpackProc(unit);
											}
											return;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Hooked to add nullcheck. -Pr0nogo
	s32 return_interceptor_if_cannot_attack(Unit* hangar_unit) // 00465810
	{
		s32 result = 0;
		Unit* steward_target = hangar_unit->order_target.unit;
		if (steward_target && !(steward_target->status & UnitStatus::Invincible) || helper::attack_recent_attacker(hangar_unit))
		{
			Unit* unit_with_hangar = hangar_unit->hangar_unit.parent;
			steward_target = hangar_unit->order_target.unit;
			if (steward_target->sprite) {
				hangar_unit->order_target.pt.x = steward_target->sprite->position.x;
				hangar_unit->order_target.pt.y = steward_target->sprite->position.y;
			}
			if (!unit_with_hangar)
			{
				unit_with_hangar = hangar_unit;
			}
			u32 range = helper::get_target_acquisition_range_upgrade(unit_with_hangar) + 2;
			if (!unit_with_hangar || helper::is_target_within_min_movement_range(steward_target, unit_with_hangar, range * 32))
			{
				result = 1;
			}
			else
			{
				helper::issue_order_targeting_nothing(hangar_unit, OrderId::Return);
				result = 0;
			}
		}
		else
		{
			u8 steward_order_id = hangar_unit->main_order_id;
			hangar_unit->user_action_flags |= 1u;
			if (steward_order_id)
			{
				while (1)
				{
					Order* order_tail = hangar_unit->order_queue_tail;
					if (!order_tail)
					{
						break;
					}
					if (!orders_dat::CanBeInterrupted[order_tail->orderId] && order_tail->orderId != OrderId::Return)
					{
						break;
					}
					helper::remove_order_from_unit_queue(order_tail, hangar_unit);
				}
				helper::perform_another_order(OrderId::Return, 0, hangar_unit, UnitId::None, {}, 0);
			}
			helper::orders_nothing_2(hangar_unit);
			result = 0;
		}
		return result;
	}
}