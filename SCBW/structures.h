//All-in-one include file
//Structs by poiuy_qwert, A_of_s_t, ravenwolf and pastelmind.
//Special thanks to Heinermann

#pragma once

const int PLAYER_COUNT			= 12;
const int PLAYABLE_PLAYER_COUNT	= 8;
const int UNIT_ARRAY_LENGTH		= 1700;
const int BULLET_ARRAY_LENGTH	= 100;
const int SPRITE_ARRAY_LENGTH	= 2500;
const int SELECTION_ARRAY_LENGTH = 12;
const int UNIT_TYPE_COUNT		= 228;
const int TECH_TYPE_COUNT		= 44;
const int UPGRADE_TYPE_COUNT	= 61;
const int WEAPON_TYPE_COUNT		= 130;
const int FLINGY_TYPE_COUNT		= 209;
const int IMAGE_TYPE_COUNT		= 999;
const int SOUND_TYPE_COUNT		= 1144;
#include "structures/CUnit.h"
#include "structures/CBullet.h"
#include "structures/CSprite.h"
#include "structures/CImage.h"
#include "structures/COrder.h"
#include "structures/Layer.h"
#include "structures/Player.h" //made into external file to have .cpp

#pragma pack(1)

struct CampaignMission
{
	char* missionName;
	int missionIndex;
};

struct chkStruct {
	uint8 section[4];
	uint32 function;
	uint32 flags;
};

struct scenarioFormat {
	chkStruct* chkStruct;
	uint32 chkStructCount;
};

struct vectorHead {
	uint32 next;
	uint32 prev;
};

struct unk_61 {
	uint8 anonymous_0;
	uint8 anonymous_1;
	uint8 anonymous_2;
	uint8 anonymous_3;
	uint8 anonymous_4;
	uint8 anonymous_5;
};

struct unk_62 {
	uint8 anonymous_0;
	uint8 anonymous_1;
	uint8 anonymous_2;
	uint8 anonymous_3;
	uint8 anonymous_4;
	uint8 anonymous_5;
	uint8 anonymous_6;
	uint8 anonymous_7;
};

struct bStringMap {
	uint8		value;				// 0x0
	uint8		id;					// 0x1
};

struct wStringMap {
	uint16		tblstring;			// 0x0
	uint16		index;				// 0x2
};

struct dlgEvent {
	uint32		user;				// 0x0
	uint16		field_4;			// 0x4
	uint16		field_6;			// 0x6
	uint16		wVirtKey;			// 0x8
	uint16		wVirtKeyMod;		// 0xA
	uint16		no;					// 0xC
	uint32		current;			// 0xE
	uint16		field_12;			// 0x12
};

struct GameData { // taken from IDA and from https://gist.githubusercontent.com/neivv/d56baedfdfa0145575825e160b64adb7/raw/1bfc4e2113966eac0749f7f02de3fa9a091fb84b/nuottei.txt
	uint32		saveTime;					// 0x0
	char	gameName[0x18];				// 0x4
	uint32		saveTimestamp;				// 0x1C
	uint16		mapWidthTiles;				// 0x20
	uint16		mapHeightTiles;				// 0x22
	uint8		activePlayercount;			// 0x24
	uint8		maxPlayerCount;				// 0x25
	uint8		gameSpeed;					// 0x26
	uint8		gameState;					// 0x27
	uint16		gameType;					// 0x28
	uint16		gameSubtype;				// 0x2A
	uint32		cdkeyHash;					// 0x2C
	uint16		tileset;					// 0x30
	uint8		isReplay;					// 0x32
	uint8		computerPlayerCount;		// 0x33
	uint8		hostName[0x19];				// 0x34
	char	mapTitle[0x20];				// 0x4D
	uint16		gotGameType;				// 0x6D
	uint16		gotGameSubtype;				// 0x6F
	uint16		gotGameSubtypeDisplay;		// 0x71
	uint16		gotGameSubtypeLabel;		// 0x73
	uint8		victoryCondition;			// 0x75
	uint8		resourceType;				// 0x76
	uint8		useStandardUnitStats;		// 0x77
	uint8		fogOfWar;					// 0x78
	uint8		startingUnits;				// 0x79
	uint8		useFixedPositions;			// 0x7A
	uint8		restrictionFlags;			// 0x7B
	uint8		alliesEnabled;				// 0x7C
	uint8		teamsEnabled;				// 0x7D
	uint8		cheatsEnabled;				// 0x7E
	uint8		tournamentModeEnabled;		// 0x7F
	uint32		victoryConditionValue;		// 0x80
	uint32		resourceMineralValue;		// 0x84
	uint32		resourceGasValue;			// 0x88
};
C_ASSERT(sizeof(GameData) == 0x8C);

struct ReplayData { // taken from https://gist.githubusercontent.com/neivv/d56baedfdfa0145575825e160b64adb7/raw/1bfc4e2113966eac0749f7f02de3fa9a091fb84b/nuottei.txt
	uint32		replayDataStart;			// 0x0
	uint32		boolUnk;					// 0x4
	uint32		replayFrameBegin;			// 0x8
	uint32		length[0x2];				// 0xC
	uint32		replayFrameEnd;				// 0x14
	uint32		latestCommandFrame;			// 0x18
	uint32		replayFramePos;				// 0x1C
};
C_ASSERT(sizeof(ReplayData) == 0x20);

struct ReplayFrame { // taken from https://gist.githubusercontent.com/neivv/d56baedfdfa0145575825e160b64adb7/raw/1bfc4e2113966eac0749f7f02de3fa9a091fb84b/nuottei.txt
	uint32		frame;						// 0x0
	uint8		dataLength;					// 0x4
	uint8		storm;						// 0x5 | { byte storm_player, byte cmd[...] }[...]
};
C_ASSERT(sizeof(ReplayFrame) == 0x6);

struct Options { // taken from IDA
	uint32		speed;						// 0x0
	uint32		mouseScroll;				// 0x4
	uint32		keyboardScroll;				// 0x8
	uint32		music;						// 0xC
	uint32		sfx;						// 0x10
	uint32		tipNumber;					// 0x14
	uint32		flags;						// 0x18
	uint32		m_mouseScroll;				// 0x1C
	uint32		m_keyboardScroll;			// 0x20
	uint32		useChatColors;				// 0x24
	uint32		null;						// 0x28
};
C_ASSERT(sizeof(Options) == 0x2C);

struct PlayerName {
	char	playerName[0x19];			// 0x0
};

struct UNITDEATHS {
  uint32        player[12];
};
struct UNITSCORES_I {
	uint32        player[12];
};
//Based on http://code.google.com/p/bwapi/source/browse/branches/bwapi4/bwapi/BWAPI/Source/BW/CImage.h

struct TEXT {
  char  textdisplay[218][13];  //[last is text at bottom; ex: "Cheat enabled."]
  unknown_byte   unknown1;
  unknown_byte   unknown2;
  uint8    color[13];
  unknown_byte   unknown3[3];  //[last is text at bottom; ex: "Cheat enabled."]
  uint8    timer[13];
};

struct TILE {
/*00*/  uint16 index;
/*02*/  uint8 buildability; /**< 8th bit should sign not buildable. */
/*03*/  uint8 groundHeight; /**< Ground Height(4 lower bits) - Deprecated? Some values are incorrect. */
/*04*/  uint16 leftEdge;
/*06*/  uint16 topEdge;
/*08*/  uint16 rightEdge;
/*0A*/  uint16 buttomEdge;
/*0C*/  uint16 _1;
/*0E*/  uint16 _2; /**<  Unknown - Edge piece has rows above it. (Recently noticed; not fully understood.)
                o 1 = Basic edge piece.
                o 2 = Right edge piece.
                o 3 = Left edge piece.*/
/*10*/  uint16 _3;
/*12*/  uint16 _4;
/*14*/  uint16 miniTile[16]; /** MegaTile References (VF4/VX4) */
};

struct DOODAD {
/*00*/  uint16 index;
/*02*/ uint8 buildability; /**< 8th bit should sign not buildable. */
/*03*/  uint8 groundHeightAndOverlayFlags; /**< Ground Height(4 lower bits) - Deprecated? Some values are incorrect.
                   * Overlay Flags:
                   * o 0x0 - None
                   * o 0x1 - Sprites.dat Reference
                   * o 0x2 - Units.dat Reference (unit sprite)
                   * o 0x4 - Overlay is Flipped
                   */
/*04*/  uint16 overlayID;
/*06*/  uint16 _1;
/*08*/  uint16 doodatGrupString;
/*0A*/  uint16 _2;
/*0C*/  uint16 dddataBinIndex;
/*0E*/  uint16 doodatHeight;
/*10*/  uint16 doodatWidth;
/*12*/  uint16 _3;
/*14*/  uint16 miniTile[16]; /** MegaTile References (VF4/VX4) */
};	//size 34

struct CThingy {
/*00*/  CThingy *prev;
/*04*/  CThingy *next;
/*08*/  uint32     unitType;
/*0C*/  CSprite *sprite;
};

struct CHKUnit {
/*00*/	uint32	 id;
/*04*/	uint16	 x;
/*06*/	uint16	 y;
/*08*/	uint16	 unitid;
/*0A*/	uint16	 relation;
/*0C*/	uint16	 validStateFlags;
/*0E*/	uint16	 validProperties;
/*10*/	uint8	 playerId;
/*11*/	uint8	 hp;
/*12*/	uint8	 shields;
/*13*/	uint8	 energy;
/*14*/	uint16	 resources;
/*16*/	uint16	 field_16;
/*18*/	uint16	 hangar;
/*1A*/	uint16	 stateFlags;
/*1C*/	uint32	 field_1c;
/*20*/	uint32	 relatedUnit;
};

//---- Taken from locations.cpp ----//

struct MapSize {
  uint16   width;
  uint16   height;
};

struct LOCATION {
  Box32 dimensions;
  uint16   stringId;
  uint16   flags;
};

//---- Taken from buttons.cpp ----//

typedef uint32 ButtonState;
typedef ButtonState (__fastcall *REQ_FUNC)(uint32, uint32, CUnit*);
//(uint32 reqVar, uint32 playerId, CUnit* selected);
typedef void (__fastcall *ACT_FUNC)(uint32,uint32);
//(uint32 actVar, uint32 shiftClick);

struct BUTTON {
/*00*/  uint16       position;
/*02*/  uint16       iconID;
/*04*/  REQ_FUNC  *reqFunc;
/*08*/  ACT_FUNC  *actFunc;
/*0C*/  uint16       reqVar;
/*0E*/  uint16       actVar;
/*10*/  uint16       reqStringID;
/*12*/  uint16       actStringID;
};

struct StatusScreenState {
	GrpHead* button_grp;
	uint16 grp_frame;//0x4
	uint16 type;//0x6
	uint8 id;//0x8	
};

struct BUTTON_SET {
  uint32     buttonsInSet;
  BUTTON  *firstButton;
  uint32     connectedUnit;
};

//---- Taken from triggers.h ----//
struct CONDITION {
  uint32        location;
  uint32        player;
  uint32        number;
  uint16        unit;
  uint8        comparison;
  uint8        condition;
  uint8        type;
  uint8        flags;
};

typedef bool32 (__fastcall *ConditionPointer)(CONDITION*);

struct ACTION {
  uint32        location;
  uint32        string;
  uint32        wavString;
  uint32        time;
  uint32        player;
  uint32        number;
  uint16        unit;
  uint8        action;
  uint8        number2;
  uint8        flags;
};

typedef bool32 (__fastcall *ActionPointer)(ACTION*);

//GRP frame header
struct GrpFrame {
  int8  x;
  int8  y;
  int8  width;
  int8  height;
  uint32 dataOffset;	//might be uint16* (but as a pointer, would still fit in uint32)
};

C_ASSERT(sizeof(GrpFrame) == 8); 
//static_assert(sizeof(GrpFrame) == 8, "The size of the GrpFrame structure is invalid");

//GRP file header
struct GrpHead {
  uint16 frameCount;
  int16 width;
  int16 height;
  GrpFrame frames[1];
};

C_ASSERT(sizeof(GrpHead) == 14); 
//static_assert(sizeof(GrpHead) == 14, "The size of the GrpHead structure is invalid");

//LO* file header
struct LO_Header {
  uint32 frameCount;
  uint32 overlayCount;
  uint32 frameOffsets[1];


  Point8 getOffset(int frameNum, int overlayNum) const {
    return ((Point8*)(this->frameOffsets[frameNum] + (uint32)this))[overlayNum];
  }
};

C_ASSERT(sizeof(LO_Header) == 12);
//static_assert(sizeof(LO_Header) == 12, "The size of the LO_Header structure is invalid");


//Image Remapping Data
struct ColorShiftData {
  uint32 index;
  void* data;
  char name[12];
};

C_ASSERT(sizeof(ColorShiftData) == 20);
//static_assert(sizeof(ColorShiftData) == 20, "The size of the ColorShiftData structure is invalid");

//-------- Flag structures --------//

//Based on BWAPI's Offsets.h
struct ActiveTile {
/*00*/  uint8 visibilityFlags;//0xff
/*01*/  uint8 exploredFlags;//0xff00
/*02*/  uint8 isWalkable         : 1; // Set on tiles that can be walked on
		uint8 unknown1           : 1; // Unused?
		uint8 isUnwalkable       : 1; // Set on tiles that can't be walked on
		uint8 unknown2           : 1; // Unused?
		uint8 hasDoodadCover     : 1; // Set when a doodad cover (trees) occupies the area
		uint8 unknown3           : 1; // Unused?
		uint8 hasCreep           : 1; // Set when creep occupies the area
		uint8 alwaysUnbuildable  : 1; // always unbuildable, like water
/*03*/  uint8 groundHeight       : 3; // ground height; 1 for low ground, 2 for middle ground and 4 for high ground
		uint8 currentlyOccupied  : 1; // unbuildable but can be made buildable
		uint8 creepReceeding     : 1; // Set when the nearby structure supporting the creep is destroyed
		uint8 cliffEdge          : 1; // Set if the tile is a cliff edge
		uint8 temporaryCreep     : 1; // Set when the creep occupying the area was created. Not set if creep tiles were preplaced. Used in drawing routine.
		uint8 unknown4           : 1; // Unused?
};

C_ASSERT(sizeof(ActiveTile) == 4);
//static_assert(sizeof(ActiveTile) == 4, "The size of the ActiveTile structure is invalid");

struct GroupFlag {
  uint8 isZerg         : 1;	/*0x01*/
  uint8 isTerran       : 1;	/*0x02*/
  uint8 isProtoss      : 1;	/*0x04*/
  uint8 isMen          : 1;	/*0x08*/
  uint8 isBuilding     : 1;	/*0x10*/
  uint8 isFactory      : 1;	/*0x20*/
  uint8 isIndependent  : 1;	/*0x40*/
  uint8 isNeutral      : 1;	/*0x80*/
};

C_ASSERT(sizeof(GroupFlag) == 1);
//static_assert(sizeof(GroupFlag) == 1, "The size of the GroupFlag structure is invalid");

struct TargetFlag {
  uint16 air         : 1;
  uint16 ground      : 1;
  uint16 mechanical  : 1;
  uint16 organic     : 1;
  uint16 nonBuilding : 1;
  uint16 nonRobotic  : 1;
  uint16 terrain     : 1;
  uint16 orgOrMech   : 1;
  uint16 playerOwned : 1;
};

C_ASSERT(sizeof(TargetFlag) == 2);
//static_assert(sizeof(TargetFlag) == 2, "The size of the TargetFlag structure is invalid");

//-------- End of flag structures --------//

struct UnitFinderData {
  int32 unitIndex;
  int32 position;
  bool operator < (const UnitFinderData& rhs) const {
    return this->position < rhs.position;
  }
};

C_ASSERT(sizeof(UnitFinderData) == 8);
//static_assert(sizeof(UnitFinderData) == 8, "The size of the UnitFinderData structure is invalid");

//BinDlg is defined in https://code.google.com/p/vgce/source/browse/trunk/docs/Blizzard/Starcraft/BIN%20FORMAT.txt
//as Dialog Structure
//That structure is variable depending on the content

struct BinDlg {
/*0x00*/  BinDlg  *next;
/*0x04*/  Bounds  bounds;			//0x4 left,0x6 top,0x8 right,0xA bottom,0xC width,0xE height
/*0x10*/  uint8      *buffer;
/*0x14*/  char    *pszText;
/*0x18*/  uint32     flags;			//use BinDlgFlags::Enum
/*0x1C*/  uint32     unk_1c;
/*0x20*/  uint16     index;
/*0x22*/  uint16     controlType;		//use DialogControlTypes::Enum
/*0x24*/  uint16     graphic;

union {
/*0x26*/  uint32     *user;
/*0x26*/  CUnit	  *unitUser;
/*0x26*/  BinDlg  *dlgUser;
/*0x26*/  BUTTON  *buttonUser;
		  StatusScreenState* statButtonUser;//iquare
/*0x26*/  uint32     nonUserValue;
};

/*0x2A*/  void    *fxnInteract;
/*0x2E*/  void    *fxnUpdate;
/*0x32*/  BinDlg  *parent;

union {
struct {
/*0x36*/  Box16   responseArea;
/*0x3E*/  BinDlg  *unk_3e;
}initialVersion0x36;
struct {
/*0x36*/  int16     responseAreaWidth;		//guess
/*0x38*/  int16     responseAreaHeight;		//guess
/*0x3A*/  uint32     *responseAreaBitArray;	//set with a memory allocation function, bit array is a guess
/*0x3E*/  uint32     *unk3e;					//unverified filler
}onInitDialog0x36;
struct {
/*0x36*/  void    *unkFunc36;				//should be a function
/*0x3A*/  uint16     scrollMinValue;			//guess to be confirmed if used
/*0x3C*/  uint16     scrollMaxValue;			//guess to be confirmed if used
/*0x3E*/  uint16     scrollCurrentValue;		//guess to be confirmed if used
/*0x40*/  uint16     unk40;					//unverified filler
}scrollDialog0x36;							//probably for scrollbar involving dialogs?
struct {
/*0x36*/  BinDlg  *unkDlg36;
/*0x3A*/  uint32     unk3a;					//unverified filler
/*0x3E*/  uint32     unk3e;					//unverified filler
}dialogOn0x36;								//for when 0x36 is clearly BinDlg* but the rest is unknown
struct {
/*0x36*/  bool32  unkBool32_36;				//based on the value only being compared to 0
/*0x3A*/  uint32     unk3a;					//unverified filler
/*0x3E*/  uint32     unk3e;					//unverified filler
}checkBox0x36;								//not completely sure
struct {
/*0x36*/  uint8      unkFlags36;				//"flag" because of possible unkFlags36 |= 1
/*0x37*/  unknown_byte     unk37[3];					//unverified filler
/*0x3A*/  uint8*     *unk3aArrayCharPtr;		//seen in a different context so may be wrong and belong to another category
/*0x3E*/  uint8      pszTextLen;				//should be the length of pszText
/*0x3F*/  unknown_byte     unk3f[3];					//unverified filler
}textDialog0x36;							//seems to apply to some text related dialogs
};

union {
/*0x42*/  void    *childrenSmk;
/*0x42*/  BinDlg  *childrenDlg;
};

union {
struct {
/*0x46*/  Point16 textPos;
/*0x4A*/  uint16     responseAreaWidth;
/*0x4C*/  uint16     responseAreaHeight;
/*0x4E*/  unknown_byte     unk_4e[8];
}initialVersion0x46;
struct {
/*0x46*/  Point16 textPos;					//unverified filler
/*0x4A*/  uint32     unk4a;
/*0x4E*/  unknown_byte     unk_4e[8];				//unverified filler
}onInitDialog0x46;
struct {
/*0x46*/  uint8	  unk46;
/*0x47*/  uint8      unk47;					//unverified filler
/*0x48*/  uint8      unk48;
/*0x49*/  uint8      unk49;
/*0x4A*/  uint8      unk4a;
/*0x4B*/  uint8      unk4b;
/*0x4C*/  bool8   unk4c;
/*0x4D*/  uint8      unk4d;
/*0x4E*/  uint8      unk4e;
/*0x4F*/  unknown_byte     unk_4f[7];				//unverified filler
}alternateVersion0x46;
};

};

C_ASSERT(sizeof(BinDlg) == 86); //0x56
//static_assert(sizeof(BinDlg) == 86, "The size of the BinDlg structure is invalid");

struct SMK_Structure {
/*0x00*/ uint32		smkOverlayOffset;
/*0x04*/ uint16		flags;				//use SmkFlags::Enum
/*0x06*/ uint32		internalUse1;
/*0x0A*/ uint32		smkFilenameOffset;
/*0x0E*/ uint32		internalUse2;
/*0x12*/ Point16	smkOverlayPosition;
/*0x16*/ uint32		internalUse3;
/*0x1A*/ uint32		internalUse4;
};

C_ASSERT(sizeof(SMK_Structure) == 30); //0x1E
//static_assert(sizeof(SMK_Structure) == 30, "The size of the SMK_Structure structure is invalid");

struct GuiOverlay {
/*0x00*/  unknown_byte     unk_0[6];
/*0x06*/  uint16     overlayType;
/*0x08*/  uint16     id;
/*0x0A*/  unknown_byte     unk_a[34];
};

C_ASSERT(sizeof(GuiOverlay) == 44); //0x2C
//static_assert(sizeof(GuiOverlay) == 44, "The size of the GuiOverlay structure is invalid");

//-------- AI related stuff --------//

struct AiCaptain { //AiRegion
	/*00*/  uint16 region;
	/*02*/  uint16 targetRegion;
	/*04*/  uint8  playerId;
	/*05*/  uint8  captainType;
	/*06*/  uint8  defensePriorityAdd;
	/*07*/  uint8  unkCount;
	/*08*/  uint8  captainFlags;
	/*09*/  uint8  unknown_0x9;
	/*0A*/  uint16 groundUnitCount;
	/*0C*/  uint16 neededGroundStrength;
	/*0E*/  uint16 neededAirStrength;
	/*10*/  uint16 regionGndStrength;
	/*12*/  uint16 regionAirStrength;
	/*14*/  uint16 fullGndStrength;
	/*16*/  uint16 fullAirStrength;
	/*18*/  uint16 enemyAirStrength;
	/*1A*/  uint16 enemyGroundStrength;
	/*1C*/  uint32 airTarget;
	/*20*/  uint32 groundTarget;
	/*24*/  CUnit* slowestUnit;
	/*28*/  CUnit* followTarget;
	/*2C*/  CUnit* mainMedic;
	/*30*/  void* town;
};
C_ASSERT(sizeof(AiCaptain) == 52);
//static_assert(sizeof(AiCaptain) == 52, "The size of the AiCaptain structure is invalid");

struct UnitAi {
	CLink<UnitAi>	link;				// 0x0, 0x4
	uint8 type;							// 0x8
};
C_ASSERT(sizeof(UnitAi) == 9);

struct Region {
	uint16 accessabilityFlags;
	uint16 group;
	uint8 _dc4[0x14];
	Box16 area;
	uint8 _dc20[0x20];
};

struct pathing {
	uint16 regionCount;
	uint8 _dc2[0x449fa];
	Region regions[5000];
	uint8 _dc92bfc[0x4e24];
};

C_ASSERT(sizeof(pathing) == 0x97a20);

struct GuardAi {
	CLink<GuardAi>	link;				// 0x0, 0x4
	uint8 type;							// 0x8
	uint8 timesDied;						// 0x9
	uint8 unkA[2];							// 0xA
	CUnit* parent;						// 0xC
	uint16 id;								// 0x10
	Point16 home;						// 0x12
	Point16 unkPos;						// 0x16
	uint8 unk1A[2];						// 0x1A
	uint32 previousUpdate;					// 0x1C
};
C_ASSERT(sizeof(GuardAi) == 32); // 0x20

struct AiTown; // declaring this here to avoid compiler errors
struct WorkerAi {
	CLink<WorkerAi>	link;				// 0x0, 0x4
	uint8 type;							// 0x8
	uint8 target_resource;					// 0x9
	uint8 reassign_count;					// 0xA
	uint8 waitTimer;						// 0xB
	uint32 last_update_second;				// 0xC
	CUnit* parent;						// 0x10
	AiTown* town;						// 0x14
};
C_ASSERT(sizeof(WorkerAi) == 24); // 0x18

struct BuildingAi {
	CLink<BuildingAi>	link;			// 0x0, 0x4
	uint8 type;							// 0x8
	uint8 trainQueueTypes[5];				// 0x9
	uint8 unkE[2];							// 0xE
	CUnit* parent;						// 0x10
	AiTown* town;						// 0x14
	void* trainQueueValues[5];			// 0x18
};
C_ASSERT(sizeof(BuildingAi) == 44); // 0x2C

struct MilitaryAi {
	CLink<MilitaryAi>	link;			// 0x0, 0x4
	uint8 type;							// 0x8
	uint8 padding[3];						// 0x9
	CUnit* parent;						// 0xC
	AiCaptain* region;					// 0x10
};
C_ASSERT(sizeof(MilitaryAi) == 20); // 0x14

struct WorkerAiArray {
	WorkerAi worker_ais[1000];
	WorkerAi* first_free_worker;
};

struct BuildingAiArray {
	BuildingAi building_ais[1000];
	BuildingAi* first_free_building;
};

struct TownReq {
	uint8 flags_and_count;
	uint8 priority;
	uint16 id;
};

struct AiTown {
	CLink<AiTown> link;							//	0x0 and 0x4
	WorkerAiArray* free_workers;				//	0x8
	WorkerAi* workers;							//	0xC
	BuildingAiArray* free_buildings;			//	0x10
	BuildingAi* buildings;						//	0x14
	uint8 player;									//	0x18
	uint8 inited;									//	0x19
	uint8 worker_count;							//	0x1a
	uint8 worker_limit;							//	0x1b
	uint8 resource_area;							//	0x1c
	uint8 resource_area_not_set;					//	0x1d
	uint8 in_battle;								//	0x1e
	uint8 unk1f;									//	0x1f
	Point16 position;							//	0x20
	CUnit* main_building;						//	0x24
	CUnit* building_scv;						//	0x28
	CUnit* mineral;								//	0x2c
	CUnit* gas_buildings[3];					//	0x30
	TownReq town_units[0x64];					//	0x3c
};
C_ASSERT(sizeof(AiTown) == 0x1cc);

struct AiSpendingRequest {
	uint8 priority;
	uint8 type;
	uint16 id;
	void* val;
};

struct SpendingRequest {
	uint8 priority;
	uint8 ty;
	uint16 id; 
	void* val;
};
struct AI_Main {
	uint32 mineralCollection;
	uint32 gasCollection;
	uint32 supplyCollection;
	uint32 minerals_available;
	uint32 gas_available;
	uint32 supply_available;
	SpendingRequest requests[0x3f];
	uint8_t request_count;
	uint8_t liftoff_cooldown;
	uint16 AI_NukeRate;
	uint16 AI_Attacks;
	uint16 AI_LastNukeTime;
	struct {
		uint16 isSecureFinished : 1;	/*0001*/
		uint16 isTownStarted : 1;	/*0002*/
		uint16 isDefaultBuildOff : 1;	/*0004*/
		uint16 isTransportsOff : 1;	/*0008*/
		uint16 isFarmsNotimingOn : 1;	/*0010*/
		uint16 isUseMapSettings : 1;	/*0020*/
		uint16 flag_0x40 : 1;	/*0040*/
		uint16 spreadCreep : 1;	/*0080*/
		uint16 flag_0x100 : 1;	/*0100*/
		uint16 hasStrongestGndArmy : 1;	/*0200*/ //Set if the AI's ground army is stronger than the total strength of the strongest enemy force
		uint16 bUpgradesFinished : 1;	/*0400*/
		uint16 bTargetExpansion : 1;	/*0800*/
	} AI_Flags;
	uint16 AI_PanicBlock;
	uint16 AI_MaxForce;
	uint16_t AI_AttackGroup;
	uint16_t wanted_unit;
	uint8_t dc222[0x2];
	uint32 lastIndividualUpdateTime; // 0x224
	uint32 last_attack_seconds;
	uint8_t strategic_suicide; // 0x22c
	uint8_t spellcasterTimer;
	uint8_t attack_failed;
	uint8_t difficulty;
	uint16_t attackers[0x40]; // 0x230
	/*2B0*/  uint32 AI_DefenseBuild_GG[10];
	/*2D8*/  uint32 AI_DefenseBuild_AG[10];
	/*300*/  uint32 AI_DefenseBuild_GA[10];
	/*328*/  uint32 AI_DefenseBuild_AA[10];
	/*350*/  uint32 AI_DefenseUse_GG[10];
	/*378*/  uint32 AI_DefenseUse_AG[10];
	/*3A0*/  uint32 AI_DefenseUse_GA[10];
	/*3C8*/  uint32 AI_DefenseUse_AA[10];
	uint8_t unit_build_limits[0xe4]; // 0x3f0
	CUnit* mainMedic; // 0x4d4
	uint8_t unk4d8[0x10];
};

C_ASSERT(sizeof(AI_Main) == 1256); /*0x4E8*/
//static_assert(sizeof(AI_Main) == 1256, "The size of the AI_Main structure is invalid");

// From BWAPI Offsets.h, used in scbwdata.h by UnitAvailability
struct _uavail {
	uint8 available[PLAYER_COUNT][UNIT_TYPE_COUNT];
};

class StringTbl {
  public:
    /// Returns the number of string entries in the TBL file.
    uint16 getStringCount() const { return *buffer; }

    /// Returns the string at @p index in the TBL file.
    /// If the @p index is 0, returns nullptr.
    /// If the @p index is out of bounds, returns an empty string.
    const char* getString(uint16 index) const {
      //Based on function @ 0x004C36F0
      if (index == 0) return nullptr;
      else if (index <= getStringCount())
        return (const char*)(buffer) + buffer[index];
      else
		  return (const char*)0x00501B7D;	//StringEmpty in scbwdata.h, SC default empty string
    }

  private:
    uint16* buffer;
};

#pragma pack()
