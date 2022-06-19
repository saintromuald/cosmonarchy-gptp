// All-in-one include file
// Structs by poiuy_qwert, A_of_s_t, ravenwolf and pastelmind.
// Special thanks to Heinermann

#pragma once
#include "types.h"

#define AiTownLimit 100
#define ResAreaLimit 250

const s32 PLAYER_COUNT = 12;
const s32 PLAYABLE_PLAYER_COUNT = 8;
const s32 UNIT_ARRAY_LENGTH = 1700;
const s32 BULLET_ARRAY_LENGTH = 100;
const s32 SPRITE_ARRAY_LENGTH = 2500;
const s32 SelectionCap = 12;
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

struct UnitPath
{
	s32		start;
	s32		next;
	s32		finish;
	s32		time_started;
	s32		unit_index;
	s8		is_calculating;
	s8		field_19;
	s8		field_1A;
	s8		path_count;
	s8		num_areas;
	s8		cur_areas;
	s8		num_segments;
	s8		cur_segments;
	s32		steps[24];
};

struct CampaignMission
{
	char*	mission_name;
	s32		mission_index;
};

struct CHKStruct
{
	u8	section[4];
	u32 function;
	u32 flags;
};

struct ScenarioFormat
{
	CHKStruct* chk_struct;
	u32 chkStructCount;
};

struct VectorHead
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

struct StringMap_b
{
	u8		value;				// 0x0
	u8		id;					// 0x1
};

struct StringMap_w
{
	u16		tblstring;			// 0x0
	u16		index;				// 0x2
};

struct DialogEvent
{
	u32		user;				// 0x0
	u16		field_4;			// 0x4
	u16		field_6;			// 0x6
	u16		wVirtKey;			// 0x8
	u16		wVirtKeyMod;		// 0xA
	u16		no;					// 0xC
	Point16	current;			// 0xE
	u16		field_12;			// 0x12
};

struct Base
{
	Point16	position;			// 0x0
	u8		unk4;				// 0x4
	u8		unk5;				// 0x5
	u8		unk6;				// 0x6
	u8		flags;				// 0x7
	u32		unk8;				// 0x8
	u32		unkC;				// 0xC
	u32		unk10;				// 0x10
	u32		unk14;				// 0x14
	u32		unk18;				// 0x18
	u32		unk1C;				// 0x1C
	u32		unk20;				// 0x20
	u32		unk24;				// 0x24
	u32		unk28;				// 0x28
	u32		unk2C;				// 0x2C
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
	u8		resource_type;				// 0x76
	u8		useStandardUnitStats;		// 0x77
	u8		fogOfWar;					// 0x78
	u8		starting_units;				// 0x79
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
	u32		mouse_scroll;				// 0x4
	u32		keyboard_scroll;			// 0x8
	u32		music;						// 0xC
	u32		sfx;						// 0x10
	u32		tip_number;					// 0x14
	u32		flags;						// 0x18
	u32		m_mouse_scroll;				// 0x1C
	u32		m_keyboard_scroll;			// 0x20
	u32		use_chat_colors;			// 0x24
	u32		null;						// 0x28
};
C_ASSERT(sizeof(Options) == 0x2C);

struct PlayerName
{
	char	player_name[0x19];			// 0x0
};

struct MenDeaths
{
	u32        player[12];
};
struct UnitDeaths
{
	u32        player[12];
};
struct UnitScores_I
{
	u32        player[12];
};
// Based on http://code.google.com/p/bwapi/source/browse/branches/bwapi4/bwapi/BWAPI/Source/BW/Image.h

struct Text
{
	char	text_display[218][13];  // last is text at bottom; ex: "Cheat enabled."
	unk_t   unknown1;
	unk_t   unknown2;
	u8		color[13];
	unk_t   unknown3[3];			// last is text at bottom; ex: "Cheat enabled."
	u8		timer[13];
};

struct Tile
{
	/*00*/  u16 index;
	/*02*/  u8 buildability;	/**< 8th bit should sign not buildable. */
	/*03*/  u8 ground_height;	/**< Ground Height(4 lower bits) - Deprecated? Some values are incorrect. */
	/*04*/  u16 left_edge;
	/*06*/  u16 top_edge;
	/*08*/  u16 right_edge;
	/*0A*/  u16 bottom_edge;
	/*0C*/  u16 _1;
	/*0E*/  u16 _2; /**<  Unknown - Edge piece has rows above it. (Recently noticed; not fully understood.)
					o 1 = Basic edge piece.
					o 2 = Right edge piece.
					o 3 = Left edge piece.*/
	/*10*/  u16 _3;
	/*12*/  u16 _4;
	/*14*/  u16 minitile[16]; /** MegaTile References (VF4/VX4) */
};

struct Doodad
{
	/*00*/  u16 index;
	/*02*/	u8	buildability; /**< 8th bit should sign not buildable. */
	/*03*/  u8	ground_height_and_overlay_flags; /**< Ground Height(4 lower bits) - Deprecated? Some values are incorrect.
					   * Overlay Flags:
					   * o 0x0 - None
					   * o 0x1 - Sprites.dat Reference
					   * o 0x2 - Units.dat Reference (unit sprite)
					   * o 0x4 - Overlay is Flipped
					   */
	/*04*/  u16 overlayID;
	/*06*/  u16 _1;
	/*08*/  u16 doodad_group_string;
	/*0A*/  u16 _2;
	/*0C*/  u16 data_bin_index;
	/*0E*/  u16 doodad_height;
	/*10*/  u16 doodad_width;
	/*12*/  u16 _3;
	/*14*/  u16 minitile[16]; /** MegaTile References (VF4/VX4) */
}; // count 34

struct Thingy
{
	/*00*/  Thingy* prev;
	/*04*/  Thingy* next;
	/*08*/  u32     unit_id;
	/*0C*/  Sprite* sprite;
};

struct CHKUnit
{
	/*00*/	u32	 chkid;
	/*04*/	u16	 x;
	/*06*/	u16	 y;
	/*08*/	u16	 unit_id;
	/*0A*/	u16	 relation;
	/*0C*/	u16	 valid_state_flags;
	/*0E*/	u16	 valid_properties;
	/*10*/	u8	 player;
	/*11*/	u8	 hp;
	/*12*/	u8	 shields;
	/*13*/	u8	 energy;
	/*14*/	u16	 resources;
	/*16*/	u16	 field_16;
	/*18*/	u16	 hangar;
	/*1A*/	u16	 state_flags;
	/*1C*/	u32	 field_1c;
	/*20*/	u32	 related_unit;
};

//---- Taken from locations.cpp ----//

struct MapSize
{
	u16   width;
	u16   height;
};

struct Location
{
	Box32 dimensions;
	u16   string_id;
	u16   flags;
};

//---- Taken from buttons.cpp ----//

typedef u32 ButtonState;
typedef ButtonState(__fastcall* ReqFunc)(u32, u32, Unit*);
typedef void(__fastcall* ActFunc)(u32, u32);

struct Button
{
	/*00*/  u16		 position;
	/*02*/  u16		 icon_id;
	/*04*/  ReqFunc* requirement_func;
	/*08*/  ActFunc* action_func;
	/*0C*/  u16		 requirement_var;
	/*0E*/  u16		 action_var;
	/*10*/  u16		 requirement_string_id;
	/*12*/  u16		 action_string_id;
};

struct StatusScreenState
{
	GrpHead*	button_grp;	// 0x0
	u16			grp_frame;	// 0x4
	u16			type;		// 0x6
	u8			id;			// 0x8	
};

struct ButtonSet
{
	u32			buttons_in_set;
	Button*		first_button;
	u32			connected_unit;
};

//---- Taken from triggers.h ----//
struct Condition
{
	u32			location;
	u32			player;
	u32			number;
	u16			unit;
	u8			comparison;
	u8			condition;
	u8			type;
	u8			flags;
};

typedef b32(__fastcall* ConditionPointer)(Condition*);

struct Action
{
	u32        location;
	u32        string;
	u32        wavString;
	u32        time;
	u32        player;
	u32        number;
	u16        unit;
	u8         action;
	u8         number2;
	u8         flags;
};
typedef b32(__fastcall* ActionPointer)(Action*);

//GRP frame header
struct GrpFrame
{
	s8  x;
	s8  y;
	s8  width;
	s8  height;
	u32 data_offset; // might be u16* (but as a pointer, would still fit in u32)
};
C_ASSERT(sizeof(GrpFrame) == 8);
//static_assert(sizeof(GrpFrame) == 8, "The count of the GrpFrame structure is invalid");

//GRP file header
struct GrpHead
{
	u16			frameCount;
	s16			width;
	s16			height;
	GrpFrame	frames[1];
};

C_ASSERT(sizeof(GrpHead) == 14);
//static_assert(sizeof(GrpHead) == 14, "The count of the GrpHead structure is invalid");

// LO* file header
struct LO_Header
{
	u32 frameCount;
	u32 overlayCount;
	u32 frameOffsets[1];

	Point8 get_offset(s32 frameNum, s32 overlayNum) const {
		return ((Point8*)(this->frameOffsets[frameNum] + (u32)this))[overlayNum];
	}
};

C_ASSERT(sizeof(LO_Header) == 12);
//static_assert(sizeof(LO_Header) == 12, "The count of the LO_Header structure is invalid");

// Image Remapping Data
struct ColorShiftData
{
	u32		index;
	void*	data;
	char	name[12];
};

C_ASSERT(sizeof(ColorShiftData) == 20);
//static_assert(sizeof(ColorShiftData) == 20, "The count of the ColorShiftData structure is invalid");

//-------- Flag structures --------//

// Based on BWAPI's Offsets.h
union ActiveTile
{
	u32 value;
	struct
	{
	/*00*/  u8 visibility_flags;			// 0x000000ff
	/*01*/  u8 explored_flags;				// 0x0000ff00
	/*02*/  u8 is_walkable : 1;				// 0x00010000 Set on tiles that can be walked on
			u8 unknown1 : 1;				// 0x00020000 Unused?
			u8 is_unwalkable : 1;			// 0x00040000 Set on tiles that can't be walked on
			u8 unknown2 : 1;				// 0x00080000 Unused?
			u8 has_doodad_cover : 1;		// 0x00100000 Set when a doodad cover (trees) occupies the area
			u8 unknown3 : 1;				// 0x00200000 Unused?
			u8 has_creep : 1;				// 0x00400000 Set when creep occupies the area
			u8 always_unbuildable : 1;		// 0x00800000 always unbuildable, like water
	/*03*/  u8 ground_height : 3;			// 0x07000000 ground height; 1 for low ground, 2 for middle ground and 4 for high ground
			u8 currently_occupied : 1;		// 0x08000000 unbuildable but can be made buildable
			u8 creep_receeding : 1;			// 0x10000000 Set when the nearby structure supporting the creep is destroyed
			u8 cliff_edge : 1;				// 0x20000000 Set if the tile is a cliff edge
			u8 temporary_creep : 1;			// 0x40000000 Set when the creep occupying the area was created. Not set if creep tiles were preplaced. Used in drawing routine.
			u8 unknown4 : 1;				// 0x80000000 Unused?
	};
};
C_ASSERT(sizeof(ActiveTile) == 4);

struct GroupFlag
{
	u8 is_zerg			: 1; /*0x01*/
	u8 is_terran		: 1; /*0x02*/
	u8 is_protoss		: 1; /*0x04*/
	u8 is_men			: 1; /*0x08*/
	u8 is_building		: 1; /*0x10*/
	u8 is_factory		: 1; /*0x20*/
	u8 is_independent	: 1; /*0x40*/
	u8 is_neutral		: 1; /*0x80*/
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
//static_assert(sizeof(TargetFlag) == 2, "The count of the TargetFlag structure is invalid");

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
//static_assert(sizeof(UnitFinderData) == 8, "The count of the UnitFinderData structure is invalid");

// BinDlg is defined in https://code.google.com/p/vgce/source/browse/trunk/docs/Blizzard/Starcraft/BIN%20FORMAT.txt as Dialog Structure
// That structure is variable depending on the content

struct BinDlg
{
	/*0x00*/  BinDlg*		next;
	/*0x04*/  Bounds		bounds;			// 0x4 left,0x6 top,0x8 right,0xA bottom,0xC width,0xE height
	/*0x10*/  u8*			buffer;
	/*0x14*/  char*			psz_text;
	/*0x18*/  u32			flags;			// use BinDlgFlags::Enum
	/*0x1C*/  u32			unk_1c;
	/*0x20*/  u16			index;
	/*0x22*/  u16			control_type;	// use DialogControlTypes::Enum
	/*0x24*/  u16			graphic;

	union
	{
		/*0x26*/  u32*		user;
		/*0x26*/  Unit*		unit_user;
		/*0x26*/  BinDlg*	dialog_user;
		/*0x26*/  Button*	button_user;
		StatusScreenState*	stat_button_user; // from Neiv's list
		/*0x26*/  u32		non_user_value;
	};

	/*0x2A*/  void*			fxn_interact;
	/*0x2E*/  void*			fxn_update;
	/*0x32*/  BinDlg*		parent;

	union {
		struct
		{
			/*0x36*/  Box16		response_area;
			/*0x3E*/  BinDlg*	unk_3e;
		} initialVersion0x36;
		struct
		{
			/*0x36*/  s16		response_area_width;	// guess
			/*0x38*/  s16		response_area_height;	// guess
			/*0x3A*/  u32*		response_area_bit_array;// set with a memory allocation function, bit array is a guess
			/*0x3E*/  u32*		unk3e;					// unverified filler
		} onInitDialog0x36;
		struct
		{
			/*0x36*/  void*		unk_func_36;			// should be a function
			/*0x3A*/  u16		scrollMinValue;			// unconfirmed
			/*0x3C*/  u16		scrollMaxValue;			// unconfirmed
			/*0x3E*/  u16		scrollCurrentValue;		// unconfirmed
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
			/*0x3E*/  u8		pszTextLen;				// should be the length of psz_text
			/*0x3F*/  unk_t     unk3f[3];				// unverified filler
		} textDialog0x36;								// seems to apply to some text related dialogs
	};

	union {
		/*0x42*/  void*			children_smk;
		/*0x42*/  BinDlg*		children_dialog;
	};

	union {
		struct
		{
			/*0x46*/  Point16	textPos;
			/*0x4A*/  u16		response_area_width;
			/*0x4C*/  u16		response_area_height;
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
//static_assert(sizeof(BinDlg) == 86, "The count of the BinDlg structure is invalid");

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
//static_assert(sizeof(SMK_Structure) == 30, "The count of the SMK_Structure structure is invalid");

struct GuiOverlay
{
	/*0x00*/  unk_t     unk_0[6];
	/*0x06*/  u16		overlay_type;
	/*0x08*/  u16		id;
	/*0x0A*/  unk_t     unk_a[34];
};

C_ASSERT(sizeof(GuiOverlay) == 44); //0x2C
//static_assert(sizeof(GuiOverlay) == 44, "The count of the GuiOverlay structure is invalid");

//-------- AI related stuff --------//

struct AiTown;
struct MilitaryAi;
struct MilitaryAiArray; // fuck you visual studio
struct AiRegion
{
	/*0x00*/  u16 region;
	/*0x02*/  u16 target_region;
	/*0x04*/  u8  player;
	/*0x05*/  u8  state;
	/*0x06*/  u8  defense_priority_add;
	/*0x07*/  u8  defend_timeout;
	/*0x08*/  u8  captain_flags;
	/*0x09*/  u8  unknown_0x9;
	/*0x0A*/  u16 ground_unit_count;
	/*0x0C*/  u16 ground_strength_needed;
	/*0x0E*/  u16 air_strength_needed;
	/*0x10*/  u16 ground_strength;
	/*0x12*/  u16 air_strength;
	/*0x14*/  u16 ground_strength_full;
	/*0x16*/  u16 air_strength_full;
	/*0x18*/  u16 air_strength_enemy;
	/*0x1A*/  u16 ground_strength_enemy;
	/*0x1C*/  Unit* air_target;
	/*0x20*/  Unit* ground_target;
	/*0x24*/  Unit* slowest_unit;
	/*0x28*/  Unit* follow_target;
	/*0x2C*/  MilitaryAiArray* military;
	/*0x30*/  MilitaryAi* first_military_ai;
};

C_ASSERT(sizeof(AiRegion) == 52);

struct Region
{
	u16		accessibility_flags;	// 0x00
	u16		group_index;			// 0x02
	u16		tile_count;				// 0x04
	u8		path_count;				// 0x06
	u8		neighbor_count;			// 0x07
	u32		field_8;				// 0x08
	u16*	neighbors;				// 0x0C
	u32		region_center_x;		// 0x10
	u32		region_center_y;		// 0x14
	u16		left;					// 0x18
	u16		top;					// 0x1A
	u16		right;					// 0x1C
	u16		bottom;					// 0x1E
	u32		properties;				// 0x20
	u32		field_24;				// 0x24
	u32		field_28;				// 0x28
	u16		local_data[10];			// 0x2C
};
C_ASSERT(sizeof(Region) == 64);

struct RegionArray
{
	u16 regionCount;
	u8 _dc2[0x449fa];
	Region regions[5000];
	u8 _dc92bfc[0x4e24];
};

struct GuardAi
{
	Link<GuardAi> link;				// 0x0, 0x4
	u8 type;						// 0x8
	u8 times_died;					// 0x9
	u8 unk_a[2];					// 0xA
	Unit* parent;					// 0xC
	u16 unit_id;					// 0x10
	Point16 home;					// 0x12
	Point16 unk_pos;				// 0x16
	u8 unk_1a[2];					// 0x1A
	u32 previous_update;			// 0x1C
};
C_ASSERT(sizeof(GuardAi) == 32); // 0x20

struct AiTown; // declaring this here to avoid compiler errors
struct WorkerAi
{
	Link<WorkerAi> link;			// 0x0, 0x4
	u8 type;						// 0x8
	u8 target_resource;				// 0x9
	u8 reassign_count;				// 0xA
	u8 wait_timer;					// 0xB
	u32 last_update_second;			// 0xC
	Unit* parent;					// 0x10
	AiTown* town;					// 0x14
};
C_ASSERT(sizeof(WorkerAi) == 24); // 0x18

struct BuildingAi
{
	Link<BuildingAi> link;			// 0x0, 0x4
	u8 type;						// 0x8
	u8 trainQueueTypes[5];			// 0x9
	u8 unkE[2];						// 0xE
	Unit* parent;					// 0x10
	AiTown* town;					// 0x14
	void* trainQueueValues[5];		// 0x18
};
C_ASSERT(sizeof(BuildingAi) == 44); // 0x2C

struct MilitaryAi
{
	Link<MilitaryAi> link;			// 0x0, 0x4
	u8 type;						// 0x8
	u8 padding1;					// 0x9
	u16 padding2;					// 0xA
	Unit* unit;						// 0xC
	AiRegion* region;				// 0x10
};
C_ASSERT(sizeof(MilitaryAi) == 20); // 0x14

struct UnitAi
{
	union {
		struct {
			Link<UnitAi> link;			// 0x0, 0x4
			u8 type;	                // 0x8
		};
		GuardAi guard_ai;
		WorkerAi worker_ai;
		BuildingAi building_ai;
		MilitaryAi military_ai;
	};
};

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

struct GuardAiArray
{
	GuardAi guard_ais[1000];
	GuardAi* first_free_guard;
};

struct GuardAiList
{
	GuardAiArray* free_ais;
	GuardAi* first;
};

struct GuardAiListPlayers {
	GuardAiList player[8];
};

struct TownReq
{
	union {
		u8 flags_and_count;
		struct {
			u8 only_if_needed : 1;
			u8 is_upgrade : 1;
			u8 is_tech : 1;
			u8 count : 5;
		};
	};
	u8 priority;
	u16 id;
};

struct AiTown
{
	AiTown* prev;						//	0x0
	AiTown* next;						//	0x4
	WorkerAiArray* free_workers;		//	0x8
	WorkerAi* workers;					//	0xC
	BuildingAiArray* free_buildings;	//	0x10
	BuildingAi* buildings;				//	0x14
	u8 player;							//	0x18
	u8 inited;							//	0x19
	u8 worker_count;					//	0x1a
	u8 worker_limit;					//	0x1b
	u8 resarea;							//	0x1c
	u8 resource_units_not_set;			//	0x1d
	u8 inBattle;						//	0x1e
	u8 unk1f;							//	0x1f
	Point16 position;					//	0x20
	Unit* main_building;				//	0x24
	Unit* building_SCV;					//	0x28
	Unit* mineral;						//	0x2c
	Unit* gas_buildings[3];				//	0x30
	TownReq town_units[0x64];			//	0x3c
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
	u8		priority;
	u8		type;
	u16		id;
	void*	val;
};

struct AI_Main
{
	u32 mineral_collection;				/*0000*/
	u32 gas_collection;					/*0004*/
	u32 supply_collection;				/*0008*/
	u32 minerals_available;				/*000C*/
	u32 gas_available;					/*0010*/
	u32 supply_available;				/*0014*/
	SpendingRequest requests[0x3f];		/*0018*/
	u8 request_count;					/*0210*/
	u8 liftoff_cooldown;				/*0211*/
	u8 ai_nuke_rate;					/*0212*/
	u8 ai_attacks;						/*0213*/
	u32 ai_last_nuke_time;				/*0214*/
	struct
	{
		u16 is_secure_finished : 1;			/*0001*/
		u16 is_town_started : 1;			/*0002*/
		u16 is_default_build_off : 1;		/*0004*/
		u16 is_transports_off : 1;			/*0008*/
		u16 is_farms_notiming_on : 1;		/*0010*/
		u16 is_use_map_settings : 1;		/*0020*/
		u16 flag_0x40 : 1;					/*0040*/
		u16 spread_creep : 1;				/*0080*/
		u16 flag_0x100 : 1;					/*0100*/
		u16 has_strongest_ground_army : 1;	/*0200*/ // Set if the AI's ground army is stronger than the total strength of the strongest enemy force
		u16 upgrades_finished : 1;			/*0400*/
		u16 target_expansion : 1;			/*0800*/
	} AI_Flags;
	u16 ai_panic_block;					/*021A*/
	u16 ai_max_force;					/*021C*/
	u16 ai_attack_group;				/*021E*/
	u16 wanted_unit;					/*0220*/
	u8	dc222;							/*0222*/
	u8	defense_priority_base;			/*0223*/
	u32 previous_building_hit_second;	/*0224*/
	u32 last_attack_seconds;			/*0228*/
	u8	strategic_suicide;				/*022C*/
	u8	spellcaster_timer;				/*022D*/
	u8	attack_failed;					/*022E*/
	u8	difficulty;						/*022F*/
	u16 attackers[0x40];				/*0230*/
	u32 ai_defensebuild_gg[10];			/*02B0*/
	u32 ai_defensebuild_ag[10];			/*02D8*/
	u32 ai_defensebuild_ga[10];			/*0300*/
	u32 ai_defensebuild_aa[10];			/*0328*/
	u32 ai_defenseuse_gg[10];			/*0350*/
	u32 ai_defenseuse_ag[10];			/*0378*/
	u32 ai_defenseuse_ga[10];			/*03A0*/
	u32 ai_defenseuse_aa[10];			/*03C8*/
	u8 unit_build_limits[0xe4];			/*03F0*/
	Unit* main_medic;					/*04D4*/
	u8 unk4d8[0x10];					/*04D8*/
};
C_ASSERT(sizeof(AI_Main) == 1256); /*0x4E8*/

struct AiScript
{
	Link<AiScript>		link;			// 0x0, 0x4
	s32					script_offset;	// 0x8
	s32					sleep;			// 0xC
	s32					player;			// 0x10
	Box32				loc_bounds;		// 0x14
	Vector32			loc_center;		// 0x24
	AiTown*				town;			// 0x2C
	s32					flags;			// 0x30
};
C_ASSERT(sizeof(AiScript) == 52);

struct ResourceArea {
	Point16 position;
	u8 mineral_field_count;
	u8 geyser_count;
	bool is_start_location;
	u8 flags;					// 0x1 = Occupied / start location, 0x2 = allies_watch claimed(0069268F)
	u32 total_mineral_count;
	u32 total_gas_count;
	u8 padding[32];
};
C_ASSERT(sizeof(ResourceArea) == 48);

struct ResourceAreaArray {
	ResourceArea areas[250];
	u32 count;
	u32 frames_till_update;
};

// From BWAPI Offsets.h, used in scbwdata.h by unit_availability
struct _uavail
{
	u8 available[PLAYER_COUNT][UNIT_TYPE_COUNT];
};

struct StringTbl
{
	/// Returns the number of string entries in the TBL file.
	u16 get_string_count() const
	{
		return *buffer;
	}

	/// Returns the string at @p index in the TBL file.
	/// If the @p index is 0, returns nullptr.
	/// If the @p index is out of bounds, returns an empty string.
	const char* get_string(u16 index) const
	{
		//Based on function @ 0x004C36F0
		if (index == 0) return nullptr;
		else if (index <= get_string_count())
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

struct Ping
{
	b32 isActive;
	u32 x;
	u32 y;
	u16 timer;
	u16 player;
};

struct DSound
{
	s32 lpVtbl;
};

struct CritSect
{
	s32 debugInfo;
	s32 lockCount;
	s32 recursionCount;
	s32 owningThread;
	s32 lockSemaphore;
	s32 spinCount;
};

struct FogMask
{
//	u8  unexplored[1980];	// 0x0 (0x00657AA0)
//	u8  explored[2112];		// 0x7BC (0x0065825C)
	u8  unexplored[4092];
	u8  visible[4];			// 0x840 (0x00658A9C)
};

struct ColorTable
{
	u8  playerColorId[8];
};

struct PlayerNamesEntry {
	char player_name[25];
	char force_name[30];
	char some_stuff[17];
};

struct PlayerNames
{
	PlayerNamesEntry player[8];
};

struct MovementGroup {
	Point16 pos;
	Point16 current_unit_target;
	Point16 pathed_pos;
	Point16 nonpathed_pos;
	s32 unit_offset;
	s16 sqrt_2_times_unit_area_sum;
	s16 unit_area_sum;
	s32 flags;
};

struct Event {
	u32 extended_command_type;
	u32 ext_param;
	u32 value;
	u16 type;
	u16 x_pos;
	u16 y_pos;
};

struct FindUnitLocationParam {
	u32		player;
	u16		unit_id;
	u16		location_flags;
	u32		check_height;
};

struct KillUnitArgs {
	void*	func; // IsValid(), arg 1 Unit *unit, ecx player, edx unit_id
	u32		player;
	u16		unit_id;
	u16		flags;
	u32		check_height;
};

struct FxnDrawIconData {
	u16 w;
	u16 h;
	GrpHead* stat_icon;
};

#pragma pack()