#include "BWhelper.h"
#include "api.h"
#include "globals.h"
#include <cassert>

namespace helper {
	// sign flag
	s8 SETS(s32 x) {
		if (sizeof(s32) == 1)
			return s8(x) < 0;
		if (sizeof(s32) == 2)
			return s16(x) < 0;
		if (sizeof(s32) == 4)
			return s32(x) < 0;
		return s64(x) < 0;
	}

	s8 OFSUB(s32 x, s32 y) {
		s32 y2 = y;
		s8 sx = SETS(x);
		return (sx ^ SETS(y2)) & (sx ^ SETS(x - y2));
	}

	bool has_money_can_make(Unit* builder, u32 unit_to_build) {
		const u32 func_address = 0x00467250;
		static b32 result;
		__asm {
			PUSHAD
			PUSH unit_to_build
			MOV EDI, builder
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	// For loadRaceUI

	void get_race_sound_list(s8 a1) {
		static s32 init_xd = a1;
		const u32 func_address = 0x0048FB40;
		__asm {
			PUSHAD
			PUSH init_xd
			CALL func_address
			POPAD
		}
	}

	s32 dialog_grp_construct(s32 a1, s32 a2, s32 a3, s32(__fastcall* a4)(s32, s32, s32, s32)) {
		const u32 func_address = 0x004197B0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			MOV a1, EAX
			MOV a2, ECX
			CALL func_address
			MOV EAX, result
			POPAD
		}
		return result;
	}

	void* load_graphic(const char* a1, u32 a2, u32 a3, const char* log_filename, u32 log_line) {
		static void* result;
		const u32 func_address = 0x004D2B30;
		__asm {
			PUSHAD
			PUSH log_line
			PUSH log_filename
			MOV ESI, a3
			MOV EDX, a2
			MOV ECX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 rand_func() {
		const u32 func_address = 0x0040C7BD;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_hp_below_33_percent(Unit* unit) {
		const u32 func_address = 0x004022C0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unit_hp_between_33_66_percent(Unit* unit) {
		const u32 func_address = 0x00402270;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, 0 // Unused arg
			MOV EDX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 get_max_hp(Unit* unit) {
		const u32 func_address = 0x00401400;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u16 get_last_queue_slot_id(Unit* unit) {
		const u32 func_address = 0x0047B270;
		static u32 result;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	void add_text_to_dialog(BinDlg* dialog, s16 index, const char* text) {
		const u32 func_address = 0x004258B0;
		__asm {
			PUSHAD
			PUSH text
			MOVZX ECX, index
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	void stats_panel_draw_energy(BinDlg* dialog) {
		const u32 func_address = 0x00425A30;
		__asm {
			PUSHAD
			MOV EBX, dialog
			CALL func_address
			POPAD
		}
	}

	void hide_optional_status_screen_controls(BinDlg* dialog) {
		const u32 func_address = 0x00457310;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	void show_control_group(BinDlg* dialog, u16 firstId, u16 lastId) {
		const u32 func_address = 0x00457250;
		u32 last_id_xd = lastId;
		__asm {
			PUSHAD
			PUSH last_id_xd
			MOVZX ECX, firstId
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	void hide_dialog(BinDlg* dialog) {
		const u32 func_address = 0x00418700;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL func_address
			POPAD
		}
	}

	// For draw_supply_unit_info
	s32 get_supplies_used_no_args() {
		const u32 func_address = 0x00424732;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_supplies_available(u8 a1, s32 a2) {
		const u32 func_address = 0x00488900;
		static u32 result;
		__asm {
			PUSHAD
			MOVZX AL, a1
			MOV ECX, a2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void unit_stat_act_standard(void* a1) {
		const u32 Func_unitStatAct_Standard = 0x00426F50;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitStatAct_Standard
			POPAD
		}
	}

	// For ai_target_enemy_proc

	bool ai_unit_can_attack(Unit* a1) {
		const u32 Func_AI_UnitCanAttack = 0x00476180;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_AI_UnitCanAttack
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	// For draw_building_info

	s32 ui_needs_redraw() {
		const u32 Func_needRedrawUI = 0x00424A10;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_needRedrawUI
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* generic_status_update_drawn_values() {
		const u32 Func_genericStatusUpdateDrawnValues = 0x004248F0;
		static Unit* result;
		__asm {
			PUSHAD
			CALL Func_genericStatusUpdateDrawnValues
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void draw_progress_bar(BinDlg* dialog) {
		const u32 Func_drawProgressBar = 0x004260C0;
		__asm {
			PUSHAD
			MOV EDI, dialog
			CALL Func_drawProgressBar
			POPAD
		}
	}

	void draw_nuke_silo_info(BinDlg* dialog) {
		const u32 Func_drawNukeSiloInfo = 0x00426FF0;
		__asm {
			PUSHAD
			PUSH dialog
			CALL Func_drawNukeSiloInfo
			POPAD
		}
	}

	// Equivalent to code unit_is_active_transport @ 0x004E6BA0
	// Modified to remove Ovileth checks as they are no longer tied to upgrades in HYDRA
	bool unit_is_active_transport(Unit* unit) {
		bool result = false;
		if (!(unit->status & UnitStatus::IsHallucination)) {
			result = units_dat::space_provided[unit->id] != 0;
		}
		return result;
	}

	bool is_queue_slot_active(Unit* unit, s32 slot) {
		const u32 Func_isQueueSlotActive = 0x00401E70;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EDX, slot
			CALL Func_isQueueSlotActive
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void draw_training_progress(BinDlg* dialog) {
		const u32 Func_drawTrainingProgress = 0x004268D0;
		__asm {
			PUSHAD
			PUSH dialog
			CALL Func_drawTrainingProgress
			POPAD
		}
	}

	bool is_constructing_addon(const Unit* a1) {
		const u32 Func_isConstructingAddon = 0x004E66B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isConstructingAddon
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool is_attempting_protoss_build(Unit* a1) {
		const u32 Func_isAttemptingProtossBuild = 0x004E4C40;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isAttemptingProtossBuild
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void draw_addon_progress(BinDlg* a1) {
		const u32 Func_drawAddonProgress = 0x00425F30;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawAddonProgress
			POPAD
		}
	}

	void draw_tech_progress(BinDlg* a1) {
		const u32 Func_drawTechProgress = 0x004266F0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawTechProgress
			POPAD
		}
	}

	void draw_upgrade_progress(BinDlg* a1) {
		const u32 Func_drawUpgradeProgress = 0x00426500;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawUpgradeProgress
			POPAD
		}
	}

	bool unit_is_geyser(Unit* a1) {
		const u32 Func_unitIsGeyser = 0x004688B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsGeyser
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void draw_gas_quantity(BinDlg* a1) {
		const u32 Func_drawGasQuantity = 0x00426190;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_drawGasQuantity
			POPAD
		}
	}

	s8 get_unit_ground_weapon(Unit* a1) {
		const u32 Func_unitGetGroundWeapon = 0x00475AD0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitGetGroundWeapon
			MOV result, EAX
			POPAD
		}
		return (s8)result;
	}

	void draw_display_buttons(BinDlg* a1) {
		const u32 Func_drawDisplayButtons = 0x00426C60;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawDisplayButtons
			POPAD
		}
	}

	BinDlg* get_control_from_index(BinDlg* a1, s16 a2) {
		const u32 Func_getControlFromIndex = 0x00418080;
		static BinDlg* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CX, a2
			CALL Func_getControlFromIndex
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void set_text_string(BinDlg* a1) {
		const u32 Func_setTextStr = 0x004263E0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setTextStr
			POPAD
		}
	}

	void set_unit_status_string_text(BinDlg* a1) {
		const u32 Func_setUnitStatusStrText = 0x00425B50;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setUnitStatusStrText
			POPAD
		}
	}

	s32 get_unit_name(Unit* a1) {
		const u32 Func_unitGetName = 0x0047B5A0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitGetName
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For get_unit_strength

	s32 get_current_hp_plus_shields(Unit* a1) {
		const u32 Func_getCurrentHitPointsPlusShields = 0x004026D0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_getCurrentHitPointsPlusShields
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_base_strength(s32 a1, Unit* a2) {
		const u32 Func_getBaseStrength = 0x004316D0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV EDI, a2
			CALL Func_getBaseStrength
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For AI_RestorationRequirementsProc

	s32 can_weapon_target_unit(u8 a1, Unit* a2, Unit* a3) {
		const u32 Func_canWeaponTargetUnit = 0x00475CE0;
		static s32 result;
		__asm {
			PUSHAD
			MOVZX EAX, a1
			MOV EDX, a2
			PUSH a3
			CALL Func_canWeaponTargetUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For AI_EMPShieldRequirementsProc

	s32 ai_target_unit_is_worth_hitting(Unit* a1, Unit* a2) {
		const u32 Func_AI_TargetUnitIsWorthHitting = 0x00440E30;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_AI_TargetUnitIsWorthHitting
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 create_splash_weapon_proc(u16 range, u16 pos_x, u16 pos_y, s32(__fastcall* callback)(s32, s32), Unit* caster) {
		const u32 func_address = 0x004E82E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH caster
			PUSH callback
			MOVZX ECX, pos_y
			MOVZX EDX, pos_x
			MOVZX EAX, range
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 unit_shields_clump_in_area_accumulator_proc(Unit* a1, Unit* a2) {
		const u32 func_address = 0x00440A60;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For hallucinationHit
	void unit_was_hit(Unit* a1, Unit* a2, s32 a3) {
		const u32 func_address = 0x004795D0;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EBX, a1
			CALL func_address
			POPAD
		}
	}

	void unit_attack_notify(Unit* unit) {
		const u32 func_address = 0x0048F230;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	Image* create_shield_overlay(s32 a1, Unit* a2) {
		const u32 func_address = 0x004E6140;
		static Image* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For draw_image_hp_bar

	s32 unk_billions(s32 a1, s32 a2, s32 a3, s32 a4) {
		const u32 func_address = 0x0047A750;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV EBX, a3
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 unk_billions2(s32 a1, s32 a2, s32 a3, s32 a4) {
		const u32 func_address = 0x0047A6E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV EDI, a3
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 hpBarSurfaceBuffer(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5) {
		const u32 func_address = 0x0047A350;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			PUSH a3
			PUSH a2
			PUSH a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 hpBarSurfaceBuffer2(s32 a1, s32 a2, u16 a3, s32 a4) {
		const u32 func_address = 0x0047A120;
		static s32 result;
		u32 a3_xd = a3;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3_xd
			PUSH a2
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 renderHPBar(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5) {
		const u32 func_address = 0x0047A4E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			PUSH a3
			PUSH a2
			PUSH a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_unit_max_energy(Unit* a1) {
		const u32 func_address = 0x00491870;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 getHpBarDrawSizes(s32 a1, s32 a2, s32 a3, s32 a4) {
		const u32 func_address = 0x0047A670;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV EDI, a3
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For can_cast_spell

	bool unitIsFrozen(Unit* a1) {
		const u32 func_address = 0x004020B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool isUnitCasterNotHallucination(Unit* a1) {
		const u32 func_address = 0x00401210;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	b32 get_active_tile_flag(s16 y, s16 x, s32 includeFlags, s32 excludeFlags) {
		const u32 func_address = 0x00472F80;
		static b32 result;
		static s32 x_xd = x;
		__asm {
			PUSHAD
			PUSH excludeFlags
			PUSH includeFlags
			PUSH x_xd
			MOVZX AX, y
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* get_target_unit_ability_error_stat_string(Unit* a1, char a2, s32 a3) {
		const u32 func_address = 0x00491E80;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX BL, a2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For modify_unit_shields

	s32 SErrSetLastError(char a1) {
		const u32 func_address = 0x00410172;
		static s32 result;
		static s32 a1_xd = a1;
		__asm {
			PUSHAD
			PUSH a1_xd
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void update_image_data_draw(Image* a1) {
		const u32 func_address = 0x004D57B0;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL func_address
			POPAD
		}
	}

	Image* create_healthbar(Sprite* sprite) {
		const u32 func_address = 0x004D6420;
		static Image* result;
		_asm {
			PUSHAD
			MOV ECX, 0
			MOV EDX, sprite
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void remove_hp_bar(Image* a1) {
		const u32 func_address = 0x004D5030;
		_asm {
			PUSHAD
			MOV ESI, a1
			CALL func_address
			POPAD
		}
	}

	void play_image_iscript(Image* img, u8 iscript_anim) {
		const u32 func_address = 0x004D8470;
		u32 iscript_anim_xd = iscript_anim;
		_asm {
			PUSHAD
			PUSH iscript_anim_xd
			MOV ECX, img
			CALL func_address
			POPAD
		}
	}

	bool can_enter_transport(Unit* a1, Unit* a2) {
		const u32 func_address = 0x004E6E00;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a2
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s16 get_region_id(s32 a1, s32 a2) {
		const u32 func_address = 0x0049C9F0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDI, a1
			MOV ECX, a2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (s16)result;
	}

	s32 get_unit_distance_to_halt(s32 a1, Unit* a2, s32 a3) {
		const u32 func_address = 0x00402140;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOV ECX, a1
			MOV EAX, a2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For map_data_transfer

	s32 dataTransfer00(s32 a1, char a2, s32 a3) {
		const u32 func_address = 0x00472A00;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CL, a2
			MOV ESI, a3
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer01(u32 a1, s32 a2, u32 a3) {
		const u32 func_address = 0x0045A600;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer02(s32 a1, s32 a2, char a3) {
		const u32 func_address = 0x0045A230;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			MOVZX BL, a3
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer03(s32 a1, char a2, s32 a3) {
		const u32 func_address = 0x0045A1F0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX BL, a2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer04(u32 a1, s32 a2, s32 a3) {
		const u32 func_address = 0x0045A390;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer05(s32 a1, s32 a2) {
		const u32 func_address = 0x0045A190;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV EDI, a2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	VectorHead* vectorTransfer(char a1, char a2) {
		const u32 func_address = 0x0045A010;
		static VectorHead* result;
		s32 a2_xd = a2;
		__asm {
			PUSHAD
			PUSH a2_xd
			MOVZX BL, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer06(s32 a1) {
		const u32 func_address = 0x00472900;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For get_random_yes_unit_response
	u32 get_random_sfx_between(u32 min, u32 max) {
		const u32 func_address = 0x0048E940;
		static s32 result;
		__asm {
			PUSHAD
			MOV ESI, min
			MOV EDI, max
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for ai_train_unit
	char ai_training_worker(Unit* result, Unit* parent) {
		const u32 func_address = 0x00435700;
		static char resultFunc;
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL func_address
			MOV resultFunc, AL
			POPAD
		}
		return resultFunc;
	}

	void ai_training_overlord(Unit* result, Unit* parent) {
		const u32 func_address = 0x00435770;
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL func_address
			POPAD
		}
	}

	void ai_training_normal(Unit* result, Unit* parent) {
		const u32 func_address = 0x00435DB0;
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL func_address
			POPAD
		}
	}

	void ai_order_to_destination(Unit* unit, s32 order_id, s32 x, s32 y) {
		const u32 func_address = 0x0043D5D0;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			PUSH order_id
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	void ai_assign_region_to_slowest_unit(AiRegion* region) {
		const u32 func_address = 0x00436F70;
		__asm {
			PUSHAD
			PUSH region
			CALL func_address
			POPAD
		}
	}

	// for ai_create_nuke
	Unit* create_unit(s16 unit_id, s32 x, s32 y, s32 player) {
		const u32 func_address = 0x004A09D0;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH player
			PUSH y
			MOV EAX, x
			MOVZX CX, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void update_unit_stats_and_finish_building(Unit* unit) {
		const u32 func_address = 0x004A01F0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	u32 update_unit_strength_and_apply_default_orders(Unit* unit) {
		const u32 Func_updateUnitStrengthAndApplyDefaultOrders = 0x0049FA40;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateUnitStrengthAndApplyDefaultOrders
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void hide_unit(Unit* unit) {
		const u32 Func_hideUnit = 0x004E6340;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_hideUnit
			POPAD
		}
	}

	// for AI_TrainingBroodling
	void ai_set_region_state(s32 region_type, AiRegion* region) {
		const u32 func_address = 0x004390A0;
		__asm {
			PUSHAD
			MOV EBX, region_type
			MOV ESI, region
			CALL func_address
			POPAD
		}
	}

	void add_military_ai(AiRegion* region, Unit* unit, s32 always_this_region) {
		const u32 func_address = 0x0043DA20;
		__asm {
			PUSHAD
			PUSH always_this_region
			MOV EAX, region
			MOV EBX, unit
			CALL func_address
			POPAD
		}
	}

	// for AI_AddUnitAI
	BuildingAi* create_building_ai(BuildingAiList* buildingAI) {
		const u32 func_address = 0x004044C0;
		static BuildingAi* result;
		__asm {
			PUSHAD
			MOV ESI, buildingAI
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void issue_order_targeting_nothing(Unit* unit, u8 order_id) {
		const u32 func_address = 0x00475310;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOVZX CL, order_id
			CALL func_address
			POPAD
		}
	}

	Unit* set_secondary_order(Unit* result, u8 order_id) {
		const u32 func_address = 0x004743D0;
		static Unit* result2;
		__asm {
			PUSHAD
			MOV EAX, result
			MOVZX CL, order_id
			CALL func_address
			MOV result2, EAX
			POPAD
		}
		return result2;
	}

	void ai_make_guard(s32 player, Unit* unit) {
		const u32 func_address = 0x0043A010;
		__asm {
			PUSHAD
			PUSH unit
			PUSH player
			CALL func_address
			POPAD
		}
	}

	bool unit_is_active_resource_depot(Unit* unit) {
		const u32 func_address = 0x00401CF0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	u32 get_default_refinery_type(u32 player) {
		const u32 func_address = 0x004320D0;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_unit_was_hit
	// Some of these should probably be methods in api/etc
	char cancel_unit(Unit* structure) {
		const u32 func_address = 0x00468280;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, structure
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	Unit* ai_get_actual_target(Unit* attacker, Unit* target) {
		const u32 func_address = 0x0043AD60;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EDI, attacker
			MOV ESI, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* check_arbiter_cloaking_range(Unit* unit, u16 id) {
		static u32 id_xd = id;
		const u32 func_address = 0x004410C0;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH id_xd
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_order_lurker_burrow(Unit* unit) {
		const u32 func_address = 0x0043C230;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	bool unit_is_in_attack_group(Unit* unit, b32 exclude_grouping_regions) {
		const u32 func_address = 0x00436E70;
		static u32 result;
		__asm {
			PUSHAD
			PUSH exclude_grouping_regions
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 is_unit_loaded(Unit* unit) {
		const u32 func_address = 0x004E7110;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_try_return_home(Unit* unit, s32 dontIssueOrder) {
		const u32 func_address = 0x00462EA0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH dontIssueOrder
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_ask_for_help(Unit* self, Unit* attacker, s32 onlyNearbyGuards) {
		const u32 func_address = 0x0043ADA0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH onlyNearbyGuards
			PUSH attacker
			PUSH self
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s16 get_region_id_from_unit(Unit* unit) {
		const u32 func_address = 0x0049CB40;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (s16)result;
	}

	s32 get_enemy_air_strength_in_region(u16 region_id, s32 player) {
		const u32 func_address = 0x00431D00;
		static s32 result;
		__asm {
			PUSHAD
			PUSH player
			MOVZX AX, region_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_enemy_strength_in_region(u16 region_id, s32 player, s32 includeAir) {
		const u32 func_address = 0x004318E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH includeAir
			PUSH player
			MOVZX AX, region_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_is_performing_unbreakable_code(Unit* unit) {
		const u32 func_address = 0x00402310;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool ai_cast_spell_behavior(Unit* unit, bool isAggro) {
		const u32 func_address = 0x004A13C0;
		static u32 result;
		b32 is_aggro_xd = isAggro;
		__asm {
			PUSHAD
			PUSH is_aggro_xd
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 ai_flee(Unit* unit, Unit* enemy) {
		const u32 func_address = 0x0043E400;
		static s32 result;
		__asm {
			PUSHAD
			PUSH enemy
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_has_ground_path(Unit* unit, s16 x, s16 y) {
		const u32 func_address = 0x0042FA00;
		static u32 result;
		u32 y_xd = y;
		__asm {
			PUSHAD
			PUSH y_xd
			MOVZX EDX, x
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool ai_has_no_defense_build(s32 player) {
		const u32 func_address = 0x00447370;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unit_can_see_cloaked_target(Unit* target, Unit* unit) {
		const u32 func_address = 0x00401D60;
		static u32 result;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void ai_respond_to_cloaked_threat(Unit* threat, Unit* unit) {
		const u32 func_address = 0x0043C580;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, threat
			CALL func_address
			POPAD
		}
	}

	s32 ai_update_attack_target(Unit* unit, s32 acceptIfSieged, s32 acceptCritters, s32 mustReach) {
		const u32 func_address = 0x00477160;
		static s32 result;
		__asm {
			PUSHAD
			PUSH mustReach
			PUSH acceptCritters
			PUSH acceptIfSieged
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void order_interrupt(Unit* unit, u8 order_id, Vector32* pos, Unit* target) {
		const u32 func_address = 0x00474C30;
		static u32 order_id_xd = order_id;
		__asm {
			PUSHAD
			PUSH target
			PUSH pos
			PUSH order_id_xd
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	// for iterate_building_ai
	u16 ai_remove_from_towns(Unit* worker) {
		const u32 func_address = 0x00432760;
		static u16 result;
		__asm {
			PUSHAD
			MOV ECX, worker
			CALL func_address
			MOV result, AX
			POPAD
		}
		return result;
	}

	s32 ai_get_queued_count(Unit* builder, u32 unit_id) {
		const u32 func_address = 0x00466B70;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, builder
			MOV EDI, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void prepare_guard_ai(u16 y, u32 player, u16 unit_id, u16 x) {
		const u32 func_address = 0x00462670;
		u32 x_xd = x;
		u32 unit_id_xd = unit_id;
		__asm {
			PUSHAD
			PUSH x_xd
			PUSH unit_id_xd
			MOV ESI, player
			MOVZX DI, y
			CALL func_address
			POPAD
		}
	}

	// AI attack wave extender

	// for AI_TrainAttackForce
	s32 ai_add_to_region_military(AiRegion* region, s32 unit_id, s32 priority) {
		const u32 func_address = 0x0043E2E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH priority
			PUSH unit_id
			MOV EAX, region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_add_cost_to_needs(u32 unit_id, u32 player, u32 type) {
		const u32 func_address = 0x00447DC0;
		static u32 result;
		u32 type_xd = type;
		__asm {
			PUSHAD
			PUSH type_xd
			MOV EBX, player
			MOV EAX, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_add_spending_request(u16 unit_id, u32 player, u8 type, char priority, void* value) {
		const u32 func_address = 0x00447980;
		static u32 result;
		u32 type_xd = type;
		u32 priority_xd = priority;
		__asm {
			PUSHAD
			PUSH value
			PUSH priority_xd
			MOV ECX, type_xd
			MOV EDX, player
			MOVZX EAX, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for AIRegion_GroupForAttack
	u32 ai_recalculate_region_strength(AiRegion* region) {
		const u32 func_address = 0x0043A390;
		static u32 result;
		__asm {
			PUSHAD
			MOV ESI, region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* ai_target_enemy_in_region(AiRegion* region) {
		const u32 func_address = 0x0043CC40;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EDI, region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_move_military_to_defend(AiRegion* region) {
		const u32 func_address = 0x0043DE40;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDI, region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_get_region_military_types(AiRegion* region, b32* hasGroundMilitary, b32* cantAttackGround, b32* cantAttackAir) {
		const u32 func_address = 0x00438FC0;
		__asm {
			PUSHAD
			MOV EAX, region
			MOV EBX, hasGroundMilitary
			MOV EDI, cantAttackGround
			MOV ESI, cantAttackAir
			CALL func_address
			POPAD
		}
	}

	s32 ai_can_travel_to_region(AiRegion* region1, AiRegion* region2) {
		const u32 func_address = 0x0043A790;
		static s32 result;
		__asm {
			PUSHAD
			PUSH region2
			PUSH region1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* ai_pick_attack_target_no_buildings(AiRegion* region) {
		const u32 func_address = 0x0043A510;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_move_military_to_other_region(AiRegion* target, AiRegion* source, u32 pullUnitsNearRegion, u32 sendPureSpellcasters, u32 dontSendFromBunkers) {
		const u32 func_address = 0x0043E580;
		static u32 result;
		__asm {
			PUSHAD
			PUSH dontSendFromBunkers
			PUSH sendPureSpellcasters
			PUSH pullUnitsNearRegion
			MOV ESI, source
			MOV EDI, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	AiTown* ai_iterate_town_something(AiRegion* region) {
		const u32 func_address = 0x0043DD20;
		static AiTown* result;
		__asm {
			PUSHAD
			MOV ESI, region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_attack_timer_decrement(u32 player) {
		const u32 func_address = 0x00447090;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_has_forces_moving_to_group(u32 player) {
		const u32 func_address = 0x0043A2E0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for ai_attack_manager
	u16 populate_regions_with_own(char* region_data, u32 player) {
		const u32 func_address = 0x00438B30;
		static u32 result;
		__asm {
			PUSHAD
			PUSH player
			MOV EBX, region_data
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	void populate_regions_with_enemy(u32 player, char* region_data) {
		const u32 func_address = 0x00439B50;
		__asm {
			PUSHAD
			PUSH region_data
			PUSH player
			CALL func_address
			POPAD
		}
	}

	s32 ai_get_neighboring_region_types(char* region_data) {
		const u32 func_address = 0x00436D80;
		static s32 result;
		__asm {
			PUSHAD
			MOV EBX, region_data
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_finish_region_accessibility(char* region_data) {
		const u32 func_address = 0x00436CF0;
		__asm {
			PUSHAD
			MOV EBX, region_data
			CALL func_address
			POPAD
		}
	}

	s32 ai_set_attack_grouping_region(u32 airAttack, u32 alsoVeryNear, u32 player, char* region_data, s32 x, s32 y, u32 alwaysOverwriteLastAttack) {
		const u32 func_address = 0x004399D0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH alwaysOverwriteLastAttack
			PUSH y
			PUSH x
			PUSH region_data
			PUSH player
			MOV EDX, alsoVeryNear
			MOV ECX, airAttack
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_cant_see_cloaked_target(Unit* unit, Unit* target) {
		const u32 Func_unitCantSeeCloakedTarget = 0x00401D60;
		static b32 result;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, target
			CALL Func_unitCantSeeCloakedTarget
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	bool unit_can_infest(Unit* unit) {
		const u32 Func_CanInfest = 0x00402750;
		static b32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_CanInfest
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	void ai_attack_unit(Unit* unit) {
		const u32 Func_AI_AttackUnit = 0x0043FFD0;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_AI_AttackUnit
			POPAD
		}
	}

	Unit* find_best_attack_target(Unit* unit) {
		const u32 Func_Sub443080 = 0x00443080;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub443080
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool ai_try_return_home(Unit* unit, u8 unknownByteValue) {
		const u32 func_address = 0x00462EA0;
		static b32 result;
		u32 unknown_xd = unknownByteValue;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unknown_xd
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	void set_interceptors_to_standby(Unit* unit) {
		const u32 func_address = 0x00465780;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	Order* remove_order_from_unit_queue(Order* order, Unit* unit) {
		static Order* result;
		const u32 func_address = 0x004742D0;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EAX, order
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void prepend_order_targeting_unit(Unit* unit, Unit* target, u8 order_id) {
		const u32 Func_prependOrderTargetingUnit = 0x00474A70;
		__asm {
			PUSHAD
			MOVZX ECX, order_id
			MOV EDX, unit
			MOV EAX, target
			CALL Func_prependOrderTargetingUnit
			POPAD
		}
	}

	bool is_target_within_min_movement_range(Unit* unit, Unit* target, u32 range) {
		const u32 Func_isTargetWithinMinMovementRange = 0x004763D0;
		static b32 result;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOV EAX, target
			MOV ECX, range
			CALL Func_isTargetWithinMinMovementRange
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	bool ai_is_unreachable_position(Unit* unit, s32 x, s32 y) {
		const u32 Func_Sub476610 = 0x00476610;
		static b32 result;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EBX, x
			MOV EDI, y
			CALL Func_Sub476610
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	bool is_unit_in_weapon_range(Unit* unit, Unit* target) {
		const u32 Func_isUnitInWeaponRange = 0x00476870;
		static b32 result;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, target
			CALL Func_isUnitInWeaponRange
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	void get_weapon_begin_iscript(Unit* unit, IscriptAnimation::Enum anim) {
		const u32 func_address = 0x00476ED0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOVZX EDX, anim
			CALL func_address
			POPAD
		}
	}

	void pick_new_target_if_needed(Unit* unit) {
		const u32 func_address = 0x00477510;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	void attack_move_try_pick_target(Unit* unit, u8 order_id) {
		const u32 func_address = 0x00477820;
		u32 order_id_xd = order_id;
		__asm {
			PUSHAD
			PUSH order_id_xd
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	bool attack_move_react_to_attack(Unit* unit, u8 order_id) {
		const u32 func_address = 0x00478370;
		static b32 result;
		u32 order_id_xd = order_id;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH order_id_xd
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	bool attack_unit_main(Unit* unit) {
		const u32 func_address = 0x00478540;
		static b32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	// 0x0047B450
	bool unit_is_critter(Unit* unit) {
		return false;
	}

	void move_to_collide(Unit* unit, Unit* target) {
		const u32 func_address = 0x00495400;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOV EAX, target
			CALL func_address
			POPAD
		}
	}

	bool is_unit_visible(Unit* unit) {
		const u32 Func_isUnitVisible = 0x004E5DB0;
		static b32 result;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_isUnitVisible
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	bool is_target_visible(Unit* unit, Unit* target) {
		static b32 result;
		const u32 func_address = 0x004E5E30;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EAX, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	u8 get_right_click_action_order(Unit* unit) {
		static u8 result;
		const u32 func_address = 0x004E5EA0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	// Not related with assigning orders, but only with destinations of orders
	void make_to_hold_position(Unit* unit) {
		const u32 func_address = 0x004EB5B0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	bool move_to_target(Unit* target, Unit* unit) {
		const u32 func_address = 0x004EB720;
		static b32 result;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EDI, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	bool set_move_target_xy(Unit* unit, s32 x, s32 y) {
		const u32 func_address = 0x004EB820;
		static b32 result;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EBX, x
			MOV EDI, y
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	// Related to path/movement decision
	void set_resource_target(Unit* unit) {
		const u32 func_address = 0x004EB290;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	bool change_movement_target_move_target_2(Unit* unit, s32 x, s32 y) {
		const u32 func_address = 0x004EB9C0;
		static b32 result;
		__asm {
			PUSHAD
			MOV EDX, unit
			MOV ECX, x
			MOV EAX, y
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	bool is_distance_greater_than_halt_distance(Unit* unit, s32 x, s32 y, u32 distance) {
		const u32 func_address = 0x00401240;
		static b32 result;
		__asm {
			PUSHAD
			PUSH x
			PUSH distance
			MOV EAX, y
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	void check_unstack(Unit* unit) {
		const u32 func_address = 0x0042D9A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	bool begin_building(Unit* unit) {
		const u32 func_address = 0x00467030;
		static b32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	Unit* begin_gas_building(Unit* unit, u32 unit_id) {
		const u32 func_address = 0x004678A0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// imageId may not be really a parameter
	void add_overlay_below_main(Sprite* sprite, u32 imageId, u32 x_offset, u32 y_offset, u32 direction) {
		const u32 func_address = 0x00498D70;
		__asm {
			PUSHAD
			MOV EAX, sprite
			MOV ESI, imageId
			PUSH x_offset
			PUSH y_offset
			PUSH direction
			CALL func_address
			POPAD
		}
	}

	void replace_sprite_images(Sprite* sprite, u32 imageId, u32 imageDirection) {
		const u32 func_address = 0x00499BB0;
		__asm {
			PUSHAD
			PUSH imageDirection
			PUSH imageId
			MOV EAX, sprite
			CALL func_address
			POPAD
		}
	}

	void display_last_network_error_for_player(u32 player) {
		const u32 func_address = 0x0049E530;
		__asm {
			PUSHAD
			PUSH player
			CALL func_address
			POPAD
		}
	}

	void update_unit_strength(Unit* unit) {
		const u32 func_address = 0x0049FA40;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void iscript_opcode_cases(Image* image, void* iscript_header, u32 dont_modify_image, u32 out_value) {
		const u32 func_address = 0x004D74C0;
		__asm {
			PUSHAD
			PUSH out_value
			PUSH dont_modify_image
			PUSH iscript_header
			MOV ECX, image
			CALL func_address
			POPAD
		}
	}

	void switch_to_warp_texture(Image* image) {
		const u32 func_address = 0x004D8500;
		__asm {
			PUSHAD
			MOV EAX, image
			CALL func_address
			POPAD
		}
	}

	bool set_thingy_visibility_flags(Thingy* thingy) {
		const u32 func_address = 0x004878F0;
		static b32 result;
		__asm {
			PUSHAD
			MOV ESI, thingy
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	Thingy* create_thingy(u32 spriteId, s16 x, s16 y, u32 player) {
		const u32 func_address = 0x00488210;
		static Thingy* thingy;
		s32 x_xd = x;
		__asm {
			PUSHAD
			PUSH player
			MOVSX EDI, y
			PUSH x_xd
			PUSH spriteId
			CALL func_address
			MOV thingy, EAX
			POPAD
		}
		return thingy;
	}

	void play_sound_at_pos(u32 sound_id, Point16 pos, u32 bool_play, u32 min_volume) {
		const u32 func_address = 0x0048EC10;
		__asm {
			PUSHAD
			PUSH min_volume
			PUSH bool_play
			PUSH pos
			MOV EBX, sound_id
			CALL func_address
			POPAD
		}
	}

	u32 randomize_short(u32 unknown_index) {
		const u32 func_address = 0x004DC4A0;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV EAX, unknown_index
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	const char* set_unit_stat_txt_error_message(const char* message) {
		static const char* result;
		const u32 func_address = 0x0048CCB0;
		__asm {
			PUSHAD
			MOV EAX, message
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void move_unit_duplicate(Unit* unit, s32 x, s32 y) {
		const u32 func_address = 0x004EBAE0;
		__asm {
			PUSHAD
			MOV EDX, unit
			MOV EAX, x
			MOV ECX, y
			CALL func_address
			POPAD
		}
	}

	Unit* spider_mine_acquire_target(Unit* mine) {
		const u32 func_address = 0x00441270;
		static Unit* target;
		__asm {
			PUSHAD
			MOV ESI, mine
			CALL func_address
			MOV target, EAX
			POPAD
		}
		return target;
	}

	void instant_cloak(Unit* unit) {
		const u32 func_address = 0x0049B1E0;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	void unburrow_generic(Unit* unit) {
		const u32 func_address = 0x004E99D0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void burrow_generic(Unit* unit) {
		const u32 func_address = 0x004E9A30;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	// For orders_place_mine
	s32 tech_use_allowed(Unit* unit, u16 tech_id, u32 player) {
		const u32 func_address = 0x0046DD80;
		static s32 result;
		__asm {
			PUSHAD
			PUSH player
			MOVZX DI, tech_id
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_distance_fast(s32 pos1_x, s32 pos2_x, s32 pos1_y, s32 pos2_y) {
		const u32 func_address = 0x0040C360;
		static u32 result;
		__asm {
			PUSHAD
			PUSH pos2_y
			PUSH pos1_y
			MOV EDX, pos2_x
			MOV ECX, pos1_x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 orders_hold_pos_suicidal(Unit* unit) {
		const u32 func_address = 0x004EB5B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void turn_unit(Unit* unit, s8 direction) {
		const u32 func_address = 0x00495F20;
		__asm {
			PUSHAD
			MOVZX BL, direction
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void to_idle(Unit* unit) {
		const u32 func_address = 0x004753A0;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	s32 is_unit_movable(Unit* unit) {
		const u32 func_address = 0x00401DC0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void do_next_queued_order(Unit* unit) {
		const u32 func_address = 0x00475000;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	// For burrow_generic
	void toggle_unit_path(Unit* unit) {
		const u32 func_address = 0x004E42A0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	char switch_sprite_id(Sprite* sprite, s32 sprite_id) {
		const u32 func_address = 0x00499860;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, sprite_id
			MOV ECX, sprite
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void refund_building_cost(u16 unit_id, u8 player) {
		const u32 func_address = 0x0042CE70;
		__asm {
			PUSHAD
			MOVZX ECX, player
			MOVZX EAX, unit_id
			CALL func_address
			POPAD
		}
	}

	void update_building_ai(Unit* unit) {
		const u32 func_address = 0x00433FE0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	// is_valid_morph @ 0x0045CC60;
	bool is_valid_building_morph(u16 building_id) {
		bool result;
		if (bw::is_zerg_building_morph_result(building_id)) {
			result = true;
		} else {
			result = false;
		}
		return result;
	}

	void update_new_unit_vision(u32 id, s32 x, s32 y) {
		const u32 func_address = 0x0045CE90;
		__asm {
			PUSHAD
			MOV ECX, id
			PUSH y
			PUSH x
			CALL func_address
			POPAD
		}
	}

	void cancel_zerg_building_morph(Unit* unit) {
		const u32 func_address = 0x0045D410;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void stop_creep_disappearing_at_building(Unit* unit) {
		const u32 func_address = 0x0047D770;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	// unk1 is ECX, unk2 is pushed value
	void increment_unit_scores_ex(Unit* unit, s32 count, b32 update_morph_merge_unit_score) {
		const u32 func_address = 0x00488D50;
		__asm {
			PUSHAD
			PUSH update_morph_merge_unit_score
			MOV ECX, count
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	void play_building_completed_sound(Unit* building) {
		const u32 func_address = 0x0048F310;
		__asm {
			PUSHAD
			MOV EAX, building
			CALL func_address
			POPAD
		}
	}

	void refresh_sprite_data(Sprite* sprite) {
		const u32 func_address = 0x004983A0;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL func_address
			POPAD
		}
	}

	void replace_unit_with_id(Unit* unit, u16 new_unit_id) {
		const u32 func_address = 0x0049FED0;
		u32 new_unit_id_xd = new_unit_id;
		__asm {
			PUSHAD
			PUSH new_unit_id_xd
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void fix_target_location(Point16* coords, u32 unit_id) {
		const u32 func_address = 0x00401FA0;
		__asm {
			PUSHAD
			MOV EAX, unit_id
			MOV EDX, coords
			CALL func_address
			POPAD
		}
	}

	// Equivalent to function @ 0x00402160
	bool is_unit_positions_2_equal(Unit* unit) {
		return (unit->next_target_waypoint.x == unit->position.x && unit->next_target_waypoint.y == unit->position.y);
	}

	// if canBeAborted is TRUE, the unit can be aborted because there's
	// no room around the constructing building
	b32 progress_build(Unit* unit, u32 hpGain, b32 showHidden) {
		const u32 func_address = 0x004679A0;
		static b32 bResult;
		__asm {
			PUSHAD
			MOV EDX, hpGain
			MOV EAX, unit
			PUSH showHidden
			CALL func_address
			MOV bResult, EAX
			POPAD
		}
		return bResult;
	}

	void prepend_order_targeting_nothing(Unit* unit, Order* order, u8 order_id) {
		const u32 func_address = 0x00474760;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EDI, order
			MOVZX EBX, order_id
			CALL func_address
			POPAD
		}
	}

	void add_order_targeting_nothing(Unit* unit, Order* order, u8 order_id) {
		const u32 func_address = 0x004749A0;
		__asm {
			PUSHAD
			MOVZX EBX, order_id
			MOV ESI, unit
			MOV EDI, order
			CALL func_address
			POPAD
		}
	}

	bool can_still_place_building() {
		const u32 func_address = 0x0048DDA0;
		b32 pre_return_value;
		__asm {
			PUSHAD
			CALL func_address
			MOV pre_return_value, EAX
			POPAD
		}
		return (pre_return_value != 0);
	}

	void end_building_placement() {
		const u32 func_address = 0x0048E310;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	u32 rand_between(u32 min, u32 max, u32 seed_type) {
		const u32 func_address = 0x004DC550;
		static u32 return_value;
		__asm {
			PUSHAD
			PUSH max
			MOV EAX, seed_type
			MOV EDX, min
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	char refresh_layers_3_and_4() {
		const u32 func_address = 0x0048D9A0;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void unit_completion_init(Unit* unit, b32 has_construction_anim) {
		const u32 func_address = 0x004E65E0;
		__asm {
			PUSHAD
			MOV EDI, unit
			PUSH has_construction_anim
			CALL func_address
			POPAD
		}
	}

	bool order_to_move_to_target(Unit* unit, Unit* target) {
		const u32 func_address = 0x004EB980;
		static b32 result;
		__asm {
			PUSHAD
			MOV EAX, target
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	b32 unk_terrain_collision_2(Unit* unit, s32 x, s32 y) {
		const u32 func_address = 0x004F1870;
		static b32 rValue;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			MOV EAX, unit
			CALL func_address
			MOV rValue, EAX
			POPAD
		}
		return rValue;
	}

	// For update_placement_state_for_placement_box
	s32 can_build_on_tile_location(Unit* builder, s32 player, s32 xTile, s32 yTile, s32 unit_id, s32 placementStateEntry, s32 bCheckVision, s32 bAlsoInvisible, s32 bWithoutVision) {
		const u32 func_address = 0x00473FB0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH bWithoutVision
			PUSH bAlsoInvisible
			PUSH bCheckVision
			PUSH placementStateEntry
			PUSH unit_id
			PUSH yTile
			PUSH xTile
			PUSH player
			PUSH builder
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For refresh_screen
	s32 update_placement_state_for_placement_box() {
		const u32 func_address = 0x0048DCE0;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 unk_building_placement_related(s32 a1, Point16* a2) {
		const u32 func_address = 0x0048D7F0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a2
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 init_placement_box_pos(Point16* in_out) {
		const u32 func_address = 0x0048D660;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, in_out
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char unk_building_placement_related_mouse(s32 a1, s32 a2) {
		const u32 func_address = 0x0048D7B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void set_unit_pathing(Unit* unit, u8 unkPathRelated) {
		const u32 func_address = 0x00401360;
		__asm {
			PUSHAD
			MOVZX EAX, unkPathRelated
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	Unit* connect_addon_check(Unit* unit) {
		const u32 func_address = 0x004404A0;
		Unit* found_addon;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV found_addon, EAX
			POPAD
		}
		return found_addon;
	}

	void modify_building_grounded_status(Unit* unit) {
		const u32 func_address = 0x00463470;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void toggle_pathing_related(Unit* unit) {
		const u32 func_address = 0x004634E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void flying_building_lift_if_still_blocked(Unit* unit) {
		const u32 func_address = 0x00463640;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void complete_addon(Unit* unit, Unit* addon) {
		const u32 func_address = 0x00463D50;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOV EAX, addon
			CALL func_address
			POPAD
		}
	}

	void crush_immobile_units(Unit* unit) {
		const u32 func_address = 0x00464300;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	void disconnect_addon(Unit* unit) {
		const u32 func_address = 0x00464930;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void clear_building_from_tile_flag(Unit* unit, s32 x, s32 y) {
		const u32 func_address = 0x00469EC0;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH y
			PUSH x
			CALL func_address
			POPAD
		}
	}

	void set_building_tile_flag(Unit* unit, s32 x, s32 y) {
		const u32 func_address = 0x00469F60;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH y
			PUSH x
			CALL func_address
			POPAD
		}
	}

	void end_collision_tracking(Unit* unit) {
		const u32 func_address = 0x0046A560;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void start_collision_tracking(Unit* unit) {
		const u32 func_address = 0x0046A5A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	s32 modify_creep_state(s32 x_tile, s32 y_tile, CreepChange* creepChange, u32 param) {
		const u32 func_address = 0x00413FA0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH param
			PUSH creepChange
			PUSH y_tile
			PUSH x_tile
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void force_order_done(Unit* unit) {
		const u32 func_address = 0x00475420;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	bool update_creep_disappearance_unit(Unit* unit) {
		const u32 func_address = 0x0047DF90;
		b32 pre_return_value;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			MOV pre_return_value, EAX
			POPAD
		}

		return (pre_return_value != 0);
	}

	void update_creep_disappearance_unit_id(u32 unit_id, u32 x, u32 y, b32 completed) {
		const u32 func_address = 0x0047DE40;
		__asm {
			PUSHAD
			MOV EAX, completed
			MOV ECX, y
			MOV EDX, x
			PUSH unit_id
			CALL func_address
			POPAD
		}
	}

	void place_building_proc() {
		const u32 func_address = 0x0048E6E0;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void play_landing_interrupt_sound(Unit* unit) {
		const u32 func_address = 0x0048F5A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	Unit* set_unit_movement_speed(Unit* unit, u32 speed) {
		static Unit* result;
		const u32 func_address = 0x004951C0;
		__asm {
			PUSHAD
			MOV EDX, speed
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void set_shadow_y(Sprite* sprite, u8 newVerticalOffset) {
		const u32 func_address = 0x00498150;
		__asm {
			PUSHAD
			MOV EAX, sprite
			MOVZX ECX, newVerticalOffset
			CALL func_address
			POPAD
		}
	}

	void create_landing_dust(Sprite* sprite) {
		const u32 func_address = 0x004997E0;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL func_address
			POPAD
		}
	}

	void remove_from_selection(Unit* unit) {
		const u32 func_address = 0x004C3B40;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL func_address
			POPAD
		}
	}

	b32 spreads_creep(u32 unit_id, b32 status) {
		const u32 func_address = 0x00413870;
		b32 return_value;
		__asm {
			PUSHAD
			MOV EAX, unit_id
			PUSH status
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void apply_creep_at_location_from_unit_id(s32 x, s32 y, u32 unit_id) {
		const u32 func_address = 0x004148F0;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			PUSH unit_id
			CALL func_address
			POPAD
		}
	}

	void create_building_larva(Unit* main_building) {
		const u32 func_address = 0x0049D660;
		__asm {
			PUSHAD
			MOV ESI, main_building
			CALL func_address
			POPAD
		}
	}

	b32 show_finished_unit(Unit* unit) {
		const u32 func_address = 0x0049EC30;
		b32 bResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV bResult, EAX
			POPAD
		}
		return bResult;
	}

	Unit* find_best_unit(Box16* coords, u32 searchProc) {
		const u32 func_address = 0x004E8830;
		static Unit* unit;
		__asm {
			PUSHAD
			MOV EAX, coords
			MOV EDI, searchProc
			CALL func_address
			MOV unit, EAX
			POPAD
		}
		return unit;
	}

	u8 spread_creep(u32 unit_id, s32 x, s32 y) {
		const u32 func_address = 0x00414680;
		static u8 return_value;
		__asm {
			PUSHAD
			MOV ECX, x
			MOV EAX, y
			MOV EDX, unit_id
			CALL func_address
			MOV return_value, AL
			POPAD
		}
		return return_value;
	}

	void set_larva_order_state(Unit* unit) {
		const u32 func_address = 0x004E8DA0;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	bool get_larva_spawn_pos(Unit* unit, Vector32* coords) {
		const u32 func_address = 0x004E8E10;
		static b32 return_pre_value;
		__asm {
			PUSHAD
			MOV EDI, coords
			MOV EAX, unit
			CALL func_address
			MOV return_pre_value, EAX
			POPAD
		}
		return (return_pre_value != 0);
	}

	u32 get_num_surrounding_creep_tiles(u32 x, u32 y) {
		const u32 func_address = 0x00414290;
		static u32 return_val;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			CALL func_address
			MOV return_val, EAX
			POPAD
		}
		return return_val;
	}

	void set_creep_relational_data(u32 x, u32 y, SetCreepRelationalData_callback callback, u32 arg) {
		const u32 func_address = 0x00413FA0;
		__asm {
			PUSHAD
			PUSH arg
			PUSH callback
			PUSH y
			PUSH x
			CALL func_address
			POPAD
		}
	}

	u32 iterate_directional_creep_data(u32 x, u32 y, u32 callback, u32 arg) {
		const u32 func_address = 0x00413B30;
		static u32 return_val;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			MOV EBX, callback
			MOV EDI, arg
			CALL func_address
			MOV return_val, EAX
			POPAD
		}
		return return_val;
	}

	// Note: this function was made from what was seen in this case of use, some things are hardcoded because it's hard to tell what is used or not.
	// Basically, used elsewhere, the function would be implemented differently
	void targeted_order(Unit* unit, u32 unit_id, u32 unk1, u32 unk2, u32 order_id, u32 unk3, u32 unk4, u32 unk5) {
		const u32 func_address = 0x004754F0;
		__asm {
			PUSHAD
			MOV EAX, 0
			MOV EDX, 0
			MOV EBX, unit_id
			MOV ESI, unit
			PUSH unk1
			PUSH unk2
			PUSH unk3
			PUSH order_id
			PUSH unk4
			PUSH unk5
			CALL func_address
			POPAD
		}
	}

	void clear_unit_sprites(Sprite* unit_sprite, Sprite* subunit_sprite) {
		const u32 func_address = 0x00496F00;
		__asm {
			PUSHAD
			MOV ECX, subunit_sprite
			MOV ESI, unit_sprite
			CALL func_address
			POPAD
		}
	}

	void refresh_all_visible_images_at_screen_pos(Sprite* thingy_sprite, u32 visibility_flags) {
		const u32 func_address = 0x00497480;
		__asm {
			PUSHAD
			MOV EBX, thingy_sprite
			PUSH visibility_flags
			CALL func_address
			POPAD
		}
	}

	void unit_die(Unit* unit) {
		const u32 func_address = 0x004A0740;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void remove_unit(Unit* unit) {
		const u32 func_address = 0x004A0990;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	void hide_and_disable_unit(Unit* unit) {
		const u32 func_address = 0x004E6340;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void cancel_research_proc(Unit* unit) {
		const u32 func_address = 0x00453E30;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL func_address
			POPAD
		}
	}

	void cancel_upgrade_proc(Unit* unit) {
		const u32 func_address = 0x00454280;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void refund_queue_slot(Unit* unit, u16 slot) {
		const u32 func_address = 0x00466A70;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOVZX EAX, slot
			CALL func_address
			POPAD
		}
	}

	void cancel_last_training(Unit* unit) {
		const u32 func_address = 0x00466E40;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void cancel_addon_proc(Unit* unit) {
		const u32 func_address = 0x004E66E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	b32 order_is_allowed(Unit* unit, u16 order, u32 nationID) {
		const u32 func_address = 0x0046DC20;
		static b32 bResult;
		__asm {
			PUSHAD
			MOVZX EBX, order
			MOV EAX, unit
			PUSH nationID
			CALL func_address
			MOV bResult, EAX
			POPAD
		}
		return bResult;
	}

	bool place_building_message(u32 result_function_473FB0) {
		const u32 func_address = 0x0048D930;
		static b32 result;
		__asm {
			PUSHAD
			MOV EAX, result_function_473FB0
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	bool CMDRECV_place_building_allowed(Unit* unit, u8 order_id, u16 builtUnitId) {
		const u32 func_address = 0x0048DBD0;
		static b32 result;
		__asm {
			PUSHAD
			MOVZX EAX, builtUnitId
			MOVZX EDX, order_id
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	void process_building_placement(Unit* unit, u32 order_id, u16 dimensionX, u16 dimensionY, u32 builtUnitId) {
		const u32 func_address = 0x0048DE70;
		static Point16 dim;
		dim.x = dimensionX; dim.y = dimensionY;
		__asm {
			PUSHAD
			PUSH builtUnitId
			PUSH dim
			PUSH order_id
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void CMDRECV_place_building(s16 x, s16 y, u8 order_id, u16 builtUnitId) {
		const u32 func_address = 0x0048E190;
		static Point16 pos;
		pos.x = x; pos.y = y;
		__asm {
			PUSHAD
			MOVZX EAX, builtUnitId	// bugfix: should have been "MOVZX EAX, builtUnitId" but for some reason it crashed
			MOVZX ECX, order_id
			PUSH pos
			CALL func_address
			POPAD
		}
	}

	Unit* templar_merge_partner(Unit* unit, Unit** units_list, u32 units_list_length) {
		const u32 func_address = 0x004BFA80;
		static Unit* return_value;
		__asm {
			PUSHAD
			MOV EAX, units_list_length
			PUSH units_list
			PUSH unit
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void parse_orders_dat_requirements() {
		const u32 func_address = 0x0046D450;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	u16* parse_tech_dat_requirements() {
		const u32 func_address = 0x0046D4C0;
		static u16* result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 parse_requirement_opcodes(Unit* unit, u32 order_id, u32 player, u16* address, u32 req_offset) {
		static s32 return_value;
		const u32 func_address = 0x0046D610;
		__asm {
			PUSHAD
			PUSH address
			PUSH player
			PUSH order_id
			MOV EAX, req_offset
			MOV ESI, unit
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void apply_speed_upgrade_from_unit_type(Unit* unit) {
		const u32 func_address = 0x00454370;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	void give_ai(Unit* unit) {
		const u32 func_address = 0x00463040;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void recheck_attacks(Unit* unit) {
		const u32 func_address = 0x00476D30;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	void increment_unit_scores(Unit* unit, s32 count) {
		const u32 func_address = 0x00488BF0;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH count
			CALL func_address
			POPAD
		}
	}

	void create_unit_selection(Unit* unit, u32 selection_index) {
		const u32 Func_CreateUnitSelection = 0x004E6180;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH selection_index
			CALL Func_CreateUnitSelection
			POPAD
		}
	}

	void destroy_unit_selection(Unit* unit) {
		const u32 Func_DestroyUnitSelection = 0x004E6290;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_DestroyUnitSelection
			POPAD
		}
	}

	void drop_powerup(Unit* powerup) {
		const u32 Func_drop_powerup = 0x004F3B70;
		__asm {
			PUSHAD
			PUSH powerup
			CALL Func_drop_powerup
			POPAD
		}
	}

	u32 get_weapon_cooldown(Unit* unit, u8 weapon_id) {
		const u32 Func_getUpgradedWpnCooldown = 0x00475DC0;
		static u32 return_value;
		__asm {
			PUSHAD
			MOVZX EAX, weapon_id
			MOV ESI, unit
			CALL Func_getUpgradedWpnCooldown
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	bool is_ready_to_attack(Unit* unit, u32 weapon_id) {
		const u32 Func_is_ready_to_attack = 0x00476640;
		static b32 pre_result;
		__asm {
			PUSHAD
			PUSH weapon_id
			MOV EAX, unit
			CALL Func_is_ready_to_attack
			MOV pre_result, EAX
			POPAD
		}
		return (pre_result != 0);
	}

	u32 get_stat_txt_string(Unit* unit, u32 player, const char* message) {
		const u32 Func_get_statTxt_Str_0 = 0x0048EF30;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV EAX, message
			MOV ECX, player
			MOV EDX, unit
			CALL Func_get_statTxt_Str_0
			MOV EAX, return_value
			POPAD
		}
		return return_value;
	}

	bool can_cast_spell(Unit* unit) {
		const u32 Func_canCastSpell_0 = 0x00492140;
		static b32 result;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_canCastSpell_0
			MOV result, EAX
			POPAD
		}
		// 0 means "can cast spell"
		return (result == 0);
	}

	u32 ordered_spell_progress_moving(Unit* unit, b32 wasMainOrderStateZero) {
		const u32 Func_Sub492620 = 0x00492620;
		static u32 return_value;
		__asm {
			PUSHAD
			PUSH wasMainOrderStateZero
			MOV EAX, unit
			CALL Func_Sub492620
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void refund_all_queue_slots(Unit* unit) {
		const u32 Func_RefundAllQueueSlots = 0x00466E80;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RefundAllQueueSlots
			POPAD
		}
	}

	void increment_unit_death_scores(Unit* unit, u8 player) {
		const u32 Func_IncrementUnitDeathScores = 0x00488AF0;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOVZX EDX, player
			CALL Func_IncrementUnitDeathScores
			POPAD
		}
	}

	void give_sprite(Unit* unit, u8 player) {
		const u32 func_address = 0x0049E4E0;
		__asm {
			PUSHAD
			MOVZX EDX, player
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	void give_unit(Unit* unit, u32 player, s32 unkScore) {
		const u32 func_address = 0x0049EFA0;
		__asm {
			PUSHAD
			PUSH unkScore
			MOV ECX, unit
			PUSH player
			CALL func_address
			POPAD
		}
	}

	void change_unit_buttonset(Unit* unit, u16 buttonSetId) {
		const u32 func_address = 0x004E5D60;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOVZX ECX, buttonSetId
			CALL func_address
			POPAD
		}
	}

	// What make the target reappear
	void show_and_enable_unit(Unit* unit) {
		const u32 func_address = 0x004E6490;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	bool is_infestable_unit(Unit* unit) {
		const u32 func_address = 0x00402210;
		static b32 result;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	// For update_unit_strength_and_apply_default_orders
	void begin_cloak(Unit* unit, s32 sound_id) {
		const u32 func_address = 0x0049B5B0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, sound_id
			CALL func_address
			POPAD
		}
	}

	void refresh_unit_vision(Unit* unit) {
		const u32 func_address = 0x004E5F30;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	void init_unit_trap_doodad(Unit* unit) {
		const u32 func_address = 0x004E6490;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	Order* perform_another_order(u8 order_id, Order* order, Unit* unit, s16 fow_unit_id, Point16 position_xy, Unit* target) {
		const u32 func_address = 0x004745F0;
		static Order* result;
		__asm {
			PUSHAD
			MOVZX BL, order_id
			MOV EDI, order
			MOV ESI, unit
			MOVZX DX, fow_unit_id
			PUSH position_xy
			PUSH target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void order(s32 some_bool, Unit* unit, s8 order_id, Point16 pos, Unit* target_unit, u32 fow_unit_id) {
		const u32 func_address = 0x00474810;
		__asm {
			PUSHAD
			PUSH fow_unit_id
			PUSH target_unit
			PUSH pos
			MOV EAX, some_bool
			MOV EDX, unit
			MOVZX CL, order_id
			CALL func_address
			POPAD
		}
	}

	u32 get_unit_strength(Unit* a1, s32 a2, bool a3) {
		const u32 Func_getUnitStrength = 0x00431800;
		static u32 result;
		b32 a3_xd = a3;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			PUSH a3_xd
			CALL Func_getUnitStrength
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void play_worker_error_sound(Unit* unit, u32 error_text_index) {
		const u32 func_address = 0x0048F6C0;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH error_text_index
			CALL func_address
			POPAD
		}
	}

	bool is_unit_upgrade_available(Unit* unit) {
		const u32 func_address = 0x004033D0;
		static b32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	void apply_speed_upgrade_from_upgrade_id(Unit* unit, u8 upgrade_id) {
		const u32 func_address = 0x00454540;
		__asm {
			PUSHAD
			MOVZX EAX, upgrade_id
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	void play_upgrade_complete_sound() {
		const u32 func_address = 0x0048F070;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void play_research_complete_sound() {
		const u32 func_address = 0x0048F150;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void set_upgrade_level(u32 player, u32 upgrade_id, u8 upgrade_level) {
		const u32 func_address = 0x004CE770;
		__asm {
			PUSHAD
			MOVZX EDX, upgrade_level
			MOV ECX, upgrade_id
			MOV EAX, player
			CALL func_address
			POPAD
		}
	}

	// return true if remaining_build_time was 0 when called, remaining_build_time change affected by cheating
	bool advance_remaining_build_time(Unit* unit) {
		u32 func_address = 0x00466940;
		static b32 bTimeWasZero;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV bTimeWasZero, EAX
			POPAD
		}
		return (bTimeWasZero != 0);
	}

	// return value may be useless since it's the exact value of pathing_flags
	u8 delete_pathing(Unit* unit) {
		u32 func_address = 0x0046A820;
		static u8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}

		return result;
	}

	void order_immediate(Unit* unit, u8 order) {
		u32 func_address = 0x00474B40;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOVZX EAX, order
			CALL func_address
			POPAD
		}
	}

	void play_unit_ready_sound(Unit* unit) {
		const u32 func_address = 0x0048F440;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	// get status (defensive_matrix_hp to acidSporeTime) of unit_src and give it to unit_dst
	void merge_status_into_unit(Unit* unit_dst, Unit* unit_src) {
		const u32 func_address = 0x00493180;
		__asm {
			PUSHAD
			MOV ECX, unit_src
			MOV EAX, unit_dst
			CALL func_address
			POPAD
		}
	}

	void end_cloak(Unit* unit, u32 sound_id) {
		const u32 func_address = 0x0049B440;
		__asm {
			PUSHAD
			MOV EBX, sound_id
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	// return true if waiting for recent_order_timer to reach 0 or if setMoveTarget_xy returned true
	bool delayed_set_move_target_xy(Unit* unit, u16 x, u16 y) {
		const u32 func_address = 0x004EB960;
		b32 result;
		__asm {
			PUSHAD
			MOVSX EDI, y
			MOVSX EBX, x
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	void set_all_overlay_directions_generic(Unit* unit, u32 unk) {
		const u32 func_address = 0x00401140;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unk
			CALL func_address
			POPAD
		}

	}

	void ai_order_unit_cloaking(Unit* unit) {
		const u32 func_address = 0x0043B970;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	u32 ai_return_to_nearest_base_forced(Unit* unit) {
		const u32 func_address = 0x0043DB50;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	s32 get_unit_max_weapon_range(Unit* unit, u8 weapon_id) {
		const u32 func_address = 0x00475870;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOVZX BL, weapon_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for is_ready_to_attack
	u32 get_distance_to_unit(Unit* unit_1, Unit* unit_2) {
		const u32 func_address = 0x00430D30;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit_1
			MOV ECX, unit_2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 distance_from_dbox_corner(Unit* a1, __int16 a2, __int16 a3) {
		const u32 func_address = 0x0043E50;
		static u32 result;
		s16 a2_xd = a2;
		s16 a3_xd = a3;
		__asm {
			PUSHAD
			PUSH a3_xd
			PUSH a2_xd
			PUSH a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool is_angle_in_attack_angle(s32 a1, s32 a2, Unit* a3, unsigned __int8 a4) {
		const u32 func_address = 0x00475BE0;
		static s32 result;
		static bool result2;
		u32 a4_xd = a4;
		__asm {
			PUSHAD
			PUSH a4_xd
			MOV EAX, a1
			MOV ECX, a2
			MOV ESI, a3
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result2 = (bool)result;
	}

	// for can_cast_ordered_spell_now
	bool is_target_within_min_range(Unit* unit, u32 min_range, Unit* target) {
		const u32 func_address = 0x00430F10;
		static s32 result;
		static bool result2;

		__asm {
			PUSHAD
			PUSH target
			PUSH min_range
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result = (bool)result;
	}

	s32 get_flingy_halt_distance(Unit* unit) {
		static s32 result;
		const u32 func_address = 0x00494F90;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for orders_watch_target
	void unknown_orders_func(s32 a1, __int16 a2, __int16 a3) {
		const u32 func_address = 0x00475350;
		s32 a2_xd = a2;
		s32 a3_xd = a3;
		__asm {
			PUSHAD
			PUSH a3_xd
			PUSH a2_xd
			MOV EAX, a1
			CALL func_address
			POPAD
		}
	}

	// for orders_stay_in_range
	void prepend_order_targeting_ground(__int8 order_id, __int16 x, __int16 y, Unit* unit) {
		const u32 func_address = 0x00474A20;
		__asm {
			PUSHAD
			PUSH unit
			MOVZX CX, y
			MOVZX DX, x
			MOVZX AL, order_id
			CALL func_address
			POPAD
		}
	}

	s32 move_towards(Unit* unit, Unit* target) {
		const u32 func_address = 0x004EB900;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EAX, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 orders_hold_position_suicidal(Unit* a1) {
		const u32 func_address = 0x004EB5B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_direction_from_points(s32 dest_y, s32 x, s32 dest_x, s32 y) {
		const u32 func_address = 0x00495300;
		static s32 result;
		__asm {
			PUSHAD
			PUSH y
			MOV EAX, dest_y
			MOV EDX, x
			MOV ECX, dest_x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_direction_from_points(Point16 a, Point16 b) {
		return get_direction_from_points(b.y, a.x, b.x, a.y);
	}
	
	void set_secondary_waypoint(Unit* unit, s32 x, s32 y) {
		const u32 func_address = 0x00494BB0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV ECX, y
			MOV EDX, x
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
	}

	// For generic_right_click_unit_proc_2
	// Not sure if unused is actually unused. -Pr0nogo
	s32 is_order_valid(char order_id, Unit* self, Unit* unused, Unit* target, s32* error) {
		const u32 func_address = 0x00474D90;
		static s32 result;
		__asm {
			PUSHAD
			PUSH error
			MOV ESI, target
			MOV EDI, unused
			MOV EBX, self
			MOVZX AL, order_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u16 get_unit_ability_error_stat_string(Unit* a1, char a2, s32 a3) {
		const u32 func_address = 0x00491E80;
		static u32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOVZX BL, a2
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	// for cmdrecv_hold_position

	Unit* get_active_player_next_selection() {
		const u32 func_address = 0x0049A850;
		static Unit* result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 order_allowed(Unit* a1, u16 a2, s32 a3) {
		const u32 func_address = 0x0046DC20;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOVZX BX, a2
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void targeted_order(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, char a7, s32 a8, s32 a9, s32 a10) {
		const u32 func_address = 0x004754F0;
		s32 a7_xd = a7;
		__asm {
			PUSHAD
			PUSH a10
			PUSH a9
			PUSH a8
			PUSH a7_xd
			PUSH a6
			PUSH a5
			MOVZX AX, a4
			MOV ESI, a3
			MOV EBX, a2
			MOVZX DX, a1
			CALL func_address
			POPAD
		}
	}

	// For check_unit_visibility
	u32 get_detecting_players(Unit* unit) {
		// There can be a nullsprite crash in this func despite being nullchecked previously. I am not sure how this is possible, nor how to fix it. -Pr0nogo
		const u32 func_address = 0x00441130;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 iterate_player_units_something(u32 aResult, Unit* unit) {
		const u32 func_address = 0x0049E8A0;
		static u32 result;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, aResult
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void make_sprite_visible(Sprite* sprite) {
		const u32 func_address = 0x00497ED0;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL func_address
			POPAD
		}
	}

	s32 update_building_land_unit_selection(u32 detection_status, Unit* selection) {
		const u32 func_address = 0x004C3B40;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, detection_status
			MOV EDX, selection
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char remove_selection_circle(Sprite* sprite) {
		const u32 func_address = 0x004975D0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, sprite
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	Image* make_sprite_invisible(Sprite* sprite) {
		const u32 func_address = 0x00498A10;
		static Image* result;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void function_004756B0(Unit* unit, u8 order_id, u32 unk1, u32 unk2, u32 command_type) {
		const u32 Func_Sub4756B0 = 0x004756B0;
		__asm {
			PUSHAD
			MOVZX EDX, order_id
			MOV ESI, unit
			PUSH unk1
			PUSH unk2
			PUSH command_type
			CALL Func_Sub4756B0
			POPAD
		}
	}

	void function_004756E0(Unit* unit, Unit* target, u32 order_id, u32 command_type) {
		const u32 func_address = 0x004756E0;
		__asm {
			PUSHAD
			MOV EDX, target
			MOV ESI, unit
			PUSH order_id
			PUSH command_type
			CALL func_address
			POPAD
		}
	}

	u32 can_do_targeted_order(Unit* unit, u8 order_id) {
		const u32 func_address = 0x0049A410;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOVZX EDX, order_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// Replaces 0049A480
	bool can_target_self(Unit* unit, u8 order_id) {
		bool return_value = false;
		if (unit->status & UnitStatus::GroundedBuilding && (order_id == OrderId::RallyPointToVisibleUnit || order_id == OrderId::RallyPointToTile)) {
			return_value = true;
		} else if ((unit->id == UnitId::ZergIsthrathaleth) && order_id == OrderId::CarapaceSwarm) {
			return_value = true;
		} else if (unit->status & UnitStatus::IsHallucination) {
			return_value = false;
		} else if (units_dat::space_provided[unit->id] == 0) {
			return_value = false;
		} else if (order_id == OrderId::MoveUnload) {
			return_value = true;
		}
		return return_value;
	}

	char ignore_iscript_go_to_idle(Unit* unit) {
		static char result;
		const u32 func_address = 0x00476550;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	u32 can_unload(Unit* transport, Unit* loaded_unit, Point16* pos) {
		const u32 func_address = 0x004E76C0;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, transport
			MOV ESI, loaded_unit
			PUSH pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void unload_unit_ext(Unit* unit, u32 unk) {
		const u32 func_address = 0x004E7E10;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unk
			CALL func_address
			POPAD
		}
	}

	void stat_nuke_count(BinDlg* dialog, u32 index) {
		const u32 func_address = 0x00425310;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL func_address
			POPAD
		}
	}

	void stats_hangar_count(BinDlg* dialog, u32 index) {
		const u32 func_address = 0x004253D0;
		__asm {
			PUSHAD
			MOV EBX, dialog
			PUSH index
			CALL func_address
			POPAD
		}
	}

	void stats_shield_level(BinDlg* dialog, u32 index) {
		const u32 func_address = 0x00425510;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL func_address
			POPAD
		}
	}

	void stats_armor_level(BinDlg* dialog, u32 index) {
		const u32 func_address = 0x00425600;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL func_address
			POPAD
		}
	}

	void stats_weapon_level(BinDlg* dialog, u32 index, u32 weapon_id) {
		const u32 func_address = 0x00425790;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH weapon_id
			PUSH index
			CALL func_address
			POPAD
		}
	}

	bool stat_spidermine_count(BinDlg* dialog, u32 index) {
		const u32 func_address = 0x00426300;
		static b32 result;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	// For get_unit_rank_string

	__int8 unitGetRace(Unit* a1) {
		const u32 func_address = 0x0047B150;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (__int8)result;
	}

	const char* get_stat_txt_string(u16 id) {
		const u32 func_address = 0x004C36F0;
		static s32 result;
		__asm {
			PUSHAD
			MOVZX CX, id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (const char*)result;
	}

	// For stats_weapon_level / stats_armor_level

	char getArmorCount(Unit* unit, s32 unused) {
		const u32 func_address = 0x00453FC0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, unused
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void show_dialog(BinDlg* dialog) {
		const u32 func_address = 0x004186A0;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL func_address
			POPAD
		}
	}

	s32 disable_dialog(BinDlg* dialog) {
		const u32 func_address = 0x00418640;
		static s32 result;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void update_dialog(BinDlg* dialog) {
		const u32 func_address = 0x0041C400;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	Unit* find_random_attack_target(Unit* unit) {
		const u32 func_address = 0x00442FC0;
		static Unit* target;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV target, EAX
			POPAD
		}
		return target;
	}

	b32 attack_apply_cooldown(Unit* unit) {
		const u32 func_address = 0x00478B40;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void set_image_direction(Image* image, s8 direction) {
		const u32 func_address = 0x004D5F80;
		u32 direction_xd = direction;
		__asm {
			PUSHAD
			PUSH direction_xd
			MOV ESI, image
			CALL func_address
			POPAD
		}
	}

	void apply_spores(Unit* unit) {
		const u32 func_address = 0x004F4480;
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL func_address
			POPAD
		}
	}

	// creates attack overlay & deals damage from the bullet (just calls overlayAndNotify if the weapon deals no damage, or calls target->damage_with if it does
	void weapon_bullet_shot(Unit* target, Bullet* bullet, u32 damage_divisor) {
		const u32 func_address = 0x00479AE0;
		if (bullet && target && bullet->source_unit && weapon_id_is_line_attack(bullet->weapon_id)) {
			if (!iquare_globals.was_hit_by_line_attack(bullet->source_unit, target)) {
				iquare_globals.register_line_attack_hit(bullet->source_unit, target);
			} else {
				return;
			}
		}
		if (iquare_globals.was_already_hit_by_multihit_bullet(bullet, target)) {
			return;
		}
		__asm {
			PUSHAD
			PUSH damage_divisor
			MOV EAX, target
			MOV EDX, bullet
			CALL func_address
			POPAD
		}
	}

	u8 set_archon_pathing(Unit* unit) {
		const u32 func_address = 0x0046A820;
		static u8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool has_supplies_for_unit(u32 unit_id, u32 player, b32 can_show_error_message) {
		return 1; // Supply was removed

		const u32 func_address = 0x0042CF70;
		static b32 result;
		__asm {
			PUSHAD
			PUSH can_show_error_message
			PUSH unit_id
			PUSH player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	Unit* zerg_dual_birth(Unit* unit) {
		const u32 func_address = 0x0045D910;
		static Unit* rValue;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			MOV rValue, EAX
			POPAD
		}
		return rValue;
	}

	void order_new_unit_to_rally(Unit* unit, Unit* factory) {
		const u32 func_address = 0x00466F50;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, factory
			CALL func_address
			POPAD
		}
	}

	void ai_training_unit(Unit* unit, Unit* creator) {
		const u32 func_address = 0x004A2830;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, creator
			CALL func_address
			POPAD
		}
	}

	// for orders_hallucination1
	bool spell_order(Unit* unit, u32 tech_id, u16* energy_cost, u32 sight_range, u32 error_message_index) {
		const u32 func_address = 0x004926D0;
		static b32 result;
		__asm {
			PUSHAD
			PUSH energy_cost
			PUSH sight_range
			PUSH tech_id
			MOV EAX, error_message_index
			MOV EBX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	bool place_hallucination(Unit* unit) {
		const u32 func_address = 0x004F66D0;
		static b32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	// for create_hallucination
	void apply_hallucination_stats(Unit* halUnit) {
		const u32 func_address = 0x004F6180;
		__asm {
			PUSHAD
			MOV EAX, halUnit
			CALL func_address
			POPAD
		}
	}

	s32 hallucinate_scarabs(Unit* a1, Unit* a2) {
		const u32 func_address = 0x004F65B0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a2
			MOV ESI, a1
			CALL func_address
			MOV EAX, result
			POPAD
		}
		return result;
	}

	void replace_feedback_sprite(Unit* unit, u32 spriteId) {
		const u32 func_address = 0x004F6350;
		__asm {
			PUSHAD
			MOV EBX, unit
			PUSH spriteId
			CALL func_address
			POPAD
		}
	}

	void ai_unit_was_hit(Unit* self, Unit* attacker, u32 main_target_reactions, u32 dont_call_help) {
		const u32 func_address = 0x0043F320;
		__asm {
			PUSHAD
			PUSH dont_call_help
			PUSH main_target_reactions
			MOV EDX, attacker
			MOV ECX, self
			CALL func_address
			POPAD
		}
	}

	// checks if the distance between target and bullet's positions is < radius
	bool is_unit_distance_within(Unit* target, Bullet* bullet, u32 radius) {
		const u32 func_address = 0x0048ADB0;
		u32 result;
		__asm {
			PUSHAD
			PUSH radius;
			PUSH bullet;
			MOV ECX, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void do_weapon_hit(Unit* target, Unit* attacker, u8 dmgDivisor) {
		const u32 func_address = 0x004795D0;
		u32 divisor_xd = dmgDivisor;
		__asm {
			PUSHAD
			PUSH divisor_xd
			PUSH attacker
			MOV EBX, target
			CALL func_address
			POPAD
		}
	}

	// for air_splash
	Unit* find_best_unit(Box16* radius, s32 bullet, s32(*func)(Unit*, Bullet*)) {
		const u32 func_address = 0x004E8830;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, radius
			MOV EBX, result
			MOV EDI, func
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 splash_func(Bullet* bullet, Unit* unit) {
		const u32 func_address = 0x0048B570;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDI, bullet
			MOV ESI, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void mind_control_hit(Unit* attacker, Unit* target) {
		const u32 func_address = 0x004F6910;
		__asm {
			PUSHAD
			MOV ESI, target
			PUSH attacker
			CALL func_address
			POPAD
		}
	}

	// Somehow related to AI stuff; details unknown.
	u16 get_unit_strength(Unit* unit, bool useGroundStrength) {
		const u32 func_address = 0x00431800;
		static u16 result;
		b32 use_ground_strength_xd = useGroundStrength;
		__asm {
			PUSHAD
			PUSH use_ground_strength_xd
			MOV EAX, unit
			CALL func_address
			MOV result, AX
			POPAD
		}
		return result;
	}

	// Hallucination-related
	Unit* create_hallucination(Unit* target, u32 player) {
		const u32 func_address = 0x004F6B90;
		static Unit* return_value;
		__asm {
			PUSHAD
			PUSH player
			MOV ECX, target
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	// For update_corrosive_spores
	u32 get_corrosive_spores_image(Unit* unit) {
		const u32 func_address = 0x004F41F0;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void insert_first_order(Unit* unit, u8 order_id) {
		const u32 func_address = 0x004749D0;
		__asm {
			PUSHAD
			MOVZX EAX, order_id
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	/// Return the count of overlays
	u8 has_overlay(Unit* unit) {
		const u32 func_address = 0x0047B720;
		static u8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void function_004934B0(Unit* unit, Unit* battery) {
		const u32 func_address = 0x004934B0;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EBX, battery
			CALL func_address
			POPAD
		}
	}

	// Identical to unitIsHangerUnit @ 0x00401450
	bool is_hangar_unit(Unit* unit) {
		if (unit->id == UnitId::ProtossSteward
		||  unit->id == UnitId::ProtossPlasmaShell) {
			return true;		
		}
		else {
			return false;
		}
	}

	Unit* acquire_medic_heal_target(Unit* medic) {
		const u32 func_address = 0x004422A0;
		static Unit* target;
		__asm {
			PUSHAD
			PUSH medic
			CALL func_address
			MOV target, EAX
			POPAD
		}
		return target;
	}

	u32 do_medic_heal(Unit* unit, Unit* target) {
		const u32 func_address = 0x00463C40;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH target
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void order_to_attack_target(Unit* unit, Unit* target, u32 process_subunit, u32 target_is_cloaked) {
		const u32 func_address = 0x00476FC0;
		__asm {
			PUSHAD
			PUSH target_is_cloaked
			PUSH process_subunit
			MOV EAX, target
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	b32 attack_recent_attacker(Unit* unit) {
		const u32 func_address = 0x004770E0;
		static b32 return_value;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return (return_value != 0);
	}

	b32 ai_order_heal(Unit* unit, Unit* target) {
		const u32 func_address = 0x00463530;
		static b32 result;
		__asm {
			PUSHAD
			MOV ESI, target
			MOV EBX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void perform_another_order_at_coords(Unit* unit, s32 x, s32 y, u8 order_id) {
		const u32 func_address = 0x004748E0;
		__asm {
			PUSHAD
			MOVZX EBX, order_id
			MOV ESI, unit
			XOR EDI, EDI
			PUSH y
			PUSH x
			CALL func_address
			POPAD
		}
	}

	void update_target_something(Unit* unit, Unit* target, u8 order_id) {
		const u32 func_address = 0x00474940;
		__asm {
			PUSHAD
			MOVZX EBX, order_id
			MOV ESI, unit
			MOV EAX, target
			XOR EDI, EDI
			CALL func_address
			POPAD
		}
	}

	b32 ai_are_on_connected_regions(Unit* unit, s32 x, s32 y) {
		const u32 func_address = 0x00438E70;
		static b32 result;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_unload_failure(Unit* unit) {
		const u32 func_address = 0x0043DF30;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	void insert_order_before(Unit* unit, Order* order, u8 order_id, s16 x, s16 y, Unit* target, u16 targetUnitId) {
		const u32 func_address = 0x00474540;
		static Point16 pos;
		pos.x = x; pos.y = y;
		__asm {
			PUSHAD
			PUSH target
			PUSH pos
			MOVZX EBX, order_id
			MOVZX EDX, targetUnitId
			MOV ESI, unit
			MOV EDI, order
			CALL func_address
			POPAD
		}
	}

	void do_order(Unit* unit, u32 order_id, s32 x, s32 y, bool stopPreviousOrders) {
		const u32 func_address = 0x00474C70;
		b32 stop_previous_orders_xd = stopPreviousOrders ? 1 : 0;
		__asm {
			PUSHAD
			PUSH stop_previous_orders_xd
			PUSH y
			PUSH x
			PUSH order_id
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	b32 ai_update_attack_target(Unit* unit, u32 unk1, u32 unk2, u32 unk3) {
		const u32 func_address = 0x00477160;
		__asm {
			PUSHAD
			PUSH unk1
			PUSH unk2
			PUSH unk3
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	Unit* get_loaded_unit_from_index(Unit* transport, s32 index) {
		const u32 func_address = 0x004E6C40;
		static Unit* return_value;
		__asm {
			PUSHAD
			MOV ECX, index
			MOV EAX, transport
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	b32 get_first_loaded_worker(Unit* transport) {
		const u32 func_address = 0x004E6D90;
		static b32 return_value;
		__asm {
			PUSHAD
			MOV EAX, transport
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	b32 ai_find_transport(Unit* unit, Unit** unitTargetOffset) {
		const u32 func_address = 0x004E6EF0;
		b32 result;
		__asm {
			PUSHAD
			PUSH unitTargetOffset
			MOV EBX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void load_unit_proc(Unit* unit, Unit* unitToLoad) {
		const u32 func_address = 0x004E78E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, unitToLoad
			CALL func_address
			POPAD
		}
	}

	b32 unload_unit_proc(Unit* unit) {
		const u32 func_address = 0x004E7F70;
		static b32 return_value;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void set_unit_retreat_point(Unit* unit, u8 angle) {
		const u32 func_address = 0x004011A0;
		__asm {
			PUSHAD
			MOVZX EAX, angle
			MOV EDX, unit
			CALL func_address
			POPAD
		}
	}

	// Return true when unit reached next_target_waypoint either at call or after some action inside the function, else return false
	// At least that's what I think it does (not sure what I called "action inside the function" is, and how it's used to return true -GPTP
	bool get_closer_to_destination(Unit* unit) {
		const u32 func_address = 0x00402BE0;
		static b32 bResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV bResult, EAX
			POPAD
		}
		return (bResult == 1);
	}

	void order_return_to_idle(Unit* unit) {
		const u32 func_address = 0x00463770;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	bool check_for_close_unit(Unit* unit) {
		const u32 func_address = 0x00464290;
		static b32 bResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV bResult, EAX
			POPAD
		}
		return (bResult == 1);
	}

	void play_subunit_iscript(Unit* unit, IscriptAnimation::Enum anim) {
		const u32 func_address = 0x00474380;
		u32 anim_xd = anim;
		assert(unit->subunit);
		__asm {
			PUSHAD
			MOV EBX, unit
			PUSH anim_xd
			CALL func_address
			POPAD
		}
	}

	// Identical to get_hp_gain_for_repair @ 0x00402C40
	u32 get_hp_gain_for_repair(Unit* unit) {
		if (*g_cheat_state & CheatFlags::OperationCwal) {
			return unit->build_repair_hp_gain * 16;
		}
		else {
			return unit->build_repair_hp_gain;
		}
	}

	void refund_unit_train_cost(u32 unit_id, u8 player) {
		const u32 func_address = 0x0042CEC0;
		__asm {
			PUSHAD
			MOV ECX, unit_id
			MOVZX EAX, player
			CALL func_address
			POPAD
		}
	}

	void clear_building_ai_queue_slot(Unit* unit, u32 build_queue_slot) {
		const u32 func_address = 0x00432430;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, build_queue_slot
			CALL func_address
			POPAD
		}
	}

	void add_hangar_unit(Unit* main_unit, Unit* added_unit) {
		const u32 func_address = 0x00466300;
		__asm {
			PUSHAD
			MOV EDI, main_unit
			MOV ESI, added_unit
			CALL func_address
			POPAD
		}
	}

	Unit* attempt_train_hatch_unit(Unit* trainer, u32 builtUnitID, u32 flag) {
		const u32 func_address = 0x00468200;
		static Unit* builtUnit;
		__asm {
			PUSHAD
			MOV ESI, trainer
			MOV EDI, builtUnitID
			PUSH flag
			CALL func_address
			MOV builtUnit, EAX
			POPAD
		}
		return builtUnit;
	}

	Bullet* create_bullet(u8 weapon_id, Unit* source, s16 x, s16 y, u8 attacking_player, u8 direction) {
		const u32 func_address = 0x0048C260;
		static u32 attacking_player_xd;
		static u32 direction_xd;
		static s32 x_xd;
		static s32 y_xd;
		attacking_player_xd = attacking_player;
		direction_xd = direction;
		x_xd = x;
		y_xd = y;

		static Bullet* result;
		__asm {
			PUSHAD
			PUSH direction_xd
			PUSH attacking_player_xd
			PUSH y_xd
			PUSH x_xd
			MOV EAX, source
			MOVZX ECX, weapon_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For progress_bullet_movement
	void progress_bullet_movement(Bullet* bullet) {
		const u32 func_address = 0x0048B250;
		__asm {
			PUSHAD
			MOV EAX, bullet
			CALL func_address
			POPAD
		}
	}

	// For bulletBehavior_Bounce
	Bullet* change_move_pos(Bullet* bullet, s32 a2, s32 a3) {
		const u32 func_address = 0x00495240;
		static Bullet* result;
		__asm {
			PUSHAD
			MOV EAX, bullet
			MOV ECX, a2
			MOV ESI, a3
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* find_next_bounce_target(Bullet* bullet) {
		const u32 func_address = 0x0048B1E0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, bullet
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_is_subunit(Unit* a1) {
		const u32 func_address = 0x00401D40;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unit_is_carrier(Unit* a1) {
		return a1->id == UnitId::ProtossSolarion;
	}

	bool unit_is_reaver(Unit* unit) {
		return unit->id == UnitId::ProtossAccantorOld;
	}

	bool unit_has_path_to_target(Unit* unit, Unit* target) {
		const u32 func_address = 0x0049CBB0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 can_be_infested(Unit* unit, Unit* target) {
		const u32 func_address = 0x00402210;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EDX, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	//for progress_bullet
	void do_missile_damage(Bullet* bullet) {
		const u32 func_address = 0x0048B770;
		__asm {
			PUSHAD
			PUSH bullet
			CALL func_address
			POPAD
		}
	}

	bool can_unit_id_fit_at_xy(s32 x, s32 y, u32 unit_id) {
		const u32 func_address = 0x0042D810;
		static b32 bPreReturnValue;
		__asm {
			PUSHAD
			MOV EAX, x
			MOV EDI, y
			MOV EBX, unit_id
			CALL func_address
			MOV bPreReturnValue, EAX
			POPAD
		}
		return (bPreReturnValue != 0);
	}

	Unit** get_all_units_in_bounds(Box16* coords) {
		const u32 func_address = 0x0042FF80;
		static Unit** units_in_bounds;
		__asm {
			PUSHAD
			MOV EAX, coords
			CALL func_address
			MOV units_in_bounds, EAX
			POPAD
		}
		return units_in_bounds;
	}

	u32 get_unit_damage_bonus(Unit* unit, u8 weapon_id) {
		const u32 func_address = 0x00475EC0;
		static u32 damageBonus;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOVZX EDX, weapon_id
			CALL func_address
			MOV damageBonus, EAX
			POPAD
		}
		return damageBonus;
	}

	// identical to increment_unit_kill_count @ 0x004759C0
	void increment_unit_kill_count(Unit* unit) {
		Unit* temp_unit = unit;
		if (temp_unit->kill_count < 255) {
			temp_unit->kill_count++;
		}
		while (temp_unit->id == UnitId::ProtossSteward && temp_unit->hangar_unit.parent) {
			temp_unit = temp_unit->hangar_unit.parent;
			if (temp_unit->kill_count < 255) {
				temp_unit->kill_count++;
			}
		}
	}

	void attack_overlay_and_notify(Unit* attacker, Unit* target, u32 weaponType, u32 direction) {
		const u32 func_address = 0x00479730;
		__asm {
			PUSHAD
			MOV ESI, attacker
			PUSH direction
			MOV EAX, target
			PUSH weaponType
			CALL func_address
			POPAD
		}
	}

	u32 get_unit_bullet_damage(Bullet* bullet, Unit* target) {
		const u32 func_address = 0x0048ACD0;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV EAX, target
			MOV EDX, bullet
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	u32 get_spell_stat_string(Unit* target, u32 techId, u8 player) {
		const u32 func_address = 0x00491E80;
		static u32 return_value;
		__asm {
			PUSHAD
			MOVZX EBX, player
			PUSH techId
			MOV EAX, target
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void disable_unit(Unit* unit) {
		const u32 func_address = 0x00492CC0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	// Identical to set_all_image_group_flags_palette_11 @ 0x00497430;
	void set_all_image_group_flags_palette_11(Sprite* sprite) {
		for (Image* current_image = sprite->images.head;
			current_image != nullptr;
			current_image = current_image->link.next) {
			if (current_image->palette_type == PaletteType::RLE_HPFLOATDRAW) {
				current_image->flags |= ImageFlags::Redraw;
			}
		}
	}
	
	Unit* iterate_units_at_location_target_proc(Box16* a1, Unit*(__fastcall* a2)(Unit*, Unit*), Unit* a3) {
		const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EBX, a2
			MOV EAX, a1
			CALL Func_IterateUnitsAtLocationTargetProc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// hardcoding the larva count function, keeping the return value though it's unused here
	u32 iterate_units_at_location_target_proc_larva_count(Unit* unit, Box16* coords) {
		const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
		static u32 return_value;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, coords
			MOV EBX, 0x004E8C80
			CALL Func_IterateUnitsAtLocationTargetProc
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	// hardcodes the recall targets selection function
	u32 iterate_units_at_location_target_proc_recall_targets(Unit* unit, Box16* coords) {
		const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
		static u32 return_value;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, coords
			MOV EBX, 0x00494400
			CALL Func_IterateUnitsAtLocationTargetProc
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void IterateUnitsAtLocationTargetProc_CorrosiveAcid(u32 attacking_player_id, Box16* coords) {
		const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
		const u32 func_address = 0x004F4670;
		__asm {
			PUSHAD
			PUSH attacking_player_id
			MOV EAX, coords
			MOV EBX, func_address
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void iterate_units_at_location_target_proc_ensnare(Unit* attacker, Box16* coords) {
		const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
		const u32 func_address = 0x004F46C0;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, func_address
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void iterate_units_at_location_target_proc_plague(Unit* attacker, Box16* coords) {
		const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
		const u32 func_address = 0x004F4AF0;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, func_address
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void plague_proc(Unit* unit, Unit* attacker) {
		const u32 func_address = 0x004F4AF0;
		__asm {
			PUSHAD
			MOV EDX, attacker
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	void iterate_units_at_location_target_proc_maelstrom(Unit* attacker, Box16* coords) {
		const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
		const u32 func_address = 0x004F6760;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, func_address
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void iterate_units_at_location_target_proc_stasis_field(Unit* attacker, Box16* coords) {
		const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
		const u32 func_address = 0x004F68D0;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, func_address
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}	

	void apply_ensnare(Unit* unit) {
		const u32 func_address = 0x004F45E0;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	// For create_scanner
	s32 play_sound_from_unit(s32 sound_id, Unit* unit, s32 play, s32 min_volume) {
		const u32 func_address = 0x0048ED50;
		static s32 result;
		__asm {
			PUSHAD
			PUSH min_volume
			PUSH play
			MOV ESI, unit
			MOV EBX, sound_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// 0x00401170
	bool unit_is_owned_by_current_player(Unit* unit) {
		bool return_value;
		if (*g_is_in_replay != 0 || unit->player != *g_local_nation_id) {
			return_value = false;
		} else {
			return_value = true;
		}
		return return_value;
	}

	// 0x00402A70
	bool is_unit_burrowed(Unit* unit) {
		return (unit->status & (UnitStatus::CloakingForFree + UnitStatus::Burrowed)) != 0;
	}

	void sc_memcpy_0(u32 dest, u32 src, u32 memsize) {
		const u32 func_address = 0x00408FD0;
		__asm {
			PUSHAD
			PUSH memsize
			PUSH src
			PUSH dest
			CALL func_address
			ADD ESP, 0x0C
			POPAD
		}
	}

	void show_rally_target(Unit* unit) {
		const u32 func_address = 0x00468670;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	bool unit_is_unselectable(u16 unit_id) {
		const u32 func_address = 0x0046ED80;
		b32 bHalfReturnValue;
		__asm {
			PUSHAD
			XOR EBX, EBX
			MOVZX EBX, unit_id
			MOV EAX, EBX
			CALL func_address
			MOV bHalfReturnValue, EAX
			POPAD
		}
		return (bHalfReturnValue != 0);
	}

	void function_0046F040(Unit* current_unit, Unit** unit_list, Unit* clicked_unit, u32 list_length) {
		const u32 func_address = 0x0046F040;
		__asm {
			PUSHAD
			PUSH clicked_unit
			PUSH current_unit
			MOV ECX, unit_list
			MOV EAX, list_length
			CALL func_address
			POPAD
		}
	}

	u32 combine_lists(Unit* unit, Unit** unit_list_1, Unit** unit_list_2, u32 list_length) {
		const u32 func_address = 0x0046F290;
		u32 return_value;
		__asm {
			PUSHAD
			PUSH unit
			PUSH list_length
			PUSH unit_list_1
			MOV EDI, unit_list_2
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void select_units_if_alt_not_held(Unit** unit_list, u32 unit_list_length, char play_sound, u32 send_command) {
		const u32 func_address = 0x0046FA00;
		static u32 play_sound_xd = play_sound;
		__asm {
			PUSHAD
			PUSH send_command
			PUSH play_sound_xd
			MOV EBX, unit_list
			MOV EDI, unit_list_length
			CALL func_address
			POPAD
		}
	}

	u32 unit_to_unit_id(Unit* unit) {
		const u32 func_address = 0x0047B1D0;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	bool unit_is_standard_and_movable(Unit* unit) {
		const u32 func_address = 0x0047B770;
		b32 return_value_unconverted;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV return_value_unconverted, EAX
			POPAD
		}
		return (return_value_unconverted != 0);
	}

	void select_building_sfx(Unit* unit) {
		const u32 func_address = 0x0048F910;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	void create_new_unit_selections_from_list(Unit** unit_list, u32 unit_list_length) {
		const u32 func_address = 0x0049AE40;
		__asm {
			PUSHAD
			PUSH unit_list_length
			MOV EAX, unit_list
			CALL func_address
			POPAD
		}
	}

	void cmdact_select(Unit** unit_list, u32 unit_list_length) {
		const u32 func_address = 0x004C0860;
		__asm {
			PUSHAD
			PUSH unit_list
			PUSH unit_list_length
			CALL func_address
			POPAD
		}
	}

	// Equivalent to secondaryOrd_Cloak @ 0x00491790
	void secondary_order_cloak(Unit* unit) {
		unit->is_cloaked++;
		if (unit->is_cloaked == 1 && !(unit->status & UnitStatus::RequiresDetection) && !unit->invisible_link.next)
		{
			unit->invisible_link.next = *g_first_invisible_unit;
			unit->invisible_link.prev = 0;
			if (*g_first_invisible_unit) {
				(*g_first_invisible_unit)->invisible_link.prev = unit;
			}
			*g_first_invisible_unit = unit;
			bw::refresh_console();
		}
	}

	void removeOverlays2(u32 from, u32 to, Unit* unit) {
		const u32 func_address = 0x00492370;
		__asm {
			PUSHAD
			MOV EAX, from
			MOV EBX, unit
			PUSH to
			CALL func_address
			POPAD
		}
	}

	bool unit_hp_is_in_redzone(Unit* unit) {
		const u32 func_address = 0x004022C0;
		static b32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	//hooked by hooks\update_status_effects.cpp
	void restore_all_unit_stats(Unit* unit) {
		const u32 func_address = 0x00492F70;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	// For update_units

	bool is_selected_unit_group_enabled() {
		const u32 func_address = 0x0049A2C0;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void cancel_target_order() {
		const u32 func_address = 0x0048CA10;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	s32 update_disruption_web_status() {
		const u32 func_address = 0x004EB2F0;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* progress_dying_unit_frame(Unit* unit) {
		const u32 func_address = 0x004EB5E0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void reveal_sight_at_location(s32 sight_range, u32 vision_flags, s32 pt_x, s32 pt_y, bool is_air) {
		const u32 func_address = 0x004806F0;
		b32 is_air_unit_xd = is_air;
		__asm {
			PUSHAD
			PUSH is_air_unit_xd
			PUSH pt_y
			PUSH pt_x
			MOV ECX, vision_flags
			MOV EAX, sight_range
			CALL func_address
			POPAD
		}
	}

	void update_visibility(Unit* unit) {
		const u32 func_address = 0x004EBE10;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	void progress_active_unit_frame(Unit* unit) {
		const u32 func_address = 0x004EBC30;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	void check_unit_visibility(Unit* unit) {
		const u32 func_address = 0x00443390;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void progress_unit_frame(Unit* unit) {
		const u32 func_address = 0x004ECF70;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	void unit_update(Unit* unit) {
		const u32 func_address = 0x004EC420;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	void update_unit_power() {
		const u32 func_address = 0x00493790;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	// For update_burrowing_cloaking_units
	s32 is_unit_cloaking(Unit* unit) {
		const u32 func_address = 0x004916E0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For do_cloak
	void update_sprite_flag_on_palette(Sprite* sprite) {
		const u32 Func_updateSpriteFlagOnPalette = 0x00497E80;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_updateSpriteFlagOnPalette
			POPAD
		}
	}

	void show_images_hidden_by_cloak(Sprite* sprite) {
		const u32 Func_showImagesHiddenByCloak = 0x00498170;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_showImagesHiddenByCloak
			POPAD
		}
	}

	void update_sprite_images_for_cloak(Sprite* sprite) {
		const u32 Func_updateSpriteImagesForCloak = 0x00498A90;
		__asm {
			PUSHAD
			MOV EDI, sprite
			CALL Func_updateSpriteImagesForCloak
			POPAD
		}
	}

	u32 getLarvaeUnitsFromList(Unit** unitList, Unit** outputList) {
		const u32 Func_getLarvaeUnitsFromList = 0x00423190;
		static u32 larvaeList_Count;
		__asm {
			PUSHAD
			MOV ESI, outputList
			MOV EDX, unitList
			CALL Func_getLarvaeUnitsFromList
			MOV larvaeList_Count, EAX
			POPAD
		}
		return larvaeList_Count;
	}

	s32 bwfxn_print_xy(const char* text, s32 x, s32 y) {
		const u32 BWFXN_PrintXY_function = 0x004202B0;
		static s32 result;
		_asm {
			PUSHAD
			PUSH y
			MOV EAX, text
			MOV ESI, x
			CALL BWFXN_PrintXY_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void get_color_handle(u8 color) {
		const u32 func_address = 0x0041F610;
		_asm {
			PUSHAD
			MOV AL, color
			CALL func_address
			POPAD
		}
	}

	void set_font_handle(u32* font) {
		const u32 func_address = 0x0041FB30;

		__asm {
			PUSHAD
			MOV ECX, font
			CALL func_address
			POPAD
		}
	}

	// For targeted_order
	Order* queue_new_order(Order* a1, s16 a2, s16 a3, Unit* a4, u8 a5, s32 a6, s16 a7, Vector32* a8, Unit* a9) {
		const u32 func_address = 0x00474B90;
		s32 a7_xd = a7;
		u32 a5_xd = a5;
		static Order* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX EDX, a2
			MOVZX ECX, a3
			PUSH a9
			PUSH a8
			PUSH a7_xd
			PUSH a6
			PUSH a5_xd
			PUSH a4
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 show_stat_txt_to_player(s32 a1, s32 a2) {
		const u32 func_address = 0x0048CF00;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void set_order_target(u8 a1, s16 a2, s16 a3, s32 a4, s32 a5, Unit* a6, s32 a7, Vector32* a8, Unit* a9) {
		const u32 func_address = 0x00475470;
		__asm {
			PUSHAD
			MOVZX ECX, a1
			MOVZX EDX, a2
			MOVZX EAX, a3
			MOV EDI, a4
			PUSH a9
			PUSH a8
			PUSH a7
			PUSH a6
			PUSH a5
			CALL func_address
			POPAD
		}
	}

	// For select_units

	s32 select_single_unit_from_id(s32 index) {
		const u32 func_address = 0x00496D30;
		static s32 result;
		__asm {
			PUSHAD
			PUSH index
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 select_multiple_units_from_unit_list(u32 unit_count, Unit** selection, char play_sound, u32 send_command) {
		const u32 func_address = 0x0049AEF0;
		static s32 result;
		static u32 play_sound_xd = play_sound;
		__asm {
			PUSHAD
			PUSH send_command
			PUSH play_sound_xd
			PUSH selection
			PUSH unit_count
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 snet_get_turns_in_transit(u32* turns) {
		const u32 func_address = 0x00410274;
		static s32 result;
		_asm {
			PUSHAD
			PUSH turns
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8 send_turn() {
		const u32 func_address = 0x00485A40;
		static u32 result;
		_asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 serr_get_last_error() {
		const u32 func_address = 0x004100FA;
		static s32 result;
		_asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8 packet_error_handle(s32 result_arg, s32 a1, const char* a3, s32 a4, s32 a5) {
		const u32 func_address = 0x004BB0B0;
		static u32 result;
		_asm {
			PUSHAD
			MOV EAX, result_arg
			PUSH a5
			PUSH a4
			PUSH a3
			PUSH a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// unkRepairInfo may be related to unit->worker.repair_resource_loss_timer
	void get_repair_info(Unit* repairTarget, u32* repairCostMineral, u32* repairCostGas, s16* unkRepairInfo) {
		const u32 func_address = 0x00466D10;
		__asm {
			PUSHAD
			MOV EAX, repairTarget
			PUSH unkRepairInfo
			PUSH repairCostGas
			PUSH repairCostMineral
			CALL func_address
			POPAD
		}
	}

	bool subtract_repair_resources(Unit* worker, Unit* target) {
		const u32 func_address = 0x00466FB0;
		static b32 result;
		__asm {
			PUSHAD
			MOV ESI, worker
			PUSH target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	// display an error message and return false if not enough resources
	bool verify_resources(u32 mineralCost, u32 gasCost, u32 player) {
		const u32 func_address = 0x00466EE0;
		b32 result;
		__asm {
			PUSHAD
			MOV EAX, mineralCost
			PUSH gasCost
			MOV ECX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	u8 getHangerTrainCount(Unit* unit) {
		const u32 func_address = 0x00465270;
		static u8 result = 0;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool unit_is_training_or_morphing(Unit* unit) {
		const u32 func_address = 0x00401500;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8 initialize_randomizer_info() {
		const u32 func_address = 0x0047E410;
		static u32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (u8)result;
	}

	void ai_loop() {
		const u32 func_address = 0x004A2B00;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	s32 update_active_tile_info() {
		const u32 func_address = 0x0049C4C0;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 remove_fow_cheat() {
		const u32 func_address = 0x00480880;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void update_units() {
		const u32 func_address = 0x004ED000;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	s32 image_drawing_bullet_drawing() {
		const u32 func_address = 0x0048C3D0;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void update_images() {
		const u32 func_address = 0x00496330;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void update_thingies() {
		const u32 func_address = 0x00488510;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	// AKA createPsiFieldSpriteForUnit()
	// Returns true if successful, false otherwise.
	bool add_psi_field_sprite(Unit* unit) {
		if (!unit) {
			return false;
		}
		const u32 func_address = 0x00493BF0;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	void order_interrupt(Unit* unit, Unit* target, u16 x, u16 y, u32 order_id) {
		const u32 func_address = 0x00474C30;
		static Point16 pos;
		pos.x = x; pos.y = y;
		__asm {
			PUSHAD
			MOV ESI, unit
			PUSH target
			PUSH pos
			PUSH order_id
			CALL func_address
			POPAD
		}
	}

	void unit_order_move_to_target_unit(Unit* unit, Unit* target) {
		const u32 func_address = 0x00479FE0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, target
			CALL func_address
			POPAD
		}
	}

	void queue_game_command(u8* params, u32 param_length) {
		const u32 func_address = 0x00485BD0;
		__asm {
			PUSHAD
			MOV EDX, param_length
			MOV ECX, params
			CALL func_address
			POPAD
		}
	}

	bool do_units_collide(Unit* first, Unit* second) {
		const u32 func_address = 0x00469B60;
		static u32 result = 0;
		__asm {
			PUSHAD
			MOV EAX, first
			MOV ECX, second
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void unburrow(Unit* unit) {
		const u32 func_address = 0x004E97C0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void* SMemAlloc(u32 amount) {
		const u32 func_address = 0x0041006A;
		const char* unkFilename  = "";
		u32   unkLine            = 0;
		u32   defaultValue       = 0;
		static void* pointer_on_allocated_memory;
		__asm {
			PUSHAD
			PUSH defaultValue		// probably what to fill the memory with
			PUSH unkLine			// unknown use, seems to use hardcoded value
			PUSH unkFilename		// unknown use, seems to use hardcoded adress (of some text?)
			PUSH amount				// amount of memory to allocate
			CALL func_address
			MOV pointer_on_allocated_memory, EAX
			POPAD
		}
		return pointer_on_allocated_memory;
	}

	void* SMemAlloc(u32 amount, const char* unkFilename, u32 unkLine, u32 defaultValue) {
		const u32 func_address = 0x0041006A;
		static void* pointer_on_allocated_memory;
		__asm {
			PUSHAD
			PUSH defaultValue		// probably what to fill the memory with
			PUSH unkLine			// unknown use, seems to use hardcoded value
			PUSH unkFilename		// unknown use, seems to use hardcoded adress (of some text?)
			PUSH amount				// amount of memory to allocate
			CALL func_address
			MOV pointer_on_allocated_memory, EAX
			POPAD
		}
		return pointer_on_allocated_memory;
	}

	// Likely to call SMemAlloc!
	void set_callback_timer(BinDlg* dialog, u16 timer_index, u32 time, u32 callback) {
		const u32 func_address = 0x00416090;
		__asm {
			PUSHAD
			PUSH callback
			PUSH time
			MOV AX, timer_index
			MOV ECX, dialog
			CALL func_address
			POPAD
		}
	}

	void remove_from_selections(Unit* unit) {
		const u32 func_address = 0x0049A7F0;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	u32 fxn_interact(BinDlg* dialog, u32 data_struct_offset) {
		static u32 return_value;
		static u32 fxn_interact;
		fxn_interact = (u32)dialog->fxn_interact;
		__asm {
			PUSHAD
			MOV ESI, dialog // possibly useless, but just in case
			MOV ECX, dialog
			MOV EDX, data_struct_offset
			CALL fxn_interact
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void register_user_dialog_action(BinDlg* dialog, u32 array_of_fxnInteractFunc, u32 sizeOfArray) {
		const u32 func_address = 0x00418100;
		__asm {
			PUSHAD
			MOV EAX, dialog
			MOV EDI, array_of_fxnInteractFunc
			MOV ECX, sizeOfArray
			CALL func_address
			POPAD
		}
	}

	void enable_control(BinDlg* dialog) {
		const u32 func_address = 0x00418E00;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL func_address
			POPAD
		}
	}

	void replay_stat_buttons(BinDlg* dialog) {
		const u32 func_address = 0x00427A80;
		__asm {
			PUSHAD
			MOV EBX, dialog
			CALL func_address
			POPAD
		}
	}

	void update_command_card() {
		const u32 func_address = 0x004591D0;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void refresh_button_tooltip() {
		const u32 func_address = 0x00459770;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void bindlg_blit_surface(BinDlg* dialog) {
		const u32 func_address = 0x004C35F0;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL func_address
			POPAD
		}
	}

	// dev info: examples of calls: 429520,428e60,428420,4287D0,4292C0
	s32 req_check(u32 requirement_func, u16 requirement_var, u32 player, Unit* unit) {
		static s32 return_value;
		__asm {
			PUSHAD
			MOVZX ECX, requirement_var
			MOV EDX, player
			PUSH unit
			CALL requirement_func
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void update_mineral_patch_image(Unit* unit) {
		IscriptAnimation::Enum anim = IscriptAnimation::Built;
		if (unit->building.resource.resource_amount == 0) {
			anim = IscriptAnimation::Landing;
		} else if (unit->building.resource.resource_amount < 250) {
			anim = IscriptAnimation::SpecialState1;
		} else if (unit->building.resource.resource_amount < 500) {
			anim = IscriptAnimation::SpecialState2;
		} else if (unit->building.resource.resource_amount < 750) {
			anim = IscriptAnimation::AlmostBuilt;
		}

		if (unit->building.resource.resource_iscript != anim) {
			unit->building.resource.resource_iscript = anim;
			unit->sprite->play_iscript_animation(anim);
		}
	}

	void update_image_position_offset(Image* image) {
		const u32 func_address = 0x004D5A00;
		__asm {
			PUSHAD
			MOV ECX, image
			CALL func_address
			POPAD
		}

	}

	// Identical to function @ 0x004F3AF0
	void set_resource_amount_carried(Unit* worker, u8 amount_carried, u32 chunk_image_id, bool is_mineral) {
		if (worker->resource_type != 0) {
			return;
		}
		worker->resource_type = is_mineral ? 2 : 1;
		Image* chunk_image = worker->sprite->create_overlay(chunk_image_id);
		if (chunk_image && !(chunk_image->flags & ImageFlags::UsesSpecialOffset)) {
			chunk_image->flags |= ImageFlags::UsesSpecialOffset;
			update_image_position_offset(chunk_image);
		}
		worker->worker.resource_carry_amount = amount_carried;
		bw::refresh_console();
	}

	s32 ai_has_buildings_needing_power(u32 player) {
		const u32 func_address = 0x00433410;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_count_economy_units(u32 player, u32 unit_id) {
		const u32 func_address = 0x00433470;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			PUSH unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_supplies_remaining(u8 player, u16 unit_id) {
		const u32 func_address = 0x004889D0;
		static s32 result;
		__asm {
			PUSHAD
			MOVZX EBX, player
			MOV DI, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_supplies_used(s32 byte_data, u8 player) {
		const u32 func_address = 0x004888C0;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, byte_data
			MOVZX ECX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_add_spending_request
	bool ai_isnt_at_define_max(s32 player, s32 unit_id) {
		const u32 func_address = 0x00447830;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, unit_id
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void delete_building_ai(BuildingAi* building, BuildingAiList* buildingList) {
		const u32 func_address = 0x00404500;
		__asm {
			PUSHAD
			MOV EDX, buildingList
			MOV EAX, building
			CALL func_address
			POPAD
		}
	}

	void delete_worker_ai(WorkerAi* ai, WorkerAiList* workerList) {
		const u32 func_address = 0x00404470;
		__asm {
			PUSHAD
			MOV EDX, workerList
			MOV EAX, ai
			CALL func_address
			POPAD
		}
	}

	void delete_ai_town_if_needed(AiTown* town) {
		const u32 func_address = 0x004347E0;
		__asm {
			PUSHAD
			MOV EAX, town
			CALL func_address
			POPAD
		}
	}

	void remove_from_attack_force(u32 unit_id, u32 player) {
		const u32 func_address = 0x00447040;
		__asm {
			PUSHAD
			MOV EDX, player
			MOV EAX, unit_id
			CALL func_address
			POPAD
		}
	}

	bool ai_has_worker_or_building(u32 region_id, u32 player) {
		const u32 func_address = 0x00432AD0;
		static b32 result;
		__asm {
			PUSHAD
			MOV EDI, player
			MOV EAX, region_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For is_order_valid
	bool unit_is_resource(Unit* unit) {
		const u32 func_address = 0x004688E0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void return_all_interceptors(Unit* unit) {
		const u32 func_address = 0x004655F0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void orders_self_destructing(Unit* unit) {
		const u32 func_address = 0x00475710;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	char apply_maelstrom(Unit* unit, u8 time) {
		const u32 func_address = 0x004553F0;
		static s32 result;
		u32 time_xd = time;
		__asm {
			PUSHAD
			PUSH time_xd
			MOV EDI, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	// For bullet_damage
	s32 _fastcall lakizilisk_splash_proc(Unit* target, Bullet* bullet) {
		const u32 func_address = 0x0048B680;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, bullet
			MOV ECX, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 _fastcall ground_splash_proc(Unit* target, Bullet* bullet) {
		const u32 func_address = 0x0048B750;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, bullet
			MOV ECX, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 _fastcall air_splash_proc(Unit* unit, Bullet* bullet) {
		const u32 func_address = 0x0048B150;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, result
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 _fastcall air_splash_proc_2(Unit* unit, Bullet* bullet) {
		const u32 func_address = 0x0048B730;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, result
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_has_no_main_order_type(Unit* unit) {
		const u32 func_address = 0x00475A50;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 set_unit_event_complete() {
		const u32 func_address = 0x004C36C0;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For use_tech
	const void issue_tri_target_order(u8 order1, u8 order2, u8 order3) {
		const u32 func_address = 0x0048CAF0;
		if (order2 == 0) {
			_asm {
				PUSHAD
				MOVZX EDX, order1
				MOVZX ECX, DL
				MOVZX EAX, DL
				CALL func_address
				POPAD
			}
		} else {
			if (order3 == 0) {
				_asm {
					PUSHAD
					MOVZX ECX, order1
					MOVZX EDX, order2
					MOVZX EAX, CL
					CALL func_address
					POPAD
				}
			} else {
				_asm {
					PUSHAD
					MOVZX EDX, order1
					MOVZX ECX, order2
					MOVZX EAX, order3
					CALL func_address
					POPAD
				}
			}
		}
	}

	s32 check_energy_for_tech_use(s32 tech_id) {
		const s32 func_address = 0x00423660;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, tech_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_order_unit_cloaking
	void apply_cloaking_order(Unit* unit) {
		const u32 func_address = 0x00491B30;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	// For find_best_unit
	Unit** find_all_units(Box16* coords) {
		const u32 func_address = 0x004308A0;
		static Unit** result;
		__asm {
			PUSHAD
			PUSH coords
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For draw_building_info
	void draw_supply_unit_info(BinDlg* dialog) {
		const u32 func_address = 0x00427540;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	// For get_unit_name
	const char* get_unit_name_stat_string(u16 unit_id) {
		const u32 func_address = 0x0047B090;
		static const char* result;
		__asm {
			PUSHAD
			MOVZX CX, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For update_building_placement_state_units
	bool does_building_block(Unit* unit, s32 xTile, s32 yTile) {
		const u32 func_address = 0x00473410;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, yTile
			MOV EDX, xTile
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 is_tile_blocked_by(Unit** array, Unit* builder, s32 xTile, s32 yTile, char bDontIgnoreReacting, char bAlsoInvisible) {
		const u32 func_address = 0x00473300;
		static s32 result;
		b32 dont_ignore_reacting_xd = bDontIgnoreReacting;
		b32 also_invisible_xd = bAlsoInvisible;
		__asm {
			PUSHAD
			PUSH also_invisible_xd
			PUSH dont_ignore_reacting_xd
			PUSH yTile
			PUSH xTile
			PUSH builder
			MOV EAX, array
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For irradiateDamageLoop
	Unit* iterateLoadedUnits(Unit* unit, Unit* (__fastcall* func)(Unit*, Unit*), s32 a3) {
		const u32 func_address = 0x004E6D00;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH a3
			PUSH func
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For move_to_target
	s32 is_point_at_unit_border(s32 unused, Unit* other, Unit* self, Point16 position_xy) {
		const u32 func_address = 0x0042DA90;
		static s32 result;
		__asm {
			PUSHAD
			PUSH position_xy
			PUSH self
			MOV EDX, other
			MOV ECX, unused
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// Returns 1 if unfindable
	s32 ai_prepare_moving_to(Unit* unit, s32 x, s32 y) {
		const u32 func_address = 0x004E7420;
		static s32 result;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			PUSH unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* create_unit_at_pos(u16 unit_id, u16 player, u32 x, u32 y) {
		const u32 func_address = 0x004CD360; // AKA createUnitXY()
		if (unit_id == UnitId::None || unit_id >= UnitId::Count) {
			return nullptr;
		}
		static Unit* unit;
		__asm {
			PUSHAD
			MOVZX ECX, unit_id
			MOVZX EAX, player
			PUSH y
			PUSH x
			CALL func_address
			MOV unit, EAX
			POPAD
		}
		return unit;
	}

	// more from api.cpp
	Flingy* create_flingy(u32 flingy_id, u32 player, u32 x, u32 y) {
		const u32 func_address = 0x00496460;
		static Flingy* flingy;
		__asm {
			PUSHAD
			MOV EAX, player
			MOV ECX, x
			MOV EDX, y
			PUSH flingy_id
			CALL func_address
			MOV flingy, EAX
			POPAD
		}
		return flingy;
	}

	// For spell_order
	bool can_target_spell(u16 a1, Unit* a2, s16 a3, Unit* a4, s16 a5, s16 a6, s32* a7) {
		const u32 func_address = 0x00492020;
		static s32 result;
		s32 a5_xd = a5;
		s32 a6_xd = a6;
		__asm {
			PUSHAD
			PUSH a7
			PUSH a6_xd
			PUSH a5_xd
			PUSH a4
			MOVZX SI, a3
			MOV EDI, a2
			MOVZX DX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 spell_order_move_towards_target(Unit* caster, u32 range) {
		const u32 func_address = 0x00492450;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, range
			MOV ECX, caster
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s16 can_target_spell_on_fow_unit(s32 techId, s16 unit_id) {
		const u32 func_address = 0x004914B0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH unit_id
			MOV EAX, techId
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (s16)result;
	}

	bool should_stay_at_range(Unit* unit) {
		const u32 func_address = 0x00475A10;
		static bool result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool is_unit_target_out_of_max_range(Unit* unit, Unit* target) {
		const u32 func_address = 0x00476430;
		static bool result;
		__asm {
			PUSHAD
			PUSH target
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool is_unreachable(Unit* target, Unit* unit) {
		const u32 func_address = 0x00476C50;
		static bool result;
		__asm {
			PUSHAD
			MOV EAX, target
			MOV ESI, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void move_for_melee_range(Unit* target, Unit* unit) {
		const u32 func_address = 0x00477F10;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, target
			CALL func_address
			POPAD
		}
	}

	// For create_unit
	Unit* unit_constructor(s32 x, s32 y, u16 unit_id, s32 player, s32 unused) {
		const u32 func_address = 0x004A06C0;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH unused
			PUSH player
			MOV DI, unit_id
			MOV EDX, y
			MOV ESI, x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* list_push_back_used_unit_entry(Unit* unit) {
		const u32 func_address = 0x0049DCA0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* list_push_back_hidden_unit_entry(Unit* unit) {
		const u32 func_address = 0x0049DCE0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Sprite* unit_destructor(Unit* unit) {
		const u32 func_address = 0x004A0990;
		static Sprite* result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void* get_image_attack_frame(void* result_arg, Sprite* sprite, s32 index, u32 overlay_type) {
		const u32 func_address = 0x00401DF0;
		static void* result;
		__asm {
			PUSHAD
			PUSH overlay_type
			MOV EDI, index
			MOV ECX, sprite
			MOV EAX, result_arg
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void set_sprite_main_image_offset(Sprite* sprite, char x, char y) {
		const u32 func_address = 0x00401E40;
		u32 x_xd = x;
		u32 y_xd = y;
		__asm {
			PUSHAD
			PUSH y_xd
			PUSH x_xd
			MOV EAX, sprite
			CALL func_address
			POPAD
		}
	}

	// For ai_training_worker
	char add_worker_structure_ai(Unit* worker, AiTown* town) {
		const u32 func_address = 0x00433DD0;
		static char result;
		__asm {
			PUSHAD
			MOV EDI, town
			MOV EBX, worker
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For create_bullet
	s32 initialize_bullet(char player, Unit* parent, s16 y, Bullet* bullet, s32 weapon_id, s32 x, s32 angle) {
		const u32 func_address = 0x0048BEC0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH angle
			PUSH x
			PUSH weapon_id
			PUSH bullet
			MOVZX DX, y
			MOV EAX, parent
			MOVZX CL, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For progress_unit_frame
	char refresh_sprite(Sprite* sprite, s32 spriteId) {
		const u32 func_address = 0x004EBBD0;
		static char result;
		__asm {
			PUSHAD
			PUSH spriteId
			MOV ESI, sprite
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void update_unit_state(Unit* unit) {
		const u32 func_address = 0x004EC290;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void progress_order(Unit* unit) {
		const u32 func_address = 0x004EC4D0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void perform_secondary_orders(Unit* result, s32 unused) {
		const u32 func_address = 0x004EC170;
		__asm {
			PUSHAD
			MOV ECX, unused
			MOV EAX, result
			CALL func_address
			POPAD
		}
	}

	void sprite_to_iscript_loop(Sprite* sprite) {
		const u32 func_address = 0x00497920;
		__asm {
			PUSHAD
			MOV ESI, sprite
			CALL func_address
			POPAD
		}
	}

	s16 create_tooltip(char* text) {
		const u32 func_address = 0x004591A6;
		static s16 result;
		__asm {
			PUSHAD
			PUSH text
			CALL func_address
			MOV result, AX
			POPAD
		}
		return result;
	}

	char refresh_selection_screen() {
		const u32 func_address = 0x00481480;
		static char result;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	// For ai_send_suicide
	bool is_threat(Unit* target, Unit* unit) {
		const u32 func_address = 0x00476930;
		static bool result;
		__asm {
			PUSHAD
			MOV EAX, target
			MOV ECX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void prepare_suicide(Unit* unit) {
		const u32 func_address = 0x0043CD40;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	u32 ai_attack_clear(s32 player) {
		const u32 func_address = 0x00439500;
		static u32 result;
		__asm {
			PUSHAD
			PUSH player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool ai_attack_manager(s32 player, s32 x, s32 y, s32 bOverrideLastAttack, s32 bFallbackAirOnly) {
		const u32 func_address = 0x0043ABB0;
		static bool result;
		__asm {
			PUSHAD
			PUSH bFallbackAirOnly
			PUSH bOverrideLastAttack
			PUSH y
			PUSH x
			PUSH player
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For orders_pickup_4
	void issue_order_targeting_ground(Unit* unit, u32 order_id, u32 x, u32 y) {
		const u32 func_address = 0x00475260;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			PUSH order_id
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	// For init_game
	void switch_all_available_computer_slots_to_open() {
		const u32 func_address = 0x004DBBE0;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	bool set_last_open_slot_to_computer(s8 race) {
		const u32 func_address = 0x004DBB70;
		static s8 result;
		s32 race_xd = race;
		__asm {
			PUSHAD
			PUSH race_xd
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For start_game
	s32 boot_reason(u8 player) {
		const u32 func_address = 0x004B8870;
		static s32 result;
		__asm {
			PUSHAD
			MOVZX AL, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 is_authorized_map(void* a1) {
		const u32 func_address = 0x004C4870;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void set_timer_func() {
		const u32 func_address = 0x00472E00;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	s32 get_tick_count() {
		static u32 func_address = *(u32*)0x004FE0C4;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For initialize_bullet
	Flingy* initialize_flingy(s32 flingy_id, s16 y, s32 x, Flingy* flingy, char player, char angle) {
		const u32 func_address = 0x00496360;
		static Flingy* result;
		s32 player_xd = player;
		s32 angle_xd = angle;
		__asm {
			PUSHAD
			PUSH angle_xd
			PUSH player_xd
			PUSH flingy
			MOV ECX, x
			MOVZX EDX, y
			MOV EAX, flingy_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void bullet_behavior_reassign(Bullet* bullet) {
		const u32 func_address = 0x0048BD50;
		__asm {
			PUSHAD
			MOV ECX, bullet
			CALL func_address
			POPAD
		}
	}

	s32 weapon_behavior_can_hit_target_check(Unit* target, Unit* attacker) {
		const u32 func_address = 0x004765B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, attacker
			MOV EAX, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void appear_on_target(Bullet* bullet, s16 x, s16 y) {
		const u32 func_address = 0x004955C0;
		__asm {
			PUSHAD
			MOVZX ECX, y
			MOVZX EDX, x
			MOV EAX, bullet
			CALL func_address
			POPAD
		}
	}

	Bullet* attack_fly_to_target(Bullet* bullet, s32 x, s32 y) {
		const u32 func_address = 0x00494B70;
		static Bullet* result;
		__asm {
			PUSHAD
			MOV ECX, y
			MOV EDX, x
			MOV EAX, bullet
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void display_error_message_id_with_sfx(s32 player, s32 message_id, s32 sound_id) {
		const u32 func_address = 0x0048EE30;
		__asm {
			PUSHAD
			MOV ESI, sound_id
			MOV EDI, message_id
			MOV EBX, player
			CALL func_address
			POPAD
		}
	}

	// For ai_manage_building_placement
	s32 get_bunker_placement_position(s32 placement_id, s32* region_id) {
		const u32 func_address = 0x004442E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH region_id
			PUSH placement_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* get_disabled_unit_in_range(s32 owner, Point16 position) {
		const u32 func_address = 0x00440420;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH position
			PUSH owner
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_get_build_location(s32 unit_id, char* placement_data, s32 player, Point16 try_position, s32 max_range) {
		const u32 func_address = 0x004461B0;
		__asm {
			PUSHAD
			PUSH max_range
			PUSH try_position
			PUSH player
			MOV ESI, placement_data
			MOV EBX, unit_id
			CALL func_address
			POPAD
		}
	}

	void ai_placement_data_something(s8* placement_data, Point16 try_position) {
		const u32 func_address = 0x00443610;
		__asm {
			PUSHAD
			PUSH try_position
			MOV EBX, placement_data
			CALL func_address
			POPAD
		}
	}

	u32 get_tile_distance(s32 x, s32 y, s32 x2, s32 y2) {
		const u32 func_address = 0x00444100;
		static u32 result;
		__asm {
			PUSHAD
			PUSH y2
			PUSH x2
			PUSH y
			PUSH x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For initialize_unit_counts
	char team_game_count_races(char force, char* bIsLegal, char race) {
		const u32 func_address = 0x004857B0;
		s32 race_xd = race;
		static char result;
		__asm {
			PUSHAD
			PUSH race_xd
			MOV EDI, bIsLegal
			MOVZX EAX, force
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For stat_res_main_proc
	/*void show_supply_info(BinDlg* dialog, s16 graphic, s16 mathA, u16 mathB) {
		const u32 Func_showSupplyInfo = 0x004E53B0;
		s32 graphic_xd = graphic;
		s32 mathA_xd = mathA;
		s32 mathB_xd = mathB;
		__asm {
			PUSHAD
			PUSH mathB_xd
			PUSH mathA_xd
			PUSH graphic_xd
			MOV EBX, dialog
			CALL Func_showSupplyInfo
			POPAD
		}
	}

	void print_stat_res_info(BinDlg* dialog, u32 icon_graphic, u32* accrued_value, u32 resource_count, b32 do_not_accrue) {
		const u32 Func_printStatResInfo = 0x004E52E0;
		__asm {
			PUSHAD
			PUSH do_not_accrue
			PUSH resource_count
			PUSH accrued_value
			PUSH icon_graphic
			MOV EBX, dialog
			CALL Func_printStatResInfo
			POPAD
		}
	}*/

	// For redraw_supply_info
	bool unit_stat_cond_standard() {
		const u32 func_address = 0x00424980;
		static bool result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For draw_supply_unit_status
	void draw_shield_upgrade(BinDlg* dialog, u16 status_screen_index) {
		const u32 func_address = 0x00425510;
		u32 status_screen_index_xd = status_screen_index;
		__asm {
			PUSHAD
			PUSH status_screen_index_xd
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	void draw_armor(BinDlg* dialog, s32 unit_id, u16 status_screen_index) {
		const u32 Func_drawArmor = 0x00425600;
		u32 status_screen_index_xd = status_screen_index;
		__asm {
			PUSHAD
			PUSH status_screen_index_xd
			MOV EDX, unit_id
			MOV EAX, dialog
			CALL Func_drawArmor
			POPAD
		}
	}

	// For progressBulletFrame
	void bullet_behavior_fly(Bullet* bullet) {
		const u32 func_address = 0x0048B3B0;
		__asm {
			PUSHAD
			MOV EAX, bullet
			CALL func_address
			POPAD
		}
	}

	void bullet_behavior_follow(Bullet* bullet) {
		const u32 func_address = 0x0048B500;
		__asm {
			PUSHAD
			MOV EAX, bullet
			CALL func_address
			POPAD
		}
	}

	void bullet_behavior_bounce(Bullet* bullet) {
		const u32 func_address = 0x0048B2D0;
		__asm {
			PUSHAD
			MOV EDI, bullet
			CALL func_address
			POPAD
		}
	}

	void bullet_behavior_persist(Bullet* bullet) {
		const u32 func_address = 0x0048BC70;
		__asm {
			PUSHAD
			MOV ECX, bullet
			CALL func_address
			POPAD
		}
	}

	void bullet_behavior_instant(Bullet* bullet) {
		const u32 func_address = 0x0048A560;
		__asm {
			PUSHAD
			MOV EAX, bullet
			CALL func_address
			POPAD
		}
	}

	void bullet_behavior_area(Bullet* bullet) {
		const u32 func_address = 0x0048B440;
		__asm {
			PUSHAD
			MOV EAX, bullet
			CALL func_address
			POPAD
		}
	}

	// For give_unit_to_player
	void give_all_unit_properties_to_player(Unit* unit, s8 player) {
		const u32 func_address = 0x0047C9F0;
		s32 player_id_xd = player;
		__asm {
			PUSHAD
			PUSH player_id_xd
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	Unit* get_hidden_miner(Unit* unit) {
		const u32 func_address = 0x00468A60;
		static Unit* result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_has_hangar(Unit* unit) {
		return unit->id == UnitId::ProtossSolarion;
	}

	s32 forHangarUnits(Unit* unit, Unit* (__fastcall* func)(Unit*, Unit*), s32 arg) {
		const u32 func_address = 0x00465200;
		static s32 result;
		__asm {
			PUSHAD
			PUSH arg
			PUSH func
			MOV EDI, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* get_creating_unit(Unit* unit) {
		const u32 func_address = 0x00466A30;
		static Unit* result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void move_sprite(Sprite* sprite, s16 x, s16 y) {
		const u32 func_address = 0x00497A10;
		__asm {
			PUSHAD
			MOV ECX, sprite
			MOVZX EBX, x
			MOV DI, y
			CALL func_address
			POPAD
		}
	}

	u32 update_unit_sorting_info(u32 x_diff, Unit* unit, u32 y_diff) {
		const u32 func_address = 0x0046A000;
		static s32 result;
		__asm {
			PUSHAD
			PUSH y_diff
			MOV ESI, unit
			MOV EAX, x_diff
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}
	
	// For bullet_behavior_reassign
	void play_sprite_iscript(Sprite* sprite, char scriptId, s32 bPlay) {
		const u32 func_address = 0x00499D00;
		s32 scriptId_xd = scriptId;
		__asm {
			PUSHAD
			PUSH bPlay
			PUSH scriptId_xd
			MOV ECX, sprite
			CALL func_address
			POPAD
		}
	}

	Bullet* assign_weapon_data(Bullet* bullet, char behavior_type) {
		const u32 func_address = 0x0048AA80;
		static Bullet* result;
		__asm {
			PUSHAD
			MOVZX EDX, behavior_type
			MOV EAX, bullet
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}
	
	// For ai_training_normal
	AiRegion* get_ai_region_info_from_unit_loc(Unit* unit) {
		const u32 func_address = 0x00438BF0;
		static AiRegion* result;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void add_guard_ai_to_unit(Unit* unit) {
		const u32 func_address = 0x00462960;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	s8 create_unit_allowed(u16 unit_id, Unit* unit, s32 player) {
		const u32 func_address = 0x0046E1C0;
		static s8 result;
		__asm {
			PUSHAD
			PUSH player
			MOV ESI, unit
			MOVZX EAX, unit_id
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	b32 SBmpLoadImage(const char *filename, void* palette, void* buffer, u32 buffersize, u32* width, u32* height, u32* bpp) {
		const u32 func_address = 0x00410154;
		static b32 result;
		__asm {
			PUSHAD
			PUSH bpp
			PUSH height
			PUSH width
			PUSH buffersize
			PUSH buffer
			PUSH palette
			PUSH filename
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_military_should_wait_for_group
	bool ai_are_on_connected_regions(Unit* unit1, Unit* unit2) {
		const u32 func_address = 0x00438E10;
		static bool result;
		__asm {
			PUSHAD
			MOV ECX, unit2
			MOV EAX, unit1
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool ai_check_military_grouping(Unit* unit) {
		const u32 func_address = 0x00439550;
		static bool result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For remove_unit_ai
	s32 remove_from_ai_structs(Unit* unit, UnitAi* unitAI) {
		const u32 func_address = 0x00439D60;
		static s32 result;
		__asm {
			PUSHAD
			PUSH unitAI
			PUSH unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 remove_worker_or_building_ai(Unit* unit, b32 force_remove_building_ai) {
		const u32 func_address = 0x00434C90;
		static s32 result;
		__asm {
			PUSHAD
			PUSH force_remove_building_ai
			MOV EDI, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_remove_cleric(Unit* unit) {
		const u32 func_address = 0x004477C0;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL func_address
			POPAD
		}
	}

	void ai_training_simple_military(Unit* new_unit, Unit* source_unit) {
		const u32 func_address = 0x0043E280;
		__asm {
			PUSHAD
			PUSH source_unit
			PUSH new_unit
			CALL func_address
			POPAD
		}
	}

	// For delete_ai_town_if_needed
	AiTown* try_begin_delete_town(AiTown* town) {
		const u32 func_address = 0x00434330;
		static AiTown* result;
		__asm {
			PUSHAD
			MOV ESI, town
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	AiScript* delete_aiscripts_using_town(AiTown* town) {
		const u32 func_address = 0x0045AE20;
		static AiScript* result;
		__asm {
			PUSHAD
			MOV EBX, town
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For display_establishing_shot
	void* load_init_credits_bin(const char* a1) {
		s32 func_address = 0x004D8DF0;
		void* result_;
		__asm {
			MOV EAX, a1
			CALL func_address
			MOV result_, EAX
		}
		return result_;
	}

	bool SFileOpenArchive(const char* archiveName, u32 dwPriority, u32 dwFlags, void* const* handle) {
		const u32 func_address = 0x004100B2;
		static u32 result;
		__asm {
			PUSHAD
			PUSH handle
			PUSH dwFlags
			PUSH dwPriority
			PUSH archiveName
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool SFileCloseArchive(void* archive) {
		const u32 func_address = 0x004100BE;
		static u32 result;
		__asm {
			PUSHAD
			PUSH archive
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool sfile_open_file_ex(void* hMPQ, const char* szFileName, u32 dwSearchScope, HANDLE** phFile) {
		const u32 func_address = 0x004100C4;
		static u32 result;
		__asm {
			PUSHAD
			PUSH phFile
			PUSH dwSearchScope
			PUSH szFileName
			PUSH hMPQ
			CALL func_address
			MOV result, eAX
			POPAD
		}
		return (bool)result;
	}

	//bool SFileCloseFile(void* hFile)
	//{
	//	const u32 Func_SFileCloseFile = 0x4100B8;
	//	static u32 result;
	//	__asm
	//	{
	//		PUSHAD
	//		PUSH hFile
	//		CALL Func_SFileCloseFile
	//		MOV result, EAX
	//		POPAD
	//	}
	//	return (bool)result;
	//}

	// For ai_are_on_connected_regions
	bool ai_regions_are_connected(u32 player, u32 region1, u32 region2) {
		const u32 func_address = 0x00437E70;
		static bool result;
		__asm {
			PUSHAD
			MOV ESI, region2
			MOV EBX, region1
			MOV EAX, player
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}
	
	// For btsncond_zerg_building_rally
	u32 btnscond_rally(u32 unused, u32 player, Unit* unit) {
		const u32 func_address = 0x00429520;
		static u32 result;
		__asm {
			PUSHAD
			PUSH unit
			MOV EDX, player
			MOV ECX, unused
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For AI_GetBuildingLocation
	u32 ai_get_building_placement_data(u32 max_range, u32 unit_id, char* placement_data, u32 player, Point16 desired_pos) {
		const u32 func_address = 0x00445D60;
		static u32 result;
		__asm {
			PUSHAD
			PUSH desired_pos
			PUSH player
			PUSH placement_data
			PUSH unit_id
			MOV EAX, max_range
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char* ai_remove_unreachable_regions(char* placement_data, Point16 desired_pos) {
		const u32 func_address = 0x00443900;
		static char* result;
		__asm {
			PUSHAD
			PUSH desired_pos
			PUSH placement_data
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_remove_dangerous_regions(u32 unit_id, char* placement_data, u32 placement_id, Point16 desired_pos) {
		const u32 func_address = 0x00443870;
		static u32 result;
		__asm {
			PUSHAD
			PUSH desired_pos
			PUSH placement_id
			MOV ECX, placement_data
			MOV EAX, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_remove_far_tiles(char* placement_data, Point16 desired_pos, u32 max_range) {
		const u32 func_address = 0x00444E20;
		static u32 result;
		__asm {
			PUSHAD
			PUSH max_range
			PUSH desired_pos
			MOV ECX, placement_data
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_remove_different_height(char* placement_data, Point16 desired_pos) {
		const u32 func_address = 0x004436C0;
		static u32 result;
		__asm {
			PUSHAD
			PUSH desired_pos
			PUSH placement_data
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_placement_related_something(u32 unit_id, char* placement_data, Point16 desired_pos) {
		const u32 func_address = 0x00443E30;
		static u32 result;
		__asm {
			PUSHAD
			PUSH desired_pos
			PUSH placement_data
			PUSH unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_remove_unfit_tiles(u32 unit_id, char* placement_data, u32 player, Point16 desired_pos) {
		const u32 func_address = 0x00444CE0;
		__asm {
			PUSHAD
			PUSH desired_pos
			PUSH player
			PUSH placement_data
			MOV ECX, unit_id
			CALL func_address
			POPAD
		}
	}

	u32 ai_remove_addon_places(u32 unit_id, char* placement_data, u32 player, Point16 desired_pos) {
		const u32 func_address = 0x00443CC0;
		static u32 result;
		__asm {
			PUSHAD
			PUSH desired_pos
			PUSH player
			PUSH placement_data
			PUSH unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_turret_placement_position(char* placement_data, u32 player, Point16 desired_pos, u32 unit_id, u32 unk_flag) {
		const u32 func_address = 0x00443AC0;
		static u32 result;
		__asm {
			PUSHAD
			PUSH unk_flag
			PUSH unit_id
			PUSH desired_pos
			PUSH player
			PUSH placement_data
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_pylon_placement_position(char* placement_data, u32 player, Point16 desired_pos) {
		const u32 func_address = 0x004448D0;
		static u32 result;
		__asm {
			PUSHAD
			PUSH desired_pos
			PUSH player
			PUSH placement_data
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_default_placement_position(u32 unit_id, char* placement_data, u32 player, Point16 desired_pos) {
		const u32 func_address = 0x004449A0;
		static u32 result;
		__asm {
			PUSHAD
			PUSH desired_pos
			PUSH player
			PUSH placement_data
			MOV ECX, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_check_military_grouping
	void get_region_center_from_id(u16 region_id, u32* out_x, u32* out_y) {
		const u32 func_address = 0x0049CAA0;
		__asm {
			PUSHAD
			PUSH out_y
			PUSH out_x
			MOVZX EAX, region_id
			CALL func_address
			POPAD
		}
	}

	// For ignore_iscript_and_go_to_idle
	void play_idle_iscript(Sprite* sprite) {
		const u32 func_address = 0x00497880;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL func_address
			POPAD
		}
	}

	// For issue_order_targeting_ground
	void order_copy(s32 a1, Unit* a2, s8 a3, Point16* a4, Unit* a5, s32 a6) {
		const u32 func_address = 0x00474810;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV EDX, a2
			MOVZX CL, a3
			PUSH a4
			PUSH a5
			PUSH a6
			CALL func_address
			POPAD
		}
	}

	static u32 __cdecl remove_creep_at_unit_nop(u32 x_tile, u32 y_tile, Box32* rect) {
		return 0;
	}

	// For unit_destructor_special
	s32 remove_creep_at_unit(s32 left, s32 top, s16 unit_id) {
		const u32 func_address = 0x00414560;
		static s32 result;
		__asm {
			PUSHAD
			PUSH remove_creep_at_unit_nop
			MOVZX EDX, unit_id
			MOV EAX, top
			MOV ECX, left
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_attack_unit
	void stimpack_proc(Unit* unit) {
		const u32 func_address = 0x004554A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	/*
	// For soundThread
	void fastFileRead(s32 a1, s32 a2, char* a3, s32 fileName, s32 buffer, s32 bytesRead, char* logFileName, s32 logLine) {
		const u32 Func_fastFileRead = 0x004D2D10;
		__asm {
			PUSHAD
			PUSH logLine
			PUSH logFileName
			PUSH bytesRead
			PUSH buffer
			PUSH fileName
			MOV EDI, a3
			MOV ECX, a2
			MOV EAX, a1
			CALL Func_fastFileRead
			POPAD
		}
	}

	s32 parseWaveFile(s32 a1, s32 a2, s32 a3) {
		static s32 result;
		const u32 Func_parseWaveFile = 0x004BBD80;
		__asm {
			PUSHAD
			MOV ESI, a3
			MOV EDI, a2
			MOV ECX, a1
			CALL Func_parseWaveFile
			MOV result, EAX
			POPAD
		}
		return result;
	}
	*/

	bool SMemFree(void* location) {
		static bool result;
		const u32 func_address = 0x00410070;
		const char* logFileName  = "";
		u32   logLine            = 0;
		u32   defaultValue       = 0;
		__asm {
			PUSHAD
			PUSH defaultValue
			PUSH logLine
			PUSH logFileName
			PUSH location
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool SMemFree(void* location, const char* logFileName, s32 logLine, char defaultValue) {
		static bool result;
		const u32 func_address = 0x00410070;
		__asm {
			PUSHAD
			PUSH defaultValue
			PUSH logLine
			PUSH logFileName
			PUSH location
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	u32 get_sound_duration(s32 sound_id) {
		static u32 result;
		const u32 func_address = 0x004BB890;
		__asm {
			PUSHAD
			MOV ESI, sound_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For soundInit
	/*
	void dSoundDestroy() {
		const u32 Func_dSoundDestroy = 0x004BC180;
		__asm {
			PUSHAD
			CALL Func_dSoundDestroy
			POPAD
		}
	}

	s32 dSoundCreate(s32 a1) {
		static s32 result;
		const u32 Func_dSoundCreate = 0x004BB640;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_dSoundCreate
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 setCoopLevel(s32 a1, s32 a2) {
		static s32 result;
		const u32 Func_setCoopLevel = 0x004BB600;
		__asm {
			PUSHAD
			MOV EDI, a1
			MOV ESI, a2
			CALL Func_setCoopLevel
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 createSoundBuffer(s32 a1) {
		static s32 result;
		const u32 Func_createSoundBuffer = 0x004BB5A0;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_createSoundBuffer
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 unkSoundCoreFunc() {
		static s32 result;
		const u32 Func_unkSoundCoreFunc = 0x004BBA90;
		__asm {
			PUSHAD
			CALL Func_unkSoundCoreFunc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 sFileDdaInitialize(char directSound) {
		static s32 result;
		const u32 Func_sFileDdaInitialize = 0x004101C0;
		__asm {
			PUSHAD
			PUSH directSound
			CALL Func_sFileDdaInitialize
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 sVidInitialize(char video) {
		static s32 result;
		const u32 Func_sVidInitialize = 0x004101C6;
		__asm {
			PUSHAD
			PUSH video
			CALL Func_sVidInitialize
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 loadButtonSFXFile() {
		static s32 result;
		const u32 Func_loadButtonSFXFile = 0x004BCB20;
		__asm {
			PUSHAD
			CALL Func_loadButtonSFXFile
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void initVolume() {
		const u32 Func_initVolume = 0x004BC450;
		__asm {
			PUSHAD
			CALL Func_initVolume
			POPAD
		}
	}

	s32 sRegLoadValue(char keyName, char valueName, char a3, char value) {
		static s32 result;
		const u32 Func_sRegLoadValue = 0x00410076;
		__asm {
			PUSHAD
			PUSH value
			PUSH a3
			PUSH valueName
			PUSH keyName
			CALL Func_sRegLoadValue
			MOV result, EAX
			POPAD
		}
		return result;
	}

	HANDLE beginThreadEx(s32 lpThreadAttributes, s32 dwStackSize, s32 threadId, s32 a4, s32 dwCreationFlags, s32 lpThreadId) {
		static HANDLE result;
		const u32 Func_beginThreadEx = 0x0040CD3C;
		__asm {
			PUSHAD
			PUSH lpThreadId
			PUSH dwCreationFlags
			PUSH a4
			PUSH threadId
			PUSH dwStackSize
			PUSH lpThreadAttributes
			CALL Func_beginThreadEx
			MOV result, EAX
			POPAD
		}
		return result;
	}
	*/
	
	// For ai_perform_task
	void ai_overlord_behavior(Unit* unit) {
		const u32 func_address = 0x00434DF0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void ai_nuke_silo_behavior(Unit* unit) {
		const u32 func_address = 0x00434D50;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	bool ai_unit_construction_behavior(s32 player, Unit* unit, s32 someBool) {
		static s32 result;
		const u32 func_address = 0x00433890;
		__asm {
			PUSHAD
			PUSH someBool
			PUSH unit
			PUSH player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	char ai_plan_do_merge_morph(s32 player, Unit* unit, s32 someBool) {
		static s32 result;
		const u32 func_address = 0x00435F10;
		__asm {
			PUSHAD
			PUSH someBool
			PUSH unit
			PUSH player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	s32 ai_morph_behavior(s32 player, Unit* unit, void* unit_ai) {
		static s32 result;
		const u32 func_address = 0x00434FF0;
		__asm {
			PUSHAD
			PUSH unit_ai
			PUSH unit
			MOV EBX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_train_behavior(Unit* unit, void* unit_ai, s32 player) {
		const u32 func_address = 0x00434720;
		__asm {
			PUSHAD
			MOV EBX, player
			MOV EDX, unit_ai
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void ai_upgrade_behavior(Unit* unit, s32 player) {
		const u32 func_address = 0x00434670;
		__asm {
			PUSHAD
			MOV ECX, player
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	s32 ai_research_behavior(Unit* unit, s32 player) { // return value unused
		static s32 result;
		const u32 func_address = 0x0043462F;
		__asm {
			PUSHAD
			MOV ECX, player
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char ai_construct_behavior(Unit* unit, void* unit_ai, s32 player) {
		static s32 result;
		const u32 func_address = 0x00434E70;
		__asm {
			PUSHAD
			PUSH player
			MOV EAX, unit_ai
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	s32 ai_make_detectors_behavior(Unit* unit, s32 player) {
		static s32 result;
		const u32 func_address = 0x00435130;
		__asm {
			PUSHAD
			MOV EDI, player
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_plan_do_merge_morph

	Unit* ai_get_mutalisk_for_morph(s32 player) {
		static Unit* result;
		const u32 func_address = 0x004375F0;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_get_high_templar_for_summon(s32 player, Unit** first, Unit** second) {
		static s32 result;
		const u32 func_address = 0x00437410;
		__asm {
			PUSHAD
			PUSH second
			PUSH first
			PUSH player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_get_dark_templar_for_summon(s32 player, Unit** first, Unit** second) {
		static s32 result;
		const u32 func_address = 0x00437320;
		__asm {
			PUSHAD
			PUSH second
			MOV EBX, first
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void assign_order_with_target(Unit* unit, Unit* target, s32 order_id) {
		const u32 func_address = 0x004752B0;
		__asm {
			PUSHAD
			PUSH order_id
			MOV ESI, target
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	s16 ai_reserve(s32 player, s32 someBool) {
		static s16 result;
		const u32 func_address = 0x00447D00;
		__asm {
			PUSHAD
			PUSH someBool
			MOV ECX, player
			CALL func_address
			MOV result, AX
			POPAD
		}
		return result;
	}

	AI_Main* ai_pop_spending_request(s32 player) {
		static AI_Main* result;
		const u32 func_address = 0x004476D0;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* AI_GetHydraliskForMorph(s32 player) {
		static Unit* result;
		const u32 func_address = 0x00437570;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 is_unit_id_race_equal_to_unit_race(u16 unit_id, Unit* unit) {
		static s32 result;
		const u32 func_address = 0x00432480;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOVZX EAX, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool has_resources_for_unit(u16 unit_id, s32 player) {
		static u32 result;
		const u32 func_address = 0x004487B0;
		__asm {
			PUSHAD
			PUSH player
			MOVZX ECX, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	char ai_perform_task(Unit* unit) {
		static char result;
		const u32 func_address = 0x004361A0;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	s32 ai_get_request_build_type(s32 player) {
		static s32 result;
		const u32 func_address = 0x00447590;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool ai_train_unit(Unit* unit, u16 unit_id, char requestId, void* value) {
		static bool result;
		const u32 func_address = 0x004672F0;
		u32 unit_id_xd = unit_id;
		s32 request_id_xd = requestId;
		__asm {
			PUSHAD
			PUSH value
			PUSH request_id_xd
			PUSH unit_id_xd
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* ai_create_unit(u32 unit_id, s32 player, s32 x, s32 y) {
		static Unit* result;
		const u32 func_address = 0x004A0AD0;
		__asm {
			PUSHAD
			PUSH player
			PUSH unit_id
			MOV ECX, x
			MOV EAX, y
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool is_ai_script_name_valid(u32 player, u32 name) {
		const u32 func_address = 0x0045AEA0;
		static b32 result;
		__asm {
			PUSHAD
			MOV ESI, name
			MOV EDX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 parse_aiscript_name(s32 name, s32* is_bw) {
		static s32 result;
		const u32 Func_parseAIScriptName = 0x0045B170;
		__asm {
			PUSHAD
			MOV ESI, is_bw
			MOV EDX, name
			CALL Func_parseAIScriptName
			MOV result, EAX
			POPAD
		}
		return result;
	}

	AiScript* run_aiscript(Box32* area, u32 player, u32 block, u32 is_bw) {
		static AiScript* result;
		const u32 Func_AI_RunAIScript = 0x0045AEF0;
		__asm {
			PUSHAD
			PUSH is_bw
			PUSH block
			PUSH player
			MOV ESI, area
			CALL Func_AI_RunAIScript
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// not functional!
	AiScript* ai_stop(AiScript* ai_script) {
		static AiScript* result;
		const u32 Func_AI_Stop = 0x00403380;
		const void* Var_AI_thread_end = (void*)0x0068C0FC;
		__asm {
			PUSHAD
			MOV EAX, ai_script
			MOV EDX, Var_AI_thread_end
			CALL Func_AI_Stop
			MOV result, EAX
			POPAD
		}
		return result;
	}

	AiTown* aiscript_start_town(u32 x, u32 y, u32 resarea, u32 player) {
		static AiTown* result;
		const u32 Func_AiScript_StartTown = 0x00434220;
		__asm {
			PUSHAD
			PUSH resarea
			PUSH y
			PUSH x
			MOV EAX, player
			CALL Func_AiScript_StartTown
			MOV result, EAX
			POPAD
		}
		return result;
	}

	AiTown* aiscript_start_areatown(Box32* area, u32 resarea, u32 player) {
		static AiTown* result;
		const u32 Func_AiScript_StartAreaTown = 0x00434060;
		__asm {
			PUSHAD
			PUSH resarea
			PUSH area
			MOV EAX, player
			CALL Func_AiScript_StartAreaTown
			MOV result, EAX
			POPAD
		}
		return result;
	}

	VictoryFunc* Victory = (VictoryFunc*)0x4C5310;

	void display_text_message(char* text, u32 color, u32 ticks, b32 from_trigger) {
		const u32 Func_DisplayTextMessage = 0x0048CD30;
		__asm {
			PUSHAD
			PUSH from_trigger
			PUSH ticks
			PUSH color
			MOV EAX, text
			CALL Func_DisplayTextMessage
			POPAD
		}
	}

	u32 get_text_display_time(char* text) {
		static u32 result;
		const u32 Func_GetTextDisplayTime = 0x004C51D0;
		__asm {
			PUSHAD
			MOV EAX, text
			CALL Func_GetTextDisplayTime
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For prepare_unit_move_custom
	s32 sort_unit(Unit* unit) {
		static s32 result;
		const u32 Func_sort_unit = 0x0046A300;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_sort_unit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void refresh_grp_image(Sprite* sprite) {
		const u32 Func_refresh_grp_image = 0x00497530;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_refresh_grp_image
			POPAD
		}
	}

	char* get_chk_string(u32 i) {
		static char* result;
		const u32 func_get_chk_string = 0x004BD0C0;
		__asm {
			PUSHAD
			MOV EDX, i
			CALL func_get_chk_string
			MOV result, EAX
			POPAD
		}
		return result;
	}

	SFileOpenFile_Func* SFileOpenFile       = (SFileOpenFile_Func*)    0x0041013C;
	SFileCloseFile_Func* SFileCloseFile     = (SFileCloseFile_Func*)   0x004100B8;
	SFileDdaBeginEx_Func* SFileDdaBeginEx   = (SFileDdaBeginEx_Func*)  0x004101CC;
	SFileDdaEnd_Func* SFileDdaEnd           = (SFileDdaEnd_Func*)      0x004101D8;
	SFileGetFileSize_Func* SFileGetFileSize = (SFileGetFileSize_Func*) 0x00410142;
	SFileReadFile_Func* SFileReadFile       = (SFileReadFile_Func*)    0x00410148;

	playNextMusic_Func* play_next_bw_music  = (playNextMusic_Func*) 0x004A6250;
	stopMusic_Func* stop_bw_music           = (stopMusic_Func*)     0x004A5F50;

	void prepare_formation_movement(MovementGroup* movement_group, char can_be_obstructed) {
		const u32 func_address = 0x0049A8C0;
		s32 can_be_obstructed_xd = can_be_obstructed;
		__asm {
			PUSHAD
			PUSH can_be_obstructed_xd
			MOV EDI, movement_group
			CALL func_address
			POPAD
		}
	}

	Unit* find_fow_unit(u16 y_pos, u16 x_pos, u32 fow_unit_id) {
		static Unit* result;
		const u32 func_address = 0x00487FD0;
		__asm {
			PUSHAD
			PUSH fow_unit_id
			MOVZX ECX, x_pos
			MOVZX EAX, y_pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void targeted_order_unit(Unit* a1, Unit* a2, int a3, char a4) {
		const u32 func_address = 0x004756E0;
		u32 a4_xd = a4;
		__asm {
			PUSHAD
			PUSH a4_xd
			PUSH a3
			MOV ESI, a2
			MOV EDX, a1
			CALL func_address
			POPAD
		}
	}

	void targeted_order_ground(int a1, int a2, Unit* a3, int a4, int a5, int a6) {
		const u32 func_address = 0x004756B0;
		__asm {
			PUSHAD
			PUSH a6
			PUSH a5
			PUSH a4
			MOV ESI, a3
			MOV EBX, a2
			MOV EDX, a1
			CALL func_address
			POPAD
		}
	}

	Unit* get_next_commanded_unit() {
		static Unit* result;
		const u32 func_address = 0x0049A850;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void get_formation_movement_target(Unit* unit, MovementGroup* movement_group) {
		const u32 func_address = 0x0049A500;
		__asm {
			PUSHAD
			PUSH movement_group
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	u8 get_right_click_order(u32 index, Unit* unit, s32 x, s32 y, Unit** target, s32 unit_id) {
		static u8 result;
		void* function = g_get_right_click_order_array[index];
		__asm {
			PUSHAD
			PUSH unit_id
			PUSH target
			PUSH y
			MOV EDX, x
			MOV ECX, unit
			CALL function
			MOV result, AL
			POPAD
		}
		return result;
	}

	void edit_unit_flags(Unit* unit, u16* flags) {
		const u32 func_address = 0x004CD480;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV EBX, flags
			CALL func_address
			POPAD
		}
	}

	void unit_placement_error(const char *a1, char a2) {
		const u32 func_address = 0x0048D140;
		s32 a2_xd = a2;
		__asm {
			PUSHAD
			MOV ESI, a1
			PUSH a2_xd
			CALL func_address
			POPAD
		}
	}

	// For HandleWindowMessages
	char check_last_file_error() {
		static char result;
		const u32 func_address = 0x004D28D0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	s32 has_messages_waiting(void* tag_msg, s32 b_something) {
		static s32 result;
		const u32 func_address = 0x004D1650;
		__asm {
			PUSHAD
			PUSH b_something
			MOV EAX, tag_msg
			CALL func_address
			MOV result, EAX 
			POPAD
		}
		return result;
	}

	void send_mouse_move_event(void* fast_call_func) {
		const u32 func_address = 0x004D1AE0;
		__asm {
			PUSHAD
			PUSH fast_call_func
			POPAD
		}
	}

	s32 send_input_to_all_dialogs(DialogEvent* dlg_event) {
		static s32 result;
		const u32 func_address = 0x00419FD0;
		__asm {
			PUSHAD
			MOV EBX, dlg_event
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	VectorHead* play_briefing_wav_begin() {
		static VectorHead* result;
		const u32 func_address = 0x004BBE50;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void iterate_timers() {
		const u32 func_address = 0x00415DC0;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void ai_build(u32 unit_id, b32 only_if_needed, u32 count, AiTown* town, u32 priority) { // 004323D0
		const u32 func_address = 0x004323D0;
		__asm {
			PUSHAD
			PUSH priority
			MOV ESI, town
			MOV EAX, count
			MOV EDX, only_if_needed
			MOV ECX, unit_id
			CALL func_address
			POPAD
		}
	}

	void ai_add_build_request(AiTown* town, s32 method, s32 count, s32 id, s32 priority, b32 only_if_needed) { // 00431F90
		const u32 func_address = 0x00431F90;
		__asm {
			PUSHAD
			PUSH only_if_needed
			PUSH priority
 			PUSH id
			PUSH count
			PUSH method
			MOV ESI, town
			CALL func_address
			POPAD
		}
	}
	
	// For ai_train_defensive_military
	u32 ai_region_train_attack_force(AiRegion* region) {
		static u32 result;
		const u32 func_address = 0x0043E670;
		__asm {
			PUSHAD
			MOV EAX, region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_defend_region(AiRegion* military_region, s32 defense_region, s32 against_air, s32 region_strength) {
		static u32 result;
		const u32 func_address = 0x0043F100;
		__asm {
			PUSHAD
			PUSH region_strength
			PUSH against_air
			PUSH defense_region
			MOV EAX, military_region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_add_base_unit_spending_request(u32 final_unit_id, u32 player_id, AiRegion* region, u8 priority) {
		static u32 result;
		const u32 func_address = 0x004484A0;
		u32 priority_xd = priority;
		__asm {
			PUSHAD
			PUSH priority_xd
			MOV EAX, final_unit_id
			MOV ECX, player_id
			MOV ESI, region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_has_power(u32 id, u32 x, u32 y, u32 player) {
		static bool result;
		const u32 func_address = 0x004936B0;
		__asm {
			PUSHAD
			PUSH player
			PUSH y
			PUSH x
			MOV EAX, id
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void play_sound_global(u32 id) {
		const u32 func_address = 0x004BC2C0;
		__asm {
			PUSHAD
			PUSH id
			CALL func_address
			POPAD
		}
	}

	VectorHead* play_sound_from_file(const char* filename, char* unused) {
		static VectorHead* result;
		const u32 func_address = 0x004BC490;
		__asm {
			PUSHAD
			MOV EAX, filename
			MOV EDI, unused
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	b32 is_valid_for(u32 a1, Unit* unit, u32 a3) {
		static b32 result;
		const u32 func_address = 0x0043B6D0;
		__asm {
			PUSHAD
			PUSH a3
			MOV ESI, unit
			MOV ECX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	b32 region_is_for_attack_prepare(AiRegion* region) {
		static b32 result;
		const u32 func_address = 0x004369F0;
		__asm {
			PUSHAD
			MOV EAX, region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_count_units_in_town(AiTown* town, u32 unit_id, bool finished) {
		static u32 result;
		const u32 func_address = 0x00433210;
		b32 finished_xd = finished;
		__asm {
			PUSHAD
			PUSH finished_xd
			PUSH unit_id
			MOV EAX, town
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 load_scenario_singleplayer(char* title, u32 unused, char* error, char* player, u32 speed) {
		static u32 result;
		const u32 func_address = 0x004A8590;
		__asm {
			PUSHAD
			PUSH speed
			MOV ESI, player
			MOV EDI, error
			MOV ECX, unused
			MOV EAX, title
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_remove_different_height
	u32 get_ground_height_at_px(s32 y, s32 x) {
		static u32 result;
		const u32 func_address = 0x004BD0F0;
		__asm {
			PUSHAD
			MOV EAX, y
			MOV ECX, x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For get_standard_attack_target
	s32 is_ai_target_related(Unit* attacker, Unit* target) {
		static s32 result;
		const u32 func_address = 0x004A1140;
		__asm {
			PUSHAD
			MOV ECX, attacker
			MOV ESI, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_target_priority(Unit* attacker, Unit* target) {
		static s32 result;
		const u32 func_address = 0x00442160;
		__asm {
			PUSHAD
			MOV EBX, attacker
			MOV EDI, target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 add_unit_to_possible_targets(s32 threat_level, Unit* unit) {
		static u32 result;
		const u32 func_address = 0x00440160;
		__asm {
			PUSHAD
			MOV ECX, threat_level
			MOV ESI, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For Talos ability
	void enable_doodad_state(Unit* unit) {
		const u32 func_address = 0x0047BA00;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void disable_doodad_state(Unit* unit) {
		const u32 func_address = 0x0047B960;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	char unfreeze_unit(Unit* unit) {
		static char result;
		const u32 func_address = 0x004F62D0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool unit_can_attack_target(Unit* target, Unit* attacker, b32 check_detection) {
		static bool result;
		const u32 func_address = 0x00476730;
		__asm {
			PUSHAD
			PUSH check_detection
			MOV ESI, attacker
			MOV EBX, target
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	s32 dialog_set_progress_bar_value(BinDlg* dialog, s32 index, u32 value) {
		static s32 result;
		const u32 func_address = 0x00457200;
		__asm {
			PUSHAD
			PUSH value
			MOV ECX, index
			MOV EAX, dialog
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool target_is_inside_minimum_range(Unit* unit, Unit* target) {
		static bool result;
		const u32 func_address = 0x004764D0;
		__asm {
			PUSHAD
			MOV ESI, target
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool is_unit_enemy(Unit* unit, Unit* target) {
		static bool result;
		const u32 func_address = 0x0047B910;
		__asm {
			PUSHAD
			MOV ECX, target
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	u32 play_transmission_from_location(u32 sfx_id, u32 volume, u32 misc, Unit* unit) {
		static u32 result;
		const u32 func_address = 0x004BC270;
		__asm {
			PUSHAD
			PUSH unit
			PUSH misc
			PUSH volume
			PUSH sfx_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 update_text_display(s32 update_case) {
		static s32 result;
		const u32 func_address = 0x0048CB80;
		__asm {
			PUSHAD
			MOV EAX, update_case
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void set_gathering_flags(Unit* unit) {
		const u32 func_address = 0x00468AA0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	bool get_gas_exit_point(Unit* worker, Unit* resource_node, Point16* position, s32* direction) {
		static bool result;
		const u32 func_address = 0x00468B00;
		__asm {
			PUSHAD
			PUSH direction
			PUSH position
			PUSH resource_node
			PUSH worker
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void move_unit(Unit* unit, s16 x, s16 y) {
		const u32 func_address = 0x004EB9F0;
		__asm {
			PUSHAD
			MOVZX ECX, y
			MOVZX EAX, x
			MOV EDX, unit
			CALL func_address
			POPAD
		}
	}

	Unit* finished_mining(Unit* worker, Unit* resource_node) {
		static Unit* result;
		const u32 func_address = 0x00468E40;
		_asm {
			PUSHAD
			MOV EAX, worker
			MOV ECX, resource_node
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Order* delete_order(Unit* unit, u8 order_id) {
		static Order* result;
		const u32 func_address = 0x00474400;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOVZX EDX, order_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void order_targeting_nothing(u8 order_id, Unit* unit, u32 stop_previous_orders) {
		const u32 func_address = 0x00474D60;
		__asm {
			PUSHAD
			PUSH stop_previous_orders
			MOVZX ECX, order_id
			MOV EDX, unit
			CALL func_address
			POPAD
		}
	}

	void hide_unit_partial(Unit* unit, s32 hide_unit) {
		const u32 func_address = 0x00493CA0;
		__asm {
			PUSHAD
			PUSH hide_unit
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	void Order_BeginHarvest(Unit* node, Unit* worker) {
		const u32 func_address = 0x00468D10;
		__asm {
			PUSHAD
			MOV EAX, node
			MOV ECX, worker
			CALL func_address
			POPAD
		}
	}

	Unit* remove_harvesters(Unit* node) {
		static Unit* result;
		const u32 func_address = 0x00468770;
		__asm {
			PUSHAD
			MOV EDI, node
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* remove_from_current_harvesters(Unit* worker, Unit* node) {
		static Unit* result;
		const u32 func_address = 0x004686D0;
		__asm {
			PUSHAD
			PUSH node
			MOV EAX, worker
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_worker_repair_controller(Unit* unit, AiTown* town) {
		static s32 result;
		const u32 func_address = 0x00433C70;
		__asm {
			PUSHAD
			PUSH town
			PUSH unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_try_progress_spending_queue_worker(AiTown* town, Unit* unit) {
		static s32 result;
		const u32 func_address = 0x004348C0;
		__asm {
			PUSHAD
			PUSH unit
			PUSH town
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_is_mineral_field(Unit* unit) {
		static bool result;
		const u32 func_address = 0x00468890;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void ai_auto_harvest(Unit* resource, Unit* worker, Unit* fallback) {
		const u32 func_address = 0x00433B90;
		__asm {
			PUSHAD
			PUSH fallback
			MOV ECX, worker
			MOV EAX, resource
			CALL func_address
			POPAD
		}
	}

	Unit* harvest_next_nearest_resources_ex(s32 worker_count_bool, Unit* worker, Point16 worker_pos) {
		static Unit* result;
		const u32 func_address = 0x004432D0;
		__asm {
			PUSHAD
			PUSH worker_pos
			PUSH worker
			MOV EAX, worker_count_bool
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* get_ai_refinery(AiTown* town, s32 gas_state, s32 index) {
		static Unit* result;
		const u32 func_address = 0x00432B90;
		__asm {
			PUSHAD
			PUSH index
			PUSH gas_state
			MOV ESI, town
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_get_vespene_type(Unit* node, s32 player_id) {
		static u32 result;
		const u32 func_address = 0x004326E0;
		__asm {
			PUSHAD
			MOV EDX, player_id
			MOV EAX, node
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_resource_need_split(u8* out_gas_percentage, u32 player, u8* out_mineral_percentage) {
		const u32 func_address = 0x004475B0;
		__asm {
			PUSHAD
			PUSH out_mineral_percentage
			MOV ESI, player
			MOV EBX, out_gas_percentage
			CALL func_address
			POPAD
		}
	}

	u32 ai_add_town_spending_request(u32 player, u32 priority, u32 unit_id, void* param) {
		static u32 result;
		const u32 func_address = 0x00448400;
		__asm {
			PUSHAD
			PUSH param
			MOV ESI, unit_id
			MOV EDI, priority
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_closest_base_from_startloc(u32 player_id, u32 x, u32 y) {
		static u32 result;
		const u32 func_address = 0x00443A20;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			MOV EAX, player_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* get_resource_area_gas(u32 resarea, Unit** gas_1, Unit** gas_2, Unit** gas_3) {
		static Unit* result;
		const u32 func_address = 0x00444460;
		__asm {
			PUSHAD
			PUSH gas_3
			PUSH gas_2
			PUSH gas_1
			PUSH resarea
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* get_resource_area_mineral(u32 resarea) {
		static Unit* result;
		const u32 func_address = 0x004445E0;
		__asm {
			PUSHAD
			MOV EDX, resarea
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_optimal_mineral_workers(u32 resarea, u32 player) {
		static u32 result;
		const u32 func_address = 0x004439B0;
		__asm {
			PUSHAD
			MOV ECX, player
			MOV EAX, resarea
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_get_old_peons(AiTown* town, u32 count) {
		const u32 func_address = 0x004355C0;
		__asm {
			PUSHAD
			PUSH count
			PUSH town
			CALL func_address
			POPAD
		}
	}

	BTNSACT_ChangeButtons_Func* BTNSACT_ChangeButtons = (BTNSACT_ChangeButtons_Func*) 0x00459AF0;
	updateSelectedUnitData_Func* updateSelectedUnitData = (updateSelectedUnitData_Func*) 0x004C38B0;

	void update_hud_info() {
		const u32 func_address = 0x004d93f0;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void remove_powerup_image_overlay(Unit* worker) {
		const u32 func_address = 0x004F3900;
		__asm {
			PUSHAD
			MOV EAX, worker
			CALL func_address
			POPAD
		}
	}

	void clear_tile_flags_from_area(u32 flags, u32 tile_x, u32 tile_y, u32 tile_width, u32 tile_height) {
		const u32 func_address = 0x00472E50;
		__asm {
			PUSHAD
			PUSH tile_y
			PUSH tile_x
			MOV ECX, tile_height
			MOV EDI, tile_width
			MOV EAX, flags
			CALL func_address
			POPAD
		}
	}

	void set_map_tile_flags(u32 flags, u32 tile_x, u32 tile_y, u32 tile_width, u32 tile_height) {
		const u32 func_address = 0x00472EB0;
		__asm {
			PUSHAD
			PUSH tile_y
			PUSH tile_x
			MOV EDX, tile_height
			MOV EDI, tile_width
			MOV ESI, flags
			CALL func_address
			POPAD
		}
	}

	void set_creep(u32 tile_x, u32 tile_y, u32 tile_width, u32 tile_height, b32 skip_border) {
		const u32 func_address = 0x0045CD50;
		__asm {
			PUSHAD
			PUSH skip_border
			PUSH tile_height
			PUSH tile_width
			PUSH tile_y
			PUSH tile_x
			CALL func_address
			POPAD
		}
	}

	s32 get_creep_spread_area(u32 unit_id, u32 x, u32 y, b32 completed, Box32* rect) {
		static s32 result;
		const u32 func_address = 0x00413DB0;
		__asm {
			PUSHAD
			MOV EDX, unit_id
			PUSH y
			PUSH x
			MOV EAX, completed
			MOV ECX, rect
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 clip_creep_area_in_bounds(u32 tile_x, u32 tile_y, ClipCreepAreaInBounds_Callback* callback) {
		static s32 result;
		const u32 func_address = 0x00413D10;
		__asm {
			PUSHAD
			PUSH callback
			MOV EDI, tile_y
			MOV EBX, tile_x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 mark_cagra_to_disappear(u32 tile_x, u32 tile_y) {
		static s32 result;
		const u32 func_address = 0x0047DD60;
		__asm {
			PUSHAD
			MOV EBX, tile_y
			PUSH tile_x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void* process_options_change(BinDlg* control) {
		static void* result;
		const u32 func_address = 0x004C9780;
		__asm {
			PUSHAD
			PUSH control
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void load_dialog(const char* filename, void* function) {
		const u32 func_address = 0x004F57A0;
		__asm {
			PUSHAD
			MOV EDX, function
			MOV ECX, filename
			CALL func_address
			POPAD
		}
	}

	BinDlg** destroy_dialog(BinDlg* control) {
		static BinDlg** result;
		const u32 func_address = 0x00419EA0;
		__asm {
			PUSHAD
			MOV ESI, control
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	main_menu_options_custom_interact_func* main_menu_options_custom_interact = (main_menu_options_custom_interact_func*)0x004CA0A0;
	game_menu_dialog_interact_func* game_menu_dialog_interact = (game_menu_dialog_interact_func*)0x004CA240;

	u32 stop_creep_disappearing(u32 x_tile, u32 y_tile) {
		static u32 result;
		const u32 func_address = 0x0047D540;
		__asm {
			PUSHAD
			MOV EDI, y_tile
			MOV EBX, x_tile
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void secondary_order_spawning_larva(Unit* unit) {
		const u32 func_address = 0x004EA780;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	AiTown* create_ai_town(u32 player, u32 x, u32 y) {
		static AiTown* result;
		const u32 func_address = 0x00432020;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			MOV EBX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Sprite* sprite_destructor(Sprite* sprite) {
		static Sprite* result;
		const u32 func_address = 0x00497B40;
		__asm {
			PUSHAD
			MOV EDI, sprite
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	UnitPath* reassign_path(Unit* unit) {
		static UnitPath* result;
		const u32 func_address = 0x0042F740;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 initialize_unit(Unit* unit, u32 y, u32 unit_id, u32 x, u8 player_id) {
		static u32 result;
		const u32 func_address = 0x0049ECF0;
		u32 player_id_xd = player_id;
		__asm {
			PUSHAD
			PUSH player_id_xd
			PUSH x
			PUSH unit_id
			MOV EDX, y
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void update_hp_gain(Unit* unit) {
		const u32 func_address = 0x0047B180;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	bool unit_is_geyser_ex(Unit* resource_node, Unit* worker) {
		static bool result;
		const u32 func_address = 0x00468930;
		__asm {
			PUSHAD
			PUSH worker
			MOV ECX, resource_node
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void prepend_order(Unit* unit, u8 order_id, Point16 target_pos, Unit* target_unit) {
		const u32 func_address = 0x00474AF0;
		__asm {
			PUSHAD
			PUSH target_unit
			PUSH target_pos
			MOVZX EAX, order_id
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	bool ai_is_building_safe(Unit* building) {
		static bool result;
		const u32 func_address = 0x00438910;
		__asm {
			PUSHAD
			MOV ESI, building
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* ai_find_best_repair_unit(Unit* unit) {
		static Unit* result;
		const u32 func_address = 0x00443160;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_find_expansion(u32 player, u32 resarea, u32 amount) {
		static u32 result;
		const u32 func_address = 0x004453E0;
		__asm {
			PUSHAD
			PUSH amount
			PUSH resarea
			MOV EAX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	b32 ai_try_expanding_to(u32 resarea) {
		static b32 result;
		const u32 func_address = 0x00443A00;
		__asm {
			PUSHAD
			MOV EAX, resarea
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char get_target_acquisition_range_upgrade(Unit* unit) {
		static char result;
		const u32 func_address = 0x00476000;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* let_next_unit_mine(Unit* worker) {
		static Unit* result;
		const u32 func_address = 0x00468970;
		__asm {
			PUSHAD
			MOV EBX, worker
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 begin_harvest(Unit* worker, Unit* node) {
		static s32 result;
		const u32 func_address = 0x00468C70;
		__asm {
			PUSHAD
			MOV EDI, node
			MOV EAX, worker
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_missing_attack_force_units(AiRegion* ai_region, u32* attack_wave, u32 is_campaign) {
		static u32 result;
		const u32 func_address = 0x00438050;
		__asm {
			PUSHAD
			PUSH is_campaign
			PUSH attack_wave
			PUSH ai_region
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_defender_bunker_behavior(Unit* unit, u16 region_id) {
		static s32 result;
		const u32 func_address = 0x0043C480;
		__asm {
			PUSHAD
			MOVZX EAX, region_id
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool ai_has_incomplete_military_in_region(AiRegion* ai_region) {
		static bool result;
		const u32 func_address = 0x00436EB0;
		__asm {
			PUSHAD
			MOV EAX, ai_region
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void delete_military_ai(MilitaryAi* military_ai, AiRegion* ai_region) {
		const u32 func_address = 0x004371D0;
		__asm {
			PUSHAD
			MOV EAX, military_ai
			MOV EDX, ai_region
			CALL func_address
			POPAD
		}
	}

	void update_burrowing_cloaking_units() {
		const u32 func_address = 0x00492EB0;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	s32 use_weapon_iscript(Unit* unit, u8 weapon_id, u32 flag_or_timer, char iscript_anim) {
		static s32 result;
		const u32 func_address = 0x00476C90;
		u32 iscript_anim_xd = iscript_anim;
		__asm {
			PUSHAD
			PUSH iscript_anim_xd
			MOV EDI, flag_or_timer
			MOVZX EBX, weapon_id
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool place_building_resource_restriction_check(u32 check_vision, u16 tile_x, u16 tile_y) {
		static bool result;
		const u32 func_address = 0x00473990;
		__asm {
			PUSHAD
			MOVZX ECX, tile_y
			MOVZX EDX, tile_x
			MOV EAX, check_vision
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Sprite* create_sprite(u32 sprite_id, u32 x, u32 y, u32 player) {
		static Sprite* result;
		const u32 func_address = 0x004990F0;
		__asm {
			PUSHAD
			PUSH player
			PUSH x
			MOV EDI, y
			PUSH sprite_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void refresh_all_visible_images_at_screen_position(Sprite* sprite, u8 vision_filter) {
		const u32 func_address = 0x00497480;
		u32 vision_filter_xd = vision_filter;
		__asm {
			PUSHAD
			PUSH vision_filter_xd
			MOV EBX, sprite
			CALL func_address
			POPAD
		}
	}

	void set_tooltip_info(u32 mineral_cost, u32 vespene_cost, u32 p_energy, u32 t_energy, u32 z_energy, u32 p_supply, u32 t_supply, u16 z_supply) {
		const u32 func_address = 0x00458800;
		u32 z_supply_xd = z_supply;
		__asm {
			PUSHAD
			MOV EAX, mineral_cost
			MOV ECX, vespene_cost
			PUSH z_supply_xd
			PUSH t_supply
			PUSH p_supply
			PUSH z_energy
			PUSH t_energy
			MOV EDX, p_energy
			CALL func_address
			POPAD
		}
	}

	void bw_print_icon(Bounds* bounds, FxnDrawIconData* data, int pos_x, int pos_y) {
		const u32 func_address = 0x0041D090;
		__asm {
			PUSHAD
			PUSH pos_y
			PUSH pos_x
			MOV EDI, data
			MOV EAX, bounds
			CALL func_address
			POPAD
		}
	}

	void bw_print_xy(char* text, s32 pos_x, s32 pos_y) {
		const u32 func_address = 0x004202B0;
		__asm {
			PUSHAD
			PUSH pos_y
			MOV ESI, pos_x
			MOV EAX, text
			CALL func_address
			POPAD
		}
	}

	bool unit_is_in_notify_range(Unit* unit) {
		static bool result;
		const u32 func_address = 0x004E5AD0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	u32 reset_all_unit_attack_notify_timers_in_range(Unit* unit) {
		static u32 result;
		const u32 func_address = 0x0048EAE0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void do_unit_event_notification(Unit* unit, char event_type, u32 timer, bool ping, u32 message_id) {
		const u32 func_address = 0x0045EFE0;
		b32 ping_xd = ping;
		__asm {
			PUSHAD
			PUSH message_id
			PUSH ping_xd
			MOV EDI, timer
			MOVZX EBX, event_type
			MOV EAX, unit
			call func_address
			POPAD
		}
	}

	s32 stat_data_unit_wireframe_select_update(BinDlg* dialog, Event* event_arg) {
		static s32 result;
		const u32 func_address = 0x00456F50;
		__asm {
			PUSHAD
			MOV EDX, event_arg
			MOV ECX, dialog
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char status_screen_update_tooltip_mouse_held(BinDlg* dialog, Event* dialog_event) {
		static char result;
		const u32 func_address = 0x00457DE0;
		__asm {
			PUSHAD
			PUSH dialog_event
			MOV ESI, dialog
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	u32 status_screen_button(BinDlg* dialog_1, BinDlg* dialog_2) {
		static u32 result;
		const u32 func_address = 0x00458220;
		__asm {
			PUSHAD
			MOV EDX, dialog_2
			MOV ECX, dialog_1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void status_screen_draw_stat_button_border(BinDlg* dialog) {
		const u32 func_address = 0x00456D30;
		__asm {
			PUSHAD
			MOV EDI, dialog
			CALL func_address
			POPAD
		}
	}

	u32 status_screen_wireframe_coloring_related(Unit* unit) {
		static u32 result;
		const u32 func_address = 0x004567C0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char status_screen_wireframe_shield_related(u32 unused, Unit* unit) {
		static char result;
		const u32 func_address = 0x004566B0;
		__asm {
			PUSHAD
			MOV ECX, unused
			MOV EDX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	u32 status_screen_wireframe_hp_related(Unit* unit) {
		static u32 result;
		const u32 func_address = 0x00456730;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char toggle_unit_pathing_properties(Unit* unit) {
		static char result;
		const u32 func_address = 0x004030D0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	u32 is_unit_at_height(Unit* unit, u32 height) {
		static u32 result;
		const u32 func_address = 0x0045F8D0;
		__asm {
			PUSHAD
			MOV ESI, height
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool kill_units_at_location_proc(Unit* unit, KillUnitArgs* args) {
		static bool result;
		const u32 func_address = 0x004C7E20;
		__asm {
			PUSHAD
			MOV EDX, args
			MOV ECX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void kill_powerup(Unit* unit) {
		const u32 func_address = 0x004F3C60;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	void send_message_something(u32 unused, const char* string) {
		const u32 func_address = 0x004F31D0;
		__asm {
			PUSHAD
			MOV EDX, string
			MOV ECX, unused
			CALL func_address
			POPAD
		}
	}

	u32 set_ingame_input_procs() {
		static u32 result;
		const u32 func_address = 0x00484CC0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void update_current_buttonset() {
		const u32 func_address = 0x004599A0;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	bool is_unit_owned_by_player_ex(u32 player, Unit* unit) {
		static bool result;
		const u32 func_address = 0x0045FEF0;
		__asm {
			PUSHAD
			PUSH unit
			MOV ECX, player
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	char issue_place_building_order(char* unused, u32 unit_id, char order_id) {
		static char result;
		const u32 func_address = 0x0048E730;
		u32 order_id_xd = order_id;
		__asm {
			PUSHAD
			PUSH order_id_xd
			MOV ESI, unit_id
			MOV EDI, unused
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool compare_unit_rank(Unit* unit, Unit* compare_unit) {
		static bool result;
		const u32 func_address = 0x0049A350;
		__asm {
			PUSHAD
			MOV ESI, compare_unit
			MOV EDI, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void do_targeted_order(u32 x, u32 y, Unit* target, u32 fow_unit_id) {
		const u32 func_address = 0x0046F5B0;
		__asm {
			PUSHAD
			PUSH fow_unit_id
			PUSH target
			PUSH y
			PUSH x
			CALL func_address
			POPAD
		}
	}

	void minimap_game_target_order(Event* event_arg) {
		const u32 func_address = 0x004A55F0;
		__asm {
			PUSHAD
			MOV EAX, event_arg
			CALL func_address
			POPAD
		}
	}

	char input_target_order_left_mouse_click(Event* event_arg) {
		static char result;
		const u32 func_address = 0x004BD500;
		__asm {
			PUSHAD
			MOV ECX, event_arg
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	char input_place_building_left_mouse_click(Event* event_arg, u32 unused, Point16 position) {
		static char result;
		const u32 func_address = 0x0048E5D0;
		__asm {
			PUSHAD
			MOV ECX, event_arg
			MOV EDI, unused
			MOV ESI, position
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void initialize_input_procs() {
		const u32 func_address = 0x004D2FF0;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	u16 get_minimap_cursor_pos(u16* x, u16* y) {
		const u32 func_address = 0x004A3D70;
		static u16 result;
		__asm {
			PUSHAD
			MOV EAX, x
			MOV ECX, y
			CALL func_address
			MOV result, AX
			POPAD
		}
	}

	void update_palette() {
		const u32 func_address = 0x0041E5C0;
		__asm {
			PUSHAD
			MOV EDI, 1
			MOV ESI, 1
			CALL func_address
			POPAD
		}
	}

	u32 order_acquire_generic_idle_unit_proc(Unit* unit, u32 x_pos, u16 y_pos, Unit** target, u16 fow_unit_id) {
		static u32 result;
		static u32 fow_unit_id_xd = fow_unit_id;
		static u32 y_pos_xd = y_pos;
		const u32 func_address = 0x00455900;
		__asm {
			PUSHAD
			PUSH fow_unit_id_xd
			PUSH target
			PUSH y_pos_xd
			MOV EDX, x_pos
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 order_acquire_generic_right_click_harvest_proc(Unit* node, Unit* worker) {
		static u32 result;
		const u32 func_address = 0x004557C0;
		__asm {
			PUSHAD
			MOV EDI, node
			MOV ESI, worker
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char get_generic_unit_idle_order(u32 order_unit_id, Unit* unit) {
		static char result;
		const u32 func_address = 0x00455760;
		__asm {
			PUSHAD
			MOV EDX, unit
			MOV ECX, order_unit_id
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* iterate_units_at_location_target_proc_in_point(Box16* location, void* func, Unit* target) {
		static Unit* result;
		const u32 is_unit_in_point_function = 0x004E8280;
		__asm {
			PUSHAD
			PUSH target
			MOV EBX, func
			MOV EAX, location
			CALL is_unit_in_point_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void set_rally_unit(Unit* factory, Unit* target) {
		const u32 func_address = 0x00466B40;
		__asm {
			PUSHAD
			MOV ECX, target
			MOV EAX, factory
			CALL func_address
			POPAD
		}
	}

	void set_rally_position(Unit* factory, u16 x_pos, u16 y_pos) {
		const u32 func_address = 0x00466910;
		u32 x_pos_xd = x_pos;
		u32 y_pos_xd = y_pos;
		__asm {
			PUSHAD
			PUSH y_pos_xd
			PUSH x_pos_xd
			MOV EAX, factory
			CALL func_address
			POPAD
		}
	}

	s32 is_outside_of_game_screen(u32 current_x, u32 current_y) {
		static s32 result;
		const u32 func_address = 0x004D1140;
		__asm {
			PUSHAD
			MOV ECX, current_x
			MOV EAX, current_y
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool accepts_right_click_commands(Unit* unit) {
		static bool result;
		const u32 func_address = 0x004556D0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* find_unit_at_point(u32 x_pos, u32 y_pos) {
		static Unit* result;
		const u32 func_address = 0x0046F3A0;
		__asm {
			PUSHAD
			PUSH y_pos
			PUSH x_pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 can_move(Unit* unit) {
		static s32 result;
		const u32 func_address = 0x00455660;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Thingy* show_command_response(u32 x_pos, u16 y_pos, Unit* unit) {
		static Thingy* result;
		const u32 func_address = 0x00456490;
		__asm {
			PUSHAD
			PUSH unit
			MOV DI, y_pos
			MOV EAX, x_pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char cmdact_right_click_order(Unit* target, u32 x_pos, u32 y_pos, u32 unit_id, u32 keycode_used) {
		static char result;
		const u32 func_address = 0x004C0380;
		__asm {
			PUSHAD
			PUSH keycode_used
			PUSH unit_id
			PUSH y_pos
			PUSH x_pos
			MOV ESI, target
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	s32 show_right_click_error_if_needed(Unit* target) {
		static s32 result;
		const u32 func_address = 0x00455A00;
		__asm {
			PUSHAD
			PUSH target
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void play_yes_unit_response(Unit* unit) {
		const u32 func_address = 0x0048F4D0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	u32 sort_all_units(Unit** units, Unit** units_2, Unit* unit) {
		static u32 result;
		const u32 func_address = 0x0046F0F0;
		__asm {
			PUSHAD
			PUSH unit
			PUSH units_2
			PUSH units
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char tech_is_researched_bw(u32 player, u16 tech_id) {
		static char result;
		const u32 func_address = 0x004CE8A0;
		__asm {
			PUSHAD
			MOVZX ECX, tech_id
			MOV EAX, player
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void drone_build_execute(Unit* unit) {
		const u32 func_address = 0x0045D2E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	Unit* find_unit_at_tile(u16 y_pos, u16 x_pos, u32 order_id) {
		static Unit* result;
		const u32 func_address = 0x004734B0;
		__asm {
			PUSHAD
			PUSH order_id
			MOVZX ECX, x_pos 
			MOVZX EAX, y_pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void inherit_gas_overlays(Sprite* sprite) {
		const u32 func_address = 0x004D5BB0;
		__asm {
			PUSHAD
			MOV EDI, sprite
			CALL func_address
			POPAD
		}
	}

	// For create_all_selection_circles
	Image* create_selection_circle(Sprite* sprite, u8 color, u16 image_id) {
		static Image* result;
		const u32 func_address = 0x004D7070;
		u32 color_xd = color;
		u32 image_id_xd = image_id;
		__asm {
			PUSHAD
			PUSH image_id_xd
			PUSH color_xd
			MOV EAX, sprite
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* transport_status_update_drawn_values() {
		static Unit* result;
		const u32 func_address = 0x00424FC0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void set_spell_special_button_graphic(BinDlg* dialog) {
		const u32 func_address = 0x00424BA0;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	void get_active_portrait_unit_name(BinDlg* dialog) {
		const u32 func_address = 0x00425EC0;
		__asm {
			PUSHAD
			PUSH dialog
			CALL func_address
			POPAD
		}
	}

	void unit_stat_action_building(void* dialog) {
		const u32 func_address = 0x00427890;
		__asm {
			PUSHAD
			MOV ECX, dialog
			CALL func_address
			POPAD
		}
	}

	void kill_all_loaded_units(Unit* unit) {
		const u32 func_address = 0x0049FDD0;
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL func_address
			POPAD
		}
	}

	// For trigact_transmission
	s32 trigact_play_wav(TriggerAction* action, char* unused) {
		static s32 result;
		const u32 func_address = 0x004C77D0;
		__asm {
			PUSHAD
			MOV EDI, unused
			MOV ECX, action
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* get_doodad_for_unit_state(u8 location, u16 unit_id, u32 player) {
		static Unit* result;
		const u32 func_address = 0x004C7380;
		u32 unit_id_xd = unit_id;
		__asm {
			PUSHAD
			PUSH player
			PUSH unit_id_xd
			MOVZX EAX, location
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8 minimap_ping(u32 x_pos, u32 y_pos, u32 color) {
		static u8 result;
		const u32 func_address = 0x004A34C0;
		__asm {
			PUSHAD
			PUSH color
			PUSH y_pos
			PUSH x_pos
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	s32 game_talking_portrait(u32 x_pos, u32 time, u32 unit_id, u32 y_pos) {
		static s32 result;
		const u32 func_address = 0x0045EDD0;
		__asm {
			PUSHAD
			PUSH y_pos
			PUSH unit_id
			MOV ESI, time
			MOV EDI, x_pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void create_text_message_with_timer(const char* string, u32 timer) {
		const u32 func_address = 0x0048CF20;
		__asm {
			PUSHAD
			PUSH timer
			MOV ESI, string
			CALL func_address
			POPAD
		}
	}

	// For select_unit_initial
	void remove_selection_circle_image(Image* image) {
		const u32 func_address = 0x004D4FA0;
		__asm {
			PUSHAD
			MOV ESI, image
			CALL func_address
			POPAD
		}
	}

	char illegal_team_check(Unit* unit) {
		static char result;
		const u32 func_address = 0x00485710;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For ai_update_guard_needs
	void delete_guard_ai(GuardAi* guard_ai, GuardAiList* guard_ai_list) {
		const u32 func_address = 0x00403030;
		__asm {
			PUSHAD
			MOV EDX, guard_ai_list
			MOV EAX, guard_ai
			CALL func_address
			POPAD
		}
	}

	u32 ai_request_guard(GuardAi* guard_ai, u32 player) {
		static u32 result;
		const u32 func_address = 0x00448630;
		__asm {
			PUSHAD
			MOV ECX, player
			MOV EAX, guard_ai
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* find_nearest_available_military(u32 player, u32 unit_id, Point16 point) {
		static Unit* result;
		const u32 func_address = 0x004379B0;
		u32 x_pos = point.x;
		u32 y_pos = point.y;
		__asm {
			PUSHAD
			PUSH x_pos
			PUSH unit_id
			MOV EBX, y_pos
			MOV ECX, player
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void create_bunker_attack_overlay(Unit* unit) {
		const u32 func_address = 0x00477FD0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	u32 draw_unit_box(u32 x_pos, u32 y_pos, u8 x_dims, u8 y_dims, char color, char is_building) {
		static u32 result;
		const u32 func_address = 0x004A3FD0;
		s32 is_building_xd = is_building;
		__asm {
			PUSHAD
			PUSH is_building_xd
			PUSH y_dims
			PUSH x_dims
			PUSH y_pos
			MOVZX ECX, color
			MOV EAX, x_pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void draw_minimap_unit_box_2(u32 player) {
		const u32 func_address = 0x004A47B0;
		__asm {
			PUSHAD
			MOV ECX, player
			CALL func_address
			POPAD
		}
	}

	char draw_minimap_unit_box(u32 player) {
		static char result;
		const u32 func_address = 0x004A4650;
		__asm {
			PUSHAD
			MOV ECX, player
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void draw_all_minimap_unit_boxes(u32 player) {
		const u32 func_address = 0x004A48E0;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL func_address
			POPAD
		}
	}

	u32 thingy_visiblity_check(u32 some_bool, Thingy* thingy) {
		static u32 result;
		const u32 func_address = 0x00487CF0;
		__asm {
			PUSHAD
			MOV ECX, thingy
			MOV EAX, some_bool
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u16 get_remaining_build_time_percentage(Unit* unit) {
		static u16 result;
		const u32 func_address = 0x004669E0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV result, AX
			POPAD
		}
		return result;
	}

	s32 get_sfx_volume_from_screen_distance(u32 x, u32 y) {
		static s32 result;
		const u32 func_address = 0x0048E8D0;
		__asm {
			PUSHAD
			MOV ECX, x
			MOV EAX, y
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_sfx_pan_from_screen_distance(u32 x) {
		static s32 result;
		const u32 func_address = 0x0048E850;
		__asm {
			PUSHAD
			MOV EAX, x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void syswarn_file_not_found(u32 code, const char* filename) {
		const u32 func_address = 0x004212C0;
		__asm {
			MOV EBX, code
			PUSH filename
			CALL func_address
		}
	}

	void EAX_order(Unit* unit, u32 order_func_offset) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL order_func_offset
			POPAD
		}
	}

	void EBX_order(Unit* unit, u32 order_func_offset) {
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL order_func_offset
			POPAD
		}
	}

	void ECX_order(Unit* unit, u32 order_func_offset) {
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL order_func_offset
			POPAD
		}
	}

	void EDI_order(Unit* unit, u32 order_func_offset) {
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL order_func_offset
			POPAD
		}
	}

	void ESI_order(Unit* unit, u32 order_func_offset) {
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL order_func_offset
			POPAD
		}
	}

	void PUSH_order(Unit* unit, u32 order_func_offset) {
		__asm {
			PUSHAD
			PUSH unit
			CALL order_func_offset
			POPAD
		}
	}

	bool is_better_target(Unit* self, Unit* auto_target, Unit* compare_unit) {
		static u32 result;
		const u32 func_address = 0x004769D0;
		__asm {
			PUSHAD
			MOV EDI, compare_unit
			MOV EAX, auto_target
			MOV ECX, self
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void react_to_hit(Unit* attacker, Unit* victim) {
		const u32 func_address = 0x004776C0;
		__asm {
			PUSHAD
			MOV ECX, victim
			MOV EAX, attacker
			CALL func_address
			POPAD
		}
	}

	char ask_for_help(Unit* attacker, Unit* helper) {
		static u32 result;
		const u32 func_address = 0x00476E60;
		__asm {
			PUSHAD
			MOV ESI, helper
			MOV EAX, attacker
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	s32 play_loaded_sound(u32 loaded_sound, s32 volume, s32 pan, u32 sound_id, u32 unit_id, u32 sound_flags, u32 some_bool, u32 priority) {
		static s32 result;
		const u32 func_address = 0x004BBAF0;
		__asm {
			PUSHAD
			PUSH priority
			PUSH some_bool
			PUSH sound_flags
			PUSH unit_id
			PUSH sound_id
			PUSH pan
			PUSH volume
			MOV EBX, loaded_sound
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 queue_sound(u32 sound_id, s32 volume, s32 pan, u32 unit_id) {
		static s32 result;
		const u32 func_address = 0x004BBEB0;
		__asm {
			PUSHAD
			PUSH unit_id
			PUSH pan
			PUSH volume
			MOV ESI, sound_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_finished_summon(Unit* unit) {
		static u32 result;
		const u32 func_address = 0x00436E40;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	u32 get_total_hp_plus_shields(Unit* unit) {
		static u32 result;
		const u32 func_address = 0x00402710;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_is_morphing(Unit* unit) {
		static u32 result;
		const u32 func_address = 0x0045CD00;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	char cmdact_unload(Unit* unit) {
		const u32 func_address = 0x004BFDB0;
		static u8 result;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Target ai_prepare_to_flee(Unit* target, Unit* attacker) {
		static Target result;
		if (!AssertSprite(target->sprite)) { // Protecting against nullsprite crashes! -Pr0nogo
			result.pt = attacker->position;
			result.unit = 0;
			return result;
		}
		const u32 func_address = 0x00476A50;
		__asm {
			PUSHAD
			PUSH target
			MOV EAX, attacker
			CALL func_address
			MOV dword ptr [offset result.pt], EAX
			MOV dword ptr [offset result.unit], EDX
			POPAD
		}
		return result;
	}

	// LOCAL
	bool unit_is_enemy(Unit* other) {
		static u32 result;
		const u32 func_address = 0x004D1380;
		__asm {
			PUSHAD
			MOV EAX, other
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	u32 can_target_order_on_fow_unit(Unit* self, char order_id, u16 fow_unit_id, s32* error) {
		static u32 result;
		const u32 func_address = 0x004746D0;
		__asm {
			PUSHAD
			MOV EDI, error
			MOVZX ECX, fow_unit_id
			MOVZX EDX, order_id
			MOV EAX, self
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_can_attack_ground_type(u16 fow_unit_id, Unit* self) {
		static u32 result;
		const u32 func_address = 0x00475C80;
		__asm {
			PUSHAD
			MOV ECX, self
			MOVZX EAX, fow_unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	u16 get_targeting_error_string(u32 weapon_id, s32* error, Unit* target) {
		static u32 result;
		const u32 func_address = 0x0046EFE0;
		__asm {
			PUSHAD
			MOV ECX, target
			MOV EDX, error
			MOV EAX, weapon_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	char cmdact_target_order(u8 order_id, Unit* target, u16 x_pos, u16 y_pos, u16 fow_unit_id, char keycode_used) {
		static u32 result;
		const u32 func_address = 0x004C0300;
		u32 x_pos_xd = x_pos;
		u32 y_pos_xd = y_pos;
		u32 fow_unit_id_xd = fow_unit_id;
		s32 keycode_used_xd = keycode_used;
		__asm {
			PUSHAD
			PUSH keycode_used_xd
			PUSH fow_unit_id_xd
			PUSH y_pos_xd
			PUSH x_pos_xd
			MOV ESI, target
			MOVZX EDX, order_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	// Inverted args for sanity
	void show_cursor_marker(u16 x_pos, u16 y_pos) {
		const u32 func_address = 0x00488660;
		__asm {
			PUSHAD
			MOVZX ECX, x_pos
			MOVZX EAX, y_pos
			CALL func_address
			POPAD
		}
	}

	// Note: buffer_size may be max number of characters * 2, possibly for 16bits text support
	void bw_sprintf_s(char* buffer, int buffer_size, const char* format_string, int value) {
		const u32 func_address = 0x0041F1B0;
		__asm {
			PUSHAD
			PUSH value
			PUSH format_string
			MOV ESI, buffer_size
			MOV EDI, buffer
			CALL func_address
			ADD ESP, 0x08
			POPAD
		}
	}

	Image* set_image_palette_type(Image* out_image, u8 palette_type) {
		static Image* result;
		const u32 func_address = 0x004D58B0;
		__asm {
			PUSHAD
			MOV EAX, out_image
			MOVZX EBX, palette_type
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u16 is_valid_script(Image* image, u32 iscript_id) {
		static u32 result;
		const u32 func_address = 0x004D6640;
		__asm {
			PUSHAD
			MOV EDI, iscript_id
			MOV EBX, image
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	Image* update_image_frame_index(Image* image) {
		static Image* result;
		const u32 func_address = 0x004D5940;
		__asm {
			PUSHAD
			MOV EAX, image
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_path_distance_to(Unit* unit, Point16 position) {
		static u32 result;
		const u32 func_address = 0x0042FA30;
		u32 x = position.x;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOVZX EAX, position.y
			PUSH x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void fire_unit_weapon(Unit* unit, u8 weapon_id) {
		const u32 func_address = 0x00479C90;
		u32 weapon_id_xd = weapon_id;
		__asm {
			PUSHAD
			PUSH weapon_id_xd
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	Thingy* ufl_unstable_randomize(Flingy* flingy) {
		static Thingy* result;
		const u32 func_address = 0x004955F0;
		__asm {
			PUSHAD
			MOV EAX, flingy
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void do_weapon_damage(u32 damage, Unit* target, u8 weapon_id, u32 divisor, char direction, Unit* attacker, u32 attacking_player) {
		const u32 func_address = 0x00479930;
		u32 weapon_id_xd = weapon_id;
		u32 direction_xd = direction;
		__asm {
			PUSHAD
			PUSH attacking_player
			PUSH attacker
			PUSH direction_xd
			PUSH divisor
			PUSH weapon_id_xd
			MOV EDI, target
			MOV EAX, damage
			CALL func_address
			POPAD
		}
	}

	Image* iscript_create_image(Image* relative_image, u32 image_id, char x, char y, u8 draw_above) {
		static Image* result;
		const u32 func_address = 0x004D6D90;
		s32 x_xd = x;
		s32 y_xd = y;
		u32 draw_above_xd = draw_above;
		__asm {
			PUSHAD
			PUSH draw_above_xd
			PUSH y_xd
			PUSH x_xd
			PUSH image_id
			PUSH relative_image
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 iscript_use_lo_file(void* out_coords, Image* image, u8 a3, u8 frame_offset) {
		static u32 result;
		const u32 func_address = 0x004D5770;
		__asm {
			PUSHAD
			MOVZX EDX, frame_offset
			MOVZX ECX, a3
			MOV EDX, image
			MOV EAX, out_coords
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void hide_image(Image* image) {
		const u32 func_address = 0x00401100;
		__asm {
			PUSHAD
			MOV ESI, image
			CALL func_address
			POPAD
		}
	}

	u16 show_image(Image* image) {
		static u32 result;
		const u32 func_address = 0x00401120;
		__asm {
			PUSHAD
			MOV ECX, image
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	Image* image_destructor(Image* image) {
		static Image* result;
		const u32 func_address = 0x004D4CE0;
		__asm {
			PUSHAD
			MOV ESI, image
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void flip_image(Image* image, char direction) {
		const u32 func_address = 0x004D5F30;
		__asm {
			PUSHAD
			MOV EAX, image
			MOVZX ECX, direction
			CALL func_address
			POPAD
		}
	}

	void iscript_play_sound(Image* image, u32 sound_id) {
		const u32 func_address = 0x004D6610;
		__asm {
			PUSHAD
			MOV EBX, sound_id
			MOV EAX, image
			CALL func_address
			POPAD
		}
	}

	void turn_unit_left(Unit* unit, char amount) {
		const u32 func_address = 0x00496170;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOVZX ECX, amount
			CALL func_address
			POPAD
		}
	}

	u32 get_difference_between_points(char point_1, char point_2) {
		static u32 result;
		const u32 func_address = 0x00494BD0;
		__asm {
			PUSHAD
			MOVZX EAX, point_1
			MOVZX ECX, point_2
			CALL func_address
			MOV result, EAX
			POPAD
		}
	}

	u32 get_modified_unit_speed(u32 speed, Unit* unit) {
		static u32 result;
		const u32 func_address = 0x0047B5F0;
		__asm {
			PUSHAD
			MOV EAX, speed
			MOV EDX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Image* get_empty_image() {
		static Image* result;
		const u32 func_address = 0x004D4E30;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Image* images_insert(Image* image, Image* next, Image* prev) {
		static Image* result;
		const u32 func_address = 0x0047A070;
		__asm {
			PUSHAD
			MOV EAX, image
			MOV EDX, next
			MOV ECX, prev
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void some_play_image_crap_that_crashes(char x_pos, char y_pos, Image* image, Sprite* sprite, u32 image_id) {
		const u32 func_address = 0x004010E0;
		__asm {
			PUSHAD
			PUSH image_id
			MOVZX ECX, x_pos
			MOVZX EAX, y_pos
			MOV EBX, image
			MOV EDI, sprite
			CALL func_address
			POPAD
		}
	}

	u32 get_closest_point_of_target(Unit* unit, u32 out_x, u32 out_y) {
		static u32 result;
		const u32 func_address = 0x004762C0;
		__asm {
			PUSHAD
			PUSH out_y
			PUSH out_x
			PUSH unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_has_path_to_destination(Unit* unit, s32 pos_x, s32 pos_y) {
		static u32 result;
		const u32 func_address = 0x0049CB60;
		__asm {
			PUSHAD
			PUSH pos_y
			PUSH pos_x
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	Unit** get_all_units_in_region(u16 region_id) {
		static Unit** result;
		const u32 func_address = 0x00430B50;
		u32 region_id_xd = region_id;
		__asm {
			PUSHAD
			PUSH region_id_xd
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void subtract_queue_cost(u8 player) {
		const u32 func_address = 0x0042CE40;
		__asm {
			PUSHAD
			MOVZX EAX, player
			CALL func_address
			POPAD
		}
	}

	Unit* get_unit_powerup(Unit* unit) {
		static Unit* result;
		const u32 func_address = 0x004014E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void droleth_build_excisant(Unit* unit) {
		const u32 func_address = 0x0045D790;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void cancel_queued_trains(AiTown* town) {
		const u32 func_address = 0x004326A0;
		__asm {
			PUSHAD
			PUSH town
			CALL func_address
			POPAD
		}
	}

	u32 get_upgrade_count(AiTown* town, u32 upgrade_id) {
		static u32 result;
		const u32 func_address = 0x004325A0;
		__asm {
			PUSHAD
			MOV EAX, town
			MOV EDI, upgrade_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_tech_count(AiTown* town, u32 tech_id) {
		static u32 result;
		const u32 func_address = 0x00432530;
		__asm {
			PUSHAD
			MOV EAX, town
			MOV ESI, tech_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
	}

	u16 ai_build_bunker(AiTown* town, u32 player) {
		static u32 result;
		const u32 func_address = 0x0043EBB0;
		__asm {
			PUSHAD
			PUSH player
			PUSH town
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	u32 ai_build_turret(AiTown* town, u32 player) {
		static u32 result;
		const u32 func_address = 0x0043AAE0;
		__asm {
			PUSHAD
			PUSH player
			PUSH town
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_bunker_position(u32 player, Point16* return_pos) {
		static s32 result;
		const u32 func_address = 0x004376F0;
		__asm {
			PUSHAD
			MOV EBX, player
			MOV ESI, return_pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void do_area_fixups_for_enemy(u32 player, u16 region_id, u32 iterations) {
		const u32 func_address = 0x00444EB0;
		u32 region_id_xd = region_id;
		__asm {
			PUSHAD
			PUSH iterations
			PUSH region_id_xd
			PUSH player
			CALL func_address
			POPAD
		}
	}

	s32 ai_manage_building_placement(Unit* builder, u32 unit_id, Point16 try_pos, Point16* out_pos, s32 area_tiles) {
		static s32 result;
		const u32 func_address = 0x004465C0;
		__asm {
			PUSHAD
			PUSH area_tiles
			PUSH out_pos
			PUSH try_pos
			PUSH unit_id
			MOV ECX, builder
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_finish_unit(Unit* unit) {
		static u32 result;
		const u32 func_address = 0x00433640;
		__asm {
			PUSHAD
			MOV EDi, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_turret_position(u32 player, Point16* out_pos) {
		static s32 result;
		const u32 func_address = 0x00437670;
		__asm {
			PUSHAD
			MOV EBX, player
			MOV ESI, out_pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void get_secondary_hachirosk_build_location(u32 resarea, u16* out_x, u16* out_y) {
		const u32 func_address = 0x00444630;
		__asm {
			PUSHAD
			PUSH out_y
			PUSH out_x
			PUSH resarea
			CALL func_address
			POPAD
		}
	}

	char update_buttonset() {
		static char result;
		const u32 func_address = 0x00458DE0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void refresh_image(Image* image) {
		const u32 func_address = 0x004970A0;
		__asm {
			PUSHAD
			MOV EAX, image
			CALL func_address
			POPAD
		}
	}

	void compile_healthbar(Image* image, Sprite* sprite) {
		const u32 func_address = 0x004D6010;
		__asm {
			PUSHAD
			PUSH sprite
			MOV ESI, image
			CALL func_address
			POPAD
		}
	}

	void create_selection_circle_image(Image* image, Sprite* sprite, u8 color, u16 image_id) {
		const u32 func_address = 0x004D6810;
		__asm {
			PUSHAD
			PUSH image_id
			PUSH color
			MOV EAX, sprite
			MOV ECX, image
			CALL func_address
			POPAD
		}
	}

	b32 return_interceptor_if_cannot_attack(Unit* steward) {
		static b32 result;
		const u32 func_address = 0x00465810;
		__asm {
			PUSHAD
			MOV EAX, steward
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_next_strafe_position(Unit* unit, u8 strafe_type, Point16 pos) {
		static u32 result;
		const u32 func_address = 0x00465D30;
		__asm {
			PUSHAD
			PUSH pos
			MOVZX CL, strafe_type
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void get_weapon_iscript_init(Unit* unit, IscriptAnimation::Enum animation) {
		const u32 func_address = 0x00476ED0;
		__asm {
			PUSHAD
			MOVZX DL, animation
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	b32 set_temp_iscript_animation(IscriptAnimation::Enum animation) {
		static b32 result;
		const u32 func_address = 0x0042D600;
		__asm {
			PUSHAD
			MOVZX AL, animation
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_visibility_mask_from_position_and_size(u32 x_pos, u32 y_pos, u32 width, u32 height) {
		static u32 result;
		const u32 func_address = 0x00402570;
		__asm {
			PUSHAD
			MOV EDX, height
			PUSH width
			PUSH y_pos
			PUSH x_pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Image* copy_image_palette_type(Image* image, Image* from) {
		static Image* result;
		const u32 func_address = 0x004D5E40;
		__asm {
			PUSHAD
			MOV EAX, image
			MOV ESI, from
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool order_hangar_unit(Unit* unit, u32 range1, u32 range2) {
		static b32 result;
		const u32 Func_Sub465A60 = 0x00465A60;
		__asm {
			PUSHAD
			PUSH range1
			PUSH range2
			MOV EAX, unit
			CALL Func_Sub465A60
			MOV result, EAX
			POPAD
		}
		return (result != 0);
	}

	b32 can_draw_placement_rect_maybe(u16 unit_id, Unit* unit) {
		static b32 result;
		const u32 func_address = 0x00463A10;
		__asm {
			PUSHAD
			PUSH unit
			MOVZX EBX, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Image* prepare_placement_preview() {
		static Image* result;
		const u32 func_address = 0x004D5B60;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void place_building_normal_proc() {
		const u32 func_address = 0x0048E490;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void place_building_refinery_proc() {
		const u32 func_address = 0x0048E430;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	u32 prepare_placement_box(b32 is_addon, u32 unit_id) {
		static u32 result;
		static u32 func_address = 0x0048DAF0;
		__asm {
			PUSHAD
			PUSH unit_id
			PUSH is_addon
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char refresh_screen(u32 placement_box) {
		static char result;
		const u32 func_address = 0x0048DDC0;
		__asm {
			PUSHAD
			MOV ECX, placement_box
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void update_unit_damage_overlay(Unit* unit) {
		const u32 func_address = 0x004E6090;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	s32 get_recv_string(u32 string_id, u32 data_len, u32 buffer, char* string, s32 string_type) {
		static s32 result;
		const u32 func_address = 0x00485980;
		__asm {
			PUSHAD
			PUSH string_type
			PUSH string
			MOV EBX, buffer
			MOV ECX, data_len
			MOV EAX, string_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void CMDRECV_save_game(char* buffer) {
		const u32 func_address = 0x004C2910;
		__asm {
			PUSHAD
			MOV EAX, buffer
			CALL func_address
			POPAD
		}
	}

	FILE* CMDRECV_load_game(char* buffer) {
		static FILE* result;
		const u32 func_address = 0x004CF950;
		__asm {
			PUSHAD
			MOV EBX, buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s8 CMDRECV_restart_game() {
		static u8 result;
		const u32 func_address = 0x004BFB10;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void CMDRECV_select(char* buffer) {
		const u32 func_address = 0x004C2750;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			POPAD
		}
	}

	void CMDRECV_shift_select(char* buffer) {
		const u32 func_address = 0x004C2560;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			POPAD
		}
	}

	void CMDRECV_shift_deselect(char* buffer) {
		const u32 func_address = 0x004BFB40;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			POPAD
		}
	}

	u16 CMDRECV_build(s8* buffer) {
		static u16 result;
		const u32 func_address = 0x004C23C0;
		__asm {
			PUSHAD
			MOV ESI, buffer
			CALL func_address
			MOV result, AX
			POPAD
		}
		return result;
	}

	s8* CMDRECV_vision(s8* buffer) {
		static s8* result;
		const u32 func_address = 0x004BF9C0;
		__asm {
			PUSHAD
			MOV EAX, buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void CMDRECV_ally(char* buffer) {
		const u32 func_address = 0x004C28A0;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			POPAD
		}
	}

	void CMDRECV_pause_game() {
		const u32 func_address = 0x004C0BC0;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void CMDRECV_resume_game() {
		const u32 func_address = 0x004C0B00;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	s32 CMDRECV_use_cheat(char* buffer, char* cheat) {
		static s32 result;
		const u32 func_address = 0x004C0AD0;
		__asm {
			PUSHAD
			MOV EDI, cheat
			MOV EDX, buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8 CMDRECV_hotkey(char* buffer) {
		static u8 result;
		const u32 func_address = 0x004C2870;
		__asm {
			PUSHAD
			MOV ECX, buffer
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_right_click(Unit* unit) {
		static Unit* result;
		const u32 func_address = 0x004C2370;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void CMDRECV_target_command(char* buffer) {
		const u32 func_address = 0x004C2320;
		__asm {
			PUSHAD
			MOV EAX, buffer
			CALL func_address
			POPAD
		}
	}

	char CMDRECV_cancel_construction() {
		static char result;
		const u32 func_address = 0x004C2EF0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_cancel_unit_morph() {
		static Unit* result;
		const u32 func_address = 0x004C2EC0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_stop(s8* buffer) {
		static Unit* result;
		const u32 func_address = 0x004C2190;
		__asm {
			PUSHAD
			MOV EDI, buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_carrier_stop() {
		static Unit* result;
		const u32 func_address = 0x004C1430;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_reaver_stop() {
		static Unit* result;
		const u32 func_address = 0x004C1240;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_nothing() {
		static Unit* result;
		const u32 func_address = 0x004C1050;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_return_cargo(char* buffer) {
		static Unit* result;
		const u32 func_address = 0x004C2040;
		__asm {
			PUSHAD
			MOV EDI, buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s16 CMDRECV_train(char* buffer) {
		static s16 result;
		const u32 func_address = 0x004C1C20;
		__asm {
			PUSHAD
			MOV EAX, buffer
			CALL func_address
			MOV result, AX
			POPAD
		}
		return result;
	}

	s16 CMDRECV_cancel_train(s8* buffer) {
		static s16 result;
		const u32 func_address = 0x004C0100;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			MOV result, AX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_cloak() {
		static Unit* result;
		const u32 func_address = 0x004C0720;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_decloak() {
		static Unit* result;
		const u32 func_address = 0x004C0660;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 CMDRECV_unit_morph(char* buffer) {
		static s32 result;
		const u32 func_address = 0x004C1990;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_unsiege(char* buffer) {
		static Unit* result;
		const u32 func_address = 0x004C1F10;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_siege(char* buffer) {
		static Unit* result;
		const u32 func_address = 0x004C1E80;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_train_fighter() {
		static Unit* result;
		const u32 func_address = 0x004C1800;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_unload_all(s8* buffer) {
		static Unit* result;
		const u32 func_address = 0x004C1CC0;
		__asm {
			PUSHAD
			MOV EDI, buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char CMDRECV_unload(Unit* unit) {
		static char result;
		const u32 func_address = 0x004C24F0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_merge_archon() {
		static Unit* result;
		const u32 func_address = 0x004C0E90;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_hold_position(char* buffer) {
		static Unit* result;
		const u32 func_address = 0x004C20C0;
		__asm {
			PUSHAD
			MOV EDI, buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_burrow(char* buffer) {
		static Unit* result;
		const u32 func_address = 0x004C1FA0;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_unburrow() {
		static Unit* result;
		const u32 func_address = 0x004C1AC0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_cancel_nuke() {
		static Unit* result;
		const u32 func_address = 0x004BFCD0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void CMDRECV_liftoff(s8* buffer) {
		const u32 func_address = 0x004C1620;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			POPAD
		}
	}

	Unit* CMDRECV_research(char* buffer) {
		static Unit* result;
		const u32 func_address = 0x004C1BA0;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_cancel_research() {
		static Unit* result;
		const u32 func_address = 0x004C0070;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_upgrade(char* buffer) {
		static Unit* result;
		const u32 func_address = 0x004C1B20;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void CMDRECV_cancel_upgrade() {
		const u32 func_address = 0x004BFFC0;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	Unit* CMDRECV_cancel_addon() {
		static Unit* result;
		const u32 func_address = 0x004BFF30;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char CMDRECV_building_morph(char* buffer) {
		static char result;
		const u32 func_address = 0x004C1910;
		__asm {
			PUSHAD
			MOV EAX, buffer
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_stim_pack() {
		static Unit* result;
		const u32 func_address = 0x004C2F30;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	b32 CMD_game_hash(HashPacket* packet) {
		static b32 result;
		const u32 func_address = 0x0047CDD0;
		__asm {
			PUSHAD
			MOV EDI, packet
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void CMDRECV_voice_enable(s8* buffer) {
		const u32 func_address = 0x004BF820;
		__asm {
			PUSHAD
			MOV EDX, buffer
			CALL func_address
			POPAD
		}
	}

	s32 CMDRECV_set_latency(char* buffer, s32 unused) {
		static s32 result;
		const u32 func_address = 0x00485E60;
		__asm {
			PUSHAD
			MOV ESI, unused
			MOV EAX, buffer
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 CMDRECV_set_replay_speed(s32 speed, u8 is_paused, u32 modifier) {
		static s32 result;
		const u32 func_address = 0x004DEB90;
		__asm {
			PUSHAD
			MOV ECX, modifier
			MOVZX EDX, is_paused
			MOV EAX, speed
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char CMDRECV_leave_game(char* buffer) {
		static char result;
		const u32 func_address = 0x004C2E90;
		__asm {
			PUSHAD
			PUSH buffer
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	char CMDRECV_minimap_ping(char* buffer) {
		static char result;
		const u32 func_address = 0x004BF7A0;
		__asm {
			PUSHAD
			MOV EAX, buffer
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* CMDRECV_merge_dark_archon() {
		static Unit* result;
		const u32 func_address = 0x004C0CD0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void bwfxn_display_text(s32 player, s32 text, s32 is_replay) {
		const u32 func_address = 0x0048D1C0;
		__asm {
			PUSHAD
			PUSH is_replay
			PUSH text
			MOV EAX, player
			CALL func_address
			POPAD
		}
	}

	void save_replay_action(void* game_actions, void* buffer, u32 action_size, char player) {
		s32 player_xd = player;
		const u32 func_address = 0x004CDE70;
		__asm {
			PUSHAD
			PUSH player_xd
			MOV EDI, action_size
			MOV EBX, buffer
			MOV EAX, game_actions
			CALL func_address
			POPAD
		}
	}

	char create_dashed_selection(u32 player, Sprite* sprite, char color) {
		static char result;
		static s32 color_xd = color;
		const u32 func_address = 0x00499A10;
		__asm {
			PUSHAD
			PUSH color_xd
			MOV ESI, sprite
			MOV EDI, player
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	b32 is_melee_game(char game_type, s16 game_type_param, char game_type_2) {
		static b32 result;
		const u32 func_address = 0x004AAFA0;
		__asm {
			PUSHAD
			MOVZX EDX, game_type
			MOVZX ECX, game_type_param
			MOVZX EBX, game_type_2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Player* randomize_player_slot() {
		static Player* result;
		const u32 func_address = 0x004A9A30;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void place_building_refinery_check(Unit* unit) {
		const u32 func_address = 0x0048E1E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void move_screen(Point16 pos) {
		const u32 func_address = 0x0049C440;
		__asm {
			PUSHAD 
			MOVZX EAX, pos.x
			MOVZX ECX, pos.y
			CALL func_address
			POPAD
		}
	}

	b32 is_at_home(Unit* unit) {
		const u32 func_address = 0x00462930;
		static b32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 is_under_carapace_swarm(Unit* unit) {
		const u32 func_address = 0x00476210;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void iscript_fireweapon(Unit* unit, u8 weapon_id) {
		const u32 func_address = 0x00479D40;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV BL, weapon_id
			CALL func_address
			POPAD
		}
	}

	void iscript_useweapon(Unit* unit, u16 weapon_id) {
		const u32 func_address = 0x00479C30;
		static u32 weapon_id_xd = weapon_id;
		__asm {
			PUSHAD
			PUSH weapon_id_xd
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	void draw_button_graphic(BinDlg* dialog, void* dest) {
		const u32 func_address = 0x004F0050;
		__asm {
			PUSH dest
			MOV ECX, dialog
			CALL func_address
			POPAD
		}
	}

	void draw_alternate_button_graphic(BinDlg* dialog1, BinDlg* dialog2, void* dest) {
		const u32 func_address = 0x004EFEB0;
		__asm {
			PUSH dest
			MOV EBX, dialog2
			MOV ECX, dialog1
			CALL func_address
			POPAD
		}
	}

	void draw_standard_button_graphic(BinDlg* dialog1, BinDlg* dialog2, void* dest) {
		const u32 func_address = 0x004EFD90;
		__asm {
			PUSH dest
			MOV EDI, dialog2
			MOV ECX, dialog1
			CALL func_address
			POPAD
		}
	}

	Unit* get_best_unit(Unit* unit, Box16* dimensions, bool(__fastcall* func)(Unit*, Unit*), Unit* unit2) {
		static Unit* result;
		const u32 func_address = 0x004E8740;
		__asm {
			PUSHAD
			PUSH unit2
			PUSH func
			PUSH dimensions
			MOV ESI, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 draw_box(u32 x, u32 y, u32 w, u32 h) {
		static u32 result;
		const u32 func_address = 0x004E1D20;
		__asm {
			PUSHAD
			PUSH h
			PUSH w
			PUSH y
			PUSH x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 save_replay(char* name) {
		static u32 result;
		const u32 func_address = 0x004DFAB0;
		__asm {
			PUSHAD
			PUSH 1
			MOV EDX, 0
			MOV EAX, name
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 copy_last_replay(char* name) {
		static u32 result;
		const u32 func_address = 0x004DED30;
		__asm {
			PUSHAD
			PUSH name
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void insert_order_targeting_ground_before(u8 order_id, Order* order, Unit* unit, u16 pos_x, u16 pos_y)
	{
		const u32 func_address = 0x00474790;
		static u32 pos_x_xd = pos_x;
		static u32 pos_y_xd = pos_y;
		__asm {
			PUSHAD
			PUSH pos_y_xd
			PUSH pos_x_xd
			MOV ESI, unit
			MOV EDI, order
			MOVZX BL, order_id
			CALL func_address
			POPAD
		}
	}

	void unit_order_move_to_target_reset_order_state(Unit* unit, u8 order_id, Unit* target)
	{
		const u32 func_address = 0x00479FA0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV ECX, target
			MOVZX DL, order_id
			MOV EAX, unit
			CALL func_address
			MOV result, EAX		// Returns a Unit* according to IDA, value unused though. -Pr0nogo
			POPAD
		}
	}

	void issue_order_targeting_fow(Unit* unit, u8 order_id, u32 unit_id, u16 pos_x, u16 pos_y)
	{
		const u32 func_address = 0x00474FB0;
		static u32 order_id_xd = order_id;
		static u32 pos_x_xd = pos_x;
		static u32 pos_y_xd = pos_y;
		__asm {
			PUSHAD
			PUSH pos_y_xd
			PUSH pos_x_xd
			PUSH unit_id
			PUSH order_id_xd
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	b32 draw_clamped_dialog_image(BinDlg* dest, u32 veeqtism, u32 pos_x, u32 pos_y, u32 flags)
	{
		const u32 func_address = 0x004EF600;
		static b32 result;
		__asm {
			PUSHAD
			PUSH flags
			PUSH pos_y
			PUSH pos_x
			PUSH veeqtism
			MOV ESI, dest
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8* bw_draw(u16 x_pos, u32 y_pos, u32 x_clamp, u16 y_clamp)
	{
		const u32 func_address = 0x004E1C70;
		static u8* result;
		static u32 y_clamp_xd = y_clamp;
		__asm {
			PUSHAD
			PUSH y_clamp_xd
			PUSH x_clamp
			PUSH y_pos
			MOVZX SI, x_pos
			CALL func_address
			POPAD
		}
		return result;
	}

	void dialog_draw_text(void* arg, BinDlg* button, u32 flag1, u32 flag2)
	{
		const u32 func_address = 0x004EF870;
		__asm {
			PUSHAD
			PUSH flag2
			PUSH flag1
			MOV ECX, button
			MOV EAX, arg
			CALL func_address
			POPAD
		}
	}

	b32 draw_dialog_image(Box16* dims, void* bitmap, u32 width, u32 height, u32 flags)
	{
		const u32 func_address = 0x004EF440;
		static b32 result;
		__asm {
			PUSHAD
			PUSH flags
			PUSH height
			PUSH width
			PUSH bitmap
			MOV ECX, dims
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void image_render_fxn_0_0(u32 screen_x, u32 screen_y, void* image_data, Box32* rect, u32 color_data) {
		const u32 func_address = 0x0040ABBE;
		__asm {
			PUSHAD
			PUSH color_data
			PUSH rect
			PUSH image_data
			MOV EDX, screen_y
			MOV ECX, screen_x
			CALL func_address
			POPAD
		}
	}

	void file_fatal(HANDLE* file, u32 something)
	{
		const u32 func_address = 0x004D2880;
		__asm {
			PUSHAD
			MOV EBX, something
			MOV ECX, file
			CALL func_address
			POPAD
		}
	}

	void alloc_init_dialog_data(BinDlg* dialog, void (__fastcall* function)(u32, u32, u32, u32, u32), const char* log_filename, u32 log_line)
	{
		const u32 func_address = 0x004194E0;
		__asm {
			PUSHAD
			PUSH log_line
			PUSH log_filename
			PUSH function
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	void initialize_dialog(u32(__fastcall* function)(BinDlg*, DialogEvent*), BinDlg* dialog)
	{
		const u32 func_address = 0x00419D20;
		__asm {
			PUSHAD
			MOV ESI, dialog
			MOV EAX, function
			CALL func_address
			POPAD
		}
	}

	u32 sfile_read_file(HANDLE* file, void* item, u32 size_arg, u32* read, u32 arg5)
	{
		static u32 result;
		const u32 func_address = 0x00410148;
		__asm {
			PUSHAD
			PUSH arg5
			PUSH read
			PUSH size_arg
			PUSH item
			PUSH file
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_fastest_distance(u32 angle)
	{
		static s32 result;
		const u32 func_address = 0x00494C10;
		__asm {
			PUSHAD
			MOV EAX, angle
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void SC_sprintf_s_2(char* buffer, int buffer_size, const char* format_string, int value1, int value2) {
		const u32 func_address = 0x0041F1B0;
		__asm {
			PUSHAD
			PUSH value2
			PUSH value1
			PUSH format_string
			MOV ESI, buffer_size
			MOV EDI, buffer
			CALL func_address
			ADD ESP, 0x0C
			POPAD
		}
	}

	void insert_order_targeting_unit_before(Unit* target, u8 order_id, Order* order, Unit* unit)
	{
		const u32 func_address = 0x004747E0;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EDI, order
			MOVZX BL, order_id
			MOV ECX, target
			CALL func_address
			POPAD
		}
	}

	void set_flag_attack_move(Unit* unit)
	{
		const u32 func_address = 0x004026A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	Unit* set_unit_movement_speed_ex(Unit* unit)
	{
		static Unit* result;
		const u32 func_address = 0x00495100;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 remove_from_selections_2(Unit* unit, s32 index) {
		static s32 result;
		const u32 func_address = 0x0049A170;
		__asm {
			PUSHAD
			PUSH index
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32* get_cd_key_info(const char** owner, u32* key) {
		static u32* result;
		const u32 func_address = 0x004D9BF0;
		__asm {
			PUSHAD
			MOV EBX, owner
			PUSH key
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 local_get_lang() {
		static u32 result;
		const u32 func_address = 0x0041F1D0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 make_valid_position(Point16* old_pos, Point16* new_pos, Box16* acceptable_area) {
		static s32 result;
		const u32 func_address = 0x0042A5C0;
		__asm {
			PUSHAD
			PUSH acceptable_area
			PUSH new_pos
			PUSH old_pos
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void cancel_zerg_building(Unit* unit)
	{
		const u32 func_address = 0x0045DA40;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			POPAD
		}
	}

	void update_building_construct_shields_amount(Unit* unit)
	{
		const u32 func_address = 0x0047B6A0;
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL func_address
			POPAD
		}
	}

	void cmdrecv_place_addon(u32 pos_x, u32 pos_y, u32 order_id, u32 built_unit_id) {
		const u32 func_address = 0x0048E0A0;
		__asm {
			PUSHAD
			PUSH built_unit_id
			PUSH pos_y
			PUSH pos_x
			MOV ECX, order_id
			CALL func_address
			POPAD
		}
	}

	void cmdrecv_place_building_normal(u32 pos_x, u32 pos_y, u32 order_id, u32 built_unit_id) {
		const u32 func_address = 0x0048E010;
		__asm {
			PUSHAD
			PUSH built_unit_id
			PUSH order_id
			PUSH pos_y
			PUSH pos_x
			CALL func_address
			POPAD
		}
	}

	u32 distance_from_box_corner(Unit* unit, u16 pos_x, u16 pos_y)
	{
		const u32 func_address = 0x00430E50;
		static u32 pos_x_xd = pos_x;
		static u32 pos_y_xd = pos_y;
		static u32 result;
		__asm {
			PUSHAD
			PUSH pos_y_xd
			PUSH pos_x_xd
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void load_campaign_progress()
	{
		const u32 func_address = 0x004B5050;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	BinDlg* load_full_menu_dialog(u32 filename, void* default_value, char* unused, u32 read, char* log_filename, u32 log_line)
	{
		static BinDlg* result;
		const u32 func_address = 0x004DDBE0;
		__asm {
			PUSHAD
			PUSH log_line
			PUSH log_filename
			PUSH read
			MOV ECX, filename
			MOV EDX, default_value
			MOV EDI, unused
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 glu_load_bin_dialog(BinDlg* dialog, void* proc)
	{
		static u32 result;
		const u32 func_address = 0x0041A080;
		__asm {
			PUSHAD
			PUSH proc
			MOV EDI, 0 // unused
			MOV EAX, dialog
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 change_menu()
	{
		static u32 result;
		const u32 func_address = 0x004DCFA0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void store_selection_group_hp_and_type()
	{
		const u32 func_address = 0x00424540;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void issue_order_before_2(u8 order_id, Order* order_queue_head, Unit* unit, Point16* pos, Unit* target)
	{
		const u32 func_address = 0x004747C0;
		__asm {
			PUSHAD
			PUSH target
			PUSH pos
			MOV ESI, unit
			MOV EDI, order_queue_head
			MOV BL, order_id
			CALL func_address
			POPAD
		}
	}

	u8 apply_defensive_matrix(Unit* unit) // 00455010
	{
		static u8 result;
		const u32 func_address = 0x00455010;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	u8 apply_lockdown(Unit* unit, u32 duration) // 00455380
	{
		static u8 result;
		const u32 func_address = 0x00455380;
		__asm {
			PUSHAD
			PUSH duration
			MOV EDI, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
	}

	u8 apply_irradiate(Unit* unit) // 00454E80
	{
		static u8 result;
		const u32 func_address = 0x00454E80;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	u8 apply_stasis_field(Unit* unit, u32 duration) // 004F67B0
	{
		static u8 result;
		const u32 func_address = 0x004F67B0;
		__asm {
			PUSHAD
			PUSH duration
			MOV EDI, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	u8 apply_plague(Unit* unit) // 004F4550
	{
		static u8 result;
		const u32 func_address = 0x004F4550;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* orders_die(Unit* unit)
	{
		static Unit* result;
		const u32 func_address = 0x00479480;
		__asm {
			PUSHAD
			PUSH unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void orders_powerup_1(Unit* unit)
	{
		const u32 func_address = 0x004F3EA0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void orders_infest_mine_4(Unit* unit)
	{
		const u32 func_address = 0x004EA290;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void orders_enter_exit_gas(Unit* unit)
	{
		const u32 func_address = 0x00469980;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	Sprite* orders_powerup_2(Unit* unit)
	{
		const u32 func_address = 0x004F3E10;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void orders_reset_collision_1(Unit* unit)
	{
		const u32 func_address = 0x004671B0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void orders_reset_collision_2(Unit* unit)
	{
		const u32 func_address = 0x0042E320;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void orders_pickup_4(Unit* unit)
	{
		const u32 func_address = 0x004E7B70;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void orders_rescue_passive(Unit* unit)
	{
		const u32 func_address = 0x004A1EF0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void set_kills_string_text(BinDlg* dialog) {
		const u32 func_address = 0x00425DD0;
		__asm {
			PUSHAD
			MOV EBX, dialog
			CALL func_address
			POPAD
		}
	}

	void set_unit_hp(Unit* unit, u32 amount)
	{
		const u32 func_address = 0x00467340;
		__asm {
			PUSHAD
			MOV ECX, amount
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	s32 unit_movement_another_path(Unit* unit, Point16 dest)
	{
		const u32 func_address = 0x0042FE00;
		static s32 result;
		__asm {
			PUSHAD
			PUSH dest
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void scroll_screen_left(u32 x_diff) { // 0049C1A0 
		const u32 func_address = 0x0049C1A0;
		__asm {
			PUSHAD
			MOV EAX, x_diff
			CALL func_address
			POPAD
		}
	}

	void scroll_screen_right(u32 x_diff) { // 0049C0C0 
		const u32 func_address = 0x0049C0C0;
		__asm {
			PUSHAD
			MOV EDX, x_diff
			CALL func_address
			POPAD
		}
	}

	void scroll_screen_down(u32 y_diff) { // 0049C280 
		const u32 func_address = 0x0049C280;
		__asm {
			PUSHAD
			MOV EDX, y_diff
			CALL func_address
			POPAD
		}
	}

	void scroll_screen_up(u32 y_diff) { // 0049C360 
		const u32 func_address = 0x0049C360;
		__asm {
			PUSHAD
			MOV EAX, y_diff
			CALL func_address
			POPAD
		}
	}

	void update_carryable_sprite_flags(Sprite* sprite)
	{
		const u32 func_address = 0x004981B0;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL func_address
			POPAD
		}
	}

	u32 snet_get_player_name(u32 player_id, int player_name, s32 some_index)
	{
		const u32 func_address = 0x00410184;
		static u32 result;
		__asm {
			PUSHAD
			PUSH some_index
			PUSH player_name
			PUSH player_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void print_lobby_string(u32 player_id, int string)
	{
		const u32 func_address = 0x004B91C0;
		__asm {
			PUSHAD
			PUSH string
			MOV EAX, player_id
			CALL func_address
			POPAD
		}
	}

	b32 is_multiplayer_mode_bnet()
	{
		return *g_multiplayer_mode && *g_current_network_mode == 'BNET';
	}

	u32 snet_send_server_chat_command(char string)
	{
		const u32 func_address = 0x00410280;
		static u32 result;
		__asm {
			PUSHAD
			PUSH string
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 battlenet_play_sound(char unk1, u32 event_type, char unk2)
	{
		const u32 func_address = 0x004AD0B0;
		__asm {
			PUSHAD
			PUSH unk2
			PUSH event_type
			PUSH unk1
			CALL func_address
			POPAD
		}
		return 0;
	}

	Unit* remove_repulse_tile(Unit* unit)
	{
		const u32 func_address = 0x004533D0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8 transform_unit_sprite(Unit* unit, u32 new_sprite_id)
	{
		const u32 func_address = 0x0049F860;
		static u8 result;
		__asm {
			PUSHAD
			PUSH new_sprite_id
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void remove_from_client_selection_3(Unit* unit)
	{
		const u32 func_address = 0x0049F7A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void update_unit_order_finder(Unit* unit)
	{
		const u32 func_address = 0x0046A3A0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	void set_repulse_angle(Unit* unit)
	{
		const u32 func_address = 0x00453300;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	void transform_subunit(Unit* unit, u32 new_unit_id)
	{
		const u32 func_address = 0x0049F6A0;
		__asm {
			PUSHAD
			PUSH new_unit_id
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	u8 apply_unit_effects(Unit* unit)
	{
		const u32 func_address = 0x00492DA0;
		static u8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void refresh_layer_5() {
		const u32 func_address = 0x004BD350;
		__asm {
			PUSHAD
			CALL func_address
			POPAD
		}
	}

	void manage_vision_references(s32 vision_filter, Unit* unit)
	{
		const u32 func_address = 0x0049E8A0;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, vision_filter
			CALL func_address
			POPAD
		}
	}

	u32 get_updated_sight_range(Unit* unit, bool is_for_spellcasting)
	{
		const u32 func_address = 0x004E5B40;
		static b32 is_for_spellcasting_xd = is_for_spellcasting;
		static u32 result;
		__asm {
			PUSHAD
			PUSH is_for_spellcasting_xd
			MOV EDX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8 create_defensive_matrix(Unit* unit)
	{
		const u32 func_address = 0x00454F90;
		static u8 result;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	Image* create_image_overlay(Sprite* sprite, u32 image_id, u8 x_offset, u8 y_offset, u8 direction)
	{
		const u32 func_address = 0x00498E00;
		static u32 x_offset_xd = x_offset;
		static u32 y_offset_xd = y_offset;
		static u32 direction_xd = direction;
		static Image* result;
		__asm {
			PUSHAD
			PUSH direction
			PUSH y_offset
			PUSH x_offset
			MOV ESI, image_id
			MOV EAX, sprite
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool statport_button_press(BinDlg* dialog_1, BinDlg* dialog_2)
	{
		const u32 func_address = 0x0045F0B0;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, dialog_1
			MOV EDI, dialog_2
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	BYTE* statport_alloc()
	{
		const u32 func_address = 0x0045E360;
		static BYTE* result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool generic_dialog_interact(BinDlg* dialog, DialogEvent* dlg_event)
	{
		const u32 func_address = 0x00418EB0;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, dialog
			MOV EDX, dlg_event
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool generic_common_interact(BinDlg* dialog, DialogEvent* dlg_event)
	{
		const u32 func_address = 0x00418C40;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, dialog
			MOV EAX, dlg_event
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void glu_main_create(BinDlg* dialog)
	{
		const u32 func_address = 0x004DB280;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	void dialog_swish_in(BinDlg* dialog)
	{
		const u32 func_address = 0x004DE150;
		__asm {
			PUSHAD
			MOV EBX, dialog
			CALL func_address
			POPAD
		}
	}

	void dialog_music_fade(u32 music_id)
	{
		const u32 func_address = 0x004A6310;
		__asm {
			PUSHAD
			MOV EAX, music_id
			CALL func_address
			POPAD
		}
	}

	void glu_main_destroy(BinDlg* dialog)
	{
		const u32 func_address = 0x004DB140;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	char load_glu_pok_bindlg(const char* string)
	{
		const u32 func_address = 0x004B7180;
		static char result;
		__asm {
			PUSHAD
			MOV EAX, string
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	s32 select_game_mode(u32 game_mode)
	{
		const u32 func_address = 0x004DB6E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH game_mode
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char dialog_swish_out(BinDlg* dialog)
	{
		const u32 func_address = 0x004DD040;
		static char result;
		__asm {
			PUSHAD
			PUSH dialog
			CALL func_address
			MOV result, AL
			POPAD
		}
	}

	s32 load_staredit_process(BinDlg* dialog)
	{
		const u32 func_address = 0x004DB490;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 glu_main_display_cdrom_error_bindlg()
	{
		const u32 func_address = 0x004DB770;
		static s32 result;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 register_menu_functions(bool (__fastcall** save_game_text_box)(BinDlg* func_dialog, DialogEvent* func_dlg_event), BinDlg* dialog, u32 count, u32 palette)
	{
		const u32 func_address = 0x004DD9E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH palette
			PUSH count
			PUSH dialog
			MOV EAX, save_game_text_box
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void game_menu_bindlg(BinDlg* dialog)
	{
		const u32 func_address = 0x004CA370;
		__asm {
			PUSHAD
			MOV ECX, dialog
			CALL func_address
			POPAD
		}
	}

	void game_menu_bindlg_bypass(BinDlg* dialog)
	{
		const u32 func_address = 0x004CA450;
		__asm {
			PUSHAD
			MOV ECX, dialog
			CALL func_address
			POPAD
		}
	}

	u32 select_unit_group(u8 control_group)
	{
		static u32 result;
		const u32 func_address = 0x00496B40;
		__asm {
			PUSHAD
			PUSH control_group
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8 keypress_escape()
	{
		static u8 result;
		const u32 func_address = 0x00489DA0;
		__asm {
			PUSHAD
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	b32 unit_is_valid_caster(Unit* unit) // Based on 00401210. -Pr0nogo
	{
		return  (units_dat::base_property[unit->id] & UnitProperty::Spellcaster)
			&& !(unit->status & UnitStatus::IsHallucination);
	}

	u8 trigger_portrait_finished(BinDlg* dialog, u16 timer)
	{
		static u8 result;
		const u32 func_address = 0x0045E610;
		__asm {
			PUSHAD
			MOV ECX, dialog
			MOV DX, timer
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void display_update_portrait(u32 portrait_id, Unit* unit, u16 idle_or_talking)
	{
		const u32 func_address = 0x0045EA30;
		u32 idle_or_talking_xd = idle_or_talking;
		__asm {
			PUSHAD
			PUSH idle_or_talking_xd
			PUSH unit
			MOV EAX, portrait_id
			CALL func_address
			POPAD
		}
	}

	void trigger_portrait_something(BinDlg* dialog, u16 type_arg)
	{
		const u32 func_address = 0x0045EC40;
		__asm {
			PUSHAD
			MOV ECX, dialog
			MOV DX, type_arg
			CALL func_address
			POPAD
		}
	}

	void animate_unit_portrait_for_event(BinDlg* dialog, u16 type_arg)
	{
		const u32 func_address = 0x0045EF50;
		__asm {
			PUSHAD
			MOV ECX, dialog
			MOV DX, type_arg
			CALL func_address
			POPAD
		}
	}

	b32 set_last_event_position(u32 x, u32 y)
	{
		static b32 result;
		const u32 func_address = 0x0045E3A0;
		__asm {
			PUSHAD
			PUSH y
			MOV EAX, x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 svid_play_end(u8 video)
	{
		static u32 result;
		const u32 func_address = 0x004100EE;
		u32 video_xd = video;
		__asm {
			PUSHAD
			PUSH video_xd
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void start_unit_portrait(BinDlg* dialog)
	{
		const u32 func_address = 0x0044EDB0;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	u16 get_unit_portrait_override(u16 unit_id)
	{
		static u16 result;
		const u32 func_address = 0x0045E320;
		__asm {
			PUSHAD
			MOV AX, unit_id
			CALL func_address
			MOV result, AX
			POPAD
		}
		return result;
	}

	void play_music(u32 music_id)
	{
		const u32 func_address = 0x004A6140;
		__asm {
			PUSHAD
			MOV EDI, music_id
			CALL func_address
			POPAD
		}
	}

	u32 pixel_pos_has_cagra(s16 pos_x, s16 pos_y) // Default args are inverted
	{
		static u32 result;
		const u32 func_address = 0x004E8BC0;
		static s32 pos_y_xd = pos_y;
		__asm {
			PUSHAD
			PUSH pos_y_xd
			MOV AX, pos_x
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	b32 chk_larva_stray(Unit* larva, u32 pos_x, u32 pos_y)
	{
		static s32 result;
		const u32 func_address = 0x004E93E0;
		__asm {
			PUSHAD
			PUSH pos_y
			PUSH pos_x
			MOV EBX, larva
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void chk_larva_stray0(Unit* larva, u32 pos_x, u32 pos_y)
	{
		const u32 func_address = 0x004E94B0;
		__asm {
			PUSHAD
			PUSH pos_y
			PUSH pos_x
			PUSH larva
			CALL func_address
			POPAD
		}
	}

	void issue_queued_order_target(Unit* target, Unit* attacker, u8 order_id, u32 stop_previous)
	{
		const u32 func_address = 0x00474D10;
		u32 order_id_xd = order_id;
		__asm {
			PUSHAD
			PUSH stop_previous
			PUSH order_id_xd
			PUSH attacker
			MOV EAX, target
			CALL func_address
			POPAD
		}
	}

	void play_addon_detach_sound(Unit* unit)
	{
		const u32 func_address = 0x0048F8A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	Unit* find_best_return_cargo_target(Unit* worker)
	{
		static Unit* result;
		const u32 func_address = 0x00441310;
		__asm {
			PUSHAD
			MOV EAX, worker
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void return_resources_from_unit(Unit* unit)
	{
		const u32 func_address = 0x004F39A0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	void unit_destructor_special_cases(Unit* unit)
	{
		const u32 func_address = 0x0049F4A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void remove_references(Unit* unit)
	{
		const u32 func_address = 0x0049EB70;
		__asm {
			PUSHAD
			PUSH 1 // 2nd arg is always true
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	Bullet* clear_bullet_targets(Unit* unit)
	{
		const u32 func_address = 0x0048AAC0;
		static Bullet* result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void building_death(Unit* unit, Sprite* sprite)
	{
		const u32 func_address = 0x0049FD00;
		__asm {
			PUSHAD
			MOV EDX, sprite
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	Unit* remove_unit_from_lists(Unit* unit)
	{
		const u32 func_address = 0x0049E3F0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* clist_remove_hidden_unit_entry(Unit* unit)
	{
		const u32 func_address = 0x0049DE50;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* clist_remove_used_unit_entry(Unit* unit)
	{
		const u32 func_address = 0x0049DEA0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void unit_movement_instant_stop(Unit* unit)
	{
		const u32 func_address = 0x0046BF00;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void progress_flingy(Unit* unit)
	{
		const u32 func_address = 0x00495EE0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void finish_unit_movement(Unit* unit)
	{
		const u32 func_address = 0x0046AD90;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	u32 get_random_yes_unit_response(u32 unit_id)
	{
		const u32 func_address = 0x0048EAB0;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, unit_id
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 bw_refresh(s32 left, s32 height, s32 top, s32 width)
	{
		const u32 func_address = 0x0041E0D0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH width
			MOV ECX, top
			MOV EDX, height
			MOV EAX, result
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Image* make_cloaked(u8 param, Sprite* sprite, u8 status, u8 timer)
	{
		const u32 func_address = 0x00497FD0;
		static Image* result;
		static u32 status_xd = status;
		static u32 timer_xd = timer;
		__asm {
			PUSHAD
			PUSH timer_xd
			PUSH status_xd
			MOV ESI, sprite
			MOV AL, param
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void unit_refund_partial_technology(Unit* unit)
	{
		const u32 func_address = 0x00453DD0;
		__asm {
			PUSHAD
			MOV EDX, 0 // unused arg
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	void unit_refund_partial_upgrade(Unit* unit)
	{
		const u32 func_address = 0x00454220;
		__asm {
			PUSHAD
			MOV EDX, 0 // unused arg
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}
}