#pragma once
#include "SCBW/api.h"
#include "trigger_parser.h"

struct Sound;

#define SoundReplacementCap 1600
struct SoundReplacement {
	PlayerBitset players;
	UnitIdBitset unit_ids;
	bool any_animation;
	bool any_sound;
	bool replace;
	u32 animation;
	u32 old_sound_id;
	Sound* new_sound;
};

#define SpriteReplacementCap 800
struct SpriteReplacement {
	PlayerBitset players;
	UnitIdBitset unit_ids;
	u32 sprite_id;
};

static const char* race_names[] = {
	"Zerg",
	"Protoss",
	"Terran",
};

// TODO: Move to Globals (or new global struct if need be)
extern bool trigger_game_paused;
extern bool trigger_system_init;
extern bool trigger_system_in_game;

extern u32 sound_replacements_count;
extern SoundReplacement sound_replacements[SoundReplacementCap];

extern u32 sprite_replacements_count;
extern SpriteReplacement sprite_replacements[SpriteReplacementCap];

extern Token custom_unit_names[PlayerLimit][UnitId::Count];
// END TODO

enum WitnessMode: u8 {
	WitnessMode_Disabled = 0,   // Normal Player
	WitnessMode_Pure,			// Pure Witness,    Witness Mode && hidden on scoreboard
	WitnessMode_Defeated,		// Defeated Player, Witness Mode && shown on scoreboard
};

enum Celebration: u8 {
	Celebration_None,
	Celebration_AprilFools,
	Celebration_Christmas,

	Celebration_Count,
};

enum Faction: u8 {
	Faction_None,

	Faction_TerranOrionImperium,
	Faction_TerranHecatonCoalition,
	Faction_TerranFuturistFederation,
	Faction_TerranGoldenRepublic,
	Faction_TerranNydusTrinity,
	Faction_TerranUpstart,

	Faction_ProtossDivinePrimacy,
	Faction_ProtossOrderOfTheSacredTemplar,
	Faction_ProtossKhalendricKnights,
	Faction_ProtossStarbound,

	Faction_ZergAtaltrethTendril,
	Faction_ZergImicrenTendril,
	Faction_ZergOrkotiTendril,
	Faction_ZergGoszirithTendril,
	Faction_ZergZorthosTendril,

	Faction_Count,
};

static const char* faction_symbol_names[] = { // This array needs to the same size and order as Faction enum
	"none",

	"orion_imperium",
	"hecaton_coalition",
	"futurist_federation",
	"golden_republic",
	"nydus_trinity",
	"upstart_terran",

	"divine_primacy",
	"order_of_the_sacred_templar",
	"khalendric_knights",
	"starbound",

	"ataltreth",
	"imicren",
	"goszirith",
	"orkoti",
	"zorthos",
};
C_ASSERT(ARRAY_SIZE(faction_symbol_names) == Faction_Count);

enum AdvisorType : u8 {
	AdvisorType_Default,

	AdvisorType_Terran,
	AdvisorType_Protoss,
	AdvisorType_Zerg,

	AdvisorType_Mesk,
	AdvisorType_Ackmed,

	AdvisorType_Count,
};

// NOTE: This is case insensitive for parsing
static const char* advisor_symbol_names[] = {
	"Default",

	"Terran",
	"Protoss",
	"Zerg",

	"Mesk",
	"Ackmed",
};
C_ASSERT(ARRAY_SIZE(advisor_symbol_names) == AdvisorType_Count);

struct PlayerVars {
	Point16 camera;
	WitnessMode witness_mode;
	Faction faction;
	bool neutral_hostile;
	bool faction_was_set;
};

struct AlertState {
	u16 sound_id;
	u8 play_timer;
	u8 own_cooldown;
	u8 ally_cooldown;
};

struct AdvisorState {
	u16 sound_id;
	u8 play_timer;
	u8 cooldown;
};

enum YieldType {
	YieldType_MineralDepleted,
	YieldType_Mineral,
	YieldType_GasRidge,
	YieldType_GasGeyser,
	YieldType_GasTier2,
	YieldType_GasTier3,

	YieldType_Count,
};

static constexpr u8 default_yields[YieldType_Count] = { 2, 4, 2, 4, 5, 6 };

struct Globals {
	PlayerVars players[PlayerLimit];
	u32 celebration;

	AdvisorType current_advisor;

	AlertState alert;
	AdvisorState advisor_state;

	u8 yields[YieldType_Count];

	u8 get_yield(YieldType type) {
		return yields[type] ? yields[type] : default_yields[type];
	}
};

enum TriggerStat : u8 {
	TriggerStat_Armor,
	TriggerStat_WeaponRange,
	TriggerStat_ArmorPenetration,
	TriggerStat_Damage,
	TriggerStat_SplashInner,
	TriggerStat_SplashMedium,
	TriggerStat_SplashOuter,
	TriggerStat_TimedLife,

	TriggerStat_Count,
};


static const char* trigger_stat_names[] = {
	"armor",
	"weapon_range",
	"armor_pen",
	"damage",
	"splash_inner",
	"splash_medium",
	"splash_outer",
	"timed_life",
};
C_ASSERT(ARRAY_SIZE(trigger_stat_names) == TriggerStat_Count);

enum TriggerBehavior : u8 {
	TriggerBehavior_CloakProximity,
	TriggerBehavior_CloakSteady,
	TriggerBehavior_Hallucination,

	TriggerBehavior_Count,
};


static const char* trigger_behavior_names[] = {
	"cloak_proximity",
	"cloak_steady",
	"hallucination",
};
C_ASSERT(ARRAY_SIZE(trigger_behavior_names) == TriggerBehavior_Count);

extern Globals globals;

void init_trigger_system();
void update_trigger_system(bool in_game);

b32 parse_comment_condition(TriggerAction* action);
b32 __fastcall parse_comment_action(TriggerAction* action);
