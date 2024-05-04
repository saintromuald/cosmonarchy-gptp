#pragma once
#include "BW\api.h"

// TODO: rename this file to something more accurate, we have all kinds of player customization here, not only color

enum {
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

inline const std::vector<std::vector<PlayerColor>> player_tints = {
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

inline const std::vector<std::vector<u32>> banned_tileset_tints = {
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
		PlayerTintLightBlue,
	},
	{ // Twilight
		PlayerTintCycleDependent,
		PlayerTintDarkAchromatic,
	},
};

inline const std::vector<std::vector<u32>> banned_race_tints = {
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

inline const std::vector<PlayerColor> banned_colors = {
	PlayerColor::Harlequin,
	PlayerColor::Rainbow,
	PlayerColor::Magenta,
	PlayerColor::IridescentYellow,
	PlayerColor::IridescentGreen,
	PlayerColor::IridescentBlue,
	PlayerColor::IridescentViolet,
	PlayerColor::RainbowNew,
	PlayerColor::PatrioticFreedom,
	PlayerColor::MetzenicProzac,
};

inline const std::vector<std::vector<std::vector<PlayerColor>>> personality_colors = {
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

// Feel free to modify this to suit your needed amount of names per personality
// (you can also specify less names than this maximum)
#define PlayerPesonalityMaxNames 20
inline const char* player_personality_names[RaceId::Count][AiPersonality_AnyCount][PlayerPesonalityMaxNames] = {
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
			"Boron Enthusiasts",
			"Moron Npunchers",
		},
		// Swarm
		{
			"Pathfinding Organ",
			"BingBongBurrowers",
			"OnlyFeet Creator Union",
			"Wuckling Whirlwind",
			"UltraTHIS!",
			"Wigglehead Warriors",
			"botched_zerg_rush.png",
			"The Dank Swarm",
			"N's Miracle Workers",
			"Nfinitodny",
			"The Pocket Tech Node",
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
			":endofallthings:",
			"Cosmonarchy Nathwar",
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
			"Spooky Idiots",
			"Stenchmongers",
			"Trees and Then",
			"Competitive Cagrants",
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
			"Somehow Still Viable",
			"Metathrom's Cancrusher",
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
			"Edgemin Division",
			"The Man, Now, Dawg",
			"Vacuum Hitler Busboys",
			"Benno's Brawlers",
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
			"Exotic Mixplay Purchase",
			"Wraithcifirius",
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
			"U Combinator",
			"The Midwife Crisis",
			"The Threeborne Technique",
			"Lardcode Licker",
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
			"Voidthem Ifyoucan",
			"Ancient Artifact",
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
			"The Action-Voiders",
			"Shadowcloak Skullcrushers",
			"Impending Scribe-Pull",
		},
		// Skylord
		{
			"Solawrong!",
			"Estranged Aviary",
			"DarkDarkDark BTW",
			"Infinibounce Incorporated",
			"With a Side of Fleet Beacon",
			"Tiny \"Techrush\" Tony",
			"Stargate SV7",
			"Furious Angles",
			"Tree Tribe",
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
			"Metzen's Matriarchs",
			"A Vile Voidspeaker",
			"nullptr",
			"Little Timmy's Nexus-First",
		},
	},
};

struct PlayerData {
	bool has_custom_color[PlayerLimit];
	bool has_custom_player_name[PlayerLimit];
	bool color_used[(size_t)PlayerColor::Count];
	PlayerColor player_color_list[PlayerLimit];

	std::string full_player_names[PlayerLimit];
	std::string full_force_names[PlayerLimit];
};

#define ColorSetSize 8

inline PlayerData g_player_data;

const char* get_random_easter_egg_unit_name(u16 unit_id);

u8 get_player_color_index(u32 player, u32 index);
bool set_player_color(u32 player, PlayerColor color);
void init_player_colors();
void load_player_colors();
bool any_player_can_randomize_color();
bool is_color_banned(PlayerColor color);

void load_player_names();
void set_player_name(u32 player, const char* name, bool prevent_overwrites);
void set_player_force_name(u32 player, const char* name);