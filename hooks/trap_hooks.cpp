#include "trap_hooks.h"
#include "byte_operations_utils.h"

namespace { // helpers
    // For disableUnit

    void playImageIscript(CImage* a1, char a2); // 004D8470

    // For updateUnitStrengthAndApplyDefaultOrders
    s16 incrementUnitScores(CUnit* a1, int a2); // 00488BF0
    void incrementUnitScoresEx(int a1, CUnit* a2, int a3); // 00488D50
    void playSoundFromDirection(CUnit* a1, int a2); // 0049B5B0
    void refreshUnitVision(CUnit* a1); // 004E5F30
    void initUnitTrapDoodad(CUnit* a1); // 004E6490
    COrder* removeOrderFromUnitQueue(COrder* a1, CUnit* a2); // 004742D0
    COrder* performAnotherOrder(u8 a1, int a2, CUnit* a3, s16 a4, Point32* a5, CUnit* a6); // 004745F0
    void orders_Nothing2(CUnit* a1); // 00475000
    void orderComputerClear(CUnit* a1, u8 a2); // 00475310
    void order(int a1, CUnit* a2, s8 a3, Point32* a4, CUnit* a5, int a6); // 00474810
    u32 getUnitStrength(CUnit* a1, int a2, bool a3); // 00431800
}

namespace hooks {
	bool unitIsFloorTrap(int a1) { // 0047B070
		s16 v1; // ax

		v1 = *(s16*)(a1 + 100);
		//return v1 == UnitId::Special_FloorMissileTrap || v1 == UnitId::Special_FloorGunTrap;
        return 0;
	}

    s16 disableUnit(CUnit* pUnit) { // 0047B4B0
        int unitFlags; // eax
        CImage* i; // esi
        s16 unitType; // ax

        unitFlags = pUnit->status;
        if (!(unitFlags & UnitStatus::DoodadStatesThing) && !pUnit->lockdownTimer && !pUnit->stasisTimer && !pUnit->maelstromTimer) {
            pUnit->status = unitFlags | UnitStatus::Disabled;
            for (i = pUnit->sprite->images.head; i; i = i->link.next)
                playImageIscript(i, IscriptAnimation::AlmostBuilt);
            unitType = pUnit->id;
            pUnit->status |= UnitStatus::NoCollide;

            if (/*unitType == UnitId::Special_FloorMissileTrap
            ||  unitType == UnitId::Special_FloorGunTrap
            ||*/  unitType == UnitId::Special_UpperLevelDoor
            ||  unitType == UnitId::Special_RightUpperLevelDoor
            ||  unitType == UnitId::Special_PitDoor
            ||  unitType == UnitId::Special_RightPitDoor) {
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

    u32 updateUnitStrengthAndApplyDefaultOrders(CUnit* a1) { // 0049FA40
        CUnit* v1; // esi
        int v2; // ecx
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
        int v13; // ecx
        int v14; // eax
        CSprite* v15; // eax
        u8 v16; // cl
        u32 result; // eax
        int v18; // ecx

        v1 = a1;
        v2 = units_dat::BaseProperty[(u16)a1->id];
        if (v2 & UnitProperty::TwoUnitsIn1Egg) {
            incrementUnitScores(a1, -1);
            v1->status |= 1u;
            incrementUnitScores(v1, 1);
        }
        else {
            a1->status |= 1u;
        }

        incrementUnitScoresEx(1, v1, 1);
        if (units_dat::BaseProperty[(u16)v1->id] & UnitProperty::PermanentCloak)
            playSoundFromDirection(v1, 273);

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
                refreshUnitVision(v1);
            }
            else {
                *lastScannerSweep = v1;
                *firstScannerSweep = v1;
                refreshUnitVision(v1);
            }
        }
        else if (v1->sprite->flags & 0x20) {
            if (v3 != UnitId::ProtossInterceptor && v3 != UnitId::ProtossScarab)
                initUnitTrapDoodad(v1);
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
                    removeOrderFromUnitQueue(v7, v1);
                }
                performAnotherOrder(OrderId::RescuePassive, 0, v1, 228, 0, 0);
            }
            orders_Nothing2(v1);
        }
        else {
            if (v5 == 7) { // Neutral
                v12 = OrderId::Neutral;
            }
            else if (v5 == 1) { // Computer Game
                v12 = units_dat::ComputerIdleOrder[v4];
            }
            else {
                v12 = units_dat::HumanIdleOrder[v4];
            }
            orderComputerClear(v1, v12);
        }
        v13 = (u16)v1->id;
        v14 = units_dat::BaseProperty[v13];
        if (v14 & UnitProperty::NeutralAccessories) {
            v15 = v1->sprite;
            v1->building.powerupOrigin.x = v15->position.x;
            v1->building.powerupOrigin.y = v15->position.y;
            v16 = units_dat::HumanIdleOrder[v13];
            v1->userActionFlags |= 1u;
            order(1, v1, v16, 0, 0, 228);
            orders_Nothing2(v1);
        }
        v1->airStrength = getUnitStrength(v1, v13, 0);
        result = getUnitStrength(v1, v18, 1);
        v1->groundStrength = result;
        return result;
    }
}

namespace { // helper wrappers
    // For disableUnit

    const u32 Func_playImageIscript = 0x004D8470;
    void playImageIscript(CImage* a1, char a2) {
        __asm {
            PUSHAD
            MOV ECX, a1
            PUSH a2
            CALL Func_playImageIscript
            POPAD
        }
    }

    // For updateUnitStrengthAndApplyDefaultOrders

    const u32 Func_incrementUnitScores = 0x00488BF0;
    s16 incrementUnitScores(CUnit* a1, int a2) {
        static int result;
        __asm {
            PUSHAD
            MOV EAX, a1
            PUSH a2
            CALL Func_incrementUnitScores
            MOV result, EAX
            POPAD
        }
        return (s16)result;
    }


    const u32 Func_incrementUnitScoresEx = 0x00488D50;
    void incrementUnitScoresEx(int a1, CUnit* a2, int a3) {
        __asm {
            PUSHAD
            MOV ECX, a1
            MOV EDI, a2
            PUSH a3
            CALL Func_incrementUnitScoresEx
            POPAD
        }
    }

    const u32 Func_playSoundFromDirection = 0x0049B5B0;
    void playSoundFromDirection(CUnit* a1, int a2) {
        __asm {
            PUSHAD
            MOV EAX, a1
            MOV ECX, a2
            CALL Func_playSoundFromDirection
            POPAD
        }
    }

    const u32 Func_refreshUnitVision = 0x004E5F30;
    void refreshUnitVision(CUnit* a1) {
        __asm {
            PUSHAD
            PUSH a1
            CALL Func_refreshUnitVision
            POPAD
        }
    }

    const u32 Func_initUnitTrapDoodad = 0x004E6490;
    void initUnitTrapDoodad(CUnit* a1) {
        __asm {
            PUSHAD
            MOV EDI, a1
            CALL Func_initUnitTrapDoodad
            POPAD
        }
    }

    const u32 Func_removeOrderFromUnitQueue = 0x004742D0;
    COrder* removeOrderFromUnitQueue(COrder* a1, CUnit* a2) {
        static COrder* result;
        __asm {
            PUSHAD
            MOV EAX, a1
            MOV ECX, a2
            CALL Func_removeOrderFromUnitQueue
            MOV result, EAX
            POPAD
        }
        return result;
    }

    const u32 Func_performAnotherOrder = 0x004745F0;
    COrder* performAnotherOrder(u8 a1, int a2, CUnit* a3, s16 a4, Point32* a5, CUnit* a6) {
        static COrder* result;
        __asm {
            PUSHAD
            MOV EBX, a1
            MOV EDI, a2
            MOV ESI, a3
            MOV EDX, a4
            PUSH a5
            PUSH a6
            CALL Func_performAnotherOrder
            MOV result, EAX
            POPAD
        }
        return result;
    }

    const u32 Func_orders_Nothing2 = 0x00475000;
    void orders_Nothing2(CUnit* a1) {
        __asm {
            PUSHAD
            MOV ECX, a1
            CALL Func_orders_Nothing2
            POPAD
        }
    }
    
    const u32 Func_orderComputerClear = 0x00475310;
    void orderComputerClear(CUnit* a1, u8 a2) {
        __asm {
            PUSHAD
            MOV ESI, a1
            MOV ECX, a2
            CALL Func_orderComputerClear
            POPAD
        }
    }

    const u32 Func_order = 0x00474810;
    void order(int a1, CUnit* a2, s8 a3, Point32* a4, CUnit* a5, int a6) {
        __asm {
            PUSHAD
            MOV EAX, a1
            MOV EDX, a2
            MOV ECX, a3
            PUSH a4
            PUSH a5
            PUSH a6
            CALL Func_order
            POPAD
        }
    }

    const u32 Func_getUnitStrength = 0x00431800;
    u32 getUnitStrength(CUnit* a1, int a2, bool a3) {
        static u32 result;
        __asm {
            PUSHAD
            MOV EAX, a1
            MOV ECX, a2
            PUSH a3
            CALL Func_getUnitStrength
            MOV result, EAX
            POPAD
        }
        return result;
    }
}