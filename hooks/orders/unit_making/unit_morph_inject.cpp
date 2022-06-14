#include <hook_tools.h>
#include <SCBW/api.h>
#include <cassert>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "globals.h"

namespace
{
	//-------- cmdrecv_unit_morph --------//

	const u32 Func_AddUnitToBuildQueue = 0x00467250;
	bool addUnitToBuildQueue(Unit* unit, u16 unit_id)
	{
		static u32 result;
		static u32 unitId_;
		unitId_ = unit_id;
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
		*unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection())
		{
			if (hooks::unit_can_morph(unit, morphUnitId)
				&& unit->main_order_id != OrderId::Morph1
				&& addUnitToBuildQueue(unit, morphUnitId))
			{
				unit->order_to(OrderId::Morph1);
			}
		}
		scbw::refresh_console();
	}

	//-------- BTNSCOND_CanBuildUnit --------//

	s32 __fastcall unitMorphWrapper_BTNSCOND_CanBuildUnit(u16 buildUnitId, s32 player, Unit* unit)
	{
		if (*client_selection_count <= 1
			|| hooks::get_unit_morph_egg_id(unit->id, unit, unit->build_queue[0]) != UnitId::None)
		{
			return unit->can_make_unit(buildUnitId, player);
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
		if (hooks::get_unit_morph_egg_id(unit->id, unit, unit->build_queue[0]) != UnitId::None)
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
		morphEggType = hooks::get_unit_morph_egg_id(unit->id, unit, unit->build_queue[0]);
		assert(scbw::is_zerg_egg_new(unit, true, false, morphEggType));
		__asm
		{
			POPAD
			PUSH morphEggType
			JMP Hook_Orders_Morph1_EggType_Return
		}
	}

	//-------- cancel_build --------//

	typedef void(__stdcall* CancelZergBuildingFunc)(Unit*);
	CancelZergBuildingFunc cancel_zerg_building = (CancelZergBuildingFunc)0x0045DA40;

	const u32 Func_ChangeUnitType = 0x0049FED0;
	void change_unit_type(Unit* unit, u16 newUnitId)
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
	void replace_sprite_images(Sprite* sprite, u16 imageId, u8 imageDirection)
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

	//-------- cancel_unit --------//

	void __fastcall cancel_unit_wrapper(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		if (unit->is_dead())
		{
			return;
		}

		if (unit->status & UnitStatus::Completed)
		{
			return;
		}

		if (unit->id == UnitId::ZergNydusCanal && unit->building.nydus_exit != nullptr)
		{
			return;
		}

		// Don't bother if unit is not morphed yet
		if (scbw::is_conventional_unit_morph_source(unit->id, false, false))
		{
			return;
		}

		// Don't bother if unit has finished morphing
		if (scbw::is_conventional_unit_morph_result(unit->id))
		{
			return;
		}

		if (unit->status & UnitStatus::GroundedBuilding)
		{
			if (unit->get_race() == RaceId::Zerg)
			{
				cancel_zerg_building(unit);
				return;
			}
			resources->minerals[unit->player] += units_dat::mineral_cost[unit->id] * 3 / 4;
			resources->gas[unit->player] += units_dat::gas_cost[unit->id] * 3 / 4;
		}
		else
		{
			u16 refundUnitId;
			if (scbw::is_zerg_egg_new(unit, true, false))
			{
				refundUnitId = unit->build_queue[unit->build_queue_slot % 5];
			}
			else
			{
				refundUnitId = unit->id;
			}
			resources->minerals[unit->player] += units_dat::mineral_cost[refundUnitId];
			resources->gas[unit->player] += units_dat::gas_cost[refundUnitId];
		}

		u16 cancelChangeUnitId = hooks::get_cancel_morph_revert_id(unit);
		if (cancelChangeUnitId == UnitId::None)
		{
			if (unit->id == UnitId::TerranNuclearMissile)
			{
				Unit* silo = unit->connected_unit;
				if (silo)
				{
					silo->building.silo.nuke = nullptr;
					silo->main_order_state = 0;
				}
				scbw::refresh_console();
			}
			unit->remove();
		}
		else
		{
			change_unit_type(unit, cancelChangeUnitId);
			unit->energy = unitExt->values[ValuePreMorphEnergy];
			unitExt->units[UnitMorphConnectedUnit] = unit->connected_unit;
			unit->remaining_build_time = 0;
			unit->build_queue[unit->build_queue_slot] = UnitId::None;
			
//			replace_sprite_images(unit->sprite, sprites_dat::ImageId[flingy_dat::SpriteID[units_dat::Graphic[unit->previous_unit_type]]], 0);			
//			auto index = units_dat::Graphic[unit->previous_unit_type];
			auto index = scbw::get_unit_data(UnitsDatEntries::Flingy, unit->previous_unit_type);
//			replace_sprite_images(unit->sprite, sprites_dat::ImageId[scbw::get_flingy_data(FlingyDatEntries::SpriteId, index)], 0);
			auto fl_sId = scbw::get_flingy_data(FlingyDatEntries::SpriteId, index);
//			scbw::get_sprite_data(SpritesDatEntries::ImageId,building->sprite->sprite_id),
			replace_sprite_images(unit->sprite, scbw::get_sprite_data(SpritesDatEntries::ImageId, fl_sId), 0);
			unit->order_signal &= ~0x4;
			if (unit->id == UnitId::ZergZethrokor
			||  unit->id == UnitId::ZergVorvrokor
			||  unit->id == UnitId::ZergNathrokor)
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
			unit->order_to(OrderId::ZergBirth);
		}
	}

	//-------- getRemainingBuildTimePct --------//

	s32 get_remaining_build_time_pct(Unit* unit)
	{
		u16 unit_id = unit->id;
		if (scbw::is_zerg_egg_new(unit, true, false) || unit->isRemorphingBuilding())
		{
			unit_id = unit->build_queue[unit->build_queue_slot];
		}
		return 100 * (units_dat::time_cost[unit_id] - unit->remaining_build_time) / units_dat::time_cost[unit_id];
	}

	// Inject @ 0x004669E0
	void __declspec(naked) get_remaining_build_time_pct_wrapper()
	{
		static Unit* unit;
		static s32 percentage;
		__asm {
			PUSHAD
			MOV unit, ESI
			MOV EBP, ESP
		}
		percentage = get_remaining_build_time_pct(unit);
		__asm {
			POPAD
			MOV EAX, percentage
			RETN
		}
	}

	//-------- orders_zerg_birth --------//

	// Inject @ 0x0045DE00
	const u32 Hook_GetUnitVerticalOffsetOnBirth_Return = 0x0045DE2C;
	void __declspec(naked) get_unit_vertical_offset_on_birth_wrapper()
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
	void __declspec(naked) is_rallyable_unit_egg_wrapper()
	{
		static Unit* unit;
		__asm
		{
			POP ESI
			POP EBX
			PUSHAD
			MOV unit, EDI
		}
		if (hooks::is_rallyable_egg_unit(unit->previous_unit_type))
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
	void __declspec(naked) orders_zerg_birth_wrapper()
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

	void __declspec(naked) orders_morph_1_wrapper()
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
	void __declspec(naked) delete_building_worker_ai_egg_jump_wrapper()
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
	void __declspec(naked) add_unit_ai_egg_jump_wrapper()
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

	void __declspec(naked) begin_train_wrapper()
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
	void __declspec(naked) detach_unit_task_egg_jump_wrapper()
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
	void __declspec(naked) prep_suicide_egg_jump_wrapper()
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
	void inject_unit_morph_hooks()
	{
		jump_patch(orders_zerg_birth_wrapper,					0x0045DD60, 1);
		jump_patch(orders_morph_1_wrapper,						0x0045DEA0, 0);
		call_patch(unitMorphWrapper_CMDRECV_UnitMorph,			0x00486B50, 0);		// jmp 004C1990  cmdrecv_unit_morph
		jump_patch(unitMorphWrapper_BTNSCOND_CanBuildUnit,		0x00428E60);		// 00428E60  BTNSCOND_CanBuildUnit
		jump_patch(unitMorphWrapper_Orders_Morph1_Check,		0x0045DFB0);		// 0045DEA0  orders_morph_1 within	
		jump_patch(unitMorphWrapper_Orders_Morph1_EggType,		0x0045E019);		// 0045DEA0  orders_morph_1 within
		jump_patch(cancel_unit_wrapper,							0x00468280);		// 00468280  ai_cancel_structure EXACT
		jump_patch(get_remaining_build_time_pct_wrapper,		0x004669E0);		// 004669E0  sub_4669E0 EXACT
		jump_patch(get_unit_vertical_offset_on_birth_wrapper,	0x0045DE00);		// 0045DD60  orders_zerg_birth within
		jump_patch(is_rallyable_unit_egg_wrapper,				0x0045DE57);		// 0045DD60  orders_zerg_birth within
		
		jump_patch(begin_train_wrapper,							0x00468200);
		jump_patch(detach_unit_task_egg_jump_wrapper,			0x00434CD0, 1);
		jump_patch(prep_suicide_egg_jump_wrapper,				0x0043D20C, 1);
		// has unfinished disabled bugged hook in aise

		jump_patch(delete_building_worker_ai_egg_jump_wrapper,	0x00434CD0, 1);
		jump_patch(add_unit_ai_egg_jump_wrapper,				0x00433E64, 1);
	}
}