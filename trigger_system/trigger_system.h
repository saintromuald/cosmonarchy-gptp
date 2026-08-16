#pragma once
#include "BW/api.h"
#include "trigger_parser.h"

struct SoundMainBuffer;

#define SoundReplacementCap 1600
struct SoundReplacement {
    trigger::PlayerBitset players;
    trigger::UnitIdBitset unit_ids;
    bool any_animation;
    bool any_sound;
    bool replace;
    u32 animation;
    u32 old_sound_id;
    SoundMainBuffer* new_sound;

    bool operator==(const SoundReplacement& other) const { return memcmp(this, &other, sizeof(*this)) == 0; }
    bool operator!=(const SoundReplacement& other) const { return !(*this == other); }
};

#define SpriteReplacementCap 800
struct SpriteReplacement {
    trigger::PlayerBitset players;
    trigger::UnitIdBitset unit_ids;
    u32 sprite_id;
};


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

enum class GameModeRace : u8 {
    Default,
    None,

    All,
    TerranProtoss,
    ProtossZerg,
    ZergTerran,

    Count,
};

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

    None = Default,
    Count = GoldenRepublic + 1,
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
    v2u16 camera;
    WitnessMode witness_mode;
    Faction faction;
    bool no_starting_units;
    bool neutral_hostile;
    bool faction_was_set;
    bool hide_in_team_ui;
    bool disable_liftoff;
    bool disable_burrow;
    bool override_infrastructure_burrow;
    bool disable_tactical_ai;
    bool disable_extended_waves;
    trigger::UnitIdBitset disabled_units;
    trigger::UnitIdBitset hidden_units;
    trigger::PlayerBitset ally_alert_blacklist;
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
    std::string id;
    std::string display_name;
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
    bool disable_transmission_waits = false;
    std::unordered_map<TriggerAction*, TriggerExt> trigger_exts;
    std::vector<SoundReplacement> sound_replacements;
    std::vector<SpriteReplacement> sprite_replacements;
    std::vector<TriggerButton> trigger_buttons;
    std::vector<EcoMeasurements> eco_measurements;
    std::vector<char> uuid_rand;

    void sample_eco();
    f32 calculate_hype() const;
};

enum class TriggerStat : u8 {
    Armor,
    WeaponRange,
    ArmorPenetration,
    Damage,
    SplashInner,
    SplashMedium,
    SplashOuter,
    TimedLife,
    Wireframe,
    Buttonset,
    HealRange,
    Portrait,

    Count,
    None,
};

enum class TriggerBehavior : u8 {
    CloakProximity,
    CloakSteady,
    Hallucination,
    DisablePassive,
    Critter,
    NoCollision,
    AirTransport,
    NoEnergyOverlay,
    InfrastructureBurrow,

    Count,
    None,
};

inline auto g_globals = std::make_unique<Globals>();

Advisor get_advisor_type_from_config(u32 race);
void init_auto_celebration();
void init_trigger_system();
void update_trigger_system(bool in_game);

bool buttons_hovers();
bool buttons_handle_input(const struct KeyEvent& key_event);
void buttons_draw();

b32 parse_comment_condition(TriggerAction* action);
b32 __fastcall parse_comment_action(TriggerAction* action);
