#pragma once

namespace TechId {
	enum Enum {
		// Cosmonarchy \\

	// Shared
		TargetGround					= 17, // Ensnare
		UnusedAnticthonFailureMode		= 23, // Archon Warp
		CancelNewAbility				= 26, // Unused Tech 26 | used for: Land Grab, Mind Control, Enthralling Assault, Conjoining Root
		LuminaryAuralAssault			= 30, // Optical Flare

	// Terran
		EidolonLobotomyMine				=  1, // Lockdown
		ControlledDemolition			=  2, // EMP Shockwave
		SavantOnhitToggle				= 24, // Restoration
		ApostleLazarusAgent				= 32, // Lurker Aspect
		MadrigalTempoChange				= 36, // Unused Tech 36
		PaladinSkyfall					= 37, // Unused Tech 37
		EmergencyDetonation				= 39, // Unused Tech 39
		AzazelSublimeShepherd			= 40, // Unused Tech 40
		TalosSalvage					= 41, // Unused Tech 41

	// Protoss
		StarSovereignTheGriefOfAllGods	= 29, // Feedback
		DemiurgeGodsMadeFlesh			= 42, // Unused Tech 42

	// Zerg
		MatralethEnsnaringBrood			= 13, // Spawn Broodlings
		LarvalSubmenuSelectControl		= 28, // Dark Archon Meld
		LarvalSubmenuSelectSwarm		= 33, // Unused Tech 33
		LarvalSubmenuSelectAssault		= 38, // Unused Tech 38
		AkistrokorBlissfulEmbrace		= 35, // Unused Tech 35

	// techdata.dat is not yet extended
	//	ChasmalColonySwitch				= 47,

		// Brood War \\

		Stimpacks				=  0,
		Lockdown				=  1,
		EMPShockwave			=  2,
		SpiderMines				=  3,
		ScannerSweep			=  4, // (default)
		TankSiegeMode			=  5,
		DefensiveMatrix			=  6, // (default)
		Irradiate				=  7,
		YamatoGun				=  8,
		CloakingField			=  9,
		PersonnelCloaking		= 10,
		Burrowing				= 11,
		Infestation				= 12, // (default)
		SpawnBroodlings			= 13,
		DarkSwarm				= 14, // (default)
		Plague					= 15,
		Consume					= 16,
		Ensnare					= 17,
		Parasite				= 18, // (default)
		PsionicStorm			= 19,
		Hallucination			= 20,
		Recall					= 21,
		StasisField				= 22,
		ArchonWarp				= 23, // (default)
		Restoration				= 24,
		DisruptionWeb			= 25,
		UnusedTech26			= 26, // (unused)
		MindControl				= 27,
		DarkArchonMeld			= 28, // (default)
		Feedback				= 29, // (default)
		OpticalFlare			= 30,
		Maelstrom				= 31,
		LurkerAspect			= 32,
		UnusedTech33			= 33, // (default; possibly liftoff)
		Healing					= 34, // (default)
		UnusedTech35			= 35, // (unused)
		UnusedTech36			= 36, // (unused)
		UnusedTech37			= 37, // (unused)
		UnusedTech38			= 38, // (unused)
		UnusedTech39			= 39, // (unused)
		UnusedTech40			= 40, // (unused)
		UnusedTech41			= 41, // (unused)
		UnusedTech42			= 42, // (unused)
		UnusedTech43			= 43, // (unused)
		None					= 44,
		NuclearStrike			= 45,
		UnknownLastEntry		= 46, // based on bwapi/BWMemoryEdit/Enums/TechType.cs
	};
}


// For use with techdata_dat::AvailableSC[] and techdata_dat::ResearchedBw[]
namespace ScTech {
	enum Enum {
		Stimpacks         =  0,
		Lockdown          =  1,
		EMPShockwave      =  2,
		SpiderMines       =  3,
		ScannerSweep      =  4,	// (default)
		TankSiegeMode     =  5,
		DefensiveMatrix   =  6,	// (default)
		Irradiate         =  7,
		YamatoGun         =  8,
		CloakingField     =  9,
		PersonnelCloaking = 10,
		Burrowing         = 11,
		Infestation       = 12,	// (default)
		SpawnBroodlings   = 13,
		DarkSwarm         = 14,	// (default)
		Plague            = 15,
		Consume           = 16,
		Ensnare           = 17,
		Parasite          = 18,	// (default)
		PsionicStorm      = 19,
		Hallucination     = 20,
		Recall            = 21,
		StasisField       = 22,
		ArchonWarp        = 23,	// (default)
	};
}

//For use with techdata_dat::AvailableBW[] and techdata_dat::ResearchedBw[]
namespace BwTech {
	enum Enum {
		Restoration       =  0,
		DisruptionWeb     =  1,
		UnusedTech26      =  2,	// (unused)
		MindControl       =  3,
		DarkArchonMeld    =  4,	// (default)
		Feedback          =  5,	// (default)
		OpticalFlare      =  6,
		Maelstrom         =  7,
		LurkerAspect      =  8,
		UnusedTech33      =  9,	// (default; possibly liftoff)
		Healing           = 10,	// (default)
		UnusedTech35      = 11,	// (unused)
		UnusedTech36      = 12,	// (unused)
		UnusedTech37      = 13,	// (unused)
		UnusedTech38      = 14,	// (unused)
		UnusedTech39      = 15,	// (unused)
		UnusedTech40      = 16,	// (unused)
		UnusedTech41      = 17,	// (unused)
		UnusedTech42      = 18,	// (unused)
		UnusedTech43      = 19,	// (unused)
	};
}

