#pragma once
#include "SCBW\scbwdata.h"
#include "hooks\weapons\weapon_fire.h"

bool unit_id_is_capped_gas_structure(u16 unit_id);
bool unit_id_is_gas_structure(u16 unit_id);
bool unit_id_is_uncapped_gas_node(u16 unit_id);
bool unit_id_is_smaller_building(u16 unit_id);
b32 unit_id_is_tech(u32 unit_id);
b32 unit_id_is_tech_tier_structure(u32 unit_id);
bool unit_id_is_transport(u32 unit_id);
bool unit_id_race_check(u32 unit_id, u32 race);
bool unit_id_is_protoss_warp_factory(u32 unit_id);
bool unit_id_can_regenerate_shields(u32 unit_id);
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
bool unit_id_is_siege_mode(u32 unit_id);
bool unit_id_is_tank_mode(u32 unit_id);
bool unit_id_is_broodspawn(u32 unit_id);
bool unit_id_is_zerg_production(u16 unit_id, bool town_center_only = false);
bool unit_id_is_town_center(u16 unit_id);
bool unit_id_is_factional(u16 unit_id);
bool unit_id_uses_xlarge_overlays(u16 unit_id);
bool unit_id_is_resource_node(u16 unit_id, bool count_gas_structures);
bool unit_id_is_minerals(u16 unit_id);
bool unit_id_is_zerg_supply(u16 unit_id);

// TODO: maybe move to cmbw_behaviors? or cmbw_utils?
bool unit_regens_in_combat(Unit* unit);
bool unit_paths_over_small_units(Unit* unit);
bool unit_collides_with_titanic(Unit* unit);

bool weapon_id_is_for_harvesting(u32 weapon_id);
bool weapon_id_is_standard_weapon(u32 weapon_id);
bool weapon_id_bounces_lithe_organelle(u32 weapon_id);
bool weapon_id_bounces_crushing_tide(u32 weapon_id);
bool weapon_id_accelerates(u32 weapon_id);
bool weapon_id_is_melee_aoe(u32 weapon_id);
u32 weapon_id_intercepts(u32 weapon_id);
bool weapon_id_pierces(u32 weapon_id);
bool weapon_id_piercing_stops_on_armor(u32 weapon_id);

bool weapon_is_melee_with_aoe(Bullet* bullet, u32 weapon_id);

bool order_id_is_harvest_related(u32 order_id);

bool tech_id_needs_target(u32 tech_id);

bool image_id_is_powerup(u32 image_id);
ImageId::Enum analogue_image_id_overwrite(u32 weapon_id);
bool weapon_id_procs_onhits(u32 weapon_id);
WeaponOffsets* get_weapon_offsets(u32 weapon_id);

bool unit_id_spawns_larva(u32 unit_id);
u32 get_scribe_warp_distance(u32 unit_id);
bool unit_id_ignored_on_minimap(u32 unit_id);
bool unit_is_combatant(Unit* unit);
bool unit_id_can_repair(u32 unit_id);
bool weapon_id_is_flamethrower(u32 weapon_id);
bool unit_id_is_larva(u32 unit_id);
u32 get_mutate_image_id(u32 unit_id);

bool unit_is_tower(Unit* unit);

bool unit_applies_ensnare(Unit* unit);
bool unit_can_be_ensnared(Unit* unit);

bool weapon_id_is_line_attack(u32 weapon_id);
u32 get_splash_width_from_weapon_id(u32 weapon_id);
u32 get_splash_height_from_weapon_id(u32 weapon_id);
bool weapon_id_bounces_caustic_fission(u32 weapon_id);
u32 get_death_sfx_id(u32 unit_id);
u32 weapon_id_is_secondary(u32 weapon_id);