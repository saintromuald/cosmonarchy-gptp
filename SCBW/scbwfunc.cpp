#include "scbwfunc.h"

const uint32 Func_HasMoneyCanMake = 0x00467250;
bool HasMoneyCanMake(CUnit* builder, uint32 unitToBuild) {

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

const uint32 Func_removeOrderFromUnitQueue = 0x004742D0;
void removeOrderFromUnitQueue(CUnit* unit, COrder* order) {

	__asm {
		PUSHAD
		MOV ECX, unit
		MOV EAX, order
		CALL Func_removeOrderFromUnitQueue
		POPAD
	}

}

// For loadRaceUI
const uint32 Func_getRaceSoundList = 0x0048FB40;
void getRaceSoundList(int8 a1) {
	__asm {
		PUSHAD
		PUSH a1
		CALL Func_getRaceSoundList
		POPAD
	}
}

const uint32 Func_dialogGrpConstruct = 0x004197B0;
int32 dialogGrpConstruct(int32 a1, int32 a2, int32 a3, int32(__fastcall* a4)(int32, int32, int32, int32)) {
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

const uint32 Func_loadGraphic = 0x004D2B30;
void loadGraphic(int8 a1, int32 a2, int32 a3, int8 a4, int32 a5) {
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

const uint32 Func_randFunc = 0x0040C7BD;
int randFunc() {
	static int result;
	__asm {
		PUSHAD
		CALL Func_randFunc
		MOV result, EAX
		POPAD
	}
	return result;
}

const uint32 Func_unitHPBelow33Percent = 0x004022C0;
bool unitHPbelow33_percent(CUnit* a1) {
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

const uint32 Func_unitHPBetween33_66Percent = 0x00402270;
bool unitHPbetween33_66_percent(int32 a1, CUnit* a2) {
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

const uint32 Func_getMaxHP = 0x00401400;
int32 getMaxHP(CUnit* a1) {
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

const uint32 Func_getLastQueueSlotType = 0x0047B270;
uint16 getLastQueueSlotType(CUnit* a1) {
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

const uint32 Func_addTextToDialog = 0x004258B0;
void addTextToDialog(int32 a1, int16 a2, const char* a3) {
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

const uint32 Func_drawEnergy = 0x00425A30;
void drawEnergy(int32 a1) {
	__asm {
		PUSHAD
		MOV EBX, a1
		CALL Func_drawEnergy
		POPAD
	}
}

const uint32 Func_hideOptionalStatusScreenControls = 0x00457310;
void hideOptionalStatusScreenControls(BinDlg* a1) {
	__asm {
		PUSHAD
		MOV EAX, a1
		CALL Func_hideOptionalStatusScreenControls
		POPAD
	}
}

const uint32 Func_showControlGroup = 0x00457250;
void showControlGroup(BinDlg* a1, int16 a2, int16 a3) {
	__asm {
		PUSHAD
		PUSH a3
		MOV EAX, a1
		MOVZX CX, a2
		CALL Func_showControlGroup
		POPAD
	}
}

const uint32 Func_addTextToDialog = 0x004258B0;
void addTextToDialog(int a1, int16 a2, int a3) {
	__asm {
		PUSHAD
		PUSH a3
		MOV EAX, a1
		MOVZX CX, a2
		CALL Func_addTextToDialog
		POPAD
	}
}

const uint32 Func_drawShieldUpgrade = 0x00425510;
void drawShieldUpgrade(int a1, uint16 a2) {
	__asm {
		PUSHAD
		PUSH a2
		MOV EAX, a1
		CALL Func_drawShieldUpgrade
		POPAD
	}
}

const uint32 Func_hideDialog = 0x00418700;
void hideDialog(BinDlg* a1) {
	__asm {
		PUSHAD
		MOV ESI, a1
		CALL Func_hideDialog
		POPAD
	}
}

// For drawSupplyUnitInfo

const uint32 Func_getSuppliesUsed = 0x00424732;
int getSuppliesUsed() {
	static int result;
	__asm {
		PUSHAD
		CALL Func_getSuppliesUsed
		MOV result, EAX
		POPAD
	}
	return result;
}

const uint32 Func_getLastQueueSlotType = 0x0047B270;
uint16 getLastQueueSlotType(CUnit* a1) {
	static uint32 result;
	__asm {
		PUSHAD
		MOV EDX, a1
		CALL Func_getLastQueueSlotType
		MOV result, EAX
		POPAD
	}
	return (uint16)result;
}

const uint32 Func_getSuppliesAvailable = 0x00488900;
uint32 getSuppliesAvailable(uint8 a1, int a2) {
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

const uint32 Func_drawArmor = 0x00425600;
void drawArmor(int a1, int a2, uint16 a3) {
	__asm {
		PUSHAD
		PUSH a3
		MOV EAX, a1
		MOV EDX, a2
		CALL Func_drawArmor
		POPAD
	}
}

const uint32 Func_unitStatAct_Standard = 0x00426F50;
void unitStatAct_Standard(void* a1) {
	__asm {
		PUSHAD
		MOV ECX, a1
		CALL Func_unitStatAct_Standard
		POPAD
	}
}

// For AI_TargetEnemyProc

const uint32 Func_AI_UnitCanAttack = 0x00476180;
bool AI_UnitCanAttack(CUnit* a1) {
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

const uint32 Func_needRedrawUI = 0x00424A10;
int needRedrawUI() {
	static int result;
	__asm {
		PUSHAD
		CALL Func_needRedrawUI
		MOV result, EAX
		POPAD
	}
	return result;
}

const uint32 Func_genericStatusUpdateDrawnValues = 0x004248F0;
CUnit* genericStatusUpdateDrawnValues() {
	static CUnit* result;
	__asm {
		PUSHAD
		CALL Func_genericStatusUpdateDrawnValues
		MOV result, EAX
		POPAD
	}
	return result;
}

const uint32 Func_drawProgressBar = 0x004260C0;
void drawProgressBar(int a1) {
	__asm {
		PUSHAD
		MOV EDI, a1
		CALL Func_drawProgressBar
		POPAD
	}
}

const uint32 Func_drawNukeSiloInfo = 0x00426FF0;
void drawNukeSiloInfo(int a1) {
	__asm {
		PUSHAD
		PUSH a1
		CALL Func_drawNukeSiloInfo
		POPAD
	}
}

const uint32 Func_unitIsActiveTransport = 0x004E6BA0;
bool unitIsActiveTransport(CUnit* a1) {
	static int result;
	__asm {
		PUSHAD
		MOV ECX, a1
		CALL Func_unitIsActiveTransport
		MOV result, EAX
		POPAD
	}
	return (bool)result;
}

const uint32 Func_isQueueSlotActive = 0x00401E70;
bool isQueueSlotActive(CUnit* a1, int a2) {
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

const uint32 Func_drawTrainingProgress = 0x004268D0;
void drawTrainingProgress(BinDlg* a1) {
	__asm {
		PUSHAD
		PUSH a1
		CALL Func_drawTrainingProgress
		POPAD
	}
}

const uint32 Func_isConstructingAddon = 0x004E66B0;
bool isConstructingAddon(CUnit* a1) {
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

const uint32 Func_isAttemptingProtossBuild = 0x004E4C40;
bool isAttemptingProtossBuild(CUnit* a1) {
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

const uint32 Func_drawAddonProgress = 0x00425F30;
void drawAddonProgress(int a1) {
	__asm {
		PUSHAD
		MOV EDI, a1
		CALL Func_drawAddonProgress
		POPAD
	}
}

const uint32 Func_drawTechProgress = 0x004266F0;
void drawTechProgress(int a1) {
	__asm {
		PUSHAD
		PUSH a1
		CALL Func_drawTechProgress
		POPAD
	}
}

const uint32 Func_drawUpgradeProgress = 0x00426500;
void drawUpgradeProgress(BinDlg* a1) {
	__asm {
		PUSHAD
		PUSH a1
		CALL Func_drawUpgradeProgress
		POPAD
	}
}

const uint32 Func_unitIsGeyser = 0x004688B0;
bool unitIsGeyser(CUnit* a1) {
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

const uint32 Func_drawGasQuantity = 0x00426190;
void drawGasQuantity(BinDlg* a1) {
	__asm {
		PUSHAD
		MOV EAX, a1
		CALL Func_drawGasQuantity
		POPAD
	}
}

const uint32 Func_unitGetGroundWeapon = 0x00475AD0;
int8 unitGetGroundWeapon(CUnit* a1) {
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

const uint32 Func_drawDisplayButtons = 0x00426C60;
void drawDisplayButtons(int a1) {
	__asm {
		PUSHAD
		PUSH a1
		CALL Func_drawDisplayButtons
		POPAD
	}
}

const uint32 Func_getControlFromIndex = 0x00418080;
BinDlg* getControlFromIndex(BinDlg* a1, int16 a2) {
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

const uint32 Func_setTextStr = 0x004263E0;
void setTextStr(int a1) {
	__asm {
		PUSHAD
		PUSH a1
		CALL Func_setTextStr
		POPAD
	}
}

const uint32 Func_setUnitStatusStrText = 0x00425B50;
void setUnitStatusStrText(int a1) {
	__asm {
		PUSHAD
		PUSH a1
		CALL Func_setUnitStatusStrText
		POPAD
	}
}

const uint32 Func_unitGetName = 0x0047B5A0;
int unitGetName(CUnit* a1) {
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
const uint32 Func_getCurrentHitPointsPlusShields = 0x004026D0;
int getCurrentHitPointsPlusShields(CUnit* a1) {
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

const uint32 Func_getBaseStrength = 0x004316D0;
int getBaseStrength(int a1, CUnit* a2) {
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

const uint32 Func_getMaxHP = 0x00401400;
int getMaxHP(CUnit* a1) {
	static int result;
	__asm {
		PUSHAD
		MOV EAX, a1
		CALL Func_getMaxHP
		MOV result, EAX
		POPAD
	}
	return result;
}

// For AI_RestorationRequirementsProc
const uint32 Func_canWeaponTargetUnit = 0x00475CE0;
int canWeaponTargetUnit(uint8 a1, CUnit* a2, CUnit* a3) {
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

const uint32 Func_AI_TargetUnitIsWorthHitting = 0x00440E30;
int32 AI_TargetUnitIsWorthHitting(CUnit* a1, CUnit* a2) {
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

const uint32 Func_createSplashWeaponProc = 0x004E82E0;
int createSplashWeaponProc(int16 a1, int16 a2, int16 a3, int(__fastcall* a4)(int, int), int a5) {
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

const uint32 Func_unitShieldsClumpInAreaAccumulatorProc = 0x00440A60;
int unitShieldsClumpInAreaAccumulatorProc(CUnit* a1, CUnit* a2) {
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
const uint32 Func_unitWasHit = 0x004795D0;
void unitWasHit(CUnit* a1, CUnit* a2, int a3) {
	__asm {
		PUSHAD
		PUSH a3
		PUSH a2
		MOV EBX, a1
		CALL Func_unitWasHit
		POPAD
	}
}

const uint32 Func_doAttackNotifyEvent = 0x0048F230;
void doAttackNotifyEvent(CUnit* a1) {
	__asm {
		PUSHAD
		PUSH a1
		CALL Func_doAttackNotifyEvent
		POPAD
	}
}

const uint32 Func_createShieldOverlay = 0x004E6140;
CImage* createShieldOverlay(int a1, CUnit* a2) {
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
const uint32 Func_unk_billions = 0x0047A750;
int unk_billions(int a1, int32 a2, int a3, int a4) {
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

const uint32 Func_unk_billions2 = 0x0047A6E0;
int unk_billions2(int a1, int32 a2, int a3, int a4) {
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

const uint32 Func_hpBarSurfaceBuffer = 0x0047A350;
int hpBarSurfaceBuffer(int a1, int a2, int* a3, int a4, int a5) {
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

const uint32 Func_hpBarSurfaceBuffer2 = 0x0047A120;
int hpBarSurfaceBuffer2(int a1, int a2, uint16 a3, int a4) {
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

const uint32 Func_renderHPBar = 0x0047A4E0;
int renderHPBar(int a1, int a2, int* a3, int a4, int a5) {
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

const uint32 Func_getUnitMaxEnergy = 0x00491870;
int32 getUnitMaxEnergy(CUnit* a1) {
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

const uint32 Func_getHpBarDrawSizes = 0x0047A670;
int getHpBarDrawSizes(int a1, int32 a2, int a3, int a4) {
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
const uint32 Func_unitIsFrozen = 0x004020B0;
bool unitIsFrozen(CUnit* a1) {
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

const uint32 Func_isUnitCasterNotHallucination = 0x00401210;
bool isUnitCasterNotHallucination(CUnit* a1) {
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

const uint32 Func_getActiveTileFlag = 0x00472F80;
bool getActiveTileFlag(int16 a1, int16 a2, int a3, int a4) {
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

const uint32 Func_getTargetUnitAbilityErrorStatString = 0x00491E80;
CUnit* getTargetUnitAbilityErrorStatString(CUnit* a1, char a2, int a3) {
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
const uint32 Func_SErrSetLastError = 0x00410172;
int SErrSetLastError(char a1) {
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

// For compileHealthBar

const uint32 Func_updateImageDrawData = 0x004D57B0;
void updateImageDrawData(CImage* a1) {
	__asm {
		PUSHAD
		MOV EAX, a1
		CALL Func_updateImageDrawData
		POPAD
	}
}

const uint32 Func_playImageIscript = 0x004D8470;
void playImageIscript(CImage* a1, char a2) {
	__asm {
		PUSHAD
		PUSH a2
		MOV ECX, a1
		CALL Func_playImageIscript
		POPAD
	}
}

// For AI_FindTransport
const uint32 Func_canEnterTransport = 0x004E6E00;
bool canEnterTransport(CUnit* a1, CUnit* a2) {
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

const uint32 Func_SAI_GetRegionIdFromPxEx = 0x0049C9F0;
int16 SAI_GetRegionIdFromPxEx(int32 a1, int32 a2) {
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

const uint32 Func_getUnitDistanceToHalt = 0x00402140;
int32 getUnitDistanceToHalt(int a1, CUnit* a2, int a3) {
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
const uint32 Func_dataTransfer00 = 0x00472A00;
int32 dataTransfer00(int a1, char a2, int a3) {
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

const uint32 Func_dataTransfer01 = 0x0045A600;
int dataTransfer01(uint32 a1, int a2, uint32 a3) {
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

const uint32 Func_dataTransfer02 = 0x0045A230;
int32 dataTransfer02(int a1, int a2, char a3) {
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

const uint32 Func_dataTransfer03 = 0x0045A1F0;
int32 dataTransfer03(int a1, char a2, int a3) {
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

const uint32 Func_dataTransfer04 = 0x0045A390;
int dataTransfer04(uint32 a1, int a2, int a3) {
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

const uint32 Func_dataTransfer05 = 0x0045A190;
int32 dataTransfer05(int a1, int a2) {
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

const uint32 Func_vectorTransfer = 0x0045A010;
vectorHead* vectorTransfer(char a1, char a2) {
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

const uint32 Func_dataTransfer06 = 0x00472900;
int dataTransfer06(int a1) {
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
const uint32 Func_randGetSFXBetween = 0x0048E940;
int randGetSFXBetween(int a1, int a2) {
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

const uint32 Func_SAI_GetRegionIdFromPxEx = 0x0049C9F0;
int16 SAI_GetRegionIdFromPxEx(int32 x, int32 y) {
	static int32 result;
	__asm {
		PUSHAD
		MOV EDI, x
		MOV ECX, y
		CALL Func_SAI_GetRegionIdFromPxEx
		MOV result, EAX
		POPAD
	}
	return (int16)result;
}


// for AI_TrainUnit
const uint32 Func_AI_TrainingWorker = 0x00435700;
char AI_TrainingWorker(CUnit* result, CUnit* parent) {
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

const uint32 Func_AI_TrainingOverlord = 0x00435770;
void AI_TrainingOverlord(CUnit* result, CUnit* parent) {
	__asm {
		PUSHAD
		MOV EAX, result
		MOV ECX, parent
		CALL Func_AI_TrainingOverlord
		POPAD
	}
}

const uint32 Func_AI_TrainingNormal = 0x00435DB0;
void AI_TrainingNormal(CUnit* result, CUnit* parent) {
	__asm {
		PUSHAD
		MOV EAX, result
		MOV ECX, parent
		CALL Func_AI_TrainingNormal
		POPAD
	}
}

const uint32 Func_AI_OrderToDestination = 0x0043D5D0;
void AI_OrderToDestination(CUnit* unit, int orderId, int x, int y) {
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

const uint32 Func_AI_AssignCaptainToSlowestUnit = 0x00436F70;
void AI_AssignCaptainToSlowestUnit(AiCaptain* region) {
	__asm {
		PUSHAD
		PUSH region
		CALL Func_AI_AssignCaptainToSlowestUnit
		POPAD
	}
}

// for AI_CreateNuke

const uint32 Func_createUnit = 0x004A09D0;
CUnit* createUnit(int16 unitType, int32 x, int32 y, int32 playerId) {
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

const uint32 Func_updateUnitStatsFinishBuilding = 0x004A01F0;
void updateUnitStatsFinishBuilding(CUnit* unit) {
	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_updateUnitStatsFinishBuilding
		POPAD
	}
}

const uint32 Func_updateUnitStrengthAndApplyDefaultOrders = 0x0049FA40;
uint32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit) {
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

const uint32 Func_hideUnit = 0x004E6340;
void hideUnit(CUnit* unit) {
	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_hideUnit
		POPAD
	}
}

// for AI_TrainingBroodling

const uint32 Func_AI_AssignMilitary = 0x004390A0;
void AI_AssignMilitary(int32 regionType, AiCaptain* region) {
	__asm {
		PUSHAD
		MOV EBX, regionType
		MOV ESI, region
		CALL Func_AI_AssignMilitary
		POPAD
	}
}

const uint32 Func_AI_AddMilitary = 0x0043DA20;
void AI_AddMilitary(AiCaptain* region, CUnit* unit, int32 always_this_region) {
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

const uint32 Func_AI_CreateBuildingAi = 0x004044C0;
UnitAi* AI_CreateBuildingAi(BuildingAiArray* buildingAI) {
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

const uint32 Func_orderComputerClear = 0x00475310;
void orderComputerClear(CUnit* unit, uint8 orderId) {
	__asm {
		PUSHAD
		MOV ESI, unit
		MOVZX CL, orderId
		CALL Func_orderComputerClear
		POPAD
	}
}

const uint32 Func_setSecondaryOrder = 0x004743D0;
CUnit* setSecondaryOrder(CUnit* result, char orderId) {
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

const uint32 Func_AI_MakeGuard = 0x0043A010;
void AI_MakeGuard(int32 playerId, CUnit* unit) {
	__asm {
		PUSHAD
		PUSH unit
		PUSH playerId
		CALL Func_AI_MakeGuard
		POPAD
	}
}

const uint32 Func_unitIsActiveResourceDepot = 0x00401CF0;
bool unitIsActiveResourceDepot(CUnit* unit) {
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

const uint32 Func_getPlayerDefaultRefineryUnitType = 0x004320D0;
int32 getPlayerDefaultRefineryUnitType(int32 playerId) {
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

// for AI_UnitWasHit // some of these should probably be methods in api/etc
const uint32 Func_AI_CancelStructure = 0x00468280;
char AI_CancelStructure(CUnit* structure) {
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

const uint32 Func_AI_GetActualTarget = 0x0043AD60;
CUnit* AI_GetActualTarget(CUnit* attacker, CUnit* target) {
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

const uint32 Func_checkArbiterCloakingRange = 0x004410C0;
CUnit* checkArbiterCloakingRange(CUnit* unit, uint16 id) {
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

const uint32 Func_AI_OrderUnitCloaking = 0x0043B970;
void AI_OrderUnitCloaking(CUnit* unit) {
	__asm {
		PUSHAD
		MOV ESI, unit
		CALL Func_AI_OrderUnitCloaking
		POPAD
	}
}

const uint32 Func_AI_OrderLurkerBurrow = 0x0043C230;
void AI_OrderLurkerBurrow(CUnit* unit) {
	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_AI_OrderLurkerBurrow
		POPAD
	}
}

const uint32 Func_AI_IsUnderAttack = 0x00436E70;
bool AI_IsUnderAttack(CUnit* unit, int32 flag) {
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

const uint32 Func_unitIsActiveTransport = 0x004E6BA0;
bool unitIsActiveTransport(CUnit* unit) {
	static uint32 result;
	__asm {
		PUSHAD
		MOV ECX, unit
		CALL Func_unitIsActiveTransport
		MOV result, EAX
		POPAD
	}
	return (bool)result;
}

const uint32 Func_isUnitLoaded = 0x004E7110;
int32 isUnitLoaded(CUnit* unit) {
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

const uint32 Func_AI_TryReturnHome = 0x00462EA0;
int32 AI_TryReturnHome(CUnit* unit, int32 dontIssueOrder) {
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

const uint32 Func_AI_TransportAction = 0x0043DB50;
int32 AI_TransportAction(CUnit* unit) {
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

const uint32 Func_AI_AskForHelp = 0x0043ADA0;
int32 AI_AskForHelp(CUnit* self, CUnit* attacker, int32 onlyNearbyGuards) {
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

const uint32 Func_getRegionIdFromUnit = 0x0049CB40;
int16 getRegionIdFromUnit(CUnit* unit) {
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

const uint32 Func_getEnemyAirStrengthInRegion = 0x00431D00;
int32 getEnemyAirStrengthInRegion(uint16 regionId, int32 player) {
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

const uint32 Func_getEnemyStrengthInRegion = 0x004318E0;
int32 getEnemyStrengthInRegion(uint16 regionId, int32 player, int32 includeAir) {
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

const uint32 Func_unitIsPerformingUnbreakableCode = 0x00402310;
bool unitIsPerformingUnbreakableCode(CUnit* unit) {
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

const uint32 Func_AI_CastSpellBehavior = 0x004A13C0;
bool AI_CastSpellBehavior(CUnit* unit, bool isAggro) {
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

const uint32 Func_AI_Flee = 0x0043E400;
int32 AI_Flee(CUnit* unit, CUnit* enemy) {
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

const uint32 Func_unitHasGroundPath = 0x0042FA00;
bool unitHasGroundPath(CUnit* unit, int16 x, int16 y) {
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

const uint32 Func_AI_HasNoDefenseBuild = 0x00447370;
bool AI_HasNoDefenseBuild(int32 player) {
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

const uint32 Func_unitCanSeeCloakedTarget = 0x00401D60;
bool unitCanSeeCloakedTarget(CUnit* target, CUnit* unit) {
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

const uint32 Func_AI_RespondToCloakedThreat = 0x0043C580;
void AI_RespondToCloakedThreat(CUnit* threat, CUnit* unit) {
	__asm {
		PUSHAD
		PUSH unit
		MOV EAX, threat
		CALL Func_AI_RespondToCloakedThreat
		POPAD
	}
}

const uint32 Func_unitCanAttackTarget = 0x00476730;
bool unitCanAttackTarget(CUnit* target, CUnit* unit, int32 checkDetection) {
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

const uint32 Func_AI_UpdateAttackTarget = 0x00477160;
int32 AI_UpdateAttackTarget(CUnit* unit, int32 acceptIfSieged, int32 acceptCritters, int32 mustReach) {
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

/*	const uint32 Func_AI_PrepareToFlee = 0x00476A50;
	__int64 AI_PrepareToFlee(CUnit* enemy, CUnit* unit) {
		static __int64 result;
		__asm {
			PUSH unit
			MOV EAX, enemy
			CALL Func_AI_PrepareToFlee
			MOV result, EAX
			POPAD
		}
		return result;
	}*/

const uint32 Func_orderInterrupt = 0x00474C30;
void orderInterrupt(CUnit* unit, uint8 orderId, Point32* pos, CUnit* target) {
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
const uint32 Func_AI_RemoveFromTowns = 0x00432760;
uint16 AI_RemoveFromTowns(CUnit* worker) {
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

const uint32 Func_AI_DeleteBuildingAI = 0x00404500;
BuildingAi* AI_DeleteBuildingAI(BuildingAi* building, BuildingAiArray* buildingList) {
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

const uint32 Func_AI_GetQueuedCount = 0x00466B70;
int32 AI_GetQueuedCount(CUnit* builder, uint32 unitId) {
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

const uint32 Func_AI_PrepareGuardAI = 0x00462670;
void AI_PrepareGuardAI(uint16 y, uint32 player, uint16 unitId, uint16 x) {
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

// Attack extender

// for AI_TrainAttackForce
const uint32 Func_AI_AddRegionToMilitary = 0x0043E2E0;
int32 AI_AddRegionToMilitary(AiCaptain* region, int32 unitId, int32 priority) {
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

const uint32 Func_AI_FindUnit = 0x00447DC0;
uint32 AI_FindUnit(uint32 unitId, uint32 playerId, uint32 type) {
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

const uint32 Func_AI_AddSpendingRequest = 0x00447980;
uint32 AI_AddSpendingRequest(uint16 unitId, uint32 type, uint8 player, char priority, void* value) {
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

const uint32 Func_AI_AddBaseUnitSpendingRequest = 0x004484A0;
uint32 AI_AddBaseUnitSpendingRequest(uint32 unitId, uint32 player, AiCaptain* region, char priority) {
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
const uint32 Func_AI_RecalculateRegionStrength = 0x0043A390;
uint32 AI_RecalculateRegionStrength(AiCaptain* region) {
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

const uint32 Func_AI_TargetEnemyInRegion = 0x0043CC40;
CUnit* AI_TargetEnemyInRegion(AiCaptain* region) {
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

const uint32 Func_AI_MoveMilitaryToDefend = 0x0043DE40;
int32 AI_MoveMilitaryToDefend(AiCaptain* region) {
	static int32 result;
	__asm {
		PUSHAD
		MOV EDI, region
		CALL Func_AI_TargetEnemyInRegion
		MOV result, EAX
		POPAD
	}
	return result;
}

const uint32 Func_AI_GetRegionMilitaryTypes = 0x00438FC0;
void AI_GetRegionMilitaryTypes(AiCaptain* region, int32* hasGroundMilitary,
	int32* cantAttackGround, int32* cantAttackAir) {
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

const uint32 Func_AI_CanTravelToRegion = 0x0043A790;
int32 AI_CanTravelToRegion(AiCaptain* region1, AiCaptain* region2) {
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

const uint32 Func_AI_PickAttackTarget_NoBuildings = 0x0043A510;
CUnit* AI_PickAttackTarget_NoBuildings(AiCaptain* region) {
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

const uint32 Func_AI_MoveMilitaryToOtherRegion = 0x0043E580;
uint32 AI_MoveMilitaryToOtherRegion(AiCaptain* target, AiCaptain* source, uint32 pullUnitsNearRegion,
	uint32 sendPureSpellcasters, uint32 dontSendFromBunkers) {
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

const uint32 Func_AI_IterateTownSomething = 0x0043DD20;
AiTown* AI_IterateTownSomething(AiCaptain* region) {
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

const uint32 Func_AI_AttackTimerDecrement = 0x00447090;
uint32 AI_AttackTimerDecrement(uint32 player) {
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

const uint32 Func_AI_HasForcesMovingToAttack = 0x0043A2E0;
int32 AI_HasForcesMovingToAttack(uint32 player) {
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
const uint32 Func_PopulateRegionsWithOwn = 0x00438B30;
uint16 PopulateRegionsWithOwn(char* regionData, uint32 player) {
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

const uint32 Func_PopulateRegionsWithEnemy = 0x00439B50;
void PopulateRegionsWithEnemy(uint32 player, char* regionData) {
	__asm {
		PUSHAD
		PUSH regionData
		PUSH player
		CALL Func_PopulateRegionsWithEnemy
		POPAD
	}
}

const uint32 Func_AI_GetNeighborRegionTypes = 0x00436D80;
int32 AI_GetNeighborRegionTypes(char* regionData) {
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

const uint32 Func_AI_FinishRegionAccessibility = 0x00436CF0;
void AI_FinishRegionAccessibility(char* regionData) {
	__asm {
		PUSHAD
		MOV EBX, regionData
		CALL Func_AI_FinishRegionAccessibility
		POPAD
	}
}

const uint32 Func_AI_SetAttackGroupingRegion = 0x004399D0;
int32 AI_SetAttackGroupingRegion(uint32 airAttack, uint32 alsoVeryNear, uint32 player, char* regionData,
	int32 x, int32 y, uint32 alwaysOverwriteLastAttack) {
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

const uint32 Func_unitCantSeeCloakedTarget = 0x00401D60;
bool unitCantSeeCloakedTarget(CUnit* unit, CUnit* target) {

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

;

const uint32 Func_IsInfestable = 0x00402210;
bool isInfestableUnit(CUnit* unit) {

	static bool32 bPreResult;

	__asm {
		PUSHAD
		MOV EDX, unit
		CALL Func_IsInfestable
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}

;

const uint32 Func_CanInfest = 0x00402750;
bool unitCanInfest(CUnit* unit) {

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

;

const uint32 Func_AI_AttackUnit = 0x0043FFD0;
void AI_AttackUnit(CUnit* unit) {

	__asm {
		PUSHAD
		PUSH unit
		CALL Func_AI_AttackUnit
		POPAD
	}

}

;

const uint32 Func_Sub443080 = 0x00443080;
//named according to attack_priority_inject.cpp
CUnit* findBestAttackTarget(CUnit* unit) {

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

;

const uint32 Func_Sub462EA0 = 0x00462EA0;
bool function_00462EA0(CUnit* unit, uint8 unknownByteValue) {

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

;

const uint32 Func_Sub465780 = 0x00465780;
void function_00465780(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EDI, unit
		CALL Func_Sub465780
		POPAD
	}

}

;

const uint32 Func_removeOrderFromUnitQueue = 0x004742D0;
void removeOrderFromUnitQueue(CUnit* unit, COrder* order) {

	__asm {
		PUSHAD
		MOV ECX, unit
		MOV EAX, order
		CALL Func_removeOrderFromUnitQueue
		POPAD
	}

}

;

const uint32 Func_Sub474A70 = 0x00474A70;
void function_00474A70(CUnit* unit, CUnit* target, uint8 orderId) {

	__asm {
		PUSHAD
		MOV CL, orderId
		MOV EDX, unit
		MOV EAX, target
		CALL Func_Sub474A70
		POPAD
	}

}

;

const uint32 Func_isTargetWithinMinMovementRange = 0x004763D0;
bool isTargetWithinMinMovementRange(CUnit* unit, CUnit* target, uint32 range) {										// EDI = unit or subunit,ECX = MaxWpnRange,EAX = target

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

;

const uint32 Func_Sub476610 = 0x00476610;
bool function_00476610(CUnit* unit, int x, int y) {

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

;

const uint32 Func_isUnitInWeaponRange = 0x00476870;
bool isUnitInWeaponRange(CUnit* unit, CUnit* target) {

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

;

const uint32 Func_getWeaponBeginIscript = 0x00476ED0;
void getWeaponBeginIscript(CUnit* unit, IscriptAnimation::Enum anim) {

	__asm {
		PUSHAD
		MOV EAX, unit
		MOV DL, anim
		CALL Func_getWeaponBeginIscript
		POPAD
	}

}

;

const uint32 Func_Sub477510 = 0x00477510;
void function_00477510(CUnit* unit) {

	__asm {
		PUSHAD
		PUSH unit
		CALL Func_Sub477510
		POPAD
	}

}

;

const uint32 Func_Sub477820 = 0x00477820;
void function_00477820(CUnit* unit, uint8 orderId) {

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

;

const uint32 Func_Sub478370 = 0x00478370;
bool function_00478370(CUnit* unit, uint8 orderId) {

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

;

const uint32 Func_Sub478540 = 0x00478540;
bool function_00478540(CUnit* unit) {

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

;

//Identical to Unit::isCritter() @ 0x0047B450
bool isUnitCritter(CUnit* unit) {

	bool bReturnValue;

	/*	if(
			unit->id == UnitId::Critter_Rhynadon ||
			unit->id == UnitId::Critter_Bengalaas ||
			unit->id == UnitId::Critter_Ragnasaur ||
			unit->id == UnitId::Critter_Kakaru ||
			unit->id == UnitId::Critter_Scantid ||
			unit->id == UnitId::Critter_Ursadon
		)
			bReturnValue = true;
		else*/
	bReturnValue = false;

	return bReturnValue;

}

;

const uint32 Func_Sub495400 = 0x00495400;
void function_00495400(CUnit* unit, CUnit* target) {

	__asm {
		PUSHAD
		MOV EDI, unit
		MOV EAX, target
		CALL Func_Sub495400
		POPAD
	}

}

;

const uint32 Func_SAI_GetRegionIdFromPxEx = 0x0049C9F0;
uint16 SAI_GetRegionIdFromPxEx(int32 x, int32 y) {

	static uint16 result;

	__asm {
		PUSHAD
		MOV EDI, x
		MOV ECX, y
		CALL Func_SAI_GetRegionIdFromPxEx
		MOV result, AX
		POPAD
	}

	return result;

}

;

const uint32 Func_isUnitVisible = 0x004E5DB0;
bool isUnitVisible(CUnit* unit) {

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

;

const uint32 Func_isTargetVisible = 0x004E5E30;
bool isTargetVisible(CUnit* unit, CUnit* target) {

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

;

//Identical to getRightClickActionOrder @ 0x004E5EA0;
uint8 getRightClickActionOrder(CUnit* unit) {

	uint8 result;

	if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
		result = RightClickActions::NoMove_NormalAttack;
	else {

		uint8 temp_result = units_dat::RightClickAction[unit->id];

		if (
			temp_result == 0 &&
			unit->status & UnitStatus::GroundedBuilding &&
			unit->isFactory()
			)
			result = RightClickActions::NormalMove_NoAttack;
		else
			result = temp_result;

	}

	return result;

}

;

//not related with assigning orders, but only with
//destinations of orders
const uint32 Func_OrdersHoldPositionSuicidal = 0x004EB5B0;
void makeToHoldPosition(CUnit* unit) {

	__asm {
		PUSHAD
		MOV ESI, unit
		CALL Func_OrdersHoldPositionSuicidal
		POPAD
	}

}

;

const uint32 Func_moveToTarget = 0x004EB720;
bool moveToTarget(CUnit* unit, CUnit* target) {

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

;

const uint32 Func_SetMoveTarget_xy = 0x004EB820;
bool SetMoveTarget_xy(CUnit* unit, int x, int y) {

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

;

//Related to path/movement decision
const uint32 Func_sub_4EB290 = 0x004EB290;
void setNextWaypoint_Sub4EB290(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_sub_4EB290
		POPAD
	}
}

;

const uint32 Func_Sub4EB900 = 0x004EB900;
bool function_004EB900(CUnit* unit, CUnit* target) {

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

;

const uint32 Func_Sub4EB9C0 = 0x004EB9C0;
bool function_004EB9C0(CUnit* unit, int x, int y) {

	static bool32 bPreResult;

	__asm {
		PUSHAD
		MOV EDX, unit
		MOV ECX, x
		MOV EAX, y
		CALL Func_Sub4EB9C0
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}

const uint32 Func_isDistanceGreaterThanHaltDistance = 0x00401240;
bool isDistanceGreaterThanHaltDistance(CUnit* unit, int x, int y, uint32 distance) {

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

;

const uint32 Func_Sub42D9A0 = 0x0042D9A0;
void function_0042D9A0(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub42D9A0
		POPAD
	}

}

;

const uint32 Func_AI_TrainingOverlord = 0x00435770;
void AI_TrainingOverlord(CUnit* main_unit, CUnit* created_unit) {

	__asm {
		PUSHAD
		MOV EAX, created_unit
		MOV ECX, main_unit
		CALL Func_AI_TrainingOverlord
		POPAD
	}

}

;

const uint32 Func_Sub467030 = 0x00467030;
bool function_00467030(CUnit* unit) {

	static bool32 bPreResult;

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub467030
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}

;

const uint32 Func_Sub4678A0 = 0x004678A0;
CUnit* function_004678A0(CUnit* unit, uint32 unitId) {

	static CUnit* newUnit;

	__asm {
		PUSHAD
		MOV EAX, unit
		PUSH unitId
		CALL Func_Sub4678A0
		MOV newUnit, EAX
		POPAD
	}

	return newUnit;

}

;

//imageId may not be really a parameter
const uint32 Func_Sub498D70 = 0x00498D70;
void function_00498D70(CSprite* sprite, uint32 imageId, uint32 unk1, uint32 unk2, uint32 unk3) {

	__asm {
		PUSHAD
		MOV EAX, sprite
		MOV ESI, imageId
		PUSH unk1
		PUSH unk2
		PUSH unk3
		CALL Func_Sub498D70
		POPAD
	}

}

;

//named based on old removed unit_morph hooks set 
const uint32 Func_Sub499BB0 = 0x00499BB0;
void replaceSpriteImages(CSprite* sprite, uint32 imageId, uint32 imageDirection) {
	__asm {
		PUSHAD
		PUSH imageDirection
		PUSH imageId
		MOV EAX, sprite
		CALL Func_Sub499BB0
		POPAD
	}
}

;

const uint32 Func_displayLastNetErrForPlayer = 0x0049E530;
void displayLastNetErrForPlayer(uint32 playerId) {
	__asm {
		PUSHAD
		PUSH playerId
		CALL Func_displayLastNetErrForPlayer
		POPAD
	}
}

;

const uint32 Func_UpdateUnitStrength = 0x0049FA40;
void updateUnitStrength(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_UpdateUnitStrength
		POPAD
	}

}

;

const uint32 Func_Sub4A01F0 = 0x004A01F0;
void function_004A01F0(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub4A01F0
		POPAD
	}

}

;

const uint32 Func_iscript_OpcodeCases = 0x004D74C0;
void iscript_OpcodeCases(CImage* image, uint32 offset_, uint32 unk1, uint32 unk2) {

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

;

const uint32 Func_Sub4D8500 = 0x004D8500;
void function_004D8500(CImage* image) {

	__asm {
		PUSHAD
		MOV EAX, image
		CALL Func_Sub4D8500
		POPAD
	}

}

;

const uint32 Func_CreateUnit = 0x004A09D0;
CUnit* createUnit(uint32 unitId, int x, int y, uint32 playerId) {

	static CUnit* unit_created;

	__asm {
		PUSHAD
		PUSH playerId
		PUSH y
		MOV ECX, unitId
		MOV EAX, x
		CALL Func_CreateUnit
		MOV unit_created, EAX
		POPAD
	}

	return unit_created;

}

;

//not related with assigning orders, but only with
//destinations of orders
const uint32 Func_OrdersHoldPositionSuicidal = 0x004EB5B0;
void makeToHoldPosition(CUnit* unit) {

	__asm {
		PUSHAD
		MOV ESI, unit
		CALL Func_OrdersHoldPositionSuicidal
		POPAD
	}

}

;

const uint32 Func_Sub4EB9C0 = 0x004EB9C0;
bool function_004EB9C0(CUnit* unit, int x, int y) {

	static bool32 bPreResult;

	__asm {
		PUSHAD
		MOV EDX, unit
		MOV ECX, x
		MOV EAX, y
		CALL Func_Sub4EB9C0
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}

;

//original referenced name was sub_4878F0, but using
	//the name from bunker_hooks.cpp since it got meaning
const uint32 Func_SetThingyVisibilityFlags = 0x004878F0;
bool setThingyVisibilityFlags(CThingy* thingy) {

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

;

//original referenced name was replaceSprite (but this one is probably
//more accurate since it does create something rather than replacing)
const uint32 Func_CreateThingy = 0x00488210;
CThingy* createThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId) {

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

;

const uint32 Func_playUnitSFX = 0x0048EC10;
void playUnitSFX(uint32 soundId, uint16 x, uint16 y, uint32 unk1, uint32 unk2) {

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

;

const uint32 Func_RandomizeShort = 0x004DC4A0;
uint32 RandomizeShort(uint32 unknown_index) {

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

;

const uint32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
//hardcoding the recall targets selection function
uint32 IterateUnitsAtLocationTargetProc_RecallTargets(CUnit* unit, Box16* coords) {

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

;

const uint32 Func_setUnitStatTxtErrorMsg = 0x0048CCB0;
void setUnitStatTxtErrorMsg(char* message) {

	__asm {
		PUSHAD
		MOV EAX, message
		CALL Func_setUnitStatTxtErrorMsg
		POPAD
	}

}

;

const uint32 Func_Sub4EBAE0 = 0x004EBAE0;
void function_004EBAE0(CUnit* unit, int32 x, int32 y) {

	__asm {
		PUSHAD
		MOV EDX, unit
		MOV EAX, x
		MOV ECX, y
		CALL Func_Sub4EBAE0
		POPAD
	}

}