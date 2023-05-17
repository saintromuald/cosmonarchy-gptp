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

extern bool trigger_game_paused;
extern bool trigger_system_init;
extern bool trigger_system_in_game;

extern u32 sound_replacements_count;
extern SoundReplacement sound_replacements[SoundReplacementCap];

extern u32 sprite_replacements_count;
extern SpriteReplacement sprite_replacements[SpriteReplacementCap];

extern Token custom_unit_names[PlayerLimit][UnitId::Count];

enum WitnessMode: u8 {
	WitnessMode_Disabled = 0,   // Normal Player
	WitnessMode_Pure,			// Pure Witness,    Witness Mode && hidden on scoreboard
	WitnessMode_Defeated,		// Defeated Player, Witness Mode && shown on scoreboard
};

enum Celebration: u8 {
	Celebration_None,
	Celebration_Christmas,

	Celebration_Count,
};

enum Faction: u8 {
	Faction_None,

	Faction_TerranHecatonCoalition,
	Faction_TerranOrionImperium,
	Faction_TerranFuturistFederation,
	Faction_TerranGoldenRepublic,
	Faction_TerranNydusTrinity,
	Faction_TerranUpstart,

	Faction_ProtossDivinePrimacy,
	Faction_ProtossOrderOfTheSacredTemplar,
	Faction_ProtossKhalendricKnights,
	Faction_ProtossStarbound,

	Faction_ZergImicrenTendril,
	Faction_ZergOrkotiTendril,
	Faction_ZergGoszirithTendril,
	Faction_ZergAtaltrethTendril,
	Faction_ZergZorthosTendril,

	Faction_Count,
};

static const char* faction_symbol_names[] = { // This array needs to the same size and order as Faction enum
	"none",

	"hecaton_coalition",
	"orion_imperium",
	"futurist_federation",
	"golden_republic",
	"nydus_trinity",
	"upstart_terran",

	"divine_primacy",
	"order_of_the_sacred_templar",
	"khalendric_knights",
	"starbound",

	"imicren",
	"orkoti",
	"goszirith",
	"ataltreth",
	"zorthos",
};
C_ASSERT(ARRAY_SIZE(faction_symbol_names) == Faction_Count);

struct PlayerVars {
	Point16 camera;
	WitnessMode witness_mode;
	Faction faction;
};

struct AlertState {
	u16 sound_id;
	u8 play_timer;
	u8 own_cooldown;
	u8 ally_cooldown;
};

struct Globals {
	PlayerVars players[PlayerLimit];
	u32 celebration;

	AlertState alert;
};

extern Globals globals;

void init_trigger_system();
void update_trigger_system(bool in_game);

b32 parse_comment_condition(TriggerAction* action);
b32 __fastcall parse_comment_action(TriggerAction* action);
