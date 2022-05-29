// Contains the definitions of various data structures, arrays, pointers and
// internal constants in StarCraft.exe.
// The contents of this file are merged from: unit.cpp, triggers.cpp,
// resources.cpp, player.cpp, locations.cpp, buttons.cpp, and qdp.cpp.

#pragma once
#include "../definitions.h"
#include "structures.h"
#include "scbwhelper.h"
#pragma pack(1)

#define SCBW_DATA(type, name, offset) type const name = (type)offset;
//
/// @name Linked list nodes
//////////////////////////////////////////////////////////////// @{

/// The first nodes in StarCraft's internal linked lists (taken from BWAPI's Offsets.h)
SCBW_DATA(Unit**, firstVisibleUnit, 0x00628430);
/// Includes units in transports/bunkers, worker inside gas buildings, nukes in
/// silos, queens inside command centers, and units in production.
SCBW_DATA(Unit**,		firstHiddenUnit,		0x006283EC);
SCBW_DATA(Unit**,		lastHiddenUnit,			0x00628428);
SCBW_DATA(Unit**,		firstScannerSweep,		0x006283F4);
SCBW_DATA(Unit**,		lastScannerSweep,		0x00628434);
SCBW_DATA(Unit**,		firstPsiFieldProvider,	0x0063FF54);
SCBW_DATA(Bullet**,		firstBullet,			0x0064DEC4);
SCBW_DATA(Bullet**,		lastBullet,				0x0064DEAC);
SCBW_DATA(Bullet**,		firstFreeBullet,		0x0064EED8);
SCBW_DATA(Bullet**,		lastFreeBullet,			0x0064EEDC);
SCBW_DATA(u32*,			bulletCount,			0x0064DEBC);

struct Units12 {
	Unit* unit[12];
};
SCBW_DATA(Units12*, firstPlayerUnit, 0x006283F8);  //Indexed by player Id

//////////////////////////////////////////////////////////////// @}

/// @name Structure tables
//////////////////////////////////////////////////////////////// @{

SCBW_DATA(Units12*, completed_factory_counts, 0x0068A9D8);
SCBW_DATA(Units12*, all_factory_counts, 0x0068A9A8);

SCBW_DATA(Unit*, unitTable, 0x0059CCA8);
SCBW_DATA(u32*, unitCount, 0x006283F0);
SCBW_DATA(Bullet*, bulletTable, 0x0064B2E8);
SCBW_DATA(UNITDEATHS*, deathTable, 0x0058A364);
SCBW_DATA(UNITDEATHS*, unit_deaths, deathTable);
SCBW_DATA(Sprite*, spriteTable, 0x00629D98);

SCBW_DATA(UNITSCORES_I*, unitCountTable, 0x00584DE4);
SCBW_DATA(UNITSCORES_I*, completed_unit_counts, unitCountTable);


SCBW_DATA(UNITSCORES_I*, allUnitCountTable, 0x00582324);
SCBW_DATA(UNITSCORES_I*, all_unit_counts, unitCountTable);

SCBW_DATA(Thingy**, first_lone_sprite, 0x00654874);

//From locations.cpp
SCBW_DATA(const MapSize*, mapTileSize, 0x0057F1D4);
SCBW_DATA(Location*, locationTable, 0x0058DC60);

//for buttonsets
SCBW_DATA(BUTTON_SET*, buttonSetTable, 0x005187E8);

SCBW_DATA(ActionPointer*, actionTable,         0x00512800); // From triggers.cpp
SCBW_DATA(ActionPointer*, briefingActionTable, 0x00512800);
SCBW_DATA(char**,         chkStringTable,      0x005993D4);
SCBW_DATA(void*,          mainWindow,          0x0051BFB0);
SCBW_DATA(s32*,           musicVolume,         0x006D5BB4);
SCBW_DATA(b8*,            musicPlaying,        0x006D5BBC);

//This was in the InitializePlugin() code in qdp.cpp.
//Not sure what this does, so I'm leaving it as a comment.
//actionTable[59] = &EnableDebugModeAction;

//////////////////////////////////////////////////////////////// @}

/// @name Data Variables and Constants
//////////////////////////////////////////////////////////////// @{

//From resources.cpp
struct Resources {
	s32 minerals[12];
	s32 gas[12];
	s32 cumulativeGas[12];
	s32 cumulativeMinerals[12];
};
SCBW_DATA(Resources*, resources, 0x0057F0F0);

struct cv5 {
	u16 index;
	/*
	u8 unk : 4;
	u8 flags : 4;
	u8 ground_height_old : 4;
	u8 unk2 : 4;*/
	u16 flags;
	u16 left_edge;
	u16 top_edge;
	u16 right_edge;
	u16 bottom_edge;
	u16 unk3;
	u16 unk4;
	u16 unk5;
	u16 unk6;
	u16 megatiles[16];
};

SCBW_DATA(cv5**, tileGroup, 0x006D5EC8);

/// TBL files
SCBW_DATA(const u16*,		statTxtTbl,		0x006D1238);
SCBW_DATA(const StringTbl*, mapStringTbl,	0x005993D4);

SCBW_DATA(const char*, StringEmpty, 0x00501B7D);

/// Units that are selected by the current player (or the player viewing the replay).
struct UnitsSel {
	Unit* unit[SELECTION_ARRAY_LENGTH];
};
SCBW_DATA(UnitsSel*, clientSelectionGroup, 0x00597208);
SCBW_DATA(Unit*, clientSelectionGroupEnd, 0x00597238);
SCBW_DATA(u8*, clientSelectionCount, 0x0059723D);

/// Screen position relative to the map
SCBW_DATA(s32*, screenX, 0x00628448);
SCBW_DATA(s32*, screenY, 0x00628470);

///To use with a player ID
SCBW_DATA(Point16*, startPositions, 0x0058D720);

/// Mouse position relative to the screen
SCBW_DATA(const Vector32*, mouse, 0x006CDDC4);

/// Booleans indicating which keyboard buttons are held

//use IS_KEYCODE_USED[VK_param] where VK_param is one of the
//Virtual-Key Codes of Windows as seen in msdn here:
//https://msdn.microsoft.com/fr-fr/library/windows/desktop/dd375731%28v=vs.85%29.aspx
SCBW_DATA(const b8*, IS_KEYCODE_USED, 0x00596A18);

SCBW_DATA(const b8*, IS_HOLDING_SHIFT, 0x00596A28);
SCBW_DATA(const b8*, IS_HOLDING_CTRL, 0x00596A29);
SCBW_DATA(const b8*, IS_HOLDING_ALT, 0x00596A2A);

/// Maybe used by Map triggers to init a progressive movement toward the corresponding location, see GameImpl::setScreenPosition of BWAPI
SCBW_DATA(u32*, MoveToX, 0x0062848C);
SCBW_DATA(u32*, MoveToY, 0x006284A8);

/// Contains various information (names, player types, race types, and associated forces) of each player in the current game
SCBW_DATA(Player*, playerTable, 0x0057EEE0);

struct SupplyData {
	u32 provided[PLAYER_COUNT];
	u32 used[PLAYER_COUNT];
	u32 max[PLAYER_COUNT];
};
SCBW_DATA(SupplyData*, raceSupply, 0x00582144);  //Array; Use Unit::getRace() to get the index.

SCBW_DATA(const u32*, elapsedTimeFrames, 0x0057F23C);  //Elapsed game time in frames
SCBW_DATA(const u32*, CountdownTimer, 0x0058D6F4);  //Countdown Timer (in seconds) (From BWAPI Offsets.h)
SCBW_DATA(const u32*, elapsedTimeSeconds, 0x0058D6F8);  //Elapsed game time in seconds
SCBW_DATA(const u8*, GAME_TYPE, 0x00596820);  //Part of a larger structure; Compare with GameType::Enum.
SCBW_DATA(const s32*, ACTIVE_NATION_ID, 0x00512678);  //AKA g_ActiveNationID
SCBW_DATA(const s32*, ACTIVE_PLAYER_ID, 0x0051267C);  //Used by GetActivePlayerNextSelectionFunc
SCBW_DATA(		s32*, LOCAL_NATION_ID, 0x00512684);  //AKA g_LocalNationID; Actually stores the player ID.
SCBW_DATA(const s32*, LOCAL_HUMAN_ID, 0x00512688);  //AKA g_LocalHumanID; Invalid in replay games.
SCBW_DATA(const u8*, CURRENT_TILESET, 0x00596828);  //Tileset of current map. Compare with TilesetType::Enum
SCBW_DATA(const s32*, provider_MaxBuffer, 0x0057F0D8);
SCBW_DATA(const u32*, provider_LatencyCalls, 0x0057F090);
SCBW_DATA(u32*, sgdwBytesInCmdQueue, 0x00654AA0);
SCBW_DATA(const char*, TurnBuffer, 0x00654880);
////possible values were 0057F1DC and [00596828]

// Added from BWAPI Offsets.h, but made const since it would not be safe to modify those
SCBW_DATA(char*, CurrentMapFileName, 0x0057FD3C);
SCBW_DATA(const char*, CurrentMapName, 0x0057FE40);
SCBW_DATA(const _uavail*, UnitAvailability, 0x0057F27C);

// Ping Data
SCBW_DATA(Ping*, PingArray, 0x0059C1B8); // 16

// Added by Pr0nogo
// For raceDropdown
SCBW_DATA(bStringMap*,	singleRaceSelect,	0x0051AAA0);
SCBW_DATA(wStringMap*,	singleTypeSelect,	0x0051AAA8);
SCBW_DATA(const u16*,	networkTable,		0x006D1220);

// For initSingleplayerCustom
SCBW_DATA(const u32*, selectedGameType, 0x0059BA64);
SCBW_DATA(const u16*, selectedGameTypeParam, 0x0059BA5C);
SCBW_DATA(const char*, selectedSingleplayerRace, 0x0057F266);
SCBW_DATA(const char*, customSingleplayer, 0x0057F1E3);

//for lobotomy mine hook
SCBW_DATA(char**, statusscreen_infobtn, 0x00519F40);
SCBW_DATA(GrpHead**, cmdicons_grp, 0x0068C1E0);

// For initGameLoad
SCBW_DATA(void*, scenarioChk, 0x006D0F24);
SCBW_DATA(const ForceFlags*, playerForceFlags, 0x0058D5B8); // array of 0x4
SCBW_DATA(const u8*, playerForce, 0x0059BDA8); // array of 0x8
SCBW_DATA(const u8*, playerForceSomethingReplay, 0x006D11A1); // array of 0x8
SCBW_DATA(const FILE*, loadGameFileHandle, 0x006D1218);
SCBW_DATA(const u8*, multiPlayerMode, 0x0057F0B4);
SCBW_DATA(const u32*, initialSeed, 0x0051CA10);
SCBW_DATA(const u32*, replaySeed, 0x006D0F38);
//SCBW_DATA(const func*,			mouseOverCallback,			0x006D5EAC); // how to define a func?
SCBW_DATA(u32*, randomCounts, 0x0051C610); // size of 0x400, array?
SCBW_DATA(u32*, randomCountsTotal, 0x0051CA18);
SCBW_DATA(u16*, campaignIndex, 0x0057F244);
SCBW_DATA(void**, mapArchiveHandle, 0x00596BC4);
SCBW_DATA(const GameData*, gameData, 0x005967F8);
SCBW_DATA(const u8*, mapStarted, 0x006D121C);
SCBW_DATA(const u8*, playerHasLeft, 0x00581D62); // array of 0x8
SCBW_DATA(const u8*, lossType, 0x00581D61);
SCBW_DATA(const u8*, unkVictoryVariable, 0x00581D66);
SCBW_DATA(const u32*, saveLoadSuccess, 0x006D5A68);
SCBW_DATA(u32*, elapstedTimeModifier, 0x006D5BCC);
SCBW_DATA(const u32*, savedElapsedSeconds, 0x0057F240);
SCBW_DATA(		Options*, optionsList, 0x006CDFD4);
SCBW_DATA(const ReplayData*, replayData, 0x00596BBC);
SCBW_DATA(const u32*, replayVisions, 0x006D0F18);
SCBW_DATA(const u32*, playerVisions, 0x0057F0B0);
SCBW_DATA(const u32*, replayShowEntireMap, 0x006D0F1C); // dword bool...?
SCBW_DATA(const u32*, nextReplayCommandFrame, 0x005136C8);
SCBW_DATA(const u32*, playerExploredVisions, 0x0057EEB8);
SCBW_DATA(s32*, subTwoCheckTwo, 0x004EE0F0);

// For getResponseSfx
SCBW_DATA(u32*, selectionSoundCounter,			0x00640548);
SCBW_DATA(u32*, lastRandSFX,					0x006D5BE0);

// For loadRaceUI
SCBW_DATA(u8*, consoleIndex,					0x0057F1E2);
SCBW_DATA(u16*, currentMusicId,					0x0057F1DE);
SCBW_DATA(const char*, xCampaignFirstMission,	0x005122C0);
SCBW_DATA(const char*, campaignFirstMission,	0x005122BC);
SCBW_DATA(u32*, consoleRaceSpecific,			0x0051A3B4);

// For loadGameCreate
SCBW_DATA(u32*, isHost,					0x00596888);
SCBW_DATA(u8*, outOfGame,				0x006D5A10);
SCBW_DATA(const u32*, scMainState,		0x00596904);
SCBW_DATA(u8*, gameActive,				0x006D11EC);
SCBW_DATA(u16*, nextSCMainState,		0x0051CE90);
SCBW_DATA(u32*, replayEndFrame,			0x006D0F31);

// For levelCheatCreate
SCBW_DATA(u32*, levelCheatUnlocked,		0x006D11C0);
SCBW_DATA(const char*, playerName,		0x0057EEB5);

// For update_placement_state_for_placement_box
SCBW_DATA(u8*, placingPlayer,			0x0064088C);
SCBW_DATA(u16*, placeBuildingX,			0x00640890);
SCBW_DATA(u16*, placeBuildingY,			0x00640892);
SCBW_DATA(u32*, isPlacingAddon,			0x00640884);
SCBW_DATA(u16*, placeBuildingUnitType,	0x0064088A);
SCBW_DATA(u32*, placeBuildBaseType,		0x00640888);

SCBW_DATA(u16*, moveToTileX,			0x0057F1D0);
SCBW_DATA(u16*, moveToTileY,			0x0057F1D2);
SCBW_DATA(u32*, moveToX,				0x0062848C);
SCBW_DATA(u32*, moveToY,				0x006284A8);

// For increment_unit_scores_ex
SCBW_DATA(u32*, factoriesOwned,			0x005820B4); // Array of 12
SCBW_DATA(u32*, unitsProduced,			0x00581E14); // Array of 12
SCBW_DATA(u32*, structuresBuilt,		0x00581F64); // Array of 12
SCBW_DATA(u32*, unitScore,				0x00581ED4); // Array of 12
SCBW_DATA(u32*, buildingScore,			0x00582024); // Array of 12
SCBW_DATA(u32*, unitsOwned,				0x00581E44); // Array of 12
SCBW_DATA(u32*, buildingsOwned,			0x00582024); // Array of 12

// For incrementUnitScores
SCBW_DATA(u32*, factoriesProducedOwnedTotal,	0x00582084); // Array of 12
SCBW_DATA(u32*, structuresProducedOwnedTotal,	0x00581F34); // Array of 12
SCBW_DATA(u32*, unitsProducedOwnedTotal,		0x00581DE4); // Array of 12

// For refreshScreen
SCBW_DATA(u32*, placeBuildingMessageId,			0x00640958);
SCBW_DATA(u16*, placeBuildingUnitId,			0x0064088A);

// For targetedOrder
SCBW_DATA(u32*, COrderCount,					0x00641698);

// For createUpgradeLevelTooltip
SCBW_DATA(u16*, upgradeLevelTooltip,			0x0068C1CC);
SCBW_DATA(u16**, statTxtBuffer,					0x006D1238);


struct BufferStr {
	char buf[260];
};

// For drawUnitInfo
SCBW_DATA(u16*, suppliesAvailableTemp,			0x006CA93C);
SCBW_DATA(u8*, statusScreenFunc,				0x0068C1E5);
SCBW_DATA(BufferStr*, struct_Char,				0x006CA9F8);

// For drawSupplyUnitInfo
SCBW_DATA(BufferStr*, struct_Char2,				0x006CAB08);
SCBW_DATA(char*, unkString_Char,				0x006CA668);

// For drawWireframeShield
SCBW_DATA(char*, charBuffer,					0x0068C208);
SCBW_DATA(char*, byteShieldStr,					0x0068C20A); // count of 22
SCBW_DATA(u8*, wireframeConstants,				0x00515280); // count of 40
SCBW_DATA(char*, imgDrawPaletteIndex,			0x0050CEC1); // count of 256
SCBW_DATA(u8*, off_50CE81,						0x0050CE81);
SCBW_DATA(u8*, off_50CE82,						0x0050CE82);

// For drawBuildingInfo
SCBW_DATA(s32*, selectionGroupHP,				0x006CA94C);
SCBW_DATA(s16*, userInterfaceEnergy,			0x006CAC0C);
SCBW_DATA(s32*, userInterfaceShields,			0x006CA9EC);
SCBW_DATA(char*, redrawTransportUI,				0x006CA9F0);

// For AI_EMPShieldRequirementsProc
SCBW_DATA(s32*, tempHPShields,					0x00695798);
SCBW_DATA(s32*, tempWeapon,						0x00695604);

// For drawImageHPBar
SCBW_DATA(unk_61*, struct_5140C4,				0x005140C4);
SCBW_DATA(unk_61*, struct_5140D8,				0x005140D8);
SCBW_DATA(unk_62*, struct_5140E8,				0x005140E8);

// For getUnitBulletDamage
SCBW_DATA(char*, upgradesLevelSC,				0x0058D2B0);
SCBW_DATA(char*, upgradesLevelBW,				0x0058F2FE);

// For findNextBounceTargetProc
SCBW_DATA(Unit**, bulletBounceMissileSource,	0x0064EECC);
SCBW_DATA(Unit**, bulletBounceMissileTarget,	0x0064EED0);
SCBW_DATA(Unit**, bulletBounceMissileNext,		0x0064EED4);

// For setUnderDisruptionWeb
SCBW_DATA(void**, unkArray_59CB64,			0x0059CB64);	// size of 252
SCBW_DATA(Unit*, unitReference,				0x0059CB58);

// For startGame
SCBW_DATA(s8*, gameState,					0x0066FBFA); // 0x4 Idling in lobby | 0x5 Game starting | 0x6 Slots locked | 0x7 Sending final commands | 0x8 Waiting for save data | 0x9 End lobby
SCBW_DATA(Player*, playerReplayWatchers,	0x00640970);

SCBW_DATA(s8*, countdownTimerInterval,		0x0066FBFC);
SCBW_DATA(s32*, countdownTimeTickCount,		0x0059CC7C);
SCBW_DATA(s32*, countdownTimeTickCount_0,	0x0059CC88);
SCBW_DATA(s32*, isInGame,					0x006D5BC8);
SCBW_DATA(s32*, countdownTimeRemaining,		0x0059CC78);

// For airSplash
SCBW_DATA(Unit**, airSplashList,			0x0064EEE0);	// size of 1700
SCBW_DATA(s32*, airSplashListCount,			0x0046DEB4);
SCBW_DATA(s32*, isInGroundSplashProc,		0x0064DEB8);

// For AI_AddMilitary
SCBW_DATA(RegionArray**,SAI_Paths,			0x006D5BFC);
SCBW_DATA(AiTown*,		AiTownArray,		0x006AA090);
SCBW_DATA(AiExpandList*,AiActiveTownArray,	0x006AA050); // sizeof 0x64[0x8]

// For CMDRECV_HoldPosition
SCBW_DATA(s16*, mapHeightPixels,			0x006284B4);
SCBW_DATA(s32*, orderType4Position,			0x006D5C20);
SCBW_DATA(s32*, orderType4Target,			0x006D5C24);

// For openMapFile
SCBW_DATA(char*, mapsFolder,				0x0066FC08);
SCBW_DATA(s32*, dword_5994DC,				0x005994DC);

// For loadMap
SCBW_DATA(scenarioFormat*, stru_500574,		0x00500574);
SCBW_DATA(scenarioFormat*, stru_50057C,		0x0050057C);
SCBW_DATA(s32*, scenarioCHKSize,			0x006D0F20);

// For gameLoop
SCBW_DATA(s32*, visionUpdateCount,			0x0051CE98);
SCBW_DATA(s8*, visionUpdated,				0x0051CE9C);

// X405: For displayEstablishing Shot
SCBW_DATA(CampaignMission*, campaignMissions, 0x004FFF60);

// For statsWeaponLevel / statsArmorLevel
SCBW_DATA(s16*, unkUpgradeCount,			0x006CA940);

// For isUnitCloaked
SCBW_DATA(Unit**,	firstInvisibleUnit,		0x0063FF5C);
SCBW_DATA(s32*,	forceButtonRefresh,			0x0068C1B0);
SCBW_DATA(s8*,	portraitRedraw,				0x0068AC74); // sizeof 4
SCBW_DATA(s8*,	statusScreenRedraw,			0x0068C1F8);
SCBW_DATA(s32*,	ctrlUnderMouse,				0x0068C1E8);
SCBW_DATA(s32*,	ctrlUnderMouseValue,		0x0068C1EC); // void* in Neiv's list

// For updateUnits
SCBW_DATA(s32*,		interceptorFocusedThisFrame,		0x006957D0);
SCBW_DATA(s8*,		isTargeting,						0x00641694);
SCBW_DATA(s32*,		orderTimerReassignCount,			0x0059CCA4);
SCBW_DATA(s32*,		secondaryOrderTimerReassignCount,	0x006283E8);
SCBW_DATA(Unit**,	activeIscriptEntity,				0x006D11F4);
SCBW_DATA(Bullet**,	activeIscriptBullet,				0x006D11F8);
SCBW_DATA(Unit**,	activeIscriptUnit,					0x006D11FC);
SCBW_DATA(Unit**,	firstDyingUnit,						0x0062842C);
SCBW_DATA(Unit**,	lurkerSplashList,					0x0064DEC8); // sizeof 400
SCBW_DATA(s32*,		lurkerSplashListIndex,				0x0064EEC8);
SCBW_DATA(s32*,		lastBulletIndex,					0x0064DEA8);

// For orderAllowed
SCBW_DATA(s32*,	lastInternalError,					0x0066FF60);
SCBW_DATA(s16*, ordersRequirementTree,				0x00514CF8);

// For larva_creep_spawn
SCBW_DATA(s16*, larvaCounter,						0x0067D3F8);

// For updateBuildingPlacementState_Units
SCBW_DATA(s8*, buildingPlacementState,				0x006408F8);

// For progressUnitFrame
SCBW_DATA(s8*, selectionCircleColor,				0x00581D6A);

// For initializeBulet
SCBW_DATA(Unit**, lastBulletCreator,				0x0064DEB0);
SCBW_DATA(s32*, launchSpinRandom,					0x0064DEC0);
SCBW_DATA(Point16*, attackCurve,					0x00513650);

// For AI_ProgressBuild
SCBW_DATA(s32*,	gameMinerals,						0x0057F0F0);
SCBW_DATA(s32*,	gameGas,							0x0057F120);
SCBW_DATA(s32*, AIMineralsReserve,					0x006CA51C);
SCBW_DATA(s32*, AIGasReserve,						0x006CA4EC);

// For updateBriefingRoomObjectives
SCBW_DATA(BinDlg**, readyRoomDialog,				0x0066FF6C);
SCBW_DATA(s32*, missionObjectivesStringIndex,		0x0058D6C4);

// For AIManageBuildingPlacement
SCBW_DATA(Unit**, disabledUnitArray,				0x00692660);
SCBW_DATA(Unit**, AIBuilderUnknown,					0x006D5DCC);

// For getSuppliesAvailable
SCBW_DATA(s32*,	suppliesAvailable_Protoss,			0x00582264);
SCBW_DATA(s32*,	suppliesMax_Protoss,				0x005822C4);
SCBW_DATA(s32*,	suppliesAvailable_Terran,			0x005821D4);
SCBW_DATA(s32*,	suppliesMax_Terran,					0x00582234);
SCBW_DATA(s32*,	suppliesAvailable_Zerg,				0x00582144);
SCBW_DATA(s32*,	suppliesMax_Zerg,					0x005821A4);

// For getSuppliesUsed
SCBW_DATA(s32*,	suppliesUsed_Protoss,				0x00582294);
SCBW_DATA(s32*,	suppliesUsed_Terran,				0x00582204);
SCBW_DATA(s32*,	suppliesUsed_Zerg,					0x00582174);

// For initializeUnitCounts
SCBW_DATA(s32*,	unitCountScore,						0x00581DE4);
SCBW_DATA(s8*,	unknown_57F41CB,					0x0057F1CB);

// For StatRes_MainProc
SCBW_DATA(s32*,	unknown_player_5153F8,				0x005153F8);
SCBW_DATA(s32*,	unknown_68C22C,						0x0068C22C);
SCBW_DATA(s32*,	unknown_68C230,						0x0068C230);

// For getSuppliesTemp
SCBW_DATA(s16*,	suppliesUsedTemp,					0x006CA93E);

// For drawSupplyUnitStatus
SCBW_DATA(char*, supply_available_buffer,			0x006CA668);
SCBW_DATA(char*, supply_used_buffer,				0x006CA9F8);
SCBW_DATA(char*, supply_max_buffer,					0x006CAB08);
SCBW_DATA(char*, supply_provided_buffer,			0x006CAC78);

SCBW_DATA(char*, kills_buffer,						0x006CAC78);

// For giveUnitToPlayer
SCBW_DATA(s32*,	currentTriggerPlayerId,				0x006509B0);

// For delete_ai_town_if_needed
SCBW_DATA(ResourceArea*,	baseArray,				0x00692688);

// For get_bunker_placement_position
SCBW_DATA(u32*,		regionIdsCount,					0x0069265C);
SCBW_DATA(u16*,		regionIds,						0x0069262C);

SCBW_DATA(u8*,		unit_iterator_pos,				0x006284B6);
SCBW_DATA(u8*,		byte_68C300,					0x0068C300);
SCBW_DATA(u32**,	activeTileHandle,				0x006D1260);

SCBW_DATA(void**,	GetRightClickOrderArray,		0x005153FC);

// For prepare_unit_move_custom
SCBW_DATA(BinDlg*,	portrait,						0x0068AC98);

// For fog of war changes by DarkenedFantasies
SCBW_DATA(FogMask*,	fog_of_war_mask,				0x00657AA0);

// For score screen player name overrides, from DarkenedFantasies
SCBW_DATA(const StringTbl*,	network_table_DF,		0x006D1220);
SCBW_DATA(ColorTable*,		colorTable,				0x0058F442);
SCBW_DATA(PlayerNames*,		playerNameList,			0x0059B3D8);

// For soundThread
/*
SCBW_DATA(HANDLE*,	soundThreadEvent,				0x006D5A00);
SCBW_DATA(s32*,		unkSoundCore,					0x006D5A08);
SCBW_DATA(s32*,		soundVolume,					0x00599900);
SCBW_DATA(s8*,		soundData,						0x006D1270);
SCBW_DATA(s32*,		volumeReduction,				0x006D5A0C);
SCBW_DATA(DSound*,	directSoundInterface,			0x006D59F4);
SCBW_DATA(s32*,		unkSoundCore2,					0x005999C0);
SCBW_DATA(s32*,		currentSoundMemory,				0x006D59FC);
SCBW_DATA(CritSect*,soundLock,						0x006D5F4C);

// For soundInit
SCBW_DATA(s32*,		soundChannels,					0x005998F0);
SCBW_DATA(HMODULE*,	hdSoundModule,					0x006D59F0);
SCBW_DATA(s8*,		ddaInitialized,					0x006D1265);
SCBW_DATA(s8*,		vidInitialized,					0x006D1266);
SCBW_DATA(s32*,		soundMemoryCache,				0x005129B8);
SCBW_DATA(HANDLE*,	soundHandle,					0x006D5A04);
SCBW_DATA(s32*,		unkSoundCore3,					0x005998E4);
SCBW_DATA(s8*,		unkSoundBoolCore,				0x006D1264);
*/

// For handle_window_messages
//typedef void(__fastcall* input_procs_func[19])(s32 arg);
//SCBW_DATA(input_procs_func,	input_procs,	0x005968A0);

// For get_standard_attack_target
SCBW_DATA(u32*,		min_range,					0x006955B8); // sizeof 10
SCBW_DATA(s32*,		max_range_in_px,			0x006957A4);

// For show_right_click_error_if_needed
SCBW_DATA(u32*,		use_quiet_sounds,			0x0064086C);
SCBW_DATA(char*,	draw_out_text_left,			0x00641598);
SCBW_DATA(char*,	text_bytes,					0x00641674); // sizeof 13
SCBW_DATA(s32*,		unit_stat_update_timer,		0x00640B54);

// For CMDACT_game_speed
SCBW_DATA(s32*,		last_control_index,			0x006D051C);
SCBW_DATA(s32*,		cpu_throttle,				0x006D5C00);
SCBW_DATA(s32*,		temp_cpu_throttle,			0x00655C3C);
SCBW_DATA(char*,	bin_interact,				0x006D1224);
SCBW_DATA(void**,	last_bindlg_menu_function,	0x006D1234);

//////////////////////////////////////////////////////////////// @}

/// @name *.DAT File Data
//////////////////////////////////////////////////////////////// @{

// The following code is based on BWAPI's BW/Offsets.h
// you can get the other addresses in http://code.google.com/p/bwapi/source/browse/trunk/bwapi/BWAPI/Source/BW/Offsets.h
// Since StarCraft is already running when the plugin is loaded, the DatLoad
// structs are guaranteed to have the corrent offsets.

struct DatLoad {
	u32 address;
	u32 length;
	u32 entries;
};

namespace units_dat {
	SCBW_DATA(const DatLoad*, unitsDat, 0x00513C30);

	SCBW_DATA(u8*, Graphic, unitsDat[0].address);							// 006644F8
	SCBW_DATA(u16*, SubUnit, unitsDat[1].address);							// 006607C0
	//SCBW_DATA(u16*,		???,					unitsDat[2].address);	// 00660C38 // Subunit 2 according to EUDDB
	SCBW_DATA(u16*, InfestedUnitPartial, unitsDat[3].address);				// 00664980 // (Id - UnitId::TerranCommandCenter) for it to work, last valid id is UnitId::Special_OvermindCocoon
	SCBW_DATA(u32*, ConstructionGraphic, unitsDat[4].address);				// 006610B0
	SCBW_DATA(u8*, StartDirection, unitsDat[5].address);					// 006605F0 // Direction unit will face after it is created. From 0 (top) to 31 (before top turning clockwise). 32 means random
	SCBW_DATA(u8*, ShieldsEnabled, unitsDat[6].address);					// 006647B0
	SCBW_DATA(u16*, MaxShieldPoints, unitsDat[7].address);					// 00660E00
	SCBW_DATA(s32*, MaxHitPoints, unitsDat[8].address);						// 00662350
	SCBW_DATA(u8*, Elevation, unitsDat[9].address);							// 00663150
	SCBW_DATA(u8*, MovementFlags, unitsDat[10].address);					// 00660FC8
	SCBW_DATA(u8*, Rank, unitsDat[11].address);								// 00663DD0
	SCBW_DATA(u8*, ComputerIdleOrder, unitsDat[12].address);				// 00662EA0
	SCBW_DATA(u8*, HumanIdleOrder, unitsDat[13].address);					// 00662268
	SCBW_DATA(u8*, ReturnToIdleOrder, unitsDat[14].address);				// 00664898
	SCBW_DATA(u8*, AttackUnitOrder, unitsDat[15].address);					// 00663320
	SCBW_DATA(u8*, AttackMoveOrder, unitsDat[16].address);					// 00663A50
	SCBW_DATA(u8*, GroundWeapon, unitsDat[17].address);						// 006636B8
	SCBW_DATA(u8*, MaxGroundHits, unitsDat[18].address);					// 006645E0
	SCBW_DATA(u8*, AirWeapon, unitsDat[19].address);						// 006616E0
	SCBW_DATA(u8*, MaxAirHits, unitsDat[20].address);						// 0065FC18
	SCBW_DATA(u8*, ComputerAIInternal, unitsDat[21].address);				// 00660178 // "Computer AI Unit Type" according to EUDDB
	SCBW_DATA(u32*, BaseProperty, unitsDat[22].address);					// 00664080 // AKA SpecialAbilityFlags, UnitProperty, etc. Use with UnitProperty::Enum.
	SCBW_DATA(u8*, SeekRange, unitsDat[23].address);						// 00662DB8 // AKA Target Acquisition Range, Acquire Target Range, etc.
	SCBW_DATA(u8*, SightRange, unitsDat[24].address);						// 00663238
	SCBW_DATA(u8*, ArmorUpgrade, unitsDat[25].address);						// 006635D0
	SCBW_DATA(u8*, SizeType, unitsDat[26].address);							// 00662180
	SCBW_DATA(u8*, ArmorAmount, unitsDat[27].address);						// 0065FEC8
	SCBW_DATA(u8*, RightClickAction, unitsDat[28].address);					// 00662098
	SCBW_DATA(u16*, SoundReady, unitsDat[29].address);						// 00661FC0
	SCBW_DATA(u16*, SoundWhatFirst, unitsDat[30].address);					// 0065FFB0
	SCBW_DATA(u16*, SoundWhatLast, unitsDat[31].address);					// 00662BF0
	SCBW_DATA(u16*, SoundAnnoyedFirst, unitsDat[32].address);				// 00663B38
	SCBW_DATA(u16*, SoundAnnoyedLast, unitsDat[33].address);				// 00661EE8
	SCBW_DATA(u16*, SoundYesFirst, unitsDat[34].address);					// 00663C10
	SCBW_DATA(u16*, SoundYesLast, unitsDat[35].address);					// 00661440
	SCBW_DATA(Point16*, BuildingDimensions, unitsDat[36].address);			// 00662860	// Placement Box in PyDAT
	SCBW_DATA(Point16*, AddonPlacement, unitsDat[37].address);				// 006626E0
	SCBW_DATA(Box16*, UnitBounds, unitsDat[38].address);					// 006617C8	// Unit Dimensions in PyDAT
	//SCBW_DATA(u16*,		???,					unitsDat[39].address);	// 00662F88 // Portrait according to EUDDB
	SCBW_DATA(u16*, MineralCost, unitsDat[40].address);						// 00663888
	SCBW_DATA(u16*, GasCost, unitsDat[41].address);							// 0065FD00
	SCBW_DATA(u16*, TimeCost, unitsDat[42].address);						// 00660428
	SCBW_DATA(u16*, unitsDat43, unitsDat[43].address);						// 00660A70 // "Restriction Address" according to EUDDB
	SCBW_DATA(GroupFlag*, GroupFlags, unitsDat[44].address);				// 006637A0
	SCBW_DATA(u8*, SupplyProvided, unitsDat[45].address);					// 006646C8 // Supply shown in-game is (true supply) / 2; e.g. Zerglings have 1 true supply, Marines have 2 true supply, etc.
	SCBW_DATA(u8*, SupplyRequired, unitsDat[46].address);					// 00663CE8
	SCBW_DATA(u8*, SpaceRequired, unitsDat[47].address);					// 00664410
	SCBW_DATA(u8*, SpaceProvided, unitsDat[48].address);					// 00660988
	SCBW_DATA(u16*, BuildScore, unitsDat[49].address);						// 00663408
	SCBW_DATA(u16*, DestroyScore, unitsDat[50].address);					// 00663EB8
	SCBW_DATA(u16*, MapStringId, unitsDat[51].address);						// 00660260
	SCBW_DATA(u8*, BroodwarOnly, unitsDat[52].address);						// 006606D8
	SCBW_DATA(u16*, StarEditAvailFlags, unitsDat[53].address);				// 00661518 // "Staredit Availability Flags" according to EUDDB
} // (last entry at 0x00513EAC)

namespace flingy_dat {
	SCBW_DATA(const DatLoad*, flingyDat, 0x00515A38);

	SCBW_DATA(u16*,	SpriteID,			flingyDat[0].address);	// 006CA318
	SCBW_DATA(u32*,	TopSpeed,			flingyDat[1].address);	// 006C9EF8
	SCBW_DATA(u16*,	Acceleration,		flingyDat[2].address);	// 006C9C78
	SCBW_DATA(u32*,	HaltDistance,		flingyDat[3].address);	// 006C9930
	SCBW_DATA(u8*,	TurnSpeed,			flingyDat[4].address);	// 006C9E20 Incorrectly known as "Turn Radius"
//	SCBW_DATA(u8*,	???,				flingyDat[5].address);	// 006CA240
	SCBW_DATA(u8*,	MovementControl,	flingyDat[6].address);	// 006C9858
} // (last entry at 0x00515A80)

// incorrect, must be clarified

namespace sfx_dat {
	SCBW_DATA(const	DatLoad*,			sfxDat,		0x00515498);
	
	SCBW_DATA(u32*,	SoundPtr,			sfxDat[0].address);	// 0068DAA0
	SCBW_DATA(u8*,	Priority,			sfxDat[1].address);	// 0068D628
	SCBW_DATA(u8*,	Flags,				sfxDat[2].address);	// 0068C8C0
	SCBW_DATA(u16*,	PortraitDuration,	sfxDat[3].address);	// 0068CD38
	SCBW_DATA(u8*,	Volume,				sfxDat[4].address);	// 0068C448
}

/*
namespace weapons_dat {
	SCBW_DATA(const DatLoad*, weaponsDat, 0x00513868);

	SCBW_DATA(u16*, Label, weaponsDat[0].address);  // 006572E0
	SCBW_DATA(u32*, FlingyId, weaponsDat[1].address);  // 00656CA8
	SCBW_DATA(u8*, SpecialAttack, weaponsDat[2].address);  // 006573E8 //"Special Attack" according to EUDDB
	SCBW_DATA(TargetFlag*, TargetFlags, weaponsDat[3].address);  // 00657998
	SCBW_DATA(u32*, MinRange, weaponsDat[4].address);  // 00656A18
	SCBW_DATA(u32*, MaxRange, weaponsDat[5].address);  // 00657470
	SCBW_DATA(u8*, DamageUpgrade, weaponsDat[6].address);  // 006571D0
	SCBW_DATA(u8*, DamageType, weaponsDat[7].address);  // 00657258 Normal, Explosive, etc.. Use with DamageType::Enum.
	SCBW_DATA(u8*, Behavior, weaponsDat[8].address);  // 00656670 Fly and follow target, appear on target unit, etc. Use with WeaponBehavior::Enum.
	SCBW_DATA(u8*, RemoveAfter, weaponsDat[9].address);  // 00657040
	SCBW_DATA(u8*, ExplosionType, weaponsDat[10].address); // 006566F8 NormalHit, RadialSplash, EnemySplash, etc. Use with WeaponEffect::Enum.
	SCBW_DATA(u16*, InnerSplashRadius, weaponsDat[11].address); // 00656888
	SCBW_DATA(u16*, MediumSplashRadius, weaponsDat[12].address); // 006570C8
	SCBW_DATA(u16*, OuterSplashRadius, weaponsDat[13].address); // 00657780
	SCBW_DATA(u16*, DamageAmount, weaponsDat[14].address); // 00656EB0
	SCBW_DATA(u16*, DamageBonus, weaponsDat[15].address); // 00657678
	SCBW_DATA(u8*, Cooldown, weaponsDat[16].address); // 00656FB8
	SCBW_DATA(u8*, DamageFactor, weaponsDat[17].address); // 006564E0
	SCBW_DATA(u8*, AttackAngle, weaponsDat[18].address); // 00656990
	SCBW_DATA(u8*, LaunchSpin, weaponsDat[19].address); // 00657888
	SCBW_DATA(u8*, ForwardOffset, weaponsDat[20].address); // 00657910
	SCBW_DATA(u8*, VerticalOffset, weaponsDat[21].address); // 00656C20
	SCBW_DATA(u16*, ErrorMessage, weaponsDat[22].address); // 00656568 //Target Error Message according to EUDDB
	SCBW_DATA(u16*, Icon, weaponsDat[23].address); // 00656780 //Icon according to EUDDB
} //weapons_dat (last entry at 0x0051397C)
*/

#define MAX_WEAPONS 256
namespace weapons_dat
{
	extern	u16			Label[MAX_WEAPONS];			
	extern	u32			FlingyId[MAX_WEAPONS];			
	extern	u8			SpecialAttack[MAX_WEAPONS];	
	extern	TargetFlag	TargetFlags[MAX_WEAPONS];		
	extern	u32			MinRange[MAX_WEAPONS];			
	extern	u32			MaxRange[MAX_WEAPONS];			
	extern	u8			DamageUpgrade[MAX_WEAPONS];		
	extern	u8			DamageType[MAX_WEAPONS];			
	extern	u8			Behavior[MAX_WEAPONS];			
	extern	u8			RemoveAfter[MAX_WEAPONS];		
	extern	u8			ExplosionType[MAX_WEAPONS];		
	extern	u16			InnerSplashRadius[MAX_WEAPONS];	
	extern	u16			MediumSplashRadius[MAX_WEAPONS]; 
	extern	u16			OuterSplashRadius[MAX_WEAPONS];	
	extern	u16			DamageAmount[MAX_WEAPONS];		
	extern	u16			DamageBonus[MAX_WEAPONS];		
	extern	u8			Cooldown[MAX_WEAPONS];			
	extern	u8			DamageFactor[MAX_WEAPONS];		
	extern	u8			AttackAngle[MAX_WEAPONS];		
	extern	u8			LaunchSpin[MAX_WEAPONS];			
	extern	u8			ForwardOffset[MAX_WEAPONS];		
	extern	u8			VerticalOffset[MAX_WEAPONS];		
	extern	u16			ErrorMessage[MAX_WEAPONS];		
	extern	u16			Icon[MAX_WEAPONS];
}

namespace upgrades_dat {
	SCBW_DATA(const DatLoad*, upgradesDat, 0x005136E0);

	SCBW_DATA(u16*, MineralCostBase, upgradesDat[0].address);	//00656740
	SCBW_DATA(u16*, MineralCostFactor, upgradesDat[1].address);	//006559C0
	SCBW_DATA(u16*, GasCostBase, upgradesDat[2].address);	//00655840
	SCBW_DATA(u16*, GasCostFactor, upgradesDat[3].address);	//006557C0
	SCBW_DATA(u16*, TimeCostBase, upgradesDat[4].address);	//00655B80
	SCBW_DATA(u16*, TimeCostFactor, upgradesDat[5].address);	//00655940
	//SCBW_DATA(u16*, ???,              upgradesDat[6].address);	//006558C0	//Restriction Flags according to EUDDB
	SCBW_DATA(u16*, UpgradeIcon, upgradesDat[7].address);	//00655AC0	//"Icon, cmdicons.grp frame" according to EUDDB
	SCBW_DATA(u16*, Label, upgradesDat[8].address);	//00655A40
	SCBW_DATA(u8*, Race, upgradesDat[9].address);	//00655BFC
	SCBW_DATA(u8*, MaxRepeats, upgradesDat[10].address);	//00655700
	//SCBW_DATA(u8*, ???,				upgradesDat[11].address);	//00655B3C	//Brood War Flag according to EUDDB
} //upgrades_dat (last entry at 0x00513764)

namespace techdata_dat {
	SCBW_DATA(const DatLoad*, techdataDat, 0x005137D8);

	SCBW_DATA(u16*, MineralCost, techdataDat[0].address);	//00656248
	SCBW_DATA(u16*, GasCost, techdataDat[1].address);	//006561F0
	SCBW_DATA(u16*, TimeCost, techdataDat[2].address);	//006563D8
	SCBW_DATA(u16*, EnergyCost, techdataDat[3].address);	//00656380
	//SCBW_DATA(u16*, ???,				techdataDat[4].address);	//00656198
	//SCBW_DATA(u16*, ???,				techdataDat[5].address);	//006562F8	//Restriction Flags according to EUDDB
	//SCBW_DATA(u16*, ???,				techdataDat[6].address);	//00656430	//"Icon, cmdicons.grp frame" according to EUDDB
	SCBW_DATA(u16*, Label, techdataDat[7].address);	//006562A0
	//SCBW_DATA(u8*, ???,				techdataDat[8].address);	//00656488	//Race according to EUDDB
	//SCBW_DATA(u8*, ???,				techdataDat[9].address);	//00656350	//"Researched" according to EUDDB
	//SCBW_DATA(u8*, ???,				techdataDat[10].address);	//006564B4	//Brood War Flag according to EUDDB
} //techdata_dat (last entry at 0x00513850)

namespace orders_dat {
	SCBW_DATA(const DatLoad*, ordersDat, 0x00513EC8);

	SCBW_DATA(u16*, Label, ordersDat[0].address);	//00665280
	SCBW_DATA(u8*, UseWeaponTargeting, ordersDat[1].address);	//00664B00
	//SCBW_DATA(u8*,	???,				ordersDat[2].address);	//00665940 //May be "secondary order" used for debug/assert only / "Order goes in bSecondaryOrderID instead of bMainOrderID." according to EUDDB
	SCBW_DATA(u8*, Unused3, ordersDat[3].address);	//00665A00 //May be "Non-subunit" used for debug only
	//SCBW_DATA(u8*,	???,				ordersDat[4].address);	//00664A40
	//SCBW_DATA(u8*,	???,				ordersDat[5].address);	//006657C0 //May be "Subunit can use" used for debug only
	SCBW_DATA(b8*, CanBeInterrupted, ordersDat[6].address);	//00665040
	SCBW_DATA(u8*,	Unknown7, ordersDat[7].address); //00665100
	SCBW_DATA(u8*, CanBeQueued, ordersDat[8].address);	//00665700 //"Can Be Queued" according to EUDDB
	SCBW_DATA(u8*,	DisablingKeepsTarget, ordersDat[9].address);	//006651C0
	SCBW_DATA(u8*, CanBeObstructed, ordersDat[10].address);	//006654C0 //"Can Be Obstructed?" according to EUDDB
	//SCBW_DATA(u8*,	???,				ordersDat[11].address);	//00664C80
	//SCBW_DATA(u8*,	???,				ordersDat[12].address);	//00664BC0 ////May be "Order requires moving"
	SCBW_DATA(u8*, OrderWeaponId, ordersDat[13].address);	//00665880
	SCBW_DATA(u8*, TechUsed, ordersDat[14].address);	//00664E00
	SCBW_DATA(u8*, OrderIscriptAnim, ordersDat[15].address);	//00664D40 //Animation (IscriptAnimation) according to EUDDB
	SCBW_DATA(u16*, ButtonIcon, ordersDat[16].address);	//00664EC0
	SCBW_DATA(u16*, RequirementsOffset, ordersDat[17].address);	//00665580 //based on context
	//SCBW_DATA(u8*,	???,				ordersDat[18].address);	//00665400 //"Obscured Order" according to EUDDB
} //orders_dat (last entry at 0x00513FA0)

namespace sprites_dat {
	SCBW_DATA(const DatLoad*, spritesDat, 0x00513FB8);

	SCBW_DATA(u16*, ImageId, spritesDat[0].address);		//00666160
	SCBW_DATA(s8*, HpBarSize, spritesDat[1].address);		//00665E50
	//SCBW_DATA(s8*,  ???,				spritesDat[2].address);		//00666570
	SCBW_DATA(u8*, IsVisible, spritesDat[3].address);		//00665C48
	//SCBW_DATA(s8*,  ???,				spritesDat[4].address);		//00665AC0 //"Selection Circle" according to EUDDB, not starting at 0
	//SCBW_DATA(s8*,  ???,				spritesDat[5].address);		//00665FD8 //"Selection Vertical Offset" according to EUDDB
} //sprites_dat (last entry at 0x00513FF4)

namespace images_dat {
	SCBW_DATA(const DatLoad*, imagesDat, 0x00514010);

	//SCBW_DATA(const u32*,  ???,		imagesDat[0].address);	//00668AA0 //"GRP File" according to EUDDB
	SCBW_DATA(const u8*, IsTurnable, imagesDat[1].address);	//0066E860
	SCBW_DATA(const u8*, IsClickable, imagesDat[2].address);	//0066C150
	//SCBW_DATA(const u8*,  ???,		imagesDat[3].address);	//0066D4D8 //"Use Full Iscript" according to EUDDB
	//SCBW_DATA(const u8*,  ???,		imagesDat[4].address);	//00667718 //"Draw If Cloaked" according to EUDDB
	SCBW_DATA(const u8*, RLE_Function, imagesDat[5].address);	//00669E28 // DrawFunc
	SCBW_DATA(const u8*, Remapping, imagesDat[6].address);	//00669A40
	//SCBW_DATA(const u32*,  ???,		imagesDat[7].address);	//0066EC48 //"Iscript ID" according to EUDDB
	//SCBW_DATA(const u32*,  ???,		imagesDat[8].address);	//0066C538 //"Shields Overlay" according to EUDDB
	//SCBW_DATA(const u32*,  ???,		imagesDat[9].address);	//0066B1B0 //"Attack Overlay" according to EUDDB
	//SCBW_DATA(const u32*,  ???,		imagesDat[10].address);	//0066A210 //"Damage Overlay" according to EUDDB
	//SCBW_DATA(const u32*,  ???,		imagesDat[11].address);	//00667B00 //"Special Overlay" according to EUDDB
	//SCBW_DATA(const u32*,  ???,		imagesDat[12].address);	//00666778 //"Landing Dust Overlay" according to EUDDB
	//SCBW_DATA(const u32*,  ???,		imagesDat[13].address);	//0066D8C0 //"Lift-off Dust Overlay" according to EUDDB
} //images_dat (last entry at 0x005140AC)


//////////////////////////////////////////////////////////////// @}

/// @name Reserved data
/// Warning: The following section contains data reserved for internal use only.
///          If you are a modder, use these at your own risk!
//////////////////////////////////////////////////////////////// @{

//Helper functions for several hooks
SCBW_DATA(u8*, selectionIndexStart, 0x006284B6);
SCBW_DATA(const UnitsSel*, activePlayerSelection, 0x006284B8);
typedef Unit* (__cdecl* GetActivePlayerNextSelectionFunc)();
SCBW_DATA(GetActivePlayerNextSelectionFunc, getActivePlayerNextSelection, 0x0049A850);

/// Selections of all players
struct PlayersSel {
	Unit* unit[PLAYABLE_PLAYER_COUNT][SELECTION_ARRAY_LENGTH];
};
SCBW_DATA(const PlayersSel*, playersSelections, 0x006284E8);
struct PlayerSel {
	Unit* unit[SELECTION_ARRAY_LENGTH];
};
SCBW_DATA(const PlayerSel*, CurrentUnitSelection, 0x006284B8);

typedef void(__fastcall* PrepareForNextOrderFunc)(Unit*);
SCBW_DATA(PrepareForNextOrderFunc, prepareForNextOrder, 0x00475000);

//Contains various info on the tiles.
SCBW_DATA(ActiveTile**, activeTileArray, 0x006D1260);
SCBW_DATA(u16**, tileIdArray, 0x005993C4);
SCBW_DATA(u16**, megatileArray, 0x00628494);
//SCBW_DATA(u16**, megatileExtArrayFlags, 0x00628494+0x20000);//extender only

// Stores an ordered list of all units currently in the game.
SCBW_DATA(UnitFinderData*, unitOrderingX, 0x0066FF78);
SCBW_DATA(UnitFinderData*, unitOrderingY, 0x006769B8);
SCBW_DATA(const u32*, unitOrderingCount, 0x0066FF74);

SCBW_DATA(const Vector32*, angleDistance, 0x00512D28);
SCBW_DATA(u16*, megatileCount, 0x005998E0);
//Used internally by Starcraft to manage temporary units lists
SCBW_DATA(Unit***, tempUnitsListsArray, 0x006BB938);
SCBW_DATA(u32*, tempUnitsListCurrentArrayCount, 0x006BEE64);
SCBW_DATA(u32*, tempUnitsListArraysCountsListLastIndex, 0x006BEE6C);
SCBW_DATA(u32*, tempUnitsListArraysCountsList, 0x006BEE70);

// Font & Drawing
namespace graphics {
	struct Font; struct Bitmap;
}
SCBW_DATA(graphics::Font**, fontBase, 0x006CE0F4);
SCBW_DATA(graphics::Bitmap*, gameScreenBuffer, 0x006CEFF0);
SCBW_DATA(u8*, refreshRegions, 0x006CEFF8);  //640 x 480 divided into 1200 squares of 16x16
SCBW_DATA(Layers*, screenLayers, 0x006CEF50);
typedef void(__stdcall* DrawGameProc)(graphics::Bitmap* surface, Bounds* bounds);
static DrawGameProc const oldDrawGameProc = (DrawGameProc)0x004BD580;


//Array used in multiplayer to randomize factions
//used by unit.getFactionColor() and
//player.getFactionColor()
//Max index is PLAYABLE_PLAYER_COUNT-1
SCBW_DATA(s32*, factionsColorsOrdering, 0x0057F21C);

///Access to true player colors that differ from
///standard versions from graphics.h
///This is the PlayerColors from BWAPI
///Max index is PLAYER_COUNT-1
SCBW_DATA(u8*, playerMinimapColors, 0x00581DD6);

SCBW_DATA(u8*, playerInGameColors, 0x00581D76);

const ListExtern<Image, &Image::link> unusedImages(0x0057EB68, 0x0057EB70);
SCBW_DATA(List<Sprite>*, unusedSprites, 0x0063FE30);

struct SpriteTileData {
	Sprite* tails[256];
	Sprite* heads[256];
};
SCBW_DATA(SpriteTileData*, spritesOnTileRow, 0x00629288);

//Image-related
struct ImagesDatExtraOverlayLO_Files {
	LO_Header* attackOverlays[IMAGE_TYPE_COUNT];
	LO_Header* damageOverlays[IMAGE_TYPE_COUNT];
	LO_Header* specialOverlays[IMAGE_TYPE_COUNT];
	LO_Header* landingDustOverlays[IMAGE_TYPE_COUNT];
	LO_Header* liftoffDustOverlays[IMAGE_TYPE_COUNT];
};
SCBW_DATA(const ImagesDatExtraOverlayLO_Files*, lo_files, 0x0051F2A8);
SCBW_DATA(const LO_Header* const*, shieldOverlays, 0x0052E5C8);

SCBW_DATA(const ColorShiftData*, colorShift, 0x005128F8); //Use ColorRemapping::Enum as index
SCBW_DATA(GrpHead* const*, imageGrpGraphics, 0x0051CED0);

template <typename T>
struct PlayerFlags {
	T flags[PLAYER_COUNT];
};
SCBW_DATA(PlayerFlags<u8>*, playerAlliance, 0x0058D634);  //See scbw::isAlliedTo()
SCBW_DATA(PlayerFlags<u32>*, playerVision, 0x0057F1EC);

SCBW_DATA(Unit* const*, activePortraitUnit, 0x00597248);

SCBW_DATA(AI_Main*, AIScriptController, 0x0068FEE8);
SCBW_DATA(AiRegion* const*, AiRegionCaptains, 0x0069A604);
SCBW_DATA(u32*, aiSupplyReserved, 0x006CA4BC);
SCBW_DATA(AiScript**, first_active_aiscript, 0x0068C100);

SCBW_DATA(ResourceAreaArray*, resource_areas, 0x00692688);

//-------- Internal constants --------//

SCBW_DATA(const b32*, IS_GAME_PAUSED, 0x006509C4);  //See scbw::isGamePaused()
SCBW_DATA(const b8*, IS_BROOD_WAR, 0x0058F440);  //See scbw::isBroodWarMode()
SCBW_DATA(const u32*, CHEAT_STATE, 0x006D5A6C);  //See scbw::isCheatEnabled()
SCBW_DATA(const s32*, MAX_UNIT_WIDTH, 0x006BEE68);
SCBW_DATA(const s32*, MAX_UNIT_HEIGHT, 0x006BB930);
SCBW_DATA(const b32*, IS_IN_REPLAY, 0x006D0F14);  //See scbw::isInReplay()
SCBW_DATA(const b32*, IS_REPLAY_PAUSED, 0x006D11B0);
SCBW_DATA(b32*, IS_IN_GAME_LOOP, 0x006D11C8);
SCBW_DATA(u32*, lastRandomNumber, 0x0051CA14);  //See scbw::random(), scbw::randBetween()
SCBW_DATA(b32*, canUpdatePoweredStatus, 0x0063FF44);
SCBW_DATA(b32*, IS_PLACING_BUILDING, 0x00640880);

//-------- Tech levels --------//

struct _scTechs {
	u8 isEnabled[PLAYER_COUNT][24];		//0x0058CE24
	u8 isResearched[PLAYER_COUNT][24];	//0x0058CF44
};
struct _bwTechs {
	u8 isEnabled[PLAYER_COUNT][20];		//0x0058F050
	u8 isResearched[PLAYER_COUNT][20];	//0x0058F140
};

// Available and Researched techs are exactly next to each other,
// that's why with 2 entries structures, only 2 addresses are
// needed here when BWAPI got 4 of them.
SCBW_DATA(_scTechs*, TechSc, 0x0058CE24); //Use with ScTech::Enum
SCBW_DATA(_bwTechs*, TechBw, 0x0058F050); //Use with BwTech::Enum

//-------- Upgrade levels --------//

struct _scUpgrs {
	u8 maxLevel[PLAYER_COUNT][46];		//0x0058D088
	u8 currentLevel[PLAYER_COUNT][46];	//0x0058D2B0
};
struct _bwUpgrs {
	u8 maxLevel[PLAYER_COUNT][15];		//0x0058F278
	u8 currentLevel[PLAYER_COUNT][15];	//0x0058F32C
};
SCBW_DATA(_scUpgrs*, UpgradesSc, 0x0058D088); //Use with ScUpgrade::Enum
SCBW_DATA(_bwUpgrs*, UpgradesBw, 0x0058F278); //Use with BwUpgrade::Enum

// Requirement opcode lists
namespace requirements
{
	SCBW_DATA(u16*, units, 0x00514178);
	SCBW_DATA(u16*, orders, 0x00514CF8);
	SCBW_DATA(u16*, research, 0x00514908);
	SCBW_DATA(u16*, upgrade, 0x005145C0);
	SCBW_DATA(u16*, tech, 0x00514A48);
}

SCBW_DATA(GuardAiArray*,       guard_array,        0x0067D400);
SCBW_DATA(GuardAiListPlayers*, first_player_guard, 0x00685108);

SCBW_DATA(u32*, ai_update_cooldown, 0x0059CC94);

// For UpdateCreepDisappearanceUnit
SCBW_DATA(u32*, map_creep_width, 0x006D0F08);
SCBW_DATA(u32*, dying_unit_creep_disappearance_count, 0x00658AE4);
SCBW_DATA(u32**, current_tiles, 0x006D0E84);

//////////////////////////////////////////////////////////////// @}

#undef SCBW_DATA
#pragma pack()
