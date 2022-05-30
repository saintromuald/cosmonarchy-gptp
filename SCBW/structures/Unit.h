//This header file contains member function declarations for Unit.
//For the structure layout, see UnitLayout.h

#pragma once
#include <algorithm>
#include "UnitLayout.h"
#include "unit_constants.h"
#include <SCBW/enumerations.h>
#include "../../graphics/graphics.h"

struct Flingy
{
	/*0x000*/ Link<Unit> link;
	/*0x008*/ s32       value;                 ///for unit it's hitpoints
	/*0x00C*/ Sprite*   sprite;
	/*0x010*/ Target    moveTarget;            ///< The position or unit to move to. It is NOT an order target.
	/*0x018*/ Point16   nextMovementWaypoint;  ///< The next way point in the path the unit is following to get to its destination.
											   ///< Equal to moveToPos for air units since they don't need to navigate around buildings.
	/*0x01C*/ Point16   nextTargetWaypoint;    ///< The desired position
	/*0x020*/ u8        movementFlags;		   /// Flags specifying movement type, MovementFlags::Enum contains possible explanations
	/*0x021*/ u8        currentDirection1;	   /// The current direction the unit is facing
	/*0x022*/ u8        flingyTurnSpeed;       //AKA flingy turn radius
	/*0x023*/ u8        velocityDirection1;
	/*0x024*/ u16       flingyId;
	/*0x026*/ u8        _unknown_0x026;
	/*0x027*/ u8        flingyMovementType;
	/*0x028*/ Point16   position;
	/*0x02C*/ Vector32  halt;
	/*0x034*/ u32       flingyTopSpeed;
	/*0x038*/ u32       current_speed1;
	/*0x03C*/ u32       current_speed2;
	/*0x040*/ Vector32  current_velocity;
	/*0x048*/ u16       flingyAcceleration;
	/*0x04A*/ u8        currentDirection2;
	/*0x04B*/ u8        velocityDirection2;
};

//Wrapper structure that provides methods for the Unit structure.
struct Unit : public UnitLayout
{

	/// @name Unit Stats and Properties
	//////////////////////////////////////////////////////////////// @{

	/// Checks if the unit is a clean detector (no Lockdown, Optical Flare, etc.)
	bool canDetect() const;

	/// Returns the amount of HP displayed in-game. This is equal to
	/// (this->hitPoints + 255) / 256.
	u32 getCurrentHpInGame() const;

	/// Returns the amount of shield points displayed in-game. This is equal to
	/// unit->shields / 256.
	u32 getCurrentShieldsInGame() const;

	/// Returns the amount of HP + shields displayed in game. If the unit does not
	/// have shields enabled, this returns the amount of HP displayed.
	u32 get_current_life_ingame() const;

	/// Returns the unit's ground weapon ID. If the unit is an unburrowed Lurker,
	/// returns WeaponId::None instead.
	u8 getGroundWeapon() const;

	/// Returns the unit's air weapon ID. This is identical to reading the value
	/// from units_dat::AirWeapon[unit->id].
	u8 getAirWeapon() const;

	/// Returns the total armor amount of this unit (with upgrades).
	/// To retrieve a unit's base armor amount, use units_dat::ArmorAmount[].
	u8 getArmor() const;

	/// Returns the bonus armor amount of this unit (from upgrades).
	/// To retrieve a unit's base armor amount, use units_dat::ArmorAmount[].
	u8 getArmorBonus() const;

	/// Returns the maximum energy amount of this unit (with upgrades).
	u16 getMaxEnergy() const;

	/// Returns the maximum HP amount displayed in-game. This is equal to
	/// units_dat::MaxHitPoints[unit->id] >> 8 (minimum value is 1).
	u32 getMaxHpInGame() const;
	u32 getMaxHpRaw() const;

	u32 getMaxShieldsInGame() const;
	u32 getMaxShieldsRaw() const;


	/// Returns the maximum range of a weapon in pixels. The weapon is assumed to
	/// be attached to this unit for calculating upgrade effects.
	/// This is affected by the Weapon Range hook module.
	u32 getMaxWeaponRange(u8 weaponId) const;

	/// Retrieves the in-game name of this unit. In UMS maps, this returns custom
	/// names (if they exist).
	const char* getName() const;

	/// Retrieves the in-game name of the unit with @p unitId. In UMS maps, this
	/// returns custom unit names (if they exist).
	static const char* getName(u16 unitId);

	/// Returns the race of the current unit.
	/// @return   RaceId::Terran, Zerg, Protoss, or Neutral.
	RaceId::Enum get_race() const;

	/// Returns the race of the unit with @p unitId.
	/// @return   RaceId::Terran, Zerg, Protoss, or Neutral.
	static RaceId::Enum get_race(u16 unitId);

	/// Returns the unit's seek range (AKA target acquisition range) in matrices.
	/// This is affected by the Weapon Range hook module.
	u8 getSeekRange() const;

	/// Returns the sight range of this unit (with upgrades).
	/// If @p isForSpellCasting is true, also factors in status effects.
	u32 getSightRange(bool isForSpellCasting = false) const;

	///Return true if:
	/// * THE GATHERING cheat is enabled or
	/// * unit has at least @p energy available.
	bool hasEnergy(u32 energy) const;

	/// Returns true if:
	/// * The unit has a weapon, or
	/// * The unit's subunit has a weapon, or
	/// * If the unit is a loaded Carrier/Reaver.
	bool hasWeapon() const;

	/// Checks if the unit is unpowered / lockdowned / stasised / maelstromed.
	bool isFrozen() const;

	/// Check if the unit is a remorphing building (i.e. is a Lair, Hive,
	/// Greater Spire, Sunken or Spore Colony under construction).
	bool isRemorphingBuilding() const;

	/// Checks if the current unit is a subunit (i.e. not nullptr and has the
	/// UnitProperty::Subunit flag set).
	bool is_subunit() const;

	/// Checks if the current unit is a factory (i.e. not nullptr and has the
	/// flag isFactory from GroupFlags set).
	bool isFactory() const;

	/// Checks if the unit is a spellcaster (has energy) and not a hallucination.
	bool is_valid_caster() const;

	/// Checks if the unit is at move destination (0), or if the unit is
	/// unmovable (2) or not (1)
	u32 getMovableState();

	//////////////////////////////////////////////////////////////// @}

	/// @name Positions and Dimensions
	//////////////////////////////////////////////////////////////// @{

	/// Returns the X position of the unit's sprite on the map.
	u16 getX() const;
	/// Returns the Y position of the unit's sprite on the map.
	u16 getY() const;

	/// Returns the X position of the tile that the unit's sprite is on.
	u16 getTileX() const;
	/// Returns the Y position of the tile that the unit's sprite is on.
	u16 getTileY() const;

	/// Returns the position of the left edge of the unit's collision box.
	s16 getLeft() const;
	/// Returns the position of the right edge of the unit's collision box.
	s16 getRight() const;
	/// Returns the position of the top edge of the unit's collision box.
	s16 getTop() const;
	/// Returns the position of the bottom edge of the unit's collision box.
	s16 getBottom() const;

	//////////////////////////////////////////////////////////////// @}

	/// @name Modifying Unit State
	//////////////////////////////////////////////////////////////// @{

	/// Sets the unit's HP, and also updates burning / bleeding graphics and unit
	/// strength data (used by the AI). HP is guaranteed not to overflow.
	void setHp(s32 hitPoints);
	void heal(s32 hitPoints, Unit* source = nullptr, bool restore = true);
	void healShields(s32 shields, Unit* source = nullptr, bool restore = true);
	void healOverflow(s32 amount, Unit* source = nullptr, bool restore = true);
	void healUnderflow(s32 amount, Unit* source = nullptr, bool restore = true);
	void healEnergy(s32 energy, Unit* source = nullptr);

	/// Deals damage to this unit, using a specific weapons.dat ID.
	void damageWith(
		u8 weaponId,              ///< weapons.dat ID to use.
		s32 damage = -1,			 ///< Amount of damage dealt to this unit. -1 = inherit from weaponId
		Unit* attacker = nullptr,   ///< Attacking unit (for increasing kill count)
		u8 attackingPlayer = -1,  ///< Attacking player (for increasing kill score)
		s8 direction = 0,          ///< Attacked direction (for shield flicker overlays)
		u8 damageDivisor = 1      ///< Damage divisor (for splash damage / glave wurm calculations)
	);

	void Unit::damage_with_custom(
		s32 damage,
		u8 damageType,
		Unit* attacker,
		u8 attackingPlayerId,
		s8 direction,
		u8 damageDivisor,
		bool ignoreFlatArmor
	);

	/// Deals damage directly to unit HP, killing it if possible.
	void damageHp(s32 damage, Unit* attacker = nullptr,
		s32 attackingPlayer = -1, bool notify = true);

	/// Deals damage to shields and hp
	void inflictDamage(s32 damage, Unit* attacker = nullptr,
		s32 attackingPlayer = -1, bool notify = true, bool shield_overlay = false, s32 direction = 0, bool ignoreDmg128Overflow = false);



	/// Reduces Defensive Matrix by @p amount, removing it if possible.
	void reduce_defensive_matrix_hp(s32 amount);

	/// Immediately kills the unit. To silently remove the unit, use:
	///   unit->userActionFlags |= 0x4;
	///   unit->remove();
	void remove();

	/// Removes the Lockdown effect from the unit.
	void remove_lockdown();

	/// Removes the Maelstrom effect from the unit.
	void remove_maelstrom();

	/// Freezes the unit
	void freeze();

	/// Removes the Stasis Field effect from the unit.
	void unfreeze();

	/// Removes the Acid Spores effect from the unit.
	void removeAcidSpores();

	/// Spend an amount of energy, if no THE GATHERING cheat.
	void spendUnitEnergy(u32 energy);
	void addUnitEnergy(u32 energy);
	void addUnitEnergy(u32 energy, u32 max);

	//////////////////////////////////////////////////////////////// @}

	/// @name Unit Orders
	//////////////////////////////////////////////////////////////// @{

	/// Issue the @p order to the unit, using the given @p target unit.
	void orderTo(u8 orderId, Unit* target = nullptr);

	/// Issue the @p order to the unit, using the given position as the target.
	void orderTo(u8 orderId, u16 x, u16 y);

	//  Issue the @p order to the unit, without a target
	void order_computer(u8 orderId);

	//  Issue the @p order to the unit and specify if previous orders should be stopped
	void orderSimple(u8 orderId, bool stopPreviousOrders);

	//  Issue the @p order to the unit, using the given @p target unit and specify if previous orders should be stopped
	void issueQueuedOrderTarget(u8 orderId, Unit* target, bool stopPreviousOrders);

	/// Causes the unit to become idle.
	void orderToIdle();

	/// Issues a new order to the unit.
	void order(u8 orderId, u16 x, u16 y, Unit* target, u16 targetUnitId, bool stopPreviousOrders);

	/// Probably add the @p order to the orders queue
	void performAnotherOrder(u8 orderId, s16 x, s16 y, Unit* target, u16 targetUnitId, u32 unk = NULL);

	/// Used by several hooks, details not completely understood.
	void set_secondary_order(u8 orderId);

	//////////////////////////////////////////////////////////////// @}

	/// @name Graphics and Animations
	/// Note: See Sprite::createOverlay() and Sprite::createTopOverlay() for
	///       creating image overlays.
	//////////////////////////////////////////////////////////////// @{

	/// Returns the overlay image of this unit's sprite (or its subunit's sprite)
	/// that matches the @p imageId. If the image cannot be found, returns nullptr.
	Image* getOverlay(u16 imageId) const;
	s32 maxShields() const;
	bool shieldsEnabled() const;

	/// Makes the unit's sprite play the specified Iscript animation entry.
	void play_iscript_animation(IscriptAnimation::Enum animation);

	/// Removes the first image overlay with an ID value between @p imageIdStart
	/// and @p imageIdEnd.
	void removeOverlay(u32 imageIdStart, u32 imageIdEnd);

	/// Removes the first image overlay with the given image ID.
	void removeOverlay(u32 imageId);

	/// Get the faction (brood,tribe,squad...) color of current unit
	graphics::ColorId getFactionColor();

	/// Get the faction (brood,tribe,squad...) color of an unit
	static graphics::ColorId getFactionColor(Unit* unit);

	/// Get the (possibly non-standard) color of current unit
	graphics::ColorId getColor();

	/// Get the (possibly non-standard) color of an unit
	static graphics::ColorId getColor(Unit* unit);

	//////////////////////////////////////////////////////////////// @}

	/// @name Distances and Terrain
	//////////////////////////////////////////////////////////////// @{

	/// Returns the distance between this unit and the @p target, taking unit
	/// collision size in units.dat into account.
	/// Internally, this function uses scbw::get_distance_fast().
	u32 getDistanceToTarget(Unit* target) const;

	/// Checks whether this unit can reach the target position. This checks only
	/// for terrain, and does not consider obstacles (units and buildings).
	bool hasPathToPos(u32 x, u32 y) const;

	/// Checks whether this unit can reach the @p target unit. This checks only
	/// for terrain, and does not consider obstacles (units and buildings).
	bool hasPathToUnit(Unit* target) const;

	///  Checks whether the @p target unit is within the @p range from the unit.
	bool isTargetWithinMinRange(Unit* target, u32 range);

	//////////////////////////////////////////////////////////////// @}

	/// @name DAT Requirements
	//////////////////////////////////////////////////////////////// @{

	/// Checks if the @p unit can build / train / morph into @p unitId. This checks:
	/// * If the unit is actually owned by the commanding @p playerId
	/// * Whether the unit is stunned / disabled.
	///
	/// If the unit can build / train / morph, returns 1. If the tech needs to be
	/// researched, returns -1. If the unit cannot use the tech at all, returns 0.
	s32 can_make_unit(u16 unitId, u8 playerId) const;

	/// Checks the following:
	///  * If the unit is actually owned by the commanding @p playerId
	///  * If the unit's owning player has the @p techId researched
	///  * If the unit has enough energy (or energy cheat is enabled)
	///  * If the unit is not stunned / is a hallucination / is being built
	///  * If the unit meets the Use Tech Requirements (editable in FireGraft)
	///
	/// If the unit can use the tech, returns 1. If the tech needs to be
	/// researched, returns -1. If the unit cannot use the tech at all, returns 0.
	s32 canUseTech(u8 techId, u8 playerId) const;

	//////////////////////////////////////////////////////////////// @}

	/// @name Utility Methods
	//////////////////////////////////////////////////////////////// @{

	/// Returns true if the current unit is not frozen and the @p target unit is
	/// not invincible and can be attacked or infested. If @p checkVisibility is
	/// set to false, the function will not check whether the unit can see the
	/// @p target (i.e. it is being detected properly). Additionally:
	///
	/// * Queens (and Matriarchs) return true only if the @p target is an
	///   infestable Command Center.
	/// * Reavers (and Warbringers) return true only if they have a ground path
	///   to the @p target unit.
	/// * AI-controlled Arbiters (not including Danimoths) always return false.
	bool canAttackTarget(Unit* target, bool checkVisibility = true) const;

	/// Makes the unit use the specified weapon to attack its current target unit
	/// stored in the Unit::orderTarget.unit member. This does not affect the
	/// unit's weapon cooldown. The spawned weapon sprite obeys the weapon
	/// behavior properties in weapons.dat.
	/// Note: This function is affected by the Fire Weapon hook module.
	void fireWeapon(u8 weaponId) const;

	/// Retrieves the unit pointer by @p index from the unit table (first unit is
	/// indexed at 1). If invalid, returns nullptr instead.
	static Unit* getFromIndex(u16 index);

	/// Returns the index of this unit in the unit table. First unit == index 1.
	inline u16 getIndex() const;

	/// Returns the ID of the last player to own this unit. This is usually the
	/// same as Unit::playerId, but if the unit belongs to a defeated player,
	/// this returns the correct player ID (instead of 11).
	/// @deprecated   Will be removed in v2.42. Use Unit::isTargetEnemy().
	u8 getLastOwnerId() const;

	/// Returns the loaded unit at @p index (value between 0-7). If no unit is
	/// loaded at the slot, returns nullptr instead.
	Unit* getLoadedUnit(s32 index) const;

	/// Returns the first unit loaded in this unit. If this unit is not a
	/// transport, or has no units inside, this function returns nullptr instead.
	Unit* getFirstLoadedUnit() const;

	/// Checks if this unit has other units loaded inside.
	bool hasLoadedUnit() const;

	/// Transfers ownership of this unit to @p playerId. This function is
	/// identical to Mind Control and the "Give Units To Player" trigger action.
	/// @return   true if successful, false otherwise.
	bool giveTo(u8 playerId);

	/// Checks if the unit has an associated sprite and is not running the
	/// OrderId::Die order.
	bool isDead() const;

	/// Checks whether the @p target unit is an enemy of this unit.
	/// Internally, this calls scbw::isUnitEnemy().
	bool isTargetEnemy(Unit* target) const;

	/// Checks whether this unit can be seen by @playerId (i.e. not covered by the
	/// fog-of-war and is detectable).
	bool isVisibleTo(u8 playerId) const;

	/// Updates the unit's actual speed. This function should be called after
	/// changing any properties and status effects that affect movement speed.
	void update_speed();

	/// For Wyvern Land Grab
	u8 getSpellCountdown() const;

	//////////////////////////////////////////////////////////////// @}
	
	u32 getSupplyProvided();
	void setSupplyProvided(u8 amount);
	void resetSupplyProvided();

	s32 get_total_hp_plus_shields();
	s32 is_unit_ai_non_building();

	/// Returns whether the unit is currently constructing a Protoss building
	bool isAttemptingProtossBuild();

	/// Returns whether the unit is currently constructing an addon
	bool isConstructingAddon();

	/// Returns the number of hangar units currently being trained by the unit
	u8 getHangarTrainCount();

	/// Returns the maximum hangar space for the unit
	u8 getMaxHangarSpace();

	/// Returns whether the unit is of type Reaver (incl. hero Reaver)
	bool isReaver();

	/// Returns whether the unit is of type Carrier (incl. hero Carrier)
	bool isCarrier();

	/// Returns whether the unit is currently constructing a Protoss building
	bool unitIsTrainingOrMorphing();

	/// Returns whether the unit is currently constructing a Protoss building
	bool isQueueSlotActive(int slot);

	// Returns the right click action for the given unit
	u8 getRightClickActionOrder();
};

C_ASSERT(sizeof(Flingy) == 0x4c);
C_ASSERT(sizeof(Unit) == sizeof(UnitLayout));
//static_assert(sizeof(Unit) == sizeof(UnitLayout), "The size of the Unit structure is invalid");