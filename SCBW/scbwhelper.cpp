#include "scbwhelper.h"
#include "api.h"
#include "globals.h"
#include <cassert>

namespace helper
{
	// sign flag
	s8 SETS(s32 x)
	{
		if (sizeof(s32) == 1)
			return s8(x) < 0;
		if (sizeof(s32) == 2)
			return s16(x) < 0;
		if (sizeof(s32) == 4)
			return s32(x) < 0;
		return s64(x) < 0;
	}

	s8 OFSUB(s32 x, s32 y)
	{
		s32 y2 = y;
		s8 sx = SETS(x);
		return (sx ^ SETS(y2)) & (sx ^ SETS(x - y2));
	}

	bool has_money_can_make(Unit* builder, u32 unitToBuild)
	{
		const u32 Func_HasMoneyCanMake = 0x00467250;
		static b32 bPreResult;
		__asm {
			PUSHAD
			PUSH unitToBuild
			MOV EDI, builder
			CALL Func_HasMoneyCanMake
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	// For loadRaceUI

	void get_race_sound_list(s8 a1)
	{
		const u32 Func_getRaceSoundList = 0x0048FB40;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_getRaceSoundList
			POPAD
		}
	}

	s32 dialog_grp_construct(s32 a1, s32 a2, s32 a3, s32(__fastcall* a4)(s32, s32, s32, s32))
	{
		const u32 Func_dialogGrpConstruct = 0x004197B0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			MOV a1, EAX
			MOV a2, ECX
			CALL Func_dialogGrpConstruct
			MOV EAX, result
			POPAD
		}
		return result;
	}

	void load_graphic(s8 a1, s32 a2, s32 a3, s8 a4, s32 a5)
	{
		const u32 Func_loadGraphic = 0x004D2B30;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			MOVZX ECX, a1
			MOV EDX, a2
			MOV ESI, a3
			CALL Func_loadGraphic
			POPAD
		}
	}

	s32 rand_func()
	{
		const u32 Func_randFunc = 0x0040C7BD;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_randFunc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_hp_below_33_percent(Unit* a1)
	{
		const u32 Func_unitHPBelow33Percent = 0x004022C0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitHPBelow33Percent
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unit_hp_between_33_66_percent(s32 a1, Unit* a2)
	{
		const u32 Func_unitHPBetween33_66Percent = 0x00402270;
		static s32 result;

		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL Func_unitHPBetween33_66Percent
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 get_max_hp(Unit* a1)
	{
		const u32 Func_getMaxHP = 0x00401400;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_getMaxHP
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u16 get_last_queue_slot_id(Unit* unit)
	{
		const u32 Func_getLastQueueSlotType = 0x0047B270;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_getLastQueueSlotType
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	void add_text_to_dialog(BinDlg* dialog, s16 index, const char* text)
	{
		const u32 Func_addTextToDialog = 0x004258B0;
		static s32 _a3 = (s32)text;
		__asm {
			PUSHAD
			MOV EAX, dialog
			MOVZX CX, index
			PUSH _a3
			CALL Func_addTextToDialog
			POPAD
		}
	}

	void draw_energy(BinDlg* a1)
	{
		const u32 Func_drawEnergy = 0x00425A30;
		__asm {
			PUSHAD
			MOV EBX, a1
			CALL Func_drawEnergy
			POPAD
		}
	}

	void hide_optional_status_screen_controls(BinDlg* a1)
	{
		const u32 Func_hideOptionalStatusScreenControls = 0x00457310;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_hideOptionalStatusScreenControls
			POPAD
		}
	}

	void show_control_group(BinDlg* dialog, s16 firstId, s16 lastId)
	{
		const u32 Func_showControlGroup = 0x00457250;
		s32 lastIdArg = lastId;
		__asm {
			PUSHAD
			PUSH lastIdArg
			MOVZX ECX, firstId
			MOV EAX, dialog
			CALL Func_showControlGroup
			POPAD
		}
	}

	void hide_dialog(BinDlg* dialog)
	{
		const u32 Func_hideDialog = 0x00418700;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_hideDialog
			POPAD
		}
	}

	// For draw_supply_unit_info
	s32 get_supplies_used_no_args()
	{
		const u32 Func_getSuppliesUsed = 0x00424732;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_getSuppliesUsed
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_supplies_available(u8 a1, s32 a2)
	{
		const u32 Func_getSuppliesAvailable = 0x00488900;
		static u32 result;
		__asm {
			PUSHAD
			MOVZX AL, a1
			MOV ECX, a2
			CALL Func_getSuppliesAvailable
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void unit_stat_act_standard(void* a1)
	{
		const u32 Func_unitStatAct_Standard = 0x00426F50;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitStatAct_Standard
			POPAD
		}
	}

	// For AI_TargetEnemyProc

	bool ai_unit_can_attack(Unit* a1)
	{
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

	s32 ui_needs_redraw()
	{
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

	Unit* generic_status_update_drawn_values()
	{
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

	void draw_progress_bar(BinDlg* a1)
	{
		const u32 Func_drawProgressBar = 0x004260C0;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawProgressBar
			POPAD
		}
	}

	void draw_nuke_silo_info(BinDlg* a1)
	{
		const u32 Func_drawNukeSiloInfo = 0x00426FF0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawNukeSiloInfo
			POPAD
		}
	}

	// Equivalent to code unit_is_active_transport @ 0x004E6BA0
	// Modified to remove Ovileth checks as they are no longer tied to upgrades in HYDRA
	bool unit_is_active_transport(Unit* unit)
	{
		bool result = false;
		if (!(unit->status & UnitStatus::IsHallucination))
		{
			result = units_dat::SpaceProvided[unit->id] != 0;
		}
		return result;
	}

	bool is_queue_slot_active(Unit* unit, s32 slot)
	{
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

	void draw_training_progress(BinDlg* dialog)
	{
		const u32 Func_drawTrainingProgress = 0x004268D0;
		__asm {
			PUSHAD
			PUSH dialog
			CALL Func_drawTrainingProgress
			POPAD
		}
	}

	bool is_constructing_addon(const Unit* a1)
	{
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

	bool is_attempting_protoss_build(Unit* a1)
	{
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

	void draw_addon_progress(BinDlg* a1)
	{
		const u32 Func_drawAddonProgress = 0x00425F30;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawAddonProgress
			POPAD
		}
	}

	void draw_tech_progress(BinDlg* a1)
	{
		const u32 Func_drawTechProgress = 0x004266F0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawTechProgress
			POPAD
		}
	}

	void draw_upgrade_progress(BinDlg* a1)
	{
		const u32 Func_drawUpgradeProgress = 0x00426500;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawUpgradeProgress
			POPAD
		}
	}

	bool unit_is_geyser(Unit* a1)
	{
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

	void draw_gas_quantity(BinDlg* a1)
	{
		const u32 Func_drawGasQuantity = 0x00426190;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_drawGasQuantity
			POPAD
		}
	}

	s8 get_unit_ground_weapon(Unit* a1)
	{
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

	void draw_display_buttons(BinDlg* a1)
	{
		const u32 Func_drawDisplayButtons = 0x00426C60;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawDisplayButtons
			POPAD
		}
	}

	BinDlg* get_control_from_index(BinDlg* a1, s16 a2)
	{
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

	void set_text_string(BinDlg* a1)
	{
		const u32 Func_setTextStr = 0x004263E0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setTextStr
			POPAD
		}
	}

	void set_unit_status_string_text(BinDlg* a1)
	{
		const u32 Func_setUnitStatusStrText = 0x00425B50;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setUnitStatusStrText
			POPAD
		}
	}

	s32 get_unit_name(Unit* a1)
	{
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

	s32 get_current_hp_plus_shields(Unit* a1)
	{
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

	s32 get_base_strength(s32 a1, Unit* a2)
	{
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

	s32 can_weapon_target_unit(u8 a1, Unit* a2, Unit* a3)
	{
		const u32 Func_canWeaponTargetUnit = 0x00475CE0;
		static s32 result;
		__asm {
			PUSHAD
			MOV AL, a1
			MOV EDX, a2
			PUSH a3
			CALL Func_canWeaponTargetUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For AI_EMPShieldRequirementsProc

	s32 ai_target_unit_is_worth_hitting(Unit* a1, Unit* a2)
	{
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

	s32 create_splash_weapon_proc(s16 a1, s16 a2, s16 a3, s32(__fastcall* a4)(s32, s32), s32 a5)
	{
		const u32 Func_createSplashWeaponProc = 0x004E82E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			MOV AX, a1
			MOV DX, a2
			MOV CX, a3
			CALL Func_createSplashWeaponProc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 unit_shields_clump_in_area_accumulator_proc(Unit* a1, Unit* a2)
	{
		const u32 Func_unitShieldsClumpInAreaAccumulatorProc = 0x00440A60;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL Func_unitShieldsClumpInAreaAccumulatorProc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For hallucinationHit

	void unit_was_hit(Unit* a1, Unit* a2, s32 a3)
	{
		const u32 Func_unitWasHit = 0x004795D0;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EBX, a1
			CALL Func_unitWasHit
			POPAD
		}
	}

	void unit_attack_notify(Unit* a1)
	{
		const u32 Func_doAttackNotifyEvent = 0x0048F230;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_doAttackNotifyEvent
			POPAD
		}
	}

	Image* create_shield_overlay(s32 a1, Unit* a2)
	{
		const u32 Func_createShieldOverlay = 0x004E6140;
		static Image* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_createShieldOverlay
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For drawImageHPBar

	s32 unk_billions(s32 a1, s32 a2, s32 a3, s32 a4)
	{
		const u32 Func_unk_billions = 0x0047A750;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV EBX, a3
			CALL Func_unk_billions
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 unk_billions2(s32 a1, s32 a2, s32 a3, s32 a4)
	{
		const u32 Func_unk_billions2 = 0x0047A6E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV EDI, a3
			CALL Func_unk_billions2
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 hpBarSurfaceBuffer(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5)
	{
		const u32 Func_hpBarSurfaceBuffer = 0x0047A350;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_hpBarSurfaceBuffer
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 hpBarSurfaceBuffer2(s32 a1, s32 a2, u16 a3, s32 a4)
	{
		const u32 Func_hpBarSurfaceBuffer2 = 0x0047A120;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			PUSH a2
			MOV EAX, a1
			CALL Func_hpBarSurfaceBuffer2
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 renderHPBar(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5)
	{
		const u32 Func_renderHPBar = 0x0047A4E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_renderHPBar
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 getUnitMaxEnergy(Unit* a1)
	{
		const u32 Func_getUnitMaxEnergy = 0x00491870;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_getUnitMaxEnergy
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 getHpBarDrawSizes(s32 a1, s32 a2, s32 a3, s32 a4)
	{
		const u32 Func_getHpBarDrawSizes = 0x0047A670;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV EDI, a3
			CALL Func_getHpBarDrawSizes
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For canCastSpell

	bool unitIsFrozen(Unit* a1)
	{
		const u32 Func_unitIsFrozen = 0x004020B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsFrozen
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool isUnitCasterNotHallucination(Unit* a1)
	{
		const u32 Func_isUnitCasterNotHallucination = 0x00401210;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isUnitCasterNotHallucination
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	b32 getActiveTileFlag(s16 y, s16 x, s32 includeFlags, s32 excludeFlags)
	{
		const u32 Func_getActiveTileFlag = 0x00472F80;
		static b32 result;
		__asm {
			PUSHAD
			PUSH excludeFlags
			PUSH includeFlags
			PUSH x
			MOVZX AX, y
			CALL Func_getActiveTileFlag
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* getTargetUnitAbilityErrorStatString(Unit* a1, char a2, s32 a3)
	{
		const u32 Func_getTargetUnitAbilityErrorStatString = 0x00491E80;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_getTargetUnitAbilityErrorStatString
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For modifyUnitShields

	s32 SErrSetLastError(char a1)
	{
		const u32 Func_SErrSetLastError = 0x00410172;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_SErrSetLastError
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void updateImageDrawData(Image* a1)
	{
		const u32 Func_updateImageDrawData = 0x004D57B0;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_updateImageDrawData
			POPAD
		}
	}


	Image* CreateHealthBar(s32 a1, Sprite* a2)
	{
		const u32 CreateHealthBar_function = 0x004D6420;
		static Image* result;
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

	void RemoveHPBar(Image* a1)
	{
		const u32 RemoveHPBar_function = 0x004D5030;
		_asm {
			PUSHAD
			MOV ESI, a1
			CALL RemoveHPBar_function
			POPAD
		}
	}

	void playImageIscript(Image* img, u8 iscriptAnim)
	{
		const u32 playImageIscript_function = 0x004D8470;
		_asm {
			PUSHAD
			PUSH iscriptAnim
			MOV ECX, img
			CALL playImageIscript_function
			POPAD
		}
	}

	bool canEnterTransport(Unit* a1, Unit* a2)
	{
		const u32 Func_canEnterTransport = 0x004E6E00;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a2
			MOV EAX, a1
			CALL Func_canEnterTransport
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s16 get_region_id(s32 a1, s32 a2)
	{
		const u32 Func_SAI_GetRegionIdFromPxEx = 0x0049C9F0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDI, a1
			MOV ECX, a2
			CALL Func_SAI_GetRegionIdFromPxEx
			MOV result, EAX
			POPAD
		}
		return (s16)result;
	}

	s32 getUnitDistanceToHalt(s32 a1, Unit* a2, s32 a3)
	{
		const u32 Func_getUnitDistanceToHalt = 0x00402140;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOV ECX, a1
			MOV EAX, a2
			CALL Func_getUnitDistanceToHalt
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For mapDataTransfer

	s32 dataTransfer00(s32 a1, char a2, s32 a3)
	{
		const u32 Func_dataTransfer00 = 0x00472A00;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CL, a2
			MOV ESI, a3
			CALL Func_dataTransfer00
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer01(u32 a1, s32 a2, u32 a3)
	{
		const u32 Func_dataTransfer01 = 0x0045A600;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EAX, a1
			CALL Func_dataTransfer01
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer02(s32 a1, s32 a2, char a3)
	{
		const u32 Func_dataTransfer02 = 0x0045A230;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			MOVZX BL, a3
			CALL Func_dataTransfer02
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer03(s32 a1, char a2, s32 a3)
	{
		const u32 Func_dataTransfer03 = 0x0045A1F0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_dataTransfer03
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer04(u32 a1, s32 a2, s32 a3)
	{
		const u32 Func_dataTransfer04 = 0x0045A390;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EAX, a1
			CALL Func_dataTransfer04
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer05(s32 a1, s32 a2)
	{
		const u32 Func_dataTransfer05 = 0x0045A190;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV EDI, a2
			CALL Func_dataTransfer05
			MOV result, EAX
			POPAD
		}
		return result;
	}

	VectorHead* vectorTransfer(char a1, char a2)
	{
		const u32 Func_vectorTransfer = 0x0045A010;
		static VectorHead* result;
		__asm {
			PUSHAD
			PUSH a2
			MOVZX BL, a1
			CALL Func_vectorTransfer
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 dataTransfer06(s32 a1)
	{
		const u32 Func_dataTransfer06 = 0x00472900;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_dataTransfer06
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For getRandYesSFX
	s32 randGetSFXBetween(s32 soundIdMin, s32 soundIdMax)
	{
		const u32 Func_randGetSFXBetween = 0x0048E940;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDI, soundIdMin
			MOV ESI, soundIdMax
			CALL Func_randGetSFXBetween
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for ai_train_unit
	char ai_training_worker(Unit* result, Unit* parent)
	{
		const u32 Func_AI_TrainingWorker = 0x00435700;
		static char resultFunc;
		__asm
		{
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingWorker
			MOV resultFunc, AL
			POPAD
		}
		return resultFunc;
	}

	void ai_training_overlord(Unit* result, Unit* parent)
	{
		const u32 Func_AI_TrainingOverlord = 0x00435770;
		__asm
		{
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingOverlord
			POPAD
		}
	}

	void ai_training_normal(Unit* result, Unit* parent)
	{
		const u32 Func_AI_TrainingNormal = 0x00435DB0;
		__asm
		{
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingNormal
			POPAD
		}
	}

	void ai_order_to_destination(Unit* unit, s32 orderId, s32 x, s32 y)
	{
		const u32 Func_AI_OrderToDestination = 0x0043D5D0;
		__asm
		{
			PUSHAD
			PUSH y
			PUSH x
			PUSH orderId
			PUSH unit
			CALL Func_AI_OrderToDestination
			POPAD
		}
	}

	void ai_assign_region_to_slowest_unit(AiRegion* region)
	{
		const u32 Func_AI_AssignCaptainToSlowestUnit = 0x00436F70;
		__asm
		{
			PUSHAD
			PUSH region
			CALL Func_AI_AssignCaptainToSlowestUnit
			POPAD
		}
	}

	// for ai_create_nuke
	Unit* create_unit(s16 unitType, s32 x, s32 y, s32 playerId)
	{
		const u32 Func_createUnit = 0x004A09D0;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH playerId
			PUSH y
			MOV EAX, x
			MOVZX CX, unitType
			CALL Func_createUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void update_unit_stats_and_finish_building(Unit* unit)
	{
		const u32 Func_updateUnitStatsFinishBuilding = 0x004A01F0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateUnitStatsFinishBuilding
			POPAD
		}
	}

	u32 update_unit_strength_and_apply_default_orders(Unit* unit)
	{
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

	void hideUnit(Unit* unit)
	{
		const u32 Func_hideUnit = 0x004E6340;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_hideUnit
			POPAD
		}
	}

	// for AI_TrainingBroodling

	void ai_set_region_type(s32 regionType, AiRegion* region)
	{
		const u32 func_address = 0x004390A0;
		__asm {
			PUSHAD
			MOV EBX, regionType
			MOV ESI, region
			CALL func_address
			POPAD
		}
	}

	void add_military_ai(AiRegion* region, Unit* unit, s32 always_this_region)
	{
		const u32 Func_AI_AddMilitary = 0x0043DA20;
		__asm {
			PUSHAD
			PUSH always_this_region
			MOV EAX, region
			MOV EBX, unit
			CALL Func_AI_AddMilitary
			POPAD
		}
	}

	// for AI_AddUnitAI

	BuildingAi* create_building_ai(BuildingAiArray** buildingAI)
	{
		const u32 Func_AI_CreateBuildingAi = 0x004044C0;
		static BuildingAi* result;
		__asm {
			PUSHAD
			MOV ESI, buildingAI
			CALL Func_AI_CreateBuildingAi
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void issue_order_targeting_nothing(Unit* unit, u8 orderId)
	{
		const u32 Func_orderComputerClear = 0x00475310;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOVZX CL, orderId
			CALL Func_orderComputerClear
			POPAD
		}
	}

	Unit* set_secondary_order(Unit* result, char orderId)
	{
		const u32 Func_setSecondaryOrder = 0x004743D0;
		static Unit* result2;
		__asm {
			PUSHAD
			MOV EAX, result
			MOVZX CL, orderId
			CALL Func_setSecondaryOrder
			MOV result2, EAX
			POPAD
		}
		return result2;
	}

	void ai_make_guard(s32 playerId, Unit* unit)
	{
		const u32 Func_AI_MakeGuard = 0x0043A010;
		__asm {
			PUSHAD
			PUSH unit
			PUSH playerId
			CALL Func_AI_MakeGuard
			POPAD
		}
	}

	bool unit_is_active_resource_depot(Unit* unit)
	{
		const u32 Func_unitIsActiveResourceDepot = 0x00401CF0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_unitIsActiveResourceDepot
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	u32 get_default_refinery_type(u32 playerId)
	{
		const u32 Func_getPlayerDefaultRefineryUnitType = 0x004320D0;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, playerId
			CALL Func_getPlayerDefaultRefineryUnitType
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for ai_unit_was_hit
	// some of these should probably be methods in api/etc

	char ai_cancel_structure(Unit* structure)
	{
		const u32 Func_AI_cancel_structure = 0x00468280;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, structure
			CALL Func_AI_cancel_structure
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	Unit* ai_get_actual_target(Unit* attacker, Unit* target)
	{
		const u32 Func_AI_GetActualTarget = 0x0043AD60;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EDI, attacker
			MOV ESI, target
			CALL Func_AI_GetActualTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* check_arbiter_cloaking_range(Unit* unit, u16 id)
	{
		const u32 Func_checkArbiterCloakingRange = 0x004410C0;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH id
			MOV EAX, unit
			CALL Func_checkArbiterCloakingRange
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_order_lurker_burrow(Unit* unit)
	{
		const u32 Func_AI_OrderLurkerBurrow = 0x0043C230;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_AI_OrderLurkerBurrow
			POPAD
		}
	}

	bool ai_is_under_attack(Unit* unit, s32 flag)
	{
		const u32 Func_AI_IsUnderAttack = 0x00436E70;
		static u32 result;
		__asm {
			PUSHAD
			PUSH flag
			MOV EAX, unit
			CALL Func_AI_IsUnderAttack
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 is_unit_loaded(Unit* unit)
	{
		const u32 Func_isUnitLoaded = 0x004E7110;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_isUnitLoaded
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_try_return_home(Unit* unit, s32 dontIssueOrder)
	{
		const u32 Func_AI_TryReturnHome = 0x00462EA0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH dontIssueOrder
			MOV EAX, unit
			CALL Func_AI_TryReturnHome
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_transport_action(Unit* unit)
	{
		const u32 Func_AI_TransportAction = 0x0043DB50;
		static s32 result;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_AI_TransportAction
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_ask_for_help(Unit* self, Unit* attacker, s32 onlyNearbyGuards)
	{
		const u32 Func_AI_AskForHelp = 0x0043ADA0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH onlyNearbyGuards
			PUSH attacker
			PUSH self
			CALL Func_AI_AskForHelp
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s16 get_region_id_from_unit(Unit* unit)
	{
		const u32 Func_getRegionIdFromUnit = 0x0049CB40;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_getRegionIdFromUnit
			MOV result, EAX
			POPAD
		}
		return (s16)result;
	}

	s32 get_enemy_air_strength_in_region(u16 regionId, s32 player)
	{
		const u32 Func_getEnemyAirStrengthInRegion = 0x00431D00;
		static s32 result;
		__asm {
			PUSHAD
			PUSH player
			MOVZX AX, regionId
			CALL Func_getEnemyAirStrengthInRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_enemy_strength_in_region(u16 regionId, s32 player, s32 includeAir)
	{
		const u32 Func_getEnemyStrengthInRegion = 0x004318E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH includeAir
			PUSH player
			MOVZX AX, regionId
			CALL Func_getEnemyStrengthInRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_is_performing_unbreakable_code(Unit* unit)
	{
		const u32 Func_unitIsPerformingUnbreakableCode = 0x00402310;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitIsPerformingUnbreakableCode
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool ai_cast_spell_behavior(Unit* unit, bool isAggro)
	{
		const u32 Func_AI_CastSpellBehavior = 0x004A13C0;
		static u32 result;
		__asm {
			PUSHAD
			PUSH isAggro
			MOV EAX, unit
			CALL Func_AI_CastSpellBehavior
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 ai_flee(Unit* unit, Unit* enemy)
	{
		const u32 Func_AI_Flee = 0x0043E400;
		static s32 result;
		__asm {
			PUSHAD
			PUSH enemy
			MOV EAX, unit
			CALL Func_AI_Flee
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unit_has_ground_path(Unit* unit, s16 x, s16 y)
	{
		const u32 Func_unitHasGroundPath = 0x0042FA00;
		static u32 result;
		__asm {
			PUSHAD
			PUSH y
			MOVZX DX, x
			MOV EAX, unit
			CALL Func_unitHasGroundPath
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool ai_has_no_defense_build(s32 player)
	{
		const u32 Func_AI_HasNoDefenseBuild = 0x00447370;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL Func_AI_HasNoDefenseBuild
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unit_can_see_cloaked_target(Unit* target, Unit* unit)
	{
		const u32 Func_unitCanSeeCloakedTarget = 0x00401D60;
		static u32 result;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, target
			CALL Func_unitCanSeeCloakedTarget
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void ai_respond_to_cloaked_threat(Unit* threat, Unit* unit)
	{
		const u32 Func_AI_RespondToCloakedThreat = 0x0043C580;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, threat
			CALL Func_AI_RespondToCloakedThreat
			POPAD
		}
	}

	s32 ai_update_attack_target(Unit* unit, s32 acceptIfSieged, s32 acceptCritters, s32 mustReach)
	{
		const u32 Func_AI_UpdateAttackTarget = 0x00477160;
		static s32 result;
		__asm {
			PUSHAD
			PUSH mustReach
			PUSH acceptCritters
			PUSH acceptIfSieged
			MOV ECX, unit
			CALL Func_AI_UpdateAttackTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void orderInterrupt(Unit* unit, u8 orderId, Vector32* pos, Unit* target)
	{
		const u32 Func_orderInterrupt = 0x00474C30;
		__asm {
			PUSHAD
			PUSH target
			PUSH pos
			PUSH orderId
			MOV ESI, unit
			CALL Func_orderInterrupt
			POPAD
		}
	}

	// for iterate_building_ai

	u16 ai_remove_from_towns(Unit* worker)
	{
		const u32 Func_AI_RemoveFromTowns = 0x00432760;
		static u16 result;
		__asm {
			PUSHAD
			MOV ECX, worker
			CALL Func_AI_RemoveFromTowns
			MOV result, AX
			POPAD
		}
		return result;
	}

	s32 ai_get_queued_count(Unit* builder, u32 unitId)
	{
		const u32 Func_AI_GetQueuedCount = 0x00466B70;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, builder
			MOV EDI, unitId
			CALL Func_AI_GetQueuedCount
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void prepare_guard_ai(u16 y, u32 player, u16 unitId, u16 x)
	{
		const u32 Func_AI_PrepareGuardAI = 0x00462670;
		__asm {
			PUSHAD
			PUSH x
			PUSH unitId
			MOV ESI, player
			MOVZX DI, y
			CALL Func_AI_PrepareGuardAI
			POPAD
		}
	}

	// AI attack wave extender

	// for AI_TrainAttackForce
	s32 ai_add_to_region_military(AiRegion* region, s32 unitId, s32 priority)
	{
		const u32 Func_AI_AddRegionToMilitary = 0x0043E2E0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH priority
			PUSH unitId
			MOV EAX, region
			CALL Func_AI_AddRegionToMilitary
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_add_cost_to_needs(u32 unitId, u32 playerId, u32 type)
	{
		const u32 Func_AI_FindUnit = 0x00447DC0;
		static u32 typeXD = type;
		static u32 result;
		__asm {
			PUSHAD
			PUSH typeXD
			MOV EBX, playerId
			MOV EAX, unitId
			CALL Func_AI_FindUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_add_spending_request(u16 unitId, u32 player, u8 type, char priority, void* value)
	{
		const u32 Func_AI_AddSpendingRequest = 0x00447980;
		u32 type32 = type;
		static u32 result;
		__asm {
			PUSHAD
			PUSH value
			PUSH priority
			MOV ECX, type32
			MOV EDX, player
			MOV AX, unitId
			CALL Func_AI_AddSpendingRequest
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 AI_AddBaseUnitSpendingRequest(u32 unitId, u32 player, AiRegion* region, char priority)
	{
		const u32 Func_AI_AddBaseUnitSpendingRequest = 0x004484A0;
		static u32 result;
		__asm {
			PUSHAD
			PUSH priority
			MOV ESI, region
			MOV ECX, player
			MOV EAX, unitId
			CALL Func_AI_AddBaseUnitSpendingRequest
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for AIRegion_GroupForAttack

	u32 ai_recalculate_region_strength(AiRegion* region)
	{
		const u32 Func_AI_RecalculateRegionStrength = 0x0043A390;
		static u32 result;
		__asm {
			PUSHAD
			MOV ESI, region
			CALL Func_AI_RecalculateRegionStrength
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* AI_TargetEnemyInRegion(AiRegion* region)
	{
		const u32 Func_AI_TargetEnemyInRegion = 0x0043CC40;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EDI, region
			CALL Func_AI_TargetEnemyInRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 AI_MoveMilitaryToDefend(AiRegion* region)
	{
		const u32 Func_AI_MoveMilitaryToDefend = 0x0043DE40;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDI, region
			CALL Func_AI_MoveMilitaryToDefend
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void AI_GetRegionMilitaryTypes(AiRegion* region, s32* hasGroundMilitary,
		s32* cantAttackGround, s32* cantAttackAir)
	{
		const u32 Func_AI_GetRegionMilitaryTypes = 0x00438FC0;
		__asm {
			PUSHAD
			MOV EAX, region
			MOV EBX, hasGroundMilitary
			MOV EDI, cantAttackGround
			MOV ESI, cantAttackAir
			CALL Func_AI_GetRegionMilitaryTypes
			POPAD
		}
	}

	s32 AI_CanTravelToRegion(AiRegion* region1, AiRegion* region2)
	{
		const u32 Func_AI_CanTravelToRegion = 0x0043A790;
		static s32 result;
		__asm {
			PUSHAD
			PUSH region2
			PUSH region1
			CALL Func_AI_CanTravelToRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* AI_PickAttackTarget_NoBuildings(AiRegion* region)
	{
		const u32 Func_AI_PickAttackTarget_NoBuildings = 0x0043A510;
		static Unit* result;
		__asm {
			PUSHAD
			PUSH region
			CALL Func_AI_PickAttackTarget_NoBuildings
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 AI_MoveMilitaryToOtherRegion(AiRegion* target, AiRegion* source, u32 pullUnitsNearRegion,
		u32 sendPureSpellcasters, u32 dontSendFromBunkers)
	{
		const u32 Func_AI_MoveMilitaryToOtherRegion = 0x0043E580;
		static u32 result;
		__asm {
			PUSHAD
			PUSH dontSendFromBunkers
			PUSH sendPureSpellcasters
			PUSH pullUnitsNearRegion
			MOV ESI, source
			MOV EDI, target
			CALL Func_AI_MoveMilitaryToOtherRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	AiTown* AI_IterateTownSomething(AiRegion* region)
	{
		const u32 Func_AI_IterateTownSomething = 0x0043DD20;
		static AiTown* result;
		__asm {
			PUSHAD
			MOV ESI, region
			CALL Func_AI_IterateTownSomething
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 AI_AttackTimerDecrement(u32 player)
	{
		const u32 Func_AI_AttackTimerDecrement = 0x00447090;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, player
			CALL Func_AI_AttackTimerDecrement
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 AI_HasForcesMovingToAttack(u32 player)
	{
		const u32 Func_AI_HasForcesMovingToAttack = 0x0043A2E0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL Func_AI_HasForcesMovingToAttack
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for ai_attack_manager

	u16 PopulateRegionsWithOwn(char* regionData, u32 player)
	{
		const u32 Func_PopulateRegionsWithOwn = 0x00438B30;
		static u32 result;
		__asm {
			PUSHAD
			PUSH player
			MOV EBX, regionData
			CALL Func_PopulateRegionsWithOwn
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	void PopulateRegionsWithEnemy(u32 player, char* regionData)
	{
		const u32 Func_PopulateRegionsWithEnemy = 0x00439B50;
		__asm {
			PUSHAD
			PUSH regionData
			PUSH player
			CALL Func_PopulateRegionsWithEnemy
			POPAD
		}
	}

	s32 AI_GetNeighborRegionTypes(char* regionData)
	{
		const u32 Func_AI_GetNeighborRegionTypes = 0x00436D80;
		static s32 result;
		__asm {
			PUSHAD
			MOV EBX, regionData
			CALL Func_AI_GetNeighborRegionTypes
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void AI_FinishRegionAccessibility(char* regionData)
	{
		const u32 Func_AI_FinishRegionAccessibility = 0x00436CF0;
		__asm {
			PUSHAD
			MOV EBX, regionData
			CALL Func_AI_FinishRegionAccessibility
			POPAD
		}
	}

	s32 AI_SetAttackGroupingRegion(u32 airAttack, u32 alsoVeryNear, u32 player, char* regionData,
		s32 x, s32 y, u32 alwaysOverwriteLastAttack)
	{
		const u32 Func_AI_SetAttackGroupingRegion = 0x004399D0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH alwaysOverwriteLastAttack
			PUSH y
			PUSH x
			PUSH regionData
			PUSH player
			MOV EDX, alsoVeryNear
			MOV ECX, airAttack
			CALL Func_AI_SetAttackGroupingRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unitCantSeeCloakedTarget(Unit* unit, Unit* target)
	{
		const u32 Func_unitCantSeeCloakedTarget = 0x00401D60;
		static b32 bPreResult;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, target
			CALL Func_unitCantSeeCloakedTarget
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool unitCanInfest(Unit* unit)
	{
		const u32 Func_CanInfest = 0x00402750;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_CanInfest
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	void AI_AttackUnit(Unit* unit)
	{
		const u32 Func_AI_AttackUnit = 0x0043FFD0;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_AI_AttackUnit
			POPAD
		}
	}

	Unit* findBestAttackTarget(Unit* unit)
	{
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

	bool Ai_TryReturnHome(Unit* unit, u8 unknownByteValue)
	{
		const u32 Func_Sub462EA0 = 0x00462EA0;
		static u32 _unknownByteValue;
		static b32 bPreResult;
		_unknownByteValue = unknownByteValue;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH _unknownByteValue
			CALL Func_Sub462EA0
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	void function_00465780(Unit* unit)
	{
		const u32 Func_Sub465780 = 0x00465780;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub465780
			POPAD
		}
	}

	Order* remove_order_from_unit_queue(Order* order, Unit* unit)
	{
		static Order* result;
		const u32 Func_removeOrderFromUnitQueue = 0x004742D0;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EAX, order
			CALL Func_removeOrderFromUnitQueue
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void prepend_order_targeting_unit(Unit* unit, Unit* target, u8 orderId)
	{
		const u32 Func_prependOrderTargetingUnit = 0x00474A70;
		__asm {
			PUSHAD
			MOV CL, orderId
			MOV EDX, unit
			MOV EAX, target
			CALL Func_prependOrderTargetingUnit
			POPAD
		}
	}

	bool is_target_within_min_movement_range(Unit* unit, Unit* target, u32 range)
	{
		const u32 Func_isTargetWithinMinMovementRange = 0x004763D0;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOV EAX, target
			MOV ECX, range
			CALL Func_isTargetWithinMinMovementRange
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool function_00476610(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_Sub476610 = 0x00476610;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EBX, x
			MOV EDI, y
			CALL Func_Sub476610
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool isUnitInWeaponRange(Unit* unit, Unit* target)
	{
		const u32 Func_isUnitInWeaponRange = 0x00476870;
		static b32 bPreResult;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, target
			CALL Func_isUnitInWeaponRange
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	void getWeaponBeginIscript(Unit* unit, IscriptAnimation::Enum anim)
	{
		const u32 Func_getWeaponBeginIscript = 0x00476ED0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV DL, anim
			CALL Func_getWeaponBeginIscript
			POPAD
		}
	}

	void PickNewAttackTargetIfNeeded(Unit* unit)
	{
		const u32 Func_Sub477510 = 0x00477510;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_Sub477510
			POPAD
		}
	}

	void function_00477820(Unit* unit, u8 orderId)
	{
		const u32 Func_Sub477820 = 0x00477820;
		static u32 orderId_;
		orderId_ = orderId;
		__asm {
			PUSHAD
			PUSH orderId_
			MOV EAX, unit
			CALL Func_Sub477820
			POPAD
		}
	}

	bool function_00478370(Unit* unit, u8 orderId)
	{
		const u32 Func_Sub478370 = 0x00478370;
		static b32 bPreResult;
		static u32 orderId_;
		orderId_ = orderId;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH orderId_
			CALL Func_Sub478370
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool Order_AttackUnit_Main(Unit* unit)
	{
		const u32 Func_Sub478540 = 0x00478540;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub478540
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	// Identical to Unit::isCritter() @ 0x0047B450
	bool isUnitCritter(Unit* unit)
	{
		bool bReturnValue;
		/*
		if(
				unit->id == UnitId::Critter_Rhynadon ||
				unit->id == UnitId::Critter_Bengalaas ||
				unit->id == UnitId::Critter_Ragnasaur ||
				unit->id == UnitId::Critter_Kakaru ||
				unit->id == UnitId::Critter_Scantid ||
				unit->id == UnitId::Critter_Ursadon
			) {
				bReturnValue = true;
			}
			else
			*/
		bReturnValue = false;
		return bReturnValue;
	}

	void moveToCollide(Unit* unit, Unit* target)
	{
		const u32 Func_moveToCollide = 0x00495400;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOV EAX, target
			CALL Func_moveToCollide
			POPAD
		}
	}

	bool isUnitVisible(Unit* unit)
	{
		const u32 Func_isUnitVisible = 0x004E5DB0;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_isUnitVisible
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool isTargetVisible(Unit* unit, Unit* target)
	{
		const u32 Func_isTargetVisible = 0x004E5E30;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EAX, target
			CALL Func_isTargetVisible
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	// Identical to getRightClickActionOrder @ 0x004E5EA0;
	u8 getRightClickActionOrder(Unit* unit)
	{
		u8 result;
		if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
		{
			result = RightClickActions::NoMove_NormalAttack;
		}
		else
		{
			u8 temp_result = units_dat::RightClickAction[unit->id];
			if (
				temp_result == 0 &&
				unit->status & UnitStatus::GroundedBuilding &&
				unit->isFactory()
				)
			{
				result = RightClickActions::NormalMove_NoAttack;
			}
			else
			{
				result = temp_result;
			}
		}
		return result;
	}

	// Not related with assigning orders, but only with destinations of orders
	void make_to_hold_position(Unit* unit)
	{
		const u32 Func_OrdersHoldPositionSuicidal = 0x004EB5B0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_OrdersHoldPositionSuicidal
			POPAD
		}
	}

	bool move_to_target(Unit* target, Unit* unit)
	{
		const u32 Func_moveToTarget = 0x004EB720;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EDI, target
			CALL Func_moveToTarget
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool set_move_target_xy(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_SetMoveTarget_xy = 0x004EB820;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EBX, x
			MOV EDI, y
			CALL Func_SetMoveTarget_xy
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	// Related to path/movement decision
	void set_resource_target(Unit* unit)
	{
		const u32 Func_setNextWaypoint = 0x004EB290;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_setNextWaypoint
			POPAD
		}
	}

	bool change_movement_target_move_target_2(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_changeMovementTargetMoveTarget2 = 0x004EB9C0;
		static b32 bPreResult;
		__asm
		{
			PUSHAD
			MOV EDX, unit
			MOV ECX, x
			MOV EAX, y
			CALL Func_changeMovementTargetMoveTarget2
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool is_distance_greater_than_halt_distance(Unit* unit, s32 x, s32 y, u32 distance)
	{
		const u32 Func_isDistanceGreaterThanHaltDistance = 0x00401240;
		static b32 bPreResult;
		__asm
		{
			PUSHAD
			PUSH x
			PUSH distance
			MOV EAX, y
			MOV ECX, unit
			CALL Func_isDistanceGreaterThanHaltDistance
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	void check_unstack(Unit* unit)
	{
		const u32 Func_checkUnstack = 0x0042D9A0;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_checkUnstack
			POPAD
		}
	}

	bool begin_building(Unit* unit)
	{
		const u32 Func_beginBuilding = 0x00467030;
		static b32 bPreResult;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_beginBuilding
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	Unit* begin_gas_building(Unit* unit, u32 unitId)
	{
		const u32 Func_beginGasBuilding = 0x004678A0;
		static Unit* newUnit;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			PUSH unitId
			CALL Func_beginGasBuilding
			MOV newUnit, EAX
			POPAD
		}
		return newUnit;
	}

	// imageId may not be really a parameter
	void addOverlayBelowMain(Sprite* sprite, u32 imageId, u32 x_offset, u32 y_offset, u32 direction)
	{
		const u32 Func_addOverlayBelowMain = 0x00498D70;
		__asm
		{
			PUSHAD
			MOV EAX, sprite
			MOV ESI, imageId
			PUSH x_offset
			PUSH y_offset
			PUSH direction
			CALL Func_addOverlayBelowMain
			POPAD
		}
	}

	void replace_sprite_images(Sprite* sprite, u32 imageId, u32 imageDirection)
	{
		const u32 Func_Sub499BB0 = 0x00499BB0;
		__asm
		{
			PUSHAD
			PUSH imageDirection
			PUSH imageId
			MOV EAX, sprite
			CALL Func_Sub499BB0
			POPAD
		}
	}

	void display_last_network_error_for_player(u32 playerId)
	{
		const u32 Func_displayLastNetErrorForPlayer = 0x0049E530;
		__asm
		{
			PUSHAD
			PUSH playerId
			CALL Func_displayLastNetErrorForPlayer
			POPAD
		}
	}

	void update_unit_strength(Unit* unit)
	{
		const u32 Func_UpdateUnitStrength = 0x0049FA40;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_UpdateUnitStrength
			POPAD
		}
	}

	void iscript_opcode_cases(Image* image, u32 offset_, u32 unk1, u32 unk2)
	{
		const u32 Func_iscript_OpcodeCases = 0x004D74C0;
		__asm
		{
			PUSHAD
			PUSH unk1
			PUSH unk2
			PUSH offset_
			MOV ECX, image
			CALL Func_iscript_OpcodeCases
			POPAD
		}
	}

	void switch_to_warp_texture(Image* image)
	{
		const u32 Func_switchToWarpTexture = 0x004D8500;
		__asm
		{
			PUSHAD
			MOV EAX, image
			CALL Func_switchToWarpTexture
			POPAD
		}
	}

	bool set_thingy_visibility_flags(Thingy* thingy)
	{
		const u32 Func_SetThingyVisibilityFlags = 0x004878F0;
		static b32 bPreResult;
		__asm
		{
			PUSHAD
			MOV ESI, thingy
			CALL Func_SetThingyVisibilityFlags
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	Thingy* create_thingy(u32 spriteId, s16 x, s16 y, u32 playerId)
	{
		const u32 Func_CreateThingy = 0x00488210;
		static Thingy* thingy;
		s32 x_ = x;
		__asm
		{
			PUSHAD
			PUSH playerId
			MOVSX EDI, y
			PUSH x_
			PUSH spriteId
			CALL Func_CreateThingy
			MOV thingy, EAX
			POPAD
		}
		return thingy;
	}

	void playSoundAtPos(u32 soundId, u16 x, u16 y, u32 unk1, u32 unk2)
	{
		const u32 Func_playUnitSFX = 0x0048EC10;
		static Point16 pos;
		pos.x = x; pos.y = y;
		__asm
		{
			PUSHAD
			PUSH unk2
			PUSH unk1
			PUSH pos
			MOV EBX, soundId
			CALL Func_playUnitSFX
			POPAD
		}
	}

	u32 randomize_short(u32 unknown_index)
	{
		const u32 Func_RandomizeShort = 0x004DC4A0;
		static u32 return_value;
		__asm
		{
			PUSHAD
			MOV EAX, unknown_index
			CALL Func_RandomizeShort
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void setUnitStatTxtErrorMsg(char* message)
	{
		const u32 Func_setUnitStatTxtErrorMsg = 0x0048CCB0;
		__asm
		{
			PUSHAD
			MOV EAX, message
			CALL Func_setUnitStatTxtErrorMsg
			POPAD
		}
	}

	void move_unit_duplicate(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_moveUnit_Duplicate = 0x004EBAE0;
		__asm
		{
			PUSHAD
			MOV EDX, unit
			MOV EAX, x
			MOV ECX, y
			CALL Func_moveUnit_Duplicate
			POPAD
		}
	}

	Unit* SpiderMine_AcquireTarget(Unit* spiderMine)
	{
		const u32 Func_SpiderMine_AcquireTarget = 0x00441270;
		static Unit* target;
		__asm {
			PUSHAD
			MOV ESI, spiderMine
			CALL Func_SpiderMine_AcquireTarget
			MOV target, EAX
			POPAD
		}
		return target;
	}

	void instantCloak(Unit* unit)
	{
		const u32 Func_instantCloak = 0x0049B1E0;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_instantCloak
			POPAD
		}
	}

	void unburrowGeneric(Unit* unit)
	{
		const u32 Func_unburrowGeneric = 0x004E99D0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unburrowGeneric
			POPAD
		}
	}

	void burrowGeneric(Unit* unit)
	{
		const u32 Func_burrowGeneric = 0x004E9A30;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_burrowGeneric
			POPAD
		}
	}

	// For orders_PlaceMine

	s32 techUseAllowed(Unit* a1, __int16 a2, s32 a3)
	{
		const u32 Func_techUseAllowed = 0x0046DD80;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX DI, a2
			PUSH a3
			CALL Func_techUseAllowed
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_distance_fast(s32 a1, s32 a2, s32 a3, s32 a4)
	{
		const u32 Func_getDistanceFast = 0x0040C360;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			PUSH a3
			PUSH a4
			CALL Func_getDistanceFast
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 orders_HoldPosSuicidal(Unit* a1)
	{
		const u32 Func_orders_HoldPosSuicidal = 0x004EB5B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_orders_HoldPosSuicidal
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void turnUnit(Unit* a1, char a2)
	{
		const u32 Func_turnUnit = 0x00495F20;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_turnUnit
			POPAD
		}
	}

	void to_idle(Unit* a1)
	{
		const u32 Func_toIdle = 0x004753A0;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_toIdle
			POPAD
		}
	}

	s32 is_unit_movable(Unit* a1)
	{
		const u32 Func_isUnitMovable = 0x00401DC0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isUnitMovable
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void orders_nothing_2(Unit* a1)
	{
		const u32 Func_orders_Nothing2 = 0x00475000;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_orders_Nothing2
			POPAD
		}
	}

	bool set_move_target_xy(s32 a1, s32 a2, Unit* a3)
	{
		const u32 Func_setMoveTargetXY = 0x004EB820;
		static s32 result;
		__asm {
			PUSHAD
			MOV EBX, a1
			MOV EDI, a2
			MOV ESI, a3
			CALL Func_setMoveTargetXY
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	// For burrowGeneric

	void toggle_unit_path(Unit* unit)
	{
		const u32 Func_toggleUnitPath = 0x004E42A0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_toggleUnitPath
			POPAD
		}
	}

	char switchSpriteId(Sprite* sprite, s32 spriteId)
	{
		const u32 Func_switchSpriteId = 0x00499860;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, spriteId
			MOV ECX, sprite
			CALL Func_switchSpriteId
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void iscript_OpcodeCases_copy(Image* image, u32 offset_, u32 unk1, u32 unk2)
	{
		const u32 Func_iscript_OpcodeCases = 0x004D74C0;
		__asm {
			PUSHAD
			PUSH unk1
			PUSH unk2
			PUSH offset_
			MOV ECX, image
			CALL Func_iscript_OpcodeCases
			POPAD
		}
	}

	void replaceSpriteImages_copy(Sprite* sprite, u32 imageId, u32 imageDirection)
	{
		const u32 Func_Sub499BB0 = 0x00499BB0;
		__asm {
			PUSHAD
			PUSH imageDirection
			PUSH imageId
			MOV EAX, sprite
			CALL Func_Sub499BB0
			POPAD
		}
	}

	void refundBuildingCost(u16 unitId, u8 playerId)
	{
		const u32 Func_refundBuildingCost = 0x0042CE70;
		__asm {
			PUSHAD
			MOV AX, unitId
			MOV CL, playerId
			CALL Func_refundBuildingCost
			POPAD
		}
	}

	void updateBuildingsAI(Unit* unit)
	{
		const u32 Func_updateBuildingsAI = 0x00433FE0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateBuildingsAI
			POPAD
		}
	}

	//Identical to isValidMorph @ 0x0045CC60;
	bool isValidMorph(u16 buildingId)
	{
		bool result;
		if (scbw::is_zerg_building_morph_result(buildingId))
		{
			result = true;
		}
		else
		{
			result = false;
		}
		return result;
	}

	void SpreadInitialCreep(u32 id, s32 x, s32 y)
	{
		const u32 Func_updateNewUnitVision = 0x0045CE90;
		__asm {
			PUSHAD
			MOV ECX, id
			PUSH y
			PUSH x
			CALL Func_updateNewUnitVision
			POPAD
		}
	}

	void zergPlaceBuildingCntd(Unit* unit)
	{
		const u32 Func_ZergPlaceBuildingCntd = 0x0045D410;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_ZergPlaceBuildingCntd
			POPAD
		}
	}

	void stopCreepDisappearingAtBuilding(Unit* unit)
	{
		const u32 Func_stopCreepDisappearingAtBuilding = 0x0047D770;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_stopCreepDisappearingAtBuilding
			POPAD
		}
	}

	void updateCreepDisappearance(Sprite* sprite, u32 unitId, s32 x, s32 y)
	{
		const u32 Func_updateCreepDisappearance = 0x0047DE40;
		__asm {
			PUSHAD
			MOV ECX, y
			MOV EDX, x
			MOV ESI, sprite
			PUSH unitId
			CALL Func_updateCreepDisappearance
			POPAD
		}
	}

	//unk1 is ECX, unk2 is pushed value
	void increment_unit_scores_ex(Unit* building, s32 unk1, s32 unk2)
	{
		const u32 Func_incrementUnitScoresEx = 0x00488D50;
		__asm {
			PUSHAD
			MOV EDI, building
			MOV ECX, unk1
			PUSH unk2
			CALL Func_incrementUnitScoresEx
			POPAD
		}
	}

	void play_building_completed_sound(Unit* building)
	{
		const u32 Func_PlayBuildingCompleteSound = 0x0048F310;
		__asm {
			PUSHAD
			MOV EAX, building
			CALL Func_PlayBuildingCompleteSound
			POPAD
		}
	}

	void refreshSpriteData(Sprite* sprite)
	{
		const u32 Func_refreshSpriteData = 0x004983A0;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_refreshSpriteData
			POPAD
		}
	}

	void replace_unit_with_type(Unit* unit, u16 newUnitId)
	{
		const u32 Func_ReplaceUnitWithType = 0x0049FED0;
		u32 newUnitId_ = newUnitId;
		__asm {
			PUSHAD
			PUSH newUnitId_
			MOV EAX, unit
			CALL Func_ReplaceUnitWithType
			POPAD
		}
	}

	void fix_target_location(Point16* coords, u32 unitId)
	{
		const u32 Func_fixTargetLocation = 0x00401FA0;
		__asm {
			PUSHAD
			MOV EAX, unitId
			MOV EDX, coords
			CALL Func_fixTargetLocation
			POPAD
		}
	}

	// Equivalent to function @ 0x00402160
	bool is_unit_positions_2_equal(Unit* unit)
	{
		return (unit->next_target_waypoint.x == unit->position.x && unit->next_target_waypoint.y == unit->position.y);
	}

	// if canBeAborted is TRUE, the unit can be aborted because there's
	// no room around the constructing building
	b32 progress_build(Unit* unit, u32 hpGain, b32 showHidden)
	{
		const u32 Func_buildingAddon = 0x004679A0;
		static b32 bResult;
		__asm
		{
			PUSHAD
			MOV EDX, hpGain
			MOV EAX, unit
			PUSH showHidden
			CALL Func_buildingAddon
			MOV bResult, EAX
			POPAD
		}
		return bResult;
	}

	void prepend_order_targeting_nothing(Unit* unit, Order* order, u8 orderId)
	{
		const u32 Func_prependOrderTargetingNothing = 0x00474760;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EDI, order
			MOV BL, orderId
			CALL Func_prependOrderTargetingNothing
			POPAD
		}
	}

	void add_order_targeting_nothing(Unit* unit, Order* order, u8 orderId)
	{
		const u32 Func_addOrderTargetingNothing = 0x004749A0;
		__asm {
			PUSHAD
			MOV BL, orderId
			MOV ESI, unit
			MOV EDI, order
			CALL Func_addOrderTargetingNothing
			POPAD
		}
	}

	bool can_still_place_building()
	{
		const u32 Func_canStillPlaceBuilding = 0x0048DDA0;
		b32 pre_return_value;
		__asm {
			PUSHAD
			CALL Func_canStillPlaceBuilding
			MOV pre_return_value, EAX
			POPAD
		}
		return (pre_return_value != 0);
	}

	void end_building_placement()
	{
		const u32 Func_endBuildingPlacement = 0x0048E310;
		__asm {
			PUSHAD
			CALL Func_endBuildingPlacement
			POPAD
		}
	}

	u32 rand_between(u32 min, u32 max, u32 someIndex)
	{
		const u32 Func_RandBetween = 0x004DC550;
		static u32 return_value;
		__asm {
			PUSHAD
			PUSH max
			MOV EAX, someIndex
			MOV EDX, min
			CALL Func_RandBetween
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	char refresh_layers_3_and_4()
	{
		const u32 Func_RefreshLayer3And4 = 0x0048D9A0;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_RefreshLayer3And4
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void init_building(Unit* unit, b32 flag)
	{
		const u32 Func_initBuilding = 0x004E65E0;
		__asm {
			PUSHAD
			MOV EDI, unit
			PUSH flag
			CALL Func_initBuilding
			POPAD
		}
	}

	bool order_to_move_to_target(Unit* unit, Unit* target)
	{
		const u32 Func__moveToTarget = 0x004EB980;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, target
			MOV ECX, unit
			CALL Func__moveToTarget
			MOV bPreResult, EAX
			POPAD
		}
		return bPreResult != 0;
	}

	b32 unk_terrain_collision_2(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_unk_terrainCollision2 = 0x004F1870;
		static b32 rValue;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			MOV EAX, unit
			CALL Func_unk_terrainCollision2
			MOV rValue, EAX
			POPAD
		}
		return rValue;
	}

	// For update_placement_state_for_placement_box

	s32 can_build_on_tile_location(Unit* builder, s32 playerId, s32 xTile, s32 yTile, s32 unitType, s32 placementStateEntry, s32 bCheckVision, s32 bAlsoInvisible, s32 bWithoutVision)
	{
		const u32 Func_canBuildOnTileLocation = 0x00473FB0;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH bWithoutVision
			PUSH bAlsoInvisible
			PUSH bCheckVision
			PUSH placementStateEntry
			PUSH unitType
			PUSH yTile
			PUSH xTile
			PUSH playerId
			PUSH builder
			CALL Func_canBuildOnTileLocation
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For refresh_screen

	s32 update_placement_state_for_placement_box()
	{
		const u32 Func_updatePlacementStateForPlacementBox = 0x0048DCE0;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_updatePlacementStateForPlacementBox
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 unk_building_placement_related(s32 a1, Point16* a2)
	{
		const u32 Func_unk_buildingPlacementRelated = 0x0048D7F0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a2
			MOV EAX, a1
			CALL Func_unk_buildingPlacementRelated
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 init_placement_box_pos(Point16* in_out)
	{
		const u32 Func_initPlacementBoxPos = 0x0048D660;
		static s32 result;
		__asm {
			PUSHAD
//			MOV ECX, a1
			MOV EDX, in_out
			CALL Func_initPlacementBoxPos
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char unk_building_placement_related_mouse(s32 a1, s32 a2)
	{
		const u32 Func_unk_buildingPlacementRelatedMouse = 0x0048D7B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL Func_unk_buildingPlacementRelatedMouse
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void setUnitPathing(Unit* unit, u8 unkPathRelated)
	{
		const u32 Func_SetUnitPathing = 0x00401360;
		__asm {
			PUSHAD
			MOV AL, unkPathRelated
			MOV ECX, unit
			CALL Func_SetUnitPathing
			POPAD
		}
	}

	Unit* connectAddonCheck(Unit* unit)
	{
		const u32 Func_ConnectAddonCheck = 0x004404A0;
		Unit* found_addon;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_ConnectAddonCheck
			MOV found_addon, EAX
			POPAD
		}
		return found_addon;
	}

	void modify_building_grounded_status(Unit* unit)
	{
		const u32 Func_Sub463470 = 0x00463470;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub463470
			POPAD
		}
	}

	void toggle_pathing_related(Unit* unit)
	{
		const u32 Func_Sub4634E0 = 0x004634E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4634E0
			POPAD
		}
	}

	void flying_building_lift_if_still_blocked(Unit* unit)
	{
		const u32 Func_flying_building_lift_if_still_blocked = 0x00463640;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_flying_building_lift_if_still_blocked
			POPAD
		}
	}

	void completeAddon(Unit* unit, Unit* addon)
	{
		const u32 Func_CompleteAddon = 0x00463D50;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOV EAX, addon
			CALL Func_CompleteAddon
			POPAD
		}
	}

	void crush_immobile_units(Unit* unit)
	{
		const u32 Func_Sub464300 = 0x00464300;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_Sub464300
			POPAD
		}
	}

	void disconnectFromAddOn(Unit* unit)
	{
		const u32 Func_Sub464930 = 0x00464930;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub464930
			POPAD
		}
	}

	void clear_building_from_tile_flag(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_Sub469EC0 = 0x00469EC0;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH y
			PUSH x
			CALL Func_Sub469EC0
			POPAD
		}
	}

	void SetBuildingTileFlag(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_Sub469F60 = 0x00469F60;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH y
			PUSH x
			CALL Func_Sub469F60
			POPAD
		}
	}

	void end_collision_tracking(Unit* unit)
	{
		const u32 Func_end_collision_tracking = 0x0046A560;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_end_collision_tracking
			POPAD
		}
	}

	void start_collision_tracking(Unit* unit)
	{
		const u32 Func_start_collision_tracking = 0x0046A5A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_start_collision_tracking
			POPAD
		}
	}

	s32 modifyCreepState(s32 x_tile, s32 y_tile, CreepChange* creepChange, u32 param)
	{
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

	void force_order_done(Unit* unit)
	{
		const u32 Func_force_order_done = 0x00475420;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_force_order_done
			POPAD
		}
	}

	bool update_creep_disappearance_unit(Unit* unit)
	{
		const u32 Func_Sub47DF90 = 0x0047DF90;
		b32 pre_return_value;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_Sub47DF90
			MOV pre_return_value, EAX
			POPAD
		}

		return (pre_return_value != 0);
	}

	void update_creep_disappearance_unit_id(u32 unit_id, u32 x, u32 y, b32 completed)
	{
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

	void placebuildingProc()
	{
		const u32 Func_PracebuildingProc = 0x0048E6E0;
		__asm {
			PUSHAD
			CALL Func_PracebuildingProc
			POPAD
		}
	}

	void playBuildingLandSound(Unit* unit)
	{
		const u32 Func_PlayBuildingLandSound = 0x0048F5A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_PlayBuildingLandSound
			POPAD
		}
	}

	void SetUnitMovementSpeed(Unit* unit, u32 newSpeed)
	{
		const u32 Func_SetUnitMovementSpeed = 0x004951C0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV EDX, newSpeed
			CALL Func_SetUnitMovementSpeed
			POPAD
		}
	}

	void set_shadow_y(Sprite* sprite, u8 newVerticalOffset)
	{
		const u32 Func_Sub498150 = 0x00498150;
		__asm {
			PUSHAD
			MOV EAX, sprite
			MOV CL, newVerticalOffset
			CALL Func_Sub498150
			POPAD
		}
	}

	void create_landing_dust(Sprite* sprite)
	{
		const u32 Func_Sub4997E0 = 0x004997E0;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_Sub4997E0
			POPAD
		}
	}

	void function_004C3B40(Unit* unit)
	{
		const u32 Func_Sub4C3B40 = 0x004C3B40;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_Sub4C3B40
			POPAD
		}
	}

	b32 spreads_creep(u32 unitId, b32 defaultReturn)
	{
		const u32 Func_spreadsCreep = 0x00413870;
		b32 bReturnValue;
		__asm {
			PUSHAD
			MOV EAX, unitId
			PUSH defaultReturn
			CALL Func_spreadsCreep
			MOV bReturnValue, EAX
			POPAD
		}
		return bReturnValue;
	}

	void applyCreepAtLocationFromUnitType(s32 x, s32 y, u32 unitId)
	{
		const u32 Func_applyCreepAtLocationFromUnitType = 0x004148F0;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			PUSH unitId
			CALL Func_applyCreepAtLocationFromUnitType
			POPAD
		}
	}

	void CreateBuildingLarva(Unit* main_building)
	{
		const u32 Func_Sub49D660 = 0x0049D660;
		__asm {
			PUSHAD
			MOV ESI, main_building
			CALL Func_Sub49D660
			POPAD
		}
	}

	b32 show_finished_unit(Unit* unit)
	{
		const u32 Func_Sub49EC30 = 0x0049EC30;
		b32 bResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_Sub49EC30
			MOV bResult, EAX
			POPAD
		}
		return bResult;
	}

	Unit* FindBestUnit(Box16* coords, u32 searchProc)
	{
		const u32 Func_FindBestUnit = 0x004E8830;
		static Unit* unit;
		__asm {
			PUSHAD
			MOV EAX, coords
			MOV EDI, searchProc
			CALL Func_FindBestUnit
			MOV unit, EAX
			POPAD
		}
		return unit;
	}
	/* !!! WARNING !!! Use EXTREME caution when calling this.
	There is absolutely no guarantee that it will find the intended unit,
	or that it will find a unit at all */
	Unit* getUnitPointer()
	{
		static Unit* unit = nullptr;
		__asm {
			XOR EAX, EAX
			SuperFunUnitFinder :
			MOV EDX, DWORD PTR[EBP + EAX * 4]
				INC EAX
				CMP EDX, 0x0059CCA8
				JB SuperFunUnitFinder
				CMP EDX, 0x006283E8
				JNB SuperFunUnitFinder
				MOV unit, EDX
		}
		return unit;
	}

	u8 spreadCreep(u32 unitId, s32 x, s32 y)
	{
		const u32 Func_Sub414680 = 0x00414680;
		static u8 return_value;
		__asm {
			PUSHAD
			MOV ECX, x
			MOV EAX, y
			MOV EDX, unitId
			CALL Func_Sub414680
			MOV return_value, AL
			POPAD
		}
		return return_value;
	}

	void function_004E8DA0(Unit* unit)
	{
		const u32 Func_Sub4E8DA0 = 0x004E8DA0;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub4E8DA0
			POPAD
		}
	}

	bool getLarvaSpawnPos(Unit* unit, Vector32* coords)
	{
		const u32 Func_getLarvaSpawnPos = 0x004E8E10;
		static b32 return_pre_value;
		__asm {
			PUSHAD
			MOV EDI, coords
			MOV EAX, unit
			CALL Func_getLarvaSpawnPos
			MOV return_pre_value, EAX
			POPAD
		}
		return (return_pre_value != 0);
	}

	u32 getNumSurroundingCreepTiles(u32 x, u32 y)
	{
		const u32 Func_getNumSurroundingCreepTiles = 0x00414290;
		static u32 return_val;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			CALL Func_getNumSurroundingCreepTiles
			MOV return_val, EAX
			POPAD
		}
		return return_val;
	}

	void setCreepRelationalData(u32 x, u32 y, setCreepRelationalData_callback callback, u32 arg)
	{
		const u32 Func_setCreepRelationalData = 0x00413FA0;
		__asm {
			PUSHAD
			PUSH arg
			PUSH callback
			PUSH y
			PUSH x
			CALL Func_setCreepRelationalData
			POPAD
		}
	}

	u32 iterateDirectionalCreepData(u32 x, u32 y, u32 callback, u32 arg)
	{
		const u32 Func_iterateDirectionalCreepData = 0x00413B30;
		static u32 return_val;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			MOV EBX, callback
			MOV EDI, arg
			CALL Func_iterateDirectionalCreepData
			MOV return_val, EAX
			POPAD
		}
		return return_val;
	}

	void function_004E97C0(Unit* unit)
	{
		const u32 Func_Sub_4E97C0 = 0x004E97C0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub_4E97C0
			POPAD
		}
	}

	// Note: this function was made from what was seen in this case of use, some things are hardcoded because it's hard to tell what is used or not.
	// Basically, used elsewhere, the function would be implemented differently
	void TargetedOrderCopy(Unit* unit, u32 unitId, u32 unk1, u32 unk2, u32 orderId, u32 unk3, u32 unk4, u32 unk5)
	{
		const u32 Func_TargetedOrderCopy = 0x004754F0;
		__asm
		{
			PUSHAD
			MOV EAX, 0
			MOV EDX, 0
			MOV EBX, unitId
			MOV ESI, unit
			PUSH unk1
			PUSH unk2
			PUSH unk3
			PUSH orderId
			PUSH unk4
			PUSH unk5
			CALL Func_TargetedOrderCopy
			POPAD
		}
	}

	void clearUnitSprites(Sprite* unit_sprite, Sprite* subunit_sprite)
	{
		const u32 Func_clearUnitSprites = 0x00496F00;
		__asm
		{
			PUSHAD
			MOV ECX, subunit_sprite
			MOV ESI, unit_sprite
			CALL Func_clearUnitSprites
			POPAD
		}
	}

	void function_00497480(Sprite* thingySprite, u32 visibility_flags)
	{
		const u32 Func_Sub497480 = 0x00497480;
		__asm
		{
			PUSHAD
			MOV EBX, thingySprite
			PUSH visibility_flags
			CALL Func_Sub497480
			POPAD
		}
	}

	void _UnitDestructor(Unit* unit)
	{
		const u32 Func__UnitDestructor = 0x004A0740;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func__UnitDestructor
			POPAD
		}
	}

	void UnitDestructor(Unit* unit)
	{
		const u32 Func_UnitDestructor = 0x004A0990;
		__asm
		{
			PUSHAD
			MOV ECX, unit
			CALL Func_UnitDestructor
			POPAD
		}
	}

	void hide_and_disable_unit(Unit* unit)
	{
		const u32 Func_unitDeathSomething_0 = 0x004E6340;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_unitDeathSomething_0
			POPAD
		}
	}

	void CancelResearch_Proc(Unit* unit)
	{
		const u32 Func_Sub453E30 = 0x00453E30;
		__asm
		{
			PUSHAD
			MOV EDX, unit
			CALL Func_Sub453E30
			POPAD
		}
	}

	void cancel_upgrade_proc(Unit* unit)
	{
		const u32 Func_Sub454280 = 0x00454280;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub454280
			POPAD
		}
	}

	void refund_queue_slot(Unit* unit, u16 slot)
	{
		const u32 Func_refundQueueSlot = 0x00466A70;
		__asm
		{
			PUSHAD
			MOV EDI, unit
			MOVZX EAX, slot
			CALL Func_refundQueueSlot
			POPAD
		}
	}

	void cancel_last_training(Unit* unit)
	{
		const u32 Func_Sub466E40 = 0x00466E40;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub466E40
			POPAD
		}
	}

	void cancel_addon_proc(Unit* unit)
	{
		const u32 Func_Sub4E66E0 = 0x004E66E0;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4E66E0
			POPAD
		}
	}

	b32 order_is_allowed(Unit* unit, u16 order, u32 nationID)
	{
		const u32 Func_OrderAllowed = 0x0046DC20;
		static b32 bResult;
		__asm
		{
			PUSHAD
			MOV BX, order
			MOV EAX, unit
			PUSH nationID
			CALL Func_OrderAllowed
			MOV bResult, EAX
			POPAD
		}
		return bResult;
	}

	bool place_building_message(u32 result_function_473FB0)
	{
		const u32 Func_placeBuildingMsg = 0x0048D930;
		static b32 bPreResult;
		__asm
		{
			PUSHAD
			MOV EAX, result_function_473FB0
			CALL Func_placeBuildingMsg
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool CMDRECV_place_building_allowed(Unit* unit, u8 orderId, u16 builtUnitId)
	{
		const u32 Func_CMDRECV_PlaceBuildingAllowed = 0x0048DBD0;
		static b32 bPreResult;
		__asm
		{
			PUSHAD
			MOV AX, builtUnitId
			MOV DL, orderId
			MOV ECX, unit
			CALL Func_CMDRECV_PlaceBuildingAllowed
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	void function_0048DE70(Unit* unit, u32 orderId, u16 dimensionX, u16 dimensionY, u32 builtUnitId)
	{
		const u32 Func_Sub48DE70 = 0x0048DE70;
		static Point16 dim;
		dim.x = dimensionX; dim.y = dimensionY;
		__asm
		{
			PUSHAD
			PUSH builtUnitId
			PUSH dim
			PUSH orderId
			MOV EAX, unit
			CALL Func_Sub48DE70
			POPAD
		}
	}

	void CMDRECV_place_building(s16 x, s16 y, u8 orderId, u16 builtUnitId)
	{
		const u32 Func_cmdRECV_PlaceBuilding = 0x0048E190;
		static Point16 pos;
		pos.x = x; pos.y = y;
		__asm
		{
			PUSHAD
			MOVZX EAX, builtUnitId	// bugfix: should have been "MOV AX, builtUnitId" but for some reason it crashed
			MOV CL, orderId
			PUSH pos
			CALL Func_cmdRECV_PlaceBuilding
			POPAD
		}
	}

	Unit* templar_merge_partner(Unit* unit, Unit** units_list, u32 units_list_length)
	{
		const u32 Func_templarMergePartner = 0x004BFA80;
		static Unit* return_value;
		__asm
		{
			PUSHAD
			MOV EAX, units_list_length
			PUSH units_list
			PUSH unit
			CALL Func_templarMergePartner
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void parse_orders_dat_requirements()
	{
		const u32 Func_parseOrdersDatReqs = 0x0046D450;
		__asm
		{
			PUSHAD
			CALL Func_parseOrdersDatReqs
			POPAD
		}
	}

	s32 parse_requirement_opcodes(Unit* unit, u32 orderId, u32 playerId, u32 address, u32 req_offset)
	{
		const u32 Func_parseRequirementOpcodes = 0x0046D610;
		static s32 return_value;
		__asm
		{
			PUSHAD
			MOV EAX, req_offset
			MOV ESI, unit
			PUSH address
			PUSH playerId
			PUSH orderId
			CALL Func_parseRequirementOpcodes
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void apply_speed_upgrade_from_unit_type(Unit* unit)
	{
		const u32 Func_apply_speed_upgrade_from_unit_type = 0x00454370;
		__asm
		{
			PUSHAD
			MOV ESI, unit
			CALL Func_apply_speed_upgrade_from_unit_type
			POPAD
		}
	}

	void give_ai(Unit* unit)
	{
		const u32 func_address = 0x00463040;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	void recheck_attacks(Unit* unit)
	{
		const u32 Func_recheck_attacks = 0x00476D30;
		__asm
		{
			PUSHAD
			MOV EDI, unit
			CALL Func_recheck_attacks
			POPAD
		}
	}

	void increment_unit_scores(Unit* unit, s32 unkScore)
	{
		const u32 Func_incrementUnitScores = 0x00488BF0;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			PUSH unkScore
			CALL Func_incrementUnitScores
			POPAD
		}
	}

	void create_unit_selection(Unit* unit, u32 selectionIndex)
	{
		const u32 Func_CreateUnitSelection = 0x004E6180;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH selectionIndex
			CALL Func_CreateUnitSelection
			POPAD
		}
	}

	void destroy_unit_selection(Unit* unit)
	{
		const u32 Func_DestroyUnitSelection = 0x004E6290;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_DestroyUnitSelection
			POPAD
		}
	}

	void drop_powerup(Unit* powerup)
	{
		const u32 Func_drop_powerup = 0x004F3B70;
		__asm {
			PUSHAD
			PUSH powerup
			CALL Func_drop_powerup
			POPAD
		}
	}

	u32 get_upgraded_weapon_cooldown(Unit* unit, u8 weaponId)
	{
		const u32 Func_getUpgradedWpnCooldown = 0x00475DC0;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV AL, weaponId
			MOV ESI, unit
			CALL Func_getUpgradedWpnCooldown
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	bool is_ready_to_attack(Unit* unit, u32 weaponId)
	{
		const u32 Func_is_ready_to_attack = 0x00476640;
		static b32 pre_result;
		__asm {
			PUSHAD
			PUSH weaponId
			MOV EAX, unit
			CALL Func_is_ready_to_attack
			MOV pre_result, EAX
			POPAD
		}
		return (pre_result != 0);
	}

	u32 get_stat_txt_string(Unit* unit, u32 playerId, char* message)
	{
		const u32 Func_get_statTxt_Str_0 = 0x0048EF30;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV EAX, message
			MOV ECX, playerId
			MOV EDX, unit
			CALL Func_get_statTxt_Str_0
			MOV EAX, return_value
			POPAD
		}
		return return_value;
	}

	bool can_cast_spell(Unit* unit)
	{
		const u32 Func_canCastSpell_0 = 0x00492140;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_canCastSpell_0
			MOV bPreResult, EAX
			POPAD
		}
		// 0 means "can cast spell"
		return (bPreResult == 0);
	}

	u32 ordered_spell_progress_moving(Unit* unit, b32 wasMainOrderStateZero)
	{
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

	void refund_all_queue_slots(Unit* unit)
	{
		const u32 Func_RefundAllQueueSlots = 0x00466E80;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RefundAllQueueSlots
			POPAD
		}
	}

	void increment_unit_death_scores(Unit* unit, u8 player)
	{
		const u32 Func_IncrementUnitDeathScores = 0x00488AF0;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOVZX EDX, player
			CALL Func_IncrementUnitDeathScores
			POPAD
		}
	}

	void initialize_empty_unit_linked_list_references(Unit* unit, u8 playerId)
	{
		const u32 Func_initializeEmptyUnitsLinkedListRef = 0x0049E4E0;
		__asm {
			PUSHAD
			MOV DL, playerId
			MOV ECX, unit
			CALL Func_initializeEmptyUnitsLinkedListRef
			POPAD
		}
	}

	void give_unit_helper(Unit* unit, u32 playerId, s32 unkScore)
	{
		const u32 Func_GiveUnitHelper = 0x0049EFA0;
		__asm {
			PUSHAD
			PUSH unkScore
			MOV ECX, unit
			PUSH playerId
			CALL Func_GiveUnitHelper
			POPAD
		}
	}

	void change_unit_buttonset(Unit* unit, u16 buttonSetId)
	{
		const u32 Func_changeUnitButtonSet = 0x004E5D60;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV CX, buttonSetId
			CALL Func_changeUnitButtonSet
			POPAD
		}
	}

	// What make the target reappear
	void show_and_enable_unit(Unit* unit)
	{
		const u32 Func_InitUnitTrapDoodad = 0x004E6490;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_InitUnitTrapDoodad
			POPAD
		}
	}

	bool is_infestable_unit(Unit* unit)
	{
		const u32 Func_IsInfestable = 0x00402210;
		static b32 result;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_IsInfestable
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	// For update_unit_strength_and_apply_default_orders
	void anim_cloak(Unit* a1, s32 a2)
	{
		const u32 Func_AnimCloak = 0x0049B5B0;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_AnimCloak
			POPAD
		}
	}

	void refresh_unit_vision(Unit* a1)
	{
		const u32 Func_refreshUnitVision = 0x004E5F30;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_refreshUnitVision
			POPAD
		}
	}

	void init_unit_trap_doodad(Unit* a1)
	{
		const u32 Func_initUnitTrapDoodad = 0x004E6490;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_initUnitTrapDoodad
			POPAD
		}
	}

	Order* perform_another_order(u8 order_id, Order* order, Unit* unit, s16 fow_unit_id, Point16 position_xy, Unit* target)
	{
		const u32 Func_performAnotherOrder = 0x004745F0;
		static Order* result;
		__asm {
			PUSHAD
			MOVZX BL, order_id
			MOV EDI, order
			MOV ESI, unit
			MOVZX DX, fow_unit_id
			PUSH position_xy
			PUSH target
			CALL Func_performAnotherOrder
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void order(s32 a1, Unit* a2, s8 a3, Vector32* a4, Unit* a5, s32 a6)
	{
		const u32 Func_order = 0x00474810;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV EDX, a2
			MOVZX CL, a3
			PUSH a4
			PUSH a5
			PUSH a6
			CALL Func_order
			POPAD
		}
	}

	u32 get_unit_strength(Unit* a1, s32 a2, bool a3)
	{
		const u32 Func_getUnitStrength = 0x00431800;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			PUSH a3
			CALL Func_getUnitStrength
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void play_worker_error_sound(Unit* unit, u32 errorTextIndex)
	{
		const u32 Func_PlayWorkerErrorSound = 0x0048F6C0;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH errorTextIndex
			CALL Func_PlayWorkerErrorSound
			POPAD
		}
	}

	bool is_unit_upgrade_available(Unit* unit)
	{
		const u32 Func_isUnitUpgradeAvailable = 0x004033D0;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_isUnitUpgradeAvailable
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	void apply_speed_upgrade_from_upgrade_id(Unit* unit, u8 upgradeType)
	{
		const u32 Func_ApplySpeedUpgradeFromUpgradeType = 0x00454540;
		__asm {
			PUSHAD
			MOV AL, upgradeType
			PUSH unit
			CALL Func_ApplySpeedUpgradeFromUpgradeType
			POPAD
		}
	}

	void play_upgrade_complete_sound()
	{
		const u32 Func_playUpgradeCompleteSound = 0x0048F070;
		__asm {
			PUSHAD
			CALL Func_playUpgradeCompleteSound
			POPAD
		}
	}

	void play_research_complete_sound()
	{
		const u32 Func_playResearchCompleteSound = 0x0048F150;
		__asm {
			PUSHAD
			CALL Func_playResearchCompleteSound
			POPAD
		}
	}

	void set_upgrade_level(u32 playerId, u32 upgradeType, u8 upgradeLevel)
	{
		const u32 Func_setUpgradeLevel = 0x004CE770;
		__asm {
			PUSHAD
			MOV DL, upgradeLevel
			MOV ECX, upgradeType
			MOV EAX, playerId
			CALL Func_setUpgradeLevel
			POPAD
		}
	}

	// return true if remainingBuildTime was 0 when called, remainingBuildTime change affected by cheating
	bool advance_remaining_build_time(Unit* unit)
	{
		u32 Func_advanceRemainingBuildTime = 0x00466940;
		static b32 bTimeWasZero;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_advanceRemainingBuildTime
			MOV bTimeWasZero, EAX
			POPAD
		}
		return (bTimeWasZero != 0);
	}

	// return value may be useless since it's the exact value of pathingFlags
	u8 delete_pathing(Unit* unit)
	{
		u32 Func_Sub46A820 = 0x0046A820;
		static u8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub46A820
			MOV result, AL
			POPAD
		}

		return result;
	}

	void order_immediate(Unit* unit, u8 order)
	{
		u32 Func_OrderImmediate = 0x00474B40;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV AL, order
			CALL Func_OrderImmediate
			POPAD
		}
	}

	void play_morphing_complete_sound(Unit* unit)
	{
		const u32 Func_PlayMorphingCompleteSound = 0x0048F440;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_PlayMorphingCompleteSound
			POPAD
		}
	}

	// get status (defensiveMatrixHp to acidSporeTime) of unit_src and give it to unit_dst
	void merge_status_into_unit(Unit* unit_dst, Unit* unit_src)
	{
		const u32 Func_Sub493180 = 0x00493180;
		__asm {
			PUSHAD
			MOV ECX, unit_src
			MOV EAX, unit_dst
			CALL Func_Sub493180
			POPAD
		}
	}

	void anim_decloak(Unit* unit, u32 soundId)
	{
		const u32 Func_animDecloak = 0x0049B440;
		__asm {
			PUSHAD
			MOV EBX, soundId
			MOV EAX, unit
			CALL Func_animDecloak
			POPAD
		}
	}

	// return true if waiting for recentOrderTimer to reach 0 or if setMoveTarget_xy returned true
	bool delayed_set_move_target_xy(Unit* unit, u16 x, u16 y)
	{
		const u32 Func_Sub4EB960 = 0x004EB960;
		b32 bPreResult;
		__asm {
			PUSHAD
			MOVSX EDI, y
			MOVSX EBX, x
			MOV EAX, unit
			CALL Func_Sub4EB960
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	void set_all_overlay_directions_generic(Unit* unit, u32 unk)
	{
		const u32 Func_setAllOverlayDirectionsGeneric = 0x00401140;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unk
			CALL Func_setAllOverlayDirectionsGeneric
			POPAD
		}

	}

	void ai_order_unit_cloaking(Unit* unit)
	{
		const u32 Func_AI_orderUnitCloaking = 0x0043B970;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_AI_orderUnitCloaking
			POPAD
		}
	}

	u32 ai_return_to_nearest_base_forced(Unit* unit)
	{
		const u32 Func_Sub43DB50 = 0x0043DB50;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_Sub43DB50
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	s32 get_unit_max_weapon_range(Unit* a1, unsigned __int8 a2)
	{
		const u32 Func_getUnitMaxWeaponRange = 0x00475870;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_getUnitMaxWeaponRange
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for isReadyToAttack

	u32 getDistanceToUnit(Unit* a1, Unit* a2)
	{
		const u32 Func_getDistanceToUnit = 0x00430D30;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_getDistanceToUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 distanceFromDboxCorner(Unit* a1, __int16 a2, __int16 a3)
	{
		const u32 Func_distanceFromDboxCorner = 0x0043E50;
		static u32 result;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_distanceFromDboxCorner
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool isAngleInAttackAngle(s32 a1, s32 a2, Unit* a3, unsigned __int8 a4)
	{
		const u32 Func_isAngleInAttackAngle = 0x00475BE0;
		static s32 result;
		static bool result2;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV ESI, a3
			CALL Func_isAngleInAttackAngle
			MOV result, EAX
			POPAD
		}
		return result2 = (bool)result;
	}

	// for canCastOrderedSpellNow

	bool is_target_within_min_range(Unit* unit, u32 minRange, Unit* target)
	{
		const u32 Func_isTargetWithinMinRange = 0x00430F10;
		static s32 result;
		static bool result2;

		__asm {
			PUSHAD
			PUSH target
			PUSH minRange
			MOV ECX, unit
			CALL Func_isTargetWithinMinRange
			MOV result, EAX
			POPAD
		}
		return result = (bool)result;
	}

	s32 getFlingyHaltDistance(s32 a1)
	{
		const u32 Func_getFlingyHaltDistaince = 0x00494F90;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_getFlingyHaltDistaince
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for ordersWatchTarget

	void unknownOrdersFunc(s32 a1, __int16 a2, __int16 a3)
	{
		const u32 Func_unknownOrdersFunc = 0x00475350;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EAX, a1
			CALL Func_unknownOrdersFunc
			POPAD
		}
	}

	// for ordersStayInRange

	void prependOrderTargetingGround(__int8 order_id, __int16 x, __int16 y, Unit* unit)
	{
		const u32 Func_prependOrderTargetingGround = 0x00474A20;
		__asm {
			PUSHAD
			PUSH unit
			MOVZX CX, y
			MOVZX DX, x
			MOVZX AL, order_id
			CALL Func_prependOrderTargetingGround
			POPAD
		}
	}

	s32 moveTowards(Unit* unit, Unit* target)
	{
		const u32 Func_moveTowards = 0x004EB900;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EAX, target
			CALL Func_moveTowards
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ordersHoldPositionSuicidal(Unit* a1)
	{
		const u32 Func_ordersHoldPositionSuicidal = 0x004EB5B0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_ordersHoldPositionSuicidal
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 getDirectionFromPoints(s32 a1, s32 a2, s32 a3, s32 a4)
	{
		const u32 Func_getDirectionFromPoints = 0x00495300;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV EDX, a2
			MOV ECX, a3
			CALL Func_getDirectionFromPoints
			MOV result, EAX
			POPAD
		}
		return result;
	}
	
	void set_secondary_waypoint(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_setSecondaryWaypoint = 0x00494BB0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV ECX, y
			MOV EDX, x
			MOV EAX, unit
			CALL Func_setSecondaryWaypoint
			MOV result, EAX
			POPAD
		}
	}

	Vector32 setResourceTarget(Unit* a1)
	{
		const u32 Func_setResourceTarget = 0x004EB290;
		static __int32 result;
		static Vector32 resultPoint;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_setResourceTarget
			MOV result, EAX
			POPAD
		}
		return resultPoint = *(Vector32*)&result;
	}

	// for genericRightClickUnitProc2

	s32 isOrderValid(char a1, Unit* a2, s32 a3, Unit* a4, s32 a5)
	{
		const u32 Func_isOrderValid = 0x00474D90;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a5
			MOV ESI, a4
			MOV EDI, a3
			MOV EBX, a2
			MOVZX AL, a1
			CALL Func_isOrderValid
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u16 getUnitAbilityErrorStatString(Unit* a1, char a2, s32 a3)
	{
		const u32 Func_getUnitAbilityErrorStatString = 0x00491E80;
		static u32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOVZX BL, a2
			MOV EAX, a1
			CALL Func_getUnitAbilityErrorStatString
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	// for CMDRECV_HoldPosition

	Unit* getActivePlayerFirstSelection()
	{
		const u32 Func_getActivePlayerFirstSelection = 0x0049A850;
		static Unit* result;
		__asm {
			PUSHAD
			CALL Func_getActivePlayerFirstSelection
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 orderAllowed(Unit* a1, u16 a2, s32 a3)
	{
		const u32 Func_orderAllowed = 0x0046DC20;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOVZX BX, a2
			MOV EAX, a1
			CALL Func_orderAllowed
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void targetedOrder(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, char a7, s32 a8, s32 a9, s32 a10)
	{
		const u32 Func_targetedOrder = 0x004754F0;
		__asm {
			PUSHAD
			PUSH a10
			PUSH a9
			PUSH a8
			PUSH a7
			PUSH a6
			PUSH a5
			MOVZX AX, a4
			MOV ESI, a3
			MOV EBX, a2
			MOVZX DX, a1
			CALL Func_targetedOrder
			POPAD
		}
	}

	// For check_unit_visibility

	u32 get_detecting_players(Unit* unit)
	{
		const u32 Func_getDetectingPlayers = 0x00441130;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_getDetectingPlayers
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 iteratePlayerUnitsSomething(u32 aResult, Unit* unit)
	{
		const u32 Func_iteratePlayerUnitsSomething = 0x0049E8A0;
		static u32 result;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, aResult
			CALL Func_iteratePlayerUnitsSomething
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void makeSpriteVisible(Sprite* sprite)
	{
		const u32 Func_makeSpriteVisible = 0x00497ED0;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_makeSpriteVisible
			POPAD
		}
	}

	s32 updateBuildingLandUnitSelection(u32 unused, Unit* selection)
	{
		const u32 Func_updateBuildingLandUnitSelection = 0x004C3B40;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, unused
			MOV EDX, selection
			CALL Func_updateBuildingLandUnitSelection
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char removeSelectionCircle(Sprite* sprite)
	{
		const u32 Func_removeSelectionCircle = 0x004975D0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, sprite
			CALL Func_removeSelectionCircle
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	Image* makeSpriteInvisible(Sprite* sprite)
	{
		const u32 Func_makeUndetected = 0x00498A10;
		static Image* result;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_makeUndetected
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void function_004756B0(Unit* unit, u8 orderId, u32 unk1, u32 unk2, u32 bCommandType)
	{
		const u32 Func_Sub4756B0 = 0x004756B0;
		__asm {
			PUSHAD
			MOV DL, orderId
			MOV ESI, unit
			PUSH unk1
			PUSH unk2
			PUSH bCommandType
			CALL Func_Sub4756B0
			POPAD
		}
	}

	void function_004756E0(Unit* unit, Unit* target, u32 orderId, u32 bCommandType)
	{
		const u32 Func_Sub4756E0 = 0x004756E0;
		__asm {
			PUSHAD
			MOV EDX, target
			MOV ESI, unit
			PUSH orderId
			PUSH bCommandType
			CALL Func_Sub4756E0
			POPAD
		}
	}

	u32 function_0049A410(Unit* unit, u8 orderId)
	{
		const u32 Func_Sub49A410 = 0x0049A410;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV DL, orderId
			CALL Func_Sub49A410
			MOV result, EAX
			POPAD
		}
		return result;
	}

	//Logically identical to 0049A480 sub_49A480
	bool CanTargetSelf(Unit* unit, u8 orderId)
	{
		bool return_value;
		if (
			unit->status & UnitStatus::GroundedBuilding &&
			(orderId == OrderId::RallyPoint1 || orderId == OrderId::RallyPoint2)
			)
			return_value = true;
		else
			if (
				(unit->id == UnitId::ZergIsthrathaleth || unit->id == UnitId::Hero_UncleanOne) &&
				orderId == OrderId::DarkSwarm
				)
				return_value = true;
			else
				if (unit->status & UnitStatus::IsHallucination)
					return_value = false;
				else
					if (
						unit->id == UnitId::ZergOvileth &&
						UpgradesSc->currentLevel[unit->player][UpgradeId::VentralSacs] == 0
						)
						return_value = false;
					else
						if (units_dat::SpaceProvided[unit->id] == 0)
							return_value = false;
						else
							if (orderId == OrderId::MoveUnload)
								return_value = true;
							else
								return_value = false;
		return return_value;
	}

	void function_0049A500(Unit* unit, u8* array_of_data)
	{
		const u32 Func_Sub49A500 = 0x0049A500;
		__asm {
			PUSHAD
			PUSH array_of_data
			PUSH unit
			CALL Func_Sub49A500
			POPAD
		}
	}

	void function_0049A8C0(u8* array_of_data, b32 bCanBeObstructed)
	{
		const u32 Func_Sub49A8C0 = 0x0049A8C0;
		__asm {
			PUSHAD
			PUSH bCanBeObstructed
			MOV EDI, array_of_data
			CALL Func_Sub49A8C0
			POPAD
		}
	}

	void IgnoreAllScriptAndGotoIdle(Unit* unit)
	{
		const u32 Func_IgnoreAllScriptAndGotoIdle = 0x00476550;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_IgnoreAllScriptAndGotoIdle
			POPAD
		}
	}

	u32 canUnload(Unit* transport, Unit* loaded_unit, Point16* pos)
	{
		const u32 Func_canUnload = 0x004E76C0;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, transport
			MOV ESI, loaded_unit
			PUSH pos
			CALL Func_canUnload
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void function_004E7E10(Unit* unit, u32 unk)
	{
		const u32 Func_Sub4E7E10 = 0x004E7E10;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unk
			CALL Func_Sub4E7E10
			POPAD
		}
	}

	void StatsNukesCount(BinDlg* dialog, u32 index)
	{
		const u32 Func_Sub425310 = 0x00425310;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_Sub425310
			POPAD
		}
	}

	void StatHangerCount(BinDlg* dialog, u32 index)
	{
		const u32 Func_StatHangerCount = 0x004253D0;
		__asm {
			PUSHAD
			MOV EBX, dialog
			PUSH index
			CALL Func_StatHangerCount
			POPAD
		}
	}

	void StatsShieldLevel(BinDlg* dialog, u32 index)
	{
		const u32 Func_Sub425510 = 0x00425510;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_Sub425510
			POPAD
		}
	}

	void StatsArmorLevel(BinDlg* dialog, u32 index)
	{
		const u32 Func_Sub425600 = 0x00425600;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_Sub425600
			POPAD
		}
	}

	void StatsWeaponLevel(BinDlg* dialog, u32 index, u32 weaponId)
	{
		const u32 Func_Sub425790 = 0x00425790;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH weaponId
			PUSH index
			CALL Func_Sub425790
			POPAD
		}
	}

	bool StatSpidermineCount(BinDlg* dialog, u32 index)
	{
		const u32 Func_StatSpidermineCount = 0x00426300;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_StatSpidermineCount
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	// For get_unit_rank_string

	__int8 unitGetRace(Unit* a1)
	{
		const u32 Func_unitGetRace = 0x0047B150;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitGetRace
			MOV result, EAX
			POPAD
		}
		return (__int8)result;
	}

	char* getStatTxtString(u16 a1)
	{
		const u32 Func_getStatTxtString = 0x004C36F0;
		static s32 result;
		__asm {
			PUSHAD
			MOVZX CX, a1
			CALL Func_getStatTxtString
			MOV result, EAX
			POPAD
		}
		return (char*)result;
	}

	// For stats_weapon_level / stats_armor_level

	char getArmorCount(Unit* unit, s32 unused)
	{
		const u32 Func_getArmorCount = 0x00453FC0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, unused
			MOV EAX, unit
			CALL Func_getArmorCount
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void show_dialog(BinDlg* dialog)
	{
		const u32 Func_showDialog = 0x004186A0;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_showDialog
			POPAD
		}
	}

	s32 disable_dialog(BinDlg* dialog)
	{
		const u32 Func_disableDialog = 0x00418640;
		static s32 result;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_disableDialog
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void update_dialog(BinDlg* dialog)
	{
		const u32 Func_updateDialog = 0x0041C400;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL Func_updateDialog
			POPAD
		}
	}

	Unit* findRandomAttackTarget(Unit* unit)
	{
		const u32 Func_findRandomAttackTarget = 0x00442FC0;
		static Unit* target;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_findRandomAttackTarget
			MOV target, EAX
			POPAD
		}
		return target;
	}

	b32 attackApplyCooldown(Unit* unit)
	{
		const u32 Func_attackApplyCooldown = 0x00478B40;
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_attackApplyCooldown
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void setImageDirection(Image* image, s8 direction)
	{
		const u32 Func_SetImageDirection = 0x004D5F80;
		static u32 direction_;
		direction_ = direction;
		__asm {
			PUSHAD
			PUSH direction_
			MOV ESI, image
			CALL Func_SetImageDirection
			POPAD
		}
	}

	void ApplySpores(Unit* unit)
	{
		const u32 Func_ApplySpores = 0x004F4480;
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL Func_ApplySpores
			POPAD
		}
	}

	// creates attack overlay & deals damage from the bullet (just calls overlayAndNotify if the weapon deals no damage, or calls target->damage_with if it does
	void weaponBulletShot(Unit* target, Bullet* bullet, u32 dmgDivisor)
	{
		const u32 Helper_WeaponBulletShot = 0x00479AE0;
		if (bullet != nullptr && target != nullptr && bullet->sourceUnit && (bullet->weaponId == WeaponId::PariahIconoclastBlades || bullet->weaponId == WeaponId::StrigaPhaseSaber))
		{
			if (!globals.was_hit_by_pariah(bullet->sourceUnit, target))
			{
				globals.register_pariah_hit(bullet->sourceUnit, target);
			}
			else
			{
				return;
			}
		}
		__asm {
			PUSHAD
			PUSH dmgDivisor
			MOV EAX, target
			MOV EDX, bullet
			CALL Helper_WeaponBulletShot
			POPAD
		}
	}

	u8 set_archon_pathing(Unit* unit)
	{
		const u32 Func_Sub46A820 = 0x0046A820;
		static u8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub46A820
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool has_supplies_for_unit(u32 unitId, u32 playerId, b32 canShowErrorMessage)
	{
		return 1; // Supply was removed

		const u32 Func_hasSuppliesForUnit = 0x0042CF70;
		static b32 bPreResult;
		__asm
		{
			PUSHAD
			PUSH canShowErrorMessage
			PUSH unitId
			PUSH playerId
			CALL Func_hasSuppliesForUnit
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	Unit* zerg_dual_birth(Unit* unit)
	{
		const u32 Func_zergDualBirth = 0x0045D910;
		static Unit* rValue;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_zergDualBirth
			MOV rValue, EAX
			POPAD
		}
		return rValue;
	}

	void order_new_unit_to_rally(Unit* unit, Unit* factory)
	{
		const u32 Func_order_new_unit_to_rally = 0x00466F50;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, factory
			CALL Func_order_new_unit_to_rally
			POPAD
		}
	}

	void ai_training_unit(Unit* unit, Unit* creator)
	{
		const u32 Func_AI_TrainingUnit = 0x004A2830;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, creator
			CALL Func_AI_TrainingUnit
			POPAD
		}
	}

	// for orders_Hallucination1
	bool spell_order(Unit* unit, u32 techId, u16* techEnergyCost, u32 sightRange, u32 error_message_index)
	{
		const u32 Func_spellOrder = 0x004926D0;
		static b32 bPreResult;
		__asm {
			PUSHAD
			PUSH techEnergyCost
			PUSH sightRange
			PUSH techId
			MOV EAX, error_message_index
			MOV EBX, unit
			CALL Func_spellOrder
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool place_hallucination(Unit* unit)
	{
		const u32 Func_placeHallucination = 0x004F66D0;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_placeHallucination
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	// for createHallucination

	void apply_hallucination_stats(Unit* halUnit)
	{
		const u32 Func_applyHallucinationStats = 0x004F6180;
		__asm {
			PUSHAD
			MOV EAX, halUnit
			CALL Func_applyHallucinationStats
			POPAD
		}
	}

	s32 hallucinate_scarabs(Unit* a1, Unit* a2)
	{
		const u32 Func_hallucinateScarabs = 0x004F65B0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a2
			MOV ESI, a1
			CALL Func_hallucinateScarabs
			MOV EAX, result
			POPAD
		}
		return result;
	}

	void replace_feedback_sprite(Unit* unit, u32 spriteId)
	{
		const u32 Func_replaceFeedbackSprite = 0x004F6350;
		__asm {
			PUSHAD
			MOV EBX, unit
			PUSH spriteId
			CALL Func_replaceFeedbackSprite
			POPAD
		}
	}

	void AI_unit_was_hit(u32 main_target_reaction, u32 dont_call_help, Unit* attacker, Unit* target)
	{
		const u32 Func_Ai_UnitWasHit = 0x0043F320;
		__asm {
			PUSHAD
			PUSH main_target_reaction
			PUSH dont_call_help
			MOV ECX, target
			MOV EDX, attacker
			CALL Func_Ai_UnitWasHit
			POPAD
		}
	}

	// checks if the distance between target and bullet's positions is < radius
	bool isUnitDistanceWithin(Unit* target, Bullet* bullet, u32 radius)
	{
		const u32 Helper_IsUnitDistanceWithin = 0x0048ADB0;
		u32 result;
		__asm {
			PUSHAD
			PUSH radius;
			PUSH bullet;
			MOV ECX, target
				CALL Helper_IsUnitDistanceWithin
				MOV result, EAX
				POPAD
		}
		return result;
	}

	void doWeaponHit(Unit* target, Unit* attacker, u8 dmgDivisor)
	{
		const u32 Helper_DoWeaponHit = 0x004795D0;
		u32 divisor = dmgDivisor;
		__asm {
			PUSHAD
			PUSH divisor
			PUSH attacker
			MOV EBX, target
			CALL Helper_DoWeaponHit
			POPAD
		}
	}

	// for airSplash

	Unit* findBestUnit(Box16* radius, s32 bullet, s32(*func)(Unit*, Bullet*))
	{
		const u32 Func_findBestUnit = 0x004E8830;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, radius
			MOV EBX, bullet
			MOV EDI, func
			CALL Func_findBestUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 splashFunc(Bullet* bullet, Unit* unit)
	{
		const u32 Func_splashFunc = 0x0048B570;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDI, bullet
			MOV ESI, unit
			CALL Func_splashFunc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void MindControlHit(Unit* attacker, Unit* target)
	{
		const u32 Func_MindControl_Hit = 0x004F6910;
		__asm {
			PUSHAD
			MOV ESI, target
			PUSH attacker
			CALL Func_MindControl_Hit
			POPAD
		}
	}

	//Somehow related to AI stuff; details unknown.
	u16 get_unit_strength(Unit* unit, bool useGroundStrength)
	{
		const u32 Helper_GetUnitStrength = 0x00431800;
		static u16 strength;
		static u32 useGroundStrength_;
		useGroundStrength_ = useGroundStrength ? 1 : 0;
		__asm {
			PUSHAD
			PUSH useGroundStrength_
			MOV EAX, unit
			CALL Helper_GetUnitStrength
			MOV strength, AX
			POPAD
		}
		return strength;
	}

	// Hallucination-related
	// Moved here instead of including an entire other file...
	Unit* createHallucination(Unit* target, u32 playerId)
	{
		const u32 Func_createHallucination = 0x004F6B90;
		static Unit* return_value;
		__asm {
			PUSHAD
			PUSH playerId
			MOV ECX, target
			CALL Func_createHallucination
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	// For updateCorrosiveSpores
	u32 getCorrosiveSporesImage(Unit* unit)
	{
		const u32 Func_getCorrosiveSporesImage = 0x004F41F0;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_getCorrosiveSporesImage
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void insertFirstOrder(Unit* unit, u8 orderId)
	{
		const u32 Helper_InsertFirstOrder = 0x004749D0;
		__asm {
			PUSHAD
			MOV AL, orderId
			MOV ECX, unit
			CALL Helper_InsertFirstOrder
			POPAD
		}
	}

	/// Return the count of overlays
	u8 has_overlay(Unit* unit)
	{
		const u32 Func_hasOverlay = 0x0047B720;
		static u8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_hasOverlay
			MOV result, AL
			POPAD
		}
		return result;
	}

	void function_004934B0(Unit* unit, Unit* battery)
	{
		const u32 Func_Sub4934B0 = 0x004934B0;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EBX, battery
			CALL Func_Sub4934B0
			POPAD
		}
	}

	// Identical to unitIsHangerUnit @ 0x00401450
	bool isHangarUnit(Unit* unit)
	{
		if (unit->id == UnitId::ProtossSteward
			|| unit->id == UnitId::ProtossPlasmaShell)
		{
			return true;		
		}
		else
		{
			return false;
		}
	}

	Unit* MedicHeal_TargetAcquire(Unit* medic)
	{
		const u32 Func_MedicHeal_TargetAcquire = 0x004422A0;
		static Unit* target;
		__asm {
			PUSHAD
			PUSH medic
			CALL Func_MedicHeal_TargetAcquire
			MOV target, EAX
			POPAD
		}
		return target;
	}

	u32 doMedicHeal(Unit* unit, Unit* target)
	{
		const u32 Func_doMedicHeal = 0x00463C40;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH target
			CALL Func_doMedicHeal
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void order_to_attack_target(Unit* unit, Unit* target, u32 unk1, u32 unk2)
	{
		const u32 func_address = 0x00476FC0;
		__asm {
			PUSHAD
			PUSH unk2
			PUSH unk1
			MOV EAX, target
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	b32 attack_recent_attacker(Unit* unit)
	{
		const u32 Func_Sub4770E0 = 0x004770E0;
		static b32 return_value;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4770E0
			MOV return_value, EAX
			POPAD
		}
		return (return_value != 0);
	}

	b32 AI_OrderHeal(Unit* unit, Unit* target)
	{
		const u32 Func_AI_OrderHeal = 0x00463530;
		static b32 result;
		__asm {
			PUSHAD
			MOV ESI, target
			MOV EBX, unit
			CALL Func_AI_OrderHeal
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void function_4748E0(Unit* unit, s32 x, s32 y, u8 orderId)
	{
		const u32 Func_sub4748E0 = 0x004748E0;
		__asm {
			PUSHAD
			MOV BL, orderId
			MOV ESI, unit
			XOR EDI, EDI
			PUSH y
			PUSH x
			CALL Func_sub4748E0
			POPAD
		}
	}

	void function_474940(Unit* unit, Unit* target, u8 orderId)
	{
		const u32 Func_sub474940 = 0x00474940;
		__asm {
			PUSHAD
			MOV BL, orderId
			MOV ESI, unit
			MOV EAX, target
			XOR EDI, EDI
			CALL Func_sub474940
			POPAD
		}
	}

	b32 Ai_AreOnConnectedRegions(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_Ai_AreOnConnectedRegions = 0x00438E70;
		static b32 result;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			MOV EAX, unit
			CALL Func_Ai_AreOnConnectedRegions
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void function_0043DF30(Unit* unit)
	{
		const u32 Func_Sub43DF30 = 0x0043DF30;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_Sub43DF30
			POPAD
		}
	}

	void insert_order_before(Unit* unit, Order* order, u8 orderId, s16 x, s16 y, Unit* target, u16 targetUnitId)
	{
		static Point16 pos;
		pos.x = x; pos.y = y;
		const u32 Func_insert_order_before = 0x00474540;
		__asm {
			PUSHAD
			PUSH target
			PUSH pos
			MOV BL, orderId
			MOVZX EDX, targetUnitId
			MOV ESI, unit
			MOV EDI, order
			CALL Func_insert_order_before
			POPAD
		}
	}

	void _doOrder(Unit* unit, u32 orderId, s32 x, s32 y, bool stopPreviousOrders)
	{
		const u32 Func__doOrder = 0x00474C70;
		b32 bStopPreviousOrders = stopPreviousOrders ? 1 : 0;
		__asm {
			PUSHAD
			PUSH bStopPreviousOrders
			PUSH y
			PUSH x
			PUSH orderId
			PUSH unit
			CALL Func__doOrder
			POPAD
		}
	}

	b32 Ai_UpdateAttackTarget(Unit* unit, u32 unk1, u32 unk2, u32 unk3)
	{
		const u32 Func_Ai_UpdateAttackTarget = 0x00477160;
		__asm {
			PUSHAD
			PUSH unk1
			PUSH unk2
			PUSH unk3
			MOV ECX, unit
			CALL Func_Ai_UpdateAttackTarget
			POPAD
		}
	}

	Unit* getLoadedUnitFromIndex(Unit* transport, s32 index)
	{
		const u32 Func_getLoadedUnitFromIndex = 0x004E6C40;
		static Unit* return_value;
		__asm {
			PUSHAD
			MOV ECX, index
			MOV EAX, transport
			CALL Func_getLoadedUnitFromIndex
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	b32 getFirstLoadedWorker(Unit* transport)
	{
		const u32 Func_getFirstLoadedWorker = 0x004E6D90;
		static b32 return_value;
		__asm {
			PUSHAD
			MOV EAX, transport
			CALL Func_getFirstLoadedWorker
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	b32 Ai_FindTransport(Unit* unit, Unit** unitTargetOffset)
	{
		const u32 Func_Ai_FindTransport = 0x004E6EF0;
		b32 result;
		__asm {
			PUSHAD
			PUSH unitTargetOffset
			MOV EBX, unit
			CALL Func_Ai_FindTransport
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void loadUnitProc(Unit* unit, Unit* unitToLoad)
	{
		const u32 Func_Sub4E78E0 = 0x004E78E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, unitToLoad
			CALL Func_Sub4E78E0
			POPAD
		}
	}

	b32 unloadUnitProc(Unit* unit)
	{
		const u32 Func_Sub4E7F70 = 0x004E7F70;
		static b32 return_value;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4E7F70
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	// angleValue is a guess
	void unitSetRetreatPoint(Unit* unit, u8 angleValue)
	{
		const u32 Func_UnitSetRetreatPoint = 0x004011A0;
		__asm {
			PUSHAD
			MOV AL, angleValue
			MOV EDX, unit
			CALL Func_UnitSetRetreatPoint
			POPAD
		}
	}

	//return true when unit reached next_target_waypoint either at call
	//or after some action inside the function, else return false
	//At least that's what I think it does (not sure what I called
	//"action inside the function" is, and how it's used to return
	//true
	bool getCloserToDestination_sub402BE0(Unit* unit)
	{
		const u32 Func_Sub402BE0_GetCloserToDestination = 0x00402BE0;
		static b32 bResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_Sub402BE0_GetCloserToDestination
			MOV bResult, EAX
			POPAD
		}
		return (bResult == 1);
	}

	void orderReturnToIdle(Unit* unit)
	{
		const u32 Func_OrderReturnToIdle = 0x00463770;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_OrderReturnToIdle
			POPAD
		}
	}

	bool checkForCloseUnit(Unit* unit)
	{
		const u32 Func_Sub464290_CheckForCloseUnit = 0x00464290;
		static b32 bResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub464290_CheckForCloseUnit
			MOV bResult, EAX
			POPAD
		}
		return (bResult == 1);
	}

	void playSubunitIscript(Unit* unit, IscriptAnimation::Enum anim)
	{
		const u32 Func_PlaySubunitIscript = 0x00474380;
		u32 anim_ = anim;
		assert(unit->subunit);
		__asm {
			PUSHAD
			MOV EBX, unit
			PUSH anim_
			CALL Func_PlaySubunitIscript
			POPAD
		}
	}

	// Identical to getHPGainForRepair @ 0x00402C40
	u32 getHPGainForRepair(Unit* unit)
	{
		if (*CHEAT_STATE & CheatFlags::OperationCwal)
		{
			return unit->buildRepairHpGain * 16;
		}
		else
		{
			return unit->buildRepairHpGain;
		}
	}

	void refund_unit_train_cost(u32 unitId, u8 playerId)
	{
		const u32 Func_refundUnitTrainCost = 0x0042CEC0;
		__asm {
			PUSHAD
			MOV ECX, unitId
			MOV AL, playerId
			CALL Func_refundUnitTrainCost
			POPAD
		}
	}

	void function_00432430(Unit* unit, u32 buildQueueSlot)
	{
		const u32 Func_Sub432430 = 0x00432430;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, buildQueueSlot
			CALL Func_Sub432430
			POPAD
		}
	}

	void addHangarUnit(Unit* main_unit, Unit* added_unit)
	{
		const u32 Func_addHangerUnit = 0x00466300;
		__asm {
			PUSHAD
			MOV EDI, main_unit
			MOV ESI, added_unit
			CALL Func_addHangerUnit
			POPAD
		}
	}

	Unit* attemptTrainHatchUnit(Unit* trainer, u32 builtUnitID, u32 flag)
	{
		const u32 Func_attemptTrainHatchUnit = 0x00468200;
		static Unit* builtUnit;
		__asm {
			PUSHAD
			MOV ESI, trainer
			MOV EDI, builtUnitID
			PUSH flag
			CALL Func_attemptTrainHatchUnit
			MOV builtUnit, EAX
			POPAD
		}
		return builtUnit;
	}

	Bullet* create_bullet(u8 weaponId, Unit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction)
	{
		const u32 Helper_CreateBullet = 0x0048C260;
		static u32 new_attackingPlayer;
		static u32 new_direction;
		static s32 new_x;
		static s32 new_y;
		new_attackingPlayer = attackingPlayer;
		new_direction = direction;
		new_x = x;
		new_y = y;

		/// Paladin attacking behavior
		if (weaponId == WeaponId::PaladinCrusaderMissiles)
		{
			if (source->sprite->main_graphic->direction == 0
				|| source->sprite->main_graphic->direction == 1
				|| source->sprite->main_graphic->direction == 2
				|| source->sprite->main_graphic->direction == 3)
			{
				new_x = source->sprite->position.x + 6;
				new_y = source->sprite->position.y - 45;
			}
			else if (source->sprite->main_graphic->direction == 4
				|| source->sprite->main_graphic->direction == 5)
			{
				new_x = source->sprite->position.x + 4;
				new_y = source->sprite->position.y - 44;
			}
			else if (source->sprite->main_graphic->direction == 6
				|| source->sprite->main_graphic->direction == 7
				|| source->sprite->main_graphic->direction == 8
				|| source->sprite->main_graphic->direction == 9)
			{
				new_x = source->sprite->position.x + 1;
				new_y = source->sprite->position.y - 43;
			}
			else if (source->sprite->main_graphic->direction == 10
				|| source->sprite->main_graphic->direction == 11)
			{
				new_x = source->sprite->position.x - 3;
				new_y = source->sprite->position.y - 42;
			}
			else if (source->sprite->main_graphic->direction == 12
				|| source->sprite->main_graphic->direction == 13)
			{
				new_x = source->sprite->position.x - 6;
				new_y = source->sprite->position.y - 41;
			}
			else if (source->sprite->main_graphic->direction == 14
				|| source->sprite->main_graphic->direction == 15)
			{
				new_x = source->sprite->position.x - 8;
				new_y = source->sprite->position.y - 44;
			}
			else if (source->sprite->main_graphic->direction == 16)
			{
				new_x = source->sprite->position.x - 8;
				new_y = source->sprite->position.y - 48;
			}
		}
		static Bullet* bullet;
		__asm {
			PUSHAD
			PUSH new_direction
			PUSH new_attackingPlayer
			PUSH new_y
			PUSH new_x
			MOV EAX, source
			MOVZX ECX, weaponId
			CALL Helper_CreateBullet
			MOV bullet, EAX
			POPAD
		}
		return bullet;
	}

	// For progressBulletMovement

	void progressBulletMovement_func(Bullet* a1)
	{
		const u32 Func_progressBulletMovement = 0x0048B250;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_progressBulletMovement
			POPAD
		}
	}

	// For bulletBehavior_Bounce
	Bullet* changeMovePos(Bullet* a1, s32 a2, s32 a3)
	{
		const u32 Func_changeMovePos = 0x00495240;
		static Bullet* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			MOV ESI, a3
			CALL Func_changeMovePos
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* findNextBounceTarget_func(Bullet* a1)
	{
		const u32 Func_findNextBounceTarget = 0x0048B1E0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_findNextBounceTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unitIsSubunit(Unit* a1)
	{
		const u32 Func_unitIsSubunit = 0x00401D40;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsSubunit
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unitIsCarrier(Unit* a1)
	{
		return a1->id == UnitId::ProtossSolarion;
		/*
		const u32 Func_unitIsCarrier = 0x00401470;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsCarrier
			MOV result, EAX
			POPAD
		}
		return (bool)result;
		*/
	}

	bool unitIsReaver(Unit* a1)
	{
		return a1->id == UnitId::ProtossAccantor;
		/*
		const u32 Func_unitIsReaver = 0x00401490;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsReaver
			MOV result, EAX
			POPAD
		}
		return (bool)result;
		*/
	}

	bool unitHasPathToUnit(Unit* a1, Unit* a2)
	{
		const u32 Func_unitHasPathToUnit = 0x0049CBB0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_unitHasPathToUnit
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 canBeInfested(Unit* a1, Unit* a2)
	{
		const u32 Func_canBeInfested = 0x00402210;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL Func_canBeInfested
			MOV result, EAX
			POPAD
		}
		return result;
	}

	//for progressBullet

	void do_missile_damage(Bullet* bullet)
	{
		const u32 func_doMissileDmg = 0x0048B770;
		__asm {
			PUSHAD
			PUSH bullet
			CALL func_doMissileDmg
			POPAD
		}
	}

	bool can_unit_id_fit_at_xy(s32 x, s32 y, u32 unitId)
	{
		const u32 Func_Sub42D810 = 0x0042D810;
		static b32 bPreReturnValue;
		__asm {
			PUSHAD
			MOV EAX, x
			MOV EDI, y
			MOV EBX, unitId
			CALL Func_Sub42D810
			MOV bPreReturnValue, EAX
			POPAD
		}
		return (bPreReturnValue != 0);
	}

	Unit** get_all_units_in_bounds(Box16* coords)
	{
		const u32 Func_GetAllUnitsInBounds = 0x0042FF80;
		static Unit** units_in_bounds;
		__asm {
			PUSHAD
			MOV EAX, coords
			CALL Func_GetAllUnitsInBounds
			MOV units_in_bounds, EAX
			POPAD
		}
		return units_in_bounds;
	}

	u32 get_unit_damage_bonus(Unit* unit, u8 weaponId)
	{
		const u32 Func_getUnitDamageBonus = 0x00475EC0;
		static u32 damageBonus;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV DL, weaponId
			CALL Func_getUnitDamageBonus
			MOV damageBonus, EAX
			POPAD
		}
		return damageBonus;
	}

	// identical to increment_unit_kill_count @ 0x004759C0
	void increment_unit_kill_count(Unit* unit)
	{
		Unit* temp_unit = unit;
		if (temp_unit->killCount < 255)
		{
			temp_unit->killCount++;
		}
		while (
			temp_unit->id == UnitId::ProtossSteward &&
			temp_unit->interceptor.parent != nullptr
			)
		{
			temp_unit = temp_unit->interceptor.parent;
			if (temp_unit->killCount < 255)
			{
				temp_unit->killCount++;
			}
		}
	}

	void attack_overlay_and_notify(Unit* attacker, Unit* target, u32 weaponType, u32 direction)
	{
		const u32 Func_attackOverlayAndNotify = 0x00479730;
		__asm {
			PUSHAD
			MOV ESI, attacker
			PUSH direction
			MOV EAX, target
			PUSH weaponType
			CALL Func_attackOverlayAndNotify
			POPAD
		}
	}

	u32 get_unit_bullet_damage(Bullet* bullet, Unit* target)
	{
		const u32 Func_GetUnitBulletDamage = 0x0048ACD0;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV EAX, target
			MOV EDX, bullet
			CALL Func_GetUnitBulletDamage
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	u32 get_spell_stat_string(Unit* target, u32 techId, u8 playerId)
	{
		const u32 Func_getSpellStatString = 0x00491E80;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV BL, playerId
			PUSH techId
			MOV EAX, target
			CALL Func_getSpellStatString
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void disableUnitFunc(Unit* unit)
	{
		const u32 Func_disableUnit = 0x00492CC0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_disableUnit
			POPAD
		}
	}

	// Identical to setAllImageGroupFlagsPal11 @ 0x00497430;
	void setAllImageGroupFlagsPal11(Sprite* sprite)
	{
		for (Image* current_image = sprite->images.head;
			current_image != nullptr;
			current_image = current_image->link.next)
		{
			if (current_image->paletteType == PaletteType::RLE_HPFLOATDRAW)
			{
				current_image->flags |= ImageFlags::Redraw;
			}
		}
	}
	
	const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
	Unit* iterateUnitsAtLocationTargetProc(Box16* a1, Unit*(__fastcall* a2)(Unit*, Unit*), Unit* a3)
	{
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
	u32 IterateUnitsAtLocationTargetProc_LarvaCount(Unit* unit, Box16* coords)
	{
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
	u32 IterateUnitsAtLocationTargetProc_RecallTargets(Unit* unit, Box16* coords)
	{
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

	void IterateUnitsAtLocationTargetProc_CorrosiveAcid(u32 attackingPlayerId, Box16* coords)
	{
		const u32 CorrosiveAcidProc_function = 0x004F4670;
		__asm {
			PUSHAD
			PUSH attackingPlayerId
			MOV EAX, coords
			MOV EBX, CorrosiveAcidProc_function
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void IterateUnitsAtLocationTargetProc_Ensnare(Unit* attacker, Box16* coords)
	{
		const u32 EnsnareProc_function = 0x004F46C0;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, EnsnareProc_function
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void IterateUnitsAtLocationTargetProc_Plague(Unit* attacker, Box16* coords)
	{
		const u32 PlagueProc_function = 0x004F4AF0;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, PlagueProc_function
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void PlagueProc(Unit* unit, Unit* attacker) {
		const u32 func_address = 0x004F4AF0;
		__asm {
			PUSHAD
			MOV EDX, attacker
			MOV ECX, unit
			CALL func_address
			POPAD
		}
	}

	void IterateUnitsAtLocationTargetProc_Maelstrom(Unit* attacker, Box16* coords)
	{
		const u32 MaelstromProc_function = 0x004F6760;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, MaelstromProc_function
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void IterateUnitsAtLocationTargetProc_StasisField(Unit* attacker, Box16* coords)
	{
		const u32 StasisFieldProc_function = 0x004F68D0;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, StasisFieldProc_function
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}	

	void createEnsnareOverlay(Unit* a1)
	{
		const u32 Func_createEnsnareOverlay = 0x004F45E0;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_createEnsnareOverlay
			POPAD
		}
	}

	// For createScanner

	s32 setUnitStatTxtErrorMsg(s32 a1)
	{
		const u32 Func_setUnitStatTxtErrorMsg = 0x0048CCB0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_setUnitStatTxtErrorMsg
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 play_sound_from_unit(s32 soundId, Unit* unit, s32 bPlay, s32 minVolume)
	{
		const u32 Func_playSoundFromSource = 0x0048ED50;
		static s32 result;
		__asm {
			PUSHAD
			PUSH minVolume
			PUSH bPlay
			MOV ESI, unit
			MOV EBX, soundId
			CALL Func_playSoundFromSource
			MOV result, EAX
			POPAD
		}
		return result;
	}

	//Identical to 0x00401170
	bool unitIsOwnedByCurrentPlayer(Unit* unit)
	{
		bool return_value;
		if (*IS_IN_REPLAY != 0 || unit->player != *LOCAL_NATION_ID)
		{
			return_value = false;
		}
		else
		{
			return_value = true;
		}
		return return_value;
	}

	//Identical to 0x00402A70
	bool isUnitBurrowed(Unit* unit)
	{
		return (unit->status & (UnitStatus::CloakingForFree + UnitStatus::Burrowed)) != 0;
	}

	void SC_memcpy_0(u32 dest, u32 src, u32 memsize)
	{
		const u32 Func_memcpy_0 = 0x00408FD0;
		__asm {
			PUSHAD
			PUSH memsize
			PUSH src
			PUSH dest
			CALL Func_memcpy_0
			ADD ESP, 0x0C
			POPAD
		}
	}

	void function_00468670(Unit* unit)
	{
		const u32 Func_Sub_468670 = 0x00468670;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub_468670
			POPAD
		}
	}

	bool unit_isUnselectable(u16 unitId)
	{
		const u32 Func_Unit_isUnselectable = 0x0046ED80;
		b32 bHalfReturnValue;
		__asm {
			PUSHAD
			XOR EBX, EBX
			MOV BX, unitId
			MOV EAX, EBX
			CALL Func_Unit_isUnselectable
			MOV bHalfReturnValue, EAX
			POPAD
		}
		return (bHalfReturnValue != 0);
	}

	void function_0046F040(Unit* current_unit, Unit** unit_list, Unit* clicked_unit, u32 list_length)
	{
		const u32 Func_Sub_6F040 = 0x0046F040;
		__asm {
			PUSHAD
			PUSH clicked_unit
			PUSH current_unit
			MOV ECX, unit_list
			MOV EAX, list_length
			CALL Func_Sub_6F040
			POPAD
		}
	}

	u32 combineLists_Sub_6F290(Unit* unit, Unit** unit_list_1, Unit** unit_list_2, u32 list_length)
	{
		const u32 Func_Sub_46F290 = 0x0046F290;
		u32 return_value;
		__asm {
			PUSHAD
			PUSH unit
			PUSH list_length
			PUSH unit_list_1
			MOV EDI, unit_list_2
			CALL Func_Sub_46F290
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void applyNewSelect_Sub_6FA00(Unit** unit_list, u32 unit_list_length)
	{
		const u32 Func_Sub_46FA00 = 0x0046FA00;
		__asm {
			PUSHAD
			PUSH 0x01
			PUSH 0x01
			MOV EBX, unit_list
			MOV EDI, unit_list_length
			CALL Func_Sub_46FA00
			POPAD
		}
	}

	u32 CUnitToUnitID(Unit* unit)
	{
		const u32 Func_CUnitToUnitID = 0x0047B1D0;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_CUnitToUnitID
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	bool unit_IsStandardAndMovable(Unit* unit)
	{
		const u32 Func_Unit_IsStandardAndMovable = 0x0047B770;
		b32 return_value_unconverted;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_Unit_IsStandardAndMovable
			MOV return_value_unconverted, EAX
			POPAD
		}
		return (return_value_unconverted != 0);
	}

	void selectBuildingSFX(Unit* unit)
	{
		const u32 Func_SelectBuildingSFX = 0x0048F910;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_SelectBuildingSFX
			POPAD
		}
	}

	void CreateNewUnitSelectionsFromList(Unit** unit_list, u32 unit_list_length)
	{
		const u32 Func_CreateNewUnitSelectionsFromList = 0x0049AE40;
		__asm {
			PUSHAD
			PUSH unit_list_length
			MOV EAX, unit_list
			CALL Func_CreateNewUnitSelectionsFromList
			POPAD
		}
	}

	void CMDACT_Select(Unit** unit_list, u32 unit_list_length)
	{
		const u32 Func_CMDACT_Select = 0x004C0860;
		__asm {
			PUSHAD
			PUSH unit_list
			PUSH unit_list_length
			CALL Func_CMDACT_Select
			POPAD
		}
	}

	// Equivalent to secondaryOrd_Cloak @ 0x00491790
	void secondaryOrder_Cloak(Unit* unit)
	{
		static Unit** const firstBurrowedUnit = (Unit**)0x0063FF5C;
		unit->isCloaked++;
		if (
			unit->isCloaked == 1 &&
			!(unit->status & UnitStatus::RequiresDetection) &&
			unit->burrow_link.next == nullptr
			)
		{
			unit->burrow_link.next = *firstBurrowedUnit;
			unit->burrow_link.prev = nullptr;
			if (*firstBurrowedUnit != nullptr)
			{
				(*firstBurrowedUnit)->burrow_link.prev = unit;
			}
			*firstBurrowedUnit = unit;
			scbw::refresh_console();
		}
	}

	void removeOverlays2(u32 from, u32 to, Unit* unit)
	{
		const u32 Func_removeOverlays2 = 0x00492370;
		__asm {
			PUSHAD
			MOV EAX, from
			MOV EBX, unit
			PUSH to
			CALL Func_removeOverlays2
			POPAD
		}
	}

	bool unitHpIsInRedZone(Unit* unit)
	{
		const u32 Helper_UnitHpIsInRedZone = 0x004022C0;
		static b32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Helper_UnitHpIsInRedZone
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	//hooked by hooks\update_status_effects.cpp
	void RestoreAllUnitStats(Unit* unit)
	{
		const u32 Func_RestoreAllUnitStats = 0x00492F70;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RestoreAllUnitStats
			POPAD
		}
	}

	// For update_units

	bool is_selected_unit_group_enabled()
	{
		const u32 Func_isSelectedUnitGroupEnabled = 0x0049A2C0;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_isSelectedUnitGroupEnabled
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void cancel_target_order()
	{
		const u32 Func_cancelTargetOrder = 0x0048CA10;
		__asm {
			PUSHAD
			CALL Func_cancelTargetOrder
			POPAD
		}
	}

	s32 update_disruption_web_status()
	{
		const u32 Func_UpdateDisruptionWebStatus = 0x004EB2F0;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_UpdateDisruptionWebStatus
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* progress_dying_unit_frame(Unit* unit)
	{
		const u32 Func_progressDyingUnitFrame = 0x004EB5E0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL Func_progressDyingUnitFrame
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void reveal_sight_at_location(s32 sightRange, u32 visionFlags, s32 ptX, s32 ptY, bool isAirUnit)
	{
		const u32 Func_revealSightAtLocation = 0x004806F0;
		__asm {
			PUSHAD
			PUSH isAirUnit
			PUSH ptY
			PUSH ptX
			MOV ECX, visionFlags
			MOV EAX, sightRange
			CALL Func_revealSightAtLocation
			POPAD
		}
	}

	void update_visibility(Unit* unit)
	{
		const u32 Func_updateVisibility = 0x004EBE10;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_updateVisibility
			POPAD
		}
	}

	void progress_active_unit_frame(Unit* unit)
	{
		const u32 Func_progressActiveUnitFrame = 0x004EBC30;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_progressActiveUnitFrame
			POPAD
		}
	}

	void check_unit_visibility(Unit* unit)
	{
		const u32 Func_checkUnitVisibility = 0x00443390;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_checkUnitVisibility
			POPAD
		}
	}

	void progress_unit_frame(Unit* unit)
	{
		const u32 Func_progressUnitFrame = 0x004ECF70;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_progressUnitFrame
			POPAD
		}
	}

	void unit_update(Unit* unit)
	{
		const u32 Func_unitUpdate = 0x004EC420;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_unitUpdate
			POPAD
		}
	}

	void update_unit_power()
	{
		const u32 Func_updateUnitPower = 0x00493790;
		__asm {
			PUSHAD
			CALL Func_updateUnitPower
			POPAD
		}
	}

	// For update_burrowing_cloaking_units

	s32 is_unit_cloaking(Unit* unit)
	{
		const u32 Func_isUnitCloaking = 0x004916E0;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_isUnitCloaking
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void play_sound_from_direction(Unit* unit, s32 soundId)
	{
		const u32 Func_playSoundFromDirect = 0x0049B5B0;
		__asm {
			PUSHAD
			MOV ECX, soundId
			MOV EAX, unit
			CALL Func_playSoundFromDirect
			POPAD
		}
	}

	// For do_cloak

	void update_sprite_flag_on_palette(Sprite* sprite)
	{
		const u32 Func_updateSpriteFlagOnPalette = 0x00497E80;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_updateSpriteFlagOnPalette
			POPAD
		}
	}

	void show_images_hidden_by_cloak(Sprite* sprite)
	{
		const u32 Func_showImagesHiddenByCloak = 0x00498170;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_showImagesHiddenByCloak
			POPAD
		}
	}

	void update_sprite_images_for_cloak(Sprite* sprite)
	{
		const u32 Func_updateSpriteImagesForCloak = 0x00498A90;
		__asm {
			PUSHAD
			MOV EDI, sprite
			CALL Func_updateSpriteImagesForCloak
			POPAD
		}
	}

	u32 getLarvaeUnitsFromList(Unit** unitList, Unit** outputList)
	{
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

	void selectMultipleUnitsFromUnitList(Unit** unitList, u32 unitsCount, b32 unk1, b32 unk2)
	{
		const u32 Func_selectMultipleUnitsFromUnitList = 0x0049AEF0;
		__asm {
			PUSHAD
			PUSH unk2
			PUSH unk1
			PUSH unitList
			PUSH unitsCount
			CALL Func_selectMultipleUnitsFromUnitList
			POPAD
		}
	}

	s32 BWFXN_PrintXY(const char* text, s32 x, s32 y)
	{
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

	s32 GetColorHandle(u8 color)
	{
		const u32 GetColorHandle_function = 0x0041F610;
		static s32 result;
		_asm {
			PUSHAD
			MOV AL, color
			CALL GetColorHandle_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For targetedOrder

	Order* queue_new_order(Order* a1, s16 a2, s16 a3, Unit* a4, u8 a5, s32 a6, s16 a7, Vector32* a8, Unit* a9)
	{
		const u32 Func_queue_new_order = 0x00474B90;
		static Order* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV DX, a2
			MOV CX, a3
			PUSH a9
			PUSH a8
			PUSH a7
			PUSH a6
			PUSH a5
			PUSH a4
			CALL Func_queue_new_order
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 show_stat_txt_to_player(s32 a1, s32 a2)
	{
		const u32 Func_show_stat_txt_to_player = 0x0048CF00;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_show_stat_txt_to_player
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void set_order_target(u8 a1, s16 a2, s16 a3, s32 a4, s32 a5, Unit* a6, s32 a7, Vector32* a8, Unit* a9)
	{
		const u32 Func_set_order_target = 0x00475470;
		__asm {
			PUSHAD
			MOV CL, a1
			MOV DX, a2
			MOV AX, a3
			MOV EDI, a4
			PUSH a9
			PUSH a8
			PUSH a7
			PUSH a6
			PUSH a5
			CALL Func_set_order_target
			POPAD
		}
	}

	// For selectUnits

	s32 select_single_unit_from_id(s32 index)
	{
		const u32 Func_select_single_unit_from_id = 0x00496D30;
		static s32 result;
		__asm {
			PUSHAD
			PUSH index
			CALL Func_select_single_unit_from_id
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 selectMultipleUnitsFromUnitList(s32 a1, Unit** a2, u8 a3, s32 a4)
	{
		const u32 Func_selectMultipleUnitsFromUnitList = 0x0049AEF0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_selectMultipleUnitsFromUnitList
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 SNetGetTurnsInTransit(u32* turns)
	{
		const u32 SNetGetTurnsInTransit_function = 0x00410275;
		static s32 result;
		_asm {
			PUSHAD
			PUSH turns
			CALL SNetGetTurnsInTransit_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8 sendTurn()
	{
		const u32 sendTurn_function = 0x00485A40;
		static u32 result;
		_asm {
			PUSHAD
			CALL sendTurn_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 SErrGetLastError()
	{
		const u32 SErrGetLastError_function = 0x004100FA;
		static s32 result;
		_asm {
			PUSHAD
			CALL SErrGetLastError_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u8 packetErrHandle(s32 result_arg, s32 a1, const char* a3, s32 a4, s32 a5)
	{
		const u32 packetErrHandle_function = 0x004BB0B0;
		static u32 result;
		_asm {
			PUSHAD
			MOV EAX, result_arg
			PUSH a5
			PUSH a4
			PUSH a3
			PUSH a1
			CALL packetErrHandle_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// unkRepairInfo may be related to unit->worker.repairResourceLossTimer
	void getRepairInfo(Unit* repairTarget, u32* repairCostMineral, u32* repairCostGas, s16* unkRepairInfo)
	{
		const u32 Func_getRepairInfo = 0x00466D10;
		__asm {
			PUSHAD
			MOV EAX, repairTarget
			PUSH unkRepairInfo
			PUSH repairCostGas
			PUSH repairCostMineral
			CALL Func_getRepairInfo
			POPAD
		}
	}

	bool ord_repair_subtract(Unit* worker, Unit* target)
	{
		const u32 Func_ord_repair_subtract = 0x00466FB0;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV ESI, worker
			PUSH target
			CALL Func_ord_repair_subtract
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	// display an error message and return false if not enough resources
	bool verifyResources(u32 mineralCost, u32 gasCost, u32 playerId)
	{
		const u32 Func_verifyResources = 0x00466EE0;
		b32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, mineralCost
			PUSH gasCost
			MOV ECX, playerId
			CALL Func_verifyResources
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	u8 getHangerTrainCount(Unit* unit)
	{
		const u32 Func_getHangerTrainCount = 0x00465270;
		static u8 result = 0;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_getHangerTrainCount
			MOV result, AL
			POPAD
		}
	}

	bool unit_is_training_or_morphing(Unit* unit)
	{
		const u32 Func_unitIsTrainingOrMorphing = 0x00401500;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_unitIsTrainingOrMorphing
			MOV result, EAX
			POPAD
		}
	}

	u8 initializeRandomizerInfo()
	{
		const u32 Func_initializeRandomizerInfo = 0x0047E410;
		static u32 result;
		__asm {
			PUSHAD
			CALL Func_initializeRandomizerInfo
			MOV result, EAX
			POPAD
		}
		return (u8)result;
	}

	void AI_Loop()
	{
		const u32 Func_AI_Loop = 0x004A2B00;
		__asm {
			PUSHAD
			CALL Func_AI_Loop
			POPAD
		}
	}

	s32 updateActiveTileInfo()
	{
		const u32 Func_updateActiveTileInfo = 0x0049C4C0;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_updateActiveTileInfo
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 removeFOWCheat()
	{
		const u32 Func_removeFOWCheat = 0x00480880;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_removeFOWCheat
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void update_units()
	{
		const u32 Func_updateUnits = 0x004ED000;
		__asm {
			PUSHAD
			CALL Func_updateUnits
			POPAD
		}
	}

	s32 imageDrawingBulletDrawing()
	{
		const u32 Func_imageDrawingBulletDrawing = 0x0048C3D0;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_imageDrawingBulletDrawing
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void updateImages()
	{
		const u32 Func_updateImages = 0x00496330;
		__asm {
			PUSHAD
			CALL Func_updateImages
			POPAD
		}
	}

	void updateThingys()
	{
		const u32 Func_updateThingys = 0x00488510;
		__asm {
			PUSHAD
			CALL Func_updateThingys
			POPAD
		}
	}

	// AKA createPsiFieldSpriteForUnit()
	// Returns true if successful, false otherwise.
	bool addPsiFieldSprite(Unit* unit)
	{
		if (!unit)
		{
			return false;
		}
		const u32 Func_addPsiFieldSprite = 0x00493BF0;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_addPsiFieldSprite
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	void orderInterrupt(Unit* unit, Unit* target, u16 x, u16 y, u32 orderId)
	{
		const u32 Func_orderInterrupt = 0x00474C30;
		static Point16 pos;
		pos.x = x; pos.y = y;
		__asm {
			PUSHAD
			MOV ESI, unit
			PUSH target
			PUSH pos
			PUSH orderId
			CALL Func_orderInterrupt
			POPAD
		}
	}

	void unitOrderMoveToTargetUnit(Unit* unit, Unit* target)
	{
		const u32 Func_unitOrderMoveToTargetUnit = 0x00479FE0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, target
			CALL Func_unitOrderMoveToTargetUnit
			POPAD
		}
	}

	void queue_game_command(u8* params, u32 param_length)
	{
		const u32 Func_QueueGameCommand = 0x00485BD0;
		__asm {
			PUSHAD
			MOV EDX, param_length
			MOV ECX, params
			CALL Func_QueueGameCommand
			POPAD
		}
	}

	Thingy* createThingyFunc(u32 spriteId, s16 x, s16 y, u32 playerId)
	{
		const u32 Func_CreateThingy = 0x00488210;
		static Thingy* thingy;
		s32 x_ = x;
		__asm {
			PUSHAD
			PUSH playerId
			MOVSX EDI, y
			PUSH x_
			PUSH spriteId
			CALL Func_CreateThingy
			MOV thingy, EAX
			POPAD
		}
		return thingy;
	}

	bool func_dounitscollide(Unit* first, Unit* second)
	{
		const u32 Func_DoUnitsCollide = 0x00469B60;
		static u32 result = 0;
		__asm {
			PUSHAD
			MOV EAX, first
			MOV ECX, second
			CALL Func_DoUnitsCollide
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void unburrow(Unit* unit)
	{
		const u32 Func_unburrow = 0x004E97C0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unburrow
			POPAD
		}
	}

	void* SMemAlloc(u32 amount, u32 unkFilename, u32 unkLine, u32 defaultValue)
	{
		const u32 Func_SMemAlloc = 0x0041006A;
		static void* pointer_on_allocated_memory;
		__asm {
			PUSHAD
			PUSH defaultValue		// probably what to fill the memory with
			PUSH unkLine			// unknown use, seems to use hardcoded value
			PUSH unkFilename		// unknown use, seems to use hardcoded adress (of some text?)
			PUSH amount				// amount of memory to allocate
			CALL Func_SMemAlloc
			MOV pointer_on_allocated_memory, EAX
			POPAD
		}
		return pointer_on_allocated_memory;
	}

	// Likely to call SMemAlloc!
	void SetCallbackTimer(BinDlg* dialog, u32 unk1, u32 unk2, u32 unk3)
	{
		const u32 Func_SetCallbackTimer = 0x00416090;
		__asm
		{
			PUSHAD
			PUSH unk3
			PUSH unk2
			MOV EAX, unk1
			MOV ECX, dialog
			CALL Func_SetCallbackTimer
			POPAD
		}
	}

	void removeFromSelections(Unit* unit)
	{
		const u32 Func_removeFromSelections = 0x0049A7F0;
		__asm
		{
			PUSHAD
			MOV EDI, unit
			CALL Func_removeFromSelections
			POPAD
		}
	}

	u32 fxnInteract(BinDlg* dialog, u32 data_struct_offset)
	{
		static u32 return_value;
		static u32 fxnInteract;
		fxnInteract = (u32)dialog->fxnInteract;
		__asm
		{
			PUSHAD
			MOV ESI, dialog // possibly useless, but just in case
			MOV ECX, dialog
			MOV EDX, data_struct_offset
			CALL fxnInteract
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void register_user_dialog_action(BinDlg* dialog, u32 array_of_fxnInteractFunc, u32 sizeOfArray)
	{
		const u32 Func_registerUserDialogAction = 0x00418100;
		__asm
		{
			PUSHAD
			MOV EAX, dialog
			MOV EDI, array_of_fxnInteractFunc
			MOV ECX, sizeOfArray
			CALL Func_registerUserDialogAction
			POPAD
		}
	}

	void enable_control(BinDlg* dialog)
	{
		const u32 Func_enableControl = 0x00418E00;
		__asm
		{
			PUSHAD
			MOV ESI, dialog
			CALL Func_enableControl
			POPAD
		}
	}

	void replay_stat_buttons(BinDlg* dialog)
	{
		const u32 Func_ReplayStatBtns = 0x00427A80;
		__asm
		{
			PUSHAD
			MOV EBX, dialog
			CALL Func_ReplayStatBtns
			POPAD
		}
	}

	void update_command_card()
	{
		const u32 Func_updateCommandCard = 0x004591D0;
		__asm
		{
			PUSHAD
			CALL Func_updateCommandCard
			POPAD
		}
	}

	void refresh_button_tooltip()
	{
		const u32 Func_refresh_button_tooltip = 0x00459770;
		__asm
		{
			PUSHAD
			CALL Func_refresh_button_tooltip
			POPAD
		}
	}

	void BINDLG_BlitSurface(BinDlg* dialog)
	{
		const u32 Func_BINDLG_BlitSurface = 0x004C35F0;
		__asm
		{
			PUSHAD
			MOV ESI, dialog
			CALL Func_BINDLG_BlitSurface
			POPAD
		}
	}

	// dev info: examples of calls: 429520,428e60,428420,4287D0,4292C0
	s32 req_check(u32 reqFunc, u16 reqVar, u32 playerId, Unit* unit)
	{
		static s32 return_value;
		__asm
		{
			PUSHAD
			MOV CX, reqVar
			MOV EDX, playerId
			PUSH unit
			CALL reqFunc
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void updateMineralPatchImage(Unit* mineralPatch)
	{
		IscriptAnimation::Enum anim;
		if (mineralPatch->building.resource.resourceAmount >= 750)
		{
			anim = IscriptAnimation::WorkingToIdle;
		}
		else if (mineralPatch->building.resource.resourceAmount >= 500)
		{
			anim = IscriptAnimation::AlmostBuilt;
		}
		else if (mineralPatch->building.resource.resourceAmount >= 250)
		{
			anim = IscriptAnimation::SpecialState2;
		}
		else if (mineralPatch->building.resource.resourceAmount >= 50)
		{
			anim = IscriptAnimation::SpecialState1;
		}
		else
		{
			anim = IscriptAnimation::Landing;
		}

		if (anim != mineralPatch->building.resource.resourceIscript)
		{
			mineralPatch->building.resource.resourceIscript = anim;
			mineralPatch->sprite->play_iscript_animation(anim);
		}
	}

	void update_image_position_offset(Image* image)
	{

		const u32 Func_updateImagePositionOffset = 0x004D5A00;
		__asm
		{
			PUSHAD
			MOV ECX, image
			CALL Func_updateImagePositionOffset
			POPAD
		}

	}

	// Identical to function @ 0x004F3AF0
	void set_resource_amount_carried(Unit* worker, u8 amountCarried, u32 chunkImageId, bool isMineral)
	{
		if (worker->resourceType != 0)
		{
			return;
		}
		worker->resourceType = isMineral ? 2 : 1;
		Image* chunkImage = worker->sprite->create_overlay(chunkImageId);
		if (chunkImage && !(chunkImage->flags & ImageFlags::Flag07))
		{
			chunkImage->flags |= ImageFlags::Flag07;
			update_image_position_offset(chunkImage);
		}
		worker->worker.resourceCarryAmount = amountCarried;
		scbw::refresh_console();
	}

	s32 ai_has_buildings_needing_power(u32 player)
	{
		const u32 Func_Ai_HasBuildingsNeedingPower = 0x00433410;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV EAX, player
			CALL Func_Ai_HasBuildingsNeedingPower
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_count_economy_units(u32 player, u32 unit_id)
	{
		const u32 Func_Ai_CountEconomyUnits = 0x00433470;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV EAX, player
			PUSH unit_id
			CALL Func_Ai_CountEconomyUnits
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_supplies_remaining(u8 player, u16 unit_id)
	{
		const u32 Func_getSuppliesRemaining = 0x004889D0;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV BL, player
			MOV DI, unit_id
			CALL Func_getSuppliesRemaining
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 get_supplies_used(s32 byte_data, u8 player)
	{
		const u32 Func_getSuppliesUsed = 0x004888C0;
		static u32 result;
		__asm
		{
			PUSHAD
			MOV EAX, byte_data
			MOV CL, player
			CALL Func_getSuppliesUsed
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_add_spending_request
	bool is_ai_allowed_to_build(s32 a1, s32 a2)
	{
		const u32 Func_isAIAllowedToBuild = 0x00447830;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV ECX, a2
			MOV EAX, a1
			CALL Func_isAIAllowedToBuild
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void delete_building_ai(BuildingAi* building, BuildingAiArray** buildingList)
	{
		const u32 Func_DeleteBuildingAI = 0x00404500;
		__asm
		{
			PUSHAD
			MOV EDX, buildingList
			MOV EAX, building
			CALL Func_DeleteBuildingAI
			POPAD
		}
	}

	void delete_worker_ai(WorkerAi* ai, WorkerAiArray** workerArray)
	{
		const u32 Func_DeleteWorkerAi = 0x00404470;
		__asm
		{
			PUSHAD
			MOV EDX, workerArray
			MOV EAX, ai
			CALL Func_DeleteWorkerAi
			POPAD
		}
	}

	void delete_ai_town_if_needed(AiTown* town)
	{
		const u32 Func_DeleteAITownIfNeeded = 0x004347E0;
		__asm
		{
			PUSHAD
			MOV EAX, town
			CALL Func_DeleteAITownIfNeeded
			POPAD
		}
	}

	// For isOrderValid
	bool unit_is_resource(Unit* unit)
	{
		const u32 Func_unitIsResource = 0x004688E0;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_unitIsResource
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void return_all_interceptors(Unit* unit)
	{
		const u32 Func_returnAllInterceptors = 0x004655F0;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_returnAllInterceptors
			POPAD
		}
	}

	char ignore_all_scripts_and_go_to_idle(Unit* unit)
	{
		const u32 Func_ignoreAllScriptsAndGoToIdle = 0x00476550;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV ESI, unit
			CALL Func_ignoreAllScriptsAndGoToIdle
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void orders_self_destructing(Unit* unit)
	{
		const u32 Func_orders_SelfDestructing = 0x00475710;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_orders_SelfDestructing
			POPAD
		}
	}

	char set_maelstrom_timer(Unit* unit, u8 time)
	{
		const u32 Func_setMaelstromTimer = 0x004553F0;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH time
			MOV EDI, unit
			CALL Func_setMaelstromTimer
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	// For CBulletDamage
	s32 _fastcall lurkerSplashProc(Unit* target, Bullet* bullet)
	{
		const u32 Func_lurkerSplashProc = 0x0048B680;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV EDX, bullet
			MOV ECX, target
			CALL Func_lurkerSplashProc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 _fastcall groundSplashProc(Unit* target, Bullet* bullet)
	{
		const u32 Func_groundSplashProc = 0x0048B750;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV EDX, bullet
			MOV ECX, target
			CALL Func_groundSplashProc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 _fastcall airSplashProc(Unit* unit, Bullet* bullet)
	{
		const u32 Func_airSplashProc = 0x0048B150;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV EDX, bullet
			MOV ECX, unit
			CALL Func_airSplashProc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 _fastcall airSplashProc2(Unit* unit, Bullet* bullet)
	{
		const u32 Func_airSplashProc2 = 0x0048B730;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV EDX, bullet
			MOV ECX, unit
			CALL Func_airSplashProc2
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unitHasNoMainOrderType(Unit* unit)
	{
		const u32 Func_unitHasNoMainOrderType = 0x00475A50;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_unitHasNoMainOrderType
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 setEventUnitComplete()
	{
		const u32 Func_setEventUnitComplete = 0x004C36C0;
		static s32 result;
		__asm
		{
			PUSHAD
			CALL Func_setEventUnitComplete
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For use_tech
	const void issueTriTargetOrder(u8 order1, u8 order2, u8 order3)
	{
		const u32 Func_TriTargetOrder = 0x0048CAF0;
		if (order2 == 0)
		{
			_asm
			{
				PUSHAD
				MOV DL, order1
				MOV CL, DL
				MOV AL, DL
				CALL Func_TriTargetOrder
				POPAD
			}
		}
		else
		{
			if (order3 == 0)
			{
				_asm
				{
					PUSHAD
					MOV CL, order1
					MOV DL, order2
					MOV AL, CL
					CALL Func_TriTargetOrder
					POPAD
				}
			}
			else
			{
				_asm
				{
					PUSHAD
					MOV DL, order1
					MOV CL, order2
					MOV AL, order3
					CALL Func_TriTargetOrder
					POPAD
				}
			}
		}
	}

	s32 checkEnergyForTechUse(s32 tech_id)
	{
		const s32 techUseFunc = 0x00423660;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, tech_id
			CALL techUseFunc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_order_unit_cloaking
	void applyCloakingOrder(Unit* unit)
	{
		const u32 Func_applyCloakingOrder = 0x00491B30;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_applyCloakingOrder
			POPAD
		}
	}

	// For findBestUnit

	Unit** findAllUnits(Box16* coords)
	{
		const u32 Func_findAllUnits = 0x004308A0;
		static Unit** result;
		__asm {
			PUSHAD
			PUSH coords
			CALL Func_findAllUnits
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For draw_building_info
	void draw_supply_unit_info(BinDlg* dialog) {
		const u32 Func_drawSupplyUnitInfo = 0x00427540;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL Func_drawSupplyUnitInfo
			POPAD
		}
	}

	// For get_unit_name
	const char* get_unit_name_stat_string(u16 unitId) {
		const u32 Func_getUnitNameStatString = 0x0047B090;
		static const char* result;
		__asm {
			PUSHAD
			MOVZX CX, unitId
			CALL Func_getUnitNameStatString
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For update_building_placement_state_units
	bool does_building_block(Unit* unit, s32 xTile, s32 yTile) {
		const u32 Func_doesBuildingBlock = 0x00473410;
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, yTile
			MOV EDX, xTile
			MOV EAX, unit
			CALL Func_doesBuildingBlock
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 is_tile_blocked_by(Unit** array, Unit* builder, s32 xTile, s32 yTile, char bDontIgnoreReacting, char bAlsoInvisible) {
		const u32 Func_isTileBlockedBy = 0x00473300;
		static s32 result;
		__asm {
			PUSHAD
			PUSH bAlsoInvisible
			PUSH bDontIgnoreReacting
			PUSH yTile
			PUSH xTile
			PUSH builder
			MOV EAX, array
			CALL Func_isTileBlockedBy
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For irradiateDamageLoop
	Unit* iterateLoadedUnits(Unit* unit, Unit* (__fastcall* func)(Unit*, Unit*), s32 a3)
	{
		const u32 Func_iterateLoadedUnits = 0x004E6D00;
		static Unit* result;
		__asm
		{
			PUSHAD
			PUSH a3
			PUSH func
			MOV EAX, unit
			CALL Func_iterateLoadedUnits
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For move_to_target
	s32 isPointAtUnitBorder(s32 unused, Unit* other, Unit* self, s32 position_xy)
	{
		const u32 Func_isPointAtUnitBorder = 0x0042DA90;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH position_xy
			PUSH self
			MOV EDX, other
			MOV ECX, unused
			CALL Func_isPointAtUnitBorder
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// Returns 1 if unfindable
	s32 AI_PrepareMovingTo(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_AI_PrepareMovingTo = 0x004E7420;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH y
			PUSH x
			PUSH unit
			CALL Func_AI_PrepareMovingTo
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* createUnitAtPos(u16 unitType, u16 playerId, u32 x, u32 y)
	{
		const u32 Func_CreateUnitAtPos = 0x004CD360; // AKA createUnitXY()
		if (unitType >= UNIT_TYPE_COUNT)
		{
			return nullptr;
		}
		static Unit* unit;
		__asm {
			PUSHAD
			MOV CX, unitType
			MOV AX, playerId
			PUSH y
			PUSH x
			CALL Func_CreateUnitAtPos
			MOV unit, EAX
			POPAD
		}
		return unit;
	}

	// more from api.cpp
	Flingy* createFlingy(u32 flingy_id, u32 playerId, u32 x, u32 y)
	{
		const u32 Func_CreateFlingy = 0x00496460;
		static Flingy* flingy;
		__asm
		{
			PUSHAD
			MOV EAX, playerId
			MOV ECX, x
			MOV EDX, y
			PUSH flingy_id
			CALL Func_CreateFlingy
			MOV flingy, EAX
			POPAD
		}
		return flingy;
	}

	// For spell_order
	bool getOrderTargetErrorString(u16 a1, Unit* a2, s16 a3, Unit* a4, s16 a5, s16 a6, u16* a7)
	{
		const u32 Func_getOrderTargetErrorString = 0x00492020;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH a7
			PUSH a6
			PUSH a5
			PUSH a4
			MOVZX SI, a3
			MOV EDI, a2
			MOVZX DX, a1
			CALL Func_getOrderTargetErrorString
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 spellOrder_MoveTowardsTarget(Unit* caster, u32 range)
	{
		const u32 Func_spellOrder_MoveTowardsTarget = 0x00492450;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV EAX, range
			MOV ECX, caster
			CALL Func_spellOrder_MoveTowardsTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s16 canTargetSpellOnFOWUnit(s32 techId, s16 unitId)
	{
		const u32 Func_canTargetSpellOnFOWUnit = 0x004914B0;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH unitId
			MOV EAX, techId
			CALL Func_canTargetSpellOnFOWUnit
			MOV result, EAX
			POPAD
		}
		return (s16)result;
	}

	bool hasGroundWeaponGraphic(Unit* unit)
	{
		const u32 Func_hasGroundWeaponGraphic = 0x00475A10;
		static bool result;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_hasGroundWeaponGraphic
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool is_unit_target_out_of_max_range(Unit* unit, Unit* target)
	{
		const u32 Func_isUnitTargetOutOfMaxRange = 0x00476430;
		static bool result;
		__asm
		{
			PUSHAD
			PUSH target
			MOV EAX, unit
			CALL Func_isUnitTargetOutOfMaxRange
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool isUnreachable(Unit* target, Unit* unit)
	{
		const u32 Func_isUnreachable = 0x00476C50;
		static bool result;
		__asm
		{
			PUSHAD
			MOV EAX, target
			MOV ESI, unit
			CALL Func_isUnreachable
			MOV result, AL
			POPAD
		}
		return result;
	}

	s32 moveForMeleeRange(Unit* target, Unit* unit)
	{
		const u32 Func_moveForMeleeRange = 0x00477F10;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH unit
			MOV EAX, target
			CALL Func_moveForMeleeRange
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For create_unit
	Unit* unitConstructor(s32 x, s32 y, u16 unitType, s32 playerId, s32 unused)
	{
		const u32 Func_unitConstructor = 0x004A06C0;
		static Unit* result;
		__asm
		{
			PUSHAD
			PUSH unused
			PUSH playerId
			MOV DI, unitType
			MOV EDX, y
			MOV ESI, x
			CALL Func_unitConstructor
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CListPushBackUsedUnitEntry(Unit* unit)
	{
		const u32 Func_CListPushBackUsedUnitEntry = 0x0049DCA0;
		static Unit* result;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_CListPushBackUsedUnitEntry
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* CListPushBackHiddenUnitEntry(Unit* unit)
	{
		const u32 Func_CListPushBackHiddenUnitEntry = 0x0049DCE0;
		static Unit* result;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_CListPushBackHiddenUnitEntry
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Sprite* unitDestructor(Unit* unit)
	{
		const u32 Func_unitDestructor = 0x004A0990;
		static Sprite* result;
		__asm
		{
			PUSHAD
			MOV ECX, unit
			CALL Func_unitDestructor
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void* getImageAttackFrame(void* resultArg, Sprite* sprite, s32 index, u32 overlay_type)
	{
		const u32 Func_getImageAttackFrame = 0x00401DF0;
		static void* result;
		__asm
		{
			PUSHAD
			PUSH overlay_type
			MOV EDI, index
			MOV ECX, sprite
			MOV EAX, resultArg
			CALL Func_getImageAttackFrame
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void setSpriteMainImageOffset(Sprite* sprite, char x, char y)
	{
		const u32 Func_setSpriteMainImageOffset = 0x00401E40;
		__asm
		{
			PUSHAD
			PUSH y
			PUSH x
			MOV EAX, sprite
			CALL Func_setSpriteMainImageOffset
			POPAD
		}
	}

	// For ai_training_worker
	char add_worker_structure_ai(Unit* worker, AiTown* town)
	{
		const u32 Func_AI_AddWorkerStructureAI = 0x00433DD0;
		static char result;
		__asm
		{
			PUSHAD
			MOV EDI, town
			MOV EBX, worker
			CALL Func_AI_AddWorkerStructureAI
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For create_bullet
	s32 initializeBullet(char playerId, Unit* parent, s16 y, Bullet* bulletResult, s32 weaponId, s32 x, s32 angle)
	{
		const u32 Func_initializeBullet = 0x0048BEC0;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH angle
			PUSH x
			PUSH weaponId
			PUSH bulletResult
			MOVZX DX, y
			MOV EAX, parent
			MOVZX CL, playerId
			CALL Func_initializeBullet
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For progress_unit_frame
	char refresh_sprite(Sprite* sprite, s32 spriteId)
	{
		const u32 Func_refreshSprite = 0x004EBBD0;
		static char result;
		__asm
		{
			PUSHAD
			PUSH spriteId
			MOV ESI, sprite
			CALL Func_refreshSprite
			MOV result, AL
			POPAD
		}
		return result;
	}

	void updateUnitTimers(Unit* unit)
	{
		const u32 Func_updateUnitTimers = 0x004EC290;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_updateUnitTimers
			POPAD
		}
	}

	void progress_order(Unit* unit)
	{
		const u32 Func_progressOrder = 0x004EC4D0;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_progressOrder
			POPAD
		}
	}

	void perform_secondary_orders(Unit* result, s32 unused)
	{
		const u32 Func_performSecondaryOrders = 0x004EC170;
		__asm
		{
			PUSHAD
			MOV ECX, unused
			MOV EAX, result
			CALL Func_performSecondaryOrders
			POPAD
		}
	}

	void sprite_to_iscript_loop(Sprite* sprite)
	{
		const u32 Func_spriteToIscriptLoop = 0x00497920;
		__asm
		{
			PUSHAD
			MOV ESI, sprite
			CALL Func_spriteToIscriptLoop
			POPAD
		}
	}

	s16 CreateTooltip(char* text)
	{
		const u32 Func_CreateTooltip = 0x004591A6;
		static s16 result;
		__asm
		{
			PUSHAD
			PUSH text
			CALL Func_CreateTooltip
			MOV result, AX
			POPAD
		}
		return result;
	}

	char refreshSelectionScreen()
	{
		const u32 Func_refreshSelectionScreen = 0x00481480;
		static char result;
		__asm
		{
			PUSHAD
			CALL Func_refreshSelectionScreen
			MOV result, AL
			POPAD
		}
	}

	// For ai_send_suicide
	bool is_threat(Unit* target, Unit* unit)
	{
		const u32 Func_isThreat = 0x00476930;
		static bool result;
		__asm
		{
			PUSHAD
			MOV EAX, target
			MOV ECX, unit
			CALL Func_isThreat
			MOV result, AL
			POPAD
		}
		return result;
	}

	void prepare_suicide(Unit* unit)
	{
		const u32 Func_prepSuicide = 0x0043CD40;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_prepSuicide
			POPAD
		}
	}

	u32 ai_attack_clear(s32 player)
	{
		const u32 Func_AI_AttackClear = 0x00439500;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH player
			CALL Func_AI_AttackClear
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool ai_attack_manager(s32 player, s32 x, s32 y, s32 bOverrideLastAttack, s32 bFallbackAirOnly)
	{
		const u32 Func_AI_AttackManager = 0x0043ABB0;
		static bool result;
		__asm
		{
			PUSHAD
			PUSH bFallbackAirOnly
			PUSH bOverrideLastAttack
			PUSH y
			PUSH x
			PUSH player
			CALL Func_AI_AttackManager
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For orders_Pickup4_0
	void issueOrderTargetingGround(Unit* unit, u8 orderId, s16 x, s16 y)
	{
		const u32 Func_issueOrderTargetingGround = 0x00475260;
		__asm
		{
			PUSHAD
			PUSH y
			PUSH x
			PUSH orderId
			MOV ESI, unit
			CALL Func_issueOrderTargetingGround
			POPAD
		}
	}

	// For initGame
	void switchAllAvailableComputerSlotsToOpen()
	{
		const u32 Func_switchAllAvailableComputerSlotsToOpen = 0x004DBBE0;
		__asm
		{
			PUSHAD
			CALL Func_switchAllAvailableComputerSlotsToOpen
			POPAD
		}
	}

	bool setLastOpenSlotToComputer(s8 race)
	{
		const u32 Func_setLastOpenSlotToComputer = 0x004DBB70;
		static s8 result;
		__asm
		{
			PUSHAD
			PUSH race
			CALL Func_setLastOpenSlotToComputer
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For startGame
	s32 bootReason(u8 player)
	{
		const u32 Func_bootReason = 0x004B8870;
		static s32 result;
		__asm
		{
			PUSHAD
			MOVZX AL, player
			CALL Func_bootReason
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 isAuthedMap(void* a1)
	{
		const u32 Func_isAuthedMap = 0x004C4870;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV ECX, a1
			CALL Func_isAuthedMap
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void setTimerFunc()
	{
		const u32 Func_setTimerFunc = 0x00472E00;
		__asm {
			PUSHAD
			CALL Func_setTimerFunc
			POPAD
		}
	}

	s32 getTickCount()
	{
		static u32 Func_getTickCount = *(u32*)0x004FE0C4;
		static s32 result;
		__asm {
			PUSHAD
			CALL Func_getTickCount
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For initializeBullet
	Flingy* initializeFlingy(s32 flingy_id, s16 y, s32 x, Flingy* flingy, char player, char angle)
	{
		const u32 Func_initializeFlingy = 0x00496360;
		static Flingy* result;
		__asm
		{
			PUSHAD
			PUSH angle
			PUSH player
			PUSH flingy
			MOV ECX, x
			MOV DX, y
			MOV EAX, flingy_id
			CALL Func_initializeFlingy
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void bulletBehaviorReassign(Bullet* bullet)
	{
		const u32 Func_bulletBehaviorReassign = 0x0048BD50;
		__asm
		{
			PUSHAD
			MOV ECX, bullet
			CALL Func_bulletBehaviorReassign
			POPAD
		}
	}

	s32 weaponBehaviorCanHitTargetCheck(Unit* target, Unit* attacker)
	{
		const u32 Func_weaponBehaviorCanHitTargetCheck = 0x004765B0;
		static s32 result;
		__asm
		{
			PUSHAD
			MOV ECX, attacker
			MOV EAX, target
			CALL Func_weaponBehaviorCanHitTargetCheck
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void appearOnTarget(Bullet* bullet, s16 x, s16 y)
	{
		const u32 Func_appearOnTarget = 0x004955C0;
		__asm
		{
			PUSHAD
			MOV CX, y
			MOV DX, x
			MOV EAX, bullet
			CALL Func_appearOnTarget
			POPAD
		}
	}

	Bullet* attackFlyToTarget(Bullet* bullet, s32 x, s32 y)
	{
		const u32 Func_attackFlyToTarget = 0x00494B70;
		static Bullet* result;
		__asm
		{
			PUSHAD
			MOV ECX, y
			MOV EDX, x
			MOV EAX, bullet
			CALL Func_attackFlyToTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void display_error_message_id_with_sfx(s32 playerId, s32 messageId, s32 soundId)
	{
		const u32 Func_displayErrorMessageIDWithSFX = 0x0048EE30;
		__asm
		{
			PUSHAD
			MOV ESI, soundId
			MOV EDI, messageId
			MOV EBX, playerId
			CALL Func_displayErrorMessageIDWithSFX
			POPAD
		}
	}

	// For ai_manage_building_placement
	s32 get_bunker_placement_position(s32 placementId, s32* regionId)
	{
		const u32 Func_getBunkerPlacementPosition = 0x004442E0;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH regionId
			PUSH placementId
			CALL Func_getBunkerPlacementPosition
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* get_disabled_unit_in_range(s32 owner, Point16 position)
	{
		const u32 Func_getDisabledUnitInRange = 0x00440420;
		static Unit* result;
		__asm
		{
			PUSHAD
			PUSH position
			PUSH owner
			CALL Func_getDisabledUnitInRange
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_get_build_location(s32 unitId, char* placementData, s32 playerId, Point16 tryPosition, s32 maxRange)
	{
		const u32 Func_AIGetBuildLocation = 0x004461B0;
		__asm
		{
			PUSHAD
			PUSH maxRange
			PUSH tryPosition
			PUSH playerId
			MOV ESI, placementData
			MOV EBX, unitId
			CALL Func_AIGetBuildLocation
			POPAD
		}
	}

	void ai_placement_data_something(s8* placementData, Point16 tryPosition)
	{
		const u32 Func_AIPlacementDataSomething = 0x00443610;
		__asm
		{
			PUSHAD
			PUSH tryPosition
			MOV EBX, placementData
			CALL Func_AIPlacementDataSomething
			POPAD
		}
	}

	u32 getTileDistance(s32 x, s32 y, s32 x2, s32 y2)
	{
		const u32 Func_getTileDistance = 0x00444100;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH y2
			PUSH x2
			PUSH y
			PUSH x
			CALL Func_getTileDistance
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For initializeUnitCounts
	char teamGame_CountRaces(char force, char* bIsLegal, char race)
	{
		const u32 Func_teamGame_CountRaces = 0x004857B0;
		s32 raceArg = race;
		static char result;
		__asm
		{
			PUSHAD
			PUSH raceArg
			MOV EDI, bIsLegal
			MOV AL, force
			CALL Func_teamGame_CountRaces
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For StatRes_MainProc
	void showSupplyInfo(BinDlg* dialog, s16 graphic, s16 mathA, u16 mathB)
	{
		const u32 Func_showSupplyInfo = 0x004E53B0;
		s32 graphicArg = graphic;
		s32 mathAArg = mathA;
		s32 mathBArg = mathB;
		__asm
		{
			PUSHAD
			PUSH mathBArg
			PUSH mathAArg
			PUSH graphicArg
			MOV EBX, dialog
			CALL Func_showSupplyInfo
			POPAD
		}
	}

	void printStatResInfo(BinDlg* dialog, s16 graphic, char** a3, char* a4, s32 bSomething)
	{
		const u32 Func_printStatResInfo = 0x004E52E0;
		s32 graphicArg = graphic;
		__asm
		{
			PUSHAD
			PUSH bSomething
			PUSH a4
			PUSH a3
			PUSH graphicArg
			MOV EBX, dialog
			CALL Func_printStatResInfo
			POPAD
		}
	}

	// For redrawSupplyInfo
	bool unitStatCond_Standard()
	{
		const u32 Func_unitStatCondStandard = 0x00424980;
		static bool result;
		__asm
		{
			PUSHAD
			CALL Func_unitStatCondStandard
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For drawSupplyUnitStatus
	void draw_shield_upgrade(BinDlg* dialog, u16 statusScreenIndex)
	{
		const u32 Func_drawShieldUpgrade = 0x00425510;
		u32 statusScreenIndexArg = statusScreenIndex;
		__asm
		{
			PUSHAD
			PUSH statusScreenIndexArg
			MOV EAX, dialog
			CALL Func_drawShieldUpgrade
			POPAD
		}
	}

	void drawArmor(BinDlg* dialog, s32 unitId, u16 statusScreenIndex)
	{
		const u32 Func_drawArmor = 0x00425600;
		u32 statusScreenIndexArg = statusScreenIndex;
		__asm
		{
			PUSHAD
			PUSH statusScreenIndexArg
			MOV EDX, unitId
			MOV EAX, dialog
			CALL Func_drawArmor
			POPAD
		}
	}

	// For progressBulletFrame
	void bulletBehaviorFly(Bullet* bullet)
	{
		const u32 Func_bulletBehaviorFly = 0x0048B3B0;
		__asm
		{
			PUSHAD
			MOV EAX, bullet
			CALL Func_bulletBehaviorFly
			POPAD
		}
	}

	void bulletBehaviorFollow(Bullet* bullet)
	{
		const u32 Func_bulletBehaviorFollow = 0x0048B500;
		__asm
		{
			PUSHAD
			MOV EAX, bullet
			CALL Func_bulletBehaviorFollow
			POPAD
		}
	}

	void bulletBehaviorBounce(Bullet* bullet)
	{
		const u32 Func_bulletBehaviorBounce = 0x0048B2D0;
		__asm
		{
			PUSHAD
			MOV EDI, bullet
			CALL Func_bulletBehaviorBounce
			POPAD
		}
	}

	void bulletBehaviorPersist(Bullet* bullet)
	{
		const u32 Func_bulletBehaviorPersist = 0x0048BC70;
		__asm
		{
			PUSHAD
			MOV ECX, bullet
			CALL Func_bulletBehaviorPersist
			POPAD
		}
	}

	void bulletBehaviorInstant(Bullet* bullet)
	{
		const u32 Func_bulletBehaviorInstant = 0x0048A560;
		__asm
		{
			PUSHAD
			MOV EAX, bullet
			CALL Func_bulletBehaviorInstant
			POPAD
		}
	}

	void bulletBehaviorArea(Bullet* bullet)
	{
		const u32 Func_bulletBehaviorArea = 0x0048B440;
		__asm
		{
			PUSHAD
			MOV EAX, bullet
			CALL Func_bulletBehaviorArea
			POPAD
		}
	}

	// For giveUnitToPlayer
	void giveAllUnitPropertiesToPlayer(Unit* unit, s8 playerId)
	{
		const u32 Func_giveAllUnitPropertiesToPlayer = 0x0047C9F0;
		s32 playerIdArg = playerId;
		__asm
		{
			PUSHAD
			PUSH playerIdArg
			MOV EAX, unit
			CALL Func_giveAllUnitPropertiesToPlayer
			POPAD
		}
	}

	Unit* getHiddenMiner(Unit* unit)
	{
		const u32 Func_getHiddenMiner = 0x00468A60;
		static Unit* result;
		__asm
		{
			PUSHAD
			MOV ECX, unit
			CALL Func_getHiddenMiner
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unitHasHanger(Unit* unit)
	{
		return unit->id == UnitId::ProtossAccantor || unit->id == UnitId::ProtossSolarion;
	}

	s32 forHangarUnits(Unit* unit, Unit* (__fastcall* func)(Unit*, Unit*), s32 arg)
	{
		const u32 Func_forHangarUnits = 0x00465200;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH arg
			PUSH func
			MOV EDI, unit
			CALL Func_forHangarUnits
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* getCreatingUnit(Unit* unit)
	{
		const u32 Func_getCreatingUnit = 0x00466A30;
		static Unit* result;
		__asm
		{
			PUSHAD
			MOV ECX, unit
			CALL Func_getCreatingUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void MoveSprite(Sprite* sprite, s16 x, s16 y)
	{
		const u32 Func_MoveSprite = 0x00497A10;
		__asm
		{
			PUSHAD
			MOV ECX, sprite
			MOV BX, x
			MOV DI, y
			CALL Func_MoveSprite
			POPAD
		}
	}

	u32 updateUnitSortingInfo(u32 x_diff, Unit* unit, u32 y_diff)
	{
		const u32 Func_updateUnitSortingInfo = 0x0046A000;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH y_diff
			MOV ESI, unit
			MOV EAX, x_diff
			CALL Func_updateUnitSortingInfo
			MOV result, EAX
			POPAD
		}
		return result;
	}
	
	// For bulletBehaviorReassign
	void playSpriteIscript(Sprite* sprite, char scriptId, s32 bPlay)
	{
		const u32 Func_playSpriteIscript = 0x00499D00;
		s32 scriptIdArg = scriptId;
		__asm
		{
			PUSHAD
			PUSH bPlay
			PUSH scriptIdArg
			MOV ECX, sprite
			CALL Func_playSpriteIscript
			POPAD
		}
	}

	Bullet* assignWeaponData(Bullet* bullet, char behaviorType)
	{
		const u32 Func_assignWeaponData = 0x0048AA80;
		static Bullet* result;
		__asm
		{
			PUSHAD
			MOV DL, behaviorType
			MOV EAX, bullet
			CALL Func_assignWeaponData
			MOV result, EAX
			POPAD
		}
		return result;
	}
	
	// For ai_training_normal
	AiRegion* getAiRegionInfoFromUnitLoc(Unit* unit)
	{
		const u32 Func_getAiRegionInfoFromUnitLoc = 0x00438BF0;
		static AiRegion* result;
		__asm
		{
			PUSHAD
			MOV ESI, unit
			CALL Func_getAiRegionInfoFromUnitLoc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void add_guard_ai_to_unit(Unit* unit)
	{
		const u32 Func_AI_AddGuardAiToUnit = 0x00462960;
		__asm
		{
			PUSHAD
			PUSH unit
			CALL Func_AI_AddGuardAiToUnit
			POPAD
		}
	}

	s8 create_unit_allowed(u16 unitId, Unit* unit, s32 playerID)
	{
		const u32 Func_createUnitAllowed = 0x0046E1C0;
		static s8 result;
		__asm
		{
			PUSHAD
			PUSH playerID
			MOV ESI, unit
			MOV AX, unitId
			CALL Func_createUnitAllowed
			MOV result, AL
			POPAD
		}
		return result;
	}

	b32 SBmpLoadImage(const char *filename, void* palette, void* buffer, u32 buffersize, u32* width, u32* height, u32* bpp) // 00410154
	{
		const u32 Func_SBmpLoadImage = 0x00410154;
		static b32 result;
		__asm
		{
			PUSHAD
			PUSH bpp
			PUSH height
			PUSH width
			PUSH buffersize
			PUSH buffer
			PUSH palette
			PUSH filename
			CALL Func_SBmpLoadImage
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_military_should_wait_for_group
	bool ai_are_on_connected_regions(Unit* unit1, Unit* unit2)
	{
		const u32 Func_AI_AreOnConnectedRegions = 0x00438E10;
		static bool result;
		__asm
		{
			PUSHAD
			MOV ECX, unit2
			MOV EAX, unit1
			CALL Func_AI_AreOnConnectedRegions
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool ai_check_military_grouping(Unit* unit)
	{
		const u32 Func_AI_CheckMilitaryGrouping = 0x00439550;
		static bool result;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_AI_CheckMilitaryGrouping
			MOV result, AL
			POPAD
		}
		return result;
	}

	// For remove_unit_ai
	s32 remove_from_ai_structs(Unit* unit, UnitAi* unitAI)
	{
		const u32 Func_AI_RemoveFromAIStructs = 0x00439D60;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH unitAI
			PUSH unit
			CALL Func_AI_RemoveFromAIStructs
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 remove_worker_or_building_ai(Unit* unit, UnitAi* unitAI)
	{
		const u32 Func_AI_RemoveWorkerOrBuildingAI = 0x00434C90;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH unitAI
			MOV EDI, unit
			CALL Func_AI_RemoveWorkerOrBuildingAI
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_remove_medic(Unit* unit)
	{
		const u32 Func_AI_MedicRemove = 0x004477C0;
		__asm
		{
			PUSHAD
			MOV EDX, unit
			CALL Func_AI_MedicRemove
			POPAD
		}
	}

	void ai_training_simple_military(Unit* newUnit, Unit* sourceUnit)
	{
		const u32 Func_AI_TrainingSimpleMilitary = 0x0043E280;
		__asm
		{
			PUSHAD
			PUSH sourceUnit
			PUSH newUnit
			CALL Func_AI_TrainingSimpleMilitary
			POPAD
		}
	}

	// For delete_ai_town_if_needed
	AiTown* try_begin_delete_town(AiTown* town)
	{
		const u32 Func_tryBeginDeleteTown = 0x00434330;
		static AiTown* result;
		__asm
		{
			PUSHAD
			MOV ESI, town
			CALL Func_tryBeginDeleteTown
			MOV result, EAX
			POPAD
		}
		return result;
	}

	AiScript* delete_aiscripts_using_town(AiTown* town)
	{
		const u32 Func_deleteAIScriptsUsingTown = 0x0045AE20;
		static AiScript* result;
		__asm
		{
			PUSHAD
			MOV EBX, town
			CALL Func_deleteAIScriptsUsingTown
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For displayEstablishingShot
	void* loadInitCreditsBIN(char* a1)
	{
		s32 address = 0x004D8DF0;
		void* result_;
		__asm
		{
			MOV EAX, a1
			CALL address
			MOV result_, EAX
		}
		return result_;
	}

	bool SFileOpenArchive(char* archiveName, u32 dwPriority, u32 dwFlags, void* const* handle)
	{
		const u32 Func_SFileOpenArchive = 0x004100B2;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH handle
			PUSH dwFlags
			PUSH dwPriority
			PUSH archiveName
			CALL Func_SFileOpenArchive
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool SFileCloseArchive(void* archive)
	{
		const u32 Func_SFileCloseArchive = 0x004100BE;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH archive
			CALL Func_SFileCloseArchive
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool SFileOpenFileEx(void* hMPQ, const char* szFileName, u32 dwSearchScope, void** phFile)
	{
		const u32 Func_SFileOpenFileEx = 0x004100C4;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH phFile
			PUSH dwSearchScope
			PUSH szFileName
			PUSH hMPQ
			CALL Func_SFileOpenFileEx
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
	bool ai_regions_are_connected(u32 playerId, u32 region1, u32 region2)
	{
		const u32 Func_AI_RegionsAreConnected = 0x00437E70;
		static bool result;
		__asm
		{
			PUSHAD
			MOV ESI, region2
			MOV EBX, region1
			MOV EAX, playerId
			CALL Func_AI_RegionsAreConnected
			MOV result, AL
			POPAD
		}
		return result;
	}
	
	// For BTNSCOND_ZergBuildingRally
	u32 BTNSCOND_Rally(u32 unused, u32 player, Unit* unit)
	{
		const u32 Func_BTNSCOND_Rally = 0x00429520;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH unit
			MOV EDX, player
			MOV ECX, unused
			CALL Func_BTNSCOND_Rally
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For AI_GetBuildingLocation
	u32 ai_get_building_placement_data(u32 maxRange, u32 unitId, char* placementData, u32 playerId, Point16 desiredPos)
	{
		const u32 Func_Ai_GetBuildingPlacementData = 0x00445D60;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH desiredPos
			PUSH playerId
			PUSH placementData
			PUSH unitId
			MOV EAX, maxRange
			CALL Func_Ai_GetBuildingPlacementData
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char* ai_remove_unreachable_regions(char* placementData, Point16 desiredPos)
	{
		const u32 Func_Ai_RemoveUnreachableRegions = 0x00443900;
		static char* result;
		__asm
		{
			PUSHAD
			PUSH desiredPos
			PUSH placementData
			CALL Func_Ai_RemoveUnreachableRegions
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_remove_dangerous_regions(u32 unitId, char* placementData, u32 placementId, Point16 desiredPos)
	{
		const u32 Func_Ai_RemoveDangerousRegions = 0x00443870;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH desiredPos
			PUSH placementId
			MOV ECX, placementData
			MOV EAX, unitId
			CALL Func_Ai_RemoveDangerousRegions
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_remove_far_tiles(char* placementData, Point16 desiredPos, u32 maxRange)
	{
		const u32 Func_Ai_RemoveFarAwayTiles = 0x00444E20;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH maxRange
			PUSH desiredPos
			MOV ECX, placementData
			CALL Func_Ai_RemoveFarAwayTiles
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_remove_different_height(char* placementData, Point16 desiredPos)
	{
		const u32 Func_Ai_RemoveDifferentHeight = 0x004436C0;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH desiredPos
			PUSH placementData
			CALL Func_Ai_RemoveDifferentHeight
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 ai_placement_related_something(u16 unitId, char* placementData, Point16 desiredPos)
	{
		const u32 Func_Ai_PlacementRelatedSomething = 0x00443E30;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH desiredPos
			PUSH placementData
			PUSH unitId
			CALL Func_Ai_PlacementRelatedSomething
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_remove_unfit_tiles(u32 unitId, char* placementData, u32 playerId, Point16 desiredPos)
	{
		const u32 Func_Ai_RemoveTilesIfUnfit = 0x00444CE0;
		__asm
		{
			PUSHAD
			PUSH desiredPos
			PUSH playerId
			PUSH placementData
			MOV ECX, unitId
			CALL Func_Ai_RemoveTilesIfUnfit
			POPAD
		}
	}

	u32 ai_remove_addon_places(u32 unitId, char* placementData, u32 playerId, Point16 desiredPos)
	{
		const u32 Func_Ai_RemoveAddonPlaces = 0x00443CC0;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH desiredPos
			PUSH playerId
			PUSH placementData
			PUSH unitId
			CALL Func_Ai_RemoveAddonPlaces
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_turret_placement_position(char* placementData, u32 playerId, Point16 desiredPos, u16 unitId, u32 unkFlag)
	{
		const u32 Func_getTurretPlacementPosition = 0x00443AC0;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH unkFlag
			PUSH unitId
			PUSH desiredPos
			PUSH playerId
			PUSH placementData
			CALL Func_getTurretPlacementPosition
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_pylon_placement_position(char* placementData, u8 playerId, Point16 desiredPos)
	{
		const u32 Func_getPylonPlacementPosition = 0x004448D0;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH desiredPos
			PUSH playerId
			PUSH placementData
			CALL Func_getPylonPlacementPosition
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 get_default_placement_position(u32 unitId, char* placementData, u32 playerId, Point16 desiredPos)
	{
		const u32 Func_getDefaultPlacementPosition = 0x004449A0;
		static u32 result;
		__asm
		{
			PUSHAD
			PUSH desiredPos
			PUSH playerId
			PUSH placementData
			MOV ECX, unitId
			CALL Func_getDefaultPlacementPosition
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_check_military_grouping
	void get_region_center_from_id(u16 regionId, u32* out_x, u32* out_y)
	{
		const u32 Func_getRegionCenter = 0x0049CAA0;
		__asm
		{
			PUSHAD
			PUSH out_y
			PUSH out_x
			MOV AX, regionId
			CALL Func_getRegionCenter
			POPAD
		}
	}

	// For ignoreIscriptGoToIdle
	void playIdleIscript(Sprite* sprite)
	{
		const u32 Func_PlayIdleIscript = 0x00497880;
		__asm
		{
			PUSHAD
			MOV EAX, sprite
			CALL Func_PlayIdleIscript
			POPAD
		}
	}

	// For issueOrderTargetingGround
	void order_copy(s32 a1, Unit* a2, s8 a3, Point16* a4, Unit* a5, s32 a6)
	{
		const u32 Func_order = 0x00474810;
		__asm
		{
			PUSHAD
			MOV EAX, a1
			MOV EDX, a2
			MOVZX CL, a3
			PUSH a4
			PUSH a5
			PUSH a6
			CALL Func_order
			POPAD
		}
	}

	static u32 __cdecl removeCreepAtUnit_nop(u32 x_tile, u32 y_tile, Box32* rect)
	{
		return 0;
	}

	// For unit_destructor_special
	s32 removeCreepAtUnit(s32 left, s32 top, s16 unit_id)
	{
		const u32 Func_removeCreepAtUnit = 0x00414560;
		static s32 result;
		__asm
		{
			PUSHAD
			PUSH removeCreepAtUnit_nop
			MOV DX, unit_id
			MOV EAX, top
			MOV ECX, left
			CALL Func_removeCreepAtUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For AI_AttackUnit
	void stimpackProc(Unit* unit)
	{
		const u32 Func_stimpackSFXRand = 0x004554A0;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_stimpackSFXRand
			POPAD
		}
	}

	/*
	// For soundThread
	void fastFileRead(s32 a1, s32 a2, char* a3, s32 fileName, s32 buffer, s32 bytesRead, char* logFileName, s32 logLine)
	{
		const u32 Func_fastFileRead = 0x004D2D10;
		__asm
		{
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

	s32 parseWaveFile(s32 a1, s32 a2, s32 a3)
	{
		static s32 result;
		const u32 Func_parseWaveFile = 0x004BBD80;
		__asm
		{
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

	bool SMemFree(void* location, char* logFileName, s32 logLine, char defaultValue)
	{
		static s32 result;
		const u32 Func_SMemFree = 0x00410070;
		__asm
		{
			PUSHAD
			PUSH defaultValue
			PUSH logLine
			PUSH logFileName
			PUSH location
			CALL Func_SMemFree
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	u32 getSoundDuration(s32 a1)
	{
		static u32 result;
		const u32 Func_getSoundDuration = 0x004BB890;
		__asm
		{
			PUSHAD
			MOV ESI, a1
			CALL Func_getSoundDuration
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For soundInit
	void dSoundDestroy()
	{
		const u32 Func_dSoundDestroy = 0x004BC180;
		__asm
		{
			PUSHAD
			CALL Func_dSoundDestroy
			POPAD
		}
	}

	s32 dSoundCreate(s32 a1)
	{
		static s32 result;
		const u32 Func_dSoundCreate = 0x004BB640;
		__asm
		{
			PUSHAD
			MOV ESI, a1
			CALL Func_dSoundCreate
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 setCoopLevel(s32 a1, s32 a2)
	{
		static s32 result;
		const u32 Func_setCoopLevel = 0x004BB600;
		__asm
		{
			PUSHAD
			MOV EDI, a1
			MOV ESI, a2
			CALL Func_setCoopLevel
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 createSoundBuffer(s32 a1)
	{
		static s32 result;
		const u32 Func_createSoundBuffer = 0x004BB5A0;
		__asm
		{
			PUSHAD
			MOV ESI, a1
			CALL Func_createSoundBuffer
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 unkSoundCoreFunc()
	{
		static s32 result;
		const u32 Func_unkSoundCoreFunc = 0x004BBA90;
		__asm
		{
			PUSHAD
			CALL Func_unkSoundCoreFunc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 sFileDdaInitialize(char directSound)
	{
		static s32 result;
		const u32 Func_sFileDdaInitialize = 0x004101C0;
		__asm
		{
			PUSHAD
			PUSH directSound
			CALL Func_sFileDdaInitialize
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 sVidInitialize(char video)
	{
		static s32 result;
		const u32 Func_sVidInitialize = 0x004101C6;
		__asm
		{
			PUSHAD
			PUSH video
			CALL Func_sVidInitialize
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 loadButtonSFXFile()
	{
		static s32 result;
		const u32 Func_loadButtonSFXFile = 0x004BCB20;
		__asm
		{
			PUSHAD
			CALL Func_loadButtonSFXFile
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void initVolume()
	{
		const u32 Func_initVolume = 0x004BC450;
		__asm
		{
			PUSHAD
			CALL Func_initVolume
			POPAD
		}
	}

	s32 sRegLoadValue(char keyName, char valueName, char a3, char value)
	{
		static s32 result;
		const u32 Func_sRegLoadValue = 0x00410076;
		__asm
		{
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

	HANDLE beginThreadEx(s32 lpThreadAttributes, s32 dwStackSize, s32 threadId, s32 a4, s32 dwCreationFlags, s32 lpThreadId)
	{
		static HANDLE result;
		const u32 Func_beginThreadEx = 0x0040CD3C;
		__asm
		{
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
	void ai_overlord_behavior(Unit* unit)
	{
		const u32 Func_AI_OverlordBehavior = 0x00434DF0;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_AI_OverlordBehavior
			POPAD
		}
	}

	void ai_nuke_silo_behavior(Unit* unit)
	{
		const u32 Func_AI_NukeSiloBehavior = 0x00434D50;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_AI_NukeSiloBehavior
			POPAD
		}
	}

	bool ai_unit_construction_behavior(s32 playerId, Unit* unit, s32 someBool)
	{
		static s32 result;
		const u32 Func_AI_UnitConstructionBehavior = 0x00433890;
		__asm
		{
			PUSHAD
			PUSH someBool
			PUSH unit
			PUSH playerId
			CALL Func_AI_UnitConstructionBehavior
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	char ai_plan_do_merge_morph(s32 playerId, Unit* unit, s32 someBool)
	{
		static s32 result;
		const u32 Func_AI_PlanDoMergeMorph = 0x00435F10;
		__asm
		{
			PUSHAD
			PUSH someBool
			PUSH unit
			PUSH playerId
			CALL Func_AI_PlanDoMergeMorph
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	s32 ai_morph_behavior(s32 player, Unit* unit, void* unit_ai)
	{
		static s32 result;
		const u32 Func_AI_MorphBehavior = 0x00434FF0;
		__asm
		{
			PUSHAD
			PUSH unit_ai
			PUSH unit
			MOV EBX, player
			CALL Func_AI_MorphBehavior
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void ai_train_behavior(Unit* unit, void* unit_ai, s32 player)
	{
		const u32 Func_AI_TrainBehavior = 0x00434720;
		__asm
		{
			PUSHAD
			MOV EBX, player
			MOV EDX, unit_ai
			MOV EAX, unit
			CALL Func_AI_TrainBehavior
			POPAD
		}
	}

	void ai_upgrade_behavior(Unit* unit, s32 player)
	{
		const u32 Func_AI_UpgradeBehavior = 0x00434670;
		__asm
		{
			PUSHAD
			MOV ECX, player
			MOV EAX, unit
			CALL Func_AI_UpgradeBehavior
			POPAD
		}
	}

	s32 ai_research_behavior(Unit* unit, s32 player) // return value unused
	{
		static s32 result;
		const u32 Func_AI_ResearchBehavior = 0x0043462F;
		__asm
		{
			PUSHAD
			MOV ECX, player
			MOV EAX, unit
			CALL Func_AI_ResearchBehavior
			MOV result, EAX
			POPAD
		}
		return result;
	}

	char ai_construct_behavior(Unit* unit, void* unit_ai, s32 player)
	{
		static s32 result;
		const u32 Func_AI_ConstructBehavior = 0x00434E70;
		__asm
		{
			PUSHAD
			PUSH player
			MOV EAX, unit_ai
			MOV ECX, unit
			CALL Func_AI_ConstructBehavior
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	s32 ai_make_detectors_behavior(Unit* unit, s32 player)
	{
		static s32 result;
		const u32 Func_AI_MakeDetectorBehavior = 0x00435130;
		__asm
		{
			PUSHAD
			MOV EDI, player
			MOV EAX, unit
			CALL Func_AI_MakeDetectorBehavior
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For ai_plan_do_merge_morph

	Unit* ai_get_mutalisk_for_morph(s32 player)
	{
		static Unit* result;
		const u32 Func_AI_GetMutaliskForMorph = 0x004375F0;
		__asm
		{
			PUSHAD
			MOV EAX, player
			CALL Func_AI_GetMutaliskForMorph
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_get_high_templar_for_summon(s32 player, Unit** first, Unit** second)
	{
		static s32 result;
		const u32 Func_AI_GetHighTemplarsForSummon = 0x00437410;
		__asm
		{
			PUSHAD
			PUSH second
			PUSH first
			PUSH player
			CALL Func_AI_GetHighTemplarsForSummon
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 ai_get_dark_templar_for_summon(s32 player, Unit** first, Unit** second)
	{
		static s32 result;
		const u32 Func_AI_GetDarkTemplarsForSummon = 0x00437320;
		__asm
		{
			PUSHAD
			PUSH second
			MOV EBX, first
			MOV EAX, player
			CALL Func_AI_GetDarkTemplarsForSummon
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void assign_order_with_target(Unit* unit, Unit* target, s32 orderId)
	{
		const u32 Func_AssignOrderWithTarget = 0x004752B0;
		__asm
		{
			PUSHAD
			PUSH orderId
			MOV ESI, target
			MOV EAX, unit
			CALL Func_AssignOrderWithTarget
			POPAD
		}
	}

	s16 ai_reserve(s32 player, s32 someBool)
	{
		static s16 result;
		const u32 Func_AI_Reserve = 0x00447D00;
		__asm
		{
			PUSHAD
			PUSH someBool
			MOV ECX, player
			CALL Func_AI_Reserve
			MOV result, AX
			POPAD
		}
		return result;
	}

	AI_Main* ai_pop_spending_request(s32 player)
	{
		static AI_Main* result;
		const u32 Func_AI_PopSpendingRequest = 0x004476D0;
		__asm
		{
			PUSHAD
			MOV EAX, player
			CALL Func_AI_PopSpendingRequest
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Unit* AI_GetHydraliskForMorph(s32 player)
	{
		static Unit* result;
		const u32 Func_AI_GetHydraliskForMorph = 0x00437570;
		__asm
		{
			PUSHAD
			MOV EAX, player
			CALL Func_AI_GetHydraliskForMorph
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 is_unit_id_race_equal_to_unit_race(u16 unitId, Unit* unit)
	{
		static s32 result;
		const u32 Func_isUnitTypeRaceUnitRace = 0x00432480;
		__asm
		{
			PUSHAD
			MOV ECX, unit
			MOV AX, unitId
			CALL Func_isUnitTypeRaceUnitRace
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool hasResourcesForUnit(u16 unitId, s32 player)
	{
		static bool result;
		const u32 Func_hasResourcesForUnit = 0x004487B0;
		__asm
		{
			PUSHAD
			PUSH player
			MOV CX, unitId
			CALL Func_hasResourcesForUnit
			MOV result, AL
			POPAD
		}
		return result;
	}

	char ai_perform_task(Unit* unit)
	{
		static char result;
		const u32 Func_AI_PerformTask = 0x004361A0;
		__asm
		{
			PUSHAD
			MOV ECX, unit
			CALL Func_AI_PerformTask
			MOV result, AL
			POPAD
		}
		return result;
	}

	s32 ai_get_request_build_type(s32 player)
	{
		static s32 result;
		const u32 Func_AI_GetRequestedBuildType = 0x00447590;
		__asm
		{
			PUSHAD
			MOV EAX, player
			CALL Func_AI_GetRequestedBuildType
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool ai_train_unit(Unit* unit, u16 unitId, char requestId, void* value)
	{
		static bool result;
		const u32 Func_AI_TrainUnit = 0x004672F0;
		__asm
		{
			PUSHAD
			PUSH value
			PUSH requestId
			PUSH unitId
			MOV EAX, unit
			CALL Func_AI_TrainUnit
			MOV result, AL
			POPAD
		}
		return result;
	}

	Unit* ai_create_unit(u32 unit_id, s32 player, s32 x, s32 y) {
		static Unit* result;
		const u32 Func_AI_CreateUnit = 0x004A0AD0;
		__asm
		{
			PUSHAD
			PUSH player
			PUSH unit_id
			MOV ECX, x
			MOV EAX, y
			CALL Func_AI_CreateUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 parse_aiscript_name(s32 name, s32* is_bw) {
		static s32 result;
		const u32 Func_parseAIScriptName = 0x0045B170;
		__asm
		{
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
		__asm
		{
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
		__asm
		{
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
		__asm
		{
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
		__asm
		{
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
		__asm
		{
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
		__asm
		{
			PUSHAD
			MOV EAX, text
			CALL Func_GetTextDisplayTime
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For prepare_unit_move_custom
	s32 sort_unit(Unit* unit)
	{
		static s32 result;
		const u32 Func_sort_unit = 0x0046A300;
		__asm
		{
			PUSHAD
			MOV ESI, unit
			CALL Func_sort_unit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void refresh_grp_image(Sprite* sprite)
	{
		const u32 Func_refresh_grp_image = 0x00497530;
		__asm
		{
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

	playNextMusic_Func* playNextMusic = (playNextMusic_Func*) 0x004A6250;
	stopMusic_Func* stopMusic         = (stopMusic_Func*)     0x004A5F50;

	void prepare_formation_movement(MovementGroup* a1, char a2) { // 0049A8C0
		const u32 func_address = 0x0049A8C0;
		__asm {
			PUSHAD
			PUSH a2
			MOV EDI, a1
			CALL func_address
			POPAD
		}
	}

	Unit* find_fow_unit(__int16 ax0, __int16 a2, unsigned __int16 a3) { // 00487FD0
		static Unit* result;
		const u32 func_address = 0x00487FD0;
		__asm {
			PUSHAD
			PUSH a3
			MOV CX, a2
			MOV AX, ax0
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void targeted_order_unit(Unit* a1, Unit* a2, int a3, char a4) { // 004756E0
		const u32 func_address = 0x004756E0;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			MOV ESI, a2
			MOV EDX, a1
			CALL func_address
			POPAD
		}
	}

	void targeted_order_ground(int a1, int a2, Unit* a3, int a4, int a5, int a6) { // 004756B0
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

	Unit* get_next_commanded_unit() { // 0049A850
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

	void get_formation_movement_target(Unit *a1, MovementGroup* a2) { // 0049A500
		const u32 func_address = 0x0049A500;
		__asm {
			PUSHAD
			PUSH a2
			PUSH a1
			CALL func_address
			POPAD
		}
	}

	u8 get_right_click_order(u32 index, Unit* unit, s32 x, s32 y, Unit** target, s32 unit_id) {
		static u8 result;
		void* function = GetRightClickOrderArray[index];
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
		__asm {
			PUSHAD
			MOV ESI, a1
			PUSH a2
			CALL func_address
			POPAD
		}
	}

	// For HandleWindowMessages
	char check_last_file_error()
	{
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

	s32 send_input_to_all_dialogs(dlgEvent* dlg_event) {
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
		__asm {
			PUSHAD
			PUSH priority
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
		__asm {
			PUSHAD
			PUSH finished
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

	s32 get_threat_level(Unit* attacker, Unit* target) {
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

	bool unit_can_attack_target(Unit* target, Unit* attacker, s32 check_duration) {
		static bool result;
		const u32 func_address = 0x00476730;
		__asm {
			PUSHAD
			PUSH check_duration
			MOV ESI, attacker
			MOV EBX, target
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	s32 dialog_set_progress_bar_value(BinDlg* dialog, s16 index, s16 value) {
		static s32 result;
		const u32 func_address = 0x00457200;
		__asm {
			PUSHAD
			PUSH value
			MOV CX, index
			MOV EAX, dialog
			call func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// Replaces original functionality
	bool unit_is_factory(Unit* unit) { // 0047B2E0
		return units_dat::GroupFlags[unit->id].isFactory;
	}

	bool is_unit_target_inside_min_range(Unit* unit, Unit* target) {
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
			MOV CX, y
			MOV AX, x
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
			MOV DL, order_id
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
			MOV CL, order_id
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

	s32 ai_scv_repair_controller(Unit* unit, AiTown* town) {
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

	s32 mark_creep_to_disappear(u32 tile_x, u32 tile_y) {
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

	void load_dialog(char* filename, void* function) {
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

	unitPath* reassign_path(Unit* unit) {
		static unitPath* result;
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
		__asm {
			PUSHAD
			PUSH player_id
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
			MOV AL, order_id
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
			MOV AX, region_id
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
		__asm {
			PUSHAD
			PUSH iscript_anim
			MOV EDI, flag_or_timer
			MOV BL, weapon_id
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
			MOV CX, tile_y
			MOV DX, tile_x
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
		__asm {
			PUSHAD
			PUSH vision_filter
			MOV EBX, sprite
			CALL func_address
			POPAD
		}
	}

	void set_tooltip_info(u32 mineral_cost, u32 gas_cost, u32 p_energy, u32 t_energy, u32 z_energy, u32 p_supply, u32 t_supply, u16 z_supply) {
		const u32 func_address = 0x00458800;
		__asm {
			PUSHAD
			MOV EAX, mineral_cost
			MOV ECX, gas_cost
			PUSH z_supply
			PUSH t_supply
			PUSH p_supply
			PUSH z_energy
			PUSH t_energy
			MOV EDX, p_energy
			CALL func_address
			POPAD
		}
	}

	s32 bw_print_icon(Box16* area, Box16* dims, u16 right, u16 bottom) {
		static s32 result;
		const u32 func_address = 0x0041D090;
		__asm {
			PUSHAD
			PUSH bottom
			PUSH right
			MOV EDI, dims
			MOV EAX, area
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 bw_print_xy(char* dest, s32 width, s32 height) {
		static u32 result;
		const u32 func_address = 0x004202B0;
		__asm {
			PUSHAD
			PUSH height
			MOV ESI, width
			MOV EAX, dest
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result;
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
		__asm {
			PUSHAD
			PUSH message_id
			PUSH ping
			MOV EDI, timer
			MOV BL, event_type
			MOV unit, EAX
			call func_address
			POPAD
		}
	}
}