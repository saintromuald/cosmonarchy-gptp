#pragma once
#include "BW\bwdata.h"
#include "hooks\weapons\weapon_fire.h"

bool unit_id_is_geyser_refinery(u16 unit_id);
bool unit_id_is_capped_gas_node(u16 unit_id);
bool unit_id_is_gas_node(u16 unit_id);
bool unit_id_is_uncapped_gas_node(u16 unit_id);
bool unit_id_is_smaller_building(u16 unit_id);
b32 unit_id_is_tech(u32 unit_id, bool allow_addons = true);
b32 unit_id_is_tech_tier_structure(u32 unit_id);
bool unit_id_is_transport(u32 unit_id);
bool unit_id_is_race(u32 unit_id, u32 race);
bool unit_id_is_combatant(u32 unit_id);
bool unit_id_is_protoss_warp_factory(u32 unit_id);
bool unit_id_has_flash_shielding(u32 unit_id);
bool unit_id_has_shield_regeneration(u32 unit_id);
bool unit_id_is_stackable_building(u32 unit_id); // Allows the AI to place the building to be placed more closely
bool unit_id_has_addon(u32 unit_id); // Allows the AI to reserve space for addon
u32 unit_id_get_larva_type(u32 unit_id);
u32 unit_id_swarmhold_value(u32 unit_id);
bool unit_id_is_vespene_node(u32 unit_id);
bool unit_id_is_ground_garrison(u32 unit_id);
bool unit_id_is_garrison(u32 unit_id);
bool unit_id_is_melee_air_unit(u32 unit_id);
bool unit_id_is_morphed_from_double_unit(u32 unit_id);
bool unit_id_is_power_generator(u32 unit_id);
bool unit_id_is_psi_provider(u16 unit_id);
bool unit_id_can_do_controlled_demolition(u32 unit_id);
UnitId::Enum unit_id_to_siege_mode(UnitId::Enum unit_id);
UnitId::Enum unit_id_to_tank_mode(UnitId::Enum unit_id);
bool unit_id_is_siege_mode(UnitId::Enum unit_id);
bool unit_id_is_tank_mode(UnitId::Enum unit_id);
bool unit_id_is_broodspawn(u32 unit_id);
bool unit_id_is_zerg_production(u16 unit_id, bool town_center_only = false);
bool unit_id_is_town_center(u16 unit_id);
bool unit_id_is_factional(u16 unit_id);
bool unit_id_uses_xlarge_overlays(u16 unit_id);
bool unit_id_is_resource_node(u16 unit_id, bool count_gas_structures);
bool unit_id_is_minerals(u16 unit_id);
bool unit_id_is_zerg_supply(u16 unit_id);

// TODO: maybe move to cmbw_behaviors? or cmbw_utils?
bool unit_paths_over_small_units(Unit* unit);
bool unit_collides_with_titanic(Unit* unit);

bool weapon_id_is_for_harvesting(WeaponId::Enum weapon_id);
bool weapon_id_is_standard_weapon(WeaponId::Enum weapon_id);
bool weapon_id_bounces_lithe_organelle(WeaponId::Enum weapon_id);
bool weapon_id_bounces_crushing_tide(WeaponId::Enum weapon_id);
bool weapon_id_accelerates(WeaponId::Enum weapon_id);
bool weapon_id_is_melee_aoe(WeaponId::Enum weapon_id);
u32 weapon_id_intercepts(WeaponId::Enum weapon_id);
bool weapon_id_pierces(WeaponId::Enum weapon_id);
bool weapon_id_piercing_stops_on_armor(WeaponId::Enum weapon_id);

bool weapon_is_melee_with_aoe(Bullet* bullet, WeaponId::Enum weapon_id);
bool weapon_id_cleaves(WeaponId::Enum weapon_id);

bool order_id_is_harvest_related(u32 order_id);

bool tech_id_has_custom_targeting(u32 tech_id);

bool image_id_is_powerup(u32 image_id);
ImageId::Enum analogue_image_id_overwrite(WeaponId::Enum weapon_id);
bool weapon_id_can_proc_onhits(WeaponId::Enum weapon_id);
WeaponOffsets* get_weapon_offsets(WeaponId::Enum weapon_id);

bool unit_id_spawns_larva_by_order(u32 unit_id);
u32 get_worker_construction_distance(u32 unit_id);
bool unit_id_ignored_on_minimap(u32 unit_id);
bool unit_is_combatant(Unit* unit);
bool unit_id_can_repair(u32 unit_id);
bool weapon_id_is_flamethrower(WeaponId::Enum weapon_id);
bool unit_id_is_larva(u32 unit_id);
bool unit_id_is_zerg_egg(u32 unit_id);
u32 get_mutate_image_id(u32 unit_id);

bool unit_is_tower(Unit* unit);

bool unit_applies_ensnare(Unit* unit);
bool unit_can_be_ensnared(Unit* unit);

bool weapon_id_is_line_attack(WeaponId::Enum weapon_id);
u32 get_splash_width_from_weapon_id(WeaponId::Enum weapon_id);
u32 get_splash_height_from_weapon_id(WeaponId::Enum weapon_id);
bool weapon_id_bounces_caustic_fission(WeaponId::Enum weapon_id);
u32 get_death_sfx_id(u32 unit_id);
bool weapon_id_is_automatic(WeaponId::Enum weapon_id);
bool unit_id_has_custom_air_weapon(u32 unit_id);
bool weapon_id_returns_to_attacker(WeaponId::Enum weapon_id);
bool weapon_id_is_incremental(WeaponId::Enum weapon_id);
bool unit_id_has_proximity_cloak(u32 unit_id);
bool unit_id_is_ridge_refinery(u32 unit_id);
bool skip_military_tracking_by_unit_id(u32 unit_id);
bool unit_id_is_trap(u32 unit_id);
bool unit_building_can_act(Unit* unit);
bool unit_could_spawn_custom_larva(Unit* unit);
bool unit_spawns_custom_larva(Unit* unit);
bool unit_could_spread_custom_creep(Unit* unit);
bool unit_spreads_custom_creep(Unit* unit);
bool unit_id_should_give_gas_in_massacre_mode(u32 unit_id);
bool rapid_rendezvous_can_target_unit_id(u32 unit_id);
bool unit_id_ignores_ground_collision(u32 unit_id);
bool unit_id_has_projection_attack(u32 unit_id);
bool player_id_is_extended(u32 player_id);
bool order_id_is_build_order(u32 order_id);
bool unit_id_has_custom_weapon_handling(u32 unit_id);
bool unit_id_is_burrowed_attacker(u32 unit_id);
bool buttonset_id_is_faction_selection(u32 buttonset_id);
bool portrait_id_is_default_advisor(u32 portrait_id);
u32 get_player_portrait_id_override(u32 portrait_id, u32 player);
bool unit_id_is_tiny_unit(u32 unit_id);
bool unit_id_is_spell(u32 unit_id);
bool unit_id_has_cleric_heal(u32 unit_id);
bool unit_id_is_valid_for_most_abilities(u32 unit_id);
bool unit_id_is_hero(u32 unit_id);
bool weapon_id_can_self_splash(WeaponId::Enum weapon_id);
bool unit_id_is_mine_layer(UnitId::Enum unit_id);
bool unit_id_can_construct(UnitId::Enum unit_id);
bool unit_id_is_hangar_unit(UnitId::Enum unit_id);
bool unit_id_is_air_spawner(UnitId::Enum unit_id);
bool unit_id_is_flyer_or_air_spawner(UnitId::Enum unit_id);
bool weapon_id_deals_no_damage(WeaponId::Enum weapon_id);
bool unit_id_blocks_construction(UnitId::Enum unit_id);
UnitId::Enum get_racial_worker_id(u32 race_id);
UnitId::Enum get_racial_town_center_id(u32 race_id);

namespace UnitId {
bool can_cast_recall(UnitId::Enum unit_id);
bool has_manual_weapon_fire_sfx(UnitId::Enum unit_id);
bool is_atg_regression(UnitId::Enum old_unit_id, UnitId::Enum new_unit_id);
u32 get_overlay_offset(UnitId::Enum unit_id);
SpriteId::Enum get_corpse_sprite_id(UnitId::Enum unit_id);
bool is_askosi(UnitId::Enum);
bool is_bloodsummon(UnitId::Enum);
bool is_forerunner(UnitId::Enum);
SoundId::Enum get_unit_ready_sound(UnitId::Enum unit_id);
bool has_autovitality(UnitId::Enum unit_id);
bool turns_to_face_target(UnitId::Enum unit_id);
bool has_interruptable_windup(UnitId::Enum unit_id);
SoundId::Enum get_windup_sfx(UnitId::Enum unit_id);
bool has_automandate(UnitId::Enum unit_id);
bool is_terran_worker(UnitId::Enum unit_id);
bool is_zerg_worker(UnitId::Enum unit_id);
bool is_askosi_worker(UnitId::Enum unit_id);
bool is_beacon(UnitId::Enum unit_id);
bool has_fusion_sealers(Unit* unit);
bool match_id_to_base(UnitId::Enum match_id, UnitId::Enum baseline_id);
bool is_triple_birth(UnitId::Enum unit_id);
bool has_forward_march(UnitId::Enum unit_id);
bool spreads_cagra_while_burrowed(UnitId::Enum unit_id);
bool has_swarming_spite(UnitId::Enum unit_id);
bool has_power_breach(UnitId::Enum unit_id);
bool has_far_transport_pickup_range(UnitId::Enum unit_id);
bool has_interceptors(UnitId::Enum unit_id);
bool has_impact_dissipation(UnitId::Enum unit_id);
bool has_disruption_field(UnitId::Enum unit_id);
bool incubates_on_damage_taken(UnitId::Enum unit_id);
bool has_damage_cloak(UnitId::Enum unit_id);
bool has_forlorn_haste(UnitId::Enum unit_id);
} // namespace UnitId

namespace WeaponId {
u32  get_custom_melee_factor(WeaponId::Enum weapon_id);
bool weapon_splits(WeaponId::Enum weapon_id);
bool bounces_binding_plasma(WeaponId::Enum weapon_id);
bool can_attack_target(WeaponId::Enum weapon_id, Unit* target);
bool bounces_power_breach(WeaponId::Enum weapon_id);
} // namespace WeaponId