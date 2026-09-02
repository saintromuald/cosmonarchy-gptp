#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <ostream>
#include <istream>
#include <assert.h>
#include "BW\api.h"
#include "data_ext.h"
#include "Utils.h"
#include "systems/audio_system.h"
#include "effect_field.h"

#include <variant>

#define ATTACK_LIMIT 64

struct IquareGlobals;

struct EffectFieldExtras {
    Unit* origin = nullptr;
    u32 intervals = 1;
    v2u16 move_target = {};
    u32 move_speed = 0;
};

struct LineAttackHit {
    Unit* source;
    Unit* target;
};

struct MultihitBullet {
    Bullet* bullet;
    s32 divider;
    std::vector<Unit*> already_hit;

    MultihitBullet() : bullet(0), already_hit({}), divider(0) {}
    MultihitBullet(Bullet* bullet, std::vector<Unit*> already_hit, u32 divider) : bullet(bullet), already_hit(already_hit), divider(divider) {}
};

struct IquareGlobals {
    std::string output;

    bool unitid_extender = false;

    EffectRef m_next_effect_field_ref = 0;
    std::vector<EffectField> m_effects;
    std::vector<EffectField> m_effects_to_add; // We follow a two step process to allow adding new effects, while iterating over current effects
    std::vector<LineAttackHit> m_line_attack_hits;
    std::vector<MultihitBullet> m_multihit_bullets;
    std::unordered_map<Unit*, std::vector<Unit*>> m_lakizilisk_spine_hits;

    Bullet* bounce_bullet;

    void aise_save();
    void aise_load(const char* ptr, u32 len);
    void tick();
    void unit_death(Unit* unit);
    void remove_unit_from_line_attack_table(Unit* unit, bool source_only);

    bool was_hit_by_line_attack(Unit* source, Unit* target);
    void register_line_attack_hit(Unit* source, Unit* target);

    EffectField* get_effect(EffectRef ref);
    EffectRef create_effect(EffectId id, u32 time, v2u16 position, SearchRadius radius, SpriteId::Enum sprite_id, u32 elevation, u32 player, Unit* source, const EffectFieldExtras& extras = {});
    EffectRef create_effect(EffectId id, u32 time, v2u16 position, v2u16 size, SpriteId::Enum sprite_id, u32 elevation, u32 player, Unit* source, const EffectFieldExtras& extras = {});
    EffectRef create_effect(EffectId id, u32 time, v2u16 position, u32 radius, SpriteId::Enum sprite_id, u32 elevation, u32 player, Unit* source, const EffectFieldExtras& extras = {});
    bool was_already_hit_by_multihit_bullet(Bullet* bullet, Unit* target);
    bool multihit_bullet_is_set(Bullet* bullet);
    u32 get_multihit_count(Bullet* bullet);
    void add_multihit_empty_bullet(Bullet* bullet, s32 divider);
    void add_bullet_multihit(Bullet* bullet, Unit* target);
    void add_incremental_bullet_multihit(Bullet* bullet, Unit* target);
    void remove_multihit_bullet(Bullet* bullet);
    s32 get_divider(Bullet* bullet);
    void clear_lakizilisk(Unit* lakizilisk);
    void add_lakizilisk_spine_pair(Unit* lakizilisk, Unit* target);
    bool spine_pair_exists(Unit* lakizilisk, Unit* target);
};

void apply_irradiate(Unit* unit, u8 player, u32 time);

void inject_globals_hooks();

inline std::unique_ptr<IquareGlobals> g_iquare_globals;

namespace hooks {

struct WeaponDamageData;

}

namespace bw {

bool is_slowed(Unit* unit);
void refresh_debuffs(Unit* unit);
void inherit_debuffs(Unit* parent, Unit* child);
bool is_debuffed(Unit* unit);
void restore_func(Unit* unit);
void do_deconstruction_damage(Unit* unit, WeaponId::Enum weapon_id, bool play_sound = true);

void clear_clarion_links(Unit* unit);
void place_clarion_links(Unit* unit);
bool has_clarion_links(Unit* unit);
std::vector<Unit*> get_clarion_links(Unit* unit);

// Returns damage left after spread
s32 spread_clarion_damage(const hooks::WeaponDamageData& data, fp32 factor);
}