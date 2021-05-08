#pragma once

//V241 for VS2008

//General use
namespace TechId {
enum Enum {
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
	NuclearStrike			= 45, //based on bwapi/BWMemoryEdit/Enums/TechType.cs
	UnknownLastEntry		= 46,	//based on bwapi/BWMemoryEdit/Enums/TechType.cs

	// HYDRA \\

//Terran
	LobotomyMine			=  1, // Eidolon lay trap ability
	ReverseThrust			=  2, // unused
	LandGrab				= 17, // General target ground ability
	TertiaryEyes			= 24, // Siren specialist linking ability
	LazarusAgent			= 32, // Apostle ability
	TempoChange				= 36, // Madrigal weapon switch ability
	Skyfall					= 37, // Paladin missile salvo ability
	NaniteField				= 38, // Shaman mech repair ability
	Observance				= 39, // Seraph armor reduction ability
	SublimeShepherd			= 40, // Azazel cloud cover ability
	PowerTransfer			= 41, // Savant energy transfer ability

//Protoss
	PlanetCracker			= 29, // Star Sovereign damage ability
	PhaseLink				= 33, // unused
	RescindMindControl		= 26, // Mind Tyrant option
	GodsMadeFlesh			= 42, // Demiurge immortality ability

//Zerg
	EnsnaringBrood			= 13, // Matraleth disruptive enchantment ability
	Reconstitution			= 26, // unused
	EntrallingAssault		= 35, // Akistrokor infestation ability

	// extended
	// ChasmalColonySwitch = 47,	// Chasmal Colony - Adaptive Barrier / Adrenal Secretion
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

