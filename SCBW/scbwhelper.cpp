#include "scbwhelper.h"
#include "api.h"
#include "globals.h"
#include <cassert>

namespace helper {

	bool HasMoneyCanMake(CUnit* builder, uint32 unitToBuild) {
		const uint32 Func_HasMoneyCanMake = 0x00467250;
		static bool32 bPreResult;
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

	void getRaceSoundList(int8 a1) {
		const uint32 Func_getRaceSoundList = 0x0048FB40;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_getRaceSoundList
			POPAD
		}
	}

	int32 dialogGrpConstruct(int32 a1, int32 a2, int32 a3, int32(__fastcall* a4)(int32, int32, int32, int32)) {
		const uint32 Func_dialogGrpConstruct = 0x004197B0;
		static int32 result;
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

	void loadGraphic(int8 a1, int32 a2, int32 a3, int8 a4, int32 a5) {
		const uint32 Func_loadGraphic = 0x004D2B30;
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

	int randFunc() {
		const uint32 Func_randFunc = 0x0040C7BD;
		static int result;
		__asm {
			PUSHAD
			CALL Func_randFunc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unitHPbelow33_percent(CUnit* a1) {
		const uint32 Func_unitHPBelow33Percent = 0x004022C0;
		static int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitHPBelow33Percent
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unitHPbetween33_66_percent(int32 a1, CUnit* a2) {
		const uint32 Func_unitHPBetween33_66Percent = 0x00402270;
		static int32 result;

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

	int32 getMaxHP(CUnit* a1) {
		const uint32 Func_getMaxHP = 0x00401400;
		static int32 result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_getMaxHP
			MOV result, EAX
			POPAD
		}
		return result;
	}

	uint16 getLastQueueSlotType(CUnit* a1) {
		const uint32 Func_getLastQueueSlotType = 0x0047B270;
		static int result;
		__asm {
			PUSHAD
			MOV EDX, a1
			CALL Func_getLastQueueSlotType
			MOV result, EAX
			POPAD
		}
		return (uint16)result;
	}

	void addTextToDialog(int32 a1, int16 a2, const char* a3) {
		const uint32 Func_addTextToDialog = 0x004258B0;
		static int32 _a3 = (int)a3;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CX, a2
			PUSH _a3
			CALL Func_addTextToDialog
			POPAD
		}
	}

	void drawEnergy(int32 a1) {
		const uint32 Func_drawEnergy = 0x00425A30;
		__asm {
			PUSHAD
			MOV EBX, a1
			CALL Func_drawEnergy
			POPAD
		}
	}

	void hideOptionalStatusScreenControls(BinDlg* a1) {
		const uint32 Func_hideOptionalStatusScreenControls = 0x00457310;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_hideOptionalStatusScreenControls
			POPAD
		}
	}

	void showControlGroup(BinDlg* a1, int16 a2, int16 a3) {
		const uint32 Func_showControlGroup = 0x00457250;
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX CX, a2
			CALL Func_showControlGroup
			POPAD
		}
	}

	void addTextToDialog(int a1, int16 a2, int a3) {
		const uint32 Func_addTextToDialog = 0x004258B0;
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX CX, a2
			CALL Func_addTextToDialog
			POPAD
		}
	}

	void drawShieldUpgrade(int a1, uint16 a2) {
		const uint32 Func_drawShieldUpgrade = 0x00425510;
		__asm {
			PUSHAD
			PUSH a2
			MOV EAX, a1
			CALL Func_drawShieldUpgrade
			POPAD
		}
	}

	void hideDialog(BinDlg* a1) {
		const uint32 Func_hideDialog = 0x00418700;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_hideDialog
			POPAD
		}
	}

	// For drawSupplyUnitInfo

	int getSuppliesUsed() {
		const uint32 Func_getSuppliesUsed = 0x00424732;
		static int result;
		__asm {
			PUSHAD
			CALL Func_getSuppliesUsed
			MOV result, EAX
			POPAD
		}
		return result;
	}

	uint32 getSuppliesAvailable(uint8 a1, int a2) {
		const uint32 Func_getSuppliesAvailable = 0x00488900;
		static uint32 result;
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

	void drawArmor(int a1, int a2, uint16 a3) {
		const uint32 Func_drawArmor = 0x00425600;
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOV EDX, a2
			CALL Func_drawArmor
			POPAD
		}
	}

	void unitStatAct_Standard(void* a1) {
		const uint32 Func_unitStatAct_Standard = 0x00426F50;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitStatAct_Standard
			POPAD
		}
	}

	// For AI_TargetEnemyProc

	bool AI_UnitCanAttack(CUnit* a1) {
		const uint32 Func_AI_UnitCanAttack = 0x00476180;
		static int result;
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

	int needRedrawUI() {
		const uint32 Func_needRedrawUI = 0x00424A10;
		static int result;
		__asm {
			PUSHAD
			CALL Func_needRedrawUI
			MOV result, EAX
			POPAD
		}
		return result;
	}

	CUnit* genericStatusUpdateDrawnValues() {
		const uint32 Func_genericStatusUpdateDrawnValues = 0x004248F0;
		static CUnit* result;
		__asm {
			PUSHAD
			CALL Func_genericStatusUpdateDrawnValues
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void drawProgressBar(int a1) {
		const uint32 Func_drawProgressBar = 0x004260C0;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawProgressBar
			POPAD
		}
	}

	void drawNukeSiloInfo(int a1) {
		const uint32 Func_drawNukeSiloInfo = 0x00426FF0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawNukeSiloInfo
			POPAD
		}
	}

	// Equivalent to code unitIsActiveTransport @ 0x004E6BA0
	// Modified to remove Ovileth checks as they are no longer tied to upgrades in HYDRA
	bool unitIsActiveTransport(CUnit* unit) {
		bool result = false;
		if (!(unit->status & UnitStatus::IsHallucination)) {
			result = true;
		}
		return result;
	}

	bool isQueueSlotActive(CUnit* a1, int a2) {
		const uint32 Func_isQueueSlotActive = 0x00401E70;
		static int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL Func_isQueueSlotActive
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void drawTrainingProgress(BinDlg* a1) {
		const uint32 Func_drawTrainingProgress = 0x004268D0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawTrainingProgress
			POPAD
		}
	}

	bool isConstructingAddon(const CUnit* a1) {
		const uint32 Func_isConstructingAddon = 0x004E66B0;
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isConstructingAddon
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool isAttemptingProtossBuild(CUnit* a1) {
		const uint32 Func_isAttemptingProtossBuild = 0x004E4C40;
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isAttemptingProtossBuild
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void drawAddonProgress(int a1) {
		const uint32 Func_drawAddonProgress = 0x00425F30;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawAddonProgress
			POPAD
		}
	}

	void drawTechProgress(int a1) {
		const uint32 Func_drawTechProgress = 0x004266F0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawTechProgress
			POPAD
		}
	}

	void drawUpgradeProgress(BinDlg* a1) {
		const uint32 Func_drawUpgradeProgress = 0x00426500;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawUpgradeProgress
			POPAD
		}
	}

	bool unitIsGeyser(CUnit* a1) {
		const uint32 Func_unitIsGeyser = 0x004688B0;
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsGeyser
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void drawGasQuantity(BinDlg* a1) {
		const uint32 Func_drawGasQuantity = 0x00426190;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_drawGasQuantity
			POPAD
		}
	}

	int8 unitGetGroundWeapon(CUnit* a1) {
		const uint32 Func_unitGetGroundWeapon = 0x00475AD0;
		static int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitGetGroundWeapon
			MOV result, EAX
			POPAD
		}
		return (int8)result;
	}

	void drawDisplayButtons(int a1) {
		const uint32 Func_drawDisplayButtons = 0x00426C60;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawDisplayButtons
			POPAD
		}
	}

	BinDlg* getControlFromIndex(BinDlg* a1, int16 a2) {
		const uint32 Func_getControlFromIndex = 0x00418080;
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

	void setTextStr(int a1) {
		const uint32 Func_setTextStr = 0x004263E0;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setTextStr
			POPAD
		}
	}

	void setUnitStatusStrText(int a1) {
		const uint32 Func_setUnitStatusStrText = 0x00425B50;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setUnitStatusStrText
			POPAD
		}
	}

	int unitGetName(CUnit* a1) {
		const uint32 Func_unitGetName = 0x0047B5A0;
		static int result;
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

	int getCurrentHitPointsPlusShields(CUnit* a1) {
		const uint32 Func_getCurrentHitPointsPlusShields = 0x004026D0;
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_getCurrentHitPointsPlusShields
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int getBaseStrength(int a1, CUnit* a2) {
		const uint32 Func_getBaseStrength = 0x004316D0;
		static int result;
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

	int canWeaponTargetUnit(uint8 a1, CUnit* a2, CUnit* a3) {
		const uint32 Func_canWeaponTargetUnit = 0x00475CE0;
		static int result;
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

	int32 AI_TargetUnitIsWorthHitting(CUnit* a1, CUnit* a2) {
		const uint32 Func_AI_TargetUnitIsWorthHitting = 0x00440E30;
		static int32 result;
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

	int createSplashWeaponProc(int16 a1, int16 a2, int16 a3, int(__fastcall* a4)(int, int), int a5) {
		const uint32 Func_createSplashWeaponProc = 0x004E82E0;
		static int result;
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

	int unitShieldsClumpInAreaAccumulatorProc(CUnit* a1, CUnit* a2) {
		const uint32 Func_unitShieldsClumpInAreaAccumulatorProc = 0x00440A60;
		static int result;
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

	void unitWasHit(CUnit* a1, CUnit* a2, int a3) {
		const uint32 Func_unitWasHit = 0x004795D0;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EBX, a1
			CALL Func_unitWasHit
			POPAD
		}
	}

	void doAttackNotifyEvent(CUnit* a1) {
		const uint32 Func_doAttackNotifyEvent = 0x0048F230;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_doAttackNotifyEvent
			POPAD
		}
	}

	CImage* createShieldOverlay(int a1, CUnit* a2) {
		const uint32 Func_createShieldOverlay = 0x004E6140;
		static CImage* result;
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

	int unk_billions(int a1, int32 a2, int a3, int a4) {
		const uint32 Func_unk_billions = 0x0047A750;
		static int result;
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

	int unk_billions2(int a1, int32 a2, int a3, int a4) {
		const uint32 Func_unk_billions2 = 0x0047A6E0;
		static int result;
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

	int hpBarSurfaceBuffer(int a1, int a2, int* a3, int a4, int a5) {
		const uint32 Func_hpBarSurfaceBuffer = 0x0047A350;
		static int result;
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

	int hpBarSurfaceBuffer2(int a1, int a2, uint16 a3, int a4) {
		const uint32 Func_hpBarSurfaceBuffer2 = 0x0047A120;
		static int result;
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

	int renderHPBar(int a1, int a2, int* a3, int a4, int a5) {
		const uint32 Func_renderHPBar = 0x0047A4E0;
		static int result;
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

	int32 getUnitMaxEnergy(CUnit* a1) {
		const uint32 Func_getUnitMaxEnergy = 0x00491870;
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_getUnitMaxEnergy
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int getHpBarDrawSizes(int a1, int32 a2, int a3, int a4) {
		const uint32 Func_getHpBarDrawSizes = 0x0047A670;
		static int result;
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

	bool unitIsFrozen(CUnit* a1) {
		const uint32 Func_unitIsFrozen = 0x004020B0;
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsFrozen
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool isUnitCasterNotHallucination(CUnit* a1) {
		const uint32 Func_isUnitCasterNotHallucination = 0x00401210;
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isUnitCasterNotHallucination
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool getActiveTileFlag(int16 a1, int16 a2, int a3, int a4) {
		const uint32 Func_getActiveTileFlag = 0x00472F80;
		static int result;
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

	CUnit* getTargetUnitAbilityErrorStatString(CUnit* a1, char a2, int a3) {
		const uint32 Func_getTargetUnitAbilityErrorStatString = 0x00491E80;
		static CUnit* result;
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

	int SErrSetLastError(char a1) {
		const uint32 Func_SErrSetLastError = 0x00410172;
		static int result;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_SErrSetLastError
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void updateImageDrawData(CImage* a1) {
		const uint32 Func_updateImageDrawData = 0x004D57B0;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_updateImageDrawData
			POPAD
		}
	}


	const uint32 CreateHealthBar_function = 0x004D6420;
	CImage* CreateHealthBar(int a1, CSprite* a2) {
		static CImage* result;
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

	const uint32 RemoveHPBar_function = 0x004D5030;
	void RemoveHPBar(CImage* a1) {
		_asm {
			PUSHAD
			MOV ESI, a1
			CALL RemoveHPBar_function
			POPAD
		}
	}

	const uint32 playImageIscript_function = 0x004D8470;
	void playImageIscript(CImage* img, uint8 iscriptAnim) {
		_asm {
			PUSHAD
			PUSH iscriptAnim
			MOV ECX, img
			CALL playImageIscript_function
			POPAD
		}
	}

	bool canEnterTransport(CUnit* a1, CUnit* a2) {
		const uint32 Func_canEnterTransport = 0x004E6E00;
		static int result;
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

	int16 SAI_GetRegionIdFromPxEx(int32 a1, int32 a2) {
		const uint32 Func_SAI_GetRegionIdFromPxEx = 0x0049C9F0;
		static int32 result;
		__asm {
			PUSHAD
			MOV EDI, a1
			MOV ECX, a2
			CALL Func_SAI_GetRegionIdFromPxEx
			MOV result, EAX
			POPAD
		}
		return (int16)result;
	}

	int32 getUnitDistanceToHalt(int a1, CUnit* a2, int a3) {
		const uint32 Func_getUnitDistanceToHalt = 0x00402140;
		static int32 result;
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

	int32 dataTransfer00(int a1, char a2, int a3) {
		const uint32 Func_dataTransfer00 = 0x00472A00;
		static int32 result;
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

	int dataTransfer01(uint32 a1, int a2, uint32 a3) {
		const uint32 Func_dataTransfer01 = 0x0045A600;
		static int result;
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

	int32 dataTransfer02(int a1, int a2, char a3) {
		const uint32 Func_dataTransfer02 = 0x0045A230;
		static int32 result;
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

	int32 dataTransfer03(int a1, char a2, int a3) {
		const uint32 Func_dataTransfer03 = 0x0045A1F0;
		static int32 result;
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

	int dataTransfer04(uint32 a1, int a2, int a3) {
		const uint32 Func_dataTransfer04 = 0x0045A390;
		static int result;
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

	int32 dataTransfer05(int a1, int a2) {
		const uint32 Func_dataTransfer05 = 0x0045A190;
		static int32 result;
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

	vectorHead* vectorTransfer(char a1, char a2) {
		const uint32 Func_vectorTransfer = 0x0045A010;
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

	int dataTransfer06(int a1) {
		const uint32 Func_dataTransfer06 = 0x00472900;
		static int result;
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

	int randGetSFXBetween(int a1, int a2) {
		const uint32 Func_randGetSFXBetween = 0x0048E940;
		static int result;
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

	char AI_TrainingWorker(CUnit* result, CUnit* parent) {
		const uint32 Func_AI_TrainingWorker = 0x00435700;
		static int32 resultFunc;
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

	void AI_TrainingOverlord(CUnit* result, CUnit* parent) {
		const uint32 Func_AI_TrainingOverlord = 0x00435770;
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingOverlord
			POPAD
		}
	}

	void AI_TrainingNormal(CUnit* result, CUnit* parent) {
		const uint32 Func_AI_TrainingNormal = 0x00435DB0;
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingNormal
			POPAD
		}
	}

	void AI_OrderToDestination(CUnit* unit, int orderId, int x, int y) {
		const uint32 Func_AI_OrderToDestination = 0x0043D5D0;
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

	void AI_AssignCaptainToSlowestUnit(AiCaptain* region) {
		const uint32 Func_AI_AssignCaptainToSlowestUnit = 0x00436F70;
		__asm {
			PUSHAD
			PUSH region
			CALL Func_AI_AssignCaptainToSlowestUnit
			POPAD
		}
	}

	// for AI_CreateNuke

	CUnit* createUnit(int16 unitType, int32 x, int32 y, int32 playerId) {
		const uint32 Func_createUnit = 0x004A09D0;
		static CUnit* result;
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

	void updateUnitStatsFinishBuilding(CUnit* unit) {
		const uint32 Func_updateUnitStatsFinishBuilding = 0x004A01F0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateUnitStatsFinishBuilding
			POPAD
		}
	}

	uint32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit) {
		const uint32 Func_updateUnitStrengthAndApplyDefaultOrders = 0x0049FA40;
		static uint32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateUnitStrengthAndApplyDefaultOrders
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void hideUnit(CUnit* unit) {
		const uint32 Func_hideUnit = 0x004E6340;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_hideUnit
			POPAD
		}
	}

	// for AI_TrainingBroodling

	void AI_AssignMilitary(int32 regionType, AiCaptain* region) {
		const uint32 Func_AI_AssignMilitary = 0x004390A0;
		__asm {
			PUSHAD
			MOV EBX, regionType
			MOV ESI, region
			CALL Func_AI_AssignMilitary
			POPAD
		}
	}

	void AI_AddMilitary(AiCaptain* region, CUnit* unit, int32 always_this_region) {
		const uint32 Func_AI_AddMilitary = 0x0043DA20;
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

	UnitAi* AI_CreateBuildingAi(BuildingAiArray* buildingAI) {
		const uint32 Func_AI_CreateBuildingAi = 0x004044C0;
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

	void orderComputerClear(CUnit* unit, uint8 orderId) {
		const uint32 Func_orderComputerClear = 0x00475310;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOVZX CL, orderId
			CALL Func_orderComputerClear
			POPAD
		}
	}

	CUnit* setSecondaryOrder(CUnit* result, char orderId) {
		const uint32 Func_setSecondaryOrder = 0x004743D0;
		static CUnit* result2;
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

	void AI_MakeGuard(int32 playerId, CUnit* unit) {
		const uint32 Func_AI_MakeGuard = 0x0043A010;
		__asm {
			PUSHAD
			PUSH unit
			PUSH playerId
			CALL Func_AI_MakeGuard
			POPAD
		}
	}

	bool unitIsActiveResourceDepot(CUnit* unit) {
		const uint32 Func_unitIsActiveResourceDepot = 0x00401CF0;
		static int32 result;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_unitIsActiveResourceDepot
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	int32 getPlayerDefaultRefineryUnitType(int32 playerId) {
		const uint32 Func_getPlayerDefaultRefineryUnitType = 0x004320D0;
		static int32 result;
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

	char AI_CancelStructure(CUnit* structure) {
		const uint32 Func_AI_CancelStructure = 0x00468280;
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, structure
			CALL Func_AI_CancelStructure
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	CUnit* AI_GetActualTarget(CUnit* attacker, CUnit* target) {
		const uint32 Func_AI_GetActualTarget = 0x0043AD60;
		static CUnit* result;
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

	CUnit* checkArbiterCloakingRange(CUnit* unit, uint16 id) {
		const uint32 Func_checkArbiterCloakingRange = 0x004410C0;
		static CUnit* result;
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

	void AI_OrderUnitCloaking(CUnit* unit) {
		const uint32 Func_AI_OrderUnitCloaking = 0x0043B970;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_AI_OrderUnitCloaking
			POPAD
		}
	}

	void AI_OrderLurkerBurrow(CUnit* unit) {
		const uint32 Func_AI_OrderLurkerBurrow = 0x0043C230;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_AI_OrderLurkerBurrow
			POPAD
		}
	}

	bool AI_IsUnderAttack(CUnit* unit, int32 flag) {
		const uint32 Func_AI_IsUnderAttack = 0x00436E70;
		static uint32 result;
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

	int32 isUnitLoaded(CUnit* unit) {
		const uint32 Func_isUnitLoaded = 0x004E7110;
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_isUnitLoaded
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int32 AI_TryReturnHome(CUnit* unit, int32 dontIssueOrder) {
		const uint32 Func_AI_TryReturnHome = 0x00462EA0;
		static int32 result;
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

	int32 AI_TransportAction(CUnit* unit) {
		const uint32 Func_AI_TransportAction = 0x0043DB50;
		static int32 result;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_AI_TransportAction
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int32 AI_AskForHelp(CUnit* self, CUnit* attacker, int32 onlyNearbyGuards) {
		const uint32 Func_AI_AskForHelp = 0x0043ADA0;
		static int32 result;
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

	int16 getRegionIdFromUnit(CUnit* unit) {
		const uint32 Func_getRegionIdFromUnit = 0x0049CB40;
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_getRegionIdFromUnit
			MOV result, EAX
			POPAD
		}
		return (int16)result;
	}

	int32 getEnemyAirStrengthInRegion(uint16 regionId, int32 player) {
		const uint32 Func_getEnemyAirStrengthInRegion = 0x00431D00;
		static int32 result;
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

	int32 getEnemyStrengthInRegion(uint16 regionId, int32 player, int32 includeAir) {
		const uint32 Func_getEnemyStrengthInRegion = 0x004318E0;
		static int32 result;
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

	bool unitIsPerformingUnbreakableCode(CUnit* unit) {
		const uint32 Func_unitIsPerformingUnbreakableCode = 0x00402310;
		static uint32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitIsPerformingUnbreakableCode
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool AI_CastSpellBehavior(CUnit* unit, bool isAggro) {
		const uint32 Func_AI_CastSpellBehavior = 0x004A13C0;
		static uint32 result;
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

	int32 AI_Flee(CUnit* unit, CUnit* enemy) {
		const uint32 Func_AI_Flee = 0x0043E400;
		static int32 result;
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

	bool unitHasGroundPath(CUnit* unit, int16 x, int16 y) {
		const uint32 Func_unitHasGroundPath = 0x0042FA00;
		static uint32 result;
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

	bool AI_HasNoDefenseBuild(int32 player) {
		const uint32 Func_AI_HasNoDefenseBuild = 0x00447370;
		static uint32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL Func_AI_HasNoDefenseBuild
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unitCanSeeCloakedTarget(CUnit* target, CUnit* unit) {
		const uint32 Func_unitCanSeeCloakedTarget = 0x00401D60;
		static uint32 result;
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

	void AI_RespondToCloakedThreat(CUnit* threat, CUnit* unit) {
		const uint32 Func_AI_RespondToCloakedThreat = 0x0043C580;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, threat
			CALL Func_AI_RespondToCloakedThreat
			POPAD
		}
	}

	bool unitCanAttackTarget(CUnit* target, CUnit* unit, int32 checkDetection) {
		const uint32 Func_unitCanAttackTarget = 0x00476730;
		static uint32 result;
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

	int32 AI_UpdateAttackTarget(CUnit* unit, int32 acceptIfSieged, int32 acceptCritters, int32 mustReach) {
		const uint32 Func_AI_UpdateAttackTarget = 0x00477160;
		static int32 result;
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

	void orderInterrupt(CUnit* unit, uint8 orderId, Point32* pos, CUnit* target) {
		const uint32 Func_orderInterrupt = 0x00474C30;
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

	uint16 AI_RemoveFromTowns(CUnit* worker) {
		const uint32 Func_AI_RemoveFromTowns = 0x00432760;
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, worker
			CALL Func_AI_RemoveFromTowns
			MOV result, EAX
			POPAD
		}
		return (uint16)result;
	}

	BuildingAi* AI_DeleteBuildingAI(BuildingAi* building, BuildingAiArray* buildingList) {
		const uint32 Func_AI_DeleteBuildingAI = 0x00404500;
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

	int32 AI_GetQueuedCount(CUnit* builder, uint32 unitId) {
		const uint32 Func_AI_GetQueuedCount = 0x00466B70;
		static int32 result;
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

	void AI_PrepareGuardAI(uint16 y, uint32 player, uint16 unitId, uint16 x) {
		const uint32 Func_AI_PrepareGuardAI = 0x00462670;
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

	int32 AI_AddRegionToMilitary(AiCaptain* region, int32 unitId, int32 priority) {
		const uint32 Func_AI_AddRegionToMilitary = 0x0043E2E0;
		static int32 result;
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

	uint32 AI_FindUnit(uint32 unitId, uint32 playerId, uint32 type) {
		const uint32 Func_AI_FindUnit = 0x00447DC0;
		static uint32 typeXD = type;
		static uint32 result;
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

	uint32 AI_AddSpendingRequest(uint16 unitId, uint32 type, uint8 player, char priority, void* value) {
		const uint32 Func_AI_AddSpendingRequest = 0x00447980;
		static uint32 typeXD = type;
		static uint32 result;
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

	uint32 AI_AddBaseUnitSpendingRequest(uint32 unitId, uint32 player, AiCaptain* region, char priority) {
		const uint32 Func_AI_AddBaseUnitSpendingRequest = 0x004484A0;
		static uint32 result;
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

	uint32 AI_RecalculateRegionStrength(AiCaptain* region) {
		const uint32 Func_AI_RecalculateRegionStrength = 0x0043A390;
		static uint32 result;
		__asm {
			PUSHAD
			MOV ESI, region
			CALL Func_AI_RecalculateRegionStrength
			MOV result, EAX
			POPAD
		}
		return result;
	}

	CUnit* AI_TargetEnemyInRegion(AiCaptain* region) {
		const uint32 Func_AI_TargetEnemyInRegion = 0x0043CC40;
		static CUnit* result;
		__asm {
			PUSHAD
			MOV EDI, region
			CALL Func_AI_TargetEnemyInRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int32 AI_MoveMilitaryToDefend(AiCaptain* region) {
		const uint32 Func_AI_MoveMilitaryToDefend = 0x0043DE40;
		static int32 result;
		__asm {
			PUSHAD
			MOV EDI, region
			CALL Func_AI_MoveMilitaryToDefend
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void AI_GetRegionMilitaryTypes(AiCaptain* region, int32* hasGroundMilitary,
		int32* cantAttackGround, int32* cantAttackAir) {
		const uint32 Func_AI_GetRegionMilitaryTypes = 0x00438FC0;
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

	int32 AI_CanTravelToRegion(AiCaptain* region1, AiCaptain* region2) {
		const uint32 Func_AI_CanTravelToRegion = 0x0043A790;
		static int32 result;
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

	CUnit* AI_PickAttackTarget_NoBuildings(AiCaptain* region) {
		const uint32 Func_AI_PickAttackTarget_NoBuildings = 0x0043A510;
		static CUnit* result;
		__asm {
			PUSHAD
			PUSH region
			CALL Func_AI_PickAttackTarget_NoBuildings
			MOV result, EAX
			POPAD
		}
		return result;
	}

	uint32 AI_MoveMilitaryToOtherRegion(AiCaptain* target, AiCaptain* source, uint32 pullUnitsNearRegion,
		uint32 sendPureSpellcasters, uint32 dontSendFromBunkers) {
		const uint32 Func_AI_MoveMilitaryToOtherRegion = 0x0043E580;
		static uint32 result;
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

	AiTown* AI_IterateTownSomething(AiCaptain* region) {
		const uint32 Func_AI_IterateTownSomething = 0x0043DD20;
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

	uint32 AI_AttackTimerDecrement(uint32 player) {
		const uint32 Func_AI_AttackTimerDecrement = 0x00447090;
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, player
			CALL Func_AI_AttackTimerDecrement
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int32 AI_HasForcesMovingToAttack(uint32 player) {
		const uint32 Func_AI_HasForcesMovingToAttack = 0x0043A2E0;
		static int32 result;
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

	uint16 PopulateRegionsWithOwn(char* regionData, uint32 player) {
		const uint32 Func_PopulateRegionsWithOwn = 0x00438B30;
		static uint32 result;
		__asm {
			PUSHAD
			PUSH player
			MOV EBX, regionData
			CALL Func_PopulateRegionsWithOwn
			MOV result, EAX
			POPAD
		}
		return (uint16)result;
	}

	void PopulateRegionsWithEnemy(uint32 player, char* regionData) {
		const uint32 Func_PopulateRegionsWithEnemy = 0x00439B50;
		__asm {
			PUSHAD
			PUSH regionData
			PUSH player
			CALL Func_PopulateRegionsWithEnemy
			POPAD
		}
	}

	int32 AI_GetNeighborRegionTypes(char* regionData) {
		const uint32 Func_AI_GetNeighborRegionTypes = 0x00436D80;
		static int32 result;
		__asm {
			PUSHAD
			MOV EBX, regionData
			CALL Func_AI_GetNeighborRegionTypes
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void AI_FinishRegionAccessibility(char* regionData) {
		const uint32 Func_AI_FinishRegionAccessibility = 0x00436CF0;
		__asm {
			PUSHAD
			MOV EBX, regionData
			CALL Func_AI_FinishRegionAccessibility
			POPAD
		}
	}

	int32 AI_SetAttackGroupingRegion(uint32 airAttack, uint32 alsoVeryNear, uint32 player, char* regionData,
		int32 x, int32 y, uint32 alwaysOverwriteLastAttack) {
		const uint32 Func_AI_SetAttackGroupingRegion = 0x004399D0;
		static int32 result;
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

	bool unitCantSeeCloakedTarget(CUnit* unit, CUnit* target) {
		const uint32 Func_unitCantSeeCloakedTarget = 0x00401D60;
		static bool32 bPreResult;
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

	bool unitCanInfest(CUnit* unit) {
		const uint32 Func_CanInfest = 0x00402750;
		static bool32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_CanInfest
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	void AI_AttackUnit(CUnit* unit) {
		const uint32 Func_AI_AttackUnit = 0x0043FFD0;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_AI_AttackUnit
			POPAD
		}
	}

	CUnit* findBestAttackTarget(CUnit* unit) {
		const uint32 Func_Sub443080 = 0x00443080;
		static CUnit* result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub443080
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool function_00462EA0(CUnit* unit, uint8 unknownByteValue) {
		const uint32 Func_Sub462EA0 = 0x00462EA0;
		static uint32 _unknownByteValue;
		static bool32 bPreResult;
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

	void function_00465780(CUnit* unit) {
		const uint32 Func_Sub465780 = 0x00465780;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub465780
			POPAD
		}
	}

	void removeOrderFromUnitQueue(CUnit* unit, COrder* order) {
		const uint32 Func_removeOrderFromUnitQueue = 0x004742D0;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EAX, order
			CALL Func_removeOrderFromUnitQueue
			POPAD
		}
	}

	void function_00474A70(CUnit* unit, CUnit* target, uint8 orderId) {
		const uint32 Func_Sub474A70 = 0x00474A70;
		__asm {
			PUSHAD
			MOV CL, orderId
			MOV EDX, unit
			MOV EAX, target
			CALL Func_Sub474A70
			POPAD
		}
	}

	bool isTargetWithinMinMovementRange(CUnit* unit, CUnit* target, uint32 range) {
		const uint32 Func_isTargetWithinMinMovementRange = 0x004763D0;
		static bool32 bPreResult;
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

	bool function_00476610(CUnit* unit, int x, int y) {
		const uint32 Func_Sub476610 = 0x00476610;
		static bool32 bPreResult;
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

	bool isUnitInWeaponRange(CUnit* unit, CUnit* target) {
		const uint32 Func_isUnitInWeaponRange = 0x00476870;
		static bool32 bPreResult;
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

	void getWeaponBeginIscript(CUnit* unit, IscriptAnimation::Enum anim) {
		const uint32 Func_getWeaponBeginIscript = 0x00476ED0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV DL, anim
			CALL Func_getWeaponBeginIscript
			POPAD
		}
	}

	void function_00477510(CUnit* unit) {
		const uint32 Func_Sub477510 = 0x00477510;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_Sub477510
			POPAD
		}
	}

	void function_00477820(CUnit* unit, uint8 orderId) {
		const uint32 Func_Sub477820 = 0x00477820;
		static uint32 orderId_;
		orderId_ = orderId;
		__asm {
			PUSHAD
			PUSH orderId_
			MOV EAX, unit
			CALL Func_Sub477820
			POPAD
		}
	}

	bool function_00478370(CUnit* unit, uint8 orderId) {
		const uint32 Func_Sub478370 = 0x00478370;
		static bool32 bPreResult;
		static uint32 orderId_;
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

	bool function_00478540(CUnit* unit) {
		const uint32 Func_Sub478540 = 0x00478540;
		static bool32 bPreResult;
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
	bool isUnitCritter(CUnit* unit) {
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

	void moveToCollide(CUnit* unit, CUnit* target) {
		const uint32 Func_moveToCollide = 0x00495400;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOV EAX, target
			CALL Func_moveToCollide
			POPAD
		}
	}

	bool isUnitVisible(CUnit* unit) {
		const uint32 Func_isUnitVisible = 0x004E5DB0;
		static bool32 bPreResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_isUnitVisible
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool isTargetVisible(CUnit* unit, CUnit* target) {
		const uint32 Func_isTargetVisible = 0x004E5E30;
		static bool32 bPreResult;
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
	uint8 getRightClickActionOrder(CUnit* unit) {
		uint8 result;
		if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed) {
			result = RightClickActions::NoMove_NormalAttack;
		}
		else {
			uint8 temp_result = units_dat::RightClickAction[unit->id];
			if (
				temp_result == 0 &&
				unit->status & UnitStatus::GroundedBuilding &&
				unit->isFactory()
				) {
				result = RightClickActions::NormalMove_NoAttack;
			}
			else {
				result = temp_result;
			}
		}
		return result;
	}

	// not related with assigning orders, but only with
	// destinations of orders
	void makeToHoldPosition(CUnit* unit) {
		const uint32 Func_OrdersHoldPositionSuicidal = 0x004EB5B0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_OrdersHoldPositionSuicidal
			POPAD
		}
	}

	bool moveToTarget(CUnit* unit, CUnit* target) {
		const uint32 Func_moveToTarget = 0x004EB720;
		static bool32 bPreResult;
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

	bool SetMoveTarget_xy(CUnit* unit, int x, int y) {
		const uint32 Func_SetMoveTarget_xy = 0x004EB820;
		static bool32 bPreResult;
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
	void setNextWaypoint(CUnit* unit) {
		const uint32 Func_setNextWaypoint = 0x004EB290;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_setNextWaypoint
			POPAD
		}
	}

	bool function_004EB900(CUnit* unit, CUnit* target) {
		const uint32 Func_Sub4EB900 = 0x004EB900;
		static bool32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, target
			MOV ECX, unit
			CALL Func_Sub4EB900
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool changeMovementTargetMoveTarget2(CUnit* unit, int x, int y) {
		const uint32 Func_changeMovementTargetMoveTarget2 = 0x004EB9C0;
		static bool32 bPreResult;
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

	bool isDistanceGreaterThanHaltDistance(CUnit* unit, int x, int y, uint32 distance) {
		const uint32 Func_isDistanceGreaterThanHaltDistance = 0x00401240;
		static bool32 bPreResult;
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

	void checkUnstack(CUnit* unit) {
		const uint32 Func_checkUnstack = 0x0042D9A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_checkUnstack
			POPAD
		}
	}

	bool beginBuilding(CUnit* unit) {
		const uint32 Func_beginBuilding = 0x00467030;
		static bool32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_beginBuilding
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	CUnit* beginGasBuilding(CUnit* unit, uint32 unitId) {
		const uint32 Func_beginGasBuilding = 0x004678A0;
		static CUnit* newUnit;
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
	void addOverlayBelowMain(CSprite* sprite, uint32 imageId, uint32 unk1, uint32 unk2, uint32 unk3) {
		const uint32 Func_addOverlayBelowMain = 0x00498D70;
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

	void replaceSpriteImages(CSprite* sprite, uint32 imageId, uint32 imageDirection) {
		const uint32 Func_Sub499BB0 = 0x00499BB0;
		__asm {
			PUSHAD
			PUSH imageDirection
			PUSH imageId
			MOV EAX, sprite
			CALL Func_Sub499BB0
			POPAD
		}
	}

	void displayLastNetErrorForPlayer(uint32 playerId) {
		const uint32 Func_displayLastNetErrorForPlayer = 0x0049E530;
		__asm {
			PUSHAD
			PUSH playerId
			CALL Func_displayLastNetErrorForPlayer
			POPAD
		}
	}

	void updateUnitStrength(CUnit* unit) {
		const uint32 Func_UpdateUnitStrength = 0x0049FA40;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_UpdateUnitStrength
			POPAD
		}
	}

	void iscript_OpcodeCases(CImage* image, uint32 offset_, uint32 unk1, uint32 unk2) {
		const uint32 Func_iscript_OpcodeCases = 0x004D74C0;
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

	void switchToWarpTexture(CImage* image) {
		const uint32 Func_switchToWarpTexture = 0x004D8500;
		__asm {
			PUSHAD
			MOV EAX, image
			CALL Func_switchToWarpTexture
			POPAD
		}
	}

	bool setThingyVisibilityFlags(CThingy* thingy) {
		const uint32 Func_SetThingyVisibilityFlags = 0x004878F0;
		static bool32 bPreResult;
		__asm {
			PUSHAD
			MOV ESI, thingy
			CALL Func_SetThingyVisibilityFlags
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	CThingy* createThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId) {
		const uint32 Func_CreateThingy = 0x00488210;
		static CThingy* thingy;
		int32 x_ = x;
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

	void playUnitSFX(uint32 soundId, uint16 x, uint16 y, uint32 unk1, uint32 unk2) {
		const uint32 Func_playUnitSFX = 0x0048EC10;
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

	uint32 RandomizeShort(uint32 unknown_index) {
		const uint32 Func_RandomizeShort = 0x004DC4A0;
		static uint32 return_value;
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
	uint32 IterateUnitsAtLocationTargetProc_RecallTargets(CUnit* unit, Box16* coords) {
		const uint32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
		static uint32 return_value;
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

	void setUnitStatTxtErrorMsg(char* message) {
		const uint32 Func_setUnitStatTxtErrorMsg = 0x0048CCB0;
		__asm {
			PUSHAD
			MOV EAX, message
			CALL Func_setUnitStatTxtErrorMsg
			POPAD
		}
	}

	void function_004EBAE0(CUnit* unit, int32 x, int32 y) {
		const uint32 Func_Sub4EBAE0 = 0x004EBAE0;
		__asm {
			PUSHAD
			MOV EDX, unit
			MOV EAX, x
			MOV ECX, y
			CALL Func_Sub4EBAE0
			POPAD
		}
	}

	CUnit* SpiderMine_AcquireTarget(CUnit* spiderMine) {
		const uint32 Func_SpiderMine_AcquireTarget = 0x00441270;
		static CUnit* target;
		__asm {
			PUSHAD
			MOV ESI, spiderMine
			CALL Func_SpiderMine_AcquireTarget
			MOV target, EAX
			POPAD
		}
		return target;
	}

	void instantCloak(CUnit* unit) {
		const uint32 Func_instantCloak = 0x0049B1E0;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_instantCloak
			POPAD
		}
	}

	void unburrowGeneric(CUnit* unit) {
		const uint32 Func_unburrowGeneric = 0x004E99D0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unburrowGeneric
			POPAD
		}
	}

	void function_004E9A30(CUnit* unit) {
		const uint32 Func_Sub4E9A30 = 0x004E9A30;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4E9A30
			POPAD
		}
	}

	// For orders_PlaceMine

	int techUseAllowed(CUnit* a1, __int16 a2, int a3) {
		const uint32 Func_techUseAllowed = 0x0046DD80;
		static int result;
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

	unsigned int getDistanceFast(int a1, int a2, int a3, int a4) {
		const uint32 Func_getDistanceFast = 0x0040C360;
		static unsigned int result;
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

	int orders_HoldPosSuicidal(CUnit* a1) {
		const uint32 Func_orders_HoldPosSuicidal = 0x004EB5B0;
		static int result;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_orders_HoldPosSuicidal
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void fixTargetLocation(unsigned __int16 a1, Point32* a2) {
		const uint32 Func_fixTargetLocation = 0x00401FA0;
		__asm {
			PUSHAD
			MOVZX AX, a1
			MOV EDX, a2
			CALL Func_fixTargetLocation
			POPAD
		}
	}

	void turnUnit(CUnit* a1, char a2) {
		const uint32 Func_turnUnit = 0x00495F20;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_turnUnit
			POPAD
		}
	}

	void toIdle(CUnit* a1) {
		const uint32 Func_toIdle = 0x004753A0;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_toIdle
			POPAD
		}
	}

	int isUnitMovable(CUnit* a1) {
		const uint32 Func_isUnitMovable = 0x00401DC0;
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isUnitMovable
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void orders_Nothing2(CUnit* a1) {
		const uint32 Func_orders_Nothing2 = 0x00475000;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_orders_Nothing2
			POPAD
		}
	}

	bool setMoveTargetXY(int a1, int a2, CUnit* a3) {
		const uint32 Func_setMoveTargetXY = 0x004EB820;
		static int result;
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

	void toggleUnitPath(CUnit* unit) {
		const uint32 Func_toggleUnitPath = 0x004E42A0;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_toggleUnitPath
			POPAD
		}
	}

	char switchSpriteId(CSprite* sprite, int32 spriteId) {
		const uint32 Func_switchSpriteId = 0x00499860;
		static int32 result;
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

	void iscript_OpcodeCases_copy(CImage* image, uint32 offset_, uint32 unk1, uint32 unk2) {
		const uint32 Func_iscript_OpcodeCases = 0x004D74C0;
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

	void replaceSpriteImages_copy(CSprite* sprite, uint32 imageId, uint32 imageDirection) {
		const uint32 Func_Sub499BB0 = 0x00499BB0;
		__asm {
			PUSHAD
			PUSH imageDirection
			PUSH imageId
			MOV EAX, sprite
			CALL Func_Sub499BB0
			POPAD
		}
	}

	void refundBuildingCost(uint16 unitId, uint8 playerId) {
		const uint32 Func_refundBuildingCost = 0x0042CE70;
		__asm {
			PUSHAD
			MOV AX, unitId
			MOV CL, playerId
			CALL Func_refundBuildingCost
			POPAD
		}
	}

	void updateBuildingsAI(CUnit* unit) {
		const uint32 Func_updateBuildingsAI = 0x00433FE0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateBuildingsAI
			POPAD
		}
	}

	//Identical to isValidMorph @ 0x0045CC60;
	bool isValidMorph(uint16 buildingId) {
		bool result;
		if (scbw::isZergBuildingMorphResult(buildingId)) {
			result = true;
		}
		else {
			result = false;
		}
		return result;
	}

	void updateNewUnitVision(uint32 id, int x, int y) {
		const uint32 Func_updateNewUnitVision = 0x0045CE90;
		__asm {
			PUSHAD
			MOV ECX, id
			PUSH y
			PUSH x
			CALL Func_updateNewUnitVision
			POPAD
		}
	}

	void zergPlaceBuildingCntd(CUnit* unit) {
		const uint32 Func_ZergPlaceBuildingCntd = 0x0045D410;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_ZergPlaceBuildingCntd
			POPAD
		}
	}

	void stopCreepDisappearingAtBuilding(CUnit* unit) {
		const uint32 Func_stopCreepDisappearingAtBuilding = 0x0047D770;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_stopCreepDisappearingAtBuilding
			POPAD
		}
	}

	void function_0047DE40(CSprite* sprite, uint32 unitId, int x, int y) {
		const uint32 Func_Sub47DE40 = 0x0047DE40;
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
	void incrementUnitScoresEx(CUnit* building, int32 unk1, int32 unk2) {
		const uint32 Func_incrementUnitScoresEx = 0x00488D50;
		__asm {
			PUSHAD
			MOV EDI, building
			MOV ECX, unk1
			PUSH unk2
			CALL Func_incrementUnitScoresEx
			POPAD
		}
	}

	const uint32 Func_PlayBuildingCompleteSound = 0x0048F310;
	void playBuildingCompleteSound(CUnit* building) {
		__asm {
			PUSHAD
			MOV EAX, building
			CALL Func_PlayBuildingCompleteSound
			POPAD
		}
	}

	const uint32 Func_refreshSpriteData = 0x004983A0;
	void refreshSpriteData(CSprite* sprite) {
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_refreshSpriteData
			POPAD
		}
	}

	const uint32 Func_ReplaceUnitWithType = 0x0049FED0;
	void replaceUnitWithType(CUnit* unit, uint16 newUnitId) {
		uint32 newUnitId_ = newUnitId;
		__asm {
			PUSHAD
			PUSH newUnitId_
			MOV EAX, unit
			CALL Func_ReplaceUnitWithType
			POPAD
		}
	}

	const uint32 Func_fixTargetLocation = 0x00401FA0;
	void fixTargetLocation(Point16* coords, uint32 unitId) {
		__asm {
			PUSHAD
			MOV EAX, unitId
			MOV EDX, coords
			CALL Func_fixTargetLocation
			POPAD
		}
	}

	//Equivalent to function @ 0x00402160
	bool isUnitPositions2Equal(CUnit* unit) {
		return (
			unit->nextTargetWaypoint.x == unit->position.x &&
			unit->nextTargetWaypoint.y == unit->position.y
			);
	}

	//if canBeAborted is TRUE, the unit can be aborted because there's
	//no room around the constructing building
	bool32 buildingAddon(CUnit* unit, uint32 hpGain, bool32 canBeAborted) {
		const uint32 Func_buildingAddon = 0x004679A0;
		static bool32 bResult;
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

	void prependOrderTargetingNothing(CUnit* unit, COrder* order, uint8 orderId) {
		const uint32 Func_prependOrderTargetingNothing = 0x00474760;
		__asm {
			PUSHAD
			MOV ESI, unit
			MOV EDI, order
			MOV BL, orderId
			CALL Func_prependOrderTargetingNothing
			POPAD
		}
	}

	void addOrderTargetingNothing(CUnit* unit, COrder* order, uint8 orderId) {
		const uint32 Func_addOrderTargetingNothing = 0x004749A0;
		__asm {
			PUSHAD
			MOV BL, orderId
			MOV ESI, unit
			MOV EDI, order
			CALL Func_addOrderTargetingNothing
			POPAD
		}
	}

	bool canStillPlaceBuilding() {
		const uint32 Func_canStillPlaceBuilding = 0x0048DDA0;
		bool32 pre_return_value;
		__asm {
			PUSHAD
			CALL Func_canStillPlaceBuilding
			MOV pre_return_value, EAX
			POPAD
		}
		return (pre_return_value != 0);
	}

	void endBuildingPlacement() {
		const uint32 Func_endBuildingPlacement = 0x0048E310;
		__asm {
			PUSHAD
			CALL Func_endBuildingPlacement
			POPAD
		}
	}

	void function_00494BB0(CUnit* unit, int x, int y) {
		const uint32 Func_Sub494BB0 = 0x00494BB0;
		__asm {
			PUSHAD
			MOV ECX, y
			MOV EDX, x
			MOV EAX, unit
			CALL Func_Sub494BB0
			POPAD
		}
	}

	uint32 RandBetween(uint32 min, uint32 max, uint32 someIndex) {
		const uint32 Func_RandBetween = 0x004DC550;
		static uint32 return_value;
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

	char refreshLayers3And4() {
		const uint32 Func_RefreshLayer3And4 = 0x0048D9A0;
		static int result;
		__asm {
			PUSHAD
			CALL Func_RefreshLayer3And4
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	void initBuilding(CUnit* unit, bool32 flag) {
		const uint32 Func_initBuilding = 0x004E65E0;
		__asm {
			PUSHAD
			MOV EDI, unit
			PUSH flag
			CALL Func_initBuilding
			POPAD
		}
	}

	bool orderToMoveToTarget(CUnit* unit, CUnit* target) {
		const uint32 Func__moveToTarget = 0x004EB980;
		static bool32 bPreResult;
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

	bool32 unk_terrainCollision2(CUnit* unit, int x, int y) {
		const uint32 Func_unk_terrainCollision2 = 0x004F1870;
		static bool32 rValue;
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

	int32 canBuildOnTileLocation(CUnit* a1, int32 player, int32 x_tile, int32 y_tile, int32 unitType,
		int32 placement_state_entry, int32 check_vision, int32 also_invisible, int32 without_vision) {
		const uint32 Func_canBuildOnTileLocation = 0x00473FB0;
		static int32 result;
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

	int32 updatePlacementStateForPlacementBox_Func() {
		const uint32 Func_updatePlacementStateForPlacementBox = 0x0048DCE0;
		static int32 result;
		__asm {
			PUSHAD
			CALL Func_updatePlacementStateForPlacementBox
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int32 unk_buildingPlacementRelated(int32 a1, Point16* a2) {
		const uint32 Func_unk_buildingPlacementRelated = 0x0048D7F0;
		static int32 result;
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

	int32 initPlacementBoxPos(Point16* in_out) {
		const uint32 Func_initPlacementBoxPos = 0x0048D660;
		static int32 result;
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

	char unk_buildingPlacementRelatedMouse(int32 a1, int32 a2) {
		const uint32 Func_unk_buildingPlacementRelatedMouse = 0x0048D7B0;
		static int32 result;
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
	void setUnitPathing(CUnit* unit, uint8 unkPathRelated) {
		const uint32 Func_SetUnitPathing = 0x00401360;
		__asm {
			PUSHAD
			MOV AL, unkPathRelated
			MOV ECX, unit
			CALL Func_SetUnitPathing
			POPAD
		}
	}

	CUnit* connectAddonCheck(CUnit* unit) {
		const uint32 Func_ConnectAddonCheck = 0x004404A0;
		CUnit* found_addon;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_ConnectAddonCheck
			MOV found_addon, EAX
			POPAD
		}
		return found_addon;
	}

	void function_00463470(CUnit* unit) {
		const uint32 Func_Sub463470 = 0x00463470;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub463470
			POPAD
		}
	}

	void function_004634E0(CUnit* unit) {
		const uint32 Func_Sub4634E0 = 0x004634E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4634E0
			POPAD
		}
	}

	void function_00463640(CUnit* unit) {
		const uint32 Func_Sub463640 = 0x00463640;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub463640
			POPAD
		}
	}

	void completeAddon(CUnit* unit, CUnit* addon) {
		const uint32 Func_CompleteAddon = 0x00463D50;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOV EAX, addon
			CALL Func_CompleteAddon
			POPAD
		}
	}

	void function_00464300(CUnit* unit) {
		const uint32 Func_Sub464300 = 0x00464300;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_Sub464300
			POPAD
		}
	}

	void disconnectFromAddOn(CUnit* unit) {
		const uint32 Func_Sub464930 = 0x00464930;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub464930
			POPAD
		}
	}

	void function_00469EC0(CUnit* unit, int x, int y) {
		const uint32 Func_Sub469EC0 = 0x00469EC0;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH y
			PUSH x
			CALL Func_Sub469EC0
			POPAD
		}
	}

	void function_00469F60(CUnit* unit, int x, int y) {
		const uint32 Func_Sub469F60 = 0x00469F60;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH y
			PUSH x
			CALL Func_Sub469F60
			POPAD
		}
	}

	void function_0046A560(CUnit* unit) {
		const uint32 Func_Sub46A560 = 0x0046A560;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub46A560
			POPAD
		}
	}

	void function_0046A5A0(CUnit* unit) {
		const uint32 Func_Sub46A5A0 = 0x0046A5A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub46A5A0
			POPAD
		}
	}

	uint32 function_00473FB0(CUnit* unit, uint8 playerId, int x, int y, uint16 unitId, uint8 unk1, uint8 unk2, uint8 unk3, uint8 unk4) {
		const uint32 Func_Sub473FB0 = 0x00473FB0;
		uint32 return_value;
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

	void actUnitReturnToIdle(CUnit* unit) {
		const uint32 Func_ActUnitReturnToIdle = 0x00475420;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_ActUnitReturnToIdle
			POPAD
		}
	}

	bool function_0047DF90(CUnit* unit) {
		const uint32 Func_Sub47DF90 = 0x0047DF90;
		bool32 pre_return_value;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_Sub47DF90
			MOV pre_return_value, EAX
			POPAD		}

		return (pre_return_value != 0);
	}

	void placebuildingProc() {
		const uint32 Func_PracebuildingProc = 0x0048E6E0;
		__asm {
			PUSHAD
			CALL Func_PracebuildingProc
			POPAD
		}
	}

	void playBuildingLandSound(CUnit* unit) {
		const uint32 Func_PlayBuildingLandSound = 0x0048F5A0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_PlayBuildingLandSound
			POPAD
		}
	}

	void SetUnitMovementSpeed(CUnit* unit, uint32 newSpeed) {
		const uint32 Func_SetUnitMovementSpeed = 0x004951C0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV EDX, newSpeed
			CALL Func_SetUnitMovementSpeed
			POPAD
		}
	}

	void function_00498150(CSprite* sprite, uint8 newVerticalOffset) {
		const uint32 Func_Sub498150 = 0x00498150;
		__asm {
			PUSHAD
			MOV EAX, sprite
			MOV CL, newVerticalOffset
			CALL Func_Sub498150
			POPAD
		}
	}

	void function_004997E0(CSprite* sprite) {
		const uint32 Func_Sub4997E0 = 0x004997E0;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_Sub4997E0
			POPAD
		}
	}

	void function_004C3B40(CUnit* unit) {
		const uint32 Func_Sub4C3B40 = 0x004C3B40;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_Sub4C3B40
			POPAD
		}
	}

	bool32 spreadsCreep(uint32 unitId, bool32 defaultReturn) {
		const uint32 Func_spreadsCreep = 0x00413870;
		bool32 bReturnValue;
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

	void function_004148F0(int x, int y, uint32 unitId) {
		const uint32 Func_Sub4148F0 = 0x004148F0;
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			PUSH unitId
			CALL Func_Sub4148F0
			POPAD
		}
	}

	void CreateBuildingLarva(CUnit* main_building) {
		const uint32 Func_Sub49D660 = 0x0049D660;
		__asm {
			PUSHAD
			MOV ESI, main_building
			CALL Func_Sub49D660
			POPAD
		}
	}

	bool32 function_0049EC30(CUnit* unit) {
		const uint32 Func_Sub49EC30 = 0x0049EC30;
		bool32 bResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_Sub49EC30
			MOV bResult, EAX
			POPAD
		}
		return bResult;
	}

	CUnit* FindBestUnit(Box16* coords, uint32 searchProc) {
		const uint32 Func_FindBestUnit = 0x004E8830;
		static CUnit* unit;
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
	CUnit* getUnitPointer() {
		static CUnit* unit = nullptr;
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

	uint8 spreadCreep(uint32 unitId, int x, int y) {
		const uint32 Func_Sub414680 = 0x00414680;
		static uint8 return_value;
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
	uint32 IterateUnitsAtLocationTargetProc_LarvaCount(CUnit* unit, Box16* coords) {
		const uint32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
		static uint32 return_value;
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

	void function_004E8DA0(CUnit* unit) {
		const uint32 Func_Sub4E8DA0 = 0x004E8DA0;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub4E8DA0
			POPAD
		}
	}

	bool function_004E8E10(CUnit* unit, Point32* coords) {
		const uint32 Func_Sub4E8E10 = 0x004E8E10;
		static bool32 return_pre_value;
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

	uint32 getNumSurroundingCreepTiles(uint32 x, uint32 y) {
		const uint32 Func_getNumSurroundingCreepTiles = 0x00414290;
		static uint32 return_val;
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

	void setCreepRelationalData(uint32 x, uint32 y, setCreepRelationalData_callback callback, uint32 arg) {
		const uint32 Func_setCreepRelationalData = 0x00413FA0;
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

	uint32 iterateDirectionalCreepData(uint32 x, uint32 y, uint32 callback, uint32 arg) {
		const uint32 Func_iterateDirectionalCreepData = 0x00413B30;
		static uint32 return_val;
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

	void function_004E97C0(CUnit* unit) {
		const uint32 Func_Sub_4E97C0 = 0x004E97C0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub_4E97C0
			POPAD
		}
	}

	// Note: this function was made from what was seen in this case of use, some things are hardcoded because it's hard to tell what is used or not.
	// Basically, used elsewhere, the function would be implemented differently
	void function_004754F0(CUnit* unit, uint32 unitId, uint32 unk1, uint32 unk2, uint32 orderId, uint32 unk3, uint32 unk4, uint32 unk5) {
		const uint32 Func_Sub_4754F0 = 0x004754F0;
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

	void clearUnitSprites(CSprite* unit_sprite, CSprite* subunit_sprite) {
		const uint32 Func_clearUnitSprites = 0x00496F00;
		__asm {
			PUSHAD
			MOV ECX, subunit_sprite
			MOV ESI, unit_sprite
			CALL Func_clearUnitSprites
			POPAD
		}
	}

	void function_00497480(CSprite* thingySprite, uint32 visibility_flags) {
		const uint32 Func_Sub497480 = 0x00497480;
		__asm {
			PUSHAD
			MOV EBX, thingySprite
			PUSH visibility_flags
			CALL Func_Sub497480
			POPAD
		}
	}

	void _UnitDestructor(CUnit* unit) {
		const uint32 Func__UnitDestructor = 0x004A0740;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func__UnitDestructor
			POPAD
		}
	}

	void UnitDestructor(CUnit* unit) {
		const uint32 Func_UnitDestructor = 0x004A0990;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_UnitDestructor
			POPAD
		}
	}

	void hideAndDisableUnit(CUnit* unit) {
		const uint32 Func_unitDeathSomething_0 = 0x004E6340;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitDeathSomething_0
			POPAD
		}
	}

	void CancelResearch_Proc(CUnit* unit) {
		const uint32 Func_Sub453E30 = 0x00453E30;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_Sub453E30
			POPAD
		}
	}

	void CancelUpgrade_Proc(CUnit* unit) {
		const uint32 Func_Sub454280 = 0x00454280;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub454280
			POPAD
		}
	}

	void refundQueueSlot(CUnit* unit, uint16 slot) {
		const uint32 Func_refundQueueSlot = 0x00466A70;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOVZX EAX, slot
			CALL Func_refundQueueSlot
			POPAD
		}
	}

	void function_00466E40(CUnit* unit) {
		const uint32 Func_Sub466E40 = 0x00466E40;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub466E40
			POPAD
		}
	}

	void CancelAddon_Proc(CUnit* unit) {
		const uint32 Func_Sub4E66E0 = 0x004E66E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4E66E0
			POPAD
		}
	}

	bool32 OrderAllowed(CUnit* unit, uint16 order, uint32 nationID) {
		const uint32 Func_OrderAllowed = 0x0046DC20;
		static bool32 bResult;
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

	bool placeBuildingMsg(uint32 result_function_473FB0) {
		const uint32 Func_placeBuildingMsg = 0x0048D930;
		static bool32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, result_function_473FB0
			CALL Func_placeBuildingMsg
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	bool CMDRECV_PlaceBuildingAllowedHelper(CUnit* unit, uint8 orderId, uint16 builtUnitId) {
		const uint32 Func_CMDRECV_PlaceBuildingAllowed = 0x0048DBD0;
		static bool32 bPreResult;
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

	void function_0048DE70(CUnit* unit, uint32 orderId, uint16 dimensionX, uint16 dimensionY, uint32 builtUnitId) {
		const uint32 Func_Sub48DE70 = 0x0048DE70;
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

	void cmdRECV_PlaceBuilding(int16 x, int16 y, uint8 orderId, uint16 builtUnitId) {
		const uint32 Func_cmdRECV_PlaceBuilding = 0x0048E190;
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

	CUnit* templarMergePartner(CUnit* unit, CUnit** units_list, uint32 units_list_length) {
		const uint32 Func_templarMergePartner = 0x004BFA80;
		static CUnit* return_value;
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

	void parseOrdersDatReqs() {
		const uint32 Func_parseOrdersDatReqs = 0x0046D450;
		__asm {
			PUSHAD
			CALL Func_parseOrdersDatReqs
			POPAD
		}
	}

	int parseRequirementOpcodes(CUnit* unit, uint32 orderId, uint32 playerId, uint32 address, uint32 req_offset) {
		const uint32 Func_parseRequirementOpcodes = 0x0046D610;
		static int return_value;
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

	void ApplySpeedUpgradeFromUnitType(CUnit* unit) {
		const uint32 Func_ApplySpeedUpgradeFromUnitType = 0x00454370;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_ApplySpeedUpgradeFromUnitType
			POPAD
		}
	}

	void function_00463040(CUnit* unit) {
		const uint32 Func_Sub463040 = 0x00463040;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub463040
			POPAD
		}
	}

	void function_00476D30(CUnit* unit) {
		const uint32 Func_Sub476D30 = 0x00476D30;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub476D30
			POPAD
		}
	}

	void incrementUnitScores(CUnit* unit, int32 unkScore) {
		const uint32 Func_incrementUnitScores = 0x00488BF0;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unkScore
			CALL Func_incrementUnitScores
			POPAD
		}
	}

	void function_004A1E50(CUnit* unit, uint32 unk) {
		const uint32 Func_Sub4A1E50 = 0x004A1E50;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EDX, unk
			CALL Func_Sub4A1E50
			POPAD
		}
	}

	void CreateUnitSelection(CUnit* unit, uint32 selectionIndex) {
		const uint32 Func_CreateUnitSelection = 0x004E6180;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH selectionIndex
			CALL Func_CreateUnitSelection
			POPAD
		}
	}

	void DestroyUnitSelection(CUnit* unit) {
		const uint32 Func_DestroyUnitSelection = 0x004E6290;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_DestroyUnitSelection
			POPAD
		}
	}

	void function_004F3B70(CUnit* powerup) {
		const uint32 Func_Sub4F3B70 = 0x004F3B70;
		__asm {
			PUSHAD
			PUSH powerup
			CALL Func_Sub4F3B70
			POPAD
		}
	}

	uint32 getUpgradedWpnCooldown(CUnit* unit, uint8 weaponId) {
		const uint32 Func_getUpgradedWpnCooldown = 0x00475DC0;
		static uint32 return_value;
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

	bool function_00476640(CUnit* unit, uint32 weaponId) {
		const uint32 Func_Sub476640 = 0x00476640;
		static bool32 pre_result;
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

	uint32 get_statTxt_Str_0(CUnit* unit, uint32 playerId, char* message) {
		const uint32 Func_get_statTxt_Str_0 = 0x0048EF30;
		static uint32 return_value;
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

	bool canCastSpell_0(CUnit* unit) {
		const uint32 Func_canCastSpell_0 = 0x00492140;
		static bool32 bPreResult;
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

	uint32 function_00492620(CUnit* unit, bool32 wasMainOrderStateZero) {
		const uint32 Func_Sub492620 = 0x00492620;
		static uint32 return_value;
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

	void refundAllQueueSlots(CUnit* unit) {
		const uint32 Func_RefundAllQueueSlots = 0x00466E80;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RefundAllQueueSlots
			POPAD
		}
	}

	void incrementUnitDeathScores(CUnit* unit, uint8 player) {
		const uint32 Func_IncrementUnitDeathScores = 0x00488AF0;
		__asm {
			PUSHAD
			MOV EDI, unit
			MOVZX EDX, player
			CALL Func_IncrementUnitDeathScores
			POPAD
		}
	}

	void initializeEmptyUnitsLinkedListRef_Sub49E4E0(CUnit* unit, uint8 playerId) {
		const uint32 Func_Sub_49E4E0 = 0x0049E4E0;
		__asm {
			PUSHAD
			MOV DL, playerId
			MOV ECX, unit
			CALL Func_Sub_49E4E0
			POPAD
		}
	}

	const uint32 Func_Sub_49EFA0 = 0x0049EFA0;
	void GiveUnitHelper(CUnit* unit, uint32 playerId, int32 unkScore) {
		__asm {
			PUSHAD
			PUSH unkScore
			MOV ECX, unit
			PUSH playerId
			CALL Func_Sub_49EFA0
			POPAD
		}
	}

	void changeUnitButtonSet_Sub_4E5D60(CUnit* unit, uint16 buttonSetId) {
		const uint32 Func_Sub_4E5D60 = 0x004E5D60;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV CX, buttonSetId
			CALL Func_Sub_4E5D60
			POPAD
		}
	}

	// What make the target reappear
	void showAndEnableUnit(CUnit* unit) {
		const uint32 Func_InitUnitTrapDoodad = 0x004E6490;
		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_InitUnitTrapDoodad
			POPAD
		}
	}

	bool isInfestableUnit(CUnit* unit) {
		const uint32 Func_IsInfestable = 0x00402210;
		static bool32 result;
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

	void playSoundFromDirection(CUnit* a1, int a2) {
		const uint32 Func_playSoundFromDirection = 0x0049B5B0;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_playSoundFromDirection
			POPAD
		}
	}

	void refreshUnitVision(CUnit* a1) {
		const uint32 Func_refreshUnitVision = 0x004E5F30;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_refreshUnitVision
			POPAD
		}
	}

	void initUnitTrapDoodad(CUnit* a1) {
		const uint32 Func_initUnitTrapDoodad = 0x004E6490;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_initUnitTrapDoodad
			POPAD
		}
	}

	COrder* removeOrderFromUnitQueue(COrder* a1, CUnit* a2) {
		const uint32 Func_removeOrderFromUnitQueue = 0x004742D0;
		static COrder* result;
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

	COrder* performAnotherOrder(uint8 a1, int a2, CUnit* a3, int16 a4, Point32* a5, CUnit* a6) {
		const uint32 Func_performAnotherOrder = 0x004745F0;
		static COrder* result;
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

	void order(int a1, CUnit* a2, int8 a3, Point32* a4, CUnit* a5, int a6) {
		const uint32 Func_order = 0x00474810;
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

	uint32 getUnitStrength(CUnit* a1, int a2, bool a3) {
		const uint32 Func_getUnitStrength = 0x00431800;
		static uint32 result;
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

	void playWorkerErrorSound(CUnit* unit, uint32 errorTextIndex) {
		const uint32 Func_PlayWorkerErrorSound = 0x0048F6C0;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH errorTextIndex
			CALL Func_PlayWorkerErrorSound
			POPAD
		}
	}

	bool isUnitUpgradeAvailable(CUnit* unit) {
		const uint32 Func_isUnitUpgradeAvailable = 0x004033D0;
		static bool32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_isUnitUpgradeAvailable
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	void ApplySpeedUpgradeFromUpgradeType(CUnit* unit, uint8 upgradeType) {
		const uint32 Func_ApplySpeedUpgradeFromUpgradeType = 0x00454540;
		__asm {
			PUSHAD
			MOV AL, upgradeType
			PUSH unit
			CALL Func_ApplySpeedUpgradeFromUpgradeType
			POPAD
		}
	}

	void playUpgradeCompleteSound() {
		const uint32 Func_playUpgradeCompleteSound = 0x0048F070;
		__asm {
			PUSHAD
			CALL Func_playUpgradeCompleteSound
			POPAD
		}
	}

	void playResearchCompleteSound() {
		const uint32 Func_playResearchCompleteSound = 0x0048F150;
		__asm {
			PUSHAD
			CALL Func_playResearchCompleteSound
			POPAD
		}
	}

	void setUpgradeLevel(uint32 playerId, uint32 upgradeType, uint8 upgradeLevel) {
		const uint32 Func_setUpgradeLevel = 0x004CE770;
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
	bool advanceRemainingBuildTime(CUnit* unit) {
		uint32 Func_advanceRemainingBuildTime = 0x00466940;
		static bool32 bTimeWasZero;
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
	uint8 function_0046A820(CUnit* unit) {
		uint32 Func_Sub46A820 = 0x0046A820;
		static uint8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub46A820
			MOV result, AL
			POPAD
		}

		return result;
	}

	void orderImmediate(CUnit* unit, uint8 order) {
		uint32 Func_OrderImmediate = 0x00474B40;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV AL, order
			CALL Func_OrderImmediate
			POPAD
		}
	}

	void playMorphingCompleteSound(CUnit* unit) {
		const uint32 Func_PlayMorphingCompleteSound = 0x0048F440;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_PlayMorphingCompleteSound
			POPAD
		}
	}

	// get status (defensiveMatrixHp to acidSporeTime) of unit_src and give it to unit_dst
	void mergeStatusIntoUnit_Sub493180(CUnit* unit_dst, CUnit* unit_src) {
		const uint32 Func_Sub493180 = 0x00493180;
		__asm {
			PUSHAD
			MOV ECX, unit_src
			MOV EAX, unit_dst
			CALL Func_Sub493180
			POPAD
		}
	}

	void function_0049B440(CUnit* unit, uint32 soundId) {
		const uint32 Func_Sub49B440 = 0x0049B440;
		__asm {
			PUSHAD
			MOV EBX, soundId
			MOV EAX, unit
			CALL Func_Sub49B440
			POPAD
		}
	}

	void changeUnitButtonSet_Sub4E5D60(CUnit* unit, uint16 buttonSetId) {
		const uint32 Func_Sub4E5D60 = 0x004E5D60;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV CX, buttonSetId
			CALL Func_Sub4E5D60
			POPAD
		}
	}

	// return true if waiting for recentOrderTimer to reach 0 or if setMoveTarget_xy returned true
	bool delayedSetMoveTarget_xy_Sub4EB960(CUnit* unit, uint16 x, uint16 y) {
		const uint32 Func_Sub4EB960 = 0x004EB960;
		bool32 bPreResult;
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

	void setAllOverlayDirectionsGeneric(CUnit* unit, uint32 unk) {
		const uint32 Func_setAllOverlayDirectionsGeneric = 0x00401140;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unk
			CALL Func_setAllOverlayDirectionsGeneric
			POPAD
		}

	}

	void AI_orderUnitCloaking(CUnit* unit) {
		const uint32 Func_AI_orderUnitCloaking = 0x0043B970;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_AI_orderUnitCloaking
			POPAD
		}
	}

	uint32 function_0043DB50(CUnit* unit) {
		const uint32 Func_Sub43DB50 = 0x0043DB50;
		static uint32 return_value;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_Sub43DB50
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	int getUnitMaxWeaponRange(CUnit* a1, unsigned __int8 a2) {
		const uint32 Func_getUnitMaxWeaponRange = 0x00475870;
		static int result;
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

	bool isTargetWithinMinMovementRange(CUnit* a1, unsigned int a2, CUnit* a3) {
		const uint32 Func_isTargetWithinMinMovementRange = 0x004763D0;
		static int result;
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

	unsigned int getDistanceToUnit(CUnit* a1, CUnit* a2) {
		const uint32 Func_getDistanceToUnit = 0x00430D30;
		static unsigned int result;
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

	unsigned int distanceFromDboxCorner(CUnit* a1, __int16 a2, __int16 a3) {
		const uint32 Func_distanceFromDboxCorner = 0x0043E50;
		static unsigned int result;
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

	bool isAngleInAttackAngle(int a1, int a2, CUnit* a3, unsigned __int8 a4) {
		const uint32 Func_isAngleInAttackAngle = 0x00475BE0;
		static int result;
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

	bool isTargetWithinMinRange(CUnit* a1, unsigned int a2, CUnit* a3) {
		const uint32 Func_isTargetWithinMinRange = 0x00430F10;
		static int result;
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

	int getFlingyHaltDistance(int a1) {
		const uint32 Func_getFlingyHaltDistaince = 0x00494F90;
		static int result;
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

	void unknownOrdersFunc(int a1, __int16 a2, __int16 a3) {
		const uint32 Func_unknownOrdersFunc = 0x00475350;
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

	void prependOrderTargetingGround(__int8 a1, __int16 a2, __int16 a3, int a4) {
		const uint32 Func_prependOrderTargetingGround = 0x00474A20;
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

	signed int moveTowards(CUnit* a1, CUnit* a2) {
		const uint32 Func_moveTowards = 0x004EB900;
		static signed int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_moveTowards
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int ordersHoldPositionSuicidal(CUnit* a1) {
		const uint32 Func_ordersHoldPositionSuicidal = 0x004EB5B0;
		static int result;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_ordersHoldPositionSuicidal
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int getDirectionFromPoints(int a1, int a2, int a3, int a4) {
		const uint32 Func_getDirectionFromPoints = 0x00495300;
		static int result;
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

	CUnit* setSecondaryWaypoint(CUnit* a1, int a2, int a3) {
		const uint32 Func_setSecondaryWaypoint = 0x00494BB0;
		static CUnit* result;
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

	Point32 setResourceTarget(CUnit* a1) {
		const uint32 Func_setResourceTarget = 0x004EB290;
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

	void ordersNothing2(CUnit* a1) {
		const uint32 Func_ordersNothing2 = 0x00475000;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_ordersNothing2
			POPAD
		}
	}

	// for genericRightClickUnitProc2

	int32 isOrderValid(char a1, CUnit* a2, int32 a3, CUnit* a4, int32 a5) {
		const uint32 Func_isOrderValid = 0x00474D90;
		static int32 result;
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

	uint16 getUnitAbilityErrorStatString(CUnit* a1, char a2, int32 a3) {
		const uint32 Func_getUnitAbilityErrorStatString = 0x00491E80;
		static uint32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOVZX BL, a2
			MOV EAX, a1
			CALL Func_getUnitAbilityErrorStatString
			MOV result, EAX
			POPAD
		}
		return (uint16)result;
	}

	// for CMDRECV_HoldPosition

	CUnit* getActivePlayerFirstSelection() {
		const uint32 Func_getActivePlayerFirstSelection = 0x0049A850;
		static CUnit* result;
		__asm {
			PUSHAD
			CALL Func_getActivePlayerFirstSelection
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int32 orderAllowed(CUnit* a1, uint16 a2, int32 a3) {
		const uint32 Func_orderAllowed = 0x0046DC20;
		static int32 result;
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

	void targetedOrder(int16 a1, int32 a2, CUnit* a3, int16 a4, int32 a5, int32 a6, char a7, int32 a8, int32 a9, int32 a10) {
		const uint32 Func_targetedOrder = 0x004754F0;
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

	uint32 getDetectingPlayers(CUnit* unit) {
		const uint32 Func_getDetectingPlayers = 0x00441130;
		static uint32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_getDetectingPlayers
			MOV result, EAX
			POPAD
		}
		return result;
	}

	uint32 iteratePlayerUnitsSomething(uint32 aResult, CUnit* unit) {
		const uint32 Func_iteratePlayerUnitsSomething = 0x0049E8A0;
		static uint32 result;
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

	void makeSpriteVisible(CSprite* sprite) {
		const uint32 Func_makeSpriteVisible = 0x00497ED0;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_makeSpriteVisible
			POPAD
		}
	}

	int32 updateBuildingLandUnitSelection(uint32 unused, CUnit* selection) {
		const uint32 Func_updateBuildingLandUnitSelection = 0x004C3B40;
		static int32 result;
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

	char removeSelectionCircle(CSprite* sprite) {
		const uint32 Func_removeSelectionCircle = 0x004975D0;
		static int32 result;
		__asm {
			PUSHAD
			MOV ECX, sprite
			CALL Func_removeSelectionCircle
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	CImage* makeSpriteInvisible(CSprite* sprite) {
		const uint32 Func_makeUndetected = 0x00498A10;
		static CImage* result;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_makeUndetected
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void function_004756B0(CUnit* unit, uint8 orderId, uint32 unk1, uint32 unk2, uint32 bCommandType) {
		const uint32 Func_Sub4756B0 = 0x004756B0;
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

	void function_004756E0(CUnit* unit, CUnit* target, uint32 orderId, uint32 bCommandType) {
		const uint32 Func_Sub4756E0 = 0x004756E0;
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

	uint32 function_0049A410(CUnit* unit, uint8 orderId) {
		const uint32 Func_Sub49A410 = 0x0049A410;
		static uint32 result;
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
	bool CanTargetSelf(CUnit* unit, uint8 orderId) {
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

	void function_0049A500(CUnit* unit, uint8* array_of_data) {
		const uint32 Func_Sub49A500 = 0x0049A500;
		__asm {
			PUSHAD
			PUSH array_of_data
			PUSH unit
			CALL Func_Sub49A500
			POPAD
		}
	}

	void function_0049A8C0(uint8* array_of_data, bool32 bCanBeObstructed) {
		const uint32 Func_Sub49A8C0 = 0x0049A8C0;
		__asm {
			PUSHAD
			PUSH bCanBeObstructed
			MOV EDI, array_of_data
			CALL Func_Sub49A8C0
			POPAD
		}
	}

	void IgnoreAllScriptAndGotoIdle(CUnit* unit) {
		const uint32 Func_IgnoreAllScriptAndGotoIdle = 0x00476550;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_IgnoreAllScriptAndGotoIdle
			POPAD
		}
	}

	uint32 function_004E76C0(CUnit* transport, CUnit* loaded_unit, Point16* pos) {
		const uint32 Func_Sub4E76C0 = 0x004E76C0;
		static uint32 result;
		__asm {
			PUSHAD
			MOV EAX, transport
			MOV ESI, loaded_unit
			PUSH pos
			CALL Func_Sub4E76C0
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void function_004E7E10(CUnit* unit, uint32 unk) {
		const uint32 Func_Sub4E7E10 = 0x004E7E10;
		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unk
			CALL Func_Sub4E7E10
			POPAD
		}
	}

	void StatsNukesCount(BinDlg* dialog, uint32 index) {
		const uint32 Func_Sub425310 = 0x00425310;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_Sub425310
			POPAD
		}
	}

	void StatHangerCount(BinDlg* dialog, uint32 index) {
		const uint32 Func_StatHangerCount = 0x004253D0;
		__asm {
			PUSHAD
			MOV EBX, dialog
			PUSH index
			CALL Func_StatHangerCount
			POPAD
		}
	}

	void StatsShieldLevel(BinDlg* dialog, uint32 index) {
		const uint32 Func_Sub425510 = 0x00425510;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_Sub425510
			POPAD
		}
	}

	void StatsArmorLevel(BinDlg* dialog, uint32 index) {
		const uint32 Func_Sub425600 = 0x00425600;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_Sub425600
			POPAD
		}
	}

	void StatsWeaponLevel(BinDlg* dialog, uint32 index, uint32 weaponId) {
		const uint32 Func_Sub425790 = 0x00425790;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH weaponId
			PUSH index
			CALL Func_Sub425790
			POPAD
		}
	}

	bool StatSpidermineCount(BinDlg* dialog, uint32 index) {
		const uint32 Func_StatSpidermineCount = 0x00426300;
		static bool32 bPreResult;
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

	__int8 unitGetRace(CUnit* a1) {
		const uint32 Func_unitGetRace = 0x0047B150;
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitGetRace
			MOV result, EAX
			POPAD
		}
		return (__int8)result;
	}

	char* getStatTxtString(__int16 a1) {
		const uint32 Func_getStatTxtString = 0x004C36F0;
		static int result;
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

	char getArmorCount(CUnit* unit, int32 unused) {
		const uint32 Func_getArmorCount = 0x00453FC0;
		static int32 result;
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

	void showDialog(BinDlg* dialog) {
		const uint32 Func_showDialog = 0x004186A0;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_showDialog
			POPAD
		}
	}

	int32 disableDialog(BinDlg* dialog) {
		const uint32 Func_disableDialog = 0x00418640;
		static int32 result;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_disableDialog
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void updateDialog(BinDlg* dialog) {
		const uint32 Func_updateDialog = 0x0041C400;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL Func_updateDialog
			POPAD
		}
	}

	CUnit* findRandomAttackTarget(CUnit* unit) {
		const uint32 Func_findRandomAttackTarget = 0x00442FC0;
		static CUnit* target;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_findRandomAttackTarget
			MOV target, EAX
			POPAD
		}
		return target;
	}

	bool32 attackApplyCooldown(CUnit* unit) {
		const uint32 Func_attackApplyCooldown = 0x00478B40;
		static uint32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_attackApplyCooldown
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void setImageDirection(CImage* image, int8 direction) {
		const uint32 Func_SetImageDirection = 0x004D5F80;
		static uint32 direction_;
		direction_ = direction;
		__asm {
			PUSHAD
			PUSH direction_
			MOV ESI, image
			CALL Func_SetImageDirection
			POPAD
		}
	}

	void ApplySpores(CUnit* unit) {
		const uint32 Func_ApplySpores = 0x004F4480;
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL Func_ApplySpores
			POPAD
		}
	}

	CBullet* createBulletRet(uint8 weaponId, CUnit* source, int16 x, int16 y, uint8 attackingPlayer, uint8 direction) {
		const uint32 Helper_CreateBullet = 0x0048C260;
		static uint32 attackingPlayer_;
		static uint32 direction_;
		static int32 x_;
		static int32 y_;
		attackingPlayer_ = attackingPlayer;
		direction_ = direction;
		x_ = x; y_ = y;
		static CBullet* bullet;
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

	// creates attack overlay & deals damage from the bullet (just calls overlayAndNotify if the weapon deals no damage, or calls target->damageWith if it does
	void weaponBulletShot(CUnit* target, CBullet* bullet, uint32 dmgDivisor) {
		const uint32 Helper_WeaponBulletShot = 0x00479AE0;
		if (bullet != nullptr && target != nullptr && bullet->sourceUnit && bullet->weaponType == WeaponId::PariahIconoclastBlades) {
			if (!globals.was_hit_by_pariah(bullet->sourceUnit, target)) {
				globals.register_pariah_hit(bullet->sourceUnit, target);
			}
			else {
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

	uint8 setArchonPathing(CUnit* unit) {
		const uint32 Func_Sub46A820 = 0x0046A820;
		static uint8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub46A820
			MOV result, AL
			POPAD
		}
		return result;
	}

	bool hasSuppliesForUnit(uint32 unitId, uint32 playerId, bool32 canShowErrorMessage) {
		const uint32 Func_hasSuppliesForUnit = 0x0042CF70;
		static bool32 bPreResult;
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

	CUnit* zergDualBirth(CUnit* unit) {
		const uint32 Func_zergDualBirth = 0x0045D910;
		static CUnit* rValue;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_zergDualBirth
			MOV rValue, EAX
			POPAD
		}
		return rValue;
	}

	void orderNewUnitToRally(CUnit* unit, CUnit* factory) {
		const uint32 Func_Sub466F50 = 0x00466F50;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, factory
			CALL Func_Sub466F50
			POPAD
		}
	}

	void AI_TrainingUnit(CUnit* unit_creator, CUnit* created_unit) {
		const uint32 Func_AI_TrainingUnit = 0x004A2830;
		__asm {
			PUSHAD
			MOV EAX, created_unit
			MOV ECX, unit_creator
			CALL Func_AI_TrainingUnit
			POPAD
		}
	}

	// for orders_Hallucination1

	bool ordersSpell_Sub_4926D0(CUnit* unit, uint32 techId, uint16* techEnergyCost, uint32 sightRange, uint32 error_message_index) {
		const uint32 Func_Sub_4926D0 = 0x004926D0;
		static bool32 bPreResult;
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

	bool placeHallucination(CUnit* unit) {
		const uint32 Func_placeHallucination = 0x004F66D0;
		static bool32 bPreResult;
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

	void applyHallucinationStats(CUnit* halUnit) {
		const uint32 Func_applyHallucinationStats = 0x004F6180;
		__asm {
			PUSHAD
			MOV EAX, halUnit
			CALL Func_applyHallucinationStats
			POPAD
		}
	}

	int32 hallucinateScarabs(CUnit* a1, CUnit* a2) {
		const uint32 Func_hallucinateScarabs = 0x004F65B0;
		static int32 result;
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

	void replaceFeedbackSprite(CUnit* unit, uint32 spriteId) {
		const uint32 Func_replaceFeedbackSprite = 0x004F6350;
		__asm {
			PUSHAD
			MOV EBX, unit
			PUSH spriteId
			CALL Func_replaceFeedbackSprite
			POPAD
		}
	}

	void Ai_UnitWasHit(uint32 main_target_reaction, uint32 dont_call_help, CUnit* attacker, CUnit* target) {
		const uint32 Func_0043F320 = 0x0043F320;
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
	bool isUnitDistanceWithin(CUnit* target, CBullet* bullet, uint32 radius) {
		const uint32 Helper_IsUnitDistanceWithin = 0x0048ADB0;
		uint32 result;
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

	void doWeaponHit(CUnit* target, CUnit* attacker, uint8 dmgDivisor) {
		const uint32 Helper_DoWeaponHit = 0x004795D0;
		uint32 divisor = dmgDivisor;
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

	CUnit* findBestUnit(Box16* radius, int32 bullet, int32(__fastcall* func)(int32, int32)) {
		const uint32 Func_findBestUnit = 0x004E8830;
		static CUnit* result;
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

	int32 splashFunc(CBullet* bullet, CUnit* unit) {
		const uint32 Func_splashFunc = 0x0048B570;
		static int32 result;
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

	void MindControlHit(CUnit* attacker, CUnit* target) {
		const uint32 Func_MindControl_Hit = 0x004F6910;
		__asm {
			PUSHAD
			MOV ESI, target
			PUSH attacker
			CALL Func_MindControl_Hit
			POPAD
		}
	}

	//Somehow related to AI stuff; details unknown.
	uint16 getUnitStrength(CUnit* unit, bool useGroundStrength) {
		const uint32 Helper_GetUnitStrength = 0x00431800;
		static uint16 strength;
		static uint32 useGroundStrength_;
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
	CUnit* createHallucination(CUnit* target, uint32 playerId) {
		const uint32 Func_createHallucination = 0x004F6B90;
		static CUnit* return_value;
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
	uint32 getCorrosiveSporesImage(CUnit* unit) {
		const uint32 Func_getCorrosiveSporesImage = 0x004F41F0;
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_getCorrosiveSporesImage
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void insertFirstOrder(CUnit* unit, uint8 orderId) {
		const uint32 Helper_InsertFirstOrder = 0x004749D0;
		__asm {
			PUSHAD
			MOV AL, orderId
			MOV ECX, unit
			CALL Helper_InsertFirstOrder
			POPAD
		}
	}

	/// Return the count of overlays
	uint8 hasOverlay(CUnit* unit) {
		const uint32 Func_hasOverlay = 0x0047B720;
		static uint8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_hasOverlay
			MOV result, AL
			POPAD
		}
		return result;
	}

	void function_004934B0(CUnit* unit, CUnit* battery) {
		const uint32 Func_Sub4934B0 = 0x004934B0;
		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EBX, battery
			CALL Func_Sub4934B0
			POPAD
		}
	}

	// Identical to unitIsHangerUnit @ 0x00401450
	bool isHangarUnit(CUnit* unit) {
		if (
			unit->id == UnitId::ProtossSteward ||
			unit->id == UnitId::ProtossPlasmaShell
			)
			return true;
		else
			return false;
	}

	CUnit* MedicHeal_TargetAcquire(CUnit* medic) {
		const uint32 Func_MedicHeal_TargetAcquire = 0x004422A0;
		static CUnit* target;
		__asm {
			PUSHAD
			PUSH medic
			CALL Func_MedicHeal_TargetAcquire
			MOV target, EAX
			POPAD
		}
		return target;
	}

	uint32 doMedicHeal(CUnit* unit, CUnit* target) {
		const uint32 Func_doMedicHeal = 0x00463C40;
		static uint32 return_value;
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

	void function_00476FC0(CUnit* unit, CUnit* target, uint32 unk1, uint32 unk2) {
		const uint32 Func_Sub476FC0 = 0x00476FC0;
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

	bool32 function_004770E0(CUnit* unit) {
		const uint32 Func_Sub4770E0 = 0x004770E0;
		static bool32 return_value;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4770E0
			MOV return_value, EAX
			POPAD
		}
		return (return_value != 0);
	}


	bool32 AI_OrderHeal(CUnit* unit, CUnit* target) {
		const uint32 Func_AI_OrderHeal = 0x00463530;
		static bool32 result;
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

	void function_4748E0(CUnit* unit, int x, int y, uint8 orderId) {
		const uint32 Func_sub4748E0 = 0x004748E0;
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

	void function_474940(CUnit* unit, CUnit* target, uint8 orderId) {
		const uint32 Func_sub474940 = 0x00474940;
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

	bool32 AI_CastSpellBehaviour(CUnit* unit, uint32 unknown) {
		const uint32 Func_AI_CastSpellBehaviour = 0x004A13C0;
		static bool32 result;
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

	bool32 function_00438E70(CUnit* unit, int x, int y) {
		const uint32 Func_sub_438E70 = 0x00438E70;
		static bool32 result;
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

	void function_0043DF30(CUnit* unit) {
		const uint32 Func_Sub43DF30 = 0x0043DF30;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_Sub43DF30
			POPAD
		}
	}

	void unknownSpecialOrder(CUnit* unit, COrder* order, uint8 orderId, int16 x, int16 y, CUnit* target, uint16 targetUnitId) {
		const uint32 Func_unknownSpecialOrder = 0x00474540;
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

	void _doOrder(CUnit* unit, uint32 orderId, int x, int y, bool stopPreviousOrders) {
		const uint32 Func__doOrder = 0x00474C70;
		bool32 bStopPreviousOrders = stopPreviousOrders ? 1 : 0;
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

	bool32 function_00477160(CUnit* unit, uint32 unk1, uint32 unk2, uint32 unk3) {
		const uint32 Func_Sub477160 = 0x00477160;
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

	CUnit* getLoadedUnitFromIndex(CUnit* transport, int index) {
		const uint32 Func_getLoadedUnitFromIndex = 0x004E6C40;
		static CUnit* return_value;
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

	bool32 getFirstLoadedWorker(CUnit* transport) {
		const uint32 Func_getFirstLoadedWorker = 0x004E6D90;
		static bool32 return_value;
		__asm {
			PUSHAD
			MOV EAX, transport
			CALL Func_getFirstLoadedWorker
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	bool32 function_004E6EF0(CUnit* unit, CUnit** unitTargetOffset) {
		const uint32 Func_Sub4E6EF0 = 0x004E6EF0;
		bool32 result;
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

	void loadUnitProc(CUnit* unit, CUnit* unitToLoad) {
		const uint32 Func_Sub4E78E0 = 0x004E78E0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, unitToLoad
			CALL Func_Sub4E78E0
			POPAD
		}
	}

	bool32 unloadUnitProc(CUnit* unit) {
		const uint32 Func_Sub4E7F70 = 0x004E7F70;
		static bool32 return_value;
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
	void unitSetRetreatPoint(CUnit* unit, uint8 angleValue) {
		const uint32 Func_UnitSetRetreatPoint = 0x004011A0;
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
	bool getCloserToDestination_sub402BE0(CUnit* unit) {
		const uint32 Func_Sub402BE0_GetCloserToDestination = 0x00402BE0;
		static bool32 bResult;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_Sub402BE0_GetCloserToDestination
			MOV bResult, EAX
			POPAD
		}
		return (bResult == 1);
	}

	const uint32 Func_OrderReturnToIdle = 0x00463770;
	void orderReturnToIdle(CUnit* unit) {
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_OrderReturnToIdle
			POPAD
		}
	}

	const uint32 Func_Sub464290_CheckForCloseUnit = 0x00464290;
	bool checkForCloseUnit_sub464290(CUnit* unit) {
		static bool32 bResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub464290_CheckForCloseUnit
			MOV bResult, EAX
			POPAD
		}
		return (bResult == 1);
	}

	void playSubunitIscript(CUnit* unit, IscriptAnimation::Enum anim) {
		const uint32 Func_PlaySubunitIscript = 0x00474380;
		uint32 anim_ = anim;
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
	uint32 getHPGainForRepair(CUnit* unit) {
		if (*CHEAT_STATE & CheatFlags::OperationCwal) {
			return unit->buildRepairHpGain * 16;
		}
		else {
			return unit->buildRepairHpGain;
		}
	}

	void refundUnitTrainCost(uint32 unitId, uint8 playerId) {
		const uint32 Func_refundUnitTrainCost = 0x0042CEC0;
		__asm {
			PUSHAD
			MOV ECX, unitId
			MOV AL, playerId
			CALL Func_refundUnitTrainCost
			POPAD
		}
	}

	void function_00432430(CUnit* unit, uint32 buildQueueSlot) {
		const uint32 Func_Sub432430 = 0x00432430;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, buildQueueSlot
			CALL Func_Sub432430
			POPAD
		}
	}

	void addHangarUnit(CUnit* main_unit, CUnit* added_unit) {
		const uint32 Func_addHangerUnit = 0x00466300;
		__asm {
			PUSHAD
			MOV EDI, main_unit
			MOV ESI, added_unit
			CALL Func_addHangerUnit
			POPAD
		}
	}

	CUnit* attemptTrainHatchUnit(CUnit* trainer, uint32 builtUnitID, uint32 flag) {
		const uint32 Func_attemptTrainHatchUnit = 0x00468200;
		static CUnit* builtUnit;
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

	void createBullet(uint8 weaponId, CUnit* source, int16 x, int16 y, uint8 attackingPlayer, uint8 direction) {
		const uint32 Helper_CreateBullet = 0x0048C260;
		static uint32 attackingPlayer_;
		static uint32 direction_;
		static int32 x_;
		static int32 y_;
		attackingPlayer_ = attackingPlayer;
		direction_ = direction;
		x_ = x; y_ = y;

		/// Paladin attacking behavior
		if (weaponId == WeaponId::PaladinCrusaderMissiles) {
			if (source->sprite->mainGraphic->direction == 0
				|| source->sprite->mainGraphic->direction == 1
				|| source->sprite->mainGraphic->direction == 2
				|| source->sprite->mainGraphic->direction == 3) {
				x_ = source->sprite->position.x + 6;
				y_ = source->sprite->position.y - 45;
			}
			else if (source->sprite->mainGraphic->direction == 4
				|| source->sprite->mainGraphic->direction == 5) {
				x_ = source->sprite->position.x + 4;
				y_ = source->sprite->position.y - 44;
			}
			else if (source->sprite->mainGraphic->direction == 6
				|| source->sprite->mainGraphic->direction == 7
				|| source->sprite->mainGraphic->direction == 8
				|| source->sprite->mainGraphic->direction == 9) {
				x_ = source->sprite->position.x + 1;
				y_ = source->sprite->position.y - 43;
			}
			else if (source->sprite->mainGraphic->direction == 10
				|| source->sprite->mainGraphic->direction == 11) {
				x_ = source->sprite->position.x - 3;
				y_ = source->sprite->position.y - 42;
			}
			else if (source->sprite->mainGraphic->direction == 12
				|| source->sprite->mainGraphic->direction == 13) {
				x_ = source->sprite->position.x - 6;
				y_ = source->sprite->position.y - 41;
			}
			else if (source->sprite->mainGraphic->direction == 14
				|| source->sprite->mainGraphic->direction == 15) {
				x_ = source->sprite->position.x - 8;
				y_ = source->sprite->position.y - 44;
			}
			else if (source->sprite->mainGraphic->direction == 16) {
				x_ = source->sprite->position.x - 8;
				y_ = source->sprite->position.y - 48;
			}
		}
		__asm {
			PUSHAD
			PUSH direction_
			PUSH attackingPlayer_
			PUSH y_
			PUSH x_
			MOV EAX, source
			MOVZX ECX, weaponId
			CALL Helper_CreateBullet
			POPAD
		}
	}

	// For progressBulletMovement

	void progressBulletMovement_func(CBullet* a1) {
		const uint32 Func_progressBulletMovement = 0x0048B250;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_progressBulletMovement
			POPAD
		}
	}

	// For bulletBehavior_Bounce

	CBullet* changeMovePos(CBullet* a1, int32 a2, int32 a3) {
		const uint32 Func_changeMovePos = 0x00495240;
		static CBullet* result;
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

	CUnit* findNextBounceTarget_func(CBullet* a1) {
		const uint32 Func_findNextBounceTarget = 0x0048B1E0;
		static CUnit* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_findNextBounceTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	bool unitIsSubunit(CUnit* a1) {
		const uint32 Func_unitIsSubunit = 0x00401D40;
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsSubunit
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unitIsCarrier(CUnit* a1) {
		const uint32 Func_unitIsCarrier = 0x00401470;
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsCarrier
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unitIsReaver(CUnit* a1) {
		const uint32 Func_unitIsReaver = 0x00401490;
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsReaver
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	bool unitHasPathToUnit(CUnit* a1, CUnit* a2) {
		const uint32 Func_unitHasPathToUnit = 0x0049CBB0;
		static int32 result;
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

	int32 canBeInfested(CUnit* a1, CUnit* a2) {
		const uint32 Func_canBeInfested = 0x00402210;
		static int32 result;
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

	int32 iterateUnitsAtLocationTargetProc(Box32* a1, int32(__fastcall* a2)(int32, int32), CUnit* a3) {
		const uint32 Func_iterateUnitsAtLocationTargetProc = 0x004E8280;
		static int32 result;
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

	void doMissileDmg(CBullet* bullet) {
		const uint32 func_doMissileDmg = 0x0048B770;
		__asm {
			PUSHAD
			PUSH bullet
			CALL func_doMissileDmg
			POPAD
		}
	}

	const uint32 Func_Sub42D810 = 0x0042D810;
	bool function_0042D810(int x, int y, uint32 unitId) {
		static bool32 bPreReturnValue;
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

	const uint32 Func_GetAllUnitsInBounds = 0x0042FF80;
	CUnit** getAllUnitsInBounds(Box16* coords) {
		static CUnit** units_in_bounds;
		__asm {
			PUSHAD
			MOV EAX, coords
			CALL Func_GetAllUnitsInBounds
			MOV units_in_bounds, EAX
			POPAD
		}
		return units_in_bounds;
	}

	const uint32 Func_getUnitDamageBonus = 0x00475EC0;
	uint32 getUnitDamageBonus(CUnit* unit, uint8 weaponId) {
		static uint32 damageBonus;
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
	void incrementUnitKillCount(CUnit* unit) {
		CUnit* temp_unit = unit;
		if (temp_unit->killCount < 255) {
			temp_unit->killCount++;
		}
		while (
			temp_unit->id == UnitId::ProtossSteward &&
			temp_unit->interceptor.parent != nullptr
			) {
			temp_unit = temp_unit->interceptor.parent;
			if (temp_unit->killCount < 255) {
				temp_unit->killCount++;
			}
		}
	}

	const uint32 Func_attackOverlayAndNotify = 0x00479730;
	void attackOverlayAndNotify(CUnit* attacker, CUnit* target, uint32 weaponType, uint32 direction) {
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

	const uint32 Func_GetUnitBulletDamage = 0x0048ACD0;
	uint32 GetUnitBulletDamage(CBullet* bullet, CUnit* target) {
		static uint32 return_value;
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

	const uint32 Func_getSpellStatString = 0x00491E80;
	uint32 getSpellStatString(CUnit* target, uint32 techId, uint8 playerId) {
		static uint32 return_value;
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

	const uint32 Func_Sub492CC0 = 0x00492CC0;
	void function_492CC0(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub492CC0
			POPAD
		}
	}

	uint8 function_492CC0_ret(CUnit* unit) {
		uint32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub492CC0
			MOV result, EAX
			POPAD
		}
		return result;
	}

	//Identical to setAllImageGroupFlagsPal11 @ 0x00497430;
	void setAllImageGroupFlagsPal11(CSprite* sprite) {
		for (
			CImage* current_image = sprite->images.head;
			current_image != nullptr;
			current_image = current_image->link.next
			) {
			if (current_image->paletteType == PaletteType::RLE_HPFLOATDRAW) {
				current_image->flags |= CImage_Flags::Redraw;
			}
		}
	}

	const uint32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
	void IterateUnitsAtLocationTargetProc_CorrosiveAcid(uint32 attackingPlayerId, Box16* coords) {
		const uint32 CorrosiveAcidProc_function = 0x004F4670;
		__asm {
			PUSHAD
			PUSH attackingPlayerId
			MOV EAX, coords
			MOV EBX, CorrosiveAcidProc_function
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void IterateUnitsAtLocationTargetProc_Ensnare(CUnit* attacker, Box16* coords) {
		const uint32 EnsnareProc_function = 0x004F46C0;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, EnsnareProc_function
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}
	void IterateUnitsAtLocationTargetProc_Plague(CUnit* attacker, Box16* coords) {
		const uint32 PlagueProc_function = 0x004F4AF0;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, PlagueProc_function
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void IterateUnitsAtLocationTargetProc_Maelstrom(CUnit* attacker, Box16* coords) {
		const uint32 MaelstromProc_function = 0x004F6760;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, MaelstromProc_function
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void IterateUnitsAtLocationTargetProc_StasisField(CUnit* attacker, Box16* coords) {
		const uint32 StasisFieldProc_function = 0x004F68D0;
		__asm {
			PUSHAD
			PUSH attacker
			MOV EAX, coords
			MOV EBX, StasisFieldProc_function
			CALL Func_IterateUnitsAtLocationTargetProc
			POPAD
		}
	}

	void createEnsnareOverlay(CUnit* a1) {
		const uint32 Func_createEnsnareOverlay = 0x004F45E0;
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_createEnsnareOverlay
			POPAD
		}
	}

	// For createScanner

	int32 setUnitStatTxtErrorMsg(int32 a1) {
		const uint32 Func_setUnitStatTxtErrorMsg = 0x0048CCB0;
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_setUnitStatTxtErrorMsg
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int32 playSoundFromSource(int32 a1, CUnit* a2, int32 a3, int32 a4) {
		const uint32 Func_playSoundFromSource = 0x0048ED50;
		static int32 result;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			MOV ESI, a2
			MOV EBX, a1
			CALL Func_playSoundFromSource
			MOV result, EAX
			POPAD
		}
		return result;
	}

	//Identical to 0x00401170
	bool unitIsOwnedByCurrentPlayer(CUnit* unit) {
		bool return_value;
		if (*IS_IN_REPLAY != 0 || unit->playerId != *LOCAL_NATION_ID) {
			return_value = false;
		}
		else {
			return_value = true;
		}
		return return_value;
	}

	//Identical to 0x00402A70
	bool isUnitBurrowed(CUnit* unit) {
		return (unit->status & (UnitStatus::CloakingForFree + UnitStatus::Burrowed)) != 0;
	}

	void SC_memcpy_0(uint32 dest, uint32 src, uint32 memsize) {
		const uint32 Func_memcpy_0 = 0x00408FD0;
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

	void function_00468670(CUnit* unit) {
		const uint32 Func_Sub_468670 = 0x00468670;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub_468670
			POPAD
		}
	}

	bool unit_isUnselectable(uint16 unitId) {
		const uint32 Func_Unit_isUnselectable = 0x0046ED80;
		bool32 bHalfReturnValue;
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

	void function_0046F040(CUnit* current_unit, CUnit** unit_list, CUnit* clicked_unit, uint32 list_length) {
		const uint32 Func_Sub_6F040 = 0x0046F040;
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

	uint32 combineLists_Sub_6F290(CUnit* unit, CUnit** unit_list_1, CUnit** unit_list_2, uint32 list_length) {
		const uint32 Func_Sub_46F290 = 0x0046F290;
		uint32 return_value;
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

	void applyNewSelect_Sub_6FA00(CUnit** unit_list, uint32 unit_list_length) {
		const uint32 Func_Sub_46FA00 = 0x0046FA00;
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

	uint32 CUnitToUnitID(CUnit* unit) {
		const uint32 Func_CUnitToUnitID = 0x0047B1D0;
		static uint32 return_value;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_CUnitToUnitID
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	bool unit_IsStandardAndMovable(CUnit* unit) {
		const uint32 Func_Unit_IsStandardAndMovable = 0x0047B770;
		bool32 return_value_unconverted;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_Unit_IsStandardAndMovable
			MOV return_value_unconverted, EAX
			POPAD
		}
		if (unit->id == UnitId::UnusedCaveIn) {
			return false;
		}
		return (return_value_unconverted != 0);
	}

	void selectBuildingSFX(CUnit* unit) {
		const uint32 Func_SelectBuildingSFX = 0x0048F910;
		__asm {
			PUSHAD
			PUSH unit
			CALL Func_SelectBuildingSFX
			POPAD
		}
	}

	bool32 selectSingleUnitFromID(uint32 unitIndex) {
		const uint32 Func_SelectSingleUnitFromID = 0x00496D30;
		bool32 bReturnedValue;
		__asm {
			PUSHAD
			PUSH unitIndex
			CALL Func_SelectSingleUnitFromID
			MOV bReturnedValue, EAX
			POPAD
		}
		return bReturnedValue;
	}

	void CreateNewUnitSelectionsFromList(CUnit** unit_list, uint32 unit_list_length) {
		const uint32 Func_CreateNewUnitSelectionsFromList = 0x0049AE40;
		__asm {
			PUSHAD
			PUSH unit_list_length
			MOV EAX, unit_list
			CALL Func_CreateNewUnitSelectionsFromList
			POPAD
		}
	}

	void CMDACT_Select(CUnit** unit_list, uint32 unit_list_length) {
		const uint32 Func_CMDACT_Select = 0x004C0860;
		__asm {
			PUSHAD
			PUSH unit_list
			PUSH unit_list_length
			CALL Func_CMDACT_Select
			POPAD
		}
	}

	// Equivalent to secondaryOrd_Cloak @ 0x00491790
	void secondaryOrder_Cloak(CUnit* unit) {
		static CUnit** const firstBurrowedUnit = (CUnit**)0x0063FF5C;
			unit->isCloaked++;
		if (
			unit->isCloaked == 1 &&
			!(unit->status & UnitStatus::RequiresDetection) &&
			unit->burrow_link.next == nullptr
			)
		{
			unit->burrow_link.next = *firstBurrowedUnit;
			unit->burrow_link.prev = nullptr;
			if (*firstBurrowedUnit != nullptr) {
				(*firstBurrowedUnit)->burrow_link.prev = unit;
			}
			*firstBurrowedUnit = unit;
			scbw::refreshConsole();
		}
	}

	void removeOverlays2(uint32 from, uint32 to, CUnit* unit) {
		const uint32 Func_removeOverlays2 = 0x00492370;
		__asm {
			PUSHAD
			MOV EAX, from
			MOV EBX, unit
			PUSH to
			CALL Func_removeOverlays2
			POPAD
		}
	}

	bool unitHpIsInRedZone(CUnit* unit) {
		const uint32 Helper_UnitHpIsInRedZone = 0x004022C0;
		static bool32 result;
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
	void RestoreAllUnitStats(CUnit* unit) {
		const uint32 Func_RestoreAllUnitStats = 0x00492F70;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RestoreAllUnitStats
			POPAD
		}
	}

	// For updateUnits

	bool isSelectedUnitGroupEnabled() {
		const uint32 Func_isSelectedUnitGroupEnabled = 0x0049A2C0;
		static int32 result;
		__asm {
			PUSHAD
			CALL Func_isSelectedUnitGroupEnabled
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	void cancelTargetOrder() {
		const uint32 Func_cancelTargetOrder = 0x0048CA10;
		__asm {
			PUSHAD
			CALL Func_cancelTargetOrder
			POPAD
		}
	}

	int32 updateDisruptionWebStatus() {
		const uint32 Func_UpdateDisruptionWebStatus = 0x004EB2F0;
		static int32 result;
		__asm {
			PUSHAD
			CALL Func_UpdateDisruptionWebStatus
			MOV result, EAX
			POPAD
		}
		return result;
	}

	CUnit* progressDyingUnitFrame(CUnit* unit) {
		const uint32 Func_progressDyingUnitFrame = 0x004EB5E0;
		static CUnit* result;
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL Func_progressDyingUnitFrame
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void revealSightAtLocation(int32 sightRange, uint32 visionFlags, int32 ptX, int32 ptY, bool isAirUnit) {
		const uint32 Func_revealSightAtLocation = 0x004806F0;
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

	void updateVisibility(CUnit* unit) {
		const uint32 Func_updateVisibility = 0x004EBE10;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_updateVisibility
			POPAD
		}
	}

	void progressActiveUnitFrame(CUnit* unit) {
		const uint32 Func_progressActiveUnitFrame = 0x004EBC30;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_progressActiveUnitFrame
			POPAD
		}
	}

	void checkUnitVisibility(CUnit* unit) {
		const uint32 Func_checkUnitVisibility = 0x00443390;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_checkUnitVisibility
			POPAD
		}
	}

	void progressUnitFrame(CUnit* unit) {
		const uint32 Func_progressUnitFrame = 0x004ECF70;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_progressUnitFrame
			POPAD
		}
	}

	void unitUpdate(CUnit* unit) {
		const uint32 Func_unitUpdate = 0x004EC420;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_unitUpdate
			POPAD
		}
	}

	void updateUnitPower() {
		const uint32 Func_updateUnitPower = 0x00493790;
		__asm {
			PUSHAD
			CALL Func_updateUnitPower
			POPAD
		}
	}

	// For updateBurrowingCloakingUnits

	int32 isUnitCloaking(CUnit* unit) {
		const uint32 Func_isUnitCloaking = 0x004916E0;
		static int32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_isUnitCloaking
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void playSoundFromDirect(CUnit* unit, int32 soundId) {
		const uint32 Func_playSoundFromDirect = 0x0049B5B0;
		__asm {
			PUSHAD
			MOV ECX, soundId
			MOV EAX, unit
			CALL Func_playSoundFromDirect
			POPAD
		}
	}

	// For doCloak

	void updateSpriteFlagOnPalette(CSprite* sprite) {
		const uint32 Func_updateSpriteFlagOnPalette = 0x00497E80;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_updateSpriteFlagOnPalette
			POPAD
		}
	}

	void showImagesHiddenByCloak(CSprite* sprite) {
		const uint32 Func_showImagesHiddenByCloak = 0x00498170;
		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_showImagesHiddenByCloak
			POPAD
		}
	}

	void updateSpriteImagesForCloak(CSprite* sprite) {
		const uint32 Func_updateSpriteImagesForCloak = 0x00498A90;
		__asm {
			PUSHAD
			MOV EDI, sprite
			CALL Func_updateSpriteImagesForCloak
			POPAD
		}
	}

	uint32 getLarvaeUnitsFromList(CUnit** unitList, CUnit** outputList) {
		const uint32 Func_getLarvaeUnitsFromList = 0x00423190;
		static uint32 larvaeList_Count;
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

	void selectMultipleUnitsFromUnitList(CUnit** unitList, uint32 unitsCount, bool32 unk1, bool32 unk2) {
		const uint32 Func_selectMultipleUnitsFromUnitList = 0x0049AEF0;
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

	int32 BWFXN_PrintXY(const char* text, int32 x, int32 y) {
		const uint32 BWFXN_PrintXY_function = 0x004202B0;
		static int32 result;
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

	int32 GetColorHandle(uint8 color) {
		const uint32 GetColorHandle_function = 0x0041F610;
		static int result;
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

	COrder* queueNewOrder(COrder* a1, int16 a2, int16 a3, CUnit* a4, uint8 a5, int32 a6, int16 a7, Point32* a8, CUnit* a9) {
		const uint32 Func_queueNewOrder = 0x00474B90;
		static COrder* result;
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

	int32 showStatTxtToPlayer(int32 a1, int32 a2) {
		const uint32 Func_showStatTxtToPlayer = 0x0048CF00;
		static int32 result;
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

	void setOrderTarget(uint8 a1, int16 a2, int16 a3, int32 a4, int32 a5, CUnit* a6, int32 a7, Point32* a8, CUnit* a9) {
		const uint32 Func_setOrderTarget = 0x00475470;
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

	int32 selectSingleUnitFromID(int32 a1) {
		const uint32 Func_selectSingleUnitFromID = 0x00496D30;
		static int32 result;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_selectSingleUnitFromID
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int32 selectMultipleUnitsFromUnitList(int32 a1, CUnit** a2, uint8 a3, int32 a4) {
		const uint32 Func_selectMultipleUnitsFromUnitList = 0x0049AEF0;
		static int32 result;
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

	int32 SNetGetTurnsInTransit(uint32* turns) {
		const uint32 SNetGetTurnsInTransit_function = 0x00410275;
		static int32 result;
		_asm {
			PUSHAD
			PUSH turns
			CALL SNetGetTurnsInTransit_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	uint8 sendTurn() {
		const uint32 sendTurn_function = 0x00485A40;
		static uint32 result;
		_asm {
			PUSHAD
			CALL sendTurn_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int32 SErrGetLastError() {
		const uint32 SErrGetLastError_function = 0x004100FA;
		static int32 result;
		_asm {
			PUSHAD
			CALL SErrGetLastError_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	uint8 packetErrHandle(int32 result_arg, int32 a1, const char* a3, int32 a4, int32 a5) {
		const uint32 packetErrHandle_function = 0x004BB0B0;
		static uint32 result;
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
	void getRepairInfo(CUnit* repairTarget, uint32* repairCostMineral, uint32* repairCostGas, int16* unkRepairInfo) {
		const uint32 Func_getRepairInfo = 0x00466D10;
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

	bool ord_repair_subtract(CUnit* worker, CUnit* target) {
		const uint32 Func_ord_repair_subtract = 0x00466FB0;
		static bool32 bPreResult;
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
	bool verifyResources(uint32 mineralCost, uint32 gasCost, uint32 playerId) {
		const uint32 Func_verifyResources = 0x00466EE0;
		bool32 bPreResult;
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

	uint8 getHangerTrainCount(CUnit* unit) {
		const uint32 Func_getHangerTrainCount = 0x00465270;
		static uint8 result = 0;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_getHangerTrainCount
			MOV result, AL
			POPAD
		}
	}

	bool unitIsTrainingOrMorphing(CUnit* unit) {
		const uint32 Func_unitIsTrainingOrMorphing = 0x00401500;
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_unitIsTrainingOrMorphing
			MOV result, EAX
			POPAD
		}
	}

	uint8 initializeRandomizerInfo() {
		const uint32 Func_initializeRandomizerInfo = 0x0047E410;
		static uint32 result;
		__asm {
			PUSHAD
			CALL Func_initializeRandomizerInfo
			MOV result, EAX
			POPAD
		}
		return (uint8)result;
	}

	void AI_Loop() {
		const uint32 Func_AI_Loop = 0x004A2B00;
		__asm {
			PUSHAD
			CALL Func_AI_Loop
			POPAD
		}
	}

	int32 updateActiveTileInfo() {
		const uint32 Func_updateActiveTileInfo = 0x0049C4C0;
		static int32 result;
		__asm {
			PUSHAD
			CALL Func_updateActiveTileInfo
			MOV result, EAX
			POPAD
		}
		return result;
	}

	int32 removeFOWCheat() {
		const uint32 Func_removeFOWCheat = 0x00480880;
		static int32 result;
		__asm {
			PUSHAD
			CALL Func_removeFOWCheat
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void updateUnits() {
		const uint32 Func_updateUnits = 0x004ED000;
		__asm {
			PUSHAD
			CALL Func_updateUnits
			POPAD
		}
	}

	int32 imageDrawingBulletDrawing() {
		const uint32 Func_imageDrawingBulletDrawing = 0x0048C3D0;
		static int32 result;
		__asm {
			PUSHAD
			CALL Func_imageDrawingBulletDrawing
			MOV result, EAX
			POPAD
		}
		return result;
	}

	void updateImages() {
		const uint32 Func_updateImages = 0x00496330;
		__asm {
			PUSHAD
			CALL Func_updateImages
			POPAD
		}
	}

	void updateThingys() {
		const uint32 Func_updateThingys = 0x00488510;
		__asm {
			PUSHAD
			CALL Func_updateThingys
			POPAD
		}
	}

	// AKA createPsiFieldSpriteForUnit()
	// Returns true if successful, false otherwise.
	bool addPsiFieldSprite(CUnit* unit) {
		if (!unit) return false;
		const uint32 Func_addPsiFieldSprite = 0x00493BF0;
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_addPsiFieldSprite
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	void orderInterrupt(CUnit* unit, CUnit* target, uint16 x, uint16 y, uint32 orderId) {
		const uint32 Func_orderInterrupt = 0x00474C30;
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

	void unitOrderMoveToTargetUnit(CUnit* unit, CUnit* target) {
		const uint32 Func_unitOrderMoveToTargetUnit = 0x00479FE0;
		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, target
			CALL Func_unitOrderMoveToTargetUnit
			POPAD
		}
	}

	void QueueGameCommand(uint8* params, uint32 param_length) {
		const uint32 Func_QueueGameCommand = 0x00485BD0;
		__asm {
			PUSHAD
			MOV EDX, param_length
			MOV ECX, params
			CALL Func_QueueGameCommand
			POPAD
		}
	}



	const uint32 Func_CreateThingy = 0x00488210;
	CThingy* createThingyFunc(uint32 spriteId, int16 x, int16 y, uint32 playerId) {
		static CThingy* thingy;
		int32 x_ = x;
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

	const uint32 Func_DoUnitsCollide = 0x00469B60;
	bool func_dounitscollide(CUnit* first, CUnit* second) {
		static uint32 result = 0;
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

	const uint32 Func_Sub_4E97C0 = 0x004E97C0;
	void unburrow(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub_4E97C0
			POPAD
		}
	}

	const uint32 Func_SMemAlloc = 0x0041006A;
	void* SMemAlloc(uint32 amount, uint32 unkFilename, uint32 unkLine, uint32 defaultValue) {

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
	const uint32 Func_SetCallbackTimer = 0x00416090;
	void SetCallbackTimer(BinDlg* dialog, uint32 unk1, uint32 unk2, uint32 unk3) {

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

	const uint32 Func_Sub49A7F0 = 0x0049A7F0;
	void function_0049A7F0(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub49A7F0
			POPAD
		}

	}

	uint32 fxnInteract(BinDlg* dialog, uint32 data_struct_offset) {

		static uint32 return_value;
		static uint32 fxnInteract;

		fxnInteract = (uint32)dialog->fxnInteract;

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

	const uint32 Func_registerUserDialogAction = 0x00418100;
	void registerUserDialogAction(BinDlg* dialog, uint32 array_of_fxnInteractFunc, uint32 sizeOfArray) {

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

	const uint32 Func_Sub418E00 = 0x00418E00;
	void function_418E00(BinDlg* dialog) {

		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_Sub418E00
			POPAD
		}

	}

	;

	const uint32 Func_ReplayStatBtns = 0x00427A80;
	void replayStatBtns(BinDlg* dialog) {
		__asm {
			PUSHAD
			MOV EBX, dialog
			CALL Func_ReplayStatBtns
			POPAD
		}
	}

	;

	const uint32 Func_Sub_4591D0 = 0x004591D0;
	void function_4591D0() {

		__asm {
			PUSHAD
			CALL Func_Sub_4591D0
			POPAD
		}

	}

	;

	const uint32 Func_Sub_459770 = 0x00459770;
	void function_459770() {
		__asm {
			PUSHAD
			CALL Func_Sub_459770
			POPAD
		}
	}

	;

	const uint32 Func_BINDLG_BlitSurface = 0x004C35F0;
	void BINDLG_BlitSurface(BinDlg* dialog) {
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_BINDLG_BlitSurface
			POPAD
		}
	}

	;

	//dev info: examples of calls: 429520,428e60,428420,4287D0,4292C0
	int32 req_check(uint32 reqFunc, uint16 reqVar, uint32 playerId, CUnit* unit) {

		static int32 return_value;

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

	void updateMineralPatchImage(CUnit* mineralPatch) {
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
		if (anim != mineralPatch->building.resource.resourceIscript) {
			mineralPatch->building.resource.resourceIscript = anim;
			mineralPatch->sprite->playIscriptAnim(anim);
		}
	}

	void updateImagePositionOffset(CImage* image) {

		const uint32 Func_updateImagePositionOffset = 0x004D5A00;

		__asm {
			PUSHAD
			MOV ECX, image
			CALL Func_updateImagePositionOffset
			POPAD
		}

	}

	//Identical to function @ 0x004F3AF0
	void setResourceAmountCarried(CUnit* worker, uint8 amountCarried, uint32 chunkImageId, bool isMineral) {

		if (worker->resourceType != 0)
			return;

		worker->resourceType = isMineral ? 2 : 1;

		CImage* chunkImage = worker->sprite->createOverlay(chunkImageId);

		if (chunkImage && !(chunkImage->flags & CImage_Flags::Flag07)) {
			chunkImage->flags |= CImage_Flags::Flag07;
			updateImagePositionOffset(chunkImage);
		}

		worker->worker.resourceCarryAmount = amountCarried;
		scbw::refreshConsole();

	}

	const uint32 Func_0x00433410 = 0x00433410;
	int Ai_HasBuildingsNeedingPower(uint32 player) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL Func_0x00433410
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_0x00433470 = 0x00433470;
	int Ai_CountEconomyUnits(uint32 player, uint32 unit_id) {
		static int result;
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

	const uint32 Func_0x004889D0 = 0x004889D0;
	int getSuppliesRemaining(uint8 player, uint16 unit_id) {
		static int result;
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

	const uint32 Func_4888c0 = 0x004888C0;
	int getSuppliesUsed(int byte_data, uint8 player) {
		static uint32 result;
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

	const uint32 Func_AI_AddCostToNeeds = 0x00447DC0;
	int32 AI_AddCostToNeeds(int32 a1, int32 a2, int32 a3) {
		static int32 result;
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

	const uint32 Func_AI_AddSpendingRequest = 0x00447980;
	int32 AI_AddSpendingRequest(int16 a1, int32 a2, char a3, char a4, void* a5) {
		static int32 result;
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
	const uint32 Func_isAIAllowedToBuild = 0x00447830;
	bool isAIAllowedToBuild(int32 a1, int32 a2) {
		static int32 result;
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

	void removeAITownGasReferences(CUnit* unit) {
		const uint32 Func_RemoveGasRefs = 0x00432760;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_RemoveGasRefs
			POPAD
		}
	}

	void deleteBuildingAi(void* building_ai_list, BuildingAi* ai) {
		const uint32 Func_DeleteBuildingAi = 0x00404500;
		__asm {
			PUSHAD
			MOV EDX, building_ai_list
			MOV EAX, ai
			CALL Func_DeleteBuildingAi
			POPAD
		}
	}

	void deleteWorkerAi(void* worker_ai_list, WorkerAi* ai) {
		const uint32 Func_DeleteWorkerAi = 0x00404470;
		__asm {
			PUSHAD
			MOV EDX, worker_ai_list
			MOV EAX, ai
			CALL Func_DeleteWorkerAi
			POPAD
		}
	}

	void deleteAiTownIfNeeded(AiTown* town) {
		const uint32 Func_DeleteAiTownIfNeeded = 0x004347E0;
		__asm {
			PUSHAD
			MOV EAX, town
			CALL Func_DeleteAiTownIfNeeded
			POPAD
		}
	}

	// For isOrderValid
	bool unitIsResource(CUnit* unit) {
		const uint32 Func_unitIsResource = 0x004688E0;
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitIsResource
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	// sign flag
	int8 SETS(int x)
	{
		if (sizeof(int) == 1)
			return int8(x) < 0;
		if (sizeof(int) == 2)
			return int16(x) < 0;
		if (sizeof(int) == 4)
			return int32(x) < 0;
		return int64(x) < 0;
	}

	int8 OFSUB(int x, int y)
	{
		int y2 = y;
		int8 sx = SETS(x);
		return (sx ^ SETS(y2)) & (sx ^ SETS(x - y2));
	}
}