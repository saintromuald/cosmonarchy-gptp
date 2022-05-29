#include <hook_tools.h>
#include <SCBW/api.h>
#include <cassert>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "globals.h"

namespace
{
	//-------- CMDRECV_UnitMorph --------//

	const u32 Func_AddUnitToBuildQueue = 0x00467250;
	bool addUnitToBuildQueue(Unit* unit, u16 unitId)
	{
		static u32 result;
		static u32 unitId_;
		unitId_ = unitId;
		__asm
		{
			PUSHAD
			PUSH unitId_
			MOV EDI, unit
			CALL Func_AddUnitToBuildQueue
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	void __stdcall unitMorphWrapper_CMDRECV_UnitMorph(u8* commandData)
	{
		const u16 morphUnitId = *((u16*)&commandData[1]);
		*selectionIndexStart = 0;
		while (Unit* unit = getActivePlayerNextSelection())
		{
			if (hooks::unit_can_morph(unit, morphUnitId)
				&& unit->mainOrderId != OrderId::Morph1
				&& addUnitToBuildQueue(unit, morphUnitId))
			{
				unit->orderTo(OrderId::Morph1);
			}
		}
		scbw::refresh_console();
	}

	//-------- BTNSCOND_CanBuildUnit --------//

	s32 __fastcall unitMorphWrapper_BTNSCOND_CanBuildUnit(u16 buildUnitId, s32 playerId, Unit* unit)
	{
		if (*clientSelectionCount <= 1
			|| hooks::get_unit_morph_egg_id(unit->id, unit, unit->buildQueue[0]) != UnitId::None)
		{
			return unit->can_make_unit(buildUnitId, playerId);
		}
		return 0;
	}

	//-------- Orders_Morph1 --------//

	const u32 Hook_Orders_Morph1_Check_Success = 0x0045DFCA;
	void __declspec(naked) unitMorphWrapper_Orders_Morph1_Check()
	{
		static Unit* unit;
		__asm
		{
			PUSHAD
			MOV EBP, ESP
			MOV unit, ESI
		}
		if (hooks::get_unit_morph_egg_id(unit->id, unit, unit->buildQueue[0]) != UnitId::None)
		{
			__asm
			{
				POPAD
				JMP Hook_Orders_Morph1_Check_Success
			}
		}
		else
		{
			__asm
			{
				POPAD
				POP EDI
				POP ESI
				MOV ESP, EBP
				POP EBP
				RETN
			}
		}
	}

	const u32 Hook_Orders_Morph1_EggType_Return = 0x0045E048;
	void __declspec(naked) unitMorphWrapper_Orders_Morph1_EggType()
	{
		static Unit* unit;
		static u32 morphEggType;
		__asm
		{
			PUSHAD
			MOV EBP, ESP
			MOV unit, ESI
		}
		unit->status &= ~(UnitStatus::Completed);
		morphEggType = hooks::get_unit_morph_egg_id(unit->id, unit, unit->buildQueue[0]);
		assert(scbw::is_zerg_egg_new(unit, true, false, morphEggType));
		__asm
		{
			POPAD
			PUSH morphEggType
			JMP Hook_Orders_Morph1_EggType_Return
		}
	}

	//-------- cancelBuild --------//

	typedef void(__stdcall* CancelZergBuildingFunc)(Unit*);
	CancelZergBuildingFunc cancelZergBuilding = (CancelZergBuildingFunc)0x0045DA40;

	const u32 Func_ChangeUnitType = 0x0049FED0;
	void changeUnitType(Unit* unit, u16 newUnitId)
	{

		static u32 newUnitId_ = newUnitId;

		newUnitId_ = newUnitId;

		__asm {
			PUSHAD
			PUSH newUnitId_
			MOV EAX, unit
			CALL Func_ChangeUnitType
			POPAD
		}

	}

	const u32 Func_ReplaceSpriteImages = 0x00499BB0;
	void replaceSpriteImages(Sprite* sprite, u16 imageId, u8 imageDirection)
	{

		static u32 imageId_;
		static u32 imageDirection_;

		imageId_ = imageId;
		imageDirection_ = imageDirection;

		__asm {
			PUSHAD
			PUSH imageDirection_
			PUSH imageId_
			MOV EAX, sprite
			CALL Func_ReplaceSpriteImages
			POPAD
		}

	}

	//-------- cancelUnit --------//

	void __fastcall cancelUnitWrapper(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		if (unit->isDead())
		{
			return;
		}

		if (unit->status & UnitStatus::Completed)
		{
			return;
		}

		if (unit->id == UnitId::ZergNydusCanal && unit->building.nydusExit != nullptr)
		{
			return;
		}

		// Don't bother if unit is not morphed yet
		if (scbw::is_conventional_unit_morph_source(unit->id, false, false))
		{
			return;
		}

		// Don't bother if unit has finished morphing
		if (scbw::isConventionalUnitMorphResult(unit->id))
		{
			return;
		}

		if (unit->status & UnitStatus::GroundedBuilding)
		{
			if (unit->getRace() == RaceId::Zerg)
			{
				cancelZergBuilding(unit);
				return;
			}
			resources->minerals[unit->player] += units_dat::MineralCost[unit->id] * 3 / 4;
			resources->gas[unit->player] += units_dat::GasCost[unit->id] * 3 / 4;
		}
		else
		{
			u16 refundUnitId;
			if (scbw::is_zerg_egg_new(unit, true, false))
			{
				refundUnitId = unit->buildQueue[unit->buildQueueSlot % 5];
			}
			else
			{
				refundUnitId = unit->id;
			}
			resources->minerals[unit->player] += units_dat::MineralCost[refundUnitId];
			resources->gas[unit->player] += units_dat::GasCost[refundUnitId];
		}

		u16 cancelChangeUnitId = hooks::get_cancel_morph_revert_id(unit);
//		scbw::printf("Revert unit id: %d", cancelChangeUnitId);
		if (cancelChangeUnitId == UnitId::None)
		{
//			scbw::print("Remove the unit");
			if (unit->id == UnitId::TerranNuclearMissile)
			{
				Unit* silo = unit->connectedUnit;
				if (silo)
				{
					silo->building.silo.nuke = nullptr;
					silo->mainOrderState = 0;
				}
				scbw::refresh_console();
			}
			unit->remove();
		}
		else
		{
			changeUnitType(unit, cancelChangeUnitId);
			unit->energy = unitExt->values[ValuePreMorphEnergy];
			unit->remainingBuildTime = 0;
			unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
			
//			replaceSpriteImages(unit->sprite, sprites_dat::ImageId[flingy_dat::SpriteID[units_dat::Graphic[unit->previousUnitType]]], 0);			
//			auto index = units_dat::Graphic[unit->previousUnitType];
			auto index = scbw::getUnitData(UnitsDatEntries::Flingy, unit->previousUnitType);
//			replaceSpriteImages(unit->sprite, sprites_dat::ImageId[scbw::getFlingyData(FlingyDatEntries::SpriteId, index)], 0);
			auto fl_sId = scbw::getFlingyData(FlingyDatEntries::SpriteId, index);
//			scbw::getSpriteData(SpritesDatEntries::ImageId,building->sprite->spriteId),
			replaceSpriteImages(unit->sprite, scbw::getSpriteData(SpritesDatEntries::ImageId, fl_sId), 0);
			unit->orderSignal &= ~0x4;
			if (unit->id == UnitId::ZergZethrokor
				|| unit->id == UnitId::ZergVorvrokor)
			{
				unit->play_iscript_animation(IscriptAnimation::Unused2);
			}
			else if (unit->id == UnitId::ZergNathrokor)
			{
				unit->play_iscript_animation(IscriptAnimation::Unused2);
			}
			else if (unit->id == UnitId::ZergQuazilisk)
			{
				unit->play_iscript_animation(IscriptAnimation::CastSpell);
				unitExt->timers[TimerQuaziliskCancel] = 4;
			}
			else
			{
				unit->play_iscript_animation(IscriptAnimation::SpecialState2);
			}
			unit->orderTo(OrderId::ZergBirth);
		}
	}

	//-------- getRemainingBuildTimePct --------//

	s32 getRemainingBuildTimePctHook(Unit* unit)
	{
		u16 unitId = unit->id;
		if (scbw::is_zerg_egg_new(unit, true, false) || unit->isRemorphingBuilding())
		{
			unitId = unit->buildQueue[unit->buildQueueSlot];
		}
		return 100 * (units_dat::TimeCost[unitId] - unit->remainingBuildTime) / units_dat::TimeCost[unitId];
	}

	// Inject @ 0x004669E0
	void __declspec(naked) getRemainingBuildTimePctWrapper()
	{
		static Unit* unit;
		static s32 percentage;
		__asm {
			PUSHAD
			MOV unit, ESI
			MOV EBP, ESP
		}
		percentage = getRemainingBuildTimePctHook(unit);
		__asm {
			POPAD
			MOV EAX, percentage
			RETN
		}
	}

	//-------- orders_zergBirth --------//

	// Inject @ 0x0045DE00
	const u32 Hook_GetUnitVerticalOffsetOnBirth_Return = 0x0045DE2C;
	void __declspec(naked) getUnitVerticalOffsetOnBirthWrapper()
	{
		static Unit* unit;
		static s16 yOffset;
		__asm {
			PUSHAD
			MOV unit, EDI
		}
		yOffset = hooks::get_unit_vertical_offset_on_birth(unit);
		__asm {
			POPAD
			MOVSX EAX, yOffset
			JMP Hook_GetUnitVerticalOffsetOnBirth_Return
		}
	}

	// Inject @ 0x0045DE57
	const u32 Hook_IsRallyableEggUnit_Yes = 0x0045DE6C;
	const u32 Hook_IsRallyableEggUnit_No = 0x0045DE8B;
	void __declspec(naked) isRallyableEggUnitWrapper()
	{
		static Unit* unit;
		__asm
		{
			POP ESI
			POP EBX
			PUSHAD
			MOV unit, EDI
		}
		if (hooks::is_rallyable_egg_unit(unit->previousUnitType))
		{
			__asm
			{
				POPAD
				JMP Hook_IsRallyableEggUnit_Yes
			}
		}
		else
		{
			__asm
			{
				POPAD
				JMP Hook_IsRallyableEggUnit_No
			}
		}
	}
}

namespace
{
	void __declspec(naked) orders_ZergBirthWrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_zerg_birth(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_Morph1Wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_morph_1(unit);
		__asm {
			POPAD
			RETN
		}
	}

	const u32 Jmp_IsEgg = 0x00434CF1;
	const u32 Jmp_IsNotEgg = 0x00434CD6;
	void __declspec(naked) deleteBuildingWorkerAiEggJmpWrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EDI
			PUSHAD
		}
		if (scbw::is_larval_egg(unit))
		{
			__asm {
				POPAD
				JMP Jmp_IsEgg
			}
		}
		else
		{
			__asm {
				POPAD
				JMP Jmp_IsNotEgg
			}
		}
	}

	const u32 Jmp_UnitAi_IsEgg = 0x00433E74;
	const u32 Jmp_UnitAi_IsNotEgg = 0x00433E6A;
	void __declspec(naked) addUnitAiEggJmpWrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EBX
			PUSHAD
		}
		if (scbw::is_larval_egg(unit))
		{
			__asm
			{
				POPAD
				JMP Jmp_UnitAi_IsEgg
			}
		}
		else
		{
			__asm
			{
				POPAD
				JMP Jmp_UnitAi_IsNotEgg
			}
		}
	}

	void __declspec(naked) beginTrainWrapper()
	{
		static u32 notify_error;
		static Unit* unit;
		static Unit* result;
		static u16 unit_id;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit, ESI
			MOV unit_id, DI
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV notify_error, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::begin_train(unit, unit_id, notify_error);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	const u32 Jmp_UnitTask_IsEgg = 0x00434CF1;
	const u32 Jmp_UnitTask_IsNotEgg = 0x00434CD6;
	void __declspec(naked) detachUnitTask_EggJmpWrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EDI
			PUSHAD
		}
		if (scbw::is_larval_egg(unit))
		{
			__asm
			{
				POPAD
				JMP Jmp_UnitTask_IsEgg
			}
		}
		else
		{
			__asm
			{
				POPAD
				JMP Jmp_UnitTask_IsEgg
			}
		}
	}

	const u32 Jmp_PrepSuicide_IsEgg = 0x0043D218;
	const u32 Jmp_PrepSuicide_IsNotEgg = 0x0043D212;
	void __declspec(naked) prepSuicide_EggJmpWrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		if (scbw::is_larval_egg(unit))
		{
			__asm
			{
				POPAD
				JMP Jmp_PrepSuicide_IsEgg
			}
		}
		else
		{
			__asm
			{
				POPAD
				JMP Jmp_PrepSuicide_IsEgg
			}
		}
	}
}

namespace hooks
{
	void injectUnitMorphHooks()
	{
		jump_patch(orders_ZergBirthWrapper,					0x0045DD60, 1);
		jump_patch(orders_Morph1Wrapper,					0x0045DEA0, 0);
		call_patch(unitMorphWrapper_CMDRECV_UnitMorph,		0x00486B50);		// jmp 004C1990  CMDRECV_UnitMorph
		jump_patch(unitMorphWrapper_BTNSCOND_CanBuildUnit,	0x00428E60);		// 00428E60  BTNSCOND_CanBuildUnit
		jump_patch(unitMorphWrapper_Orders_Morph1_Check,	0x0045DFB0);		// 0045DEA0  orders_morph_1 within	
		jump_patch(unitMorphWrapper_Orders_Morph1_EggType,	0x0045E019);		// 0045DEA0  orders_morph_1 within
		jump_patch(cancelUnitWrapper,						0x00468280);		// 00468280  AI_cancel_structure EXACT
		jump_patch(getRemainingBuildTimePctWrapper,			0x004669E0);		// 004669E0  sub_4669E0 EXACT
		jump_patch(getUnitVerticalOffsetOnBirthWrapper,		0x0045DE00);		// 0045DD60  orders_zerg_birth within
		jump_patch(isRallyableEggUnitWrapper,				0x0045DE57);		// 0045DD60  orders_zerg_birth within
		
		jump_patch(beginTrainWrapper,						0x00468200);
		jump_patch(detachUnitTask_EggJmpWrapper,			0x00434CD0, 1);
		jump_patch(prepSuicide_EggJmpWrapper,				0x0043D20C, 1);
		// has unfinished disabled bugged hook in aise

		jump_patch(deleteBuildingWorkerAiEggJmpWrapper,		0x00434CD0, 1);
		jump_patch(addUnitAiEggJmpWrapper,					0x00433E64, 1);
	}
}