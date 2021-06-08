#pragma once
#include "SCBW\api.h"

enum
{
	PlayerTintInvalid = -1,

	// Can be rolled
	PlayerTintDarkRed,
	PlayerTintLightRed,
	PlayerTintOffRed,
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
	_ = PlayerTintRollCount - 1,

	// Can't be rolled
	PlayerTintCycleDependent, // technically still a tint, but can't be rolled. don't add the assigment, if you decide to add more

	PlayerTintCount,
};

const std::vector<std::vector<u32>> player_tints =
{
	{	// Dark Red
		PlayerColor::Maroon,
		PlayerColor::Blood,
		PlayerColor::ElectricOrange,
		PlayerColor::Mahogany,
		PlayerColor::DarkCoral,
	},
	{	// Light Red
		PlayerColor::Red,
		PlayerColor::Vermillion,
		PlayerColor::Coral,
		PlayerColor::Salmon,
		PlayerColor::VividRed,
		PlayerColor::Scarlet,
		PlayerColor::PersianOrange,
		PlayerColor::Tomato,
	},
	{	// Off Red
		PlayerColor::Wine,
		PlayerColor::Auburn,
		PlayerColor::Cordovan,
		PlayerColor::Redwood,
	},
	{	// Brown
		PlayerColor::Brown,
		PlayerColor::Bronze,
		PlayerColor::SaddleBrown,
		PlayerColor::Sienna,
		PlayerColor::Chestnut,
		PlayerColor::DarkBronze,
		PlayerColor::BurntUmber,
		PlayerColor::Liver,
		PlayerColor::Earthtone,
		PlayerColor::GoldenBrown,
		PlayerColor::CoyoteBrown,
	},
	{	// Orange
		PlayerColor::Orange,
		PlayerColor::Pumpkin,
		PlayerColor::Magma,
		PlayerColor::VividOrange,
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
		PlayerColor::IridescentYellow,
		PlayerColor::BistreBrown,
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
		PlayerColor::Mint,
		PlayerColor::Emerald,
		PlayerColor::DarkSpringGreen,
	},
	{	// Dark Blue
		PlayerColor::Blue,
		PlayerColor::RoyalBlue,
		PlayerColor::Ultramarine,
		PlayerColor::CosmicCobalt,
		PlayerColor::Zaffre,
		PlayerColor::YinmnBlue,
		PlayerColor::DarkSapphire,
		PlayerColor::BdazzledBlue,
	},
	{	// Light Blue
		PlayerColor::Periwinkle,
		PlayerColor::AliceBlue,
		PlayerColor::MayaBlue,
		PlayerColor::ElectricBlue,
		PlayerColor::Glacial,
		PlayerColor::Aero,
		PlayerColor::CornflowerBlue,
		PlayerColor::NeonBlue,
		PlayerColor::IridescentBlue,
		PlayerColor::PalatinateBlue,
	},
	{	// Cyan
		PlayerColor::Teal,
		PlayerColor::Cyan,
		PlayerColor::EggshellBlue,
		PlayerColor::Turquoise,
		PlayerColor::IridescentGreen,
		PlayerColor::MidnightGreen,
	},
	{	// Violet
		PlayerColor::Purple,
		PlayerColor::TyrianPurple,
		PlayerColor::PalatinatePurple,
		PlayerColor::IridescentViolet,
		PlayerColor::JapaneseViolet,
		PlayerColor::DarkLavender,
		PlayerColor::Magenta,
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
		PlayerColor::Magenta,
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
	},
	{ // Twilight
		PlayerTintCycleDependent,
		PlayerTintDarkAchromatic,
	},
};

const std::vector<std::vector<u32>> banned_race_tints =
{
	{ // Zerg
		
	},
	{ // Terran

	},
	{ // Protoss

	},
};

std::vector<std::vector<std::vector<u32>>> personality_colors =
{
	{ // Zerg
		{ // Assault

		},
		{ // Swarm

		},
		{ // Air

		},
		{ // General

		},
	},
	{ // Terran
		{ // Mech

		},
		{ // Raider

		},
		{ // Air

		},
		{ // Shadow

		},
		{ // General

		},
	},
	{ // Protoss
		{ // Ranger

		},
		{ // Templar

		},
		{ // Skylord

		},
		{ // General

		},
	},
};

extern u8 tminimap_colors[256];
extern u8 tunit_colors[2048];

extern bool player_color_list_inited;
extern u8 player_color_list[8];

void set_melee_player_colors();
void init_player_color_list();