#include <cassert>
#include "Unit.h"
#include "../api.h"
#include "../enumerations.h"
#include "globals.h"
#include "hooks\limits\ccmu.h"
#include "hooks\weapons\weapon_damage.h"
#include "hooks\load_unload_proc.h"

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

u32 Unit::get_current_hp_ingame() const
{
	assert(this);
	return (this->hitpoints + 255) >> 8;
}

u32 Unit::getCurrentShieldsInGame() const
{
	assert(this);
	return this->shields >> 8;
}

//Identical to function @ 0x004026D0
u32 Unit::get_current_life_ingame() const
{
	assert(this);
	if (units_dat::ShieldsEnabled[this->id])
		return get_current_hp_ingame() + getCurrentShieldsInGame();
	else
		return get_current_hp_ingame();
}

s32 Unit::max_shields() const
{
	assert(this);
	return units_dat::MaxShieldPoints[this->id];
}

bool Unit::shields_enabled() const
{
	assert(this);
	return units_dat::ShieldsEnabled[this->id];
}

//Identical to function @ 0x00475AD0
u8 Unit::get_ground_weapon() const
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

u8 Unit::get_air_weapon() const
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
u16 Unit::get_max_energy() const
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
u32 Unit::get_max_hp_ingame() const
{
	assert(this);

	u32 maxHp = units_dat::MaxHitPoints[this->id];
	if (maxHp == 0)
	{
		maxHp = this->get_current_hp_ingame();
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

u32 Unit::get_max_shields_in_game() const
{
	assert(this);

	if (!this->shields_enabled())
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

u32 Unit::get_max_hp_raw() const
{
	assert(this);
	return units_dat::MaxHitPoints[this->id];
}

u32 Unit::get_max_shields_raw() const
{
	assert(this);
	return units_dat::MaxShieldPoints[this->id] * 256;
}

u8 Unit::getSpellCountdown() const
{
	assert(this);
	if (this->id == UnitId::TerranWyvern)
	{
		return this->ground_weapon_cooldown;
	}
	return this->spell_cooldown;
}

extern const u32 Func_GetMaxWeaponRange = 0x00475870;
u32 Unit::get_max_weapon_range(u8 weaponId) const
{
	assert(this);

	if (weaponId == WeaponId::None)
	{
		return 0;
	}

	assert(weaponId < WEAPON_TYPE_COUNT);

	static u32 maxWeaponRange;
	__asm
	{
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
const char* Unit::getName(u16 unit_id)
{
	assert(unit_id < UNIT_TYPE_COUNT);

	if (units_dat::MapStringId[unit_id])
	{
		return mapStringTbl->get_string(units_dat::MapStringId[unit_id]);
	}
	else
	{
		return statTxtTblNew->get_string(unit_id + 1);
	}
}

RaceId::Enum Unit::get_race() const
{
	assert(this);
	return get_race(this->id);
}

//Based on function at 0x0047B150
RaceId::Enum Unit::get_race(u16 unit_id)
{
	assert(unit_id < UNIT_TYPE_COUNT);

	if (units_dat::group_flags[unit_id].is_zerg)
	{
		return RaceId::Zerg;
	}
	else if (units_dat::group_flags[unit_id].is_protoss)
	{
		return RaceId::Protoss;
	}
	else if (units_dat::group_flags[unit_id].is_terran)
	{
		return RaceId::Terran;
	}
	else
	{
		return RaceId::Neutral;
	}
}

extern const u32 Func_GetSeekRange = 0x00476000;
u8 Unit::get_seek_range() const
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
u32 Unit::get_sight_range(bool isForSpellCasting) const
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
		scbw::is_cheat_enabled(CheatFlags::TheGathering) ||
		this->energy >= energy
		)
		return true;
	else
		return false;

}

//Identical to function Unit__SpendEnergy @ 0x00476180
bool Unit::has_weapon() const
{
	assert(this);

	if (this->get_ground_weapon() != WeaponId::None
		|| this->get_air_weapon() != WeaponId::None)
		return true;

	if (this->id == UnitId::ProtossSolarion)
	{
		if ((this->unit_with_hangar.in_hangar_count + this->unit_with_hangar.out_hangar_count) > 0)
		{
			return true;
		}
	}

	if (this->id == UnitId::ProtossAccantor)
	{
		if (this->unit_with_hangar.in_hangar_count > 0)
		{
			return true;
		}
	}

	if (this->subunit)
	{
		if (this->subunit->get_ground_weapon() != WeaponId::None
			|| this->subunit->get_air_weapon() != WeaponId::None)
		{
			return true;
		}
	}

	return false;
}

//Identical to function @ 0x004020B0
bool Unit::is_frozen() const
{
	assert(this);
	return
		this->status & UnitStatus::Frozen
		|| this->lockdown_timer
		|| this->stasis_timer
		|| this->maelstrom_timer;
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
bool Unit::is_subunit() const
{
	return (this && units_dat::base_property[this->id] & UnitProperty::Subunit);
}

// Equivalent to function @ 0x0047B2E0 until you change .dat files.
bool Unit::is_factory() const
{
	return (this && units_dat::group_flags[this->id].is_factory);
}

//Identical to function @ 0x00401210
bool Unit::is_valid_caster() const
{
	assert(this);
	return units_dat::base_property[this->id] & UnitProperty::Spellcaster
		&& !(this->status & UnitStatus::IsHallucination);
}

//Identical to function @ 0x00401DC0
//Returns:	0 - Unit has not reached destination
//					1 - Unit has reached destination
//					2 - Unit cannot move
u32 Unit::get_movable_state()
{
	if (this->move_target.pt != this->sprite->position)
		return 0;
	else if (!(this->status & UnitStatus::Unmovable))
		return 1;
	else
		return 2;
}

//-------- Positions and dimensions --------//

u16 Unit::get_x() const
{
	assert(this);
	assert(this->sprite);
	return this->sprite->position.x;
};

u16 Unit::get_y() const
{
	assert(this);
	assert(this->sprite);
	return this->sprite->position.y;
};

u16 Unit::getTileX() const
{
	assert(this);
	return this->get_x() / 32;
};

u16 Unit::getTileY() const
{
	assert(this);
	return this->get_y() / 32;
};

s16 Unit::get_left() const
{
	assert(this);
	return this->get_x() - units_dat::UnitBounds[this->id].left;
}

s16 Unit::get_right() const
{
	assert(this);
	return this->get_x() + units_dat::UnitBounds[this->id].right;
}

s16 Unit::get_top() const
{
	assert(this);
	return this->get_y() - units_dat::UnitBounds[this->id].top;
}

s16 Unit::get_bottom() const
{
	assert(this);
	return this->get_y() + units_dat::UnitBounds[this->id].bottom;
}

//-------- Modifying unit state --------//

const u32 Func_SetUnitHp = 0x00467340;
void Unit::set_hitpoints(s32 hitpoints)
{
	assert(this);

	__asm {
		PUSHAD
		MOV EAX, this
		MOV ECX, hitpoints
		CALL Func_SetUnitHp
		POPAD
	}
}

void Unit::heal(s32 hitpoints, Unit* source, bool restore)
{
	assert(this);
	if (this->hitpoints == this->get_max_hp_raw()) // If unit is fully healed already, skip healing
	{
		return;
	}

	// Cohort - Servile Vigor
	if (id == UnitId::SharedCohort && this != source)
	{
		hitpoints *= 2;
	}

	set_hitpoints(std::min((u32)this->hitpoints + hitpoints, this->get_max_hp_raw()));

	// If the unit is fully healed after this, clear any debuffs
	if (this->hitpoints == this->get_max_hp_raw() && restore)
	{
		scbw::restore_func(this);
	}
}

void Unit::heal_shields(s32 shields, Unit* source, bool restore)
{
	assert(this);
	if (this->shields == this->get_max_shields_raw()) // skip healing if max shields already
	{
		return;
	}

	this->shields = std::min((u32)this->shields + shields, this->get_max_shields_raw());
	this->set_hitpoints(this->hitpoints); // Updates unit strength

	// If heals to full clear debuffs
	if (this->shields == this->get_max_shields_raw() && restore)
	{
		scbw::restore_func(this);
	}
}

void Unit::heal_overflow(s32 amount, Unit* source, bool restore)
{
	heal(amount, source, restore);
	s32 shields = this->hitpoints + amount - get_max_hp_raw();
	if (shields > 0)
	{
		heal_shields(shields, source, restore);
	}
}

void Unit::heal_underflow(s32 amount, Unit* source, bool restore)
{
	heal_shields(amount, source, restore);
	s32 health = shields + amount - get_max_shields_raw();
	if (health > 0)
	{
		heal(health, source, restore);
	}
}

void Unit::heal_energy(s32 energy, Unit* source)
{
	assert(this);
	if (this->energy == helper::get_unit_max_energy(this) * 256)
	{
		return;
	}
	this->energy = std::min(this->energy + energy, helper::get_unit_max_energy(this) * 256);
}

const u32 Func_DoWeaponDamage = 0x00479930; // Note: hooked in hooks/weapons/weapon_damage files
void Unit::damage_with(u8 weaponId, s32 damage, Unit* attacker, u8 attackingPlayer, s8 direction, u8 damageDivisor)
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

void Unit::damage_with_custom(
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

	hooks::weapon_damage_custom(
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
	UnitExt* unitExt = globals.get_unit_ext(this);
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
		const LO_Header* shield_lo = reinterpret_cast<LO_Header*>(scbw::get_image_data(ImagesDatEntries::OverlayShieldPtr,
			this->sprite->main_graphic->id));

		u32 frameAngle = (direction - 124) / 8 % 32;
		Point8 offset = shield_lo->get_offset(this->sprite->main_graphic->direction, frameAngle);
		this->sprite->create_overlay(ImageId::ShieldOverlay, offset.x, offset.y, frameAngle);
	}
	damage_hp(damage, attacker, attackingPlayer, notify);
}

const u32 Func_DamageUnitHp = 0x004797B0;
void Unit::damage_hp(s32 damage, Unit* attacker, s32 attackingPlayer, bool notify)
{
	assert(this);

	// Logging
	auto targetId = this->id;
	auto targetPlayerId = this->player;
	if (attacker)
	{
		GPTP::logger << "damage_hp(damage (" << damage / 256 << "), attackerId (" << attacker->id << "), attackingPlayerId (" << attackingPlayer << "), notify (" << notify
			<< "), target (" << targetId << "), targetPlayerId (" << targetPlayerId << "))" << std::endl;
	}

	u32 notify_ = notify ? 1 : 0;

	if (attacker)
	{
		UnitExt* attackerExt = globals.get_unit_ext(attacker);
		attackerExt->damage_dealt_timer = 0;
	}

	UnitExt* unitExt = globals.get_unit_ext(this);
	unitExt->damage_receive_timer = 0;

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

// Logically equivalent to function @ 0x00454ED0
void Unit::reduce_defensive_matrix_hp(s32 amount)
{
	assert(this);
	if (this->defensive_matrix_hp > amount)
	{
		this->defensive_matrix_hp -= amount;
	}
	else
	{
		Image* current_image;
		this->defensive_matrix_hp = 0;
		this->defensive_matrix_timer = 0;
		this->remove_overlay(ImageId::DefensiveMatrixFront_Small, ImageId::DefensiveMatrixFront_Large);
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
				current_image = nullptr; // to exit loop
			}
		}
	}

	if (this->defensive_matrix_timer != 0 && !(this->status & UnitStatus::Burrowed))
	{
		Unit* unit = this->subunit ? this->subunit : this;
		// get_unit_overlay_adjustment code was hardcoded into function in original code)
		unit->sprite->create_top_overlay(scbw::get_unit_overlay_adjustment(unit) + ImageId::DefensiveMatrixHit_Small);
	}

}

const u32 Func_RemoveUnit = 0x00475710; // AKA orders_self_destructing()
void Unit::remove()
{
	assert(this);
	__asm
	{
		PUSHAD
		MOV EAX, this
		CALL Func_RemoveUnit
		POPAD
	}
}

const u32 Func_RemoveLockdown = 0x00454D90;
void Unit::remove_lockdown()
{
	assert(this);
	__asm
	{
		PUSHAD
		MOV ESI, this
		CALL Func_RemoveLockdown
		POPAD
	}
}

const u32 Func_RemoveMaelstrom = 0x00454D20;
void Unit::remove_maelstrom()
{
	assert(this);
	__asm
	{
		PUSHAD
		MOV ESI, this
		CALL Func_RemoveMaelstrom
		POPAD
	}
}

const u32 Func_FreezeUnit = 0x00492CC0;
void Unit::freeze()
{
	assert(this);
	__asm {
		PUSHAD
		MOV EAX, this
		CALL Func_FreezeUnit
		POPAD
	}
}

const u32 Func_UnfreezeUnit = 0x004F62D0;
void Unit::unfreeze()
{
	assert(this);
	__asm
	{
		PUSHAD
		MOV ESI, this
		CALL Func_UnfreezeUnit
		POPAD
	}
}

const u32 Func_RemoveAcidSpores = 0x004F4160;
void Unit::removeAcidSpores()
{
	assert(this);
	__asm
	{
		PUSHAD
		MOV EDX, this
		CALL Func_RemoveAcidSpores
		POPAD
	}
}

/// Identical to Unit__SpendEnergy @ 0x00491460
/// May underflow if alone
void Unit::spend_unit_energy(u32 energySpent)
{
	UnitExt* unitExt = globals.get_unit_ext(this);
	if (!(*bw_cheat_state & CheatFlags::TheGathering))
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
	this->energy = MathUtils::clamp<u32>(this->energy + energy, 0, this->get_max_energy());
}

void Unit::addUnitEnergy(u32 energy, u32 energyMax)
{
	this->energy = MathUtils::clamp<u32>(this->energy + energy, 0, energyMax);
}

// Identical to get_total_hp_plus_shields @ 00402710
s32 Unit::get_total_hp_plus_shields()
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
		hp = (this->hitpoints + 255) >> 8;
		if (!hp)
		{
			hp = 1;
		}
	}
	return shields + hp;
}

// Identical to isAIControllerNotABuilding @ 004381D0
s32 Unit::is_unit_ai_non_building()
{
	assert(this);
	UnitAi* ai = (UnitAi*)this->unit_ai;
	MilitaryAi* mai = (MilitaryAi*)ai;
	s32 result = 1;
	if (!ai
		|| ai->type != 4
		|| mai->unit != this
		|| units_dat::base_property[this->id] & UnitProperty::Building)
	{
		result = 0;
	}
	return result;
}

//-------- Unit orders --------//

// Logically equivalent to function @ 0x004752B0
// ESI = this
// EAX = target
// [ebp+08] = order_id
void Unit::order_to(u8 order_id, Unit* target)
{
	assert(this);
	this->user_action_flags |= 0x1;
	if (target)
	{
		this->order(order_id, target->get_x(), target->get_y(), target, UnitId::None, true);
	}
	else
	{
		this->order(order_id, 0, 0, target, UnitId::None, true);
	}
	prepare_for_next_order(this);
}

// Logically equivalent to function @ 0x00475260
void Unit::order_to(u8 order_id, u16 x, u16 y)
{
	assert(this);
	this->user_action_flags |= 0x1;
	this->order(order_id, x, y, nullptr, UnitId::None, true);
	prepare_for_next_order(this);
}

// Logically equivalent to function orderComputer_cl @ 0x00475310
// used in hooks so not doing assert, mind it if used in mods
void Unit::order_computer(u8 order_id)
{
	this->user_action_flags |= 0x1;
	this->order(order_id, 0, 0, nullptr, UnitId::None, true);
	prepare_for_next_order(this);
}

// Equivalent to 00474D60 __order function
void Unit::order_simple(u8 order_id, bool stopPreviousOrders)
{
	this->order(order_id, 0, 0, nullptr, UnitId::None, stopPreviousOrders);
}

// Equivalent to 00474D10 issue_queued_order_target function
void Unit::issue_queued_order_target(u8 order_id, Unit* target, bool stopPreviousOrders)
{
	if (target == nullptr)
	{
		this->order(order_id, 0, 0, nullptr, UnitId::None, stopPreviousOrders);
	}
	else
	{
		this->order(order_id, target->sprite->position.x, target->sprite->position.y, target, UnitId::None, stopPreviousOrders);
	}
}

// Logically equivalent to function to_idle @ 0x004753A0
void Unit::order_to_idle() {
	assert(this);
	if (this->order_queue_head) {
		this->user_action_flags |= 1;
		prepare_for_next_order(this);
	} else if (this->unit_ai) {
		this->order_to(OrderId::ComputerAI);
	} else {
		this->order_to(units_dat::ReturnToIdleOrder[this->id]);
	}
}

void Unit::order(u8 order_id, u16 x, u16 y, Unit* target, u16 targetUnitId, bool stopPreviousOrders)
{
	assert(this);
	const u32 Func_Order = 0x00474810;
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
		MOV CL, order_id
		MOV EDX, this
		CALL Func_Order
		POPAD
	}
}

void Unit::perform_another_order(u8 order_id, s16 x, s16 y, Unit* target, u16 targetUnitId, u32 unk)
{
	const u32 Func_PerformAnotherOrder = 0x004745F0;
	static Point16 pos;
	pos.x = x; pos.y = y;
	__asm {
		PUSHAD
		PUSH target
		PUSH pos
		MOV BL, order_id
		MOVZX EDX, targetUnitId
		MOV ESI, this
		MOV EDI, unk
		CALL Func_PerformAnotherOrder
		POPAD
	}
}

// Identical to @ 0x004743D0
void Unit::set_secondary_order(u8 order_id)
{
	assert(this);
	if (this->secondary_order_id == order_id)
	{
		return;
	}
	this->secondary_order_id = order_id;
	this->secondary_order_pos.x = 0;
	this->secondary_order_pos.y = 0;
	this->current_build_unit = nullptr;
	this->secondary_order_state = 0;
}

//-------- Graphics and animations --------//

// Similar to function @ 0x004E5C90
Image* Unit::get_overlay(u16 imageId) const
{
	assert(this);
	Image* image = this->sprite->get_overlay(imageId);
	if (!image && this->subunit)
	{
		image = this->subunit->sprite->get_overlay(imageId);
	}
	return image;
}

void Unit::play_iscript_animation(IscriptAnimation::Enum animation)
{
	assert(this);
	this->sprite->play_iscript_animation(animation);
}

void Unit::remove_overlay(u32 imageIdStart, u32 imageIdEnd)
{
	assert(this);
	this->sprite->remove_overlay(imageIdStart, imageIdEnd);
	if (this->subunit)
	{
		this->subunit->sprite->remove_overlay(imageIdStart, imageIdEnd);
	}
}

void Unit::remove_overlay(u32 imageId)
{
	assert(this);
	this->sprite->remove_overlay(imageId);
	if (this->subunit)
	{
		this->subunit->sprite->remove_overlay(imageId);
	}
}

// -----------------------------------------------------------------------//
//  Return the proper ColorId to use an unit color with graphics function
// -----------------------------------------------------------------------//

graphics::ColorId Unit::get_faction_color()
{
	assert(this);
	return get_faction_color(this);
}

graphics::ColorId Unit::get_faction_color(Unit* unit)
{
	// store a player until the end where it will properly store a ColorId
	graphics::ColorId return_value;
	if (unit != nullptr)
	{
		if (unit->sprite != nullptr)
		{
			// note: using the sprite player to preserve recruitable units color
			return_value = unit->sprite->player;
		}
		else
		{
			// note: using player color for spriteless units (if that exist)
			return_value = unit->player;
		}
		// for normal players, apply colors switching
		if (unit->player < PLAYABLE_PLAYER_COUNT)
		{
			// get the proper player in multiplayer game (don't break campaigns/UMS)
			return_value = factionsColorsOrdering[return_value];
			// fix BROWN to GREEN color swap in Desert map
			if (*CURRENT_TILESET == TilesetType::Desert && return_value == 5)
			{
				return_value = 8;
			}
			// fix WHITE to GREEN color swap in Ice map
			if (*CURRENT_TILESET == TilesetType::Ice && return_value == 6)
			{
				return_value = 8;
			}
		}
		if (return_value < PLAYER_COUNT) {
			return_value = graphics::standardColors[return_value];
		}
		else {
			return_value = graphics::BLACK; // will return 0/black if abnormal player
		}
	}
	else {
		return_value = graphics::BLACK; // will return 0/black if null pointer is the argument
	}
	return return_value;
}

graphics::ColorId Unit::get_color()
{
	assert(this);
	return get_color(this);
}

graphics::ColorId Unit::get_color(Unit* unit)
{
	// getColourID internal function
	const u32 Func_getColourID = 0x0049B0E0;
	graphics::ColorId return_value;
	if (unit != nullptr)
	{
		if (unit->sprite != nullptr) {
			// note: using the sprite player to preserve recruitable units color
			return_value = unit->sprite->player;
		}
		else {
			// note: using player color for spriteless units (if that exist)
			return_value = unit->player;
		}
		if (return_value < PLAYABLE_PLAYER_COUNT)
		{
			__asm {
				PUSHAD
				MOV AL, return_value
				CALL Func_getColourID
				MOV return_value, AL
				POPAD
			}
			return_value = playerMinimapColors[return_value];
		}
		else {
			return_value = graphics::BLACK; // will return 0/black if abnormal player
		}
	}
	else {
		return_value = graphics::BLACK; // will return 0/black if unit is nullptr
	}
	return return_value;
}

//-------- Distances and terrain --------//

u32 Unit::get_distance_to_target(Unit* target) const
{
	assert(this);
	assert(target);
	Unit* unit = (Unit*)this;
	if (this->is_subunit()) {
		unit = this->subunit; // Current unit is a turret, so use it's base instead
	}
	s32 dx = unit->get_left() - target->get_right() - 1;
	if (dx < 0)
	{
		dx = target->get_left() - unit->get_right() - 1;
		if (dx < 0)
		{
			dx = 0;
		}
	}
	s32 dy = unit->get_top() - target->get_bottom() - 1;
	if (dy < 0)
	{
		dy = target->get_top() - unit->get_bottom() - 1;
		if (dy < 0)
		{
			dy = 0;
		}
	}
	return scbw::get_distance_fast(0, 0, dx, dy);
}

bool Unit::has_path_to_pos(u32 x, u32 y) const
{
	const u32 Func_HasPathToPos = 0x0049CB60;
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

bool Unit::has_path_to_unit(Unit* target) const
{
	const u32 Func_HasPathToTarget = 0x0049CBB0;
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

bool Unit::is_target_within_min_range(Unit* target, u32 range)
{
	const u32 Func_IsTargetWithinMinRange = 0x00430F10;
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

s32 Unit::can_make_unit(u16 unit_id, u8 player) const
{
	assert(this);
	const u32 Func_CanMakeUnit = 0x0046E1C0;
	s32 playerId_ = player;
	static s32 result;
	__asm {
		PUSHAD
		PUSH playerId_
		MOV AX, unit_id
		MOV ESI, this
		CALL Func_CanMakeUnit
		MOV result, EAX
		POPAD
	}
	return result;
}

s32 Unit::canUseTech(u8 techId, u8 player) const
{
	assert(this);
	const u32 Func_CanUseTech = 0x0046DD80;
	s32 playerId_ = player;
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

bool Unit::can_attack_target(Unit* target, bool checkVisibility) const
{
	assert(this);
	assert(target);
	const u32 Func_CanUnitAttackTarget = 0x00476730;
	u32 _checkVisibility = checkVisibility ? 1 : 0;
	static u32 result;
	__asm
	{
		PUSHAD
		PUSH _checkVisibility
		MOV EBX, target
		MOV ESI, this
		CALL Func_CanUnitAttackTarget
		MOV result, EAX
		POPAD
	}

	return result;
}

extern const u32 Func_FireUnitWeapon = 0x00479C90;
void Unit::fireWeapon(u8 weaponId) const
{
	assert(this);
	static u32 weaponId_;
	weaponId_ = weaponId;
	__asm
	{
		PUSHAD
		PUSH weaponId_
		MOV ESI, this
		CALL Func_FireUnitWeapon
		POPAD
	}
}

Unit* Unit::get_unit_from_index(u16 index)
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
		return &unit_table[index - 1];
	}
	return nullptr;
}

u16 Unit::get_unit_index() const
{
	assert(this);
	if (globals.extended_unit_table == 1)
	{
		return this - (Unit*)&new_unit_table.units[0] + 1;
	}
	return this - unit_table + 1;
}

u8 Unit::get_last_owner_id() const
{
	assert(this);
	assert(this->sprite);
	if (this->player == 11) // The owner has left the game
	{
		return this->sprite->player;
	}
	else
	{
		return this->player;
	}
}

Unit* Unit::get_loaded_unit(s32 index)
{
	return hooks::get_loaded_unit_from_index(this, index);
}

// Logically equivalent to function @ 0x004E6C90
Unit* Unit::get_first_loaded_unit()
{
	assert(this);
	return hooks::get_first_loaded_unit(this);
}

// Logically equivalent to function @ 0x004E7110
bool Unit::has_loaded_unit()
{
	assert(this);
	for (s32 i = 0; i < 8; ++i)
	{
		if (!get_loaded_unit(i))
		{
			return true;
		}
	}
	return false;
}

namespace
{
	typedef u32(__stdcall* GiveUnitToPlayerFunc)(Unit* unit, u32 player);
	GiveUnitToPlayerFunc const giveUnitToPlayer = (GiveUnitToPlayerFunc)0x004C8040;
}

bool Unit::give_to(u8 player)
{
	assert(this);
	assert(player < 12);
	if (giveUnitToPlayer(this, player) != 0)
	{
		if ((this->id == UnitId::ZergNydusCanal || !(units_dat::base_property[this->id] & UnitProperty::Building))
			&& this->id != UnitId::ZergEgg
			&& this->id != UnitId::ZergLarva)
		{
			this->order_to_idle();
		}
		UnitExt* unitExt = globals.get_unit_ext(this);
		unitExt->timers[TimerOwnerChangedRecently] = 2;
		return true;
	}
	return false;
}

// Identical to function @ 00475A50
bool Unit::is_dead() const
{
	assert(this);
	return (!this->sprite) || (this->main_order_id == OrderId::Die);
}

// Probably equivalent to function @ 0047B910
bool Unit::is_target_enemy(Unit* target) const
{
	assert(this);
	return scbw::is_unit_enemy(this->player, target);
}

bool Unit::is_visible_to(u8 player) const
{
	assert(this);
	return (this->visibility_status & (1 << player)) != 0;
}

const u32 Func_UpdateSpeed = 0x00454310;
void Unit::update_speed()
{
	assert(this);

	__asm {
		PUSHAD
		MOV EAX, this
		CALL Func_UpdateSpeed
		POPAD
	}
}

u32 Unit::get_supply_provided() // overwrites previous supply provided
{
	UnitExt* unitExt = globals.get_unit_ext(this);
	if (unitExt->values[ValueSupplyProvidedSet])
	{
		if (unitExt->values[ValueSupplyProvidedId] == id) {
			return unitExt->values[ValueSupplyProvided];
		}
		else {
			raceSupply[RaceId::Terran].provided[player] -= unitExt->values[ValueSupplyProvided];
			raceSupply[RaceId::Terran].provided[player] += units_dat::SupplyProvided[unitExt->values[ValueSupplyProvidedId]];
			unitExt->values[ValueSupplyProvidedSet] = 0;
			unitExt->values[ValueSupplyProvidedId] = 0;
			unitExt->values[ValueSupplyProvided] = 0;
		}
	}

	u32 supply = units_dat::SupplyProvided[id];
	if (scbw::is_zerg_egg_new(this, false, false))
	{
		if (build_queue[build_queue_slot % 5] == UnitId::ZergIroleth)
		{
			supply = units_dat::SupplyProvided[UnitId::ZergOvileth];
		}
		if (build_queue[build_queue_slot % 5] == UnitId::ZergOthstoleth)
		{
			supply = units_dat::SupplyProvided[UnitId::ZergIroleth];
		}
		if (build_queue[build_queue_slot % 5] == UnitId::ZergAlaszileth)
		{
			supply = units_dat::SupplyProvided[UnitId::ZergOthstoleth];
		}
	}
	return supply;
}

void Unit::set_supply_provided(u8 amount) // overwrites previous supply provided
{
	UnitExt* unitExt = globals.get_unit_ext(this);
	raceSupply[RaceId::Terran].provided[player] -= get_supply_provided();
	raceSupply[RaceId::Terran].provided[player] += amount;
	unitExt->values[ValueSupplyProvidedSet] = 1;
	unitExt->values[ValueSupplyProvidedId] = id;
	unitExt->values[ValueSupplyProvided] = amount;
}

void Unit::reset_supply_provided() // overwrites previous supply provided
{
	UnitExt* unitExt = globals.get_unit_ext(this);
	set_supply_provided(units_dat::SupplyProvided[id]);
	unitExt->values[ValueSupplyProvidedSet] = 0;
	unitExt->values[ValueSupplyProvidedId] = 0;
	unitExt->values[ValueSupplyProvided] = 0;
}

bool Unit::is_attempting_protoss_build()
{
	assert(this);
	return this->main_order_id == OrderId::BuildProtoss1
		&& this->status & UnitStatus::GroundedBuilding
		&& this->order_target.unit != NULL
		&& !(this->order_target.unit->status & UnitStatus::Completed);
}

bool Unit::is_constructing_addon()
{
	assert(this);
	return this->secondary_order_id == OrderId::BuildAddon
		&& this->status & UnitStatus::GroundedBuilding
		&& this->current_build_unit != NULL
		&& !(this->current_build_unit->status & UnitStatus::Completed);
}

u8 Unit::get_hangar_train_count()
{
	assert(this);
	const u32 Func_getHangarTrainCount = 0x00465270;
	static u8 result = 0;
	__asm {
		PUSHAD
		MOV EDX, this
		CALL Func_getHangarTrainCount
		MOV result, AL
		POPAD
	}
}

u8 Unit::get_max_hangar_space()
{
	assert(this);
	const u32 Func_getMaxHangarSpace = 0x004653D0;
	static u8 result = 0;
	__asm {
		PUSHAD
		MOV EDX, this
		CALL Func_getMaxHangarSpace
		MOV result, AL
		POPAD
	}
}

bool Unit::is_reaver()
{
	assert(this);
	const u32 Func_unitIsReaver = 0x00401490;
	static s32 result;
	__asm {
		PUSHAD
		MOV EAX, this
		CALL Func_unitIsReaver
		MOV result, EAX
		POPAD
	}
	return (bool)result;
}

bool Unit::is_carrier()
{
	assert(this);
	const u32 Func_unitIsCarrier = 0x00401470;
	static s32 result;
	__asm {
		PUSHAD
		MOV EAX, this
		CALL Func_unitIsCarrier
		MOV result, EAX
		POPAD
	}
	return (bool)result;
}

bool Unit::unit_is_training_or_morphing()
{
	assert(this);
	const u32 Func_unitIsTrainingOrMorphing = 0x00401500;
	static u32 result;
	__asm {
		PUSHAD
		MOV ECX, this
		CALL Func_unitIsTrainingOrMorphing
		MOV result, EAX
		POPAD
	}
	return (bool)result;
}

bool Unit::is_queue_slot_active(int slot)
{
	assert(this);
	return this->build_queue[(slot + (unsigned __int8)this->build_queue_slot) % 5] <= (unsigned __int16)UnitId::Spell_DisruptionWeb;
}

u8 Unit::get_right_click_action_order()
{
	u8 result;
	if (this->id == UnitId::ZergLakizilisk && this->status & UnitStatus::Burrowed)
	{
		result = RightClickActions::NoMove_NormalAttack;
	}
	else
	{
		u8 temp_result = units_dat::RightClickAction[this->id];
		if (temp_result == RightClickActions::NoCommand_AutoAttack &&
			this->status & UnitStatus::GroundedBuilding &&
			this->is_factory())
		{
			result = RightClickActions::NormalMove_NoAttack;
		}
		else
		{
			result = temp_result;
		}
	}
	return result;
}