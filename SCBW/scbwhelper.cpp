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

	bool HasMoneyCanMake(Unit* builder, u32 unitToBuild)
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

	void getRaceSoundList(s8 a1)
	{
		const u32 Func_getRaceSoundList = 0x0048FB40;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_getRaceSoundList
			POPAD
		}
	}

	s32 dialogGrpConstruct(s32 a1, s32 a2, s32 a3, s32(__fastcall* a4)(s32, s32, s32, s32))
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

	void loadGraphic(s8 a1, s32 a2, s32 a3, s8 a4, s32 a5)
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

	s32 randFunc()
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

	bool unitHPbelow33_percent(Unit* a1)
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

	bool unitHPbetween33_66_percent(s32 a1, Unit* a2)
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

	s32 getMaxHP(Unit* a1)
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

	u16 getLastQueueSlotType(Unit* a1)
	{
		const u32 Func_getLastQueueSlotType = 0x0047B270;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, a1
			CALL Func_getLastQueueSlotType
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	void addTextToDialog(BinDlg* a1, s16 a2, const char* a3)
	{
		const u32 Func_addTextToDialog = 0x004258B0;
		static s32 _a3 = (s32)a3;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CX, a2
			PUSH _a3
			CALL Func_addTextToDialog
			POPAD
		}
	}

	void drawEnergy(BinDlg* a1)
	{
		const u32 Func_drawEnergy = 0x00425A30;
		__asm {
			PUSHAD
			MOV EBX, a1
			CALL Func_drawEnergy
			POPAD
		}
	}

	void hideOptionalStatusScreenControls(BinDlg* a1)
	{
		const u32 Func_hideOptionalStatusScreenControls = 0x00457310;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_hideOptionalStatusScreenControls
			POPAD
		}
	}

	void showControlGroup(BinDlg* dialog, s16 firstId, s16 lastId)
	{
		const u32 Func_showControlGroup = 0x00457250;
		__asm {
			PUSHAD
			PUSH lastId
			MOVZX CX, firstId
			MOV EAX, dialog
			CALL Func_showControlGroup
			POPAD
		}
	}

	void drawShieldUpgrade(s32 a1, u16 a2)
	{
		const u32 Func_drawShieldUpgrade = 0x00425510;
		__asm {
			PUSHAD
			PUSH a2
			MOV EAX, a1
			CALL Func_drawShieldUpgrade
			POPAD
		}
	}

	void hideDialog(BinDlg* a1)
	{
		const u32 Func_hideDialog = 0x00418700;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_hideDialog
			POPAD
		}
	}

	// For drawSupplyUnitInfo

	s32 getSuppliesUsed()
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

	u32 getSuppliesAvailable(u8 a1, s32 a2)
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

	void drawArmor(s32 a1, s32 a2, u16 a3)
	{
		const u32 Func_drawArmor = 0x00425600;
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOV EDX, a2
			CALL Func_drawArmor
			POPAD
		}
	}

	void unitStatAct_Standard(void* a1)
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

	bool AI_UnitCanAttack(Unit* a1)
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

	// For drawBuildingInfo

	s32 needRedrawUI()
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

	Unit* genericStatusUpdateDrawnValues()
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

	void drawProgressBar(BinDlg* a1)
	{
		const u32 Func_drawProgressBar = 0x004260C0;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawProgressBar
			POPAD
		}
	}

	void drawNukeSiloInfo(BinDlg* a1)
	{
		const u32 Func_drawNukeSiloInfo = 0x00426FF0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawNukeSiloInfo
			POPAD
		}
	}

	// Equivalent to code unitIsActiveTransport @ 0x004E6BA0
	// Modified to remove Ovileth checks as they are no longer tied to upgrades in HYDRA
	bool unitIsActiveTransport(Unit* unit)
	{
		bool result = false;
		if (!(unit->status & UnitStatus::IsHallucination))
		{
			result = units_dat::SpaceProvided[unit->id] != 0;
		}
		return result;
	}

	bool isQueueSlotActive(Unit* unit, s32 slot)
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

	void drawTrainingProgress(BinDlg* dialog)
	{
		const u32 Func_drawTrainingProgress = 0x004268D0;
		__asm {
			PUSHAD
			PUSH dialog
			CALL Func_drawTrainingProgress
			POPAD
		}
	}

	bool isConstructingAddon(const Unit* a1)
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

	bool isAttemptingProtossBuild(Unit* a1)
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

	void drawAddonProgress(BinDlg* a1)
	{
		const u32 Func_drawAddonProgress = 0x00425F30;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawAddonProgress
			POPAD
		}
	}

	void drawTechProgress(BinDlg* a1)
	{
		const u32 Func_drawTechProgress = 0x004266F0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawTechProgress
			POPAD
		}
	}

	void drawUpgradeProgress(BinDlg* a1)
	{
		const u32 Func_drawUpgradeProgress = 0x00426500;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawUpgradeProgress
			POPAD
		}
	}

	bool unitIsGeyser(Unit* a1)
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

	void drawGasQuantity(BinDlg* a1)
	{
		const u32 Func_drawGasQuantity = 0x00426190;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_drawGasQuantity
			POPAD
		}
	}

	s8 unitGetGroundWeapon(Unit* a1)
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

	void drawDisplayButtons(BinDlg* a1)
	{
		const u32 Func_drawDisplayButtons = 0x00426C60;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawDisplayButtons
			POPAD
		}
	}

	BinDlg* getControlFromIndex(BinDlg* a1, s16 a2)
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

	void setTextStr(BinDlg* a1)
	{
		const u32 Func_setTextStr = 0x004263E0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setTextStr
			POPAD
		}
	}

	void setUnitStatusStrText(BinDlg* a1)
	{
		const u32 Func_setUnitStatusStrText = 0x00425B50;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setUnitStatusStrText
			POPAD
		}
	}

	s32 unitGetName(Unit* a1)
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

	// For getUnitStrength

	s32 getCurrentHitPointsPlusShields(Unit* a1)
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

	s32 getBaseStrength(s32 a1, Unit* a2)
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

	s32 canWeaponTargetUnit(u8 a1, Unit* a2, Unit* a3)
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

	s32 AI_TargetUnitIsWorthHitting(Unit* a1, Unit* a2)
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

	s32 createSplashWeaponProc(s16 a1, s16 a2, s16 a3, s32(__fastcall* a4)(s32, s32), s32 a5)
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

	s32 unitShieldsClumpInAreaAccumulatorProc(Unit* a1, Unit* a2)
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

	void unitWasHit(Unit* a1, Unit* a2, s32 a3)
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

	void doAttackNotifyEvent(Unit* a1)
	{
		const u32 Func_doAttackNotifyEvent = 0x0048F230;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_doAttackNotifyEvent
			POPAD
		}
	}

	Image* createShieldOverlay(s32 a1, Unit* a2)
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

	bool getActiveTileFlag(s16 a1, s16 a2, s32 a3, s32 a4)
	{
		const u32 Func_getActiveTileFlag = 0x00472F80;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			PUSH a2
			MOVZX AX, a1
			CALL Func_getActiveTileFlag
			MOV result, EAX
			POPAD
		}
		return (bool)result;
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


	const u32 CreateHealthBar_function = 0x004D6420;
	Image* CreateHealthBar(s32 a1, Sprite* a2)
	{
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

	const u32 RemoveHPBar_function = 0x004D5030;
	void RemoveHPBar(Image* a1)
	{
		_asm {
			PUSHAD
			MOV ESI, a1
			CALL RemoveHPBar_function
			POPAD
		}
	}

	const u32 playImageIscript_function = 0x004D8470;
	void playImageIscript(Image* img, u8 iscriptAnim)
	{
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

	s16 SAI_GetRegionIdFromPxEx(s32 a1, s32 a2)
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

	vectorHead* vectorTransfer(char a1, char a2)
	{
		const u32 Func_vectorTransfer = 0x0045A010;
		static vectorHead* result;
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

	s32 randGetSFXBetween(s32 a1, s32 a2)
	{
		const u32 Func_randGetSFXBetween = 0x0048E940;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDI, a1
			MOV ESI, a2
			CALL Func_randGetSFXBetween
			MOV result, EAX
			POPAD
		}
		return result;
	}


	// for AI_TrainUnit

	char AI_TrainingWorker(Unit* result, Unit* parent)
	{
		const u32 Func_AI_TrainingWorker = 0x00435700;
		static s32 resultFunc;
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingWorker
			MOV resultFunc, EAX
			POPAD
		}
		return resultFunc;
	}

	void AI_TrainingOverlord(Unit* result, Unit* parent)
	{
		const u32 Func_AI_TrainingOverlord = 0x00435770;
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingOverlord
			POPAD
		}
	}

	void AI_TrainingNormal(Unit* result, Unit* parent)
	{
		const u32 Func_AI_TrainingNormal = 0x00435DB0;
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingNormal
			POPAD
		}
	}

	void AI_OrderToDestination(Unit* unit, s32 orderId, s32 x, s32 y)
	{
		const u32 Func_AI_OrderToDestination = 0x0043D5D0;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			PUSH orderId
			PUSH unit
			CALL Func_AI_OrderToDestination
			POPAD
		}
	}

	void AI_AssignCaptainToSlowestUnit(AiCaptain* region)
	{
		const u32 Func_AI_AssignCaptainToSlowestUnit = 0x00436F70;
		__asm {
			PUSHAD
			PUSH region
			CALL Func_AI_AssignCaptainToSlowestUnit
			POPAD
		}
	}

	// for AI_CreateNuke

	Unit* createUnit(s16 unitType, s32 x, s32 y, s32 playerId)
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

	void updateUnitStatsFinishBuilding(Unit* unit)
	{
		const u32 Func_updateUnitStatsFinishBuilding = 0x004A01F0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateUnitStatsFinishBuilding
			POPAD
		}
	}

	u32 updateUnitStrengthAndApplyDefaultOrders(Unit* unit)
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

	void AI_AssignMilitary(s32 regionType, AiCaptain* region)
	{
		const u32 Func_AI_AssignMilitary = 0x004390A0;
		__asm {
			PUSHAD
			MOV EBX, regionType
			MOV ESI, region
			CALL Func_AI_AssignMilitary
			POPAD
		}
	}

	void AI_AddMilitary(AiCaptain* region, Unit* unit, s32 always_this_region)
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

	// for AI_AddUnitAi

	UnitAi* AI_CreateBuildingAi(BuildingAiArray* buildingAI)
	{
		const u32 Func_AI_CreateBuildingAi = 0x004044C0;
		static UnitAi* result;
		__asm {
			PUSHAD
			MOV ESI, buildingAI
			CALL Func_AI_CreateBuildingAi
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void orderComputerClear(Unit* unit, u8 orderId)
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

	Unit* setSecondaryOrder(Unit* result, char orderId)
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

	void AI_MakeGuard(s32 playerId, Unit* unit)
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

	bool unitIsActiveResourceDepot(Unit* unit)
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

	s32 getPlayerDefaultRefineryUnitType(s32 playerId)
	{
		const u32 Func_getPlayerDefaultRefineryUnitType = 0x004320D0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, playerId
			CALL Func_getPlayerDefaultRefineryUnitType
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for AI_UnitWasHit
	// some of these should probably be methods in api/etc

	char AI_CancelStructure(Unit* structure)
	{
		const u32 Func_AI_CancelStructure = 0x00468280;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, structure
			CALL Func_AI_CancelStructure
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	Unit* AI_GetActualTarget(Unit* attacker, Unit* target)
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

	Unit* checkArbiterCloakingRange(Unit* unit, u16 id)
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

	void AI_OrderUnitCloaking(Unit* unit)
	{
		const u32 Func_AI_OrderUnitCloaking = 0x0043B970;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_AI_OrderUnitCloaking
			POPAD
		}
	}

	void AI_OrderLurkerBurrow(Unit* unit)
	{
		const u32 Func_AI_OrderLurkerBurrow = 0x0043C230;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_AI_OrderLurkerBurrow
			POPAD
		}
	}

	bool AI_IsUnderAttack(Unit* unit, s32 flag)
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

	s32 isUnitLoaded(Unit* unit)
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

	s32 AI_TryReturnHome(Unit* unit, s32 dontIssueOrder)
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

	s32 AI_TransportAction(Unit* unit)
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

	s32 AI_AskForHelp(Unit* self, Unit* attacker, s32 onlyNearbyGuards)
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

	s16 getRegionIdFromUnit(Unit* unit)
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

	s32 getEnemyAirStrengthInRegion(u16 regionId, s32 player)
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

	s32 getEnemyStrengthInRegion(u16 regionId, s32 player, s32 includeAir)
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

	bool unitIsPerformingUnbreakableCode(Unit* unit)
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

	bool AI_CastSpellBehavior(Unit* unit, bool isAggro)
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

	s32 AI_Flee(Unit* unit, Unit* enemy)
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

	bool unitHasGroundPath(Unit* unit, s16 x, s16 y)
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

	bool AI_HasNoDefenseBuild(s32 player)
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

	bool unitCanSeeCloakedTarget(Unit* target, Unit* unit)
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

	void AI_RespondToCloakedThreat(Unit* threat, Unit* unit)
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

	bool unitCanAttackTarget(Unit* target, Unit* unit, s32 checkDetection)
	{
		const u32 Func_unitCanAttackTarget = 0x00476730;
		static u32 result;
		__asm {
			PUSHAD
			PUSH checkDetection
			MOV ESI, unit
			MOV EBX, target
			CALL Func_unitCanAttackTarget
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	s32 AI_UpdateAttackTarget(Unit* unit, s32 acceptIfSieged, s32 acceptCritters, s32 mustReach)
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

	void orderInterrupt(Unit* unit, u8 orderId, Point32* pos, Unit* target)
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

	// for AI_IterateBuildingAI

	u16 AI_RemoveFromTowns(Unit* worker)
	{
		const u32 Func_AI_RemoveFromTowns = 0x00432760;
		static u32 result;
		__asm {
			PUSHAD
			MOV ECX, worker
			CALL Func_AI_RemoveFromTowns
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	BuildingAi* AI_DeleteBuildingAI(BuildingAi* building, BuildingAiArray* buildingList)
	{
		const u32 Func_AI_DeleteBuildingAI = 0x00404500;
		static BuildingAi* result;
		__asm {
			PUSHAD
			MOV EAX, building
			MOV EDX, buildingList
			CALL Func_AI_DeleteBuildingAI
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 AI_GetQueuedCount(Unit* builder, u32 unitId)
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

	void AI_PrepareGuardAI(u16 y, u32 player, u16 unitId, u16 x)
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

	s32 AI_AddRegionToMilitary(AiCaptain* region, s32 unitId, s32 priority)
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

	u32 AI_FindUnit(u32 unitId, u32 playerId, u32 type)
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

	u32 AI_AddSpendingRequest(u16 unitId, u32 type, u8 player, char priority, void* value)
	{
		const u32 Func_AI_AddSpendingRequest = 0x00447980;
		static u32 typeXD = type;
		static u32 result;
		__asm {
			PUSHAD
			PUSH value
			PUSH priority
			MOVZX CL, player
			MOV EDX, typeXD
			MOVZX AX, unitId
			CALL Func_AI_AddSpendingRequest
			MOV result, EAX
			POPAD
		}
		return result;
	}

	u32 AI_AddBaseUnitSpendingRequest(u32 unitId, u32 player, AiCaptain* region, char priority)
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

	u32 AI_RecalculateRegionStrength(AiCaptain* region)
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

	Unit* AI_TargetEnemyInRegion(AiCaptain* region)
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

	s32 AI_MoveMilitaryToDefend(AiCaptain* region)
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

	void AI_GetRegionMilitaryTypes(AiCaptain* region, s32* hasGroundMilitary,
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

	s32 AI_CanTravelToRegion(AiCaptain* region1, AiCaptain* region2)
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

	Unit* AI_PickAttackTarget_NoBuildings(AiCaptain* region)
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

	u32 AI_MoveMilitaryToOtherRegion(AiCaptain* target, AiCaptain* source, u32 pullUnitsNearRegion,
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

	AiTown* AI_IterateTownSomething(AiCaptain* region)
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

	// for AI_AttackManager

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

	bool function_00462EA0(Unit* unit, u8 unknownByteValue)
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

	void removeOrderFromUnitQueue(Unit* unit, Order* order)
	{
		const u32 Func_removeOrderFromUnitQueue = 0x004742D0;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EAX, order
			CALL Func_removeOrderFromUnitQueue
			POPAD
		}
	}

	void function_00474A70(Unit* unit, Unit* target, u8 orderId)
	{
		const u32 Func_Sub474A70 = 0x00474A70;
		__asm {
			PUSHAD
			MOV CL, orderId
			MOV EDX, unit
			MOV EAX, target
			CALL Func_Sub474A70
			POPAD
		}
	}

	bool isTargetWithinMinMovementRange(Unit* unit, Unit* target, u32 range)
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

	void function_00477510(Unit* unit)
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

	bool function_00478540(Unit* unit)
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

	// not related with assigning orders, but only with
	// destinations of orders
	void makeToHoldPosition(Unit* unit)
	{
		const u32 Func_OrdersHoldPositionSuicidal = 0x004EB5B0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_OrdersHoldPositionSuicidal
			POPAD
		}
	}

	bool moveToTarget(Unit* target, Unit* unit)
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

	bool SetMoveTarget_xy(Unit* unit, s32 x, s32 y)
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
	void setNextWaypoint(Unit* unit)
	{
		const u32 Func_setNextWaypoint = 0x004EB290;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_setNextWaypoint
			POPAD
		}
	}

	bool changeMovementTargetMoveTarget2(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_changeMovementTargetMoveTarget2 = 0x004EB9C0;
		static b32 bPreResult;
		__asm {
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

	bool isDistanceGreaterThanHaltDistance(Unit* unit, s32 x, s32 y, u32 distance)
	{
		const u32 Func_isDistanceGreaterThanHaltDistance = 0x00401240;
		static b32 bPreResult;
		__asm {
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

	void checkUnstack(Unit* unit)
	{
		const u32 Func_checkUnstack = 0x0042D9A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_checkUnstack
			POPAD
		}
	}

	bool beginBuilding(Unit* unit)
	{
		const u32 Func_beginBuilding = 0x00467030;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_beginBuilding
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	Unit* beginGasBuilding(Unit* unit, u32 unitId)
	{
		const u32 Func_beginGasBuilding = 0x004678A0;
		static Unit* newUnit;
		__asm {
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
	void addOverlayBelowMain(Sprite* sprite, u32 imageId, u32 unk1, u32 unk2, u32 unk3)
	{
		const u32 Func_addOverlayBelowMain = 0x00498D70;
		__asm {
			PUSHAD
			MOV EAX, sprite
			MOV ESI, imageId
			PUSH unk1
			PUSH unk2
			PUSH unk3
			CALL Func_addOverlayBelowMain
			POPAD
		}
	}

	void replaceSpriteImages(Sprite* sprite, u32 imageId, u32 imageDirection)
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

	void displayLastNetErrorForPlayer(u32 playerId)
	{
		const u32 Func_displayLastNetErrorForPlayer = 0x0049E530;
		__asm {
			PUSHAD
			PUSH playerId
			CALL Func_displayLastNetErrorForPlayer
			POPAD
		}
	}

	void updateUnitStrength(Unit* unit)
	{
		const u32 Func_UpdateUnitStrength = 0x0049FA40;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_UpdateUnitStrength
			POPAD
		}
	}

	void iscript_OpcodeCases(Image* image, u32 offset_, u32 unk1, u32 unk2)
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

	void switchToWarpTexture(Image* image)
	{
		const u32 Func_switchToWarpTexture = 0x004D8500;
		__asm {
			PUSHAD
			MOV EAX, image
			CALL Func_switchToWarpTexture
			POPAD
		}
	}

	bool setThingyVisibilityFlags(Thingy* thingy)
	{
		const u32 Func_SetThingyVisibilityFlags = 0x004878F0;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV ESI, thingy
			CALL Func_SetThingyVisibilityFlags
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	Thingy* createThingy(u32 spriteId, s16 x, s16 y, u32 playerId)
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

	void playUnitSFX(u32 soundId, u16 x, u16 y, u32 unk1, u32 unk2)
	{
		const u32 Func_playUnitSFX = 0x0048EC10;
		static Point16 pos;
		pos.x = x; pos.y = y;
		__asm {
			PUSHAD
			PUSH unk2
			PUSH unk1
			PUSH pos
			MOV EBX, soundId
			CALL Func_playUnitSFX
			POPAD
		}
	}

	u32 RandomizeShort(u32 unknown_index)
	{
		const u32 Func_RandomizeShort = 0x004DC4A0;
		static u32 return_value;
		__asm {
			PUSHAD
			MOV EAX, unknown_index
			CALL Func_RandomizeShort
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

	void setUnitStatTxtErrorMsg(char* message)
	{
		const u32 Func_setUnitStatTxtErrorMsg = 0x0048CCB0;
		__asm {
			PUSHAD
			MOV EAX, message
			CALL Func_setUnitStatTxtErrorMsg
			POPAD
		}
	}

	void function_004EBAE0(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_Sub4EBAE0 = 0x004EBAE0;
		__asm {
			PUSHAD
			MOV EDX, unit
			MOV EAX, x
			MOV ECX, y
			CALL Func_Sub4EBAE0
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

	void function_004E9A30(Unit* unit)
	{
		const u32 Func_Sub4E9A30 = 0x004E9A30;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4E9A30
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

	u32 getDistanceFast(s32 a1, s32 a2, s32 a3, s32 a4)
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

	void fixTargetLocation(unsigned __int16 a1, Point32* a2)
	{
		const u32 Func_fixTargetLocation = 0x00401FA0;
		__asm {
			PUSHAD
			MOVZX AX, a1
			MOV EDX, a2
			CALL Func_fixTargetLocation
			POPAD
		}
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

	void toIdle(Unit* a1)
	{
		const u32 Func_toIdle = 0x004753A0;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_toIdle
			POPAD
		}
	}

	s32 isUnitMovable(Unit* a1)
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

	void orders_Nothing2(Unit* a1)
	{
		const u32 Func_orders_Nothing2 = 0x00475000;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_orders_Nothing2
			POPAD
		}
	}

	bool setMoveTargetXY(s32 a1, s32 a2, Unit* a3)
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

	void toggleUnitPath(Unit* unit)
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
		if (scbw::isZergBuildingMorphResult(buildingId))
		{
			result = true;
		}
		else
		{
			result = false;
		}
		return result;
	}

	void updateNewUnitVision(u32 id, s32 x, s32 y)
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

	void function_0047DE40(Sprite* sprite, u32 unitId, s32 x, s32 y)
	{
		const u32 Func_Sub47DE40 = 0x0047DE40;
		__asm {
			PUSHAD
			MOV ECX, y
			MOV EDX, x
			MOV ESI, sprite
			PUSH unitId
			CALL Func_Sub47DE40
			POPAD
		}
	}

	//unk1 is ECX, unk2 is pushed value
	void incrementUnitScoresEx(Unit* building, s32 unk1, s32 unk2)
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

	const u32 Func_PlayBuildingCompleteSound = 0x0048F310;
	void playBuildingCompleteSound(Unit* building)
	{
		__asm {
			PUSHAD
			MOV EAX, building
			CALL Func_PlayBuildingCompleteSound
			POPAD
		}
	}

	const u32 Func_refreshSpriteData = 0x004983A0;
	void refreshSpriteData(Sprite* sprite)
	{
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_refreshSpriteData
			POPAD
		}
	}

	const u32 Func_ReplaceUnitWithType = 0x0049FED0;
	void replaceUnitWithType(Unit* unit, u16 newUnitId)
	{
		u32 newUnitId_ = newUnitId;
		__asm {
			PUSHAD
			PUSH newUnitId_
			MOV EAX, unit
			CALL Func_ReplaceUnitWithType
			POPAD
		}
	}

	const u32 Func_fixTargetLocation = 0x00401FA0;
	void fixTargetLocation(Point16* coords, u32 unitId)
	{
		__asm {
			PUSHAD
			MOV EAX, unitId
			MOV EDX, coords
			CALL Func_fixTargetLocation
			POPAD
		}
	}

	//Equivalent to function @ 0x00402160
	bool isUnitPositions2Equal(Unit* unit)
	{
		return (
			unit->nextTargetWaypoint.x == unit->position.x &&
			unit->nextTargetWaypoint.y == unit->position.y
			);
	}

	//if canBeAborted is TRUE, the unit can be aborted because there's
	//no room around the constructing building
	b32 buildingAddon(Unit* unit, u32 hpGain, b32 canBeAborted)
	{
		const u32 Func_buildingAddon = 0x004679A0;
		static b32 bResult;
		__asm {
			PUSHAD
			MOV EDX, hpGain
			MOV EAX, unit
			PUSH canBeAborted
			CALL Func_buildingAddon
			MOV bResult, EAX
			POPAD
		}
		return bResult;
	}

	void prependOrderTargetingNothing(Unit* unit, Order* order, u8 orderId)
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

	void addOrderTargetingNothing(Unit* unit, Order* order, u8 orderId)
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

	bool canStillPlaceBuilding()
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

	void endBuildingPlacement()
	{
		const u32 Func_endBuildingPlacement = 0x0048E310;
		__asm {
			PUSHAD
			CALL Func_endBuildingPlacement
			POPAD
		}
	}

	void function_00494BB0(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_Sub494BB0 = 0x00494BB0;
		__asm {
			PUSHAD
			MOV ECX, y
			MOV EDX, x
			MOV EAX, unit
			CALL Func_Sub494BB0
			POPAD
		}
	}

	u32 RandBetween(u32 min, u32 max, u32 someIndex)
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

	char refreshLayers3And4()
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

	void initBuilding(Unit* unit, b32 flag)
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

	bool orderToMoveToTarget(Unit* unit, Unit* target)
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

	b32 unk_terrainCollision2(Unit* unit, s32 x, s32 y)
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

	// For updatePlacementStateForPlacementBox

	s32 canBuildOnTileLocation(Unit* a1, s32 player, s32 x_tile, s32 y_tile, s32 unitType,
		s32 placement_state_entry, s32 check_vision, s32 also_invisible, s32 without_vision)
	{
		const u32 Func_canBuildOnTileLocation = 0x00473FB0;
		static s32 result;
		__asm {
			PUSHAD
			PUSH without_vision
			PUSH also_invisible
			PUSH check_vision
			PUSH placement_state_entry
			PUSH unitType
			PUSH y_tile
			PUSH x_tile
			PUSH player
			PUSH a1
			CALL Func_canBuildOnTileLocation
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For refreshScreen

	s32 updatePlacementStateForPlacementBox_Func()
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

	s32 unk_buildingPlacementRelated(s32 a1, Point16* a2)
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

	s32 initPlacementBoxPos(Point16* in_out)
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

	char unk_buildingPlacementRelatedMouse(s32 a1, s32 a2)
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

	void function_00463470(Unit* unit)
	{
		const u32 Func_Sub463470 = 0x00463470;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub463470
			POPAD
		}
	}

	void function_004634E0(Unit* unit)
	{
		const u32 Func_Sub4634E0 = 0x004634E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4634E0
			POPAD
		}
	}

	void function_00463640(Unit* unit)
	{
		const u32 Func_Sub463640 = 0x00463640;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub463640
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

	void function_00464300(Unit* unit)
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

	void function_00469EC0(Unit* unit, s32 x, s32 y)
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

	void function_00469F60(Unit* unit, s32 x, s32 y)
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

	void function_0046A560(Unit* unit)
	{
		const u32 Func_Sub46A560 = 0x0046A560;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub46A560
			POPAD
		}
	}

	void function_0046A5A0(Unit* unit)
	{
		const u32 Func_Sub46A5A0 = 0x0046A5A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub46A5A0
			POPAD
		}
	}

	u32 function_00473FB0(Unit* unit, u8 playerId, s32 x, s32 y, u16 unitId, u8 unk1, u8 unk2, u8 unk3, u8 unk4)
	{
		const u32 Func_Sub473FB0 = 0x00473FB0;
		u32 return_value;
		__asm {
			PUSHAD
			MOVZX EAX, unk4		/*28*/
			PUSH EAX
			MOVZX EAX, unk3		/*24*/
			PUSH EAX
			MOVZX EAX, unk2		/*20*/
			PUSH EAX
			MOVZX EAX, unk1		/*1C*/
			PUSH EAX
			MOVZX EAX, unitId	/*18*/
			PUSH EAX
			PUSH y				/*14*/
			PUSH x				/*10*/
			MOVZX EAX, playerId	/*0C*/
			PUSH EAX
			PUSH unit			/*08*/
			CALL Func_Sub473FB0
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	void actUnitReturnToIdle(Unit* unit)
	{
		const u32 Func_ActUnitReturnToIdle = 0x00475420;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_ActUnitReturnToIdle
			POPAD
		}
	}

	bool function_0047DF90(Unit* unit)
	{
		const u32 Func_Sub47DF90 = 0x0047DF90;
		b32 pre_return_value;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_Sub47DF90
			MOV pre_return_value, EAX
			POPAD		}

		return (pre_return_value != 0);
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

	void function_00498150(Sprite* sprite, u8 newVerticalOffset)
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

	void function_004997E0(Sprite* sprite)
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

	b32 spreadsCreep(u32 unitId, b32 defaultReturn)
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

	void function_004148F0(s32 x, s32 y, u32 unitId)
	{
		const u32 Func_Sub4148F0 = 0x004148F0;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			PUSH unitId
			CALL Func_Sub4148F0
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

	b32 function_0049EC30(Unit* unit)
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

	bool function_004E8E10(Unit* unit, Point32* coords)
	{
		const u32 Func_Sub4E8E10 = 0x004E8E10;
		static b32 return_pre_value;
		__asm {
			PUSHAD
			MOV EDI, coords
			MOV EAX, unit
			CALL Func_Sub4E8E10
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
	void function_004754F0(Unit* unit, u32 unitId, u32 unk1, u32 unk2, u32 orderId, u32 unk3, u32 unk4, u32 unk5)
	{
		const u32 Func_Sub_4754F0 = 0x004754F0;
		__asm {
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
			CALL Func_Sub_4754F0
			POPAD
		}
	}

	void clearUnitSprites(Sprite* unit_sprite, Sprite* subunit_sprite)
	{
		const u32 Func_clearUnitSprites = 0x00496F00;
		__asm {
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
		__asm {
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
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func__UnitDestructor
			POPAD
		}
	}

	void UnitDestructor(Unit* unit)
	{
		const u32 Func_UnitDestructor = 0x004A0990;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_UnitDestructor
			POPAD
		}
	}

	void hideAndDisableUnit(Unit* unit)
	{
		const u32 Func_unitDeathSomething_0 = 0x004E6340;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitDeathSomething_0
			POPAD
		}
	}

	void CancelResearch_Proc(Unit* unit)
	{
		const u32 Func_Sub453E30 = 0x00453E30;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_Sub453E30
			POPAD
		}
	}

	void CancelUpgrade_Proc(Unit* unit)
	{
		const u32 Func_Sub454280 = 0x00454280;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub454280
			POPAD
		}
	}

	void refundQueueSlot(Unit* unit, u16 slot)
	{
		const u32 Func_refundQueueSlot = 0x00466A70;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOVZX EAX, slot
			CALL Func_refundQueueSlot
			POPAD
		}
	}

	void function_00466E40(Unit* unit)
	{
		const u32 Func_Sub466E40 = 0x00466E40;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub466E40
			POPAD
		}
	}

	void CancelAddon_Proc(Unit* unit)
	{
		const u32 Func_Sub4E66E0 = 0x004E66E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4E66E0
			POPAD
		}
	}

	b32 OrderAllowed(Unit* unit, u16 order, u32 nationID)
	{
		const u32 Func_OrderAllowed = 0x0046DC20;
		static b32 bResult;
		__asm {
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

	bool placeBuildingMsg(u32 result_function_473FB0)
	{
		const u32 Func_placeBuildingMsg = 0x0048D930;
		static b32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, result_function_473FB0
			CALL Func_placeBuildingMsg
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool CMDRECV_PlaceBuildingAllowedHelper(Unit* unit, u8 orderId, u16 builtUnitId)
	{
		const u32 Func_CMDRECV_PlaceBuildingAllowed = 0x0048DBD0;
		static b32 bPreResult;
		__asm {
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
		__asm {
			PUSHAD
			PUSH builtUnitId
			PUSH dim
			PUSH orderId
			MOV EAX, unit
			CALL Func_Sub48DE70
			POPAD
		}
	}

	void cmdRECV_PlaceBuilding(s16 x, s16 y, u8 orderId, u16 builtUnitId)
	{
		const u32 Func_cmdRECV_PlaceBuilding = 0x0048E190;
		static Point16 pos;
		pos.x = x; pos.y = y;
		__asm {
			PUSHAD
			MOVZX EAX, builtUnitId	// bugfix: should have been "MOV AX, builtUnitId" but for some reason it crashed
			MOV CL, orderId
			PUSH pos
			CALL Func_cmdRECV_PlaceBuilding
			POPAD
		}
	}

	Unit* templarMergePartner(Unit* unit, Unit** units_list, u32 units_list_length)
	{
		const u32 Func_templarMergePartner = 0x004BFA80;
		static Unit* return_value;
		__asm {
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

	void parseOrdersDatReqs()
	{
		const u32 Func_parseOrdersDatReqs = 0x0046D450;
		__asm {
			PUSHAD
			CALL Func_parseOrdersDatReqs
			POPAD
		}
	}

	s32 parseRequirementOpcodes(Unit* unit, u32 orderId, u32 playerId, u32 address, u32 req_offset)
	{
		const u32 Func_parseRequirementOpcodes = 0x0046D610;
		static s32 return_value;
		__asm {
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

	void ApplySpeedUpgradeFromUnitType(Unit* unit)
	{
		const u32 Func_ApplySpeedUpgradeFromUnitType = 0x00454370;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_ApplySpeedUpgradeFromUnitType
			POPAD
		}
	}

	void function_00463040(Unit* unit)
	{
		const u32 Func_Sub463040 = 0x00463040;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub463040
			POPAD
		}
	}

	void function_00476D30(Unit* unit)
	{
		const u32 Func_Sub476D30 = 0x00476D30;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub476D30
			POPAD
		}
	}

	void incrementUnitScores(Unit* unit, s32 unkScore)
	{
		const u32 Func_incrementUnitScores = 0x00488BF0;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unkScore
			CALL Func_incrementUnitScores
			POPAD
		}
	}

	void function_004A1E50(Unit* unit, u32 unk)
	{
		const u32 Func_Sub4A1E50 = 0x004A1E50;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EDX, unk
			CALL Func_Sub4A1E50
			POPAD
		}
	}

	void CreateUnitSelection(Unit* unit, u32 selectionIndex)
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

	void DestroyUnitSelection(Unit* unit)
	{
		const u32 Func_DestroyUnitSelection = 0x004E6290;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_DestroyUnitSelection
			POPAD
		}
	}

	void function_004F3B70(Unit* powerup)
	{
		const u32 Func_Sub4F3B70 = 0x004F3B70;
		__asm {
			PUSHAD
			PUSH powerup
			CALL Func_Sub4F3B70
			POPAD
		}
	}

	u32 getUpgradedWpnCooldown(Unit* unit, u8 weaponId)
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

	bool function_00476640(Unit* unit, u32 weaponId)
	{
		const u32 Func_Sub476640 = 0x00476640;
		static b32 pre_result;
		__asm {
			PUSHAD
			PUSH weaponId
			MOV EAX, unit
			CALL Func_Sub476640
			MOV pre_result, EAX
			POPAD
		}
		return (pre_result != 0);
	}

	u32 get_statTxt_Str_0(Unit* unit, u32 playerId, char* message)
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

	bool canCastSpell_0(Unit* unit)
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
		//0 means "can cast spell"
		return (bPreResult == 0);
	}

	u32 function_00492620(Unit* unit, b32 wasMainOrderStateZero)
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

	void refundAllQueueSlots(Unit* unit)
	{
		const u32 Func_RefundAllQueueSlots = 0x00466E80;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RefundAllQueueSlots
			POPAD
		}
	}

	void incrementUnitDeathScores(Unit* unit, u8 player)
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

	void initializeEmptyUnitsLinkedListRef_Sub49E4E0(Unit* unit, u8 playerId)
	{
		const u32 Func_Sub_49E4E0 = 0x0049E4E0;
		__asm {
			PUSHAD
			MOV DL, playerId
			MOV ECX, unit
			CALL Func_Sub_49E4E0
			POPAD
		}
	}

	const u32 Func_Sub_49EFA0 = 0x0049EFA0;
	void GiveUnitHelper(Unit* unit, u32 playerId, s32 unkScore)
	{
		__asm {
			PUSHAD
			PUSH unkScore
			MOV ECX, unit
			PUSH playerId
			CALL Func_Sub_49EFA0
			POPAD
		}
	}

	void changeUnitButtonSet_Sub_4E5D60(Unit* unit, u16 buttonSetId)
	{
		const u32 Func_Sub_4E5D60 = 0x004E5D60;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV CX, buttonSetId
			CALL Func_Sub_4E5D60
			POPAD
		}
	}

	// What make the target reappear
	void showAndEnableUnit(Unit* unit)
	{
		const u32 Func_InitUnitTrapDoodad = 0x004E6490;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_InitUnitTrapDoodad
			POPAD
		}
	}

	bool isInfestableUnit(Unit* unit)
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

	// For updateUnitStrengthAndApplyDefaultOrders

	void AnimCloak(Unit* a1, s32 a2)
	{
		const u32 Func_playSoundFromDirection = 0x0049B5B0;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_playSoundFromDirection
			POPAD
		}
	}

	void refreshUnitVision(Unit* a1)
	{
		const u32 Func_refreshUnitVision = 0x004E5F30;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_refreshUnitVision
			POPAD
		}
	}

	void initUnitTrapDoodad(Unit* a1)
	{
		const u32 Func_initUnitTrapDoodad = 0x004E6490;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_initUnitTrapDoodad
			POPAD
		}
	}

	Order* removeOrderFromUnitQueue(Order* a1, Unit* a2)
	{
		const u32 Func_removeOrderFromUnitQueue = 0x004742D0;
		static Order* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_removeOrderFromUnitQueue
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Order* performAnotherOrder(u8 a1, s32 a2, Unit* a3, s16 a4, Point32* a5, Unit* a6)
	{
		const u32 Func_performAnotherOrder = 0x004745F0;
		static Order* result;
		__asm {
			PUSHAD
			MOVZX BL, a1
			MOV EDI, a2
			MOV ESI, a3
			MOVZX DX, a4
			PUSH a5
			PUSH a6
			CALL Func_performAnotherOrder
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void order(s32 a1, Unit* a2, s8 a3, Point32* a4, Unit* a5, s32 a6)
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

	u32 getUnitStrength(Unit* a1, s32 a2, bool a3)
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

	void playWorkerErrorSound(Unit* unit, u32 errorTextIndex)
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

	bool isUnitUpgradeAvailable(Unit* unit)
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

	void ApplySpeedUpgradeFromUpgradeType(Unit* unit, u8 upgradeType)
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

	void playUpgradeCompleteSound()
	{
		const u32 Func_playUpgradeCompleteSound = 0x0048F070;
		__asm {
			PUSHAD
			CALL Func_playUpgradeCompleteSound
			POPAD
		}
	}

	void playResearchCompleteSound()
	{
		const u32 Func_playResearchCompleteSound = 0x0048F150;
		__asm {
			PUSHAD
			CALL Func_playResearchCompleteSound
			POPAD
		}
	}

	void setUpgradeLevel(u32 playerId, u32 upgradeType, u8 upgradeLevel)
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
	bool advanceRemainingBuildTime(Unit* unit)
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
	u8 function_0046A820(Unit* unit)
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

	void orderImmediate(Unit* unit, u8 order)
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

	void playMorphingCompleteSound(Unit* unit)
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
	void mergeStatusIntoUnit_Sub493180(Unit* unit_dst, Unit* unit_src)
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

	void function_0049B440(Unit* unit, u32 soundId)
	{
		const u32 Func_Sub49B440 = 0x0049B440;
		__asm {
			PUSHAD
			MOV EBX, soundId
			MOV EAX, unit
			CALL Func_Sub49B440
			POPAD
		}
	}

	void changeUnitButtonSet_Sub4E5D60(Unit* unit, u16 buttonSetId)
	{
		const u32 Func_Sub4E5D60 = 0x004E5D60;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV CX, buttonSetId
			CALL Func_Sub4E5D60
			POPAD
		}
	}

	// return true if waiting for recentOrderTimer to reach 0 or if setMoveTarget_xy returned true
	bool delayedSetMoveTarget_xy_Sub4EB960(Unit* unit, u16 x, u16 y)
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

	void setAllOverlayDirectionsGeneric(Unit* unit, u32 unk)
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

	void AI_orderUnitCloaking(Unit* unit)
	{
		const u32 Func_AI_orderUnitCloaking = 0x0043B970;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_AI_orderUnitCloaking
			POPAD
		}
	}

	u32 function_0043DB50(Unit* unit)
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

	s32 getUnitMaxWeaponRange(Unit* a1, unsigned __int8 a2)
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

	bool isTargetWithinMinMovementRange(Unit* a1, u32 a2, Unit* a3)
	{
		const u32 Func_isTargetWithinMinMovementRange = 0x004763D0;
		static s32 result;
		static bool result2;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			MOV EDI, a3
			CALL Func_isTargetWithinMinMovementRange
			MOV result, EAX
			POPAD
		}
		return result2 = (bool)result;
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

	bool isTargetWithinMinRange(Unit* a1, u32 a2, Unit* a3)
	{
		const u32 Func_isTargetWithinMinRange = 0x00430F10;
		static s32 result;
		static bool result2;

		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			PUSH a1
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

	void prependOrderTargetingGround(__int8 a1, __int16 a2, __int16 a3, s32 a4)
	{
		const u32 Func_prependOrderTargetingGround = 0x00474A20;
		__asm {
			PUSHAD
			PUSH a4
			MOVZX CX, a3
			MOVZX DX, a2
			MOVZX AL, a1
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
			MOV EAX, unit
			MOV ECX, target
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

	Unit* setSecondaryWaypoint(Unit* a1, s32 a2, s32 a3)
	{
		const u32 Func_setSecondaryWaypoint = 0x00494BB0;
		static Unit* result;
		__asm {
			PUSHAD
			MOV ECX, a3
			MOV EDX, a2
			MOV EAX, a1
			CALL Func_setSecondaryWaypoint
			MOV result, EAX
			POPAD
		}
		return result;
	}

	Point32 setResourceTarget(Unit* a1)
	{
		const u32 Func_setResourceTarget = 0x004EB290;
		static __int32 result;
		static Point32 resultPoint;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_setResourceTarget
			MOV result, EAX
			POPAD
		}
		return resultPoint = *(Point32*)&result;
	}

	void ordersNothing2(Unit* a1)
	{
		const u32 Func_ordersNothing2 = 0x00475000;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_ordersNothing2
			POPAD
		}
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

	// For checkUnitVisibility

	u32 getDetectingPlayers(Unit* unit)
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
						UpgradesSc->currentLevel[unit->playerId][UpgradeId::VentralSacs] == 0
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

	// For getUnitRankString

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

	char* getStatTxtString(__int16 a1)
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

	// For statsWeaponLevel / statsArmorLevel

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

	void showDialog(BinDlg* dialog)
	{
		const u32 Func_showDialog = 0x004186A0;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_showDialog
			POPAD
		}
	}

	s32 disableDialog(BinDlg* dialog)
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

	void updateDialog(BinDlg* dialog)
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

	// creates attack overlay & deals damage from the bullet (just calls overlayAndNotify if the weapon deals no damage, or calls target->damageWith if it does
	void weaponBulletShot(Unit* target, Bullet* bullet, u32 dmgDivisor)
	{
		const u32 Helper_WeaponBulletShot = 0x00479AE0;
		if (bullet != nullptr && target != nullptr && bullet->sourceUnit && bullet->weaponId == WeaponId::PariahIconoclastBlades)
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

	u8 setArchonPathing(Unit* unit)
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

	bool hasSuppliesForUnit(u32 unitId, u32 playerId, b32 canShowErrorMessage)
	{
		const u32 Func_hasSuppliesForUnit = 0x0042CF70;
		static b32 bPreResult;
		__asm {
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

	Unit* zergDualBirth(Unit* unit)
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

	void orderNewUnitToRally(Unit* unit, Unit* factory)
	{
		const u32 Func_Sub466F50 = 0x00466F50;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, factory
			CALL Func_Sub466F50
			POPAD
		}
	}

	void AI_TrainingUnit(Unit* unit_creator, Unit* created_unit)
	{
		const u32 Func_AI_TrainingUnit = 0x004A2830;
		__asm {
			PUSHAD
			MOV EAX, created_unit
			MOV ECX, unit_creator
			CALL Func_AI_TrainingUnit
			POPAD
		}
	}

	// for orders_Hallucination1

	bool ordersSpell_Sub_4926D0(Unit* unit, u32 techId, u16* techEnergyCost, u32 sightRange, u32 error_message_index)
	{
		const u32 Func_Sub_4926D0 = 0x004926D0;
		static b32 bPreResult;
		__asm {
			PUSHAD
			PUSH techEnergyCost
			PUSH sightRange
			PUSH techId
			MOV EAX, error_message_index
			MOV EBX, unit
			CALL Func_Sub_4926D0
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool placeHallucination(Unit* unit)
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

	void applyHallucinationStats(Unit* halUnit)
	{
		const u32 Func_applyHallucinationStats = 0x004F6180;
		__asm {
			PUSHAD
			MOV EAX, halUnit
			CALL Func_applyHallucinationStats
			POPAD
		}
	}

	s32 hallucinateScarabs(Unit* a1, Unit* a2)
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

	void replaceFeedbackSprite(Unit* unit, u32 spriteId)
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

	void Ai_UnitWasHit(u32 main_target_reaction, u32 dont_call_help, Unit* attacker, Unit* target)
	{
		const u32 Func_0043F320 = 0x0043F320;
		__asm {
			PUSHAD
			PUSH main_target_reaction
			PUSH dont_call_help
			MOV ECX, target
			MOV EDX, attacker
			CALL Func_0043F320
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
	u16 getUnitStrength(Unit* unit, bool useGroundStrength)
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
	u8 hasOverlay(Unit* unit)
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
		if (
			unit->id == UnitId::ProtossSteward ||
			unit->id == UnitId::ProtossPlasmaShell
			)
			return true;
		else
			return false;
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

	void function_00476FC0(Unit* unit, Unit* target, u32 unk1, u32 unk2)
	{
		const u32 Func_Sub476FC0 = 0x00476FC0;
		__asm {
			PUSHAD
			PUSH unk1
			PUSH unk2
			MOV EAX, target
			MOV EDI, unit
			CALL Func_Sub476FC0
			POPAD
		}
	}

	b32 function_004770E0(Unit* unit)
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

	b32 AI_CastSpellBehaviour(Unit* unit, u32 unknown)
	{
		const u32 Func_AI_CastSpellBehaviour = 0x004A13C0;
		static b32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unknown
			CALL Func_AI_CastSpellBehaviour
			MOV result, EAX
			POPAD
		}
		return result;
	}

	b32 function_00438E70(Unit* unit, s32 x, s32 y)
	{
		const u32 Func_sub_438E70 = 0x00438E70;
		static b32 result;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			MOV EAX, unit
			CALL Func_sub_438E70
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

	void unknownSpecialOrder(Unit* unit, Order* order, u8 orderId, s16 x, s16 y, Unit* target, u16 targetUnitId)
	{
		const u32 Func_unknownSpecialOrder = 0x00474540;
		static Point16 pos;
		pos.x = x; pos.y = y;
		__asm {
			PUSHAD
			PUSH target
			PUSH pos
			MOV BL, orderId
			MOVZX EDX, targetUnitId
			MOV ESI, unit
			MOV EDI, order
			CALL Func_unknownSpecialOrder
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

	b32 function_00477160(Unit* unit, u32 unk1, u32 unk2, u32 unk3)
	{
		const u32 Func_Sub477160 = 0x00477160;
		__asm {
			PUSHAD
			PUSH unk1
			PUSH unk2
			PUSH unk3
			MOV ECX, unit
			CALL Func_Sub477160
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

	b32 function_004E6EF0(Unit* unit, Unit** unitTargetOffset)
	{
		const u32 Func_Sub4E6EF0 = 0x004E6EF0;
		b32 result;
		__asm {
			PUSHAD
			PUSH unitTargetOffset
			MOV EBX, unit
			CALL Func_Sub4E6EF0
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

	//return true when unit reached nextTargetWaypoint either at call
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

	const u32 Func_OrderReturnToIdle = 0x00463770;
	void orderReturnToIdle(Unit* unit)
	{
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_OrderReturnToIdle
			POPAD
		}
	}

	const u32 Func_Sub464290_CheckForCloseUnit = 0x00464290;
	bool checkForCloseUnit_sub464290(Unit* unit)
	{
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

	//Identical to getHPGainForRepair @ 0x00402C40
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

	void refundUnitTrainCost(u32 unitId, u8 playerId)
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

	Bullet* createBullet(u8 weaponId, Unit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction)
	{
		const u32 Helper_CreateBullet = 0x0048C260;
		static u32 attackingPlayer_;
		static u32 direction_;
		static s32 x_;
		static s32 y_;
		attackingPlayer_ = attackingPlayer;
		direction_ = direction;
		x_ = x; y_ = y;

		/// Paladin attacking behavior
		if (weaponId == WeaponId::PaladinCrusaderMissiles)
		{
			if (source->sprite->mainGraphic->direction == 0
				|| source->sprite->mainGraphic->direction == 1
				|| source->sprite->mainGraphic->direction == 2
				|| source->sprite->mainGraphic->direction == 3)
			{
				x_ = source->sprite->position.x + 6;
				y_ = source->sprite->position.y - 45;
			}
			else if (source->sprite->mainGraphic->direction == 4
				|| source->sprite->mainGraphic->direction == 5)
			{
				x_ = source->sprite->position.x + 4;
				y_ = source->sprite->position.y - 44;
			}
			else if (source->sprite->mainGraphic->direction == 6
				|| source->sprite->mainGraphic->direction == 7
				|| source->sprite->mainGraphic->direction == 8
				|| source->sprite->mainGraphic->direction == 9)
			{
				x_ = source->sprite->position.x + 1;
				y_ = source->sprite->position.y - 43;
			}
			else if (source->sprite->mainGraphic->direction == 10
				|| source->sprite->mainGraphic->direction == 11)
			{
				x_ = source->sprite->position.x - 3;
				y_ = source->sprite->position.y - 42;
			}
			else if (source->sprite->mainGraphic->direction == 12
				|| source->sprite->mainGraphic->direction == 13)
			{
				x_ = source->sprite->position.x - 6;
				y_ = source->sprite->position.y - 41;
			}
			else if (source->sprite->mainGraphic->direction == 14
				|| source->sprite->mainGraphic->direction == 15)
			{
				x_ = source->sprite->position.x - 8;
				y_ = source->sprite->position.y - 44;
			}
			else if (source->sprite->mainGraphic->direction == 16)
			{
				x_ = source->sprite->position.x - 8;
				y_ = source->sprite->position.y - 48;
			}
		}
		static Bullet* bullet;
		__asm {
			PUSHAD
			PUSH direction_
			PUSH attackingPlayer_
			PUSH y_
			PUSH x_
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
	}

	bool unitIsReaver(Unit* a1)
	{
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

	// For findNextBounceTarget

	s32 iterateUnitsAtLocationTargetProc(Box32* a1, s32(__fastcall* a2)(s32, s32), Unit* a3)
	{
		const u32 Func_iterateUnitsAtLocationTargetProc = 0x004E8280;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EBX, a2
			MOV EAX, a1
			CALL Func_iterateUnitsAtLocationTargetProc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	//for progressBullet

	void doMissileDmg(Bullet* bullet)
	{
		const u32 func_doMissileDmg = 0x0048B770;
		__asm {
			PUSHAD
			PUSH bullet
			CALL func_doMissileDmg
			POPAD
		}
	}

	bool function_0042D810(s32 x, s32 y, u32 unitId)
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

	Unit** getAllUnitsInBounds(Box16* coords)
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

	u32 getUnitDamageBonus(Unit* unit, u8 weaponId)
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

	//identical to incrementUnitKillCount @ 0x004759C0
	void incrementUnitKillCount(Unit* unit)
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

	void attackOverlayAndNotify(Unit* attacker, Unit* target, u32 weaponType, u32 direction)
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

	u32 GetUnitBulletDamage(Bullet* bullet, Unit* target)
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

	u32 getSpellStatString(Unit* target, u32 techId, u8 playerId)
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
		for (
			Image* current_image = sprite->images.head;
			current_image != nullptr;
			current_image = current_image->link.next
			)
		{
			if (current_image->paletteType == PaletteType::RLE_HPFLOATDRAW)
			{
				current_image->flags |= CImage_Flags::Redraw;
			}
		}
	}

	const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
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

	s32 playSoundFromSource(s32 soundId, Unit* unit, s32 bPlay, s32 minVolume)
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
		if (*IS_IN_REPLAY != 0 || unit->playerId != *LOCAL_NATION_ID)
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
		if (unit->id == UnitId::UnusedCaveIn)
		{
			return false;
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

	b32 selectSingleUnitFromID(u32 unitIndex)
	{
		const u32 Func_SelectSingleUnitFromID = 0x00496D30;
		b32 bReturnedValue;
		__asm {
			PUSHAD
			PUSH unitIndex
			CALL Func_SelectSingleUnitFromID
			MOV bReturnedValue, EAX
			POPAD
		}
		return bReturnedValue;
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
			scbw::refreshConsole();
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

	// For updateUnits

	bool isSelectedUnitGroupEnabled()
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

	void cancelTargetOrder()
	{
		const u32 Func_cancelTargetOrder = 0x0048CA10;
		__asm {
			PUSHAD
			CALL Func_cancelTargetOrder
			POPAD
		}
	}

	s32 updateDisruptionWebStatus()
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

	Unit* progressDyingUnitFrame(Unit* unit)
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

	void revealSightAtLocation(s32 sightRange, u32 visionFlags, s32 ptX, s32 ptY, bool isAirUnit)
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

	void updateVisibility(Unit* unit)
	{
		const u32 Func_updateVisibility = 0x004EBE10;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_updateVisibility
			POPAD
		}
	}

	void progressActiveUnitFrame(Unit* unit)
	{
		const u32 Func_progressActiveUnitFrame = 0x004EBC30;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_progressActiveUnitFrame
			POPAD
		}
	}

	void checkUnitVisibility(Unit* unit)
	{
		const u32 Func_checkUnitVisibility = 0x00443390;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_checkUnitVisibility
			POPAD
		}
	}

	void progressUnitFrame(Unit* unit)
	{
		const u32 Func_progressUnitFrame = 0x004ECF70;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_progressUnitFrame
			POPAD
		}
	}

	void unitUpdate(Unit* unit)
	{
		const u32 Func_unitUpdate = 0x004EC420;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_unitUpdate
			POPAD
		}
	}

	void updateUnitPower()
	{
		const u32 Func_updateUnitPower = 0x00493790;
		__asm {
			PUSHAD
			CALL Func_updateUnitPower
			POPAD
		}
	}

	// For updateBurrowingCloakingUnits

	s32 isUnitCloaking(Unit* unit)
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

	void playSoundFromDirect(Unit* unit, s32 soundId)
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

	// For doCloak

	void updateSpriteFlagOnPalette(Sprite* sprite)
	{
		const u32 Func_updateSpriteFlagOnPalette = 0x00497E80;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_updateSpriteFlagOnPalette
			POPAD
		}
	}

	void showImagesHiddenByCloak(Sprite* sprite)
	{
		const u32 Func_showImagesHiddenByCloak = 0x00498170;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_showImagesHiddenByCloak
			POPAD
		}
	}

	void updateSpriteImagesForCloak(Sprite* sprite)
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

	Order* queueNewOrder(Order* a1, s16 a2, s16 a3, Unit* a4, u8 a5, s32 a6, s16 a7, Point32* a8, Unit* a9)
	{
		const u32 Func_queueNewOrder = 0x00474B90;
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
			CALL Func_queueNewOrder
			MOV result, EAX
			POPAD
		}
		return result;
	}

	s32 showStatTxtToPlayer(s32 a1, s32 a2)
	{
		const u32 Func_showStatTxtToPlayer = 0x0048CF00;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_showStatTxtToPlayer
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void setOrderTarget(u8 a1, s16 a2, s16 a3, s32 a4, s32 a5, Unit* a6, s32 a7, Point32* a8, Unit* a9)
	{
		const u32 Func_setOrderTarget = 0x00475470;
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
			CALL Func_setOrderTarget
			POPAD
		}
	}

	// For selectUnits

	s32 selectSingleUnitFromID(s32 a1)
	{
		const u32 Func_selectSingleUnitFromID = 0x00496D30;
		static s32 result;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_selectSingleUnitFromID
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

	bool unitIsTrainingOrMorphing(Unit* unit)
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

	void updateUnits()
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
		if (!unit) return false;
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

	void QueueGameCommand(u8* params, u32 param_length)
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



	const u32 Func_CreateThingy = 0x00488210;
	Thingy* createThingyFunc(u32 spriteId, s16 x, s16 y, u32 playerId)
	{
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

	const u32 Func_DoUnitsCollide = 0x00469B60;
	bool func_dounitscollide(Unit* first, Unit* second)
	{
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

	const u32 Func_Sub_4E97C0 = 0x004E97C0;
	void unburrow(Unit* unit)
	{
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub_4E97C0
			POPAD
		}
	}

	const u32 Func_SMemAlloc = 0x0041006A;
	void* SMemAlloc(u32 amount, u32 unkFilename, u32 unkLine, u32 defaultValue)
	{

		static void* pointer_on_allocated_memory;

		__asm {
			PUSHAD
			PUSH defaultValue		//probably what to fill the memory with
			PUSH unkLine			//unknown use, seems to use hardcoded value
			PUSH unkFilename		//unknown use, seems to use hardcoded adress (of some text?)
			PUSH amount				//amount of memory to allocate
			CALL Func_SMemAlloc
			MOV pointer_on_allocated_memory, EAX
			POPAD
		}

		return pointer_on_allocated_memory;

	}

	;

	//Likely to call SMemAlloc!
	const u32 Func_SetCallbackTimer = 0x00416090;
	void SetCallbackTimer(BinDlg* dialog, u32 unk1, u32 unk2, u32 unk3)
	{

		__asm {
			PUSHAD
			PUSH unk3
			PUSH unk2
			MOV EAX, unk1
			MOV ECX, dialog
			CALL Func_SetCallbackTimer
			POPAD
		}

	}

	;

	const u32 Func_Sub49A7F0 = 0x0049A7F0;
	void function_0049A7F0(Unit* unit)
	{

		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub49A7F0
			POPAD
		}

	}

	u32 fxnInteract(BinDlg* dialog, u32 data_struct_offset)
	{

		static u32 return_value;
		static u32 fxnInteract;

		fxnInteract = (u32)dialog->fxnInteract;

		__asm {
			PUSHAD
			MOV ESI, dialog					//possibly useless, but just in case
			MOV ECX, dialog
			MOV EDX, data_struct_offset
			CALL fxnInteract
			MOV return_value, EAX
			POPAD
		}

		return return_value;

	}


	/**** Definitions of helper functions. Do NOT modify anything below! ****/

	const u32 Func_registerUserDialogAction = 0x00418100;
	void registerUserDialogAction(BinDlg* dialog, u32 array_of_fxnInteractFunc, u32 sizeOfArray)
	{

		__asm {
			PUSHAD
			MOV EAX, dialog
			MOV EDI, array_of_fxnInteractFunc
			MOV ECX, sizeOfArray
			CALL Func_registerUserDialogAction
			POPAD
		}

	}

	;

	const u32 Func_Sub418E00 = 0x00418E00;
	void function_418E00(BinDlg* dialog)
	{

		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_Sub418E00
			POPAD
		}

	}

	;

	const u32 Func_ReplayStatBtns = 0x00427A80;
	void replayStatBtns(BinDlg* dialog)
	{
		__asm {
			PUSHAD
			MOV EBX, dialog
			CALL Func_ReplayStatBtns
			POPAD
		}
	}

	;

	const u32 Func_Sub_4591D0 = 0x004591D0;
	void function_4591D0()
	{

		__asm {
			PUSHAD
			CALL Func_Sub_4591D0
			POPAD
		}

	}

	;

	const u32 Func_Sub_459770 = 0x00459770;
	void function_459770()
	{
		__asm {
			PUSHAD
			CALL Func_Sub_459770
			POPAD
		}
	}

	;

	const u32 Func_BINDLG_BlitSurface = 0x004C35F0;
	void BINDLG_BlitSurface(BinDlg* dialog)
	{
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_BINDLG_BlitSurface
			POPAD
		}
	}

	;

	//dev info: examples of calls: 429520,428e60,428420,4287D0,4292C0
	s32 req_check(u32 reqFunc, u16 reqVar, u32 playerId, Unit* unit)
	{

		static s32 return_value;

		__asm {
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
			anim = IscriptAnimation::WorkingToIdle;
		else
			if (mineralPatch->building.resource.resourceAmount >= 500)
				anim = IscriptAnimation::AlmostBuilt;
			else
				if (mineralPatch->building.resource.resourceAmount >= 250)
					anim = IscriptAnimation::SpecialState2;
				else
					if (mineralPatch->building.resource.resourceAmount >= 50)
						anim = IscriptAnimation::SpecialState1;
					else
						anim = IscriptAnimation::Landing;
		if (anim != mineralPatch->building.resource.resourceIscript)
		{
			mineralPatch->building.resource.resourceIscript = anim;
			mineralPatch->sprite->playIscriptAnim(anim);
		}
	}

	void updateImagePositionOffset(Image* image)
	{

		const u32 Func_updateImagePositionOffset = 0x004D5A00;

		__asm {
			PUSHAD
			MOV ECX, image
			CALL Func_updateImagePositionOffset
			POPAD
		}

	}

	//Identical to function @ 0x004F3AF0
	void setResourceAmountCarried(Unit* worker, u8 amountCarried, u32 chunkImageId, bool isMineral)
	{

		if (worker->resourceType != 0)
			return;

		worker->resourceType = isMineral ? 2 : 1;

		Image* chunkImage = worker->sprite->createOverlay(chunkImageId);

		if (chunkImage && !(chunkImage->flags & CImage_Flags::Flag07))
		{
			chunkImage->flags |= CImage_Flags::Flag07;
			updateImagePositionOffset(chunkImage);
		}

		worker->worker.resourceCarryAmount = amountCarried;
		scbw::refreshConsole();

	}

	const u32 Func_0x00433410 = 0x00433410;
	s32 Ai_HasBuildingsNeedingPower(u32 player)
	{
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL Func_0x00433410
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_0x00433470 = 0x00433470;
	s32 Ai_CountEconomyUnits(u32 player, u32 unit_id)
	{
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			PUSH unit_id
			CALL Func_0x00433470
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_0x004889D0 = 0x004889D0;
	s32 getSuppliesRemaining(u8 player, u16 unit_id)
	{
		static s32 result;
		__asm {
			PUSHAD
			MOV BL, player
			MOV DI, unit_id
			CALL Func_0x004889D0
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_4888c0 = 0x004888C0;
	s32 getSuppliesUsed(s32 byte_data, u8 player)
	{
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, byte_data
			MOV CL, player
			CALL Func_4888c0
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_AI_AddCostToNeeds = 0x00447DC0;
	s32 AI_AddCostToNeeds(s32 a1, s32 a2, s32 a3)
	{
		static s32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EBX, a2
			MOV EAX, a1
			CALL Func_AI_AddCostToNeeds
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_AI_AddSpendingRequest = 0x00447980;
	s32 AI_AddSpendingRequest(s16 a1, s32 a2, char a3, char a4, void* a5)
	{
		static s32 result;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			MOVZX CL, a3
			MOV EDX, a2
			MOVZX AX, a1
			CALL Func_AI_AddSpendingRequest
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For AI_AddSpendingRequest
	const u32 Func_isAIAllowedToBuild = 0x00447830;
	bool isAIAllowedToBuild(s32 a1, s32 a2)
	{
		static s32 result;
		__asm {
			PUSHAD
			MOV ECX, a2
			MOV EAX, a1
			CALL Func_isAIAllowedToBuild
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void removeAITownGasReferences(Unit* unit)
	{
		const u32 Func_RemoveGasRefs = 0x00432760;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_RemoveGasRefs
			POPAD
		}
	}

	void deleteBuildingAi(void* building_ai_list, BuildingAi* ai)
	{
		const u32 Func_DeleteBuildingAi = 0x00404500;
		__asm {
			PUSHAD
			MOV EDX, building_ai_list
			MOV EAX, ai
			CALL Func_DeleteBuildingAi
			POPAD
		}
	}

	void deleteWorkerAi(void* worker_ai_list, WorkerAi* ai)
	{
		const u32 Func_DeleteWorkerAi = 0x00404470;
		__asm {
			PUSHAD
			MOV EDX, worker_ai_list
			MOV EAX, ai
			CALL Func_DeleteWorkerAi
			POPAD
		}
	}

	void deleteAiTownIfNeeded(AiTown* town)
	{
		const u32 Func_DeleteAiTownIfNeeded = 0x004347E0;
		__asm {
			PUSHAD
			MOV EAX, town
			CALL Func_DeleteAiTownIfNeeded
			POPAD
		}
	}

	// For isOrderValid
	bool unitIsResource(Unit* unit)
	{
		const u32 Func_unitIsResource = 0x004688E0;
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitIsResource
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void returnAllInterceptors(Unit* unit)
	{
		const u32 Func_returnAllInterceptors = 0x004655F0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_returnAllInterceptors
			POPAD
		}
	}

	char ignoreAllScriptsAndGoToIdle(Unit* unit)
	{
		const u32 Func_ignoreAllScriptsAndGoToIdle = 0x00476550;
		static s32 result;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_ignoreAllScriptsAndGoToIdle
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void orders_SelfDestructing(Unit* unit)
	{
		const u32 Func_orders_SelfDestructing = 0x00475710;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_orders_SelfDestructing
			POPAD
		}
	}

	char setMaelstromTimer(Unit* unit, u8 time)
	{
		const u32 Func_setMaelstromTimer = 0x004553F0;
		static s32 result;
		__asm {
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
		__asm {
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
		__asm {
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
		__asm {
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
		__asm {
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
		__asm {
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
		__asm {
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

	// For AI_orderUnitCloaking

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

	// For drawBuildingInfo
	void drawSupplyUnitInfo(BinDlg* dialog) {
		const u32 Func_drawSupplyUnitInfo = 0x00427540;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL Func_drawSupplyUnitInfo
			POPAD
		}
	}

	// For getUnitName
	const char* getUnitNameStatString(u16 unitId) {
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

	// For updateBuildingPlacementState_Units
	bool doesBuildingBlock(Unit* unit, s32 xTile, s32 yTile) {
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

	s32 isTileBlockedBy(Unit** array, Unit* builder, s32 xTile, s32 yTile, char bDontIgnoreReacting, char bAlsoInvisible) {
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
	s32 iterateLoadedUnits(Unit* unit, s32 (__fastcall* func)(s32, s32), s32 a3)
	{
		const u32 Func_iterateLoadedUnits = 0x004E6D00;
		static s32 result;
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
}