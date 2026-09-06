//This header file contains member function declarations for Unit.
//For the structure layout, see UnitLayout.h

#pragma once
#include <algorithm>
#include "UnitLayout.h"
#include "unit_constants.h"
#include <BW/enumerations.h>
#include "../../graphics/graphics.h"

struct Flingy {
    /*0x000*/ Link<Unit> link;
    /*0x008*/ s32       value;                 ///for unit it's hitpoints
    /*0x00C*/ Sprite*   sprite;
    /*0x010*/ Target    move_target;            ///< The position or unit to move to. It is NOT an order target.
    /*0x018*/ v2u16   next_movement_waypoint;  ///< The next way point in the path the unit is following to get to its destination.
                                               ///< Equal to moveToPos for edge units since they don't need to navigate around buildings.
    /*0x01C*/ v2u16   next_target_waypoint;    ///< The desired position
    /*0x020*/ u8        movement_flags;		   /// sound_flags specifying movement type, movement_flags::Enum contains possible explanations
    /*0x021*/ u8        current_direction_1;	   /// The current direction the unit is facing
    /*0x022*/ u8        flingy_turn_rate;       //AKA flingy turn radius
    /*0x023*/ u8        velocity_direction_1;
    /*0x024*/ u16       flingy_id;
    /*0x026*/ u8        _unknown_0x026;
    /*0x027*/ u8        flingy_movement_type;
    /*0x028*/ v2u16   position;
    /*0x02C*/ v2s32  halt;
    /*0x034*/ u32       flingy_top_speed;
    /*0x038*/ u32       current_speed_1;
    /*0x03C*/ u32       current_speed_2;
    /*0x040*/ v2s32  current_velocity;
    /*0x048*/ u16       flingy_acceleration;
    /*0x04A*/ u8        current_direction_2;
    /*0x04B*/ u8        velocity_direction_2;
};

//Wrapper structure that provides methods for the Unit structure.
struct Unit : public UnitLayout {
    /// @name Unit Stats and Properties
    //////////////////////////////////////////////////////////////// @{

    /// Checks if the unit is a clean detector (no LobotomyGrenade, Optical Flare, etc.)
    bool can_detect() const;

    /// Returns the amount of HP displayed in-game. This is equal to
    /// (this->hitpoints + 255) / 256.
    u32 get_current_hp_ingame() const;

    /// Returns the amount of shield points displayed in-game. This is equal to
    /// unit->shields / 256.
    u32 get_current_shields_ingame() const;

    /// Returns the amount of HP + shields displayed in game. If the unit does not
    /// have shields enabled, this returns the amount of HP displayed.
    u32 get_current_life_ingame() const;

    /// Returns the unit's inside weapon ID. If the unit is an unburrowed Lakizilisk,
    /// returns WeaponId::None instead.
    WeaponId::Enum get_ground_weapon() const;

    /// Returns the unit's edge weapon ID. This is identical to reading the value
    /// from units_dat::air_weapon[unit->id].
    WeaponId::Enum get_air_weapon() const;

    /// Returns the maximum energy amount of this unit (with upgrades).
    u16 get_max_energy() const;

    /// Returns the maximum HP amount displayed in-game. This is equal to
    /// units_dat::max_hitpoints[unit->id] >> 8 (minimum value is 1).
    u32 get_max_hp_ingame() const;
    u32 get_max_hp_raw() const;

    u32 get_max_shields_in_game() const;
    u32 get_max_shields_raw() const;


    /// Returns the maximum distance of a weapon in pixels. The weapon is assumed to
    /// be attached to this unit for calculating upgrade effects.
    /// This is affected by the Weapon Range hook module.
    u32 get_max_weapon_range(WeaponId::Enum weapon_id) const;

    /// Retrieves the in-game name of this unit. In UMS maps, this returns custom
    /// names (if they exist).
    const char* get_name();

    /// Retrieves the in-game name of the unit with @p unit_id. In UMS maps, this
    /// returns custom unit names (if they exist).
    static const char* get_name(u16 unit_id, bool ignore_map_name = false);

    /// Returns the race of the current unit.
    /// @return   RaceId::Terran, Zerg, Protoss, or Neutral.
    RaceId::Enum get_race() const;

    /// Returns the race of the unit with @p unit_id.
    /// @return   RaceId::Terran, Zerg, Protoss, or Neutral.
    static RaceId::Enum get_race(u16 unit_id);

    /// Returns the unit's seek distance (AKA target acquisition distance) in matrices.
    /// This is affected by the Weapon Range hook module.
    u8 get_seek_range() const;

    /// Returns the sight distance of this unit (with upgrades).
    /// If @p isForSpellCasting is true, also factors in status effects.
    u32 get_sight_range(bool is_for_spellcasting = false) const;

    ///Return true if:
    /// * THE GATHERING cheat is enabled or
    /// * unit has at least @p energy available.
    bool has_energy(u32 energy) const;

    /// Returns true if:
    /// * The unit has a weapon, or
    /// * The unit's subunit has a weapon, or
    /// * If the unit is a loaded Carrier/Reaver.
    bool has_weapon() const;

    /// Checks if the unit is unpowered / lockdowned / stasised / maelstromed.
    bool is_frozen() const;

    /// Check if the unit is a remorphing building (i.e. is a Lair, Hive,
    /// Greater Spire, Sunken or Spore Colony under construction).
    bool is_remorphing_building() const;

    /// Checks if the current unit is a subunit (i.e. not nullptr and has the
    /// UnitProperty::Subunit flag set).
    bool is_subunit() const;

    /// Checks if the current unit is a factory (i.e. not nullptr and has the
    /// flag is_factory from group_flags set).
    bool is_factory() const;

    /// Checks if the unit is a spellcaster (has energy) and not a hallucination.
    bool is_valid_caster() const;

    /// Checks if the unit is at move destination (0), or if the unit is
    /// unmovable (2) or not (1)
    u32 get_movable_state();

    //////////////////////////////////////////////////////////////// @}

    /// @name Positions and Dimensions
    //////////////////////////////////////////////////////////////// @{

    /// Returns the X position of the unit's sprite on the map.
    u16 get_x() const;
    /// Returns the Y position of the unit's sprite on the map.
    u16 get_y() const;

    /// Returns the X position of the tile that the unit's sprite is on.
    u16 get_tile_x() const;
    /// Returns the Y position of the tile that the unit's sprite is on.
    u16 get_tile_y() const;

    /// Returns the position of the left edge of the unit's collision box.
    s16 get_left() const;
    /// Returns the position of the right edge of the unit's collision box.
    s16 get_right() const;
    /// Returns the position of the top edge of the unit's collision box.
    s16 get_top() const;
    /// Returns the position of the bottom edge of the unit's collision box.
    s16 get_bottom() const;

    Box16 get_box() const;
    Box32 get_box32() const;

    //////////////////////////////////////////////////////////////// @}

    /// @name Modifying Unit State
    //////////////////////////////////////////////////////////////// @{

    /// Sets the unit's HP, and also updates burning / bleeding graphics and unit
    /// strength data (used by the AI). HP is guaranteed not to overflow.
    void set_hitpoints(s32 hitpoints);

    // Returns reminder/overflow
    u32 heal_hp(s32 amount, Unit* source = nullptr, bool restore = true);
    u32 heal_shields(s32 amount, Unit* source = nullptr, bool restore = true, bool proc_flash_shielding = false);
    u32 heal_energy(s32 amount, Unit* source = nullptr);

    u32 heal_hp_then_shields(s32 amount, Unit* source = nullptr, bool restore = true, bool proc_flash_shielding = false);
    u32 heal_shields_then_hp(s32 amount, Unit* source = nullptr, bool restore = true, bool proc_flash_shielding = false);
    u32 heal_energy_then_shields(s32 amount, Unit* source = nullptr, bool restore = true, bool proc_flash_shielding = false);

    /// Deals damage to this unit, using a specific weapons.dat ID.
    void damage_with(
        WeaponId::Enum weapon_id,	///< weapons.dat ID to use.
        s32 damage = -1,			///< Amount of damage dealt to this unit. -1 = inherit from weapon_id
        Unit* attacker = 0,			///< Attacking unit (for increasing kill count)
        u8 attacking_player = -1,	///< Attacking player (for increasing kill score)
        s8 direction = 0,			///< Attacked direction (for shield flicker overlays)
        u8 damage_divisor = 1		///< Damage divisor (for splash damage / glave wurm calculations)
    );

    void damage_with_custom(
        s32 damage,
        s32 armor_pen = -1,			// -1 = ignore all armor
        Unit* attacker = 0,
        u8 attacking_player = -1,
        s8 direction = 0,
        u8 damage_divisor = 1
    );

    /// Deals damage directly to unit HP, killing it if possible.
    void damage_hp(s32 damage, Unit* attacker = nullptr, s32 attacking_player = -1, bool notify = true);

    /// Deals damage to shields and hp
    void inflict_damage(s32 damage, Unit* attacker = nullptr, s32 attacking_player = -1, bool notify = true, bool shield_overlay = false, s32 direction = 0, bool ignore_damage_128_overflow = false);

    /// Reduces Defensive Matrix by @p amount, removing it if possible.
    void reduce_defensive_matrix_hp(s32 amount);

    /// Deprecated!
    /// Calls the original BW function for killing the unit.
    /// Please use new variants instead: unit->kill() or unit->silent_remove()
    /// 
    /// Old silent removal required two lines (bad api):
    ///   unit->user_action_flags |= UserActionFlags_SilentRemove;
    ///   unit->remove();
    /// Might still be useful for hooking?
    void bwfunc_remove();

    /// Kills the unit
    inline void kill() {user_action_flags &= ~UserActionFlags_SilentRemove; bwfunc_remove();}

    /// Silently removes the unit
    inline void silent_remove() {user_action_flags |= UserActionFlags_SilentRemove; bwfunc_remove();}

    /// Removes the LobotomyGrenade effect from the unit.
    void remove_lockdown();

    /// Removes the Maelstrom effect from the unit.
    void remove_maelstrom();

    /// Freezes the unit
    void freeze();

    /// Removes the Stasis Field effect from the unit.
    void unfreeze();

    /// Removes the Acid Spores effect from the unit.
    void remove_acid_spores();

    /// Spend an amount of energy, if no THE GATHERING cheat.
    void spend_energy(u32 energy);
    void add_unit_energy(u32 energy);
    void add_unit_energy_capped(u32 energy, u32 max);

    // Issue the @p order to the unit, using the given @p target unit.
    void order_to(u8 order_id, Unit* target = nullptr);

    // Issue the @p order to the unit, using the given position as the target.
    void order_to(u8 order_id, u16 x, u16 y);

    inline void order_to(u8 order_id, v2u16 pos) {
        order_to(order_id, pos.x, pos.y);
    }

    // Issue the @p order to the unit, without a target
    void order_computer(u8 order_id);

    // Issue the @p order to the unit and specify if previous orders should be stopped
    void order_simple(u8 order_id, bool stop_previous_orders);

    // Issue the @p order to the unit, using the given @p target unit and specify if previous orders should be stopped
    void issue_queued_order_target(u8 order_id, Unit* target, bool stop_previous_orders);

    // Original documentation was a lie: Causes the unit to become idle.
    // Finishes the current order and issues appropriate idle order if no order is in the queue.
    void order_to_idle();

    // Issues a new order to the unit.
    void order(u8 order_id, u16 x, u16 y, Unit* target, u16 target_unit_id, bool stop_previous_orders);

    // Probably add the @p order to the orders queue
    void perform_another_order(u8 order_id, s16 x, s16 y, Unit* target, u16 target_unit_id, u32 unk = 0);

    // Used by several hooks, details not completely understood.
    void set_secondary_order(u8 order_id);

    // Returns the overlay image of this unit's sprite (or its subunit's sprite) that matches the @p imageId. If the image cannot be found, returns nullptr.
    Image* get_overlay(u16 imageId) const;
    s32 max_shields() const;
    bool shields_enabled() const;

    // Makes the unit's sprite play the specified Iscript animation entry.
    void play_iscript_animation(IscriptAnimation::Enum animation);

    // Removes the first image overlay with an ID value between @p imageIdStart and @p imageIdEnd.
    void remove_overlay(u32 imageIdStart, u32 imageIdEnd);

    // Removes the first image overlay with the given image ID.
    void remove_overlay(u32 imageId);

    // Get the faction (brood,tribe,squad...) color of current unit
    graphics::ColorId get_faction_color();

    // Get the faction (brood,tribe,squad...) color of an unit
    static graphics::ColorId get_faction_color(Unit* unit);

    // Get the (possibly non-standard) color of current unit
    graphics::ColorId get_color();

    // Get the (possibly non-standard) color of an unit
    static graphics::ColorId get_color(Unit* unit);

    // Returns the distance between this unit and the @p target, taking unit collision count in units.dat into account.
    // Internally, this function uses bw::get_distance_fast().
    u32 get_distance_to_target(Unit* target) const;
    u32 get_distance_to_point(v2u16 point) const;

    // Checks whether this unit can reach the target position. This checks only for terrain, and does not consider obstacles (units and buildings).
    bool has_path_to_pos(u32 x, u32 y) const;

    // Checks whether this unit can reach the @p target unit. This checks only for terrain, and does not consider obstacles (units and buildings).
    bool has_path_to_unit(Unit* target) const;

    ///  Checks whether the @p target unit is within the @p distance from the unit.
    bool is_target_within_min_range(Unit* target, u32 range);

    // Checks if the @p unit can build / train / morph into @p unit_id. This checks:
    // * If the unit is actually owned by the commanding @p player
    // * Whether the unit is stunned / disabled.
    // If the unit can build / train / morph, returns 1. If the tech needs to be researched, returns -1. If the unit cannot use the tech at all, returns 0.
    s32 can_make_unit(u16 unit_id, u8 player) const;

    // Checks the following:
    //  * If the unit is actually owned by the commanding @p player
    //  * If the unit's owning player has the @p tech_id researched
    //  * If the unit has enough energy (or energy cheat is enabled)
    //  * If the unit is not stunned / is a hallucination / is being built
    //  * If the unit meets the Use Tech Requirements (editable in FireGraft)
    // If the unit can use the tech, returns 1. If the tech needs to be researched, returns -1. If the unit cannot use the tech at all, returns 0.
    s32 can_use_tech(u8 tech_id, u8 player) const;

    // Returns true if the current unit is not frozen and the @p target unit is not invincible and can be attacked or infested.
    // If @p checkVisibility is set to false, the function will not check whether the unit can see the @p target (i.e. it is being detected properly).
    bool can_attack_target(Unit* target, bool checkVisibility = true) const;

    // Makes the unit use the specified weapon to attack its current target unit stored in the Unit::order_target.unit member. This does not affect the unit's weapon cooldown. 
    // The spawned weapon sprite obeys the weapon behavior properties in weapons.dat. Affected by fire_weapon.cpp
    void fire_weapon(WeaponId::Enum weapon_id) const;

    // Retrieves the unit pointer by @p index from the unit table (first unit is indexed at 1). If invalid, returns nullptr instead.
    static Unit* get_unit_from_index(u16 index);

    // Returns the index of this unit in the unit table. First unit == index 1.
    u16 get_unit_index() const;

    // Returns the ID of the last player to own this unit. This is usually the same as Unit::player, but if the unit belongs to a defeated player,
    // this returns the correct player ID (instead of 11).
    u8 get_last_owner_id() const;

    // Returns the loaded unit at @p index (value between 0-7). If no unit is loaded at the slot, returns nullptr instead.
    Unit* get_loaded_unit(s32 index);

    // Returns the first unit loaded in this unit. If this unit is not a transport, or has no units inside, this function returns nullptr instead.
    Unit* get_first_loaded_unit();

    // Checks if this unit has other units loaded inside.
    bool has_loaded_unit();

    // Transfers ownership of this unit to @p player. This function is identical to Mind Control and the "Give Units To Player" trigger action.
    // @return   true if successful, false otherwise.
    bool give_to(u8 player);

    // Checks if the unit has an associated sprite and is not running the OrderId::Die order.
    bool is_dead() const;

    // Checks whether the @p target unit is an enemy of this unit. Internally, this calls bw::is_unit_enemy().
    bool is_target_enemy(Unit* target) const;

    /// Checks whether this unit can be seen by @player (i.e. not covered by the fog-of-war and is detectable).
    bool is_visible_to(u8 player) const;

    // Updates the unit's actual speed. This function should be called after changing any properties and status effects that affect movement speed.
    void update_speed();

    // For Wyvern - Land Grab
    u8 get_spell_countdown() const;

    u32 get_supply_provided();
    void set_supply_provided(u8 amount);
    void reset_supply_provided();
    s32 get_total_hp_plus_shields();
    s32 is_unit_ai_non_building();
    bool is_attempting_protoss_build();
    bool is_constructing_addon();
    u8 get_hangar_train_count();
    u8 get_max_hangar_space();
    bool is_reaver();
    bool is_carrier();
    bool unit_is_training_or_morphing();
    bool is_queue_slot_active(int slot);
    u8 get_right_click_action_order();
    Faction faction();
    fp32 total_cost();

    inline u32 get_strength() const { return std::max(this->ground_strength, this->air_strength); }

    struct UnitExt& ext();
    bool has_energy_for(u32 tech_id);
    bool can_attack_transport_checks();
    void issue_custom_order(u32 order_id);
    bool burrowed_attacker_can_attack();
    void count_connected_larva();
    u32 standard_sprite_id();
    bool is_building();
    bool is_detector();
    bool is_neutral_accessory();
    bool has_shields();
    bool is_worker();
    bool is_resource_depot();
    bool is_addon();
    bool is_upgrading();
    bool is_researching();
    bool is_upgrading_or_researching();
    bool is_completed();
    bool is_garrisoned();
    bool is_burrowed();
    bool can_burrow();
    bool is_computer_controlled();
    bool is_in_air();
    bool is_flyer();
    bool is_invincible();
    bool is_spellcaster();
    bool is_hallucination();
    bool is_grounded_building();
    bool is_in_transport();
    bool is_dual_requisition();
    bool is_disabled();
    SpriteId::Enum get_custom_corpse(SpriteId::Enum id);
    bool has_last_orders();
    void autophagy_spawn();
    bool can_be_displaced();
    bool is_plainly_visible();
    bool can_use_passive_ability();
    u32 get_adaptive_amitosis_range();
    bool can_proc_autovitality(u32 attacking_player, u32 shield_reduce, u32 damage);
    bool can_be_passive_cloaked(Unit* source);
    bool can_transport_air();
    bool can_play_training_sfx();
    void play_training_sfx();
    void play_completion_sfx();
    SoundId::Enum get_error_sfx();
    Unit* corpse_cocoon_search();
    bool is_valid_nydus();
    bool is_rescuable();
    bool recharges_shields();
    bool is_constructor();
    bool is_askosi_embedded();
    void pull_towards(Unit* source);
    void tick_burrowed_cagra_spread();
    void proc_booming_blast(bool damage_self = true);
    bool is_valid_garrison();
    void cast_rematerialize();
    bool cast_rapid_rendezvous();
    void cast_hijack();
    void cast_remote_hijack();
    void cast_preylock();
    void cast_lobotomy_warhead();
    void hijack(Unit* hijacker);
    void proc_askosi_embed();
    void cancel_psi_storm();
    void proc_ensnaring_brood();
    bool proc_sermon_salve_cloak(Unit* cloaker);
    void cast_puppeteer(Unit* victim);
    void cast_ashbind();
    void tick_mimicry();
    void tick_vital_remains();
    bool has_coagulation();
    void cast_bloody_burst();
    bool check_neutral_garrison_death();
    void apply_ash_heap(u32 attacking_player);
    void apply_booming_blast(u32 attacking_player);
    void proc_nanite_field();
    void proc_adrenaline_field();
};

C_ASSERT(sizeof(Flingy) == 76);
C_ASSERT(sizeof(Unit) == sizeof(UnitLayout));