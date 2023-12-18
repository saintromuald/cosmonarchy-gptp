#pragma once
#pragma warning( disable: 4482 )

#include "enumerations/UnitId.h"
#include "enumerations/WeaponId.h"
#include "enumerations/UpgradeId.h"
#include "enumerations/TechId.h"
#include "enumerations/OrderId.h"
#include "enumerations/ImageId.h"
#include "enumerations/SpriteId.h"
#include "enumerations/FlingyId.h"
#include "enumerations/SoundId.h"
#include "enumerations/IconId.h"
#include "enumerations/PortraitId.h"
#include <algorithm>

namespace UnitSize
{
	enum
	{
		Independent,
		Light,
		Medium,
		Heavy,
		Titanic,
	};
}

namespace FlingyDatEntries
{
	enum
	{
		TopSpeed,
		Acceleration,
		TurnRadius,
		MovementType,
		SpriteId,
		HaltDistance,
		IscriptMask,
	};
}

namespace UnitsDatEntries
{
	enum
	{
		Flingy,
		Subunit,
		Infested,
		Construction,
		StartDirection,
		ShieldEnabled,
		MaxShields,

		SndReady = 29,
		SndWhtFirst = 30,
		SndWhtLast = 31,
		SndPssFirst = 32,
		SndPssLast = 33,
		SndYesFirst = 34,
		SndYesLast = 35,

		/*
		4, //max hits
		1, //elevation
		1, //movement flags
		1, //rank
		1, //computer idle order
		1, //human idle order
		1, //return to idle order
		1, //attack unit order
		1, //attack move order
		1, //inside weapon
		1, //max inside hits
		1, //edge weapon
		1, //max edge hits
		1, //computer ai unit type
		4, //flags
		1, //seek distance
		1, //sight distance
		1, //armor upgrade
		1, //count
		1, //armor
		1, //right-click
		2, //READY
		2, //what-first
		2, //what-last
		2, //pss-first
		2, //pss-last
		2, //yes-first
		2, //yes-last
		4, //building dimensions (2+2
		4, //addon placement
		8, //unit bounds
		2, //portrait
		2, //mineral cost
		2, //gas cost
		2, //time cost
		2, //restriction flags
		1, //group flags
		1, //supply provided
		1, //supply required
		1, //space required
		1, //space provided
		2, //build score
		2, //destroy core
		2, //map string id
		1, //broodwar only
		2, //staredit avail*/
	};
}

namespace SfxDatEntries
{
	enum
	{
		Volume,
		Flags,
		Race,
		Type,
		SoundFile,
		SoundDataPtr, // unlike sfxdata, uses pointers instead of ids
	};
}

namespace SpritesDatEntries
{
	enum
	{
		IncludeInVision,
		StartAsVisible,
		ImageId,
		SelectionCircle,
		SelectImagepos,
		Healthbar,
	};
}

namespace ImagesDatEntries
{
	enum
	{
		GrpId,
		Turnable,
		Clickable,
		UseFullIscript,
		DrawIfCloaked,
		DrawFunc,
		DrawRemap,
		IscriptId,
		OverlayShield,
		OverlayAttack,
		OverlayDamage,
		OverlaySpecial,
		OverlayLanding,
		OverlayLiftoff,
		OverlayShieldPtr,
		OverlayAttackPtr,
		OverlayDamagePtr,
		OverlaySpecialPtr,
		OverlayLandingPtr,
		OverlayLiftoffPtr,
		ImagePtr,
		BuildingOverlayStateMax,
	};
}

namespace AiType {
	enum Enum {
		Unassigned	= 0, // Never this?
		Guard		= 1,
		Worker		= 2,
		Building	= 3, // Also "available to scripts"
		Military	= 4,
	};
}

namespace RequestType {
	enum Enum {
		None		= 0,
		Military	= 1,
		Guard		= 2,
		Building	= 3,
		Worker		= 4,
		Upgrade		= 5,
		Tech		= 6,
		Addon		= 7,
		Witness		= 8,
	};
}

namespace RegionState {
	enum Enum {
		Nothing							= 0,
		ContinueAttackHere				= 1,
		Attacking						= 2,
		EnemySighted					= 3,
		TownRegionLow					= 4,
		TownRegionHigh					= 5,
		Defending						= 6,
		ChaseAway						= 7,
		PreparingAttack					= 8,
		PreparingAttackOldUnreachable	= 9, // Never occurs in vanilla?
	};
}

namespace RaceId
{
	enum Enum : u8
	{
		Zerg = 0,
		Terran = 1,
		Protoss = 2,
		Other = 3, // also called Independent
		Neutral = 4,
		Select = 5,
		Random = 6,
		None = 7, // also called Inactive

		Count = 3,
	};
}

namespace PlayerType
{
	enum Enum
	{
		Inactive			= 0,
		Computer			= 1,
		Human				= 2,
		Rescuable			= 3,
		UnusedRescueActive	= 4,
		ComputerSlot		= 5,
		OpenSlot			= 6,
		Neutral				= 7,
		ClosedSlot			= 8,
		UnusedObserver		= 9,
		HumanDefeated		= 10, // Or left the game
		ComputerDefeated	= 11, // Or left the game
	};
}

// Based on BWAPI; for use with units_dat::base_property[]
namespace UnitProperty
{
	enum Enum : u32
	{
		Building = 0x00000001,
		Addon = 0x00000002,
		Flyer = 0x00000004,
		Worker = 0x00000008,
		Subunit = 0x00000010,
		FlyingBuilding = 0x00000020,
		Hero = 0x00000040,
		RegeneratesHP = 0x00000080,
		AnimatedIdle = 0x00000100,
		Cloakable = 0x00000200,
		TwoUnitsIn1Egg = 0x00000400,
		NeutralAccessories = 0x00000800,	// AKA "Single entity" (prevents multi-select, set on all pickup items)
		ResourceDepot = 0x00001000,			// Drop-off point
		ResourceContainer = 0x00002000,		// Node
		RoboticUnit = 0x00004000,
		Detector = 0x00008000,
		Organic = 0x00010000,
		CreepBuilding = 0x00020000,
		Unused = 0x00040000,
		RequiredPsi = 0x00080000,
		Burrowable = 0x00100000,
		Spellcaster = 0x00200000,
		PermanentCloak = 0x00400000,
		NPCOrAccessories = 0x00800000,		// AKA "Pickup item" (data disc, crystals, mineral chunks, gas tanks, etc.)
		MorphFromOtherUnit = 0x01000000,
		MediumOverlay = 0x02000000,
		LargeOverlay = 0x04000000,
		AutoAttackAndMove = 0x08000000,
		Attack = 0x10000000,				// Full Auto-Attack
		Invincible = 0x20000000,
		Mechanical = 0x40000000,
		ProducesUnits = 0x80000000,			// It can produce units directly (making buildings doesn't count)
	};
}

// Based on BWAPI; For use with the Unit::status member
namespace UnitStatus
{
	enum Enum : u32
	{
		Completed				= 0x00000001,
		GroundedBuilding		= 0x00000002,
		InAir					= 0x00000004,
		Disabled				= 0x00000008,
		Burrowed				= 0x00000010,
		InBuilding				= 0x00000020,
		InTransport				= 0x00000040,
		CanBeChased				= 0x00000080,
		RequiresDetection		= 0x00000100,
		Cloaked					= 0x00000200,
		Frozen					= 0x00000400,	// Protoss unpowered buildings have this flag set
		CloakingForFree			= 0x00000800,	// Requires no energy to cloak
		CanNotReceiveOrders		= 0x00001000,
		NoBrkCodeStart			= 0x00002000,	// Unbreakable code section in iscript
		UNKNOWN2				= 0x00004000,
		CanNotAttack			= 0x00008000,
		CanTurnAroundToAttack	= 0x00010000,	// IsAUnit in IDA
		Reacts					= 0x00020000,
		IgnoreTileCollision		= 0x00040000,
		Unmovable				= 0x00080000,
		IsNormal				= 0x00100000,	// 1 for "normal" units, 0 for hallucinated units
		NoCollide				= 0x00200000,	// If set, other units wont collide with the unit (like burrowed units)
		UNKNOWN5				= 0x00400000,
		IsGathering				= 0x00800000,	// If set, the unit wont collide with other units (like worker gathering)
		SubunitWalking			= 0x01000000,
		SubunitFollow			= 0x02000000,	// Turret related
		Invincible				= 0x04000000,
		AttackMove				= 0x08000000,
		SpeedUpgrade			= 0x10000000,
		CooldownUpgrade			= 0x20000000,
		IsHallucination			= 0x40000000,	// 1 for hallucinated units, 0 for "normal" units
		IsSelfDestructing		= 0x80000000	// Set for when the unit is self-destructing (scarab, scourge, infested terran)
	};
}

// For various newFlags fields
namespace NewFlags1
{
	enum Enum : u8
	{
		Unused1 = 0x1,
		UnusedRadiationShockwave = 0x2,
		UnusedMetastasis = 0x4,
	};
}

namespace Button_State
{
	enum Enum
	{
		Invisible = 0,
		Disabled = -1,
		Enabled = 1
	};
}

// For use with scbw::print()
namespace GameTextColor
{
	enum Enum
	{
		Grey = 0x01,
		Grey2 = 0x02,
		Yellow = 0x03,
		White = 0x04,
		DarkGrey = 0x05,
		Red = 0x06,
		Green = 0x07,
		Invisible = 0x08,
		Tab = 0x09,
		Newline = 0x0A,
		Unknown = 0x0B,
		Newline2 = 0x0C,
		Invisible2 = 0x0D,
		AlignRight = 0x12,
		AlignCenter = 0x13,
		TextGone = 0x14,
		RiverBlue = 0x0E,
		Teal = 0x0F,
		Purple = 0x10,
		Orange = 0x11,
		Brown = 0x15,
		LightGrey = 0x16,
		Yellow2 = 0x17,
		DarkGreen = 0x18,
		Yellow3 = 0x19,
		Yellow4 = 0x1A,
		Navy = 0x1C,
		ArmyGreen = 0x1D,
		Grey3 = 0x1E,
		Teal2 = 0x1F,
		AlignCenter2 = 0x13,
		TextGone2 = 0x14,
	};
}

namespace LobbyTextColor
{
	enum Enum
	{
		Purple1 = 0x01,
		Purple2 = 0x02,
		YellowGreen = 0x04,
		Grey = 0x05,
		White = 0x06,
		Red = 0x07,
		Tab = 0x09,
		Black = 0x10, // Could be wrong!
		AlignRight = 0x18,
		AlignCenter = 0x19,
	};
};

// For use with scbw::play_iscript_animation()
namespace IscriptAnimation
{
	enum Enum : u8
	{
		Init = 0x00,
		Death = 0x01,
		GndAttkInit = 0x02,
		AirAttkInit = 0x03,
		Unused1 = 0x04,
		GndAttkRpt = 0x05,
		AirAttkRpt = 0x06,
		CastSpell = 0x07,
		GndAttkToIdle = 0x08,
		AirAttkToIdle = 0x09,
		Unused2 = 0x0A,
		Walking = 0x0B,
		WalkingToIdle = 0x0C,
		SpecialState1 = 0x0D,
		SpecialState2 = 0x0E,
		AlmostBuilt = 0x0F,
		Built = 0x10,
		Landing = 0x11,
		LiftOff = 0x12,
		IsWorking = 0x13,
		WorkingToIdle = 0x14,
		WarpIn = 0x15,
		Unused3 = 0x16,
		StarEditInit = 0x17,
		Disable = 0x18,
		Burrow = 0x19,
		UnBurrow = 0x1A,
		Enable = 0x1B
	};
}

static const char* animation_type_names[] = {
	"Init",
	"Death",
	"GroundAttackInit",
	"AirAttackInit",
	"Unused1",
	"GroundAttackRepeat",
	"AirAttackRepeat",
	"CastSpell",
	"GroundAttackToIdle",
	"AirAttackToIdle",
	"Unused2",
	"Walking",
	"WalkingToIdle",
	"SpecialState1",
	"SpecialState2",
	"AlmostBuilt",
	"Built",
	"Landing",
	"LiftOff",
	"IsWorking",
	"WorkingToIdle",
	"WarpIn",
	"Unused3",
	"StarEditInit",
	"Disable",
	"Burrow",
	"Unburrow",
	"Enable"
};

namespace CheatFlags
{
	enum Enum
	{
		None = 0,
		BlackSheepWall = 1 << 0,			/*0x00000001*/
		OperationCwal = 1 << 1,				/*0x00000002*/
		PowerOverwhelming = 1 << 2,			/*0x00000004*/
		SomethingForNothing = 1 << 3,		/*0x00000008*/
		ShowMeTheMoney = 1 << 4,			/*0x00000010*/
		GameOverMan = 1 << 6,				/*0x00000040*/
		ThereIsNoCowLevel = 1 << 7,			/*0x00000080*/
		StayingAlive = 1 << 8,				/*0x00000100*/
		Ophelia = 1 << 9,					/*0x00000200*/
		TheGathering = 1 << 11,				/*0x00000800*/
		MedievalMan = 1 << 12,				/*0x00001000*/
		ModifyThePhaseVariance = 1 << 13,	/*0x00002000*/
		WarAintWhatItUsedToBe = 1 << 14,	/*0x00004000*/
		FoodForThought = 1 << 17,			/*0x00020000*/
		WhatsMineIsMine = 1 << 18,			/*0x00040000*/
		BreatheDeep = 1 << 19,				/*0x00080000*/
		NoGlues = 1 << 29					/*0x20000000*/
	};
}

namespace DamageType
{
	enum Enum
	{
		Independent,
		Explosive,
		Concussive,
		Impact,
		IgnoreArmor,
		Chronal,
	};
}

// Use with weapons_dat::explosion_type[]
namespace WeaponEffect
{
	enum Enum
	{
		None = 0,
		NormalHit = 1,
		SplashRadial = 2,
		SplashEnemy = 3,
		Lockdown = 4,
		NuclearMissile = 5,
		Parasite = 6,
		Broodlings = 7,
		EMPShockwave = 8,
		Irradiate = 9,
		Ensnare = 10,
		Plague = 11,
		StasisField = 12,
		CarapaceSwarm = 13,
		Consume = 14,
		YamatoGun = 15,
		Restoration = 16,
		DisruptionWeb = 17,
		CorrosiveAcid = 18,
		Tyranny = 19,
		Feedback = 20,
		OpticalFlare = 21,
		Maelstrom = 22,
		Unknown_Crash = 23,
		SplashAir = 24
	};
}

// Use with weapons_dat::behavior[]
enum Enum
{
	WeaponBehavior_FlyDoNotFollowTarget		= 0,
	WeaponBehavior_FlyFollowTarget			= 1,
	WeaponBehavior_AppearOnTargetUnit		= 2,
	WeaponBehavior_PersistOnTargetSite		= 3,
	WeaponBehavior_AppearOnTargetSite		= 4,
	WeaponBehavior_AppearOnAttacker			= 5,
	WeaponBehavior_AttackAndSelfDestruct	= 6,
	WeaponBehavior_Bounce					= 7,
	WeaponBehavior_AttackNearbyArea			= 8,
	WeaponBehavior_GoToMaxRange				= 9
};

// Use with g_color_shift
namespace ColorRemapping
{
	enum Enum
	{
		None			= 0,
		OFire			= 1,
		GFire			= 2,
		BFire			= 3,
		BExpl			= 4,
		Trans50			= 5,  // Special - OwnCloak
		Red_Crash		= 6,
		Green_Crash		= 7,
	};
}

// Compare with (*g_game_type)
namespace GameType
{
	enum Enum
	{
		Melee = 2,
		FreeForAll = 3,
		OneOnOne = 4,
		CaptureTheFlag = 5,
		Greed = 6,
		Slaughter = 7,
		SuddenDeath = 8,
		Ladder = 9,
		UseMapSettings = 10,
		TeamMelee = 11,
		TeamFreeForAll = 12,
		TeamCTF = 13,
		TopVsBottom = 15,

		Competitive = 30,
		Scenario = 31,
		Omnipresence = 32,
		FactionWars = 33,
		Massacre = 34,
		Debug = 35,
	};
}

// Compare with (*g_current_tileset)
namespace TilesetType
{
	enum Enum
	{
		Badlands = 0,
		Space_Platform = 1,
		Installation = 2,
		Ash_World = 3,
		Jungle_World = 4,
		Desert = 5,
		Ice = 6,
		Twilight = 7,

		Count,
	};
}

enum MovableState
{
	MovableState_NotReachedDestination = 0,
	MovableState_ReachedDestination = 1,
	MovableState_UnmovableAtDestination = 2
};

namespace BinDlgFlags
{
	enum Enum
	{
		Highlighted					= 0x00000001,
		Disabled					= 0x00000002,
		Active						= 0x00000004,
		Visible						= 0x00000008,
		RespondsToEvents			= 0x00000010,
		Unknown5					= 0x00000020,
		CancelButton				= 0x00000040,
		NoHoverSound				= 0x00000080,
		SpecialHotkeyVirtualKey		= 0x00000100,
		HasHotkey					= 0x00000200,
		SmallestFontSize10			= 0x00000400,
		LargerFontSize16			= 0x00000800,
		Unknown12					= 0x00001000,
		TransparentHideColor		= 0x00002000,
		LargestFontSize16x			= 0x00004000,
		AlternateStyleWC2			= 0x00008000,
		SmallerFontSize14			= 0x00010000,
		RemoveStyles				= 0x00020000,
		ApplyTranslucency			= 0x00040000,
		DefaultButton				= 0x00080000,
		BringToFront				= 0x00100000,
		HorizontalAlignmentLeft		= 0x00200000,
		HorizontalAlignmentRight	= 0x00400000,
		HorizontalAlignmentCenter	= 0x00800000,
		VerticalAlignmentTop		= 0x01000000,
		VerticalAlignmentMiddle		= 0x02000000,
		VerticalAlignmentBottom		= 0x04000000,
		Unknown27					= 0x08000000,
		ReverseDialogDirection		= 0x10000000,
		UseAlternateStyle			= 0x20000000,
		NoClickSound				= 0x40000000,
		HighlightText				= 0x80000000
	};
}

namespace SmkFlags
{
	enum Enum
	{
		FadeIn = 0x0001,
		Dark = 0x0002,
		RepeatForever = 0x0004,
		ShowIfOver = 0x0008,
		Unknown4 = 0x0010,
		Flag_0020 = 0x0020,
		Flag_0040 = 0x0040,
		Flag_0080 = 0x0080,
		Flag_0100 = 0x0100,
		Flag_0200 = 0x0200,
		Flag_0400 = 0x0400,
		Flag_0800 = 0x0800,
		Flag_1000 = 0x1000,
		Flag_2000 = 0x2000,
		Flag_4000 = 0x4000,
		Flag_8000 = 0x8000
	};
}

namespace DialogControlTypes
{
	enum Enum
	{
		DialogBoxDC = 0x00000000, // DialogBox cannot be used, since it's used in winapi
		DefaultButton = 0x00000001,
		Button = 0x00000002,
		OptionButton = 0x00000003,
		CheckBox = 0x00000004,
		Image = 0x00000005,
		HorizontalScrollBar = 0x00000006,
		VerticalScrollBar = 0x00000007,
		TextBox = 0x00000008,
		LabelLeftAlign = 0x00000009,
		LabelCenterAlign = 0x0000000A,
		LabelRightAlign = 0x0000000B,
		ListBox = 0x0000000C,
		ComboBox = 0x0000000D,
		LightUpButton = 0x0000000E
	};
}

// Use with units_dat::RightClickActions
namespace RightClickActions
{
	enum Enum : u8
	{
		NoCommand_AutoAttack = 0,
		NormalMove_NormalAttack = 1,
		NormalMove_NoAttack = 2,
		NoMove_NormalAttack = 3,
		Harvest = 4,
		HarvestAndRepair = 5,
		Unknown = 6				// only by ComSat addon
	};
}

// List of opcodes used to set whether a given unit can use build or train a unit, technology, research technology, upgrade or perform an order
namespace RequirementOpcodes
{
	enum Enum
	{
		None = 0x0FF00,
		Or = 0x0FF01,
		CurrentUnitIs = 0x0FF02,
		MustHave = 0x0FF03,
		MustHaveAddon = 0x0FF04,
		IsNotLiftedOff = 0x0FF05,
		IsLiftedOff = 0x0FF06,
		IsNotTrainingOrMorphing = 0x0FF07,
		IsNotConstructingAddon = 0x0FF08,
		IsNotResearching = 0x0FF09,
		IsNotUpgrading = 0x0FF0A,
		IsNotConstructing = 0x0FF0B,
		DoesNotHaveAddonAttached = 0x0FF0C,
		DoesNotHaveExit = 0x0FF0D,
		HasHangarSpace = 0x0FF0E,
		MustBeResearched = 0x0FF0F,
		DoesNotHaveArmedMissile = 0x0FF10,
		IsNotBurrowed = 0x0FF11,
		CanAttack = 0x0FF12,
		CanSetRallyPoint = 0x0FF13,
		CanMove = 0x0FF14,
		HasWeapon = 0x0FF15,
		IsWorker = 0x0FF16,
		IsFlyingBuilding = 0x0FF17,
		IsTransport = 0x0FF18,
		IsPowerup = 0x0FF19,
		IsSubunit = 0x0FF1A,
		HasMines = 0x0FF1B,
		IsHeroAndEnabled = 0x0FF1C,
		CanHoldPosition = 0x0FF1D,
		AllowOnHallucinations = 0x0FF1E,
		UpgradeLv1Requires = 0x0FF1F,
		UpgradeLv2Requires = 0x0FF20,
		UpgradeLv3Requires = 0x0FF21,	// also used for anything above level 3
		Grey = 0x0FF22,
		Blank = 0x0FF23,
		MustBeBroodWar = 0x0FF24,
		IsResearched = 0x0FF25,
		IsBurrowed = 0x0FF26,
		EndOfSublist = 0x0FFFF,
	};
}

// Contains Internal error codes returned from functions (hooks\requirements\)
// Apparently used to signal to the AI when requirements fail
namespace InternalErrors
{
	enum Enum
	{
		UnknownError0 = 0,
		UnknownError1 = 1,
		UnknownError2 = 2,
		UnknownError3 = 3,
		UnknownError4 = 4,
		UnknownError5 = 5,
		UnknownError6 = 6,
		UnknownError7 = 7,
		UnitBusyOrIncompatible = 8,
		UnknownError9 = 9,
		UnitFrozen = 10,
		UnitBurrowed = 11,
		UnknownError12 = 12,
		UnknownError13 = 13,
		UnknownError14 = 14,
		UnknownError15 = 15,
		UnknownError16 = 16,
		UnknownError17 = 17,
		UnknownError18 = 18,
		UnknownError19 = 19,
		UnknownError20 = 20,
		UnknownError21 = 21,
		UnknownError22 = 22,
		MissingRequirement = 23,
		UnknownError25 = 25,
		UnknownError26 = 26,
	};
}

enum DatSoundFlags {
	DatSoundFlags_PermanentlyLoaded    = 0x01,
	DatSoundFlags_UnitResponse         = 0x02,
	DatSoundFlags_SkipFreeing          = 0x04,
	DatSoundFlags_BufferCreationQueued = 0x08,
	DatSoundFlags_OneAtATime           = 0x10,
	DatSoundFlags_NeverPreempt         = 0x20,
};

namespace ResourceType
{
	enum Enum
	{
		None = 0,
		VespeneGas = 1,
		Minerals = 2,
		Either = 3,
	};
}

namespace PlayerColor
{
	enum Enum
	{
		Invalid = -1,

		Red,
		Blue,
		Teal,
		Purple,
		Orange,
		Brown,
		White,
		Yellow,
		Green,
		Lemon,
		Peach,
		RoyalBlue,
		Asparagus,
		DeepPeriwinkle,
		Periwinkle,
		Cyan,
		Black,
		RussianGreen,
		AliceBlue,
		Harlequin,
		Saffron,
		MayaBlue,
		DarkOlive,
		LightOlive,
		Bronze,
		TyrianPurple,
		Ultramarine,
		Celadon,
		Maroon,
		FernGreen,
		Pumpkin,
		Vermillion,
		Magma,
		Umber,
		ElectricBlue,
		EggshellBlue,
		PalatinatePurple,
		JetBlack,
		Goldenrod,
		Blood,
		SilverPink,
		DimGrey,
		Grey,
		Coral,
		Salmon,
		VividRed,
		Glacial,
		Silver,
		Platinum,
		Aero,
		VineGreen,
		CosmicCobalt,
		DarkGreen,
		Turquoise,
		Rainbow,
		ArylideYellow,
		Mint,
		SmokyBlack,
		Snow,
		SaddleBrown,
		Zaffre,
		YinmnBlue,
		ElectricOrange,
		Brass,
		Scarlet,
		Emerald,
		Sienna,
		CornflowerBlue,
		Mango,
		Chestnut,
		NeonBlue,
		DarkSapphire,
		Mahogany,
		Wine,
		Auburn,
		Cordovan,
		Sunglow,
		Magenta,
		VividOrange,
		DarkBronze,
		BurntUmber,
		IridescentYellow,
		IridescentGreen,
		IridescentBlue,
		IridescentViolet,
		RainbowNew,
		Redwood,
		Beaver,
		Liver,
		DelftBlue,
		DarkCoral,
		PersianOrange,
		Tomato,
		BistreBrown,
		Beige,
		TeaGreen,
		Earthtone,
		JapaneseViolet,
		DarkSpringGreen,
		GoldenBrown,
		BdazzledBlue,
		CoyoteBrown,
		MidnightGreen,
		PatrioticFreedom,
		PalatinateBlue,
		DarkLavender,
		ForestGreen,
		ZinnwalditeBrown,
		NeonCarrot,
		Aquamarine,
		MetzenicProzac,

		Count,
	};
}

static const char* player_color_names[] =
{
	"Red",
	"Blue",
	"Teal",
	"Purple",
	"Orange",
	"Brown",
	"White",
	"Yellow",
	"Green",
	"Lemon",
	"Peach",
	"Royal Blue",
	"Asparagus",
	"Deep Periwinkle",
	"Periwinkle",
	"Cyan",
	"Black",
	"Russian Green",
	"Alice Blue",
	"Harlequin",
	"Saffron",
	"Maya Blue",
	"Dark Olive",
	"Light Olive",
	"Bronze",
	"Tyrian Purple",
	"Ultramarine",
	"Celadon",
	"Maroon",
	"Fern Green",
	"Pumpkin",
	"Vermillion",
	"Magma",
	"Umber",
	"Electric Blue",
	"Eggshell Blue",
	"Palatinate Purple",
	"Jet Black",
	"Goldenrod",
	"Blood",
	"Silver Pink",
	"Dim Grey",
	"Grey",
	"Coral",
	"Salmon",
	"Vivid Red",
	"Glacial",
	"Silver",
	"Platinum",
	"Aero",
	"Vine Green",
	"Cosmic Cobalt",
	"Dark Green",
	"Turquoise",
	"Rainbow",
	"Arylide Yellow",
	"Mint",
	"Smoky Black",
	"Snow",
	"Saddle Brown",
	"Zaffre",
	"Yinmn Blue",
	"Electric Orange",
	"Brass",
	"Scarlet",
	"Emerald",
	"Sienna",
	"Cornflower Blue",
	"Mango",
	"Chestnut",
	"Neon Blue",
	"Dark Sapphire",
	"Mahogany",
	"Wine",
	"Auburn",
	"Cordovan",
	"Sunglow",
	"Magenta",
	"Vivid Orange",
	"Dark Bronze",
	"Burnt Umber",
	"Iridescent Yellow",
	"Iridescent Green",
	"Iridescent Blue",
	"Iridescent Violet",
	"Rainbow New",
	"Redwood",
	"Beaver",
	"Liver",
	"Delft Blue",
	"Dark Coral",
	"Persian Orange",
	"Tomato",
	"Bistre Brown",
	"Beige",
	"Tea Green",
	"Earthtone",
	"Japanese Violet",
	"Dark Spring Green",
	"Golden Brown",
	"B'dazzled Blue",
	"Coyote Brown",
	"Midnight Green",
	"Patriotic Freedom",
	"Palatinate Blue",
	"Dark Lavender",
	"Forest Green",
	"Zinnwaldite Brown",
	"Neon Carrot",
	"Aquamarine",
	"Metzenic Prozac",
};

enum VespeneDeaths
{
	VespeneDeaths_Reveal				= 0x01,
	VespeneDeaths_Ping					= 0x02,
	VespeneDeaths_ToggleRandomizeColor	= 0x04,
	VespeneDeaths_DisableQuasifog		= 0x08,
};

enum AiPersonality // disruption_web_unit
{
	AiPersonality_None = 0,
	AiPersonality_TerranArmor = 1,
	AiPersonality_TerranRaider,
	AiPersonality_TerranOrbital,
	AiPersonality_TerranGeneral,
	AiPersonality_TerranCount,

	AiPersonality_ProtossRanger = 1,
	AiPersonality_ProtossTemplar,
	AiPersonality_ProtossSkylord,
	AiPersonality_ProtossGeneral,
	AiPersonality_ProtossCount,

	AiPersonality_ZergAssault = 1,
	AiPersonality_ZergSwarm,
	AiPersonality_ZergSunblot,
	AiPersonality_ZergGeneral,
	AiPersonality_ZergCount,

	AiPersonality_AnyCount = std::max(std::max(AiPersonality_TerranCount, AiPersonality_ProtossCount), AiPersonality_ZergCount),
};

enum AiBuildType // plasma_shell, does not include combos
{
	AiBuildType_None = 0,
	AiBuildType_Timing = 1,
	AiBuildType_Rush,
	AiBuildType_Greed,
	AiBuildType_Count,
};

enum AiBuildOrder // dark_swarm_unit
{
	AiBuildOrder_None = 0,
	AiBuildOrder_TerranArmor_PhalanxTiming = 1,
	AiBuildOrder_TerranArmor_FE5Fulc,
	AiBuildOrder_TerranArmor_2FulcVult,
	AiBuildOrder_TerranArmor_WalkerPower,
	AiBuildOrder_TerranArmor_IronCrusade,
	AiBuildOrder_TerranArmor_DavidAndGoliath,
	AiBuildOrder_TerranArmor_Count,

	AiBuildOrder_TerranRaider_8Rax = 1,
	AiBuildOrder_TerranRaider_3RaxMarineAndCleric,
	AiBuildOrder_TerranRaider_FastNuke,
	AiBuildOrder_TerranRaider_4RaxPressure,
	AiBuildOrder_TerranRaider_Uprising,
	AiBuildOrder_TerranRaider_HolyAckmeds,
	AiBuildOrder_TerranRaider_Count,

	AiBuildOrder_TerranOrbital_2PortWraith = 1,
	AiBuildOrder_TerranOrbital_TheDon,
	AiBuildOrder_TerranOrbital_WraithBio,
	AiBuildOrder_TerranOrbital_Hotdrop,
	AiBuildOrder_TerranOrbital_SuddenStarport,
	AiBuildOrder_TerranOrbital_AcesHigh,
	AiBuildOrder_TerranOrbital_Count,

	AiBuildOrder_TerranGeneral_2FactMech = 1,
	AiBuildOrder_TerranGeneral_EarlyRiser,
	AiBuildOrder_TerranGeneral_TheCube,
	AiBuildOrder_TerranGeneral_DoublingUp,
	AiBuildOrder_TerranGeneral_MechIntoAir,
	AiBuildOrder_TerranGeneral_Observant,
	AiBuildOrder_TerranGeneral_Count,

	AiBuildOrder_ProtossRanger_2GateStrider = 1,
	AiBuildOrder_ProtossRanger_1GateRobo,
	AiBuildOrder_ProtossRanger_EmissarySquared,
	AiBuildOrder_ProtossRanger_LatticeExpo,
	AiBuildOrder_ProtossRanger_SimulantSwarm,
	AiBuildorder_ProtossRanger_TheRegistry,
	AiBuildOrder_ProtossRanger_Count,

	AiBuildOrder_ProtossTemplar_Initiates = 1,
	AiBuildOrder_ProtossTemplar_3GateStrider,
	AiBuildOrder_ProtossTemplar_SuffocatingShadow,
	AiBuildOrder_ProtossTemplar_Legion,
	AiBuildOrder_ProtossTemplar_Martyr,
	AiBuildOrder_ProtossTemplar_LegionsHeralds,
	AiBuildOrder_ProtossTemplar_Count,

	AiBuildOrder_ProtossSkylord_DeadlyOmen = 1,
	AiBuildOrder_ProtossSkylord_SolarEscort,
	AiBuildOrder_ProtossSkylord_2GatePanoptus,
	AiBuildOrder_ProtossSkylord_Exalted,
	AiBuildOrder_ProtossSkylord_SorrowFleet,
	AiBuildOrder_ProtossSkylord_ShipsAndSwords,
	AiBuildOrder_ProtossSkylord_Count,

	AiBuildOrder_ProtossGeneral_RoboFE = 1,
	AiBuildOrder_ProtossGeneral_StridentStriders,
	AiBuildOrder_ProtossGeneral_Worship,
	AiBuildOrder_ProtossGeneral_LunaticLegion,
	AiBuildOrder_ProtossGeneral_TheSwordsmiths,
	AiBuildOrder_ProtossGeneral_Matriarchy,
	AiBuildOrder_ProtossGeneral_Count,

	AiBuildOrder_ZergAssault_3HatchHydrok = 1,
	AiBuildOrder_ZergAssault_2HatchZorkiz,
	AiBuildOrder_ZergAssault_2HatchHydrokQuazrok,
	AiBuildOrder_ZergAssault_2HatchGeszkath,
	AiBuildOrder_ZergAssault_Potency,
	AiBuildOrder_ZergAssault_EarthenTides,
	AiBuildOrder_ZergAssault_Count,

	AiBuildOrder_ZergSwarm_3HatchFlood = 1,
	AiBuildOrder_ZergSwarm_3HatchUltrok,
	AiBuildOrder_ZergSwarm_InstantQuazrok,
	AiBuildOrder_ZergSwarm_3HatchOthstol,
	AiBuildOrder_ZergSwarm_GatesOfHell,
	AiBuildOrder_ZergSwarm_PrimitivePressure,
	AiBuildOrder_ZergSwarm_Count,

	AiBuildOrder_ZergSunblot_BigAndLittle = 1,
	AiBuildOrder_ZergSunblot_TheTriad,
	AiBuildOrder_ZergSunblot_SkitteringDuo,
	AiBuildOrder_ZergSunblot_EarlyBirds,
	AiBuildOrder_ZergSunblot_FlyingFeint,
	AiBuildOrder_ZergSunblot_DescendingHorizon,
	AiBuildOrder_ZergSunblot_Count,

	AiBuildOrder_ZergGeneral_SleepingGiant = 1,
	AiBuildOrder_ZergGeneral_ShriekingSwarm,
	AiBuildOrder_ZergGeneral_Subjugation,
	AiBuildOrder_ZergGeneral_BeforeTheStorm,
	AiBuildOrder_ZergGeneral_TimeAfterTime,
	AiBuildOrder_ZergGeneral_ConstrainedCoil,
	AiBuildOrder_ZergGeneral_Count,
};

enum UnitResponseContext {
	URC_Normal	= 0,
	URC_Target	= 1,
	URC_OrderId = 2,
};

// From public gptp, note "may be wrong" -Pr0nogo
namespace PanelTooltipTypes
{
	enum Enum
	{
		WeaponPanelTooltip		= 0,	// set in getWeaponTooltipString
		ArmorPanelTooltip		= 1,	// set in getArmorTooltipString
		ShieldPanelTooltip		= 2,	// set in getShieldTooltipString
		SpecialPanelTooltip		= 3,	// some possibilities set in getSpecialTooltipString
		ResearchPanelTooltip	= 4,
		UpgradePanelTooltip		= 5,
		NoPanelTooltip			= 6,
	};
}

namespace ButtonsetId
{
	enum Enum
	{
		TerranStructuresBasic		= 71,
		TerranStructuresAdvanced	= 72,
		TerranStructuresSupreme		= 248,
		ProtossStructuresCore		= 45,
		ProtossStructuresTemplar	= 46,
		ProtossStructuresRobotics	= 115,
		ProtossStructuresStellar	= 116,
		ProtossStructuresProfane	= 117,
		ZergStructuresBasic			= 19,
		ZergStructuresAdvanced		= 20,
		ZergStructuresSupreme		= 247,
		LarvaSwarm					= 225,
		LarvaAssault				= 226,
		Blank						= 228,
		LarvaControl				= 227,
		LarvaShared					= 242,
	};
}

namespace IscriptOpcodes
{
	enum Enum
	{
		opc_playfram				= 0x00,
		opc_playframtile			= 0x01,
		opc_sethorpos				= 0x02,
		opc_setvertpos				= 0x03,
		opc_setpos					= 0x04,
		opc_wait					= 0x05,
		opc_waitrand				= 0x06,
		opc_goto					= 0x07,
		opc_imgol					= 0x08,
		opc_imgul					= 0x09,
		opc_imgolorig				= 0x0A,
		opc_switchul				= 0x0B,
		opc___0c					= 0x0C,
		opc_imgoluselo				= 0x0D,
		opc_imguluselo				= 0x0E,
		opc_sprol					= 0x0F,
		opc_highsprol				= 0x10,
		opc_lowsprul				= 0x11,
		opc_uflunstable				= 0x12,
		opc_spruluselo				= 0x13,
		opc_sprul					= 0x14,
		opc_sproluselo				= 0x15,
		opc_end						= 0x16,
		opc_setflipstate			= 0x17,
		opc_playsnd					= 0x18,
		opc_playsndrand				= 0x19,
		opc_playsndbtwn				= 0x1A,
		opc_domissiledmg			= 0x1B,
		opc_attackmelee				= 0x1C,
		opc_followmaingraphic		= 0x1D,
		opc_randcondjmp				= 0x1E,
		opc_turnccwise				= 0x1F,
		opc_turncwise				= 0x20,
		opc_turn1cwise				= 0x21,
		opc_turnrand				= 0x22,
		opc_setspawnframe			= 0x23,
		opc_sigorder				= 0x24,
		opc_attackwith				= 0x25,
		opc_attack					= 0x26,
		opc_castspell				= 0x27,
		opc_useweapon				= 0x28,
		opc_move					= 0x29,
		opc_gotorepeatattk			= 0x2A,
		opc_engframe				= 0x2B,
		opc_engset					= 0x2C,
		opc___2d					= 0x2D,
		opc_nobrkcodestart			= 0x2E,
		opc_nobrkcodeend			= 0x2F,
		opc_ignorerest				= 0x30,
		opc_attkshiftproj			= 0x31,
		opc_tmprmgraphicstart		= 0x32,
		opc_tmprmgraphicend			= 0x33,
		opc_setfldirect				= 0x34,
		opc_call					= 0x35,
		opc_return					= 0x36,
		opc_setflspeed				= 0x37,
		opc_creategasoverlays		= 0x38,
		opc_pwrupcondjmp			= 0x39,
		opc_trgtrangecondjmp		= 0x3A,
		opc_trgtarccondjmp			= 0x3B,
		opc_curdirectcondjmp		= 0x3C,
		opc_imgulnextid				= 0x3D,
		opc___3e					= 0x3E,
		opc_liftoffcondjmp			= 0x3F,
		opc_warpoverlay				= 0x40,
		opc_orderdone				= 0x41,
		opc_grdsprol				= 0x42,
		opc___43					= 0x43,
		opc_dogrddamage				= 0x44,
	};
}

enum GameCommand {
	GameCommand_Sequence			= 5,
	GameCommand_SaveGame			= 6,
	GameCommand_LoadGame			= 7,
	GameCommand_RestartGame			= 8,
	GameCommand_Select				= 9,
	GameCommand_ShiftSelect			= 10,
	GameCommand_ShiftDeselect		= 11,
	GameCommand_Build				= 12,
	GameCommand_Vision				= 13,
	GameCommand_Ally				= 14,
	GameCommand_GameSpeed			= 15,
	GameCommand_PauseGame			= 16,
	GameCommand_ResumeGame			= 17,
	GameCommand_UseCheat			= 18,
	GameCommand_Hotkey				= 19,
	GameCommand_RightClick			= 20,
	GameCommand_TargetCommand		= 21,
	GameCommand_CancelConstruction	= 24,
	GameCommand_CancelUnitMorph		= 25,
	GameCommand_Stop				= 26,
	GameCommand_CarrierStop			= 27,
	GameCommand_ReaverStop			= 28,
	GameCommand_Nothing				= 29,
	GameCommand_ReturnCargo			= 30,
	GameCommand_Train				= 31,
	GameCommand_CancelTrain			= 32,
	GameCommand_Cloak				= 33,
	GameCommand_Decloak				= 34,
	GameCommand_UnitMorph			= 35,
	GameCommand_Unsiege				= 37,
	GameCommand_Siege				= 38,
	GameCommand_TrainFighter		= 39,
	GameCommand_UnloadAll			= 40,
	GameCommand_Unload				= 41,
	GameCommand_MergeArchon			= 42,
	GameCommand_HoldPosition		= 43,
	GameCommand_Burrow				= 44,
	GameCommand_Unburrow			= 45,
	GameCommand_CancelNuke			= 46,
	GameCommand_Liftoff				= 47,
	GameCommand_Research			= 48,
	GameCommand_CancelResearch		= 49,
	GameCommand_Upgrade				= 50,
	GameCommand_CancelUpgrade		= 51,
	GameCommand_CancelAddon			= 52,
	GameCommand_BuildingMorph		= 53,
	GameCommand_Stimpack			= 54,
	GameCommand_Hash				= 55,
	GameCommand_VoiceEnable1		= 56,
	GameCommand_VoiceEnable2		= 57,
	GameCommand_Unused1				= 58,
	GameCommand_Unused2				= 59,
	GameCommand_SetLatency			= 85,
	GameCommand_SetReplaySpeed		= 86,
	GameCommand_LeaveGame			= 87,
	GameCommand_MinimapPing			= 88,
	GameCommand_MergeDarkArchon		= 90,
	GameCommand_DisplaySomeText		= 92,
	GameCommand_Custom				= 99,
};

enum UserActionFlags {
	UserActionFlags_IscriptGoToIdle	= 0x1, // after NoBrkCodeEnd, test at 0x00475403
	UserActionFlags_UMStartPath		= 0x2, // test at 0x0046BDAD
	UserActionFlags_HideUnit		= 0x4, // remove unit, test at 0x00479487
};

enum PathingFlags {
	PathingFlags_ProvidesCollision	= 0x1,
	PathingFlags_IsStacked			= 0x2,
	PathingFlags_DoDecollide		= 0x4,
};

enum MovementType {
	MovementType_Flingy				= 0x0,
	MovementType_Partial			= 0x1,
	MovementType_Iscript			= 0x2,
};

enum AiTargetResource {
	AiTargetResource_Mineral = 1,
	AiTargetResource_Gas     = 2,
	// BW used to also use 3 and 4 for gas, but now we use automatic assignment
};

enum UnitMovement {
	UnitMovement_Init				= 0,
	UnitMovement_InitSeq			= 1,
	UnitMovement_Lump				= 2,
	UnitMovement_Turret				= 3,
	UnitMovement_Bunker				= 4,
	UnitMovement_Tower				= 5,
	UnitMovement_Hidden				= 6,
	UnitMovement_Flyer				= 7,
	UnitMovement_FakeFlyer			= 8,
	UnitMovement_AtRest				= 9,
	UnitMovement_Dormant			= 10,
	UnitMovement_AtMoveTarget		= 11,
	UnitMovement_CheckIllegal		= 12,
	UnitMovement_MoveToLegal		= 13,
	UnitMovement_LumpWannabe		= 14,
	UnitMovement_FailedPath			= 15,
	UnitMovement_RetryPath			= 16,
	UnitMovement_StartPath			= 17,
	UnitMovement_UIOrderDelay		= 18,
	UnitMovement_TurnAndStart		= 19,
	UnitMovement_FaceTarget			= 20,
	UnitMovement_NewMoveTarget		= 21,
	UnitMovement_AnotherPath		= 22,
	UnitMovement_Repath				= 24,
	UnitMovement_RepathMovers		= 25,
	UnitMovement_FollowPath			= 26,
	UnitMovement_ScoutPath			= 27,
	UnitMovement_ScoutFree			= 28,
	UnitMovement_FixCollision		= 29,
	UnitMovement_WaitFree			= 30,
	UnitMovement_GetFree			= 31,
	UnitMovement_SlidePrep			= 32,
	UnitMovement_SlideFree			= 33,
	UnitMovement_ForceMoveFree		= 34,
	UnitMovement_FixTerrain			= 35,
	UnitMovement_TerrainSlide		= 36,
};

enum AdvisorEvent {
	AdvisorEvent_NoMinerals				= 0,
	AdvisorEvent_NoVespene				= 1,
	AdvisorEvent_NoEnergy				= 2,
	AdvisorEvent_NoBuildTerran			= 3,
	AdvisorEvent_NoBuildProtoss			= 4,
	AdvisorEvent_NoBuildZerg			= 5,
	AdvisorEvent_NoLanding				= 6,
	AdvisorEvent_LandingInterrupted		= 7,
	AdvisorEvent_PlacementInterrupted	= 8,

	AdvisorEvent_StructureComplete_T	= 9,
	AdvisorEvent_StructureComplete_P	= 10,
	AdvisorEvent_StructureComplete_Z	= 11,
	AdvisorEvent_AddonComplete			= 12,
	AdvisorEvent_TechUpComplete			= 13,
	AdvisorEvent_MissileComplete		= 14,

	AdvisorEvent_UnitsUnderAttack		= 15,
	AdvisorEvent_BaseUnderAttack		= 16,
	AdvisorEvent_MissileLaunch			= 17,
	AdvisorEvent_AllyUnderAttack_T		= 18,
	AdvisorEvent_AllyUnderAttack_P		= 19,
	AdvisorEvent_AllyUnderAttack_Z		= 20,
};

enum AdvisorMessageId {
	AdvisorMessageId_NoMinerals				= 850,
	AdvisorMessageId_NoVespene				= 851,
	AdvisorMessageId_NoEnergy				= 864,
	AdvisorMessageId_NoEnergy2				= 865,
	AdvisorMessageId_NoEnergy3				= 866,
	AdvisorMessageId_NoBuildResources		= 852,
	AdvisorMessageId_NoBuildOffMap			= 854,
	AdvisorMessageId_NoBuildMapEdge			= 855,
	AdvisorMessageId_NoBuildStandard		= 856,
	AdvisorMessageId_NoBuildExplore			= 857,
	AdvisorMessageId_NoBuildGeyser			= 859,
	AdvisorMessageId_NoBuildZerg			= 860,
	AdvisorMessageId_NoBuildProtoss			= 861,
	AdvisorMessageId_NoBuildCouldntReach	= 862,
	AdvisorMessageId_NoLanding				= 863,
};

enum DialogEventType {
	DialogEventType_KeyDown					= 0,
	DialogEventType_KeyRepeat				= 1,
	DialogEventType_KeyUp					= 2,
	DialogEventType_MouseMove				= 3,
	DialogEventType_LButtonDown				= 4,
	DialogEventType_LButtonUp				= 5,
	DialogEventType_LButtonDouble			= 6,
	DialogEventType_RButtonDown				= 7,
	DialogEventType_RButtonUp				= 8,
	DialogEventType_RButtonDouble			= 9,
	DialogEventType_MButtonDown				= 10,
	DialogEventType_MButtonUp				= 11,
	DialogEventType_MButtonDouble			= 12,
	DialogEventType_Idle					= 13,
	DialogEventType_Dialog					= 14,
	DialogEventType_Spacebar				= 15,
	DialogEventType_SystemKey				= 16,
	DialogEventType_MWheelUp				= 17,
	DialogEventType_MWheelDown				= 18,
};

enum DialogUserType {
	DialogUserType_Create = 0,
	DialogUserType_Destroy = 1,
	DialogUserType_Activate = 2,
	DialogUserType_Hotkey = 3,
	DialogUserType_MouseOver = 4,
	DialogUserType_SelectPrev = 5,
	DialogUserType_SelectNext = 6,
	DialogUserType_Generic = 7,
	DialogUserType_InitSelf = 8,
	DialogUserType_MouseEnter = 9,
	DialogUserType_InitChildren = 10,
	DialogUserType_SetSelectionIndex = 11,
	DialogUserType_Unknown = 12,
	DialogUserType_Show = 13,
	DialogUserType_Hide = 14,
	DialogUserType_ScrollUp = 15,
	DialogUserType_ScrollDown = 16,
};