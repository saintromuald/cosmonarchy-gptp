#include "hpbars.h"

namespace
{
}

namespace hooks
{
	void ShowHealthBar(Unit* unit)
	{
		if (unit == nullptr) return;
		helper::RemoveHPBar(unit->sprite->images.head);
		_unitTemp = unit;
		helper::CreateHealthBar(0, unit->sprite);
		_unitTemp = nullptr;
	}

	void compileHealthBar(Image* a1, Sprite* a2)
	{
		/*
		Sprite v2;// eax
		Unit* v3;	// edi
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

	Unit* _unitTemp;
}

namespace
{
}