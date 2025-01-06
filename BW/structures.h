// All-in-one include file
// Structs by poiuy_qwert, A_of_s_t, ravenwolf and pastelmind.
// Special thanks to Heinermann

#pragma once
#include "types.h"

// New Extender Limits
#define UnitLimit 4095
#define UnitIDMask 0xFFF // Happens to be the same value as UnitLimit here, but it doesn't necessarily have to be.
#define SpriteLimit 8875 // Must be a multiple of 5, as well as a multiple of 3 when decremented by one.
#define OrderLimit 4080
#define NewCheckLimit (OrderLimit - (OrderLimit / 10)) // Reserve 10% of order array for non-queued orders.
#define ImageLimit 16385 // Must be a multiple of 5.
#define ThingyLimit 2048
#define FowSpriteLimit 2048
#define BulletLimit 2048

// Const Limits (Do not modify!)
#define AiTownLimit 100
#define ResAreaLimit 250

#define PlayerLimit 8
#define FullPlayerLimit 12
#define NeutralPlayer 11
#define ScoutingPlayerLimit (PlayerLimit + 1)

const s32 PLAYER_COUNT = 12;
const s32 PLAYABLE_PLAYER_COUNT = PlayerLimit;
const s32 UNIT_ARRAY_LENGTH = UnitLimit; // 1700
const s32 BULLET_ARRAY_LENGTH = BulletLimit; // 100
const s32 SPRITE_ARRAY_LENGTH = SpriteLimit; // 2500
const s32 IMAGE_ARRAY_LENGTH = ImageLimit; // 5000
const s32 THINGY_ARRAY_LENGTH = ThingyLimit; // 500;

/*
const s32 UNIT_TYPE_COUNT = 228;
const s32 TECH_TYPE_COUNT = 44;
const s32 UPGRADE_TYPE_COUNT = 61;
const s32 WEAPON_TYPE_COUNT = 256;
const s32 FLINGY_TYPE_COUNT = 209;
const s32 IMAGE_TYPE_COUNT = 999;
const s32 SOUND_TYPE_COUNT = 1144;
*/

// use DatExt variables
extern int UNIT_TYPE_COUNT;		//= 228;
extern int TECH_TYPE_COUNT;		//= 44;
extern int UPGRADE_TYPE_COUNT;	//= 61;
extern int WEAPON_TYPE_COUNT;	//= 130;
extern int FLINGY_TYPE_COUNT;	//= 209;
extern int IMAGE_TYPE_COUNT;	//= 999;

#include "structures/Unit.h"
#include "structures/Bullet.h"
#include "structures/Sprite.h"
#include "structures/Image.h"
#include "structures/Order.h"
#include "structures/Layer.h"
#include "structures/Player.h" // made into external file to have .cpp

#pragma pack(1)

typedef b32 __fastcall unit_stat_cond();
typedef void __fastcall unit_stat_act();
struct UnitStatFuncArrayEntry {
	u32 unit_id;					// unused?
	u32 unit_stat_cond_func;
	u32 unit_stat_act_func;
};

struct UnitPath
{
	Point16		start;					 // 0x00
	s32			next;					 // 0x04
	s32			finish;					 // 0x08
	s32			time_started;			 // 0x0C
	s32			unit_index;				 // 0x10
	u32			x_y_speed;				 // 0x14
	s8			is_calculating;			 // 0x18
	s8			collision_recheck_timer; // 0x19
	s8			direction;				 // 0x1A
	s8			path_count;				 // 0x1B
	s8			num_areas;				 // 0x1C
	s8			cur_areas;				 // 0x1D
	s8			num_segments;			 // 0x1E
	s8			cur_segments;			 // 0x1F
	s32			steps[24];				 // 0x20
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
	CHKStruct*	chk_struct;
	u32			chk_struct_count;
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
	u8		value;					// 0x0
	u8		id;						// 0x1
};

struct StringMap_w
{
	u16		tbl_string;				// 0x0
	u16		index;					// 0x2
};

struct DialogEvent
{
	u32		user;					// 0x0
	u16		field_4;				// 0x4
	u16		field_6;				// 0x6
	u16		virtual_key;			// 0x8
	u16		virtual_key_modifier;	// 0xA
	u16		number_order;			// 0xC
	Point16	current;				// 0xE
	u16		field_12;				// 0x12
};

// Seems to be identical to ResourceArea
struct Base
{
	Point16	position;				// 0x0
	u8		mineral_field_count;	// 0x4
	u8		geyser_count;			// 0x5
	u8		is_start_loc;			// 0x6
	u8		flags;					// 0x7
	u32		total_mineral_count;	// 0x8
	u32		total_vespene_count;	// 0xC
	u32		unk10;					// 0x10
	u32		unk14;					// 0x14
	u32		unk18;					// 0x18
	u32		unk1C;					// 0x1C
	u32		unk20;					// 0x20
	u32		unk24;					// 0x24
	u32		unk28;					// 0x28
	u32		unk2C;					// 0x2C
};

struct GameData // taken from IDA and from https://gist.githubusercontent.com/neivv/d56baedfdfa0145575825e160b64adb7/raw/1bfc4e2113966eac0749f7f02de3fa9a091fb84b/nuottei.txt
{ 
	u32		save_time;					// 0x0
	char	game_name[0x18];			// 0x4
	u32		save_timestamp;				// 0x1C
	u16		map_width_tiles;			// 0x20
	u16		map_height_tiles;			// 0x22
	u8		active_player_count;		// 0x24
	u8		max_player_count;			// 0x25
	u8		game_speed;					// 0x26
	u8		game_state;					// 0x27
	u16		g_game_type;				// 0x28
	u16		game_subtype;				// 0x2A
	u32		cdkey_hash;					// 0x2C
	u16		tileset;					// 0x30
	u8		is_replay;					// 0x32
	u8		computer_player_count;		// 0x33
	u8		host_name[0x19];			// 0x34
	char	map_title[0x20];			// 0x4D
	u16		got_game_type;				// 0x6D
	u16		got_game_subtype;			// 0x6F
	u16		got_game_subtype_display;	// 0x71
	u16		got_game_subtype_label;		// 0x73
	u8		victory_condition;			// 0x75
	u8		resource_type;				// 0x76
	u8		use_standard_unit_stats;	// 0x77
	u8		fog_of_war;					// 0x78
	u8		starting_units;				// 0x79
	u8		use_fixed_positions;		// 0x7A
	u8		restriction_flags;			// 0x7B
	u8		allies_enabled;				// 0x7C
	u8		teams_enabled;				// 0x7D
	u8		cheats_enabled;				// 0x7E
	u8		tournament_mode_enabled;	// 0x7F
	u32		victory_condition_value;	// 0x80
	u32		resource_mineral_value;		// 0x84
	u32		resource_gas_value;			// 0x88
};
C_ASSERT(sizeof(GameData) == 0x8C);

struct ReplayData // taken from https://gist.githubusercontent.com/neivv/d56baedfdfa0145575825e160b64adb7/raw/1bfc4e2113966eac0749f7f02de3fa9a091fb84b/nuottei.txt
{ 
	u32		replay_data_start;			// 0x0
	u32		bool_unk;					// 0x4
	u32		replay_frame_begin;			// 0x8
	u32		length[0x2];				// 0xC
	u32		replay_frame_end;			// 0x14
	u32		latest_command_frame;		// 0x18
	u32		replay_frame_pos;			// 0x1C
};
C_ASSERT(sizeof(ReplayData) == 0x20);

struct ReplayFrame // taken from https://gist.githubusercontent.com/neivv/d56baedfdfa0145575825e160b64adb7/raw/1bfc4e2113966eac0749f7f02de3fa9a091fb84b/nuottei.txt
{
	u32		frame;						// 0x0
	u8		data_length;				// 0x4
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
					   * Overlay sound_flags:
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
	/*10*/  u16		 requirement_string_id; // Seems like this should be action,
	/*12*/  u16		 action_string_id;		// and this should be requirement. -Pr0nogo
};

struct StatusScreenState
{
	GrpHead*	button_grp;	// 0x0
	u16			grp_frame;	// 0x4
	u16			type;		// 0x6
	u8			id;			// 0x8
};

struct PortraitData
{
	u16			dims_x;			// 0x0
	u16			dims_y;			// 0x2
	void*		bmp_buffer;		// 0x4
	void*		smk;			// 0x8
	u16			flags;			// 0xC
	u16			portrait_id;	// 0xE
	u8			anim_type;		// 0x10
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
	u8  x;
	u8  y;
	u8  w;
	u8  h;
	u32 data_offset; // might be u16* (but as a pointer, would still fit in u32)
};
C_ASSERT(sizeof(GrpFrame) == 8);
//static_assert(sizeof(GrpFrame) == 8, "The count of the GrpFrame structure is invalid");

//GRP file header
struct GrpHead
{
	u16			frame_count;
	s16			width;
	s16			height;
	GrpFrame	frames[1];
};

C_ASSERT(sizeof(GrpHead) == 14);
//static_assert(sizeof(GrpHead) == 14, "The count of the GrpHead structure is invalid");

// LO* file header
struct LO_Header
{
	u32 frame_count;
	u32 overlay_count;
	u32 frame_offsets[1];

	Point8 get_offset(s32 frameNum, s32 overlayNum) const {
		return ((Point8*)(this->frame_offsets[frameNum] + (u32)this))[overlayNum];
	}
};

C_ASSERT(sizeof(LO_Header) == 12);
//static_assert(sizeof(LO_Header) == 12, "The count of the LO_Header structure is invalid");

// Image remapping Data
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
struct ActiveTile {
	union {
		u32 flags;
		struct {
/*00000f*/ u8 visibility_flags;
/*0000f0*/ u8 explored_flags;
/*000100*/ u8 is_walkable : 1; // Set on tiles that can be walked on
/*000200*/ u8 unknown1 : 1; // Unused?
/*000400*/ u8 is_unwalkable : 1; // Set on tiles that can't be walked on
/*000800*/ u8 unknown2 : 1; // Unused?
/*001000*/ u8 has_doodad_cover : 1; // Set when a doodad cover (trees) occupies the area
/*002000*/ u8 unknown3 : 1; // Unused?
/*004000*/ u8 has_cagra : 1; // Set when creep occupies the area
/*008000*/ u8 always_unbuildable : 1; // always unbuildable, like water
/*070000*/ u8 ground_height : 3; // inside height; 1 for low inside, 2 for middle inside and 4 for high inside
/*080000*/ u8 currently_occupied : 1; // unbuildable but can be made buildable
/*100000*/ u8 cagra_receding : 1; // Set when the nearby structure supporting the creep is destroyed
/*200000*/ u8 cliff_edge : 1; // Set if the tile is a cliff edge
/*400000*/ u8 temporary_cagra : 1; // Set when the creep occupying the area was created. Not set if creep tiles were preplaced. Used in drawing routine.
/*800000*/ u8 unknown4 : 1; // Unused?
		};
	};
};
C_ASSERT(sizeof(ActiveTile) == 4);

struct ActiveTileDF {
	union {
		u32 tileFlags;
		struct {
	/*00*/	u8 revealedMask;
	/*01*/	u8 unexploredMask;
	/*02*/	u8 isWalkable : 1; // Set on tiles that can be walked on
			u8 unknown1 : 1;
			u8 isUnwalkable : 1; // Set on tiles that can't be walked on
			u8 unknown2 : 1;
			u8 hasDoodadCover : 1; // Set when a doodad cover (trees) occupies the area
			u8 unknown3 : 1;
			u8 hasCreep : 1; // Set when creep occupies the area
			u8 alwaysUnbuildable : 1; // always unbuildable, like water
	/*03*/	u8 blocksSight : 1;
			u8 groundHeight : 2; // inside height; 1 for low inside, 2 for middle inside and 4 for high inside
			u8 currentlyOccupied : 1; // unbuildable but can be made buildable
			u8 creepReceeding : 1; // Set when the nearby structure supporting the creep is destroyed
			u8 cliffEdge : 1; // Set if the tile is a cliff edge
			u8 temporaryCreep : 1; // Set when the creep occupying the area was created. Not set if creep tiles were preplaced. Used in drawing routine.
			u8 unknown4 : 1;
		};
	};
};
C_ASSERT(sizeof(ActiveTileDF) == 4);

union GroupFlag
{
	struct {
		u8 is_zerg			: 1; /*0x01*/
		u8 is_terran		: 1; /*0x02*/
		u8 is_protoss		: 1; /*0x04*/
		u8 is_men			: 1; /*0x08*/
		u8 is_building		: 1; /*0x10*/
		u8 is_factory		: 1; /*0x20*/
		u8 is_independent	: 1; /*0x40*/
		u8 is_neutral		: 1; /*0x80*/
	};
	u8 value;
};
C_ASSERT(sizeof(GroupFlag) == 1);

struct TargetFlag
{
	u16 air : 1;
	u16 ground : 1;
	u16 mechanical : 1;
	u16 organic : 1;
	u16 non_building : 1;
	u16 non_robotic : 1;
	u16 terrain : 1;
	u16 organic_or_mechanical : 1;
	u16 player_owned : 1;
};
C_ASSERT(sizeof(TargetFlag) == 2);
//static_assert(sizeof(TargetFlag) == 2, "The count of the TargetFlag structure is invalid");

//-------- End of flag structures --------//

struct UnitFinderData
{
	s32 unit_index;
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
	/*0x14*/  char*			text;
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
		GrpHead*			grp_head_user;
		PortraitData*		portrait_data;
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
		struct
		{
			/*0x36*/  u32		time_cost;
			/*0x3A*/  u32*		font_handle;
			/*0x3E*/  BinDlg*	player_name;
		} statRes;
	};

	union {
		/*0x42*/  void*			child_smk;
		/*0x42*/  BinDlg*		child_dialog;
	};

	union {
		struct
		{
			/*0x46*/  Point16	text_pos;
			/*0x4A*/  u16		response_area_width;
			/*0x4C*/  u16		response_area_height;
			/*0x4E*/  unk_t     unk_4e[8];
		} initial_version_0x46;
		struct
		{
			/*0x46*/  Point16	text_pos;				// unverified filler
			/*0x4A*/  u32		unk4a;
			/*0x4E*/  unk_t     unk_4e[8];				// unverified filler
		} onInitDialog0x46;
		struct
		{
			/*0x46*/  u8		item_count;
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

struct AiRegion;
struct AiTown;

struct GuardAi {
	Link<GuardAi> link;				// 0x0, 0x4
	u8 type;						// 0x8
	u8 times_died;					// 0x9
	u8 unk_a[2];					// 0xA
	Unit* parent;					// 0xC
	u16 unit_id;					// 0x10
	Point16 current_home;			// 0x12
	Point16 original_home;			// 0x16
	u8 unk_1a[2];					// 0x1A
	u32 previous_update;			// 0x1C
};
C_ASSERT(sizeof(GuardAi) == 32); // 0x20

struct WorkerAi {
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

struct BuildingAi {
	Link<BuildingAi> link;			// 0x0, 0x4
	u8 type;						// 0x8
	u8 trainQueueTypes[5];			// 0x9
	u8 unkE[2];						// 0xE
	Unit* parent;					// 0x10
	AiTown* town;					// 0x14
	void* trainQueueValues[5];		// 0x18
};
C_ASSERT(sizeof(BuildingAi) == 44); // 0x2C

struct MilitaryAi {
	Link<MilitaryAi> link;			// 0x0, 0x4
	u8 type;						// 0x8
	u8 padding1;					// 0x9
	u16 padding2;					// 0xA
	Unit* unit;						// 0xC
	AiRegion* region;				// 0x10
};
C_ASSERT(sizeof(MilitaryAi) == 20); // 0x14

struct UnitAi {
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

struct WorkerAiArray {
	WorkerAi worker_ais[1000];
	WorkerAi* first_free_worker;
};

struct WorkerAiList {
	WorkerAiArray* array;
	WorkerAi* first;
};

struct BuildingAiArray {
	BuildingAi building_ais[1000];
	BuildingAi* first_free_building;
};

struct BuildingAiList {
	BuildingAiArray* array;
	BuildingAi* first;
};

struct MilitaryAiArray {
	MilitaryAi military_ais[1000];
	MilitaryAi* first_free_military;
};

struct MilitaryAiList {
	MilitaryAiArray* array;
	MilitaryAi* first;
};

struct GuardAiArray {
	GuardAi guard_ais[1000];
	GuardAi* first_free_guard;
};

struct GuardAiList {
	GuardAiArray* array;
	GuardAi* first;
};

struct GuardAiListPlayers {
	GuardAiList player[8];
};

struct AiRegion
{
	/*0x00*/  u16 region;
	/*0x02*/  u16 target_region;
	/*0x04*/  u8  player;
	/*0x05*/  u8  state;
	/*0x06*/  u8  defense_priority;
	/*0x07*/  u8  defend_time; // One unit is around ~60 frames (333 region updates per frame out of 2499 regions * 8 players)
							   // The countdown is 1 based, meaning the first tick is ignored
	/*0x08*/  u8  flags;
	/*0x09*/  u8  field_9;
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
	/*0x2C*/  MilitaryAiList military;
};

C_ASSERT(sizeof(AiRegion) == 52);

#define RegionAirOnly 0x1FFD

struct Region {
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

	bool remove_neighbour(u16 id) {
		for (u32 i = 0; i < neighbor_count; i++) {
			if (neighbors[i] == id) {
				neighbors[i] = neighbors[--neighbor_count];
				return true;
			}
		}
		return false;
	}
};
C_ASSERT(sizeof(Region) == 64);

struct RegionArray
{
	u16 region_count;
	u8 _dc2[0x449fa];
	Region regions[5000];
	u8 _dc92bfc[0x4e24];
};

struct TownReq {
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

struct AiTown {
	AiTown* prev;							//	0x00
	AiTown* next;							//	0x04
	WorkerAiList workers;					//	0x08
	BuildingAiList buildings;				//	0x10
	u8 player;								//	0x18
	u8 inited;								//	0x19
	u8 worker_count;						//	0x1A
	u8 worker_limit;						//	0x1B
	u8 resarea;								//	0x1C
	u8 resource_units_not_set;				//	0x1D
	u8 in_battle;							//	0x1E
	u8 unk1f;								//	0x1F
	Point16 position;						//	0x20
	Unit* main_building;					//	0x24
	Unit* building_scv;						//	0x28
	Unit* mineral;							//	0x2C
	Unit* old_gas_buildings[3];				//	0x30
	TownReq requests[0x64];					//	0x3C
};
C_ASSERT(sizeof(AiTown) == 0x1cc);

struct AiTownArray {
	AiTown towns[100];
	AiTown* first_free;
};

struct AiTownList {
	AiTownArray* array;
	AiTown* first;
};
C_ASSERT(sizeof(AiTownList) == 0x8);

struct SpendingRequest {
	u8		priority;
	u8		type;
	u16		id;
	union {
		AiRegion* ai_region; // RequestType::Military
		GuardAi* guard_ai;	 // RequestType::Guard
		AiTown* town;		 // RequestType::Building, RequestType::Worker, RequestType::Addon, RequestType::Upgrade, and RequestType::Tech
		void*	value;       // Empty otherwise
	};
};

struct AI_Main {
	u32 minerals_needed;				/*0000*/
	u32 vespene_needed;					/*0004*/
	u32 supply_needed;					/*0008*/
	u32 minerals_available;				/*000C*/
	u32 vespene_available;				/*0010*/
	u32 supply_available;				/*0014*/
	SpendingRequest requests[63];		/*0018*/
	u8 request_count;					/*0210*/
	u8 failed_placement;				/*0211*/
	u8 ai_nuke_rate;					/*0212*/
	u8 ai_attacks;						/*0213*/
	u32 ai_last_nuke_time;				/*0214*/
	struct {
		u16 is_secure_finished : 1;			/*0001*/
		u16 is_town_started : 1;			/*0002*/
		u16 is_default_build_off : 1;		/*0004*/
		u16 is_transports_off : 1;			/*0008*/
		u16 is_farms_notiming_on : 1;		/*0010*/
		u16 is_use_map_settings : 1;		/*0020*/
		u16 flag_0x40 : 1;					/*0040*/
		u16 spread_creep : 1;				/*0080*/
		u16 flag_0x100 : 1;					/*0100*/
		u16 has_strongest_ground_army : 1;	/*0200*/ // Set if the AI's inside army is stronger than the total strength of the strongest enemy force
		u16 upgrades_finished : 1;			/*0400*/
		u16 target_expansion : 1;			/*0800*/
	} ai_flags;
	u16 ai_panic_block;					/*021A*/
	u16 ai_max_force;					/*021C*/
	u16 ai_attack_group_region;			/*021E*/ // 1 based (0 means none)
	u16 wanted_unit;					/*0220*/
	u8	default_min;					/*0222*/
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
	u8	ai_define_max[UnitsIdLimit];	/*03F0*/
	Unit* main_medic;					/*04D4*/
	Box32 gen_cmd_loc;					/*04D8*/
};

struct AiScript {
	Link<AiScript>		link;			// 0x0, 0x4
	s32					script_offset;	// 0x8
	s32					sleep;			// 0xC
	s32					player;			// 0x10
	Box32				loc_bounds;		// 0x14
	Vec32			loc_center;		// 0x24
	AiTown*				town;			// 0x2C
	s32					flags;			// 0x30
};
C_ASSERT(sizeof(AiScript) == 52);

struct AiScriptArray {
	AiScript scripts[100];
	AiScript* first_free;
};

struct ResourceArea {
	Point16 position;
	u8 mineral_field_count;
	u8 geyser_count;
	bool is_start_location;
	u8 flags;					// 0x1 = Occupied / start location, 0x2 = allies_watch claimed(0069268F)
	u32 total_mineral_count;
	u32 total_vespene_count;
	u8 padding[32];
};
C_ASSERT(sizeof(ResourceArea) == 48);

struct ResourceAreaArray {
	ResourceArea areas[250];
	u32 count;
	u32 frames_till_update;
};

// From BWAPI Offsets.h, used in bwdata.h by g_unit_availability
struct _uavail {
	u8* available[PLAYER_COUNT];
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
		if (index == 0) {
			return nullptr; 
		} else if (index <= get_string_count()) {
			return (const char*)(buffer)+buffer[index];
		} else {
			return (const char*)0x00501B7D;	//g_string_empty in bwdata.h, SC default empty string
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
	char	player_name[25];
	char	force_name[30];
	// The rest is from DarkenedFantasies:
	u8		unknown;			// 0x37
	u32		human_id;			// 0x38
	u32		storm_id;			// 0x3C
	u32		type;				// 0x40
	u32		race;				// 0x44
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

struct TriggerCondition {
	u32 location;				// 0x00
	u32 group;					// 0x04
	u32 count;					// 0x08
	u16 unit_id;				// 0x0C
	u8	comparison_type;		// 0x0E
	u8	condition_type;			// 0x0F
	u8	extra_type;				// 0x10
	u8	flags;					// 0x11
	u16 internal_something;		// 0x12
};
C_ASSERT(sizeof(TriggerCondition) == 20);

struct TriggerAction {
	u32 source_location;		// 0x00
	u32 text_string;			// 0x04
	u32 wav_string;				// 0x08
	u32 time;					// 0x0C
	u32 source_group;			// 0x10
	u32 extra;					// 0x14
	u16 type;					// 0x18
	u8	action_type;			// 0x1A
	u8	modifier;				// 0x1B
	u8	flags;					// 0x1C
	u16 internal_something;		// 0x1D
	u8	padding;				// 0x1F
};
C_ASSERT(sizeof(TriggerAction) == 32);

struct Trigger {
	TriggerCondition	conditions[16];			// 0x000
	TriggerAction		actions[64];			// 0x140
	u32					flags;					// 0x940
	u8					execute_for[27];		// 0x944
	u8					current_action_index;	// 0x95F
};
C_ASSERT(sizeof(Trigger) == 0x960);

struct TriggerLink {
	TriggerLink*	prev;		// 0x0
	TriggerLink*	next;		// 0x4
	Trigger 		trigger;	// 0x8
};

struct PaletteEntry {
	u8			red;		// 0x0
	u8			green;		// 0x1
	u8			blue;		// 0x2
	u8			flags;		// 0x3
};

struct Bitmap {
	u16			width;		// 0x0
	u16			height;		// 0x2
	u8*			buffer;		// 0x4
};

struct IDirectSoundBuffer;
struct SoundData {
	u32 wav_data_size;
	s32 last_play_time;
	u32 duration_ms;
	IDirectSoundBuffer* ds_buffer;
};
C_ASSERT(sizeof(SoundData) == 16);

struct SoundChannel {
	IDirectSoundBuffer* ds_buffer;	// 0x00
	u16 unit_id;					// 0x04
	u8 flags;						// 0x06
	u8 sound_priority;				// 0x07
	u8 release_when_not_used;		// 0x08
	u32 sound_id;					// 0x09
	u32 volume;						// 0x0D
	u32 pan;						// 0x11
	u8 unused[3];					// 0x15
};
C_ASSERT(sizeof(SoundChannel) == 24);

struct UpdateFunction {
	u32 ui_number;
	void* function;
};

struct DrawFunction {
	u32 ui_number;
	void* function_1;
	void* function_2;
};

struct IscriptProgram {
	u16 iscript_header;					// 0x0
	u16 program_counter;				// 0x2
	u16 unsigned4;						// 0x4
	IscriptAnimation::Enum animation;	// 0x6
	u8 wait;							// 0x7
};

namespace helper {
	extern bool unit_is_standard_and_movable(Unit* unit);
}

union UnitIndex {
	struct {
		u32 index : 12;			// btw. Updated with CCMU extender
		u32 unique_id : 20;
	};
	u32 value;

	UnitIndex() noexcept : value(0) {}

	UnitIndex(Unit* unit) : value(0) {
		if (!unit) return;
		index = unit->get_unit_index();
		unique_id = unit->unique_id;
	}

	Unit* get(bool allow_non_standard_units = false);
	Unit* get_for_selection(bool allow_non_standard_units = false);
	void set(Unit* unit) { *this = unit; }
	bool is_set() { return get(true) != nullptr; }
	bool is_set_for_selection() { return get_for_selection(true) != nullptr; }
	void clear() { value = 0; }
};

C_ASSERT(sizeof(UnitIndex) == 0x4);

struct ForceNamesEntry {
	char name[30];
};

struct ForceNames {
	ForceNamesEntry force[4];
};


struct HashPacket {
	u8 id;
	u8 sequence;
	u16 sequence_hash;
	u8 visibility_hash;
	u8 visibility_id;
	u8 minimap_box_count;
};

struct SightTile {
	/*00*/	SightTile*	tiles[2];
	/*08*/	s32			relative_tile;
	/*0C*/	u32			visibility_mask;
	/*10*/	s8			w;
	/*11*/	s8			h;
	/*12*/	u8			count;
	/*13*/	u8			unk_13;
};

struct SightRange {
	/*00*/	u32			tile_sight_width;
	/*04*/	u32			tile_sight_height;
	/*08*/	u32			revealed_width;
	/*0C*/	u32			revealed_height;
	/*10*/	u32			revealed_count;
	/*14*/	u32			masked_count;
	/*18*/	SightTile*	tile;
};

typedef void __fastcall sight_callback_t(u32 tile_x, u32 tile_y, SightRange* sight_range, ActiveTile* cur_active_tile, u32 relative_flags, u32 new_flags);
struct SightCallbacks {
	sight_callback_t* inside[3];
	sight_callback_t* edge[3];
};

struct CmdBtnPalette {
	char colors[16];
};

struct GameVersion {
	u32 size;
	const char* game_title;
	const char* game_description;
	u32 game_id;
	u32 version_id;
	u32 reserved;
	u32 max_players;
	u32 init_data;
	u32 init_data_bytes;
	u32 reserved2;
	u32 category_bits;
	const char* key_owner;
	u32 key;
	u32 is_demo;
	u32 lang;
};
C_ASSERT(sizeof(GameVersion) == 60);

// Selection extender from DarkenedFantasies!
// As it currently stands, SelectionLimit must be a multiple of 6, and 252 is the largest supported value.
// Look into editing the cmdact/cmdrecv_select buffers to upgrade the selection byte to at least a word.
#define SelectionLimit 246
#define SelectionLimit_word 2*SelectionLimit
#define SelectionLimit_dword 4*SelectionLimit
#define SelectionLimit_dword_2 2*SelectionLimit_dword
#define newSrc 8-SelectionLimit_dword
#define newDestUnit 12-SelectionLimit_dword
#define newStackChar 12-SelectionLimit_dword_2
#define buffer3_stack 8-SelectionLimit_word
#define buffer3_size 9-SelectionLimit_word
#define buffer3_type 10-SelectionLimit_word
#define buffer2_stack 10-SelectionLimit_dword
#define buffer2_size 11-SelectionLimit_dword
#define buffer2_type 12-SelectionLimit_dword
#define buffer1_stack 12-SelectionLimit_word-SelectionLimit_dword
#define buffer1_size 13-SelectionLimit_word-SelectionLimit_dword
#define buffer1_type 14-SelectionLimit_word-SelectionLimit_dword
#define GroupWireframesCap 12 //Change this number when the UI can support more group selection wireframes.

const s32 SelectionCap = SelectionLimit; // Old const

struct ControlGroup {
	UnitIndex unit_index[SelectionCap];

	UnitIndex* find(UnitIndex index);
	bool has(UnitIndex index) { return find(index) != 0; }
	bool add(UnitIndex index);
	void clear();
	void remove(UnitIndex index);
	void select(bool play_response);
	void add_selection();
	void center_camera();
	
	UnitIndex* begin() { return unit_index; }
	UnitIndex* end() { return unit_index + SelectionCap; }
};

struct WireList {
	u16 count;
	u16 unit_id = UnitId::None;
	Unit* unit;
};

#define TotalControlGroups 18	// Control groups + Recent selections
#define UiControlGroups 10		// Control groups

struct ControlGroupPlayer {
	ControlGroup control_group[TotalControlGroups];
};

struct ControlGroupPlayerArray {
	ControlGroupPlayer player[PlayerLimit];
};
C_ASSERT(sizeof(ControlGroupPlayerArray) == 4 * PLAYABLE_PLAYER_COUNT * 18 * SelectionCap);

struct Point32 {
	u32 x;
	u32 y;
};

struct TILE {
	/*00*/	u16		terrain_type;
	/*02*/	u8		buildability;// < 8th bit should sign not buildable.
	/*03*/	u8		ground_height;// < Ground Height(4 lower bits) - Deprecated? Some values are incorrect.
	/*04*/	Box16	edge_types;
	/*0C*/	Box16	edge_pieces;/* left and right are unused; always 0.
						top and bottom states:
							1 = Basic edge piece
							2 = Right edge piece
							3 = Left edge piece.*/
	/*14*/	u16		tiles[16];// MegaTile References (VF4/VX4)
};

struct DOODAD {
	/*00*/	u16		index;
	/*02*/	u8		buildability;// < 8th bit should sign not buildable.
	/*03*/	u8		ground_height_and_overlay_flags;/* < Ground Height(4 lower bits) - Deprecated? Some values are incorrect.
						Overlay Flags:
							0x0 - None
							0x1 - Sprites.dat Reference
							0x2 - Units.dat Reference (unit sprite)
							0x4 - Overlay is Flipped*/
	/*04*/	u16		overlayID;
	/*06*/	u16		_1;
	/*08*/	u16		doodad_group_string;
	/*0A*/	u16		_2;
	/*0C*/	u16		data_bin_index;
	/*0E*/	u16		doodad_height;
	/*10*/	u16		doodad_width;
	/*12*/	u16		_3;
	/*14*/	u16		tiles[16];// MegaTile References (VF4/VX4)
};	//size 34

struct cv5 {
	/*00*/	u16		terrain_type;
	/*02*/	u16		flags;
	/*04*/	Box16	edge_types;
	/*0C*/	Box16	edge_pieces;//left and right are unused; always 0.
	/*14*/	u16		tiles[16];
};

struct CV5 {
	TILE group[1];
};

struct vf4 {
	u8 flags;
	s8 xHeight;
};

struct VF4 {
	vf4 tile[1][16];
};

struct vx4 {//u32 with vx4ex, u16 without
	u32 flipped	: 1;
	u32 vr4_id	: 31;//31 with vx4ex, 15 without
};

struct VX4 {
	vx4 tile[1][16];
};

struct VR4 {
	u8 minitile[1][64];
};

struct MTXM {
	u16 tile[1];
};

struct MusicStruct {
	u32 file;
	u8	type;
	u8	flags;
	u8	next_index;
	u8	field_7;
};

struct PingArray {
	Ping ping_array[16];
};

#pragma pack()