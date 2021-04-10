//This header file contains member function declarations for CUnit.
//For the structure layout, see CUnitLayout.h

#pragma once
#include <algorithm>
#include "CUnitLayout.h"
#include "UnitConstants.h"
#include <SCBW/enumerations.h>
#include "../../graphics/graphics.h"

struct CFlingy {
	/*0x000*/ CLink<CUnit> link;
	/*0x008*/ int32       value;                ///for unit it's hitpoints
	/*0x00C*/ CSprite* sprite;
	/*0x010*/ Target    moveTarget;           ///< The position or unit to move to. It is NOT an order target.
	/*0x018*/ Point16   nextMovementWaypoint; ///< The next way point in the path the unit is following to get to its destination.
											  ///< Equal to moveToPos for air units since they don't need to navigate around buildings.
	/*0x01C*/ Point16   nextTargetWaypoint;   ///< The desired position
	/*0x020*/ uint8        movementFlags;		  /// Flags specifying movement type, MovementFlags::Enum contains possible explanations
	/*0x021*/ uint8        currentDirection1;	  /// The current direction the unit is facing
	/*0x022*/ uint8        flingyTurnSpeed;      //AKA flingy turn radius
	/*0x023*/ uint8        velocityDirection1;
	/*0x024*/ uint16       flingyId;
	/*0x026*/ uint8        _unknown_0x026;
	/*0x027*/ uint8        flingyMovementType;
	/*0x028*/ Point16   position;
	/*0x02C*/ Point32   halt;
	/*0x034*/ uint32       flingyTopSpeed;
	/*0x038*/ uint32       current_speed1;
	/*0x03C*/ uint32       current_speed2;
	/*0x040*/ Point32   current_velocity;
	/*0x048*/ uint16       flingyAcceleration;
	/*0x04A*/ uint8        currentDirection2;
	/*0x04B*/ uint8        velocityDirection2;
};


//Wrapper structure that provides methods for the CUnit structure.
struct CUnit : public CUnitLayout {

	/// @name Unit Stats and Properties
	//////////////////////////////////////////////////////////////// @{

	/// Checks if the unit is a clean detector (no Lockdown, Optical Flare, etc.)
	bool canDetect() const;

	/// Returns the amount of HP displayed in-game. This is equal to
	/// (this->hitPoints + 255) / 256.
	uint32 getCurrentHpInGame() const;

	/// Returns the amount of shield points displayed in-game. This is equal to
	/// unit->shields / 256.
	uint32 getCurrentShieldsInGame() const;

	/// Returns the amount of HP + shields displayed in game. If the unit does not
	/// have shields enabled, this returns the amount of HP displayed.
	uint32 getCurrentLifeInGame() const;

	/// Returns the unit's ground weapon ID. If the unit is an unburrowed Lurker,
	/// returns WeaponId::None instead.
	uint8 getGroundWeapon() const;

	/// Returns the unit's air weapon ID. This is identical to reading the value
	/// from units_dat::AirWeapon[unit->id].
	uint8 getAirWeapon() const;

	/// Returns the total armor amount of this unit (with upgrades).
	/// To retrieve a unit's base armor amount, use units_dat::ArmorAmount[].
	uint8 getArmor() const;

	/// Returns the bonus armor amount of this unit (from upgrades).
	/// To retrieve a unit's base armor amount, use units_dat::ArmorAmount[].
	uint8 getArmorBonus() const;

	/// Returns the maximum energy amount of this unit (with upgrades).
	uint16 getMaxEnergy() const;

	/// Returns the maximum HP amount displayed in-game. This is equal to
	/// units_dat::MaxHitPoints[unit->id] >> 8 (minimum value is 1).
	uint32 getMaxHpInGame() const;
	uint32 getMaxHpRaw() const;

	/// Returns the maximum range of a weapon in pixels. The weapon is assumed to
	/// be attached to this unit for calculating upgrade effects.
	/// This is affected by the Weapon Range hook module.
	uint32 getMaxWeaponRange(uint8 weaponId) const;

	/// Retrieves the in-game name of this unit. In UMS maps, this returns custom
	/// names (if they exist).
	const char* getName() const;

	/// Retrieves the in-game name of the unit with @p unitId. In UMS maps, this
	/// returns custom unit names (if they exist).
	static const char* getName(uint16 unitId);

	/// Returns the race of the current unit.
	/// @return   RaceId::Terran, Zerg, Protoss, or Neutral.
	RaceId::Enum getRace() const;

	/// Returns the race of the unit with @p unitId.
	/// @return   RaceId::Terran, Zerg, Protoss, or Neutral.
	static RaceId::Enum getRace(uint16 unitId);

	/// Returns the unit's seek range (AKA target acquisition range) in matrices.
	/// This is affected by the Weapon Range hook module.
	uint8 getSeekRange() const;

	/// Returns the sight range of this unit (with upgrades).
	/// If @p isForSpellCasting is true, also factors in status effects.
	uint32 getSightRange(bool isForSpellCasting = false) const;

	///Return true if:
	/// * THE GATHERING cheat is enabled or
	/// * unit has at least @p energy available.
	bool hasEnergy(uint32 energy) const;

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
	bool isSubunit() const;

	/// Checks if the current unit is a factory (i.e. not nullptr and has the
	/// flag isFactory from GroupFlags set).
	bool isFactory() const;

	/// Checks if the unit is a spellcaster (has energy) and not a hallucination.
	bool isValidCaster() const;

	/// Checks if the unit is at move destination (0), or if the unit is
	/// unmovable (2) or not (1)
	uint32 getMovableState();

	//////////////////////////////////////////////////////////////// @}

	/// @name Positions and Dimensions
	//////////////////////////////////////////////////////////////// @{

	/// Returns the X position of the unit's sprite on the map.
	uint16 getX() const;
	/// Returns the Y position of the unit's sprite on the map.
	uint16 getY() const;

	/// Returns the X position of the tile that the unit's sprite is on.
	uint16 getTileX() const;
	/// Returns the Y position of the tile that the unit's sprite is on.
	uint16 getTileY() const;

	/// Returns the position of the left edge of the unit's collision box.
	int16 getLeft() const;
	/// Returns the position of the right edge of the unit's collision box.
	int16 getRight() const;
	/// Returns the position of the top edge of the unit's collision box.
	int16 getTop() const;
	/// Returns the position of the bottom edge of the unit's collision box.
	int16 getBottom() const;

	//////////////////////////////////////////////////////////////// @}

	/// @name Modifying Unit State
	//////////////////////////////////////////////////////////////// @{

	/// Sets the unit's HP, and also updates burning / bleeding graphics and unit
	/// strength data (used by the AI). HP is guaranteed not to overflow.
	void setHp(int32 hitPoints);
	void heal(int32 hitPoints, CUnit* source = nullptr);
	void healShields(int32 shields, CUnit* source = nullptr);

	/// Deals damage to this unit, using a specific weapons.dat ID.
	void damageWith(
		uint8 weaponId,              ///< weapons.dat ID to use.
		int32 damage = -1,			 ///< Amount of damage dealt to this unit. -1 = inherit from weaponId
		CUnit* attacker = nullptr,   ///< Attacking unit (for increasing kill count)
		uint8 attackingPlayer = -1,  ///< Attacking player (for increasing kill score)
		int8 direction = 0,          ///< Attacked direction (for shield flicker overlays)
		uint8 damageDivisor = 1      ///< Damage divisor (for splash damage / glave wurm calculations)
	);

	/// Deals damage to this unit, using a specific weapons.dat ID. Ignores flat armor
	void damageWithIgnoreFlatArmor(
		uint8 weaponId,              ///< weapons.dat ID to use.
		int32 damage = -1,			 ///< Amount of damage dealt to this unit. -1 = inherit from weaponId
		CUnit* attacker = nullptr,   ///< Attacking unit (for increasing kill count)
		uint8 attackingPlayer = -1,  ///< Attacking player (for increasing kill score)
		int8 direction = 0,          ///< Attacked direction (for shield flicker overlays)
		uint8 damageDivisor = 1      ///< Damage divisor (for splash damage / glave wurm calculations)
	);

	/// Deals damage directly to unit HP, killing it if possible.
	void damageHp(int32 damage, CUnit* attacker = nullptr,
		int32 attackingPlayer = -1, bool notify = true);

	/// Deals damage to shields and hp
	void inflictDamage(int32 damage, CUnit* attacker = nullptr,
		int32 attackingPlayer = -1, bool notify = true, bool shield_overlay = false, int direction = 0, bool ignoreDmg128Overflow = false);



	/// Reduces Defensive Matrix by @p amount, removing it if possible.
	void reduceDefensiveMatrixHp(int32 amount);

	/// Immediately kills the unit. To silently remove the unit, use:
	///   unit->userActionFlags |= 0x4;
	///   unit->remove();
	void remove();

	/// Removes the Lockdown effect from the unit.
	void removeLockdown();

	/// Removes the Maelstrom effect from the unit.
	void removeMaelstrom();

	/// Removes the Stasis Field effect from the unit.
	void removeStasisField();

	/// Removes the Acid Spores effect from the unit.
	void removeAcidSpores();

	/// Spend an amount of energy, if no THE GATHERING cheat.
	void spendUnitEnergy(uint32 energy);
	void addUnitEnergy(uint32 energy);
	void addUnitEnergy(uint32 energy, uint32 max);

	//////////////////////////////////////////////////////////////// @}

	/// @name Unit Orders
	//////////////////////////////////////////////////////////////// @{

	/// Issue the @p order to the unit, using the given @p target unit.
	void orderTo(uint8 orderId, CUnit* target = nullptr);

	/// Issue the @p order to the unit, using the given position as the target.
	void orderTo(uint8 orderId, uint16 x, uint16 y);

	//  Issue the @p order to the unit, without a target
	void orderComputerCL(uint8 orderId);

	//  Issue the @p order to the unit and specify if previous orders should be stopped
	void orderSimple(uint8 orderId, bool stopPreviousOrders);

	//  Issue the @p order to the unit, using the given @p target unit and specify if previous orders should be stopped
	void issueQueuedOrderTarget(uint8 orderId, CUnit* target, bool stopPreviousOrders);

	/// Causes the unit to become idle.
	void orderToIdle();

	/// Issues a new order to the unit.
	void order(uint8 orderId, uint16 x, uint16 y, CUnit* target, uint16 targetUnitId, bool stopPreviousOrders);

	/// Probably add the @p order to the orders queue
	void performAnotherOrder(uint8 orderId, int16 x, int16 y, CUnit* target, uint16 targetUnitId, uint32 unk = NULL);

	/// Used by several hooks, details not completely understood.
	void setSecondaryOrder(uint8 orderId);

	//////////////////////////////////////////////////////////////// @}

	/// @name Graphics and Animations
	/// Note: See CSprite::createOverlay() and CSprite::createTopOverlay() for
	///       creating image overlays.
	//////////////////////////////////////////////////////////////// @{

	/// Returns the overlay image of this unit's sprite (or its subunit's sprite)
	/// that matches the @p imageId. If the image cannot be found, returns nullptr.
	CImage* getOverlay(uint16 imageId) const;
	int32 maxShields() const;
	bool shieldsEnabled() const;
	bool shieldsEnabledRaw() const;

	/// Makes the unit's sprite play the specified Iscript animation entry.
	void playIscriptAnim(IscriptAnimation::Enum animation);

	/// Removes the first image overlay with an ID value between @p imageIdStart
	/// and @p imageIdEnd.
	void removeOverlay(uint32 imageIdStart, uint32 imageIdEnd);

	/// Removes the first image overlay with the given image ID.
	void removeOverlay(uint32 imageId);

	/// Get the faction (brood,tribe,squad...) color of current unit
	graphics::ColorId getFactionColor();

	/// Get the faction (brood,tribe,squad...) color of an unit
	static graphics::ColorId getFactionColor(CUnit* unit);

	/// Get the (possibly non-standard) color of current unit
	graphics::ColorId getColor();

	/// Get the (possibly non-standard) color of an unit
	static graphics::ColorId getColor(CUnit* unit);

	//////////////////////////////////////////////////////////////// @}

	/// @name Distances and Terrain
	//////////////////////////////////////////////////////////////// @{

	/// Returns the distance between this unit and the @p target, taking unit
	/// collision size in units.dat into account.
	/// Internally, this function uses scbw::getDistanceFast().
	uint32 getDistanceToTarget(CUnit* target) const;

	/// Checks whether this unit can reach the target position. This checks only
	/// for terrain, and does not consider obstacles (units and buildings).
	bool hasPathToPos(uint32 x, uint32 y) const;

	/// Checks whether this unit can reach the @p target unit. This checks only
	/// for terrain, and does not consider obstacles (units and buildings).
	bool hasPathToUnit(CUnit* target) const;

	///  Checks whether the @p target unit is within the @p range from the unit.
	bool isTargetWithinMinRange(CUnit* target, uint32 range);

	//////////////////////////////////////////////////////////////// @}

	/// @name DAT Requirements
	//////////////////////////////////////////////////////////////// @{

	/// Checks if the @p unit can build / train / morph into @p unitId. This checks:
	/// * If the unit is actually owned by the commanding @p playerId
	/// * Whether the unit is stunned / disabled.
	///
	/// If the unit can build / train / morph, returns 1. If the tech needs to be
	/// researched, returns -1. If the unit cannot use the tech at all, returns 0.
	int canMakeUnit(uint16 unitId, uint8 playerId) const;

	/// Checks the following:
	///  * If the unit is actually owned by the commanding @p playerId
	///  * If the unit's owning player has the @p techId researched
	///  * If the unit has enough energy (or energy cheat is enabled)
	///  * If the unit is not stunned / is a hallucination / is being built
	///  * If the unit meets the Use Tech Requirements (editable in FireGraft)
	///
	/// If the unit can use the tech, returns 1. If the tech needs to be
	/// researched, returns -1. If the unit cannot use the tech at all, returns 0.
	int canUseTech(uint8 techId, uint8 playerId) const;

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
	bool canAttackTarget(CUnit* target, bool checkVisibility = true) const;

	/// Makes the unit use the specified weapon to attack its current target unit
	/// stored in the CUnit::orderTarget.unit member. This does not affect the
	/// unit's weapon cooldown. The spawned weapon sprite obeys the weapon
	/// behavior properties in weapons.dat.
	/// Note: This function is affected by the Fire Weapon hook module.
	void fireWeapon(uint8 weaponId) const;

	/// Retrieves the unit pointer by @p index from the unit table (first unit is
	/// indexed at 1). If invalid, returns nullptr instead.
	static CUnit* getFromIndex(uint16 index);

	/// Returns the index of this unit in the unit table. First unit == index 1.
	uint16 getIndex() const;

	/// Returns the ID of the last player to own this unit. This is usually the
	/// same as CUnit::playerId, but if the unit belongs to a defeated player,
	/// this returns the correct player ID (instead of 11).
	/// @deprecated   Will be removed in v2.42. Use CUnit::isTargetEnemy().
	uint8 getLastOwnerId() const;

	/// Returns the loaded unit at @p index (value between 0-7). If no unit is
	/// loaded at the slot, returns nullptr instead.
	CUnit* getLoadedUnit(int index) const;

	/// Returns the first unit loaded in this unit. If this unit is not a
	/// transport, or has no units inside, this function returns nullptr instead.
	CUnit* getFirstLoadedUnit() const;

	/// Checks if this unit has other units loaded inside.
	bool hasLoadedUnit() const;

	/// Transfers ownership of this unit to @p playerId. This function is
	/// identical to Mind Control and the "Give Units To Player" trigger action.
	/// @return   true if successful, false otherwise.
	bool giveTo(uint8 playerId);

	/// Checks if the unit has an associated sprite and is not running the
	/// OrderId::Die order.
	bool isDead() const;

	/// Checks whether the @p target unit is an enemy of this unit.
	/// Internally, this calls scbw::isUnitEnemy().
	bool isTargetEnemy(CUnit* target) const;

	/// Checks whether this unit can be seen by @playerId (i.e. not covered by the
	/// fog-of-war and is detectable).
	bool isVisibleTo(uint8 playerId) const;

	/// Updates the unit's actual speed. This function should be called after
	/// changing any properties and status effects that affect movement speed.
	void updateSpeed();

	/// For Wyvern Land Grab
	uint8 getSpellCountdown() const;

	//////////////////////////////////////////////////////////////// @}

	void setSupplyProvided(uint8 amount);
	int32 getTotalHitPointsPlusShields();
	int32 CUnit::isUnitAINonBuilding();
};

C_ASSERT(sizeof(CFlingy) == 0x4c);
C_ASSERT(sizeof(CUnit) == sizeof(CUnitLayout));
//static_assert(sizeof(CUnit) == sizeof(CUnitLayout), "The size of the CUnit structure is invalid");
