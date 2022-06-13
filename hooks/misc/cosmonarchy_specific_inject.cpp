#include "hooks/misc/cosmonarchy_specific.h"
#include "globals.h"

namespace hooks
{
	void __declspec(naked) get_flingy_halt_distance_wrapper()
	{
		static Flingy* flingy;
		static u32 result;
		__asm {
			MOV flingy, ECX
			PUSHAD
		}
		result = hooks::get_flingy_halt_distance(flingy);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	const u32 jmp_00496390 = 0x00496390;
	void __declspec(naked) initialize_flingy_96377_jump_wrapper()
	{
		static u32 topSpeed;
		static u32 flingy_id;
		static u16 acceleration;
		static Flingy* flingy;
		__asm {
			PUSH EBX
			XOR EBX, EBX
			MOV[ESI + 0x3C], EBX
			POP EBX
			MOV flingy_id, EAX
			MOV flingy, ESI
			PUSHAD
		}
		topSpeed = hooks::real_top_speed(flingy, flingy_id);
		acceleration = hooks::real_acceleration(flingy, flingy_id);
		globals.flingy_init_bullet_id = -1;
		__asm {
			POPAD
			MOV ECX, topSpeed
			MOV[ESI + 0x34], ECX
			MOV CX, acceleration
			JMP jmp_00496390
		}
	}

	const u32 jmp_8BEDE = 0x0048BEDE;
	void __declspec(naked) initialize_bullet_8BED7_jump_wrapper()
	{
		static u32 weapon_id;
		static u32 player_id;
		static u32 result;
		__asm {
			MOV weapon_id, EAX
			MOV player_id, EBX
			PUSHAD
		}
		result = weapon_flingy(weapon_id, player_id);
		__asm {
			POPAD
			MOV EAX, result
			JMP jmp_8BEDE;
		}
	}

	void flingy_speed_patches() // for madrigals
	{
		jump_patch(get_flingy_halt_distance_wrapper,		0x00494F90);
		jump_patch(initialize_flingy_96377_jump_wrapper,	0x00496377, 2);
		jump_patch(initialize_bullet_8BED7_jump_wrapper,	0x0048BED7, 2);
	}

	const u32 jmp_004295F8 = 0x004295F8;
	const u32 jmp_004295F3 = 0x004295F3;
	void __declspec(naked) wrapper_295EB()
	{
		static u16 id;
		__asm {
			MOV id, DI
			PUSHAD
		}
		if (!hooks::is_high_templar_conduit(id))
		{
			__asm {
				POPAD
				MOV AL, 0x6A
				JMP jmp_004295F8
			}
		}
		else
		{
			__asm {
				POPAD
				MOV AL, 0x6A
				JMP jmp_004295F3
			}
		}

	}

	const u32 func_getTargetSomething = 0x0043FF90;
	const u32 progressOrderSwitch_e71b = 0x004EC71B;
	const u32 spellOrderCases = 0x004ECADB;
	void __declspec(naked) unused_order_patch_wrapper()
	{

		static u8 order_id;
		__asm {
			MOV[ESI + 0x85], 0x8
			CALL func_getTargetSomething
			MOV AL, [ESI + 0x4D]
			MOV order_id, AL
			MOVZX EAX, AL
			PUSHAD
		}
		if (hooks::is_unused_order(order_id))
		{
			__asm {
				POPAD
				JMP spellOrderCases
			}
		}
		else
		{
			__asm {
				POPAD
				JMP progressOrderSwitch_e71b
			}
		}
	}

	const u32 jmp_004A0682 = 0x004A0682;
	void __declspec(naked) lobotomy_mine_jump_wrapper()
	{
		Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		set_up_spider_mines(unit);
		__asm {
			POPAD
			JMP jmp_004A0682
		}

	}

	const u32 jmp_0046512B = 0x0046512B;
	void orders_place_mine_jump_wrapper_1()
	{
		Unit* unit;
		u32 result;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::get_mine_id(unit);
		__asm {
			POPAD
			MOV EAX, result
			JMP jmp_0046512B
		}
	}

	const u32 jmp_0046514A = 0x0046514A;
	void orders_place_mine_jump_wrapper_2()
	{
		Unit* unit;
		u32 result;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::get_mine_id(unit);
		__asm {
			POPAD
			MOV ECX, result
			JMP jmp_0046514A
		}
	}

	void unused_order_patches()
	{
		jump_patch(unused_order_patch_wrapper, 0x004EC70B, 7); // turns unused orders to spell orders
		// later add jump patches to make CTFCop2 and WarpIn useless
	}

	void archon_hardcode_patches() // for augurs
	{
		jump_patch(wrapper_295EB, 0x004295EB, 1);
	}

	void lobotomy_mine_patches()
	{
		jump_patch(lobotomy_mine_jump_wrapper, 0x004A066B, 3);  /*
		jump_patch(orders_place_mine_jump_wrapper_1, 0x00465126, 0);
		jump_patch(orders_place_mine_jump_wrapper_2, 0x00465145, 0);
		nops(0x00465088, 8); // remove tech requirement for any mines */
	}

	const u32 func_techTargetSpellCheck = 0x00491E80;
	const u32 jmp_004920E3 = 0x004920E3;
	void __declspec(naked) get_order_target_error_string_jump_wrapper()
	{
		static Unit* caster;
		static u8 player;
		static u32 techId;
		static Unit* target;
		static u32 func_result;
		static u32 extended_result;
		__asm {
			MOV caster, EAX
			MOV BL, [EAX + 0x4c]
			MOV EAX, EDI
			MOV target, EAX
			MOV player, BL
			CALL func_techTargetSpellCheck
			MOV func_result, EAX
			PUSHAD
		}
		extended_result = hooks::extend_tech_target_check(func_result, caster, target, techId);
		__asm {
			POPAD
			MOV EAX, extended_result
			JMP jmp_004920E3
		}
	}

	const u32 jmp_004922E1 = 0x004922E1;
	void __declspec(naked) should_stop_ordered_spell_jump_wrapper()
	{
		static Unit* caster;
		static u8 player;
		static u32 techId;
		static Unit* target;
		static u32 func_result;
		static u32 extended_result;
		__asm {
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV techId, EAX
			POP EAX
			MOV player, BL
			MOV target, EAX
			MOV caster, EDI
			CALL func_techTargetSpellCheck
			MOV func_result, EAX
			PUSHAD
		}
		extended_result = hooks::extend_tech_target_check(func_result, caster, target, techId);
		__asm {
			POPAD
			MOV EAX, extended_result
			JMP jmp_004922E1
		}
	}

	void lobotomy_mine_targeting_patches()
	{
		jump_patch(get_order_target_error_string_jump_wrapper,	0x004920D9);
		jump_patch(should_stop_ordered_spell_jump_wrapper,		0x004922DC);
	}

	void trap_jump_wrappers()
	{
		memory_patch(0x00401438, (u8)0xEB); // change to jump
		u8 patchb030[] = {0x33, 0xc0, 0x90, 0x90};
		memory_patch(0x0047B030, patchb030); // xor eax, eax; nop; nop
		memory_patch(0x0047B070, patchb030); // xor eax, eax; nop; nop
		
		memory_patch(0x004EBE99, (u8)0xEB);
		memory_patch(0x004A4B4D, (u8)0xEB);
		memory_patch(0x004A491B, (u8)0xEB);
		memory_patch(0x004A4829, (u8)0xEB);
		memory_patch(0x004A46CA, (u8)0xEB);
		memory_patch(0x004A0307, (u8)0xEB);
		memory_patch(0x004A3E04, (u8)0xEB);
		
		u8 jmpp[] = {0xEB, 0x3d};
		memory_patch(0x0049FAEF, jmpp);

//		memoryPatch(0x0049FAEF, (u8)0x75);//JNZ
		
		memory_patch(0x0047C05D + 2, (u8)0xe4); // floor gun trap
		memory_patch(0x0047C069 + 2, (u8)0xe4); // floor missile trap


		u8 jmpp2[] = {0xeb,0x2c};
		memory_patch(0x0047BEE7, jmpp2); // JNZ

		memory_patch(0x0047B516, patchb030);
		u8 jmpToEnd[] = {0xC3,0x90,0x90,0x90}; // RETN, NOP (3)
		memory_patch(0x0047B556, jmpToEnd);
		
		memory_patch(0x0047B7B1 + 2, (u8)0xE4);
		nops(0x0047B7B7, 6);
		
		// 0047BDCB
		u8 jmpp3[] = {0xeb,0x25};
		memory_patch(0x0047BDCB, jmpp3); // JNZ
	}

	const u32 progressOrder_jmp = 0x004EC6F1;
	void __declspec(naked) phase_link_order_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		hooks::order_phase_link(unit);
		__asm {
			POPAD
			JMP progressOrder_jmp
		}
	}

	void phase_link_patches()
	{
		jump_patch(phase_link_order_wrapper, 0x004EC629, 2);
	}

	void __declspec(naked) can_attack_target_tail_wrapper()
	{
		static u32 target_flags;
		static Unit* source;
		static u32 result;
		__asm {
			PUSH EAX
			MOV EAX, [EBP - 0x4]
			MOV target_flags, EAX
			POP EAX
			MOV source, EDI
			PUSHAD
		}
		result = hooks::can_attack_target_end_chunk(source, target_flags);
		__asm {
			POPAD
			MOV EAX, result
			POP EDI
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) get_ground_weapon_hook_wrapper()
	{
		static Unit* unit;
		static u32 result;
		__asm {
			MOV unit, ECX
			PUSHAD
		}
		result = unit->get_ground_weapon();
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void clarion_wrappers()
	{
		// can_attack_target - end
		// jump_patch(can_attack_target_tail_wrapper, 0x0047682C, 1);
		// tail wrapper disabled, without air weapon it don't have sense anywhere, will be reworked w/ air target functionality
		jump_patch(get_ground_weapon_hook_wrapper, 0x00475AD0);
	}

	const u32 jmp_skip = 0x00404541;
	const u32 jmp_continue = 0x00404539;
	void __declspec(naked) create_building_ai_workaround_wrapper()
	{
		static BuildingAi* bAi;
		__asm {
			MOV EDX, [ECX + 0xABE0]
			MOV bAi, EDX
			PUSHAD
		}
		if (reinterpret_cast<u32>(bAi) < 0x300000)
		{
			__asm {
				POPAD
				JMP jmp_skip
			}
		}
		else
		{
			__asm {
				POPAD
				JMP jmp_continue
			}
		}
	}

	void create_building_ai_workaround()
	{
		jump_patch(create_building_ai_workaround_wrapper, 0x00404533, 1);
	}

	static bool warningOnce00 = false;
	const u32 Hook_DropDebugging00 = 0x0047CDDB;
	const u32 Hook_DropDebugging00Back = 0x0047CDE1;
	void __declspec(naked) drop_debugging_00()
	{
		static char buffer[256];
		static u8 hashStructIndex;
		__asm {
			JNB short _Jmp
			JMP Hook_DropDebugging00Back
			_Jmp :
			PUSHAD
				MOV EBP, ESP
				MOV hashStructIndex, AL
		}
		if (!warningOnce00)
		{
			sprintf_s(buffer, "Invalid hashStructIndex!!\nhashStructIndex: %u", hashStructIndex);
			scbw::print(buffer, GameTextColor::Red);
			warningOnce00 = true;
		}
		__asm {
			POPAD
			XOR EAX, EAX
			POP EBX
			RETN
		}
	}

	static bool warningOnce01 = false;
	const u32 Hook_DropDebugging01 = 0x0047CCF8;
	void __declspec(naked) drop_debugging_01()
	{
		static char buffer[256];
		static u32 activeNationID;

		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV activeNationID, EBX
		}

		if (!warningOnce01)
		{
			sprintf_s(buffer, "Invalid PlayerFlag!!\nactiveNationID: %u", activeNationID);
			scbw::print(buffer, GameTextColor::Red);
			warningOnce01 = true;
		}

		__asm {
			POPAD
			POP EDI
			XOR EAX, EAX
			POP EBX
			POP EBP
			RETN 4
		}
	}

	static bool warningOnce02 = false;
	const u32 Hook_DropDebugging02 = 0x0047CD63;
	void __declspec(naked) drop_debugging_02()
	{
//		static char buffer[256];
		static bool isSucceeded;
		static u8 visibilityHash;
		static u8 nationHash;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV isSucceeded, AL
			MOV visibilityHash, CL
			MOV nationHash, DL
		}
		if (!isSucceeded && !warningOnce02)
		{
//			sprintf_s(buffer, "Invalid SpriteVisibility!!\nactiveNationID: %u", *ACTIVE_NATION_ID);
//			scbw::print(buffer, GameTextColor::Red);
			scbw::printf("Invalid SpriteVisibility!!\nactiveNationID: % u", *ACTIVE_NATION_ID);
			scbw::printf("SpriteVisHash (from VisionSync) %d =/= visionHash (from CMD) %d", nationHash, visibilityHash);
			warningOnce02 = true;
		}
		__asm {
			POPAD
//			MOV AL, 1 // disable desync status
			POP EBX
			POP EBP
			RETN 4
		}
	}

	struct hashStruct
	{
		u16 hash;
		u8 hashType;
		u8 byte[0x109];
	};

	static_assert(sizeof(hashStruct) == 0x10C, "Invalid hashStruct Size");
	hashStruct* hashList = (hashStruct*)0x0065EB30;
	
	static bool warningOnce03_00 = false;
	static bool warningOnce03_01 = false;
	const u32 Hook_DropDebugging03 = 0x0047CDF1;
	void __declspec(naked) drop_debugging_03()
	{
		static char buffer[256];
		static u8* data;
		static u32 hashStructIndex;
		static u8 hashType;
		static bool isSucceeded;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV data, EDI
			MOV hashStructIndex, ESI
			MOV hashType, BL
		}
		isSucceeded = true;
		if (hashList[hashStructIndex].hash != *(u16*)(data + 2) && !warningOnce03_00)
		{
			isSucceeded = false;
			sprintf_s(buffer, "Invalid Hash!!\nhashStructIndex: %u\nHashType1 = %u\nHash1 = 0x%04x\nHashType2 = %u\nHash2 = 0x%04x",
				hashStructIndex,
				hashList[hashStructIndex].hashType,
				hashList[hashStructIndex].hash,
				hashType & 0x0F,
				*(u16*)(data + 2));
			scbw::print(buffer, GameTextColor::Red);
			warningOnce03_00 = true;
		}
		if (hashList[hashStructIndex].hashType != (hashType & 0x0F) && warningOnce03_01)
		{
			isSucceeded = false;
			sprintf_s(buffer, "Invalid HashType!!\nhashStructIndex: %u\nHashType = %u", hashStructIndex, hashList[hashStructIndex].hashType);
			scbw::print(buffer, GameTextColor::Red);
			warningOnce03_01 = true;
		}
		if (isSucceeded)
		{
			__asm {
				POPAD
				POP ESI
				MOV EAX, 1
				POP EBX
				RETN
			}
		}
		else
		{
			__asm {
				POPAD
				POP ESI
				XOR EAX, EAX
				POP EBX
				RETN
			}
		}
	}

	void desync_debug_messages()
	{
		// code from BG
		jump_patch(drop_debugging_00, Hook_DropDebugging00);
		jump_patch(drop_debugging_01, Hook_DropDebugging01);
		jump_patch(drop_debugging_02, Hook_DropDebugging02);
		jump_patch(drop_debugging_03, Hook_DropDebugging03);
	}

	void __declspec(naked) createDamageOverlay_Wrapper() // 004993C0
	{
		static Sprite* sprite;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV sprite, EAX
			POP EAX
			PUSHAD
		}
		hooks::create_damage_overlay(sprite);
		__asm
		{
			POPAD
//			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
	const u32 DmgOverlay_JmpBack = 0x004993E3;
	const u32 DmgOverlay_JmpEnd = 0x0049956E;
	void __declspec(naked) newDamageOverlay_JmpWrapper(){//
		static Sprite* sprite;
		static u32 cmp1, cmp2;
		__asm {
			MOV sprite,EDX
			MOV cmp1,ESI
			MOV cmp2,EDI
			PUSHAD
		}
		if (debug_damage_overlay(sprite) && cmp1!=cmp2) {
			__asm {
				POPAD
				JMP DmgOverlay_JmpBack
			}
		}
		else {
			__asm {
				POPAD
				JMP DmgOverlay_JmpEnd
			}
		}	
	}

	void damage_overlay_debug()
	{
//		jump_patch(createDamageOverlay_Wrapper, 0x004993C0);
		jump_patch(newDamageOverlay_JmpWrapper, 0x004993DB, 3);
	}

	void __declspec(naked) game_loop_wrapper()
	{
		static s32 result;
		__asm
		{
			PUSHAD
		}
		result = hooks::game_loop();
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) create_unit_wrapper() // 004A09D0
	{
		static Unit* result;
		static u16 unit_id;
		static s32 x;
		static s32 y;
		static s32 player;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV y, EAX
			MOV EAX, [EBP + 0xC]
			MOV player, EAX
			POP EAX
			MOV x, EAX
			MOV unit_id, CX
			PUSHAD
		}
		result = hooks::create_unit(unit_id, x, y, player);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) transform_unit_main_wrapper() { // 0049F170
		static Unit* unit;
		static u8 unit_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit_id, AL
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		hooks::transform_unit_main(unit, unit_id);
		__asm {
			POPAD
			MOV EAX, 0
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) ignore_iscript_and_go_to_idle_wrapper() // 00476550
	{
		static Unit* unit;
		static char result;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::ignore_iscript_and_go_to_idle(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) init_damage_overlay_counts_wrapper() { // 004D5C20
		__asm {
			PUSHAD
		}
		hooks::init_damage_overlay_counts();
		__asm {
			POPAD
			RETN
		}
	}

	void inject_cosmonarchy_specific()
	{
		flingy_speed_patches();
		desync_debug_messages();

		// start of spell order code
		// default case of switch 004EC4FF
		unused_order_patches();
		phase_link_patches(); // replaces CTFCop2 order
		lobotomy_mine_patches();
		trap_jump_wrappers();
		clarion_wrappers();
		create_building_ai_workaround();
		damage_overlay_debug();
//		lobotomy_mine_targeting_patches();

		// add later: disable WarpIn functionality, make it useless
		archon_hardcode_patches();

		// Hook patches!
	//	jump_patch(game_loop_wrapper,						0x004D94B0, 3);
		jump_patch(create_unit_wrapper,						0x004A09D0, 1);
		jump_patch(transform_unit_main_wrapper,				0x0049F170, 1);
		jump_patch(ignore_iscript_and_go_to_idle_wrapper,	0x00476550, 1);
	//	jump_patch(init_damage_overlay_counts_wrapper,		0x004D5C20, 3);
	}
}