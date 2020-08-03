#include "miss_chance.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace { // helper declarations
	void getAbsoluteUnitBounds(CUnit* a1, Box32* a2);
	CUnit* findBestUnit(Box32* a1, int a2, int (__fastcall *a3)(int, int));
	bool compareUnitToUnitType(CUnit* a1, __int16 a2);
	unsigned int getGroundHeightAtPx(signed int a1, signed int a2);
}

namespace hooks {
	int isUnderDarkSwarm(CUnit* a1) {				// 00476210
		CUnit* _pUnit; // esi
		Box32 bounds; // [esp+4h] [ebp-8h]

		_pUnit = a1;
		if (a1->status & UnitStatus::InAir)
			return 0;
		if (!(units_dat::GroupFlags[(unsigned __int16)a1->id].isBuilding))
		{
			if (unitCountTable[10].player[155])
			{
				getAbsoluteUnitBounds(a1, &bounds);
				if (findBestUnit(&bounds, UnitId::Spell_DarkSwarm, (int(__fastcall*)(int, int))compareUnitToUnitType))
					return 255;
			}
		}
		return (activeTileHandle[_pUnit->sprite->position.x / 32 + mapTileSize->width * (_pUnit->sprite->position.y / 32)] & 0x100000) != 0 ? 119 : 0;
	}

	int canHitTargetCheck(CUnit* a1, CUnit* a2) {	// 004765B0
		int darkSwarmOrd; // ebx
		CUnit* _unit; // edi
		CUnit* _targ; // esi
		CSprite* _pSprite; // esi
		unsigned int _grndHeight; // edi
		int result; // eax

		_targ = a1;
		_unit = a2;
		darkSwarmOrd = isUnderDarkSwarm(a1);
		if (_unit->status & UnitStatus::InAir
			|| _targ->status & UnitStatus::InAir
			|| (_pSprite = _targ->sprite,
				_grndHeight = getGroundHeightAtPx(_unit->sprite->position.y, _unit->sprite->position.x),
				_grndHeight >= getGroundHeightAtPx(_pSprite->position.y, _pSprite->position.x))
			|| (result = 119, darkSwarmOrd > 119))
		{
			result = darkSwarmOrd;
		}
		return result;
	}
}

namespace { // helper definitions
	const u32 Func_getAbsoluteUnitBounds = 0x00402020;
	void getAbsoluteUnitBounds(CUnit* a1, Box32* a2) {

		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_getAbsoluteUnitBounds
			POPAD
		}
	};

	const u32 Func_findBestUnit = 0x004E8830;
	CUnit* findBestUnit(Box32* a1, int a2, int(__fastcall* a3)(int, int)) {
		static CUnit* result;

		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOV EBX, a2
			CALL Func_findBestUnit
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const u32 Func_compareUnitToUnitType = 0x004402A0;
	bool compareUnitToUnitType(CUnit* a1, __int16 a2) {
		static int result;
		static bool result2;

		__asm {
			PUSHAD
			PUSH a2
			PUSH a1
			CALL Func_compareUnitToUnitType
			MOV result, EAX
			POPAD
		}
		return result2 = (bool)result;
	};

	const u32 Func_getGroundHeightAtPx = 0x004BD0F0;
	unsigned int getGroundHeightAtPx(signed int a1, signed int a2) {
		static unsigned int result;

		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_getGroundHeightAtPx
			MOV result, EAX
			POPAD
		}
		return result;
	};
}