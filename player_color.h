#pragma once
#include "SCBW\api.h"

enum
{
	PlayerTintInvalid = -1,

	// Can be rolled
	PlayerTintDarkRed,
	PlayerTintLightRed,
	PlayerTintBrown,
	PlayerTintOrange,
	PlayerTintYellow,
	PlayerTintGreen,
	PlayerTintDarkBlue,
	PlayerTintLightBlue,
	PlayerTintCyan,
	PlayerTintViolet,
	PlayerTintAchromatic,
	PlayerTintDarkAchromatic,

	PlayerTintRollCount,
	__PlayerTintRollCorrection = PlayerTintRollCount - 1,

	// Can't be rolled
	PlayerTintCycleDependent, // technically still a tint, but can't be rolled. Don't add the assigment if you decide to add more

	PlayerTintCount,
};

// TODO: Convert the vectors into switch statements - it will be more performant, flexible, and readable

const std::vector<std::vector<u32>> player_tints =
{
	{	// Dark Red
		PlayerColor::Maroon,
		PlayerColor::Blood,
		PlayerColor::Mahogany,
		PlayerColor::DarkCoral,
	},
	{	// Light Red
		PlayerColor::Red,
		PlayerColor::Vermillion,
		PlayerColor::Coral,
		PlayerColor::VividRed,
		PlayerColor::Scarlet,
	},
	{	// Brown
		PlayerColor::Brown,
		PlayerColor::Salmon,
		PlayerColor::SaddleBrown,
		PlayerColor::Sienna,
		PlayerColor::Chestnut,
		PlayerColor::DarkBronze,
		PlayerColor::BurntUmber,
		PlayerColor::Liver,
		PlayerColor::Earthtone,
		PlayerColor::GoldenBrown,
		PlayerColor::CoyoteBrown,
		PlayerColor::Wine,
		PlayerColor::Auburn,
		PlayerColor::Cordovan,
		PlayerColor::Redwood,
		PlayerColor::ZinnwalditeBrown,
	},
	{	// Orange
		PlayerColor::Orange,
		PlayerColor::Bronze,
		PlayerColor::Pumpkin,
		PlayerColor::Magma,
		PlayerColor::ElectricOrange,
		PlayerColor::VividOrange,
		PlayerColor::PersianOrange,
		PlayerColor::Tomato,
		PlayerColor::NeonCarrot,
	},
	{	// Yellow
		PlayerColor::Yellow,
		PlayerColor::Lemon,
		PlayerColor::Peach,
		PlayerColor::Saffron,
		PlayerColor::DarkOlive,
		PlayerColor::LightOlive,
		PlayerColor::Goldenrod,
		PlayerColor::ArylideYellow,
		PlayerColor::Brass,
		PlayerColor::Mango,
		PlayerColor::Sunglow,
		PlayerColor::BistreBrown,
		PlayerColor::NeonCarrot,

//		PlayerColor::IridescentYellow,
	},
	{	// Green
		PlayerColor::Green,
		PlayerColor::Asparagus,
		PlayerColor::RussianGreen,
		PlayerColor::Harlequin,
		PlayerColor::Celadon,
		PlayerColor::FernGreen,
		PlayerColor::VineGreen,
		PlayerColor::DarkGreen,
		PlayerColor::Turquoise,
		PlayerColor::Emerald,
		PlayerColor::DarkSpringGreen,
		PlayerColor::ForestGreen,
	},
	{	// Dark Blue
		PlayerColor::Blue,
		PlayerColor::Ultramarine,
		PlayerColor::CosmicCobalt,
		PlayerColor::Zaffre,
		PlayerColor::YinmnBlue,
		PlayerColor::DarkSapphire,
		PlayerColor::BdazzledBlue,
	},
	{	// Light Blue
		PlayerColor::RoyalBlue,
		PlayerColor::Periwinkle,
		PlayerColor::AliceBlue,
		PlayerColor::MayaBlue,
		PlayerColor::ElectricBlue,
		PlayerColor::Glacial,
		PlayerColor::Aero,
		PlayerColor::CornflowerBlue,
		PlayerColor::NeonBlue,
		PlayerColor::PalatinateBlue,
		PlayerColor::Aquamarine,

//		PlayerColor::IridescentBlue,
	},
	{	// Cyan
		PlayerColor::Teal,
		PlayerColor::Asparagus,
		PlayerColor::Cyan,
		PlayerColor::EggshellBlue,
		PlayerColor::Mint,
		PlayerColor::Turquoise,
		PlayerColor::MidnightGreen,
		PlayerColor::Aquamarine,
		
//		PlayerColor::IridescentGreen,
	},
	{	// Violet
		PlayerColor::Purple,
		PlayerColor::TyrianPurple,
		PlayerColor::PalatinatePurple,
		PlayerColor::JapaneseViolet,
		PlayerColor::DarkLavender,

//		PlayerColor::IridescentViolet,
//		PlayerColor::Magenta,
	},
	{	// Achromatic
		PlayerColor::White,
		PlayerColor::DeepPeriwinkle,
		PlayerColor::SilverPink,
		PlayerColor::Silver,
		PlayerColor::Platinum,
		PlayerColor::Snow,
		PlayerColor::Beaver,
		PlayerColor::TeaGreen,
		PlayerColor::Beige,
	},
	{	// Dark Achromatic
		PlayerColor::Black,
		PlayerColor::Umber,
		PlayerColor::JetBlack,
		PlayerColor::DimGrey,
		PlayerColor::Grey,
		PlayerColor::SmokyBlack,
		PlayerColor::DelftBlue,
	},
	{	// Cycle-dependent
//		PlayerColor::Magenta,
	},
};

const std::vector<std::vector<u32>> banned_tileset_tints =
{
	{ // Badlands
		PlayerTintCycleDependent,
		PlayerTintDarkAchromatic,
	},
	{ // Space Platform
		PlayerTintDarkAchromatic,
	},
	{ // Installation
		PlayerTintDarkAchromatic,
	},
	{ // Ash
		PlayerTintCycleDependent,
		PlayerTintDarkAchromatic,
	},
	{ // Jungle
		PlayerTintCycleDependent,
		PlayerTintDarkAchromatic,
	},
	{ // Desert
		PlayerTintCycleDependent,
		PlayerTintDarkAchromatic,
		PlayerTintBrown,
	},
	{ // Ice
		PlayerTintCycleDependent,
		PlayerTintAchromatic,
		PlayerTintDarkAchromatic,
	},
	{ // Twilight
		PlayerTintCycleDependent,
		PlayerTintDarkAchromatic,
	},
};

const std::vector<std::vector<u32>> banned_race_tints =
{
	{ // Zerg
		PlayerTintDarkBlue,
		PlayerTintLightBlue,
	},
	{ // Terran

	},
	{ // Protoss
		PlayerTintBrown,
	},
};

const std::vector<std::vector<std::vector<u32>>> personality_colors =
{
	{ // Zerg
		{ // Assault
			PlayerColor::Red,
			PlayerColor::Purple,
			PlayerColor::Maroon,
			PlayerColor::Blood,
			PlayerColor::Magma,
			PlayerColor::Redwood,
			PlayerColor::Auburn,
			PlayerColor::Wine,
			PlayerColor::Mahogany,
		},
		{ // Swarm
			PlayerColor::Coral,
			PlayerColor::Salmon,
			PlayerColor::Tomato,
			PlayerColor::DarkCoral,
			PlayerColor::Yellow,
			PlayerColor::Lemon,
			PlayerColor::Saffron,
			PlayerColor::Goldenrod,
			PlayerColor::ArylideYellow,
			PlayerColor::Brass,
			
//			PlayerColor::IridescentYellow,
		},
		{ // Sunblot
			PlayerColor::White,
			PlayerColor::SilverPink,
			PlayerColor::Silver,
			PlayerColor::Platinum,
			PlayerColor::Beige,
			PlayerColor::TeaGreen,
			PlayerColor::ElectricOrange,
			PlayerColor::VividOrange,
			PlayerColor::Orange,
			PlayerColor::Pumpkin,
		},
		{ // General
			PlayerColor::Brown,
			PlayerColor::Peach,
			PlayerColor::Chestnut,
			PlayerColor::Beaver,
			PlayerColor::Earthtone,
			PlayerColor::CoyoteBrown,
			PlayerColor::Liver,
			PlayerColor::DarkOlive,
			PlayerColor::LightOlive,
		},
	},
	{ // Terran
		{ // Armor
			PlayerColor::Brown,
			PlayerColor::SaddleBrown,
			PlayerColor::Sienna,
			PlayerColor::Beaver,
			PlayerColor::LightOlive,
			PlayerColor::PersianOrange,
			PlayerColor::BistreBrown,
			PlayerColor::CoyoteBrown,
			PlayerColor::Earthtone,
			PlayerColor::DarkBronze,
		},
		{ // Raider
			PlayerColor::Blue,
			PlayerColor::RoyalBlue,
			PlayerColor::AliceBlue,
			PlayerColor::MayaBlue,
			PlayerColor::ElectricBlue,
			PlayerColor::NeonBlue,
			PlayerColor::Glacial,
			PlayerColor::Aero,
			PlayerColor::Yellow,
			PlayerColor::BurntUmber,
			PlayerColor::Bronze,
			PlayerColor::Redwood,
			PlayerColor::White,
			PlayerColor::Silver,
			PlayerColor::Platinum,
			PlayerColor::Snow,

//			PlayerColor::IridescentBlue,
//			PlayerColor::IridescentGreen,
//			PlayerColor::IridescentViolet,
		},
		{ // Orbital
			PlayerColor::Red,
			PlayerColor::Scarlet,
			PlayerColor::Vermillion,
			PlayerColor::VividRed,
			PlayerColor::Teal,
			PlayerColor::Cyan,
			PlayerColor::EggshellBlue,
			PlayerColor::Mint,
			PlayerColor::Emerald,
			PlayerColor::Turquoise,
			PlayerColor::Celadon,
			PlayerColor::Asparagus,
		},
		{ // Shadow
			PlayerColor::Green,
			PlayerColor::VineGreen,
			PlayerColor::FernGreen,
			PlayerColor::Harlequin,
			PlayerColor::RussianGreen,
			PlayerColor::DarkGreen,
			PlayerColor::DarkSpringGreen,
			PlayerColor::MidnightGreen,
			PlayerColor::DelftBlue,
			PlayerColor::Cordovan,
			PlayerColor::Purple,
			PlayerColor::TyrianPurple,
			PlayerColor::PalatinatePurple,
			PlayerColor::DarkLavender,
			PlayerColor::JapaneseViolet,
			PlayerColor::Black,
			PlayerColor::SmokyBlack,
			PlayerColor::DimGrey,
			PlayerColor::Grey,
			PlayerColor::Umber,
			PlayerColor::JetBlack,
		},
		{ // General
			PlayerColor::Yellow,
			PlayerColor::Brass,
			PlayerColor::Mango,
			PlayerColor::Goldenrod,
			PlayerColor::VividOrange,
			PlayerColor::Orange,
			PlayerColor::ArylideYellow,
			PlayerColor::IridescentYellow,
			PlayerColor::Tomato,
			PlayerColor::Coral,
			PlayerColor::DarkCoral,
		},
	},
	{ // Protoss
		{ // Ranger
			PlayerColor::Blue,
			PlayerColor::RoyalBlue,
			PlayerColor::Ultramarine,
			PlayerColor::CosmicCobalt,
			PlayerColor::Zaffre,
			PlayerColor::YinmnBlue,
			PlayerColor::DarkSapphire,
			PlayerColor::DelftBlue,
			PlayerColor::PalatinateBlue,
			PlayerColor::IridescentViolet,
		},
		{ // Templar
			PlayerColor::Red,
			PlayerColor::Purple,
			PlayerColor::Maroon,
			PlayerColor::Blood,
			PlayerColor::Mahogany,
			PlayerColor::Bronze,
			PlayerColor::Auburn,
			PlayerColor::TyrianPurple,
			PlayerColor::Cordovan,
			PlayerColor::Redwood,
		},
		{ // Skylord
			PlayerColor::FernGreen,
			PlayerColor::Celadon,
			PlayerColor::ElectricBlue,
			PlayerColor::EggshellBlue,
			PlayerColor::Glacial,
			PlayerColor::Aero,
			PlayerColor::Teal,
			PlayerColor::Cyan,
			PlayerColor::Turquoise,
			PlayerColor::IridescentGreen,
			PlayerColor::IridescentBlue,
		},
		{ // General
			PlayerColor::Yellow,
			PlayerColor::White,
			PlayerColor::Mango,
			PlayerColor::Brass,
			PlayerColor::Goldenrod,
			PlayerColor::Sunglow,
			PlayerColor::IridescentYellow,
			PlayerColor::Green,
			PlayerColor::MidnightGreen,
			PlayerColor::Mint,
			PlayerColor::Emerald,
			PlayerColor::DarkSpringGreen,
			PlayerColor::DarkGreen,
			PlayerColor::VineGreen,
			PlayerColor::TeaGreen,
			PlayerColor::Beige,
		},
	},
};

#define ColorSetCount 256
#define MinimapColorCount (ColorSetCount * TilesetType::Count)
#define UnitColorCount 2048

extern u8 tminimap_colors[MinimapColorCount];
extern u8 tunit_colors[UnitColorCount];
extern u8 player_color_list[PLAYABLE_PLAYER_COUNT];

void set_player_color(u32 player, u32 color);
void load_color_tables();
void load_player_colors();

// Feel free to modify this to suit your needed amount of names per personality
// (you can also specify less names than this maximum)
#define PlayerPesonalityMaxNames 8
char* player_personality_names[RaceId::Count][AiPersonality_AnyCount][PlayerPesonalityMaxNames] = {
	// Zerg
	{
		{},
		// Assault
		{
			"Hydra5620",
			"Amon's Lost Dogs",
			"AlternateCommander",
			"The Wine & Spine Company",
			"Snake-Eye Soldiers",
			"Crouching Tiger Hidden U",
			"Whyquare's Whydralisks",
			"Unused Claw Society",
		},
		// Swarm
		{
			"Pathfinding Organ",
			"BingBongBorrowers",
			"OnlyFeet Creator Union",
			"Wuckling Whirlwind",
			"UltraTHIS!",
			"Wigglehead Warriors",
			"botched_zerg_rush.png",
			"The Dank Swarm",
		},
		// Sunblot
		{
			"The Bees...",
			"FuckersDozen",
			"Three Bird",
			"void infest_everything()",
			"Flatulent Flapper",
			"Apple Juice?",
			"Flyq7",
			"The Decollider",
		},
		// Miscreant
		{
			"N's Miracle Workers",
			"Boron Enthusiasts",
			":endofallthings:",
			"Spooky Idiots",
			"Trees and Then",
			"Stenchmongers",
			"Moron Npunchers",
			"Nfinitodny",
		},
		// General
		{
			"Whathere?",
			"The Friendship Buffet",
			"BonziBossy",
			"King Kerrigan",
			"JaeboDong",
			"Discount Orlithrei",
			"Average Zerg Enjoyer",
			"A Fine Addition",
		},
	},
	// Terran
	{
		{},
		// Armor
		{
			"Todny Terranus",
			"ShiningAwakenings",
			"The 1%... plus 2",
			"Downloaded Cars",
			"Slow, Fat, Victorious",
			"StuckInMudDuringWinter.jpeg",
			"Foreign Dogwalkers",
			"Somehow, I'm Still Viable!",
		},
		// Raider
		{
			"The Ackmen",
			"The Ciox Crew",
			"Folca's Leftover APM",
			"Peter Parker?",
			"Alexei's Vodka Waiter",
			"The Chrishtapha",
			"Seven Player Army",
			"Agent TOD Marshall",
		},
		// Orbital
		{
			"IskatuMengsk",
			"Crows from Kenya",
			"WraithRush Enterprises",
			"Hotdrop Hiway",
			"America's Armada",
			"The Tower of Triple Tony",
			"Ghosts of Veeqiv",
			"DuhDuhDooooDoo...",
		},
		// Shadow
		{
			"Edgemin Division",
			"U Combinator",
			"The Man, Now, Dawg",
			"Metathrom's Cancrusher",
			"The Midwife Crisis",
			"Exotic Mixplay Purchase",
			"Vacuum Hitler Busboys",
			"The Threeborne Technique",
		},
		// General
		{
			"Deputy Dads",
			"A Solid-Snake Shit",
			"Pissed-On Earbuds",
			"Wifebeater 2.0",
			"Smash?",
			"Do the Worker",
			"The Dreamwhackers",
			"ILOVEUUV",
		},
	},
	// Protoss
	{
		{},
		// Ranger
		{
			"Champions of Iquaria",
			"Dethroned D.A.D.",
			"The Marryin' Clarions",
			"Tetrasham",
			"I R O N   G O L E M",
			"The Autobutts",
			"Prank7: Autoveeqality",
			"Premium Lawn-Mower",
		},
		// Templar
		{
			"Knights of a Realm",
			"7Gate Saviors",
			"Pr0faneNamingConvention",
			"UNRELEASED NEMESIS AI",
			"IskatuMEN!",
			"The White-Nose Winner",
			"Selfstorm Syndicate",
			"Male Gender Symbol",
		},
		// Skylord
		{
			"Solawrong!",
			"Estranged Aviary",
			"DarkDarkDark BTW",
			"Infinite Bounce Foundation",
			"With a Side of Fleet Beacon",
			"Tiny \"Techrush\" Tony",
			"Stargate SV7",
			"Furious Angles",
		},
		// Profane
		{
			"Tree Tribe",
			"The Action-Voiders",
			"Metzen's Matriarchs",
			"Voidthem Ifyoucan",
			"nullptr",
			"A Vile Voidspeaker",
			"Shadowcloak Skullcrushers",
			"Ancient Artifact",
		},
		// General
		{
			"F.R.A.U.D.s",
			"The Fraudclave",
			"Obscure Childhood Reference #3",
			"Incoherent Ape Noises",
			"Anime Campaign Protagonist",
			"NoMoreRoom_InMyBase_HELP",
			"Cannoncucker",
			"LongZhongDong",
		},
	},
};

void load_player_names();