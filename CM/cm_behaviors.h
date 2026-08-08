#pragma once
#include "BW\bwdata.h"
#include <variant>

using SearchRadius = std::variant<u32, v2u16>;
struct EffectField;

enum class AttackTargetingType {
    RequiresTarget,					// If target doesn't exist, attack will stop firing (default)
    FireGround,		                // Regardless of target, fires at ground
    RemembersTarget,				// If target doesn't exist, fallback on the position of last target
    AcquiresNewTargets,				// If target doesn't exist, try to find a new target, otherwise will not fire
    AcquiresNewTargetsAndRemembers, // If target doesn't exist, try to find a new target, otherwise fallback on the position of last target

    Diadem,							// Hardcoded Diadem handling
};

bool is_any_corpse(Thingy* thingy);
bool is_inactive_corpse(Thingy* thingy);
bool is_active_corpse(Thingy* thingy);
void consume_corpse(Thingy* corpse);
Thingy* get_closest_active_corpse(v2u16 position, u32 radius);
Thingy* get_closest_active_corpse(v2u16 position, SearchRadius radius);
Unit* spawn_reclaimed_unit(u32 player, v2u16 pos, UnitId::Enum unit_id);
Unit* reclaim_corpse(Thingy* corpse, u32 player);

void avenge_corpse(Thingy* corpse, u32 player);
void ascend_corpse(Thingy* corpse, u32 player);
void ascend_in_area(u32 player, v2u16 position, u32 range);
void avenge_or_ascend_corpse(Thingy* corpse, u32 player);
void avenge_corpses_in_area(v2u16 position, u32 radius, u32 player);
void ascend_corpses_in_area(v2u16 position, SearchRadius radius, u32 player);
void avenge_or_ascend_corpses_in_area(v2u16 position, u32 radius, u32 player);
void avenge_or_ascend_corpses_in_area(v2u16 position, SearchRadius radius, u32 player);
u32 unit_avenges_or_ascends(Unit* unit); // Returns (source_player + 1) if should avenge/ascend, returns 0 otherwise 
void salvage_corpse(Thingy* corpse, u32 player);

void incubate(Unit* target, u32 count, u32 unit_id, u32 player);
void set_custom_egg(Unit* egg, u32 unit_id, u32 time, u32 hp = 150 * 256);
void fire_secondary_melee_weapon_if_needed(Unit* unit, u32 weapon_id);
bool can_infest(u32 player, Unit* target, bool force = false);
void mark_infesting(u32 player, Unit* target);
void blissful_embrace_tick(Unit* target);
void blissful_embrace(Unit* target, u32 player, bool force = false);

void empress_zombify(u32 player, Unit* source, Unit* target, u32 stacks);
void mind_control(u32 player, u32 mind_controller_unit_id, Unit* target, bool ignore_alliance = false);
void recede_mind_control(Unit* target);
void spread_debuffs(Unit* from, Unit* to);
void clear_maledictus(Unit* tyrant, Unit* malefactor);
void clear_maledictus_if_needed(Unit* unit);

AttackTargetingType attack_targeting_type_of(Unit* unit);
bool can_pick_new_target(Unit* unit);
void tick_psi_storm_damage(EffectField& effect, u32 update_period, u32 tick_scalar_hp);
void apply_lobotomy_effect(v2u16 position, u32 timer, bool do_stun);
void apply_tartarus_effect(v2u16 position, u32 timer, bool do_damage, u8 attacking_player);
void apply_goremast_effect(v2u16 position, u32 timer, bool do_damage);
