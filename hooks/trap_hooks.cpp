#include "trap_hooks.h"
#include "byte_operations_utils.h"

namespace hooks {
	bool unitIsFloorTrap(CUnit* unit) { // 0047B070
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
	bool unitIsInstallationAsset(u16 id) {
		/*
				//originally:
				if (id >= UnitId::Special_FloorMissileTrap && id <= UnitId::Special_RightWallFlameTrap) {
					return true;
				}*/
		if (id >= UnitId::Special_UpperLevelDoor && id <= UnitId::Special_RightPitDoor) {
			return true;
		}
		return false;
	}
	s16 disableTrap(CUnit* pUnit) { // 0047B4B0
		s32 unitFlags; // eax
		CImage* i; // esi
		s16 unitType; // ax

		unitFlags = pUnit->status;
		if (!(unitFlags & UnitStatus::DoodadStatesThing) && pUnit->lockdownTimer == 0 && pUnit->stasisTimer == 0
			&& pUnit->maelstromTimer == 0) {
			pUnit->status = unitFlags | UnitStatus::Disabled;
			for (i = pUnit->sprite->images.head; i; i = i->link.next)
				helper::playImageIscript(i, IscriptAnimation::AlmostBuilt);
			unitType = pUnit->id;
			pUnit->status |= UnitStatus::NoCollide;

			if (/*unitType == UnitId::Special_FloorMissileTrap
			||  unitType == UnitId::Special_FloorGunTrap
			||*/  unitType == UnitId::Special_UpperLevelDoor
				|| unitType == UnitId::Special_RightUpperLevelDoor
				|| unitType == UnitId::Special_PitDoor
				|| unitType == UnitId::Special_RightPitDoor) {
				pUnit->sprite->elevationLevel = 1;
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
				pUnit->visibilityStatus = 0x80000000;
				ByteOperations::SetLowByte(pUnit->secondaryOrderTimer, 0);
			}*/
		}
		return unitFlags;
	}

	u32 updateUnitStrengthAndApplyDefaultOrdersHook(CUnit* a1) { // 0049FA40
		CUnit* v1; // esi
		s32 v2; // ecx
		s16 v3; // ax
		u16 v4; // cx
		s8 v5; // al
		char v6; // al
		COrder* v7; // eax
		u8 v8; // cl
		CUnit* v9; // eax
		CUnit* v10; // eax
		CUnit* v11; // ecx
		u8 v12; // cl
		s32 v13; // ecx
		s32 v14; // eax
		CSprite* v15; // eax
		u8 v16; // cl
		u32 result; // eax
		s32 v18; // ecx

		v1 = a1;
		v2 = units_dat::BaseProperty[(u16)a1->id];
		if (v2 & UnitProperty::TwoUnitsIn1Egg) {
			helper::incrementUnitScores(a1, -1);
			v1->status |= UnitStatus::Completed;
			helper::incrementUnitScores(v1, 1);
		} else {
			a1->status |= UnitStatus::Completed;
		}

		helper::incrementUnitScoresEx(v1, 1, 1);
		if (units_dat::BaseProperty[(u16)v1->id] & UnitProperty::PermanentCloak) {
			helper::playSoundFromDirection(v1, 273);
		}

		v3 = v1->id;
		if (v3 == UnitId::Spell_ScannerSweep || v3 == UnitId::Special_MapRevealer) {
			if (*firstHiddenUnit == v1)
				*firstHiddenUnit = v1->link.next;
			if (*lastHiddenUnit == v1)
				*lastHiddenUnit = v1->link.prev;
			if (v1->link.prev)
				v1->link.prev->link.next = v1->link.next;
			v9 = v1->link.next;
			if (v9)
				v9->link.prev = v1->link.prev;
			v1->link.prev = 0;
			v1->link.next = 0;
			v10 = *firstScannerSweep;
			if (firstScannerSweep) {
				if (*lastScannerSweep == *firstScannerSweep)
					*lastScannerSweep = v1;
				v1->link.prev = *firstScannerSweep;
				v1->link.next = v10->link.next;
				v11 = v10->link.next;
				if (v11)
					v11->link.prev = v1;
				v10->link.next = v1;
				helper::refreshUnitVision(v1);
			} else {
				*lastScannerSweep = v1;
				*firstScannerSweep = v1;
				helper::refreshUnitVision(v1);
			}
		} else if (v1->sprite->flags & 0x20) {
			if (v3 != UnitId::ProtossSteward && v3 != UnitId::ProtossPlasmaShell) {
				helper::initUnitTrapDoodad(v1);
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
			v1->visibilityStatus = 0x80000000;
			ByteOperations::SetLowByte(v1->secondaryOrderTimer, 0);
		}*/
		v5 = playerTable[(u8)v1->playerId].type;
		if (v5 == 3) { // Rescue Passive
			v6 = v1->mainOrderId;
			v1->userActionFlags |= 1u;
			if (v6) {
				while (1) {
					v7 = v1->orderQueueTail;
					if (!v7)
						break;
					v8 = v7->orderId;
					if (!orders_dat::CanBeInterrupted[v8] && v8 != -95)
						break;
					helper::removeOrderFromUnitQueue(v7, v1);
				}
				helper::performAnotherOrder(OrderId::RescuePassive, 0, v1, 228, 0, 0);
			}
			helper::orders_Nothing2(v1);
		} else {
			if (v5 == 7) { // Neutral
				v12 = OrderId::Neutral;
			} else if (v5 == 1) { // Computer Game
				v12 = units_dat::ComputerIdleOrder[v4];
			} else {
				v12 = units_dat::HumanIdleOrder[v4];
			}
			helper::orderComputerClear(v1, v12);
		}
		v13 = (u16)v1->id;
		v14 = units_dat::BaseProperty[v13];
		if (v14 & UnitProperty::NeutralAccessories) {
			v15 = v1->sprite;
			v1->building.powerupOrigin.x = v15->position.x;
			v1->building.powerupOrigin.y = v15->position.y;
			v16 = units_dat::HumanIdleOrder[v13];
			v1->userActionFlags |= 1u;
			helper::order(1, v1, v16, 0, 0, 228);
			helper::orders_Nothing2(v1);
		}
		v1->airStrength = helper::getUnitStrength(v1, v13, 0);
		result = helper::getUnitStrength(v1, v18, 1);
		v1->groundStrength = result;
		return result;
	}
}