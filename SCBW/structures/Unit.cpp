#include <cassert>
#include "Unit.h"
#include "../api.h"
#include "../enumerations.h"
#include "globals.h"
#include "hooks\limits\ccmu.h"
#include "hooks\weapons\weapon_damage.h"

#include "math_utils.h"
//-------- Unit stats and properties --------//

extern const u32 Func_CanDetect = 0x00403430; //unitHasNoStatusAilments
//Behavior can be modified with detector hook
bool Unit::canDetect() const
{
	assert(this);
	static u32 result;

	__asm {
		PUSHAD
		MOV EAX, this
		CALL Func_CanDetect
		MOV result, EAX
		POPAD
	}

	return result != 0;
}

u32 Unit::getCurrentHpInGame() const
{
	assert(this);
	return (this->hitPoints + 255) >> 8;
}

u32 Unit::getCurrentShieldsInGame() const
{
	assert(this);
	return this->shields >> 8;
}

//Identical to function @ 0x004026D0
u32 Unit::getCurrentLifeInGame() const
{
	assert(this);
	if (units_dat::ShieldsEnabled[this->id])
		return getCurrentHpInGame() + getCurrentShieldsInGame();
	else
		return getCurrentHpInGame();
}

s32 Unit::maxShields() const
{
	assert(this);
	return units_dat::MaxShieldPoints[this->id];
}

bool Unit::shieldsEnabled() const
{
	assert(this);
	return units_dat::ShieldsEnabled[this->id];
}

//Identical to function @ 0x00475AD0
u8 Unit::getGroundWeapon() const
{
	assert(this);
	if (this->id == UnitId::ZergLakizilisk && !(this->status & UnitStatus::Burrowed))
	{
		return WeaponId::None;
	}
	else
	{
		return units_dat::GroundWeapon[this->id];
	}
}

u8 Unit::getAirWeapon() const
{
	assert(this);
	return units_dat::AirWeapon[this->id];
}

u8 Unit::getArmor() const
{
	assert(this);
	return units_dat::ArmorAmount[this->id] + this->getArmorBonus();
}

extern const u32 Func_GetArmorBonus = 0x00453FC0;
u8 Unit::getArmorBonus() const
{
	assert(this);

	static u8 armorBonus;
	__asm {
		PUSHAD
		MOV EAX, this
		CALL Func_GetArmorBonus
		MOV armorBonus, AL
		POPAD
	}

	return armorBonus;
}

extern const u32 Func_GetMaxEnergy = 0x00491870;
u16 Unit::getMaxEnergy() const
{
	assert(this);

	static u16 result;

	__asm {
		PUSHAD
		MOV EAX, this
		CALL Func_GetMaxEnergy
		MOV result, AX
		POPAD
	}

	return result;
}

//Similar to function @ 0x00401400
u32 Unit::getMaxHpInGame() const
{
	assert(this);

	u32 maxHp = units_dat::MaxHitPoints[this->id];
	if (maxHp == 0)
	{
		maxHp = this->getCurrentHpInGame();
		if (maxHp == 0)
		{
			maxHp = 1;
		}
	}
	else
	{
		maxHp = maxHp >> 8;
	}
	return maxHp;
}

u32 Unit::getMaxShieldsInGame() const
{
	assert(this);

	if (!this->shieldsEnabled())
	{
		return 0;
	}

	u32 maxShields = units_dat::MaxShieldPoints[this->id];
	if (maxShields == 0)
	{
		maxShields = this->getCurrentShieldsInGame();
		if (maxShields == 0)
		{
			maxShields = 1;
		}
	}
	return maxShields;
}

u32 Unit::getMaxHpRaw() const
{
	assert(this);
	return units_dat::MaxHitPoints[this->id];
}

u32 Unit::getMaxShieldsRaw() const
{
	assert(this);
	return units_dat::MaxShieldPoints[this->id] * 256;
}

u8 Unit::getSpellCountdown() const
{
	assert(this);
	if (this->id == UnitId::TerranWyvern)
	{
		return this->groundWeaponCooldown;
	}
	return this->spellCooldown;
}

extern const u32 Func_GetMaxWeaponRange = 0x00475870;
u32 Unit::getMaxWeaponRange(u8 weaponId) const
{
	assert(this);

	if (weaponId == WeaponId::None)
	{
		return 0;
	}

	assert(weaponId < WEAPON_TYPE_COUNT);

	static u32 maxWeaponRange;
	__asm {
		PUSHAD
		MOV BL, weaponId
		MOV EAX, this
		CALL Func_GetMaxWeaponRange
		MOV maxWeaponRange, EAX
		POPAD
	}
	return maxWeaponRange;
}

const char* Unit::getName() const
{
	assert(this);
	return getName(this->id);
}
#include "SCBW/stat_txt_spec.h"
//Based on function @ 0x0047B090
const char* Unit::getName(u16 unitId)
{
	assert(unitId < UNIT_TYPE_COUNT);

	if (units_dat::MapStringId[unitId])
	{
		return mapStringTbl->getString(units_dat::MapStringId[unitId]);
	}
	else
	{
		return statTxtTblNew->getString(unitId + 1);
	}
}

RaceId::Enum Unit::getRace() const
{
	assert(this);
	return getRace(this->id);
}

//Based on function at 0x0047B150
RaceId::Enum Unit::getRace(u16 unitId)
{
	assert(unitId < UNIT_TYPE_COUNT);

	if (units_dat::GroupFlags[unitId].isZerg)
	{
		return RaceId::Zerg;
	}
	else if (units_dat::GroupFlags[unitId].isProtoss)
	{
		return RaceId::Protoss;
	}
	else if (units_dat::GroupFlags[unitId].isTerran)
	{
		return RaceId::Terran;
	}
	else
	{
		return RaceId::Neutral;
	}
}

extern const u32 Func_GetSeekRange = 0x00476000;
u8 Unit::getSeekRange() const
{
	assert(this);

	static u8 seekRange;

	__asm {
		PUSHAD
		MOV EDX, this
		CALL Func_GetSeekRange
		MOV seekRange, AL
		POPAD
	}

	return seekRange;
}

extern const u32 Func_GetSightRange = 0x004E5B40;
u32 Unit::getSightRange(bool isForSpellCasting) const
{
	assert(this);

	static u32 sightRange;
	b32 ignoreStatusEffects = (isForSpellCasting ? 0 : 1);

	__asm {
		PUSHAD
		PUSH ignoreStatusEffects
		MOV EDX, this
		CALL Func_GetSightRange
		MOV sightRange, EAX
		POPAD
	}

	return sightRange;
}

//Similar to function Unit__HasEnergy @ 0x00491480
//The original function returned 0 when got energy or code
bool Unit::hasEnergy(u32 energy) const
{

	if (
		scbw::isCheatEnabled(CheatFlags::TheGathering) ||
		this->energy >= energy
		)
		return true;
	else
		return false;

}

//Identical to function Unit__SpendEnergy @ 0x00476180
bool Unit::hasWeapon() const
{
	assert(this);

	if (this->getGroundWeapon() != WeaponId::None
		|| this->getAirWeapon() != WeaponId::None)
		return true;

	if (this->id == UnitId::ProtossSolarion)
	{
		if ((this->carrier.inHangarCount + this->carrier.outHangarCount) > 0)
		{
			return true;
		}
	}

	if (this->id == UnitId::ProtossAccantor)
	{
		if (this->carrier.inHangarCount > 0)
		{
			return true;
		}
	}

	if (this->subunit)
	{
		if (this->subunit->getGroundWeapon() != WeaponId::None
			|| this->subunit->getAirWeapon() != WeaponId::None)
		{
			return true;
		}
	}

	return false;
}

//Identical to function @ 0x004020B0
bool Unit::isFrozen() const
{
	assert(this);
	return
		this->status & UnitStatus::DoodadStatesThing
		|| this->lockdownTimer
		|| this->stasisTimer
		|| this->maelstromTimer;
}

extern const u32 Func_IsRemorphingBuilding = 0x0045CD00;
bool Unit::isRemorphingBuilding() const
{
	static b32 result;
	assert(this);

	__asm {
		PUSHAD
		MOV EAX, this
		CALL Func_IsRemorphingBuilding
		MOV result, EAX
		POPAD
	}

	return result != 0;
}

//Identical to function @ 0x00401D40
bool Unit::isSubunit() const
{
	return (this && units_dat::BaseProperty[this->id] & UnitProperty::Subunit);
}

// Equivalent to function @ 0x0047B2E0 until you change .dat files.
bool Unit::isFactory() const
{
	return (this && units_dat::GroupFlags[this->id].isFactory);
}

//Identical to function @ 0x00401210
bool Unit::isValidCaster() const
{
	assert(this);
	return units_dat::BaseProperty[this->id] & UnitProperty::Spellcaster
		&& !(this->status & UnitStatus::IsHallucination);
}

//Identical to function @ 0x00401DC0
//Returns:	0 - Unit has not reached destination
//					1 - Unit has reached destination
//					2 - Unit cannot move
u32 Unit::getMovableState()
{
	if (this->moveTarget.pt != this->sprite->position)
		return 0;
	else if (!(this->status & UnitStatus::Unmovable))
		return 1;
	else
		return 2;
}

//-------- Positions and dimensions --------//

u16 Unit::getX() const
{
	assert(this);
	assert(this->sprite);
	return this->sprite->position.x;
};

u16 Unit::getY() const
{
	assert(this);
	assert(this->sprite);
	return this->sprite->position.y;
};

u16 Unit::getTileX() const
{
	assert(this);
	return this->getX() / 32;
};

u16 Unit::getTileY() const
{
	assert(this);
	return this->getY() / 32;
};

s16 Unit::getLeft() const
{
	assert(this);
	return this->getX() - units_dat::UnitBounds[this->id].left;
}

s16 Unit::getRight() const
{
	assert(this);
	return this->getX() + units_dat::UnitBounds[this->id].right;
}

s16 Unit::getTop() const
{
	assert(this);
	return this->getY() - units_dat::UnitBounds[this->id].top;
}

s16 Unit::getBottom() const
{
	assert(this);
	return this->getY() + units_dat::UnitBounds[this->id].bottom;
}

//-------- Modifying unit state --------//

const u32 Func_SetUnitHp = 0x00467340;
void Unit::setHp(s32 hitPoints)
{
	assert(this);

	__asm {
		PUSHAD
		MOV EAX, this
		MOV ECX, hitPoints
		CALL Func_SetUnitHp
		POPAD
	}
}

void Unit::heal(s32 hitPoints, Unit* source, bool restore)
{
	assert(this);
	if (this->hitPoints == this->getMaxHpRaw()) // skip healing if max hp already
	{
		return;
	}

	// Cohort - Servile Vigor
	if (id == UnitId::SharedCohort && this != source)
	{
		hitPoints *= 2;
	}

	setHp(std::min((u32)this->hitPoints + hitPoints, this->getMaxHpRaw()));

	// If heals to full clear debuffs
	if (this->hitPoints == this->getMaxHpRaw() && restore)
	{
		scbw::restoreFunc(this);
	}
}

void Unit::healShields(s32 shields, Unit* source, bool restore)
{
	assert(this);
	if (this->shields == this->getMaxShieldsRaw()) // skip healing if max shields already
	{
		return;
	}

	this->shields = std::min((u32)this->shields + shields, this->getMaxShieldsRaw());
	helper::updateUnitStrength(this);

	// If heals to full clear debuffs
	if (this->shields == this->getMaxShieldsRaw() && restore)
	{
		scbw::restoreFunc(this);
	}
}

void Unit::healOverflow(s32 amount, Unit* source, bool restore)
{
	s32 shields = this->hitPoints + amount;
	shields -= this->getMaxHpRaw();
	this->heal(amount, source, restore);
	if (shields > 0)
	{
		this->healShields(shields, source, restore);
	}
}

void Unit::healEnergy(s32 energy, Unit* source)
{
	assert(this);
	if (this->energy == helper::getUnitMaxEnergy(this) * 256)
	{
		return;
	}
	this->energy = std::min(this->energy + energy, helper::getUnitMaxEnergy(this) * 256);
}

const u32 Func_DoWeaponDamage = 0x00479930; // Note: hooked in hooks/weapons/weapon_damage files
void Unit::damageWith(u8 weaponId, s32 damage, Unit* attacker, u8 attackingPlayer, s8 direction, u8 damageDivisor)
{
	assert(this);
	assert(weaponId < WEAPON_TYPE_COUNT);
//	assert(damageDivisor != 0);
	if (damage < 0)
	{
		damage = weapons_dat::DamageAmount[weaponId] * 256;
	}
	__asm
	{
		PUSHAD
		MOV EDI, this
		MOV EAX, damage
		PUSH attackingPlayer
		PUSH attacker
		PUSH direction
		PUSH damageDivisor
		PUSH weaponId
		CALL Func_DoWeaponDamage
		POPAD
	}
}

void Unit::damageWithCustom(
	s32 damage,
	u8 damageType,
	Unit* attacker,
	u8 attackingPlayerId,
	s8 direction,
	u8 damageDivisor,
	bool ignoreFlatArmor
)
{
	assert(this);

	hooks::weaponDamageCustom(
		damage,
		this,
		damageType,
		attacker,
		attackingPlayerId,
		direction,
		damageDivisor,
		ignoreFlatArmor
	);
}

void Unit::inflictDamage(s32 dmg, Unit* attacker, s32 attackingPlayer, bool notify, bool shield_overlay, s32 direction,
	bool ignoreDmg128Overflow)
{
	UnitExt* unitExt = globals.GetUnitExt(this);
	auto damage = dmg;
	s32 shieldReduceAmount = 0;
	if (unitExt->timers[TimerGodsMadeFlesh])
	{
		return;
	}
	if (units_dat::ShieldsEnabled[this->id])
	{
		shieldReduceAmount = std::min(damage, this->shields);
		damage -= shieldReduceAmount;
	}
	if (shieldReduceAmount == 0 && damage < 128 && !ignoreDmg128Overflow)
	{
		damage = 128;
	}
	if (shieldReduceAmount != 0)
	{
		this->shields -= shieldReduceAmount;
	}
	if (shield_overlay && units_dat::ShieldsEnabled[this->id] && this->shields != 0)
	{
		const LO_Header* shield_lo = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayShieldPtr,
			this->sprite->mainGraphic->id));

		u32 frameAngle = (direction - 124) / 8 % 32;
		Point8 offset = shield_lo->getOffset(this->sprite->mainGraphic->direction, frameAngle);
		this->sprite->createOverlay(ImageId::ShieldOverlay, offset.x, offset.y, frameAngle);
	}
	damageHp(damage, attacker, attackingPlayer, notify);
}

const u32 Func_DamageUnitHp = 0x004797B0;
void Unit::damageHp(s32 damage, Unit* attacker, s32 attackingPlayer, bool notify)
{
	assert(this);

	// Logging
	auto targetId = this->id;
	auto targetPlayerId = this->playerId;
	if (attacker != nullptr)
	{
		GPTP::logger << "damageHp(damage (" << damage / 256 << "), attackerId (" << attacker->id << "), attackingPlayerId (" << attackingPlayer << "), notify (" << notify
			<< "), target (" << targetId << "), targetPlayerId (" << targetPlayerId << "))" << std::endl;
	}

	u32 notify_ = notify ? 1 : 0;

	if (attacker)
	{
		UnitExt* attackerExt = globals.GetUnitExt(attacker);
		attackerExt->damageDealtTimer = 0;
	}

	UnitExt* unitExt = globals.GetUnitExt(this);
	unitExt->damageReceiveTimer = 0;

	__asm {
		PUSHAD
		PUSH notify_
		PUSH attackingPlayer
		PUSH attacker
		MOV EAX, damage
		MOV ECX, this
		CALL Func_DamageUnitHp
		POPAD
	}
}

//Logically equivalent to function @ 0x00454ED0
void Unit::reduceDefensiveMatrixHp(s32 amount)
{

	assert(this);

	if (this->defensiveMatrixHp > amount)
		this->defensiveMatrixHp -= amount;
	else
	{

		Image* current_image;

		this->defensiveMatrixHp = 0;
		this->defensiveMatrixTimer = 0;

		this->removeOverlay(ImageId::DefensiveMatrixFront_Small,
			ImageId::DefensiveMatrixFront_Large);

		current_image = this->sprite->images.head;

		while (current_image != nullptr)
		{

			if (current_image->id < ImageId::DefensiveMatrixBack_Small
				|| current_image->id > ImageId::DefensiveMatrixBack_Large)
			{
				current_image = current_image->link.next;
			}
			else
			{
				current_image->free();
				current_image = nullptr;	//to exit loop
			}

		}

	}

	if (this->defensiveMatrixTimer != 0 && !(this->status & UnitStatus::Burrowed))
	{
		Unit* unit = this->subunit ? this->subunit : this;
//		getUnitOverlayAdjustment code was hardcoded into function in original code)
		unit->sprite->createTopOverlay(scbw::getUnitOverlayAdjustment(unit) + ImageId::DefensiveMatrixHit_Small);
	}

}

const u32 Func_RemoveUnit = 0x00475710; //AKA orders_SelfDestructing()
void Unit::remove()
{

	assert(this);

	__asm {
		PUSHAD
		MOV EAX, this
		CALL Func_RemoveUnit
		POPAD
	}

}

const u32 Func_RemoveLockdown = 0x00454D90;
void Unit::removeLockdown()
{

	assert(this);

	__asm {
		PUSHAD
		MOV ESI, this
		CALL Func_RemoveLockdown
		POPAD
	}

}

const u32 Func_RemoveMaelstrom = 0x00454D20;
void Unit::removeMaelstrom()
{

	assert(this);

	__asm {
		PUSHAD
		MOV ESI, this
		CALL Func_RemoveMaelstrom
		POPAD
	}

}

const u32 Func_RemoveStasisField = 0x004F62D0;
void Unit::removeStasisField()
{

	assert(this);

	__asm {
		PUSHAD
		MOV ESI, this
		CALL Func_RemoveStasisField
		POPAD
	}

}

const u32 Func_RemoveAcidSpores = 0x004F4160;
void Unit::removeAcidSpores()
{

	assert(this);

	__asm {
		PUSHAD
		MOV EDX, this
		CALL Func_RemoveAcidSpores
		POPAD
	}

}

///Identical to Unit__SpendEnergy @ 0x00491460
///May underflow if alone
void Unit::spendUnitEnergy(u32 energySpent)
{
	UnitExt* unitExt = globals.GetUnitExt(this);
	if (!(*CHEAT_STATE & CheatFlags::TheGathering))
	{
		if (this->energy > energySpent)
		{
			this->energy -= energySpent;
		}
		else
		{
			this->energy = 0;
		}
	}
	// TODO: fix Entropy Gauntlets bug
	if (unitExt->values[ValueEntropyGauntlets] > 0)
	{
		this->inflictDamage(energySpent * unitExt->values[ValueEntropyGauntlets] * 0.1, unitExt->units[UnitDecaySavant], -1, false, true, 0, true); // bugged if energySpent > this->energy; will damage for more than it should
	}
}

void Unit::addUnitEnergy(u32 energy)
{
	this->energy = MathUtils::clamp<u32>(this->energy + energy, 0, this->getMaxEnergy());
}

void Unit::addUnitEnergy(u32 energy, u32 energyMax)
{
	this->energy = MathUtils::clamp<u32>(this->energy + energy, 0, energyMax);
}

// Identical to getTotalHitPointsPlusShields @ 00402710
s32 Unit::getTotalHitPointsPlusShields()
{
	assert(this);
	s32 id = this->id;
	s32 shields = 0;
	s32 hp = units_dat::MaxHitPoints[id] >> 8;
	if (units_dat::ShieldsEnabled[id])
	{
		shields = units_dat::MaxShieldPoints[id];
	}
	if (!hp)
	{
		hp = (this->hitPoints + 255) >> 8;
		if (!hp)
		{
			hp = 1;
		}
	}
	return shields + hp;
}

// Identical to isAIControllerNotABuilding @ 004381D0
s32 Unit::isUnitAINonBuilding()
{
	assert(this);
	UnitAi* ai = (UnitAi*)this->pAI;
	MilitaryAi* mai = (MilitaryAi*)ai;
	s32 result = 1;
	if (!ai
		|| ai->type != 4
		|| mai->parent != this
		|| units_dat::BaseProperty[this->id] & UnitProperty::Building)
	{
		result = 0;
	}
	return result;
}

//-------- Unit orders --------//

//Logically equivalent to function @ 0x004752B0
//ESI = this
//EAX = target
//[ebp+08] = orderId
void Unit::orderTo(u8 orderId, Unit* target)
{
	assert(this);

	this->userActionFlags |= 0x1;
	if (target)
		this->order(orderId, target->getX(), target->getY(), target, UnitId::None, true);
	else
		this->order(orderId, 0, 0, target, UnitId::None, true);

	prepareForNextOrder(this);
}

//Logically equivalent to function @ 0x00475260
void Unit::orderTo(u8 orderId, u16 x, u16 y)
{
	assert(this);

	this->userActionFlags |= 0x1;
	this->order(orderId, x, y, nullptr, UnitId::None, true);
	prepareForNextOrder(this);
}

//Logically equivalent to function orderComputer_cl @ 0x00475310
//used in hooks so not doing assert, mind it if used in mods
void Unit::orderComputerClear(u8 orderId)
{
	this->userActionFlags |= 0x1;
	this->order(orderId, 0, 0, nullptr, UnitId::None, true);
	prepareForNextOrder(this);
}

//Equivalent to 00474D60 __order function
void Unit::orderSimple(u8 orderId, bool stopPreviousOrders)
{
	this->order(orderId, 0, 0, nullptr, UnitId::None, stopPreviousOrders);
}

//Equivalent to 00474D10 issueQueuedOrderTarget function
void Unit::issueQueuedOrderTarget(u8 orderId, Unit* target, bool stopPreviousOrders)
{
	if (target == nullptr)
		this->order(orderId, 0, 0, nullptr, UnitId::None, stopPreviousOrders);
	else
		this->order(orderId, target->sprite->position.x, target->sprite->position.y, target, UnitId::None, stopPreviousOrders);
}

//Logically equivalent to function toIdle @ 0x004753A0
void Unit::orderToIdle()
{

	assert(this);

	if (this->orderQueueHead)
	{
		this->userActionFlags |= 1;
		prepareForNextOrder(this);
	}
	else
	{
		if (this->pAI)
			this->orderTo(OrderId::ComputerAI);
		else
			this->orderTo(units_dat::ReturnToIdleOrder[this->id]);
	}

}

const u32 Func_Order = 0x00474810;
void Unit::order(u8 orderId, u16 x, u16 y, Unit* target, u16 targetUnitId, bool stopPreviousOrders)
{
	assert(this);
	static Point16 pos;
	static u32 targetUnitId_;
	pos.x = x, pos.y = y;
	targetUnitId_ = targetUnitId;

	__asm {
		PUSHAD
		PUSH targetUnitId_
		PUSH target
		PUSH pos
		MOVZX EAX, stopPreviousOrders
		MOV CL, orderId
		MOV EDX, this
		CALL Func_Order
		POPAD
	}
}

const u32 Func_PerformAnotherOrder = 0x004745F0;
void Unit::performAnotherOrder(u8 orderId, s16 x, s16 y, Unit* target, u16 targetUnitId, u32 unk)
{

	static Point16 pos;
	pos.x = x; pos.y = y;

	__asm {
		PUSHAD
		PUSH target
		PUSH pos
		MOV BL, orderId
		MOVZX EDX, targetUnitId
		MOV ESI, this
		MOV EDI, unk
		CALL Func_PerformAnotherOrder
		POPAD
	}

}

//Identical to @ 0x004743D0
void Unit::setSecondaryOrder(u8 orderId)
{
	assert(this);
	if (this->secondaryOrderId == orderId)
		return;
	this->secondaryOrderId = orderId;
	this->secondaryOrderPos.x = 0;
	this->secondaryOrderPos.y = 0;
	this->currentBuildUnit = nullptr;
	this->secondaryOrderState = 0;
}

//-------- Graphics and animations --------//

//Similar to function @ 0x004E5C90
Image* Unit::getOverlay(u16 imageId) const
{
	assert(this);

	Image* image = this->sprite->getOverlay(imageId);
	if (!image && this->subunit)
		image = this->subunit->sprite->getOverlay(imageId);

	return image;
}

void Unit::playIscriptAnim(IscriptAnimation::Enum animation)
{
	assert(this);
	this->sprite->playIscriptAnim(animation);
}

void Unit::removeOverlay(u32 imageIdStart, u32 imageIdEnd)
{
	assert(this);

	this->sprite->removeOverlay(imageIdStart, imageIdEnd);
	if (this->subunit)
		this->subunit->sprite->removeOverlay(imageIdStart, imageIdEnd);
}

void Unit::removeOverlay(u32 imageId)
{
	assert(this);

	this->sprite->removeOverlay(imageId);
	if (this->subunit)
		this->subunit->sprite->removeOverlay(imageId);
}

// ----------------------------------------------------------------------------//
// Return the proper ColorId to use an unit color with graphics function
// ----------------------------------------------------------------------------//

graphics::ColorId Unit::getFactionColor()
{
	assert(this);
	return getFactionColor(this);
}

graphics::ColorId Unit::getFactionColor(Unit* unit)
{

	//store a playerId until the end where it will properly store a ColorId
	graphics::ColorId return_value;

	if (unit != nullptr)
	{

		if (unit->sprite != nullptr)
			// note: using the sprite playerId to preserve recruitable units color
			return_value = unit->sprite->playerId;
		else
			// note: using player color for spriteless units (if that exist)
			return_value = unit->playerId;

		//for normal players, apply colors switching
		if (unit->playerId < PLAYABLE_PLAYER_COUNT)
		{

			// get the proper playerId in multiplayer game (don't break campaigns/UMS)
			return_value = factionsColorsOrdering[return_value];

			//fix BROWN to GREEN color swap in Desert map
			if (*CURRENT_TILESET == TilesetType::Desert && return_value == 5)
				return_value = 8;

			//fix WHITE to GREEN color swap in Ice map
			if (*CURRENT_TILESET == TilesetType::Ice && return_value == 6)
				return_value = 8;

		}

		if (return_value < PLAYER_COUNT)
			return_value = graphics::standardColors[return_value];
		else
			return_value = graphics::BLACK; // will return 0/black if abnormal playerId

	}
	else
		return_value = graphics::BLACK; // will return 0/black if null pointer is the argument

	return return_value;

}

graphics::ColorId Unit::getColor()
{
	assert(this);
	return getColor(this);
}

graphics::ColorId Unit::getColor(Unit* unit)
{

	//getColourID internal function
	const u32 Func_getColourID = 0x0049B0E0;

	graphics::ColorId return_value;

	if (unit != nullptr)
	{

		if (unit->sprite != nullptr)
			// note: using the sprite playerId to preserve recruitable units color
			return_value = unit->sprite->playerId;
		else
			// note: using player color for spriteless units (if that exist)
			return_value = unit->playerId;

		if (return_value < PLAYABLE_PLAYER_COUNT)
		{

			__asm {
				PUSHAD
				MOV AL, return_value
				CALL Func_getColourID
				MOV return_value, AL
				POPAD
			}

			return_value = playersColors[return_value];

		}
		else
			return_value = graphics::BLACK; // will return 0/black if abnormal playerId

	}
	else
		return_value = graphics::BLACK; // will return 0/black if unit is nullptr

	return return_value;

}

//-------- Distances and terrain --------//

u32 Unit::getDistanceToTarget(Unit* target) const
{

	assert(this);
	assert(target);

	Unit* unit = (Unit*)this;

	if (this->isSubunit())
		unit = this->subunit; // Current unit is a turret, so use it's base instead

	s32 dx = unit->getLeft() - target->getRight() - 1;
	if (dx < 0)
	{
		dx = target->getLeft() - unit->getRight() - 1;
		if (dx < 0)
			dx = 0;
	}

	s32 dy = unit->getTop() - target->getBottom() - 1;
	if (dy < 0)
	{
		dy = target->getTop() - unit->getBottom() - 1;
		if (dy < 0)
			dy = 0;
	}

	return scbw::getDistanceFast(0, 0, dx, dy);
}

const u32 Func_HasPathToPos = 0x0049CB60; //AKA unitHasPathToDest()
bool Unit::hasPathToPos(u32 x, u32 y) const
{
	assert(this);
	u32 result;

	__asm {
		PUSHAD
		PUSH y
		PUSH x
		MOV EAX, this
		CALL Func_HasPathToPos
		MOV result, EAX
		POPAD
	}

	return result != 0;
}

const u32 Func_HasPathToTarget = 0x0049CBB0; //AKA unitHasPathToUnit()
bool Unit::hasPathToUnit(Unit* target) const
{
	assert(this);
	assert(target);
	u32 result;

	__asm {
		PUSHAD
		MOV EAX, this
		MOV ECX, target
		CALL Func_HasPathToTarget
		MOV result, EAX
		POPAD
	}

	return result != 0;
}

const u32 Func_IsTargetWithinMinRange = 0x00430F10;
bool Unit::isTargetWithinMinRange(Unit* target, u32 range)
{

	static b32 bPreResult;

	__asm {
		PUSHAD
		PUSH target
		PUSH range
		MOV ECX, this
		CALL Func_IsTargetWithinMinRange
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}

//-------- DAT Requirements --------//

const u32 Func_CanMakeUnit = 0x0046E1C0;
s32 Unit::canMakeUnit(u16 unitId, u8 playerId) const
{
	assert(this);
	s32 playerId_ = playerId;
	static s32 result;

	__asm {
		PUSHAD
		PUSH playerId_
		MOV AX, unitId
		MOV ESI, this
		CALL Func_CanMakeUnit
		MOV result, EAX
		POPAD
	}

	return result;
}

const u32 Func_CanUseTech = 0x0046DD80;
s32 Unit::canUseTech(u8 techId, u8 playerId) const
{
	assert(this);
	s32 playerId_ = playerId;
	u32 _techId = techId;
	static s32 result;

	__asm {
		PUSHAD
		PUSH playerId_
		MOV EDI, _techId
		MOV EAX, this
		CALL Func_CanUseTech
		MOV result, EAX
		POPAD
	}

	return result;
}

//-------- Utility methods --------//

const u32 Func_CanUnitAttackTarget = 0x00476730;
bool Unit::canAttackTarget(Unit* target, bool checkVisibility) const
{
	assert(this);
	assert(target);

	u32 _checkVisibility = checkVisibility ? 1 : 0;
	static u32 result;

	__asm {
		PUSHAD
		PUSH _checkVisibility
		MOV EBX, target
		MOV ESI, this
		CALL Func_CanUnitAttackTarget
		MOV result, EAX
		POPAD
	}

	return result != 0;
}

extern const u32 Func_FireUnitWeapon = 0x00479C90;
void Unit::fireWeapon(u8 weaponId) const
{

	assert(this);

	static u32 weaponId_;

	weaponId_ = weaponId;

	__asm {
		PUSHAD
		PUSH weaponId_
		MOV ESI, this
		CALL Func_FireUnitWeapon
		POPAD
	}

}



Unit* Unit::getFromIndex(u16 index)
{
	if (globals.extended_unit_table == 1)
	{
		if (1 <= index && index <= UNIT_ARRAY_LENGTH /* NEW_ARRAY_LENGTH*/)
		{
			return (Unit*)&new_unit_table.units[index - 1];
		}
		return nullptr;
	}
	if (1 <= index && index <= UNIT_ARRAY_LENGTH)
	{
		return &unitTable[index - 1];
	}
	return nullptr;
}

u16 Unit::getIndex() const
{
	assert(this);
	if (globals.extended_unit_table == 1)
	{
		return this - (Unit*)&new_unit_table.units[0] + 1;
	}
	return this - unitTable + 1;
}

u8 Unit::getLastOwnerId() const
{
	assert(this);
	assert(this->sprite);
	if (this->playerId == 11) // The owner has left the game
	{
		return this->sprite->playerId;
	}
	else
	{
		return this->playerId;
	}
}

Unit* Unit::getLoadedUnit(s32 index) const
{
	assert(0 <= index && index < 8);

	Unit* loadedUnit = Unit::getFromIndex(this->loadedUnit[index].index);
	if (loadedUnit && loadedUnit->sprite
		&& !(loadedUnit->mainOrderId == OrderId::Die && loadedUnit->mainOrderState == 1)
		&& loadedUnit->targetOrderSpecial == this->loadedUnit[index].unitId)
	{
		return loadedUnit;
	}
	return nullptr;
}

//Logically equivalent to function @ 0x004E6C90
Unit* Unit::getFirstLoadedUnit() const
{
	assert(this);

	for (s32 i = 0; i < units_dat::SpaceProvided[this->id]; ++i)
	{
		Unit* firstLoadedUnit = getLoadedUnit(i);
		if (firstLoadedUnit)
			return firstLoadedUnit;
	}

	return nullptr;
}

//Logically equivalent to function @ 0x004E7110
bool Unit::hasLoadedUnit() const
{
	assert(this);
	for (s32 i = 0; i < 8; ++i)
	{
		if (!getLoadedUnit(i))
		{
			return true;
		}
	}
	return false;
}

namespace
{
	typedef u32(__stdcall* GiveUnitToPlayerFunc)(Unit* unit, u32 playerId);
	GiveUnitToPlayerFunc const giveUnitToPlayer = (GiveUnitToPlayerFunc)0x004C8040;
}

bool Unit::giveTo(u8 playerId)
{
	assert(this);
	assert(playerId < 12);

	return giveUnitToPlayer(this, playerId) != 0;
}

//Identical to function @ 0x00475A50
bool Unit::isDead() const
{
	assert(this);
	return (!this->sprite) || (this->mainOrderId == OrderId::Die);
}

//Probably equivalent to function @ 0x0047B910
bool Unit::isTargetEnemy(Unit* target) const
{
	assert(this);
	return scbw::isUnitEnemy(this->playerId, target);
}

bool Unit::isVisibleTo(u8 playerId) const
{
	assert(this);
	return (this->visibilityStatus & (1 << playerId)) != 0;
}

const u32 Func_UpdateSpeed = 0x00454310;
void Unit::updateSpeed()
{
	assert(this);

	__asm {
		PUSHAD
		MOV EAX, this
		CALL Func_UpdateSpeed
		POPAD
	}
}

void Unit::setSupplyProvided(u8 amount) // overwrites previous supply provided
{
	UnitExt* unitExt = globals.GetUnitExt(this);

	u32 race = this->getRace();
	u32 previous_supply = unitExt->values[ValueSupplyProvided];
	if (previous_supply)
		raceSupply[RaceId::Terran].provided[this->playerId] -= previous_supply;
	raceSupply[RaceId::Terran].provided[this->playerId] += amount;
	unitExt->values[ValueSupplyProvided] = amount;
}