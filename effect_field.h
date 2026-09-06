#pragma once
#include "types.h"
#include "BW/structures/common.h"
#include "BW/enumerations.h"
#include "systems/audio/audio_system.h"

#include <variant>

struct Unit;
struct Thingy;

using EffectRef = u64;

using SearchRadius = std::variant<u32, v2u16>;

enum class EffectId {
    MorningstarGroundZero,
    DiademIonField,
    SalamanderFire,
    Irradiate,
    AspirantCriticality,
    PsiStorm,
    PsiStormPersistent,
    HallucinatingField,
    LanifectDisruptionField,
    StarSovereignTheGriefOfAllGods,
    KeskathalorCausticDischarge,
    IsthrathalethPlagueField,
    ReclamationField,
    VilgolethHeal,
    CyclopsRecursionRound,
    CyclopsFuturistRecursionRound,
    SevenstepMindMatter,
    OscilliantBindingPlasma,
    GladiusBindingPlasma,
    GladiusOrderBindingPlasma,
    MiasmaPlagueField,
    DivineDisciplineField,
    ScrapworldImplosion,
    AtreusBindingPlasma,

    Count,
};

struct EffectField {
    EffectRef ref;
    EffectId id;
    v2u16 position;
    SearchRadius radius;
    u32 current_time;
    u32 max_time;
    SpriteId::Enum sprite_id;
    u32 elevation;
    u32 player;
    Thingy* thingy;
    Unit* source;

    // Extras
    Unit* origin;
    u32 intervals;
    v2u16 move_target;
    u32 move_speed;

    // Runtime Data
    s32 tick = -1;
    PlayingSound playing_sound;

    void destroy();
};
