#include "trap_hooks.h"
#include "byte_operations_utils.h"

namespace hooks
{
	bool unitIsFloorTrap(Unit* unit)
	{ // 0047B070
		u16 id = unit->id;
		/*switch (id) {
		case UnitId::Special_FloorGunTrap:
		case UnitId::Special_FloorMissileTrap:
			return true;
		default:
			return false;
		}*/
		return false;
	}
	bool unitIsInstallationAsset(u16 id)
	{
		/*
				//originally:
				if (id >= UnitId::Special_FloorMissileTrap && id <= UnitId::Special_RightWallFlameTrap) {
					return true;
				}*/
		if (id >= UnitId::Special_UpperLevelDoor && id <= UnitId::Special_RightPitDoor)
		{
			return true;
		}
		return false;
	}
	s16 disableTrap(Unit* pUnit)
	{ // 0047B4B0
		s32 unitFlags; // eax
		Image* i; // esi
		s16 unit_id; // ax

		unitFlags = pUnit->status;
		if (!(unitFlags & UnitStatus::Frozen) && pUnit->lockdown_timer == 0 && pUnit->stasis_timer == 0
			&& pUnit->maelstrom_timer == 0)
		{
			pUnit->status = unitFlags | UnitStatus::Disabled;
			for (i = pUnit->sprite->images.head; i; i = i->link.next)
				helper::play_image_iscript(i, IscriptAnimation::AlmostBuilt);
			unit_id = pUnit->id;
			pUnit->status |= UnitStatus::NoCollide;

			if (/*unit_id == UnitId::Special_FloorMissileTrap
			||  unit_id == UnitId::Special_FloorGunTrap
			||*/  unit_id == UnitId::Special_UpperLevelDoor
				|| unit_id == UnitId::Special_RightUpperLevelDoor
				|| unit_id == UnitId::Special_PitDoor
				|| unit_id == UnitId::Special_RightPitDoor)
			{
				pUnit->sprite->elevation_level = 1;
			}
			/*
			ByteOperations::SetLowByte(unitFlags, pUnit->id);
			if ((s16)unitFlags == UnitId::Special_FloorMissileTrap
			|| (s16)unitFlags == UnitId::Special_FloorGunTrap
			|| (s16)unitFlags == UnitId::Special_WallMissileTrap
			|| (s16)unitFlags == UnitId::Special_WallFlameTrap
			|| (s16)unitFlags == UnitId::Special_RightWallMissileTrap
			|| (s16)unitFlags == UnitId::Special_RightWallFlameTrap) {
			pUnit->status |= UnitStatus::Cloaked | UnitStatus::RequiresDetection;
				pUnit->visibility_status = 0x80000000;
				ByteOperations::SetLowByte(pUnit->secondary_order_timer, 0);
			}*/
		}
		return unitFlags;
	}

	u32 updateUnitStrengthAndApplyDefaultOrdersHook(Unit* a1)
	{ // 0049FA40
		Unit* v1; // esi
		s32 v2; // ecx
		s16 v3; // ax
		u16 v4; // cx
		s8 v5; // al
		char v6; // al
		Order* v7; // eax
		u8 v8; // cl
		Unit* v9; // eax
		Unit* v10; // eax
		Unit* v11; // ecx
		u8 v12; // cl
		s32 v13; // ecx
		s32 v14; // eax
		Sprite* v15; // eax
		u8 v16; // cl
		u32 result; // eax
		s32 v18; // ecx

		v1 = a1;
		v2 = units_dat::BaseProperty[(u16)a1->id];
		if (v2 & UnitProperty::TwoUnitsIn1Egg)
		{
			helper::increment_unit_scores(a1, -1);
			v1->status |= UnitStatus::Completed;
			helper::increment_unit_scores(v1, 1);
		}
		else
		{
			a1->status |= UnitStatus::Completed;
		}

		helper::increment_unit_scores_ex(v1, 1, 1);
		if (units_dat::BaseProperty[(u16)v1->id] & UnitProperty::PermanentCloak)
		{
			helper::anim_cloak(v1, 273);
		}

		v3 = v1->id;
		if (v3 == UnitId::Spell_ScannerSweep || v3 == UnitId::Special_MapRevealer)
		{
			if (*first_hidden_unit == v1)
				*first_hidden_unit = v1->link.next;
			if (*last_hidden_unit == v1)
				*last_hidden_unit = v1->link.prev;
			if (v1->link.prev)
				v1->link.prev->link.next = v1->link.next;
			v9 = v1->link.next;
			if (v9)
				v9->link.prev = v1->link.prev;
			v1->link.prev = 0;
			v1->link.next = 0;
			v10 = *first_scanner_sweep;
			if (first_scanner_sweep)
			{
				if (*last_scanner_sweep == *first_scanner_sweep)
					*last_scanner_sweep = v1;
				v1->link.prev = *first_scanner_sweep;
				v1->link.next = v10->link.next;
				v11 = v10->link.next;
				if (v11)
					v11->link.prev = v1;
				v10->link.next = v1;
				helper::refresh_unit_vision(v1);
			}
			else
			{
				*last_scanner_sweep = v1;
				*first_scanner_sweep = v1;
				helper::refresh_unit_vision(v1);
			}
		}
		else if (v1->sprite->flags & 0x20)
		{
			if (v3 != UnitId::ProtossSteward && v3 != UnitId::ProtossPlasmaShell)
			{
				helper::init_unit_trap_doodad(v1);
			}
		}
		v4 = v1->id;/*
		if (v4 == UnitId::Special_FloorMissileTrap
		|| v4 == UnitId::Special_FloorGunTrap
		|| v4 == UnitId::Special_WallMissileTrap
		|| v4 == UnitId::Special_WallFlameTrap
		|| v4 == UnitId::Special_RightWallMissileTrap
		|| v4 == UnitId::Special_RightWallFlameTrap) {
			v1->status |= UnitStatus::Cloaked | UnitStatus::RequiresDetection;
			v1->visibility_status = 0x80000000;
			ByteOperations::SetLowByte(v1->secondary_order_timer, 0);
		}*/
		v5 = playerTable[(u8)v1->player].type;
		if (v5 == 3)
		{ // Rescue Passive
			v6 = v1->main_order_id;
			v1->user_action_flags |= 1u;
			if (v6)
			{
				while (1)
				{
					v7 = v1->order_queue_tail;
					if (!v7)
						break;
					v8 = v7->order_id;
					if (!orders_dat::CanBeInterrupted[v8] && v8 != -95)
						break;
					helper::remove_order_from_unit_queue(v7, v1);
				}
				helper::perform_another_order(OrderId::RescuePassive, 0, v1, 228, {}, 0);
			}
			helper::orders_nothing_2(v1);
		}
		else
		{
			if (v5 == 7)
			{ // Neutral
				v12 = OrderId::Neutral;
			}
			else if (v5 == 1)
			{ // Computer Game
				v12 = units_dat::ComputerIdleOrder[v4];
			}
			else
			{
				v12 = units_dat::HumanIdleOrder[v4];
			}
			helper::issue_order_targeting_nothing(v1, v12);
		}
		v13 = (u16)v1->id;
		v14 = units_dat::BaseProperty[v13];
		if (v14 & UnitProperty::NeutralAccessories)
		{
			v15 = v1->sprite;
			v1->powerup_origin.x = v15->position.x;
			v1->powerup_origin.y = v15->position.y;
			v16 = units_dat::HumanIdleOrder[v13];
			v1->user_action_flags |= 1u;
			helper::order(1, v1, v16, 0, 0, 228);
			helper::orders_nothing_2(v1);
		}
		v1->air_strength = helper::get_unit_strength(v1, v13, 0);
		result = helper::get_unit_strength(v1, v18, 1);
		v1->ground_strength = result;
		return result;
	}
}