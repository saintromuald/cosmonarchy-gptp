#pragma once
#include "BW/api.h"
#include "trigger_parser.h"

struct SoundMainBuffer;

#define SoundReplacementCap 1600
struct SoundReplacement {
    PlayerBitset players;
    UnitIdBitset unit_ids;
    bool any_animation;
    bool any_sound;
    bool replace;
    u32 animation;
    u32 old_sound_id;
    SoundMainBuffer* new_sound;

    inline bool operator==(SoundReplacement& other) { return memcmp(this, &other, sizeof(*this)) == 0; }
    inline bool operator!=(SoundReplacement& other) { return !(*this == other); }
};

#define SpriteReplacementCap 800
struct SpriteReplacement {
    PlayerBitset players;
    UnitIdBitset unit_ids;
    u32 sprite_id;
};

static const char* race_names[] = {
    "Zerg",
    "Terran",
    "Protoss",
};
static_assert(std::size(race_names) == RaceId::Count);

enum class CrigPlayerType {
    Human,
    HumanDefeated,
    Computer,
    ComputerDefeated,
    Witness,
    WitnessDefeated,

    None,
    Count,
};

static const char* player_type_names[] = {
    "human",
    "human_defeated",
    "computer",
    "computer_defeated",
    "witness",
    "witness_defeated",

    "none",
};
static_assert(std::size(player_type_names) == (u8)CrigPlayerType::Count);

struct TriggerSystemState {
    bool game_paused;
    bool init;
    bool in_game;
};

inline TriggerSystemState g_trigger;

enum WitnessMode : u8 {
    WitnessMode_Disabled = 0,   // Normal Player	|	No witness mode && shown on scorescreen
    WitnessMode_Pure,			// Pure Witness		|	Witness Mode && hidden on scorescreen
    WitnessMode_Defeated,		// Defeated Player	|	Witness Mode && shown on scorescreen
};

enum Celebration : u8 {
    Celebration_None,
    Celebration_AprilFools,
    Celebration_Christmas,

    Celebration_Count,
};

enum GameModeRace : u8 {
    GameModeRace_Default,
    GameModeRace_None,

    GameModeRace_All,
    GameModeRace_TerranProtoss,
    GameModeRace_ProtossZerg,
    GameModeRace_ZergTerran,

    GameModeRace_Count,
};

// NOTE: This is case insensitive for parsing
static const char* game_mode_race_symbol_names[] = {
    "Default",
    "None",

    "All",
    "TerranProtoss",
    "ProtossZerg",
    "ZergTerran",
};
C_ASSERT(ARRAY_SIZE(game_mode_race_symbol_names) == GameModeRace_Count);

enum class Advisor : u8 {
    Default,

    // Races
    Terran,
    Protoss,
    Zerg,
    Askosi,
    Bloodsummon,

    // Community
    Goku,

    // Characters
    Ackmed,
    HADRAL,
    Furen,
    Kortar,

    // Faction-specific
    Artemis,
    GoldenRepublic,

    Count,
};

// NOTE: This is case insensitive for parsing
static const char* advisor_names[]{
    "Default",

    // Races
    "Terran",
    "Protoss",
    "Zerg",
    "Askosi",
    "Bloodsummon",

    // Community
    "Goku",

    // Characters
    "Ackmed",
    "HADRAL",
    "Furen",
    "Kortar",

    // Faction-specific
    "Artemis",
    "Golden_Republic",
};
static_assert(std::size(advisor_names) == (size_t)Advisor::Count);

struct CmPlayer {
    Point16 camera;
    WitnessMode witness_mode;
    Faction faction;
    bool no_starting_units;
    bool neutral_hostile;
    bool faction_was_set;
    bool hide_in_team_ui;
    bool disable_liftoff;
    bool disable_burrow;
    UnitIdBitset disabled_units;
    GameModeRace game_mode_race;
    u32 build_multiplier = 1;

    std::vector<PlayerColor> preferred_colors;
    std::unordered_map<UnitId::Enum, std::string> custom_unit_names;
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

enum class YieldType : u8 {
    MineralDepleted,
    Mineral,
    GasRidge,
    GasGeyser,
    GasGeyserTier2,
    GasGeyserTier3,

    Count,
};

static constexpr u8 default_yields[(size_t)YieldType::Count] = { 5, 5, 3, 5, 7, 9 };

struct OngoingWait {
    s32 timer;
    u32 parser_position;
};

struct TriggerExt {
    OngoingWait ongoing_waits[PlayerLimit];
};

struct TriggerButton {
    Token id;
    Token display_name;
    s32 priority;
};

struct DesyncState {
    bool type_1;
    bool type_2;
    bool type_3;
    bool type_4;
    bool warning_player_flag;
    bool warning_hash_struct;
};

using EcoMeasurements = std::array<f32, PlayerLimit>;

struct Globals {
    bool preferred_colors_set;
    CmPlayer players[PlayerLimit];
    u32 celebration;

    AlertState alert;
    Advisor current_advisor;
    AdvisorState advisor_state;

    u8 yields[(size_t)YieldType::Count];
    u8 get_yield(YieldType type) {
        return yields[(size_t)type] ? yields[(size_t)type] : default_yields[(size_t)type];
    }

    DesyncState desync_state;
    bool had_desync = false;
    bool forbid_replay_auto_upload = false;
    std::unordered_map<TriggerAction*, TriggerExt> trigger_exts;
    std::vector<SoundReplacement> sound_replacements;
    std::vector<SpriteReplacement> sprite_replacements;
    std::vector<TriggerButton> trigger_buttons;
    std::vector<EcoMeasurements> eco_measurements;
    std::vector<char> uuid_rand;

	void sample_eco();
	f32 calculate_hype() const;
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
    TriggerStat_Wireframe,
    TriggerStat_Buttonset,
    TriggerStat_HealRange,

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
    "wireframe",
    "buttonset",
    "heal_range",
};
C_ASSERT(ARRAY_SIZE(trigger_stat_names) == TriggerStat_Count);

enum TriggerBehavior : u8 {
    TriggerBehavior_CloakProximity,
    TriggerBehavior_CloakSteady,
    TriggerBehavior_Hallucination,
    TriggerBehavior_DisablePassive,
    TriggerBehavior_Critter,
    TriggerBehavior_NoCollision,
    TriggerBehavior_AirTransport,
    TriggerBehavior_NoEnergyOverlay,

    TriggerBehavior_Count,
};

static const char* trigger_behavior_names[] = {
    "cloak_proximity",
    "cloak_steady",
    "hallucination",
    "disable_passive",
    "critter",
    "no_collision",
    "air_transport",
    "no_energy_overlay",
};
C_ASSERT(ARRAY_SIZE(trigger_behavior_names) == TriggerBehavior_Count);

inline auto g_globals = std::make_unique<Globals>();

Advisor get_advisor_type_from_config(u32 race);
void init_trigger_system();
void update_trigger_system(bool in_game);

bool buttons_hovers();
bool buttons_handle_input(const struct KeyEvent& key_event);
void buttons_draw();

b32 parse_comment_condition(TriggerAction* action);
b32 __fastcall parse_comment_action(TriggerAction* action);
