//All-in-one include file
//Structs by poiuy_qwert, A_of_s_t, ravenwolf and pastelmind.
//Special thanks to Heinermann

#pragma once
#include "types.h"

const s32 PLAYER_COUNT = 12;
const s32 PLAYABLE_PLAYER_COUNT = 8;
const s32 UNIT_ARRAY_LENGTH = 1700;
const s32 BULLET_ARRAY_LENGTH = 100;
const s32 SPRITE_ARRAY_LENGTH = 2500;
const s32 SELECTION_ARRAY_LENGTH = 12;
const s32 UNIT_TYPE_COUNT = 228;
const s32 TECH_TYPE_COUNT = 44;
const s32 UPGRADE_TYPE_COUNT = 61;
const s32 WEAPON_TYPE_COUNT = 256;
const s32 FLINGY_TYPE_COUNT = 209;
const s32 IMAGE_TYPE_COUNT = 999;
const s32 SOUND_TYPE_COUNT = 1144;
#include "structures/Unit.h"
#include "structures/Bullet.h"
#include "structures/Sprite.h"
#include "structures/Image.h"
#include "structures/Order.h"
#include "structures/Layer.h"
#include "structures/Player.h" // made into external file to have .cpp

#pragma pack(1)

struct unitPath
{
	s32		start;
	s32		next;
	s32		finish;
	s32		timeStarted;
	s32		unitIndex;
	s8		isCalculating;
	s8		field_19;
	s8		field_1A;
	s8		pathCount;
	s8		numAreas;
	s8		curAreas;
	s8		numSegments;
	s8		curSegments;
	s32		steps[24];
};

struct CampaignMission
{
	char* missionName;
	s32 missionIndex;
};

struct chkStruct
{
	u8 section[4];
	u32 function;
	u32 flags;
};

struct scenarioFormat
{
	chkStruct* chkStruct;
	u32 chkStructCount;
};

struct vectorHead
{
	u32 next;
	u32 prev;
};

struct unk_61
{
	u8 anonymous_0;
	u8 anonymous_1;
	u8 anonymous_2;
	u8 anonymous_3;
	u8 anonymous_4;
	u8 anonymous_5;
};

struct unk_62
{
	u8 anonymous_0;
	u8 anonymous_1;
	u8 anonymous_2;
	u8 anonymous_3;
	u8 anonymous_4;
	u8 anonymous_5;
	u8 anonymous_6;
	u8 anonymous_7;
};

struct bStringMap
{
	u8		value;				// 0x0
	u8		id;					// 0x1
};

struct wStringMap
{
	u16		tblstring;			// 0x0
	u16		index;				// 0x2
};

struct dlgEvent
{
	u32		user;				// 0x0
	u16		field_4;			// 0x4
	u16		field_6;			// 0x6
	u16		wVirtKey;			// 0x8
	u16		wVirtKeyMod;		// 0xA
	u16		no;					// 0xC
	u32		current;			// 0xE
	u16		field_12;			// 0x12
};

struct GameData // taken from IDA and from https://gist.githubusercontent.com/neivv/d56baedfdfa0145575825e160b64adb7/raw/1bfc4e2113966eac0749f7f02de3fa9a091fb84b/nuottei.txt
{ 
	u32		saveTime;					// 0x0
	char	gameName[0x18];				// 0x4
	u32		saveTimestamp;				// 0x1C
	u16		mapWidthTiles;				// 0x20
	u16		mapHeightTiles;				// 0x22
	u8		activePlayercount;			// 0x24
	u8		maxPlayerCount;				// 0x25
	u8		gameSpeed;					// 0x26
	u8		gameState;					// 0x27
	u16		gameType;					// 0x28
	u16		gameSubtype;				// 0x2A
	u32		cdkeyHash;					// 0x2C
	u16		tileset;					// 0x30
	u8		isReplay;					// 0x32
	u8		computerPlayerCount;		// 0x33
	u8		hostName[0x19];				// 0x34
	char	mapTitle[0x20];				// 0x4D
	u16		gotGameType;				// 0x6D
	u16		gotGameSubtype;				// 0x6F
	u16		gotGameSubtypeDisplay;		// 0x71
	u16		gotGameSubtypeLabel;		// 0x73
	u8		victoryCondition;			// 0x75
	u8		resourceType;				// 0x76
	u8		useStandardUnitStats;		// 0x77
	u8		fogOfWar;					// 0x78
	u8		startingUnits;				// 0x79
	u8		useFixedPositions;			// 0x7A
	u8		restrictionFlags;			// 0x7B
	u8		alliesEnabled;				// 0x7C
	u8		teamsEnabled;				// 0x7D
	u8		cheatsEnabled;				// 0x7E
	u8		tournamentModeEnabled;		// 0x7F
	u32		victoryConditionValue;		// 0x80
	u32		resourceMineralValue;		// 0x84
	u32		resourceGasValue;			// 0x88
};
C_ASSERT(sizeof(GameData) == 0x8C);

struct ReplayData // taken from https://gist.githubusercontent.com/neivv/d56baedfdfa0145575825e160b64adb7/raw/1bfc4e2113966eac0749f7f02de3fa9a091fb84b/nuottei.txt
{ 
	u32		replayDataStart;			// 0x0
	u32		boolUnk;					// 0x4
	u32		replayFrameBegin;			// 0x8
	u32		length[0x2];				// 0xC
	u32		replayFrameEnd;				// 0x14
	u32		latestCommandFrame;			// 0x18
	u32		replayFramePos;				// 0x1C
};
C_ASSERT(sizeof(ReplayData) == 0x20);

struct ReplayFrame // taken from https://gist.githubusercontent.com/neivv/d56baedfdfa0145575825e160b64adb7/raw/1bfc4e2113966eac0749f7f02de3fa9a091fb84b/nuottei.txt
{
	u32		frame;						// 0x0
	u8		dataLength;					// 0x4
	u8		storm;						// 0x5 | { byte storm_player, byte cmd[...] }[...]
};
C_ASSERT(sizeof(ReplayFrame) == 0x6);

struct Options // taken from IDA
{
	u32		speed;						// 0x0
	u32		mouseScroll;				// 0x4
	u32		keyboardScroll;				// 0x8
	u32		music;						// 0xC
	u32		sfx;						// 0x10
	u32		tipNumber;					// 0x14
	u32		flags;						// 0x18
	u32		m_mouseScroll;				// 0x1C
	u32		m_keyboardScroll;			// 0x20
	u32		useChatColors;				// 0x24
	u32		null;						// 0x28
};
C_ASSERT(sizeof(Options) == 0x2C);

struct PlayerName
{
	char	playerName[0x19];			// 0x0
};

struct UNITDEATHS
{
	u32        player[12];
};
struct UNITSCORES_I
{
	u32        player[12];
};
//Based on http://code.google.com/p/bwapi/source/browse/branches/bwapi4/bwapi/BWAPI/Source/BW/Image.h

struct TEXT
{
	char  textdisplay[218][13];  //[last is text at bottom; ex: "Cheat enabled."]
	unk_t   unknown1;
	unk_t   unknown2;
	u8    color[13];
	unk_t   unknown3[3];  //[last is text at bottom; ex: "Cheat enabled."]
	u8    timer[13];
};

struct TILE
{
	/*00*/  u16 index;
	/*02*/  u8 buildability; /**< 8th bit should sign not buildable. */
	/*03*/  u8 groundHeight; /**< Ground Height(4 lower bits) - Deprecated? Some values are incorrect. */
	/*04*/  u16 leftEdge;
	/*06*/  u16 topEdge;
	/*08*/  u16 rightEdge;
	/*0A*/  u16 buttomEdge;
	/*0C*/  u16 _1;
	/*0E*/  u16 _2; /**<  Unknown - Edge piece has rows above it. (Recently noticed; not fully understood.)
					o 1 = Basic edge piece.
					o 2 = Right edge piece.
					o 3 = Left edge piece.*/
	/*10*/  u16 _3;
	/*12*/  u16 _4;
	/*14*/  u16 miniTile[16]; /** MegaTile References (VF4/VX4) */
};

struct DOODAD
{
	/*00*/  u16 index;
	/*02*/ u8 buildability; /**< 8th bit should sign not buildable. */
	/*03*/  u8 groundHeightAndOverlayFlags; /**< Ground Height(4 lower bits) - Deprecated? Some values are incorrect.
					   * Overlay Flags:
					   * o 0x0 - None
					   * o 0x1 - Sprites.dat Reference
					   * o 0x2 - Units.dat Reference (unit sprite)
					   * o 0x4 - Overlay is Flipped
					   */
	/*04*/  u16 overlayID;
	/*06*/  u16 _1;
	/*08*/  u16 doodatGrupString;
	/*0A*/  u16 _2;
	/*0C*/  u16 dddataBinIndex;
	/*0E*/  u16 doodatHeight;
	/*10*/  u16 doodatWidth;
	/*12*/  u16 _3;
	/*14*/  u16 miniTile[16]; /** MegaTile References (VF4/VX4) */
};	//size 34

struct Thingy
{
	/*00*/  Thingy* prev;
	/*04*/  Thingy* next;
	/*08*/  u32     unitType;
	/*0C*/  Sprite* sprite;
};

struct CHKUnit
{
	/*00*/	u32	 id;
	/*04*/	u16	 x;
	/*06*/	u16	 y;
	/*08*/	u16	 unitid;
	/*0A*/	u16	 relation;
	/*0C*/	u16	 validStateFlags;
	/*0E*/	u16	 validProperties;
	/*10*/	u8	 playerId;
	/*11*/	u8	 hp;
	/*12*/	u8	 shields;
	/*13*/	u8	 energy;
	/*14*/	u16	 resources;
	/*16*/	u16	 field_16;
	/*18*/	u16	 hangar;
	/*1A*/	u16	 stateFlags;
	/*1C*/	u32	 field_1c;
	/*20*/	u32	 relatedUnit;
};

//---- Taken from locations.cpp ----//

struct MapSize
{
	u16   width;
	u16   height;
};

struct LOCATION
{
	Box32 dimensions;
	u16   stringId;
	u16   flags;
};

//---- Taken from buttons.cpp ----//

typedef u32 ButtonState;
typedef ButtonState(__fastcall* REQ_FUNC)(u32, u32, Unit*);
//(u32 reqVar, u32 playerId, Unit* selected);
typedef void(__fastcall* ACT_FUNC)(u32, u32);
//(u32 actVar, u32 shiftClick);

struct BUTTON
{
	/*00*/  u16       position;
	/*02*/  u16       iconID;
	/*04*/  REQ_FUNC* reqFunc;
	/*08*/  ACT_FUNC* actFunc;
	/*0C*/  u16       reqVar;
	/*0E*/  u16       actVar;
	/*10*/  u16       reqStringID;
	/*12*/  u16       actStringID;
};

struct StatusScreenState
{
	GrpHead* button_grp;
	u16 grp_frame;//0x4
	u16 type;//0x6
	u8 id;//0x8	
};

struct BUTTON_SET
{
	u32     buttonsInSet;
	BUTTON* firstButton;
	u32     connectedUnit;
};

//---- Taken from triggers.h ----//
struct CONDITION
{
	u32        location;
	u32        player;
	u32        number;
	u16        unit;
	u8        comparison;
	u8        condition;
	u8        type;
	u8        flags;
};

typedef b32(__fastcall* ConditionPointer)(CONDITION*);

struct ACTION
{
	u32        location;
	u32        string;
	u32        wavString;
	u32        time;
	u32        player;
	u32        number;
	u16        unit;
	u8        action;
	u8        number2;
	u8        flags;
};

typedef b32(__fastcall* ActionPointer)(ACTION*);

//GRP frame header
struct GrpFrame
{
	s8  x;
	s8  y;
	s8  width;
	s8  height;
	u32 dataOffset;	//might be u16* (but as a pointer, would still fit in u32)
};

C_ASSERT(sizeof(GrpFrame) == 8);
//static_assert(sizeof(GrpFrame) == 8, "The size of the GrpFrame structure is invalid");

//GRP file header
struct GrpHead
{
	u16 frameCount;
	s16 width;
	s16 height;
	GrpFrame frames[1];
};

C_ASSERT(sizeof(GrpHead) == 14);
//static_assert(sizeof(GrpHead) == 14, "The size of the GrpHead structure is invalid");

//LO* file header
struct LO_Header
{
	u32 frameCount;
	u32 overlayCount;
	u32 frameOffsets[1];


	Point8 getOffset(s32 frameNum, s32 overlayNum) const {
		return ((Point8*)(this->frameOffsets[frameNum] + (u32)this))[overlayNum];
	}
};

C_ASSERT(sizeof(LO_Header) == 12);
//static_assert(sizeof(LO_Header) == 12, "The size of the LO_Header structure is invalid");


//Image Remapping Data
struct ColorShiftData
{
	u32 index;
	void* data;
	char name[12];
};

C_ASSERT(sizeof(ColorShiftData) == 20);
//static_assert(sizeof(ColorShiftData) == 20, "The size of the ColorShiftData structure is invalid");

//-------- Flag structures --------//

//Based on BWAPI's Offsets.h
union ActiveTile
{
	u32 value;
	struct
	{
	/*00*/  u8 visibilityFlags;				// 0xff
	/*01*/  u8 exploredFlags;				// 0xff00
	/*02*/  u8 isWalkable : 1;				// Set on tiles that can be walked on
			u8 unknown1 : 1;				// Unused?
			u8 isUnwalkable : 1;			// Set on tiles that can't be walked on
			u8 unknown2 : 1;				// Unused?
			u8 hasDoodadCover : 1;			// Set when a doodad cover (trees) occupies the area
			u8 unknown3 : 1;				// Unused?
			u8 hasCreep : 1;				// Set when creep occupies the area
			u8 alwaysUnbuildable : 1;		// always unbuildable, like water
	/*03*/  u8 groundHeight : 3;			// ground height; 1 for low ground, 2 for middle ground and 4 for high ground
			u8 currentlyOccupied : 1;		// unbuildable but can be made buildable
			u8 creepReceeding : 1;			// Set when the nearby structure supporting the creep is destroyed
			u8 cliffEdge : 1;				// Set if the tile is a cliff edge
			u8 temporaryCreep : 1;			// Set when the creep occupying the area was created. Not set if creep tiles were preplaced. Used in drawing routine.
			u8 unknown4 : 1;				// Unused?
	};
};

C_ASSERT(sizeof(ActiveTile) == 4);

struct GroupFlag
{
	u8 isZerg : 1;			/*0x01*/
	u8 isTerran : 1;		/*0x02*/
	u8 isProtoss : 1;		/*0x04*/
	u8 isMen : 1;			/*0x08*/
	u8 isBuilding : 1;		/*0x10*/
	u8 isFactory : 1;		/*0x20*/
	u8 isIndependent : 1;	/*0x40*/
	u8 isNeutral : 1;		/*0x80*/
};

C_ASSERT(sizeof(GroupFlag) == 1);

struct TargetFlag
{
	u16 air : 1;
	u16 ground : 1;
	u16 mechanical : 1;
	u16 organic : 1;
	u16 nonBuilding : 1;
	u16 nonRobotic : 1;
	u16 terrain : 1;
	u16 orgOrMech : 1;
	u16 playerOwned : 1;
};

C_ASSERT(sizeof(TargetFlag) == 2);
//static_assert(sizeof(TargetFlag) == 2, "The size of the TargetFlag structure is invalid");

//-------- End of flag structures --------//

struct UnitFinderData
{
	s32 unitIndex;
	s32 position;
	bool operator < (const UnitFinderData& rhs) const {
		return this->position < rhs.position;
	}
};

C_ASSERT(sizeof(UnitFinderData) == 8);
//static_assert(sizeof(UnitFinderData) == 8, "The size of the UnitFinderData structure is invalid");

// BinDlg is defined in https://code.google.com/p/vgce/source/browse/trunk/docs/Blizzard/Starcraft/BIN%20FORMAT.txt
// as Dialog Structure
// That structure is variable depending on the content

struct BinDlg
{
	/*0x00*/  BinDlg*	next;
	/*0x04*/  Bounds	bounds;			// 0x4 left,0x6 top,0x8 right,0xA bottom,0xC width,0xE height
	/*0x10*/  u8*		buffer;
	/*0x14*/  char*		pszText;
	/*0x18*/  u32		flags;			// use BinDlgFlags::Enum
	/*0x1C*/  u32		unk_1c;
	/*0x20*/  u16		index;
	/*0x22*/  u16		controlType;	// use DialogControlTypes::Enum
	/*0x24*/  u16		graphic;

	union
	{
		/*0x26*/  u32*		user;
		/*0x26*/  Unit*		unitUser;
		/*0x26*/  BinDlg*	dlgUser;
		/*0x26*/  BUTTON*	buttonUser;
		StatusScreenState*	statButtonUser; // from iquare
		/*0x26*/  u32		nonUserValue;
	};

	/*0x2A*/  void*			fxnInteract;
	/*0x2E*/  void*			fxnUpdate;
	/*0x32*/  BinDlg*		parent;

	union {
		struct
		{
			/*0x36*/  Box16		responseArea;
			/*0x3E*/  BinDlg*	unk_3e;
		} initialVersion0x36;
		struct
		{
			/*0x36*/  s16		responseAreaWidth;		// guess
			/*0x38*/  s16		responseAreaHeight;		// guess
			/*0x3A*/  u32*		responseAreaBitArray;	// set with a memory allocation function, bit array is a guess
			/*0x3E*/  u32*		unk3e;					// unverified filler
		} onInitDialog0x36;
		struct
		{
			/*0x36*/  void*		unkFunc36;				// should be a function
			/*0x3A*/  u16		scrollMinValue;			// guess to be confirmed if used
			/*0x3C*/  u16		scrollMaxValue;			// guess to be confirmed if used
			/*0x3E*/  u16		scrollCurrentValue;		// guess to be confirmed if used
			/*0x40*/  u16		unk40;					// unverified filler
		} scrollDialog0x36;								// probably for scrollbar involving dialogs?
		struct
		{
			/*0x36*/  BinDlg*	unkDlg36;
			/*0x3A*/  u32		unk3a;					// unverified filler
			/*0x3E*/  u32		unk3e;					// unverified filler
		} dialogOn0x36;									// for when 0x36 is clearly BinDlg* but the rest is unknown
		struct
		{
			/*0x36*/  b32		unkBool32_36;			// based on the value only being compared to 0
			/*0x3A*/  u32		unk3a;					// unverified filler
			/*0x3E*/  u32		unk3e;					// unverified filler
		} checkBox0x36;									// not completely sure
		struct
		{
			/*0x36*/  u8		unkFlags36;				// "flag" because of possible unkFlags36 |= 1
			/*0x37*/  unk_t     unk37[3];				// unverified filler
			/*0x3A*/  u8**		unk3aArrayCharPtr;		// seen in a different context so may be wrong and belong to another category
			/*0x3E*/  u8		pszTextLen;				// should be the length of pszText
			/*0x3F*/  unk_t     unk3f[3];				// unverified filler
		} textDialog0x36;								// seems to apply to some text related dialogs
	};

	union {
		/*0x42*/  void* childrenSmk;
		/*0x42*/  BinDlg* childrenDlg;
	};

	union {
		struct
		{
			/*0x46*/  Point16	textPos;
			/*0x4A*/  u16		responseAreaWidth;
			/*0x4C*/  u16		responseAreaHeight;
			/*0x4E*/  unk_t     unk_4e[8];
		} initialVersion0x46;
		struct
		{
			/*0x46*/  Point16	textPos;				// unverified filler
			/*0x4A*/  u32		unk4a;
			/*0x4E*/  unk_t     unk_4e[8];				// unverified filler
		} onInitDialog0x46;
		struct
		{
			/*0x46*/  u8		unk46;
			/*0x47*/  u8		unk47;					// unverified filler
			/*0x48*/  u8		unk48;
			/*0x49*/  u8		unk49;
			/*0x4A*/  u8		unk4a;
			/*0x4B*/  u8		unk4b;
			/*0x4C*/  b8		unk4c;
			/*0x4D*/  u8		unk4d;
			/*0x4E*/  u8		unk4e;
			/*0x4F*/  unk_t     unk_4f[7];				// unverified filler
		} alternateVersion0x46;
	};

};

C_ASSERT(sizeof(BinDlg) == 86); //0x56
//static_assert(sizeof(BinDlg) == 86, "The size of the BinDlg structure is invalid");

struct SMK_Structure
{
	/*0x00*/ u32		smkOverlayOffset;
	/*0x04*/ u16		flags;				//use SmkFlags::Enum
	/*0x06*/ u32		internalUse1;
	/*0x0A*/ u32		smkFilenameOffset;
	/*0x0E*/ u32		internalUse2;
	/*0x12*/ Point16	smkOverlayPosition;
	/*0x16*/ u32		internalUse3;
	/*0x1A*/ u32		internalUse4;
};

C_ASSERT(sizeof(SMK_Structure) == 30); //0x1E
//static_assert(sizeof(SMK_Structure) == 30, "The size of the SMK_Structure structure is invalid");

struct GuiOverlay
{
	/*0x00*/  unk_t     unk_0[6];
	/*0x06*/  u16     overlayType;
	/*0x08*/  u16     id;
	/*0x0A*/  unk_t     unk_a[34];
};

C_ASSERT(sizeof(GuiOverlay) == 44); //0x2C
//static_assert(sizeof(GuiOverlay) == 44, "The size of the GuiOverlay structure is invalid");

//-------- AI related stuff --------//

struct AiTown;
struct MilitaryAiArray; // declaring this here to avoid compiler errors
struct AiRegion // AiRegion
{
	/*00*/  u16 region;
	/*02*/  u16 targetRegion;
	/*04*/  u8  playerId;
	/*05*/  u8  captainType;
	/*06*/  u8  defensePriorityAdd;
	/*07*/  u8  unkCount;
	/*08*/  u8  captainFlags;
	/*09*/  u8  unknown_0x9;
	/*0A*/  u16 groundUnitCount;
	/*0C*/  u16 neededGroundStrength;
	/*0E*/  u16 neededAirStrength;
	/*10*/  u16 regionGroundStrength;
	/*12*/  u16 regionAirStrength;
	/*14*/  u16 fullGroundStrength;
	/*16*/  u16 fullAirStrength;
	/*18*/  u16 enemyAirStrength;
	/*1A*/  u16 enemyGroundStrength;
	/*1C*/  u32 airTarget;
	/*20*/  u32 groundTarget;
	/*24*/  Unit* slowestUnit;
	/*28*/  Unit* followTarget;
	/*2C*/  MilitaryAiArray* military;
	/*30*/  AiTown* town;
};

C_ASSERT(sizeof(AiRegion) == 52);
//static_assert(sizeof(AiRegion) == 52, "The size of the AiRegion structure is invalid");

struct UnitAi
{
	Link<UnitAi>	link;				// 0x0, 0x4
	u8 type;							// 0x8
};
C_ASSERT(sizeof(UnitAi) == 9);

struct Region
{
	u16		accessabilityFlags;		// 0x00
	u16		groupIndex;				// 0x02
	u16		tileCount;				// 0x04
	u8		pathCount;				// 0x06
	u8		neighborCount;			// 0x07
	u32		field_8;				// 0x08
	u32		neighbors;				// 0x0C
	u32		regionCenterX;			// 0x10
	u32		regionCenterY;			// 0x14
	u16		left;					// 0x18
	u16		top;					// 0x1A
	u16		right;					// 0x1C
	u16		bottom;					// 0x1E
	u32		properties;				// 0x20
	u32		field_24;				// 0x24
	u32		field_28;				// 0x28
	u16		localData[14];			// 0x2C
};

struct pathing
{
	u16 regionCount;
	u8 _dc2[0x449fa];
	Region regions[5000];
	u8 _dc92bfc[0x4e24];
};

struct GuardAi
{
	Link<GuardAi>	link;				// 0x0, 0x4
	u8 type;							// 0x8
	u8 timesDied;						// 0x9
	u8 unkA[2];							// 0xA
	Unit* parent;						// 0xC
	u16 id;								// 0x10
	Point16 home;						// 0x12
	Point16 unkPos;						// 0x16
	u8 unk1A[2];						// 0x1A
	u32 previousUpdate;					// 0x1C
};
C_ASSERT(sizeof(GuardAi) == 32); // 0x20

struct AiTown; // declaring this here to avoid compiler errors
struct WorkerAi
{
	Link<WorkerAi>	link;				// 0x0, 0x4
	u8 type;							// 0x8
	u8 target_resource;					// 0x9
	u8 reassign_count;					// 0xA
	u8 waitTimer;						// 0xB
	u32 last_update_second;				// 0xC
	Unit* parent;						// 0x10
	AiTown* town;						// 0x14
};
C_ASSERT(sizeof(WorkerAi) == 24); // 0x18

struct BuildingAi
{
	Link<BuildingAi>	link;			// 0x0, 0x4
	u8 type;							// 0x8
	u8 trainQueueTypes[5];				// 0x9
	u8 unkE[2];							// 0xE
	Unit* parent;						// 0x10
	AiTown* town;						// 0x14
	void* trainQueueValues[5];			// 0x18
};
C_ASSERT(sizeof(BuildingAi) == 44); // 0x2C

struct MilitaryAi
{
	Link<MilitaryAi>	link;			// 0x0, 0x4
	u8 type;							// 0x8
	u8 padding1;						// 0x9
	u16 padding2;						// 0xA
	Unit* parent;						// 0xC
	AiRegion* region;					// 0x10
};
C_ASSERT(sizeof(MilitaryAi) == 20); // 0x14

struct WorkerAiArray
{
	WorkerAi worker_ais[1000];
	WorkerAi* first_free_worker;
};

struct BuildingAiArray
{
	BuildingAi building_ais[1000];
	BuildingAi* first_free_building;
};

struct MilitaryAiArray
{
	MilitaryAi military_ais[1000];
	MilitaryAi* first_free_military;
};

struct TownReq
{
	u8 flags_and_count;
	u8 priority;
	u16 id;
};

struct AiTown
{
	Link<AiTown> link;							//	0x0 and 0x4
	WorkerAiArray* free_workers;				//	0x8
	Unit* firstTownUnit;						//	0xC
	BuildingAiArray* free_buildings;			//	0x10
	BuildingAi* buildings;						//	0x14
	u8 player;									//	0x18
	u8 inited;									//	0x19
	u8 worker_count;							//	0x1a
	u8 worker_limit;							//	0x1b
	u8 resource_area;							//	0x1c
	u8 resource_area_not_set;					//	0x1d
	u8 in_battle;								//	0x1e
	u8 unk1f;									//	0x1f
	Point16 position;							//	0x20
	Unit* main_building;						//	0x24
	Unit* building_scv;							//	0x28
	Unit* mineral;								//	0x2c
	Unit* gas_buildings[3];						//	0x30
	TownReq town_units[0x64];					//	0x3c
};
C_ASSERT(sizeof(AiTown) == 0x1cc);

struct AiExpandList
{
	AiTown* last;
	AiTown* first;
};
C_ASSERT(sizeof(AiExpandList) == 0x8);

struct AiSpendingRequest
{
	u8 priority;
	u8 type;
	u16 id;
	void* val;
};

struct SpendingRequest
{
	u8 priority;
	u8 ty;
	u16 id;
	void* val;
};

struct AI_Main
{
	u32 mineralCollection;
	u32 gasCollection;
	u32 supplyCollection;
	u32 minerals_available;
	u32 gas_available;
	u32 supply_available;
	SpendingRequest requests[0x3f];
	u8 request_count;
	u8 liftoff_cooldown;
	u16 AI_NukeRate;
	u16 AI_Attacks;
	u16 AI_LastNukeTime;
	struct
	{
		u16 isSecureFinished : 1;	/*0001*/
		u16 isTownStarted : 1;		/*0002*/
		u16 isDefaultBuildOff : 1;	/*0004*/
		u16 isTransportsOff : 1;		/*0008*/
		u16 isFarmsNotimingOn : 1;	/*0010*/
		u16 isUseMapSettings : 1;	/*0020*/
		u16 flag_0x40 : 1;			/*0040*/
		u16 spreadCreep : 1;			/*0080*/
		u16 flag_0x100 : 1;			/*0100*/
		u16 hasStrongestGndArmy : 1;	/*0200*/ //Set if the AI's ground army is stronger than the total strength of the strongest enemy force
		u16 bUpgradesFinished : 1;	/*0400*/
		u16 bTargetExpansion : 1;	/*0800*/
	} AI_Flags;
	u16 AI_PanicBlock;
	u16 AI_MaxForce;
	u16 AI_AttackGroup;
	u16 wanted_unit;
	u8 dc222[0x2];
	u32 lastIndividualUpdateTime;	// 0x224
	u32 last_attack_seconds;
	u8 strategic_suicide;			// 0x22c
	u8 spellcasterTimer;
	u8 attack_failed;
	u8 difficulty;
	u16 attackers[0x40];				// 0x230
	/*2B0*/  u32 AI_DefenseBuild_GG[10];
	/*2D8*/  u32 AI_DefenseBuild_AG[10];
	/*300*/  u32 AI_DefenseBuild_GA[10];
	/*328*/  u32 AI_DefenseBuild_AA[10];
	/*350*/  u32 AI_DefenseUse_GG[10];
	/*378*/  u32 AI_DefenseUse_AG[10];
	/*3A0*/  u32 AI_DefenseUse_GA[10];
	/*3C8*/  u32 AI_DefenseUse_AA[10];
	u8 unit_build_limits[0xe4];		// 0x3f0
	Unit* mainMedic;					// 0x4d4
	u8 unk4d8[0x10];
};

C_ASSERT(sizeof(AI_Main) == 1256); /*0x4E8*/
//static_assert(sizeof(AI_Main) == 1256, "The size of the AI_Main structure is invalid");

// From BWAPI Offsets.h, used in scbwdata.h by UnitAvailability
struct _uavail
{
	u8 available[PLAYER_COUNT][UNIT_TYPE_COUNT];
};

struct StringTbl
{
	/// Returns the number of string entries in the TBL file.
	u16 getStringCount() const
	{
		return *buffer;
	}

	/// Returns the string at @p index in the TBL file.
	/// If the @p index is 0, returns nullptr.
	/// If the @p index is out of bounds, returns an empty string.
	const char* getString(u16 index) const
	{
		//Based on function @ 0x004C36F0
		if (index == 0) return nullptr;
		else if (index <= getStringCount())
		{
			return (const char*)(buffer)+buffer[index];
		}
		else
		{
			return (const char*)0x00501B7D;	//StringEmpty in scbwdata.h, SC default empty string
		}
	}

	u16* buffer;
};

struct ForceFlags
{
	u8 randomize_start_locations : 1;
	u8 allied : 1;
	u8 allied_victory : 1;
	u8 shared_vision : 1;
	u8 unused : 4;
};

#pragma pack()
