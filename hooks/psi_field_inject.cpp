#include "psi_field.h"
#include <hook_tools.h>
#include <SCBW/api.h>

//-------- Unit selection hooks --------//

void showAllPsiFields()
{
	static const u32 Func_showAllPsiFields = 0x00493640;
	__asm {
		PUSHAD
		CALL Func_showAllPsiFields
		POPAD
	}
}

void hideAllPsiFields()
{
	static const u32 Func_hideAllPsiFields = 0x00494100;
	__asm {
		PUSHAD
		CALL Func_hideAllPsiFields
		POPAD
	}
}

// If @p unit is a psi provider, make all psi field graphics visible.
void showAllPsiFieldsOnSelect(Unit* unit)
{
	if (!scbw::isInReplay() && unit->player == *LOCAL_NATION_ID)
	{
		if ((unit->status & UnitStatus::Completed) && hooks::isReadyToMakePsiField(unit))
		{
			showAllPsiFields();
		}
	}
}

// If @p unit is a psi provider, make all psi field graphics invisible.
void hideAllPsiFieldsOnUnselect(Unit* unit)
{
	if (!scbw::isInReplay() && unit->player == *LOCAL_NATION_ID)
	{
		if ((unit->status & UnitStatus::Completed) && hooks::isReadyToMakePsiField(unit))
		{
			hideAllPsiFields();
		}
	}
}

void __declspec(naked) showAllPsiFieldsOnSelectWrapper() // 0x004E6224
{
	static Unit* unit;
	__asm {
		MOV unit, EDI
		PUSHAD
	}
	showAllPsiFieldsOnSelect(unit);
	__asm {
		POPAD
		POP EDI
		POP ESI
		POP EBX
		MOV ESP, EBP
		POP EBP
		RETN 4
	}
}

void __declspec(naked) hideAllPsiFieldsOnUnselectWrapper() // 0x004E62BE
{
	static Unit* unit;
	__asm {
		MOV unit, ESI
		PUSHAD
	}
	hideAllPsiFieldsOnUnselect(unit);
	__asm {
		POPAD
		POP EDI	// used to be in first asm block
		POP ESI
		RETN
	}
}

//-------- Save & load game hooks --------//

void __declspec(naked) packUnitData_PsiProvider() // 0x004E3935
{
	static const u32 packUnitData_PsiProvider_Return = 0x004E3961;
	static Unit* unit;
	static u16 unitId;
	__asm {
		MOV unitId, CX
		MOV unit, ESI
		PUSHAD
	}
	if (hooks::canMakePsiField(unitId))
	{
		if (unit->building.pylonAura != nullptr)
			*((u32*)&unit->building.pylonAura) = (unit->building.pylonAura - spriteTable) / sizeof(Sprite) + 1;
	}
	__asm {
		POPAD
		JMP packUnitData_PsiProvider_Return
	}
}

void __declspec(naked) unpackUnitData_PsiProvider() //0x004E3182
{
	static const u32 unpackUnitData_PsiProvider_Return = 0x004E31A3;
	static Unit* unit;
	static u16 unitId;
	__asm {
		MOV unitId, CX
		MOV unit, ESI
		PUSHAD
	}
	if (hooks::canMakePsiField(unitId)) {
		if (unit->building.pylonAura != nullptr)
		{
			unit->building.pylonAura = &spriteTable[(s32)unit->building.pylonAura];
		}
	}
	__asm {
		POPAD
		JMP unpackUnitData_PsiProvider_Return
	}
}

namespace hooks
{
	void injectPsiFieldHooks()
	{
		jump_patch(showAllPsiFieldsOnSelectWrapper,		0x004E6224, 0);
		jump_patch(hideAllPsiFieldsOnUnselectWrapper,	0x004E62BE, 0);
		jump_patch(packUnitData_PsiProvider,			0x004E3935, 0);
		jump_patch(unpackUnitData_PsiProvider,			0x004E3182, 0);
	}
}
