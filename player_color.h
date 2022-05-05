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
			"Assault THREE!",
			"Assault Four",
			"Assault Five",
			"Assault Six",
			"Assault SEVEN!",
			"Assault Eight",
		},
		// Swarm
		{
			"Pathfinding Organ",
			"BingBongBorrowers",
			"Swarm THREE!",
			"Swarm Four",
			"Swarm Five",
			"Swarm Six",
			"Swarm SEVEN!",
			"Swarm Eight",
		},
		// Sunblot
		{
			"Sunblot One",
			"Sunblot Two",
			"Sunblot THREE!",
			"Sunblot Four",
			"Sunblot Five",
			"Sunblot Six",
			"Sunblot SEVEN!",
			"Sunblot Eight",
		},
		// Miscreant
		{
			"N's Miracle Workers",
			"Boron Enthusiasts",
			":endofallthings:",
			"Miscreant Four",
			"Miscreant Five",
			"Miscreant Six",
			"Miscreant SEVEN!",
			"Miscreant Eight",
		},
		// General
		{
			"Whathere?",
			"The Friendship Buffet",
			"BonziBossy",
			"General Four",
			"General Five",
			"General Six",
			"General SEVEN!",
			"General Eight",
		},
	},
	// Terran
	{
		{},
		// Armor
		{
			"Todny Terranus",
			"Foreign Dogwalkers",
			"ShiningAwakenings",
			"Armor Four",
			"Armor Five",
			"Armor Six",
			"Armor SEVEN!",
			"Armor Eight",
		},
		// Raider
		{
			"The Ackmen",
			"The Ciox Crew",
			"Folca's Leftover APM",
			"Raider Four",
			"Raider Five",
			"Raider Six",
			"Raider SEVEN!",
			"Raider Eight",
		},
		// Orbital
		{
			"IskatuMengsk",
			"Crows from Kenya",
			"Orbital THREE!",
			"Orbital Four",
			"Orbital Five",
			"Orbital Six",
			"Orbital SEVEN!",
			"Orbital Eight",
		},
		// Shadow
		{
			"Vacuum Hitler Busboys",
			"Shadow Two",
			"Shadow THREE!",
			"Shadow Four",
			"Shadow Five",
			"Shadow Six",
			"Shadow SEVEN!",
			"Shadow Eight",
		},
		// General
		{
			"Deputy Dads",
			"The Dreamwhackers",
			"A Solid-Snake Shit",
			"General Four",
			"General Five",
			"General Six",
			"General SEVEN!",
			"General Eight",
		},
	},
	// Protoss
	{
		{},
		// Ranger
		{
			"Champions of Iquaria",
			"Prank7: Autoveeqality",
			"Dethroned D",
			"Ranger Four",
			"Ranger Five",
			"Ranger Six",
			"Ranger SEVEN!",
			"Ranger Eight",
		},
		// Templar
		{
			"Knights of a Realm",
			"Templar Two",
			"Templar THREE!",
			"Templar Four",
			"Templar Five",
			"Templar Six",
			"Templar SEVEN!",
			"Templar Eight",
		},
		// Skylord
		{
			"Solawrong!",
			"Estranged Aviary",
			"Skylord THREE!",
			"Skylord Four",
			"Skylord Five",
			"Skylord Six",
			"Skylord SEVEN!",
			"Skylord Eight",
		},
		// Profane
		{
			"Tree Tribe",
			"Shadowcloak Skullcrusher",
			"The Action-Voiders",
			"Metzen's Matriarchs",
			"Voidthem Ifyoucan",
			"Profane Six",
			"Profane SEVEN!",
			"Profane Eight",
		},
		// General
		{
			"F.R.A.U.D.s",
			"The Fraudclave",
			"General THREE!",
			"General Four",
			"General Five",
			"General Six",
			"General SEVEN!",
			"General Eight",
		},
	},
};

void load_player_names();