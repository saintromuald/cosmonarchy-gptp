#include "target_related.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "byte_operations_utils.h"

namespace {
	unsigned int isTargetVisible(CUnit* a1, CUnit* a2);
	int getUnitMaxWeaponRange(CUnit* a1, unsigned __int8 a2);
	bool isTargetWithinMinMovementRange(CUnit* a1, unsigned int a2, CUnit* a3);
}

namespace {
	bool isUnitTargetOutOfMaxRange(CUnit* a1, CUnit* a2) {
		CUnit* _unit; // edi
		CUnit* finalUnit; // esi
		unsigned __int8 _wpn; // bl
		__int16 _type; // ax
		int _maxRange; // eax
		int v8; // eax

		_unit = a1;
		if ( !isTargetVisible(a2, a1) )
		return 1;
		finalUnit = _unit->subunit;
		if ( !finalUnit || !(units_dat::GroupFlags[(unsigned __int16)finalUnit->id] & 0x10) )
		finalUnit = _unit;
		if ( a2->status & UnitStatus::InAir )
		{
		_wpn = units_dat::AirWeapon[(unsigned __int16)finalUnit->id];
		}
		else
		{
		_type = finalUnit->id;
		if ( _type != UnitId::ZergLurker || finalUnit->status & 0x10 )
			_wpn = units_dat::GroundWeapon[_type];
		else
			_wpn = -126;
		}
		if ( _wpn == -126 )
		return 0;
		_maxRange = getUnitMaxWeaponRange(_unit, _wpn);

		ByteOperations::SetLowByte(v8, isTargetWithinMinMovementRange(a2, _maxRange, finalUnit));
		return v8 == 0;
	}
}

namespace {
	const u32 Func_isTargetVisible = 0x004E5E30;
	unsigned int isTargetVisible(CUnit* a1, CUnit* a2) {
		static CUnit* a1;
		static CUnit* a2;
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
	}

	const u32 Func_getUnitMaxWeaponRange = 0x00475870;
	int getUnitMaxWeaponRange(CUnit* a1, unsigned __int8 a2) {
		static CUnit* a1;
		static unsigned __int8 a2;
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
	}

	const u32 Func_isTargetWithinMinMovementRange = 0x004763D0;
	bool isTargetWithinMinMovementRange(CUnit* a1, unsigned int a2, CUnit* a3) {
		static CUnit* a1;
		static unsigned int a2;
		static CUnit* a3;
		static bool result;

		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			MOV EDI, a3
			CALL Func_isTargetWithinMinMovementRange
			MOV result, EAX
			POPAD
		}
		return result;
	}
}
