#include "target_related.h"
#include "hook_tools.h"
#include "BW/structures.h"

namespace {
    void __declspec(naked) is_unit_target_out_of_max_range_wrapper() { // 00476430
        static Unit* unit;
        static Unit* target;
        static bool result;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV target, EAX
            POP EAX
            MOV unit, EAX
            PUSHAD
        }
        result = hooks::is_unit_target_out_of_max_range(unit, target);
        __asm {
            POPAD
            MOVZX EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 0x04
        }
    }
    void __declspec(naked) is_unit_in_weapon_range_wrapper() { // 00476870
        static Unit* target;
        static Unit* attacker;
        static bool result;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV attacker, EAX
            POP EAX
            MOV target, EAX
            PUSHAD
        }
        result = hooks::is_unit_in_weapon_range(target, attacker);
        __asm {
            POPAD
            MOVZX EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 0x04
        }
    }

    void __declspec(naked) cmdrecv_hold_position_wrapper() { // 004C20C0
        static Unit* result;
        static s8* buffer;
        __asm {
            MOV buffer, EDI
            PUSHAD
        }
        result = hooks::cmdrecv_hold_position(buffer);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) unit_can_see_cloaked_target_wrapper() { // 00401D60
        static Unit* target;
        static Unit* unit;
        static u32 result;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV unit, EAX
            POP EAX
            MOV target, EAX
            PUSHAD
        }
        result = hooks::unit_can_see_cloaked_target(target, unit);
        __asm {
            POPAD
            MOV EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 4
        }
    }

    void __declspec(naked) is_unit_visible_wrapper() { // 004E5DB0
        static u32 result;
        static Unit* unit;
        __asm {
            MOV unit, ESI
            PUSHAD
        }
        result = hooks::is_unit_visible(unit);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) check_unit_visibility_wrapper() { // 00443390
        static Unit* unit;
        __asm {
            MOV unit, EAX
            PUSHAD
        }
        hooks::check_unit_visibility(unit);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) is_unit_cloaking_wrapper() { // 004916E0
        static s32 result;
        static Unit* unit;
        __asm {
            MOV unit, ECX
            PUSHAD
        }
        result = hooks::is_unit_cloaking(unit);
        __asm
        {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) unit_can_attack_target_wrapper() { // 00476730
        static u32 result;
        static Unit* target;
        static Unit* attacker;
        static u32 check_detection;
        __asm {
            PUSH EBP
            PUSH EAX
            MOV EBP, ESP
            MOV EAX, [EBP + 0x08]
            MOV check_detection, EAX
            POP EAX
            MOV target, EBX
            MOV attacker, ESI
            POP EBP
            PUSHAD
        }
        result = hooks::unit_can_attack_target(target, attacker, check_detection);
        __asm {
            POPAD
            MOV EAX, result
            RETN 0x04
        }
    }

    void __declspec(naked) ai_attack_unit_wrapper() { // 0043FFD0
        static Unit* unit;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV unit, EAX
            POP EAX
            PUSHAD
        }
        hooks::ai_attack_unit(unit);
        __asm {
            POPAD
            MOV ESP, EBP
            POP EBP
            RETN 0x04
        }
    }

    void __declspec(naked) return_interceptor_if_cannot_attack_wrapper() { // 00465810
        static Unit* hangar_unit;
        static s32 result;
        __asm {
            MOV hangar_unit, EAX
            PUSHAD
        }
        result = hooks::return_interceptor_if_cannot_attack(hangar_unit);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) find_fow_target_wrapper() { // 00487A10
        static Thingy* result;
        static u16 x_pos;
        static u16 y_pos;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV y_pos, AX
            POP EAX
            MOV x_pos, BX
            PUSHAD
        }
        result = hooks::find_fow_target(x_pos, y_pos);
        __asm {
            POPAD
            MOV EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 0x04
        }
    }

    void __declspec(naked) is_better_target_wrapper() { // 004769D0
        static bool result;
        static Unit* self;
        static Unit* auto_target;
        static Unit* compare_unit;
        __asm {
            MOV compare_unit, EDI
            MOV auto_target, EAX
            MOV self, ECX
            PUSHAD
        }
        result = hooks::is_better_target(self, auto_target, compare_unit);
        __asm {
            POPAD
            MOVZX AL, result
            RETN
        }
    }

    void __declspec(naked) do_targeted_order_wrapper() { // 0046F5B0
        static void* unused;
        static u32 sound_offset;
        static u32 pos_x;
        static u32 pos_y;
        static Unit* target;
        static s32 fow_unit_id;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV pos_x, EAX
            MOV EAX, [EBP + 0x0C]
            MOV pos_y, EAX
            MOV EAX, [EBP + 0x10]
            MOV target, EAX
            MOV EAX, [EBP + 0x14]
            MOV fow_unit_id, EAX
            POP EAX
            MOV sound_offset, ESI
            MOV unused, ECX
            PUSHAD
        }
        hooks::do_targeted_order(unused, sound_offset, pos_x, pos_y, target, fow_unit_id);
        __asm {
            POPAD
            MOV ESP, EBP
            POP EBP
            RETN 0x10
        }
    }

    void __declspec(naked) fow_refinery_check_wrapper() { // 004769D0
        static b32 result;
        static Unit* self;
        static u16 fow_id;
        __asm {
            MOV fow_id, CX
            MOV self, EAX
            PUSHAD
        }
        result = hooks::fow_refinery_check(self, fow_id);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) pick_new_target_if_needed_wrapper() { // 00477510
        static Unit* unit;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV unit, EAX
            POP EAX
            PUSHAD
        }
        hooks::pick_new_target_if_needed(unit);
        __asm {
            POPAD
            MOV ESP, EBP
            POP EBP
            RETN 0x04
        }
    }

    void __declspec(naked) change_direction_to_move_waypoint_wrapper() { // 00495840
        static Unit* unit;
        __asm {
            MOV unit, ESI
            PUSHAD
        }
        hooks::change_direction_to_move_waypoint(unit);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) orders_turret_guard_wrapper() { // 004777F0
        static Unit* unit;
        __asm {
            MOV unit, EAX
            PUSHAD
        }
        hooks::orders_turret_guard(unit);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) get_closest_point_of_target_wrapper() { // 004762C0
        static Unit* unit;
        static s32* out_x;
        static s32* out_y;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV unit, EAX
            MOV EAX, [EBP + 0x0C]
            MOV out_x, EAX
            MOV EAX, [EBP + 0x10]
            MOV out_y, EAX
            POP EAX
            PUSHAD
        }
        hooks::get_closest_point_of_target(unit, out_x, out_y);
        __asm {
            POPAD
            MOV ESP, EBP
            POP EBP
            RETN 0x0C
        }
    }

    void __declspec(naked) unit_order_move_to_target_reset_order_state_wrapper() { // 00479FA0
        static Unit* result;
        static Unit* unit;
        static u8 order_id;
        static Unit* target;
        __asm {
            MOV unit, EAX
            MOV order_id, DL
            MOV target, ECX
            PUSHAD
        }
        result = hooks::unit_order_move_to_target_reset_order_state(unit, order_id, target);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) is_target_within_min_range_wrapper() { // 00430F10
        static b32 result;
        static Unit* unit;
        static u32 min_range;
        static Unit* target;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV min_range, EAX
            MOV EAX, [EBP + 0x0C]
            MOV target, EAX
            POP EAX
            MOV unit, ECX
            PUSHAD
        }
        result = hooks::is_target_within_min_range(unit, min_range, target);
        __asm {
            POPAD
            MOV EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 0x08
        }
    }

    void __declspec(naked) is_target_visible_wrapper() { // 004E5E30
        static u32 result;
        static Unit* target;
        static Unit* unit;
        __asm {
            MOV target, EAX
            MOV unit, ESI
            PUSHAD
        }
        result = hooks::is_target_visible(target, unit);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) is_unit_enemy_wrapper() { // 0047B910
        static b32 result;
        static Unit* self;
        static Unit* target;
        __asm {
            MOV target, ECX
            MOV self, EAX
            PUSHAD
        }
        result = hooks::is_unit_enemy(self, target);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) is_ready_to_attack_wrapper() { // 00476640
        static b32 result;
        static Unit* unit;
        static WeaponId::Enum weapon_id;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV weapon_id, AX
            POP EAX
            MOV unit, EAX
            PUSHAD
        }
        result = hooks::is_ready_to_attack(unit, weapon_id);
        __asm {
            POPAD
            MOV EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 0x04
        }
    }

    void __declspec(naked) ai_choose_target_wrapper() { // 00431990
        static Unit* result;
        static Unit** target_array;
        static u32 player;
        static Unit* unit;
        static u32 region_id;
        static b32 is_ground;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV player, EAX
            MOV EAX, [EBP + 0x0C]
            MOV unit, EAX
            MOV EAX, [EBP + 0x10]
            MOV region_id, EAX
            MOV EAX, [EBP + 0x14]
            MOV is_ground, EAX
            POP EAX
            MOV target_array, EAX
            PUSHAD
        }
        result = hooks::ai_choose_target(target_array, player, unit, region_id, is_ground);
        __asm {
            POPAD
            MOV EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 0x10
        }
    }

    void __declspec(naked) get_air_weapon_wrapper() { // 00475840
        static WeaponId::Enum result;
        static Unit* unit;
        __asm {
            MOV unit, ECX
            PUSHAD
        }
        result = hooks::get_air_weapon(unit);
        __asm {
            POPAD
            MOVZX EAX, result
            RETN
        }
    }

    void __declspec(naked) orders_watch_target_wrapper() { // 0047BAB0
        static Unit* unit;
        __asm {
            MOV unit, EAX
            PUSHAD
        }
        hooks::orders_watch_target(unit);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) target_is_inside_minimum_range_wrapper() { // 004764D0
        static Unit* unit;
        static Unit* target;
        static bool result;
        __asm {
            MOV unit, EAX
            MOV target, ESI
            PUSHAD
        }
        result = hooks::target_is_inside_minimum_range(unit, target);
        __asm {
            POPAD
            MOVZX EAX, result
            RETN
        }
    }

    void __declspec(naked) get_ground_weapon_wrapper() { // 00475A90
        static Unit* unit;
        static WeaponId::Enum result;
        __asm {
            MOV unit, ECX
            PUSHAD
        }
        result = hooks::get_ground_weapon(unit);
        __asm {
            POPAD
            MOV AX, result
            RETN
        }
    }

    void __declspec(naked) minimap_game_target_order_wrapper() { // 004A55F0
        static DialogEvent* event;
        __asm {
            MOV event, EAX
            PUSHAD
        }
        hooks::minimap_game_target_order(event);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) can_weapon_target_unit_wrapper() { // 00475CE0
        static WeaponId::Enum weapon_id;
        static Unit* target;
        static Unit* attacker;
        static b32 result;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV attacker, EAX
            POP EAX
            MOV target, EDX
            MOV weapon_id, AX
            PUSHAD
        }
        result = hooks::can_weapon_target_unit(weapon_id, target, attacker);
        __asm {
            POPAD
            MOV EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 0x04
        }
    }

    void __declspec(naked) get_targeting_error_string_wrapper() { // 0046EFE0
        static WeaponId::Enum weapon_id;
        static u32* out_error;
        static Unit* target;
        __asm {
            MOV weapon_id, AX
            MOV out_error, EDX
            MOV target, ECX
            PUSHAD
        }
        hooks::get_targeting_error_string(weapon_id, out_error, target);
        __asm {
            POPAD
            MOV EAX, 0
            RETN
        }
    }

    void __declspec(naked) is_threat_wrapper() { // 00476930
        static Unit* candidate;
        static Unit* self;
        static b32 result;
        __asm {
            MOV candidate, EAX
            MOV self, ECX
            PUSHAD
        }
        result = hooks::is_threat(candidate, self);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) is_unit_in_attack_angle_wrapper() { // 00475BE0
        static bool result;
        static s32 pos_x;
        static s32 pos_y;
        static Unit* unit;
        static WeaponId::Enum weapon_id;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV weapon_id, AX
            POP EAX
            MOV pos_x, ECX
            MOV pos_y, EAX
            MOV unit, ESI
            PUSHAD
        }
        result = hooks::is_unit_in_attack_angle(pos_x, pos_y, unit, weapon_id);
        __asm {
            POPAD
            MOVZX EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 0x04
        }
    }
    
    // Patches \\

    const u32 GetClosestReachable_2F155 = 0x0042F155;
    const u32 GetClosestReachable_2F281 = 0x0042F281;
    void __declspec(naked) get_closest_reachable_patch_2F147() {
        __asm {
            MOV		EDI, [ESI + 0xC]
            TEST	EDI, EDI	// attacker_unit != NULL
            JE		offset loc_2F281
            MOV		EAX, [EDI + 0xC]
            TEST	EAX, EAX	// attacker_unit->sprite != NULL
            JE		offset loc_2F281
            JMP		GetClosestReachable_2F155

            loc_2F281 :
            JMP		GetClosestReachable_2F281
        }
    }

    const u32 MFixCollision_6C002 = 0x0046C002;
    const u32 MFixCollision_6C01D = 0x0046C01D;
    void __declspec(naked) fix_collision_patch_6BFFD() {
        __asm {
            MOV		EAX, [EDI + 0x14]
            TEST	EAX, EAX	// unit->move_target.unit != NULL
            JE		offset loc_6C01D
            MOV		ECX, [EAX + 0xC]
            TEST	ECX, ECX	// unit->move_target.unit->sprite != NULL
            JE		offset loc_6C01D
            JMP		MFixCollision_6C002

            loc_6C01D :
            JMP		MFixCollision_6C01D
        }
    }

    b32 __fastcall prep_suicide_get_air_weapon(u32 unit_id) {
        WeaponId::Enum weapon_id = units_dat::air_weapon[unit_id];
        return weapon_id != WeaponId::None;
    }

    b32 __fastcall prep_suicide_get_gnd_weapon(Unit* unit) {
        return unit->get_ground_weapon() != WeaponId::None;
    }

    void __fastcall CHK_UNI_get_weapon_id(int, u32 unit_id, u16* staredit_dmg_amount, u16* staredit_dmg_bonus) {
        WeaponId::Enum weapon_id = units_dat::ground_weapon[unit_id];
        if (weapon_id != WeaponId::None) {
            weapons_dat::damage_amount[weapon_id] = staredit_dmg_amount[weapon_id];
            weapons_dat::armor_pen[weapon_id] = staredit_dmg_bonus[weapon_id];
        }
        weapon_id = units_dat::air_weapon[unit_id];
        if (weapon_id != WeaponId::None) {
            weapons_dat::damage_amount[weapon_id] = staredit_dmg_amount[weapon_id];
            weapons_dat::armor_pen[weapon_id] = staredit_dmg_bonus[weapon_id];
        }
    }

    void __declspec(naked) CHK_UNIS_get_weapon_id_wrapper() {
        static const u32 _ret = 0x004CACA2;
        __asm {
            PUSH EAX
            PUSH ECX
            LEA EAX, [EBP - 0x190] // staredit_dmg_amount
            PUSH EAX
            LEA EAX, [EBP - 0xC8] // staredit_dmg_bonus
            PUSH EAX
            CALL CHK_UNI_get_weapon_id
            POP ECX
            POP EAX
            JMP _ret
        }
    }

    void __declspec(naked) CHK_UNIx_get_weapon_id_wrapper() {
        static const u32 _ret = 0x004CAE62;
        __asm {
            PUSH EAX
            PUSH ECX
            LEA EAX, [EBP - 0x208] // staredit_dmg_amount
            PUSH EAX
            LEA EAX, [EBP - 0x104] // staredit_dmg_bonus
            PUSH EAX
            CALL CHK_UNI_get_weapon_id
            POP ECX
            POP EAX
            JMP _ret
        }
    }

    u32 __stdcall bullet_damage_get_weapon_id(Bullet* bullet) {
        return get_bullet_ext(bullet)->weapon_id;
    }

    void __declspec(naked) bullet_damage_patch_8B777() {
        __asm {
            MOV EBX, [EBP + 0x8]
            PUSH EBX
            CALL bullet_damage_get_weapon_id
            MOV ECX, EAX
            RETN
        }
    }

    void __declspec(naked) bullet_damage_patch_8B8B9() {
        __asm {
            PUSH EBX
            CALL bullet_damage_get_weapon_id
            MOV EDX, [EBX + 0xC]
            MOV ECX, EAX
            RETN
        }
    }

    void __fastcall ai_choose_target_weaponext(int, u32 unit_id) {
        u16 weapon_id = units_dat::air_weapon[unit_id];
        __asm {
            MOVZX ECX, weapon_id
            CMP ECX, 130 // WeaponId::None
        }
    }
}

namespace hooks {
    void inject_target_related_hooks() {
        // Removes 'can not attack' status checks from some update functions; it was done by iquare, not exactly sure what was the intention.
        // Currently it doesn't seem to do anything. -Veeq7
    //	nops(0x004EB176, 8);
    //	nops(0x004EB316, 2);

        jump_patch(is_unit_target_out_of_max_range_wrapper,             0x00476430, 2);
        jump_patch(is_unit_in_weapon_range_wrapper,				0x00476870, 2);
        jump_patch(cmdrecv_hold_position_wrapper,					0x004C20C0, 0);
    //	jump_patch(unit_can_see_cloaked_target_wrapper,			        0x00401D60, 4);
    //	jump_patch(is_target_visible_wrapper,							0x004E5E30, 4);
    //	jump_patch(is_unit_visible_wrapper,						        0x004E5DB0, 1);
        jump_patch(check_unit_visibility_wrapper,					0x00443390, 3);
    //	jump_patch(is_unit_cloaking_wrapper,							0x004916E0, 0);
        jump_patch(unit_can_attack_target_wrapper,						0x00476730, 5);
        jump_patch(ai_attack_unit_wrapper,						0x0043FFD0, 1);
        jump_patch(return_interceptor_if_cannot_attack_wrapper,	0x00465810, 1);
        jump_patch(find_fow_target_wrapper,								0x00487A10, 5);
        jump_patch(is_better_target_wrapper,							0x004769D0, 0);
        jump_patch(do_targeted_order_wrapper,							0x0046F5B0, 1);
        jump_patch(fow_refinery_check_wrapper,					0x00474500, 5);
        jump_patch(pick_new_target_if_needed_wrapper,					0x00477510, 1);
        jump_patch(change_direction_to_move_waypoint_wrapper,		0x00495840, 3);
        jump_patch(orders_turret_guard_wrapper,					0x004777F0, 2);
        jump_patch(get_closest_point_of_target_wrapper,					0x004762C0, 1);
        jump_patch(unit_order_move_to_target_reset_order_state_wrapper,	0x00479FA0, 0);
        jump_patch(is_target_within_min_range_wrapper,					0x00430F10, 1);
        jump_patch(is_target_visible_wrapper,							0x004E5E30, 4);
        jump_patch(is_unit_enemy_wrapper,							0x0047B910, 1);
        jump_patch(is_ready_to_attack_wrapper,					0x00476640, 1);
    //	jump_patch(ai_choose_target_wrapper,							0x00431990, 1);
        jump_patch(get_air_weapon_wrapper,						0x00475840, 5);
        jump_patch(orders_watch_target_wrapper,							0x0047BAB0, 0);
        jump_patch(target_is_inside_minimum_range_wrapper,				0x004764D0, 0);
        jump_patch(get_ground_weapon_wrapper,					    0x00475A90, 3);
    //	jump_patch(minimap_game_target_order_wrapper,					0x004A55F0, 1); // untested
        jump_patch(can_weapon_target_unit_wrapper,						0x00475CE0, 0);
        jump_patch(get_targeting_error_string_wrapper,					0x0046EFE0, 0);
        jump_patch(is_threat_wrapper,								0x00476930, 1);
        jump_patch(is_unit_in_attack_angle_wrapper,				0x00475BE0, 1);

        // Patches \\

        // Fix dying Lakizilisks updating vision while their spines are still in play. -DarkenedFantasies
        jump_patch(get_closest_reachable_patch_2F147, 0x0042F147);
        jump_patch(fix_collision_patch_6BFFD, 0x0046BFFD);

        // Extend weapon ids from u8. -DarkenedFantasies
        call_patch(prep_suicide_get_air_weapon, 0x0043CFA2, 9);
        call_patch(prep_suicide_get_gnd_weapon, 0x0043CFB2, 9);
        jump_patch(CHK_UNIS_get_weapon_id_wrapper, 0x004CAC42, 1);
        jump_patch(CHK_UNIx_get_weapon_id_wrapper, 0x004CAE02, 1);
        call_patch(bullet_damage_patch_8B777, 0x0048B777, 2);
        call_patch(bullet_damage_patch_8B8B9, 0x0048B8B9);
        nop_patch(0x0048B8C9, 3);
        memory_patch(0x0048B8E4, (u8)0x83); // Change CL to ECX
        call_patch(ai_choose_target_weaponext, 0x00431B0B, 6);
    }
}