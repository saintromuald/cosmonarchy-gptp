#include "hpbars.h"

namespace {
	CImage* CreateHealthBar(int a1, CSprite* a2);
	void RemoveHPBar(CImage* a1);
	void updateImageDrawData(CImage* result);
	void playImageIscript(CImage* img, u8 iscriptAnim);
}

namespace hooks {
	void ShowHealthBar(CUnit* unit) {
		if (unit == nullptr) return;
		RemoveHPBar(unit->sprite->images.head);
		_unitTemp = unit;
		CreateHealthBar(0, unit->sprite);
		_unitTemp = nullptr;
	}

	void compileHealthBar(CImage* a1, CSprite* a2) {
		/*
		CSprite v2;// eax
		CUnit* v3;	// edi
		u8 v4;		// cl
		s32 v5;		// eax
		u16 v6;		// ax
		s32 v7;		// eax
		u8 v8;		// cl
		u8 v9;		// cl
		u32* v10;	// edx

		v2 = *(a2 + 8);
		if (_unitTemp != nullptr) {
			v3 = _unitTemp;
		} else {
			v3 = CurrentUnitSelection[*(a2 + 11)];
		}
		v4 = sprites_dat::IsVisible[v2 + 390];
		*(a1 + 15) = sprites_dat::HpBarSize[v2 + 262]
			+ (imageGrpGraphics[orders_dat::Unused3[v2 + 62] + 561]->frames[0].height >> 1)
			+ 8;
		*(*(a1 + 44) + 6) = 0;
		*(*(a1 + 44) + 7) = 0;
		*(*(a1 + 44) + 8) = v4 - (v4 - 1) % 3;
		v5 = *(a1 + 44);
		if (*(v5 + 8) < 0x13u)
			*(v5 + 8) = 19;
		*(*(a1 + 44) + 9) = 0;
		v6 = v3->id;
		if (units_dat::BaseProperty[v6] & 0x200000 && !(v3->status & 0x40000000)
		  || v3->status & 0x40000000
		  || v6 == 40) {
			*(*(a1 + 44) + 9) += 6;
		}
		v7 = *(a1 + 44);
		v8 = *(v7 + 9);
		if (units_dat::ShieldsEnabled[v3->id])
			v9 = v8 + 7;
		else
			v9 = v8 + 5;
		*(v7 + 9) = v9;
		*(*(a1 + 44) + 2) = *(*(a1 + 44) + 8);
		*(*(a1 + 44) + 4) = *(*(a1 + 44) + 9);
		*(a1 + 60) = a2;
		*(a1 + 12) = 0;
		v10 = (a1 + 36);
		*v10 = 0;
		v10[1] = 0;
		*(a1 + 48) = v3;
		updateImageDrawData(a1);
		*(a1 + 12) |= 1u;
		playImageIscript(a1, 0);
	*/
	}

	CUnit* _unitTemp;
}

namespace {
	const u32 CreateHealthBar_function = 0x004D6420;
	CImage* CreateHealthBar(int a1, CSprite* a2) {
		static CImage* result;
		_asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL CreateHealthBar_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 RemoveHPBar_function = 0x004D5030;
	void RemoveHPBar(CImage* a1) {
		_asm {
			PUSHAD
			MOV ESI, a1
			CALL RemoveHPBar_function
			POPAD
		}
	}

	const u32 updateImageDrawData_function = 0x004D57B0;
	void updateImageDrawData(CImage* result) {
		_asm {
			PUSHAD
			MOV EAX, result
			CALL updateImageDrawData_function
			POPAD
		}
	}

	const u32 playImageIscript_function = 0x004D8470;
	void playImageIscript(CImage* img, u8 iscriptAnim) {
		_asm {
			PUSHAD
			PUSH iscriptAnim
			MOV ECX, img
			CALL updateImageDrawData_function
			POPAD
		}
	}
}