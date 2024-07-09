#include <hook_tools.h>
#include <BW/api.h>
#include <cassert>
#include "hooks\orders\building_making\building_zerg.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "globals.h"
#include "math_utils.h"

namespace {
	//-------- cmdrecv_unit_morph --------//

	const u32 Func_AddUnitToBuildQueue = 0x00467250;
	bool add_unit_to_build_queue(Unit* unit, u16 unit_id) {
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

	void __stdcall unit_morph_wrapper_cmdrecv_unit_morph(u8* commandData) {
		const u16 morph_unit_id = *((u16*)&commandData[1]);
		*g_unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection())
		{
			if (hooks::unit_can_morph(unit, morph_unit_id)
				&& unit->main_order_id != OrderId::MorphUnit
				&& add_unit_to_build_queue(unit, morph_unit_id))
			{
				unit->order_to(OrderId::MorphUnit);
			}
		}
		bw::refresh_console();
	}

	//-------- BTNSCOND_CanBuildUnit --------//

	s32 __fastcall unit_morph_wrapper_btnscond_can_build_unit(u16 build_unit_id, s32 player, Unit* unit) {
		if (g_client_selection_count <= 1 || hooks::get_unit_morph_egg_id(unit->id, unit, unit->build_queue[0]) != UnitId::None) {
			return unit->can_make_unit(build_unit_id, player);
		}
		return 0;
	}

	//-------- Orders_Morph1 --------//

	const u32 Hook_Orders_Morph1_Check_Success = 0x0045DFCA;
	void __declspec(naked) unit_morph_wrapper_orders_morph_1_check() {
		static Unit* unit;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, ESI
		}
		if (hooks::get_unit_morph_egg_id(unit->id, unit, unit->build_queue[0]) != UnitId::None) {
			__asm {
				POPAD
				JMP Hook_Orders_Morph1_Check_Success
			}
		} else {
			__asm {
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
	void __declspec(naked) unit_morph_wrapper_orders_morph1_egg_type() {
		static Unit* unit;
		static u32 morphEggType;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, ESI
		}
		unit->status &= ~(UnitStatus::Completed);
		morphEggType = hooks::get_unit_morph_egg_id(unit->id, unit, unit->build_queue[0]);
		assert(bw::is_zerg_egg_new(unit, true, false, morphEggType));
		__asm {
			POPAD
			PUSH morphEggType
			JMP Hook_Orders_Morph1_EggType_Return
		}
	}

	//-------- cancel_build --------//

	typedef void(__stdcall* CancelZergBuildingFunc)(Unit*);
	CancelZergBuildingFunc cancel_zerg_building = (CancelZergBuildingFunc)0x0045DA40;

	const u32 Func_ChangeUnitType = 0x0049FED0;
	void change_unit_type(Unit* unit, u16 newUnitId) {
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
	void replace_sprite_images(Sprite* sprite, u16 imageId, u8 imageDirection) {
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

	void __fastcall cancel_unit_wrapper(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit->is_dead() || unit->status & UnitStatus::Completed || (unit->id == UnitId::ZergNydusCesiant && unit->building.nydus_exit)) {
			return;
		}

		if (bw::is_conventional_unit_morph_source(unit->id, false) || bw::is_conventional_unit_morph_result(unit->id)) {
			return;
		}

		if (unit->status & UnitStatus::GroundedBuilding) {
			if (unit->get_race() == RaceId::Zerg) {
				if (unit->id == UnitId::ZergExcisant) {
					unit->building.resource.is_mining = 0;
				}
				cancel_zerg_building(unit);
				if (unit->rally.pt.x || unit->rally.pt.y) {
					unit->rally.pt.x = 0;
					unit->rally.pt.y = 0;
				}
				return;
			}
			resources->minerals[unit->player] += units_dat::mineral_cost[unit->id] * 3 / 4;
			resources->gas[unit->player] += units_dat::vespene_cost[unit->id] * 3 / 4;
		} else {
			u16 refund_unit_id;
			if (bw::is_zerg_egg_new(unit, true, false)) {
				refund_unit_id = unit->build_queue[unit->build_queue_slot % 5];
			} else {
				refund_unit_id = unit->id;
			}
			resources->minerals[unit->player] += units_dat::mineral_cost[refund_unit_id];
			resources->gas[unit->player] += units_dat::vespene_cost[refund_unit_id];
		}

		u16 cancel_change_unit_id = hooks::get_cancel_morph_revert_id(unit);
		if (cancel_change_unit_id == UnitId::None) {
			if (unit_id_is_gas_structure(unit->id)) {
				unit->building.resource.is_mining = 0;
			}
			switch (unit->get_race()) {
				case RaceId::Protoss: {
					if (units_dat::base_property[unit->id] & UnitProperty::Building) {
						helper::play_sound_from_unit(SoundId::CancelStructureProtoss, unit, 1, 0);
						helper::create_thingy(SpriteId::CuirassStonewallProtocolOverlay, unit->position.x, unit->position.y, unit->player);
					}
					unit->remove();
				} break;
				case RaceId::Terran: {
					if (unit->id == UnitId::TerranMorningstar) {
						Unit* silo = unit->connected_unit;
						if (silo) {
							silo->building.silo.nuke = 0;
							silo->main_order_state = 0;
						}
						unit->remove();
						bw::refresh_console();
					} else if (units_dat::base_property[unit->id] & UnitProperty::Building) {
						helper::play_sound_from_unit(SoundId::Misc_Explo1_wav, unit, 1, 0);
						helper::create_thingy(SpriteId::CuirassStonewallProtocolOverlay, unit->position.x, unit->position.y, unit->player); // TODO: Find working Terran-themed overlay
						unit->remove();
					} else {
						unit->remove();
					}
				} break;
				default: {
					unit->remove();
				}
			}
		} else {
			change_unit_type(unit, cancel_change_unit_id);
			unit->energy = unit_ext->values[ValuePreMorphEnergy];
			unit_ext->units[UnitMorphConnectedUnit] = unit->connected_unit;
			unit->remaining_build_time = 0;
			unit->build_queue[unit->build_queue_slot] = UnitId::None;
			
			u32 index = units_dat::graphic[unit->previous_unit_type];
			u32 flingy_sprite_id = flingy_dat::sprite_id[index];
			replace_sprite_images(unit->sprite, sprites_dat::image_id[flingy_sprite_id], 0);
			unit->order_signal &= ~0x4;
			if (unit->id == UnitId::ZergZethrokor
			||  unit->id == UnitId::ZergVorvrokor
			||  unit->id == UnitId::ZergNathrokor)
			{
				unit->sprite->play_iscript_animation(IscriptAnimation::Unused2, true);
			} else if (unit->id == UnitId::ZergQuazilisk) {
				unit->sprite->play_iscript_animation(IscriptAnimation::Unused2, true);
				unit_ext->timers[TimerQuaziliskCancel] = 4;
			} else {
				unit->sprite->play_iscript_animation(IscriptAnimation::SpecialState2, true);
			}
			unit->rally.pt = unit->position;
			unit->rally.unit = 0;
			unit->order_to(OrderId::ZergBirth);
		}
	}

	//-------- getRemainingBuildTimePct --------//

	s32 get_remaining_build_time_pct(Unit* unit) {
		u16 unit_id = unit->id;
		if (bw::is_zerg_egg_new(unit, true, false) || unit->is_remorphing_building()) {
			unit_id = unit->build_queue[unit->build_queue_slot];
		}
		return 100 * (units_dat::time_cost[unit_id] - unit->remaining_build_time) / units_dat::time_cost[unit_id];
	}

	// Inject @ 0x004669E0
	void __declspec(naked) get_remaining_build_time_pct_wrapper() {
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
	void __declspec(naked) get_unit_vertical_offset_on_birth_wrapper() {
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
	void __declspec(naked) is_rallyable_unit_egg_wrapper() {
		static Unit* unit;
		__asm {
			POP ESI
			POP EBX
			PUSHAD
			MOV unit, EDI
		}
		if (hooks::is_rallyable_egg_unit(unit->previous_unit_type)) {
			__asm {
				POPAD
				JMP Hook_IsRallyableEggUnit_Yes
			}
		} else {
			__asm {
				POPAD
				JMP Hook_IsRallyableEggUnit_No
			}
		}
	}
}

namespace {
	void __declspec(naked) orders_zerg_birth_wrapper() {
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

	void __declspec(naked) orders_morph_1_wrapper() {
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

	void __declspec(naked) unit_completion_init_wrapper() { // 004E65E0
		static Unit* unit;
		static u32 has_construction_animation;
		static Image* result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV has_construction_animation, EAX
			POP EAX
			MOV unit, EDI
			PUSHAD
		}
		result = hooks::unit_completion_init(unit, has_construction_animation);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			MOV EAX, result
			RETN 0x04
		}
	}

	const u32 Jmp_IsEgg = 0x00434CF1;
	const u32 Jmp_IsNotEgg = 0x00434CD6;
	void __declspec(naked) delete_building_worker_ai_egg_jump_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EDI
			PUSHAD
		}
		if (bw::is_larval_egg(unit)) {
			__asm {
				POPAD
				JMP Jmp_IsEgg
			}
		} else {
			__asm {
				POPAD
				JMP Jmp_IsNotEgg
			}
		}
	}

	const u32 Jmp_UnitAi_IsEgg = 0x00433E74;
	const u32 Jmp_UnitAi_IsNotEgg = 0x00433E6A;
	void __declspec(naked) add_unit_ai_egg_jump_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EBX
			PUSHAD
		}
		if (bw::is_larval_egg(unit)) {
			__asm {
				POPAD
				JMP Jmp_UnitAi_IsEgg
			}
		} else {
			__asm {
				POPAD
				JMP Jmp_UnitAi_IsNotEgg
			}
		}
	}

	void __declspec(naked) begin_train_wrapper() {
		static u32 notify_error;
		static Unit* unit;
		static Unit* result;
		static u16 unit_id;
		__asm {
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
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	const u32 Jmp_UnitTask_IsEgg = 0x00434CF1;
	const u32 Jmp_UnitTask_IsNotEgg = 0x00434CD6;
	void __declspec(naked) detach_unit_task_egg_jump_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EDI
			PUSHAD
		}
		if (bw::is_larval_egg(unit)) {
			__asm {
				POPAD
				JMP Jmp_UnitTask_IsEgg
			}
		} else {
			__asm {
				POPAD
				JMP Jmp_UnitTask_IsEgg
			}
		}
	}

	const u32 Jmp_PrepSuicide_IsEgg = 0x0043D218;
	const u32 Jmp_PrepSuicide_IsNotEgg = 0x0043D212;
	void __declspec(naked) prep_suicide_egg_jump_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		if (bw::is_larval_egg(unit)) {
			__asm {
				POPAD
				JMP Jmp_PrepSuicide_IsEgg
			}
		} else {
			__asm {
				POPAD
				JMP Jmp_PrepSuicide_IsEgg
			}
		}
	}
}

namespace hooks {
	void inject_unit_morph_hooks() {
		jump_patch(orders_zerg_birth_wrapper,					0x0045DD60, 1);
		jump_patch(orders_morph_1_wrapper,						0x0045DEA0, 0);
		call_patch(unit_morph_wrapper_cmdrecv_unit_morph,		0x00486B50, 0);		// jmp	004C1990  cmdrecv_unit_morph
		jump_patch(unit_morph_wrapper_btnscond_can_build_unit,	0x00428E60, 0);		//		00428E60  BTNSCOND_CanBuildUnit
		jump_patch(unit_morph_wrapper_orders_morph_1_check,		0x0045DFB0, 0);		//		0045DEA0  orders_morph_1 within	
		jump_patch(unit_morph_wrapper_orders_morph1_egg_type,	0x0045E019, 0);		//		0045DEA0  orders_morph_1 within
		jump_patch(cancel_unit_wrapper,							0x00468280, 0);		//		00468280  ai_cancel_structure EXACT
		jump_patch(get_remaining_build_time_pct_wrapper,		0x004669E0, 0);		//		004669E0  sub_4669E0 EXACT
		jump_patch(get_unit_vertical_offset_on_birth_wrapper,	0x0045DE00, 0);		//		0045DD60  orders_zerg_birth within
	//	jump_patch(is_rallyable_unit_egg_wrapper,				0x0045DE57, 0);		//		0045DD60  orders_zerg_birth within
		jump_patch(unit_completion_init_wrapper,				0x004E65E0, 1);
		
		jump_patch(begin_train_wrapper,							0x00468200);
		jump_patch(detach_unit_task_egg_jump_wrapper,			0x00434CD0, 1);
		jump_patch(prep_suicide_egg_jump_wrapper,				0x0043D20C, 1);

		jump_patch(delete_building_worker_ai_egg_jump_wrapper,	0x00434CD0, 1);
		jump_patch(add_unit_ai_egg_jump_wrapper,				0x00433E64, 1);
	}
}