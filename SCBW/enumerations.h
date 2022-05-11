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

namespace UnitSize
{
	enum
	{
		Independent,
		Small,
		Medium,
		Large,
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
		Flingy,//flingy
		Subunit, //subunit
		Infested, //infested unit
		Construction, //construction graphic
		StartDirection, //start direction
		ShieldEnabled, //shield enabled
		MaxShields, //max shields

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
		1, //ground weapon
		1, //max ground hits
		1, //air weapon
		1, //max air hits
		1, //computer ai unit type
		4, //flags
		1, //seek range
		1, //sight range
		1, //armor upgrade
		1, //size
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
		Race,
		MinVolume,
		Flags,
		Priority,
		SoundFilePtrs = 1000, // unlike sfxdata, uses pointers instead of ids
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

namespace AiType
{
	enum Enum
	{
		Unassigned	= 0, // Never this?
		Guard		= 1,
		Worker		= 2,
		Building	= 3, // Also "available to scripts"
		Military	= 4,
	};
}

namespace RegionState
{
	enum Enum
	{
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
		NotUsed = 0,
		Computer = 1,
		Human = 2,
		Rescuable = 3,
		Unknown0 = 4,
		ComputerSlot = 5,
		OpenSlot = 6,
		Neutral = 7,
		ClosedSlot = 8,
		Unknown1 = 9,
		HumanDefeated = 10, // Or left the game
		ComputerDefeated = 11, // Or left the game
	};
}

// Based on BWAPI; for use with units_dat::BaseProperty[]
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
		NeutralAccessories = 0x00000800, //AKA "Single entity" (prevents multi-select, set on all pickup items)
		ResourceDepot = 0x00001000,	//Place where resources are brought back
		ResourceContainer = 0x00002000,	//Resource Source
		RoboticUnit = 0x00004000,
		Detector = 0x00008000,
		Organic = 0x00010000,
		CreepBuilding = 0x00020000,
		Unused = 0x00040000,
		RequiredPsi = 0x00080000,
		Burrowable = 0x00100000,
		Spellcaster = 0x00200000,
		PermanentCloak = 0x00400000,
		NPCOrAccessories = 0x00800000, //AKA "Pickup item" (data disc, crystals, mineral chunks, gas tanks, etc.)
		MorphFromOtherUnit = 0x01000000,
		MediumOverlay = 0x02000000, //Used to determine overlay for various spells and effects
		LargeOverlay = 0x04000000, //Used to determine overlay for various spells and effects
		AutoAttackAndMove = 0x08000000,
		Attack = 0x10000000, /**< Can attack */	
		Invincible = 0x20000000,
		Mechanical = 0x40000000,
		ProducesUnits = 0x80000000, /**< It can produce units directly (making buildings doesn't count) */
	};
}

// Based on BWAPI; For use with the Unit::status member
namespace UnitStatus
{
	enum Enum : u32
	{
		Completed = 0x00000001,
		GroundedBuilding = 0x00000002, // a building that is on the ground
		InAir = 0x00000004,
		Disabled = 0x00000008,  /**< Protoss Unpowered */
		Burrowed = 0x00000010,
		InBuilding = 0x00000020,
		InTransport = 0x00000040,
		CanBeChased = 0x00000080,  /**< @todo Unknown */
		RequiresDetection = 0x00000100,
		Cloaked = 0x00000200,
		Frozen = 0x00000400,  ///< Unknown. ++protoss unpowered buildings have this flag set
		CloakingForFree = 0x00000800,  /**< Requires no energy to cloak */
		CanNotReceiveOrders = 0x00001000,
		NoBrkCodeStart = 0x00002000,  /**< Unbreakable code section in iscript */
		UNKNOWN2 = 0x00004000,  /**< @todo Unknown */
		CanNotAttack = 0x00008000,  /**< @todo Unknown */
		CanTurnAroundToAttack = 0x00010000,  // canAttack? /**< @todo Unknown */ named IsAUnit in BWAPI
		IsBuilding = 0x00020000,
		IgnoreTileCollision = 0x00040000,
		Unmovable = 0x00080000,
		IsNormal = 0x00100000,  /**< 1 for "normal" units, 0 for hallucinated units */
		NoCollide = 0x00200000, // ++if set, other units wont collide with the unit (like burrowed units)
		UNKNOWN5 = 0x00400000,
		IsGathering = 0x00800000, // ++if set, the unit wont collide with other units (like worker gathering)
		UNKNOWN6 = 0x01000000,
		UNKNOWN7 = 0x02000000, // Turret related
		Invincible = 0x04000000,
		HoldingPosition = 0x08000000, // Set if the unit is currently holding position
		SpeedUpgrade = 0x10000000,
		CooldownUpgrade = 0x20000000,
		IsHallucination = 0x40000000,  /**< 1 for hallucinated units, 0 for "normal" units */
		IsSelfDestructing = 0x80000000  // Set for when the unit is self-destructing (scarab, scourge, infested terran)
	};
}

// For various newFlags fields
namespace NewFlags1
{
	enum Enum : u8
	{
		Unused1 = 0x1,
		RadiationShockwave = 0x2,
		Metastasis = 0x4,
	};
}

namespace NewFlags2
{
	enum Enum : u8
	{
		Unused1 = 0x1,
		Unused2 = 0x2,
		Unused4 = 0x4,
	};
}

namespace BUTTON_STATE
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

// For use with scbw::playIscriptAnim()
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

// Use with weapons_dat::ExplosionType[]
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
		EmpShockwave = 8,
		Irradiate = 9,
		Ensnare = 10,
		Plague = 11,
		StasisField = 12,
		DarkSwarm = 13,
		Consume = 14,
		YamatoGun = 15,
		Restoration = 16,
		DisruptionWeb = 17,
		CorrosiveAcid = 18,
		MindControl = 19,
		Feedback = 20,
		OpticalFlare = 21,
		Maelstrom = 22,
		Unknown_Crash = 23,
		SplashAir = 24
	};
}

// Use with weapons_dat::Behavior[]
namespace WeaponBehavior
{
	enum Enum
	{
		Fly_DoNotFollowTarget = 0,
		Fly_FollowTarget = 1,
		AppearOnTargetUnit = 2,
		PersistOnTargetSite = 3,
		AppearOnTargetSite = 4,
		AppearOnAttacker = 5,
		AttackAndSelfDestruct = 6,
		Bounce = 7,
		AttackNearbyArea = 8,
		GoToMaxRange = 9
	};
}

// Use with colorShift
namespace ColorRemapping
{
	enum Enum
	{
		None = 0,
		OFire = 1,
		GFire = 2,
		BFire = 3,
		BExpl = 4,
		Trans50 = 5,  // Special - OwnCloak
		Red_Crash = 6,
		Green_Crash = 7,
	};
}

// Compare with (*GAME_TYPE)
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
	};
}

// Compare with (*CURRENT_TILESET)
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

namespace MovableState
{
	enum Enum
	{
		NotReachedDestination = 0,
		ReachedDestination = 1,
		UnmovableAtDestination = 2
	};
}

namespace BinDlgFlags
{
	enum Enum
	{
		Unknown0 = 0x00000001,
		Disabled = 0x00000002,
		Active = 0x00000004,
		Visible = 0x00000008,
		RespondsToEvents = 0x00000010,
		Unknown5 = 0x00000020,
		CancelButton = 0x00000040,
		NoHoverSound = 0x00000080,
		SpecialHotkeyVirtualKey = 0x00000100,
		HasHotkey = 0x00000200,
		SmallestFontSize10 = 0x00000400,
		LargerFontSize16 = 0x00000800,
		Unknown12 = 0x00001000,
		TransparentHideColor0 = 0x00002000,
		LargestFontSize16x = 0x00004000,
		AlternateStyleWC2 = 0x00008000,
		SmallerFontSize14 = 0x00010000,
		RemoveStyles = 0x00020000,
		ApplyTranslucency = 0x00040000,
		DefaultButton = 0x00080000,
		BringToFront = 0x00100000,
		HorizontalAlignmentLeft = 0x00200000,
		HorizontalAlignmentRight = 0x00400000,
		HorizontalAlignmentCenter = 0x00800000,
		VerticalAlignmentTop = 0x01000000,
		VerticalAlignmentMiddle = 0x02000000,
		VerticalAlignmentBottom = 0x04000000,
		Unknown27 = 0x08000000,
		ReverseDialogDirection = 0x10000000,
		UseAlternateStyle = 0x20000000,
		NoClickSound = 0x40000000,
		HighlightText = 0x80000000
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
		LabelRightAlign = 0x0000000A,
		LabelCenterAlign = 0x0000000B,
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

		Count,
	};
}

static const char* playerColorNames[] =
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
	"Bdazzled Blue",
	"Coyote Brown",
	"Midnight Green",
	"Patriotic Freedom",
	"Palatinate Blue",
	"Dark Lavender",
	"Forest Green",
	"Zinnwaldite Brown",
	"Neon Carrot",
	"Aquamarine",
};

enum VespeneDeaths
{
	VespeneDeaths_Reveal				= 0x01,
	VespeneDeaths_Ping					= 0x02,
	VespeneDeaths_ToggleRandomizeColor	= 0x04,
};

enum AiPersonality // disruption_web_unit
{
	AiPersonality_None = 0,
	AiPersonality_TerranArmor = 1,
	AiPersonality_TerranRaider,
	AiPersonality_TerranOrbital,
	AiPersonality_TerranShadow,
	AiPersonality_TerranGeneral,
	AiPersonality_TerranCount,

	AiPersonality_ProtossRanger = 1,
	AiPersonality_ProtossTemplar,
	AiPersonality_ProtossSkylord,
	AiPersonality_ProtossProfane,
	AiPersonality_ProtossGeneral,
	AiPersonality_ProtossCount,

	AiPersonality_ZergAssault = 1,
	AiPersonality_ZergSwarm,
	AiPersonality_ZergSunblot,
	AiPersonality_ZergMiscreant,
	AiPersonality_ZergGeneral,
	AiPersonality_ZergCount,

	AiPersonality_AnyCount = std::max(
		std::max(AiPersonality_TerranCount, AiPersonality_ProtossCount), AiPersonality_ZergCount),
};

enum AiBuildType // plasma_shell
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
	AiBuildOrder_TerranArmor_FE5Fact,
	AiBuildOrder_TerranArmor_2FactVult,
	AiBuildOrder_TerranArmor_WalkerPower,
	AiBuildOrder_TerranArmor_IronCrusade,
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
	AiBuildOrder_TerranOrbital_Count,

	AiBuildOrder_TerranShadow_RoyalFlush = 1,
	AiBuildOrder_TerranShadow_UnholyTrinity,
	AiBuildOrder_TerranShadow_AngelsAbove,
	AiBuildOrder_TerranShadow_TheVerdict,
	AiBuildOrder_TerranShadow_Count,

	AiBuildOrder_TerranGeneral_2FactMech = 1,
	AiBuildOrder_TerranGeneral_EarlyRiser,
	AiBuildOrder_TerranGeneral_TheCube,
	AiBuildOrder_TerranGeneral_DoublingUp,
	AiBuildOrder_TerranGeneral_MechIntoAir,
	AiBuildOrder_TerranGeneral_Observant,
	AiBuildOrder_TerranGeneral_Count,

	AiBuildOrder_ProtossRanger_2GateStrider = 1,
	AiBuildOrder_ProtossRanger_1GateRobo,
	AiBuildOrder_ProtossRanger_ForgeFEAccantor,
	AiBuildOrder_ProtossRanger_1GateExpo,
	AiBuildOrder_ProtossRanger_SimulantSwarm,
	AiBuildOrder_ProtossRanger_Count,

	AiBuildOrder_ProtossTemplar_Initiates = 1,
	AiBuildOrder_ProtossTemplar_3GateStrider,
	AiBuildOrder_ProtossTemplar_2GateCabalist,
	AiBuildOrder_ProtossTemplar_Legion,
	AiBuildOrder_ProtossTemplar_Martyr,
	AiBuildOrder_ProtossTemplar_Count,

	AiBuildOrder_ProtossSkylord_FEStargate = 1,
	AiBuildOrder_ProtossSkylord_SolarEscort,
	AiBuildOrder_ProtossSkylord_2GatePanoptus,
	AiBuildOrder_ProtossSkylord_Exalted,
	AiBuildOrder_ProtossSkylord_SorrowFleet,
	AiBuildOrder_ProtossSkylord_Count,

	AiBuildOrder_ProtossProfane_TheBanished = 1,
	AiBuildOrder_ProtossProfane_LostAndFound,
	AiBuildOrder_ProtossProfane_CloakAndDagger,
	AiBuildOrder_ProtossProfane_Starbreach,
	AiBuildOrder_ProtossProfane_Count,

	AiBuildOrder_ProtossGeneral_RoboFE = 1,
	AiBuildOrder_ProtossGeneral_StridentStriders,
	AiBuildOrder_ProtossGeneral_2GateRobo,
	AiBuildOrder_ProtossGeneral_LunaticLegion,
	AiBuildOrder_ProtossGeneral_Count,

	AiBuildOrder_ZergAssault_3HatchHydrok = 1,
	AiBuildOrder_ZergAssault_2HatchZorkiz,
	AiBuildOrder_ZergAssault_2HatchHydrokQuazrok,
	AiBuildOrder_ZergAssault_2HatchGeszkath,
	AiBuildOrder_ZergAssault_Count,

	AiBuildOrder_ZergSwarm_3HatchFlood = 1,
	AiBuildOrder_ZergSwarm_3HatchUltrok,
	AiBuildOrder_ZergSwarm_InstantQuazrok,
	AiBuildOrder_ZergSwarm_3HatchOthstol,
	AiBuildOrder_ZergSwarm_GatesOfHell,
	AiBuildOrder_ZergSwarm_Count,

	AiBuildOrder_ZergSunblot_3HatchMuthrokQuazrok = 1,
	AiBuildOrder_ZergSunblot_2HatchGeszithalor,
	AiBuildOrder_ZergSunblot_2HatchMutalisk,
	AiBuildOrder_ZergSunblot_Count,

	AiBuildOrder_ZergMiscreant_Symbiosis = 1,
	AiBuildOrder_ZergMiscreant_Former,
	AiBuildOrder_ZergMiscreant_MisbegottenMight,
	AiBuildOrder_ZergMiscreant_RapidAnnex,
	AiBuildOrder_ZergMiscreant_Count,

	AiBuildOrder_ZergGeneral_FastOthstol = 1,
	AiBuildOrder_ZergGeneral_SwarmingZorkiz,
	AiBuildOrder_ZergGeneral_HydrokMatravil,
	AiBuildOrder_ZergGeneral_ZorkizFlood,
	AiBuildOrder_ZergGeneral_Count,
};