#include <cassert>
#include "Unit.h"
#include "../api.h"
#include "../enumerations.h"
#include "globals.h"
#include "hooks\limits\ccmu.h"
#include "hooks\weapons\weapon_damage.h"
#include "hooks\load_unload_proc.h"
#include "trigger_system/trigger_events.h"
#include <DatExtend/datext.h>
#include "math_utils.h"
#include "hooks/unit_stats/armor_bonus.h"
#include "hooks/target_related.h"
#include "data_ext.h"

//-------- Unit stats and properties --------//

// Behavior can be modified with detector hook
bool Unit::can_detect() const {
	assert(this);
	static u32 result;
	const u32 func_address = 0x00403430;
	__asm {
		PUSHAD
		MOV EAX, this
		CALL func_address
		MOV result, EAX
		POPAD
	}
	return result != 0;
}

u32 Unit::get_current_hp_ingame() const {
	assert(this);
	return (this->hitpoints + 255) >> 8;
}

u32 Unit::get_current_shields_ingame() const {
	assert(this);
	return this->shields >> 8;
}

//Identical to function @ 0x004026D0
u32 Unit::get_current_life_ingame() const {
	assert(this);
	if (units_dat::shields_enabled[this->id]) {
		return get_current_hp_ingame() + get_current_shields_ingame();
	}
	return get_current_hp_ingame();
}

s32 Unit::max_shields() const {
	assert(this);
	return units_dat::max_shield_points[this->id];
}

bool Unit::shields_enabled() const {
	assert(this);
	return units_dat::shields_enabled[this->id];
}

//Identical to function @ 0x00475AD0
u8 Unit::get_ground_weapon() const {
	if (unit_id_is_burrowed_attacker(this->id) && !(this->status & UnitStatus::Burrowed)) {
		return WeaponId::None;
	} else {
		return units_dat::ground_weapon[this->id];
	}
}

u8 Unit::get_air_weapon() const {
	assert(this);
	return units_dat::air_weapon[this->id];
}

u16 Unit::get_max_energy() const {
	static u32 func_address = 0x00491870;
	assert(this);
	assert(this);
	static u16 result;
	__asm {
		PUSHAD
		MOV EAX, this
		CALL func_address
		MOV result, AX
		POPAD
	}
	return result;
}

//Similar to function @ 0x00401400
u32 Unit::get_max_hp_ingame() const {
	assert(this);

	u32 max_hp = units_dat::max_hitpoints[this->id];
	if (max_hp == 0) {
		max_hp = this->get_current_hp_ingame();
		if (max_hp == 0)
		{
			max_hp = 1;
		}
	} else {
		max_hp = max_hp >> 8;
	}
	return max_hp;
}

u32 Unit::get_max_shields_in_game() const {
	assert(this);

	if (!this->shields_enabled()) {
		return 0;
	}

	u32 max_shields = units_dat::max_shield_points[this->id];
	if (max_shields == 0) {
		max_shields = this->get_current_shields_ingame();
		if (max_shields == 0)
		{
			max_shields = 1;
		}
	}
	return max_shields;
}

u32 Unit::get_max_hp_raw() const {
	assert(this);
	return units_dat::max_hitpoints[this->id];
}

u32 Unit::get_max_shields_raw() const {
	assert(this);
	return units_dat::max_shield_points[this->id] * 256;
}

u8 Unit::get_spell_countdown() const {
	assert(this);
	if (this->id == UnitId::TerranWyvern) {
		return this->ground_weapon_cooldown;
	}
	return this->spell_cooldown;
}

u32 Unit::get_max_weapon_range(u8 weapon_id) const {
	static u32 func_address = 0x00475870;
	assert(this);

	if (weapon_id == WeaponId::None) {
		return 0;
	}

	assert(weapon_id < WEAPON_TYPE_COUNT);

	static u32 max_weapon_range;
	__asm {
		PUSHAD
		MOV BL, weapon_id
		MOV EAX, this
		CALL func_address
		MOV max_weapon_range, EAX
		POPAD
	}
	return max_weapon_range;
}

const char* Unit::get_name() {
	assert(this);
	UnitExt* unit_ext = get_unit_ext(this);
	if (unit_ext->custom_name) {
		return unit_ext->custom_name;
	}
	const auto unit_id = (UnitId::Enum)this->id;
	if (this->player < PlayerLimit && g_globals->players[this->player].custom_unit_names.contains(unit_id)) {
		return g_globals->players[this->player].custom_unit_names[unit_id].data();
	}
	return get_name(this->id);
}

#include "BW/stat_txt_spec.h"
// Based on function @ 0x0047B090
const char* Unit::get_name(u16 unit_id) {
	assert(unit_id < UnitsIdLimit);
	if (unit_id >= UnitsIdLimit) {
		return (char*)g_string_empty;
	} else if (units_dat::map_string_id[unit_id] != 0) {
		return (char*)g_map_string_tbl->get_string(units_dat::map_string_id[unit_id]);
	} else if (DatExt::unitNamesTbl != NULL) {
		return (char*)((StringTbl*)&DatExt::unitNamesTbl)->get_string(unit_id + 1); // must be a pointer to a pointer, rather than just a pointer
	} else {
		return g_stat_txt_tbl_new->get_string(unit_id + 1);
	}
}

RaceId::Enum Unit::get_race() const {
	assert(this);
	return get_race(this->id);
}

// Based on function at 0x0047B150
RaceId::Enum Unit::get_race(u16 unit_id) {
	assert(unit_id < UnitsIdLimit);

	if (units_dat::group_flags[unit_id].is_zerg) {
		return RaceId::Zerg;
	} else if (units_dat::group_flags[unit_id].is_protoss) {
		return RaceId::Protoss;
	} else if (units_dat::group_flags[unit_id].is_terran) {
		return RaceId::Terran;
	} else {
		return RaceId::Neutral;
	}
}

u8 Unit::get_seek_range() const {
	static u32 func_address = 0x00476000;
	assert(this);
	static u8 seek_range;
	__asm {
		PUSHAD
		MOV EDX, this
		CALL func_address
		MOV seek_range, AL
		POPAD
	}
	return seek_range;
}

u32 Unit::get_sight_range(bool is_for_spellcasting) const {
	static u32 func_get_sight_range = 0x004E5B40;
	assert(this);
	static u32 sight_range;
	b32 ignore_status_effects = (is_for_spellcasting ? 0 : 1);
	__asm {
		PUSHAD
		PUSH ignore_status_effects
		MOV EDX, this
		CALL func_get_sight_range
		MOV sight_range, EAX
		POPAD
	}
	return sight_range;
}

// Similar to function Unit__HasEnergy @ 0x00491480
// The original function returned 0 when got energy or code
bool Unit::has_energy(u32 energy) const {
	if (this->energy >= energy) {
		return true;
	}
	return false;
}

// Identical to function Unit__SpendEnergy @ 0x00476180
bool Unit::has_weapon() const {
	assert(this);

	if (this->get_ground_weapon() != WeaponId::None
	||  this->get_air_weapon() != WeaponId::None)
	{
		return true;
	}

	if (this->id == UnitId::ProtossSolarion) {
		if ((this->unit_with_hangar.in_hangar_count + this->unit_with_hangar.out_hangar_count) > 0)
		{
			return true;
		}
	}

	if (this->id == UnitId::ProtossAccantorOld) {
		if (this->unit_with_hangar.in_hangar_count > 0)
		{
			return true;
		}
	}

	if (this->subunit) {
		if (this->subunit->get_ground_weapon() != WeaponId::None
			|| this->subunit->get_air_weapon() != WeaponId::None)
		{
			return true;
		}
	}

	return false;
}

// Identical to function @ 0x004020B0
bool Unit::is_frozen() const {
	assert(this);
	return bw::unit_is_stunned((Unit*)this);
}

bool Unit::is_remorphing_building() const {
	static u32 func_address = 0x0045CD00;
	static b32 result;
	assert(this);

	__asm {
		PUSHAD
		MOV EAX, this
		CALL func_address
		MOV result, EAX
		POPAD
	}

	return result != 0;
}

// Identical to function @ 0x00401D40
bool Unit::is_subunit() const {
	return (this && units_dat::base_property[this->id] & UnitProperty::Subunit);
}

// Equivalent to function @ 0x0047B2E0 until you change .dat files.
bool Unit::is_factory() const {
	return (this && units_dat::group_flags[this->id].is_factory);
}

//Identical to function @ 0x00401210
bool Unit::is_valid_caster() const {
	assert(this);
	return units_dat::base_property[this->id] & UnitProperty::Spellcaster && !(this->status & UnitStatus::IsHallucination);
}

// Identical to function @ 0x00401DC0
u32 Unit::get_movable_state() {
	if (this->move_target.pt != this->position) {
		return MovableState_NotReachedDestination;
	} else {
		if (!(this->status & UnitStatus::Unmovable)) {
			return MovableState_ReachedDestination;
		} else {
			return MovableState_UnmovableAtDestination;
		}
	}
}

//-------- Positions and dimensions --------//

u16 Unit::get_x() const {
	assert(this);
	return this->position.x;
};

u16 Unit::get_y() const {
	assert(this);
	return this->position.y;
};

u16 Unit::get_tile_x() const {
	assert(this);
	return this->get_x() / 32;
};

u16 Unit::get_tile_y() const {
	assert(this);
	return this->get_y() / 32;
};

s16 Unit::get_left() const {
	assert(this);
	return this->get_x() - units_dat::unit_bounds[this->id].left;
}

s16 Unit::get_right() const {
	assert(this);
	return this->get_x() + units_dat::unit_bounds[this->id].right;
}

s16 Unit::get_top() const {
	assert(this);
	return this->get_y() - units_dat::unit_bounds[this->id].top;
}

s16 Unit::get_bottom() const {
	assert(this);
	return this->get_y() + units_dat::unit_bounds[this->id].bottom;
}

Box16 Unit::get_box() const {
	assert(this);
	Box16 box = {};
	box.left   = this->get_left();
	box.top    = this->get_top();
	box.right  = this->get_right();
	box.bottom = this->get_bottom();
	return box;
}

//-------- Modifying unit state --------//

void Unit::set_hitpoints(s32 hitpoints) {
	static u32 func_address = 0x00467340;
	assert(this);

	hitpoints = Clamp(hitpoints, 0, units_dat::max_hitpoints[this->id]);

	__asm {
		PUSHAD
		MOV EAX, this
		MOV ECX, hitpoints
		CALL func_address
		POPAD
	}
}

static u32 cap_value(u32 value, u32 max, s32* remainder) {
	if (value > max) {
		*remainder = value - max;
		return max;
	}
	*remainder = 0;
	return value;
}

u32 Unit::heal_hp(s32 value, Unit* source, bool restore) {
	assert(this);
	assert(value >= 0);
	if (this->hitpoints == this->get_max_hp_raw()) { // If unit is fully healed already, skip healing
		return value;
	}

	set_hitpoints(cap_value(this->hitpoints + value, this->get_max_hp_raw(), &value));

	// If heals to full clear debuffs
	if (restore && this->hitpoints == this->get_max_hp_raw()) {
	//	bw::restore_func(this);
	}

	return value;
}

u32 Unit::heal_shields(s32 value, Unit* source, bool restore, bool proc_flash_shielding) {
	assert(this);
	assert(value >= 0);
	if (this->shields == this->get_max_shields_raw()) { // Skip healing if max shields already
		return value;
	}

	this->shields = cap_value(this->shields + value, this->get_max_shields_raw(), &value);
	this->set_hitpoints(this->hitpoints); // Also updates unit strength

	// If heals to full, clear debuffs
	if (restore && this->shields == this->get_max_shields_raw()) {
	//	bw::restore_func(this);
	}

	// Activate flash shielding
	if (proc_flash_shielding) {
		UnitExt* this_ext = get_unit_ext(this);
		this_ext->shield_regen_timer = UINT_MAX;
	}

	return value;
}

u32 Unit::heal_energy(s32 value, Unit* source) {
	assert(this);
	assert(value >= 0);
	if (this->energy == this->get_max_energy()) {
		return value;
	}

	this->energy = cap_value(this->energy + value, this->get_max_energy(), &value);

	return value;
}

u32 Unit::heal_hp_then_shields(s32 value, Unit* source, bool restore, bool proc_flash_shielding) {
	return heal_shields(heal_hp(value, source, restore), source, restore, proc_flash_shielding);
}

u32 Unit::heal_shields_then_hp(s32 value, Unit* source, bool restore, bool proc_flash_shielding) {
	return heal_hp(heal_shields(value, source, restore, proc_flash_shielding), source, restore);
}

u32 Unit::heal_energy_then_shields(s32 value, Unit* source, bool restore, bool proc_flash_shielding) {
	return heal_shields(heal_energy(value, source), source, restore, proc_flash_shielding);
}

void Unit::damage_with(u8 weapon_id, s32 damage, Unit* attacker, u8 attacking_player, s8 direction, u8 damage_divisor) {
	const u32 func_address = 0x00479930; // Note: hooked in hooks/weapons/weapon_damage files
	assert(this);
	assert(weapon_id < WeaponsIdLimit);
//	assert(damage_divisor != 0);
	if (damage < 0) {
		damage = weapons_dat::damage_amount[weapon_id] * 256;
	}
	__asm {
		PUSHAD
		MOV EDI, this
		MOV EAX, damage
		PUSH attacking_player
		PUSH attacker
		PUSH direction
		PUSH damage_divisor
		PUSH weapon_id
		CALL func_address
		POPAD
	}
}

void Unit::damage_with_custom(s32 damage, s32 armor_pen, Unit* attacker, u8 attacking_player_id, s8 direction, u8 damage_divisor) {
	assert(this);

	bool ignore_armor = false;
	if (armor_pen < 0) {
		armor_pen = 0;
		ignore_armor = true;
	}
	hooks::weapon_damage_custom(damage, this, armor_pen, attacker, attacking_player_id, direction, damage_divisor, ignore_armor);
}

void Unit::inflict_damage(s32 dmg, Unit* attacker, s32 attacking_player, bool notify, bool shield_overlay, s32 direction, bool ignore_damage_128_overflow) {
	UnitExt* unit_ext = get_unit_ext(this);
	auto damage = dmg;
	s32 shield_reduce_amount = 0;
//	if (unit_ext->timers[TimerGodsMadeFlesh]) {
//		return;
//	}
	if (units_dat::shields_enabled[this->id]) {
		shield_reduce_amount = std::min(damage, this->shields);
		damage -= shield_reduce_amount;
	}
	if (shield_reduce_amount == 0 && damage < 128 && !ignore_damage_128_overflow) {
		damage = 128;
	}
	if (shield_reduce_amount != 0) {
		this->shields -= shield_reduce_amount;
	}
	if (shield_overlay && units_dat::shields_enabled[this->id] && this->shields != 0) {
		const LO_Header* shield_lo = g_shield_overlays[this->sprite->main_graphic->id];

		u32 frame_angle = (direction - 124) / 8 % 32;
		Point8 offset = shield_lo->get_offset(this->sprite->main_graphic->direction, frame_angle);
		this->sprite->create_overlay(ImageId::ShieldOverlay, offset.x, offset.y, frame_angle);
	}
	damage_hp(damage, attacker, attacking_player, notify);
}

void Unit::damage_hp(s32 damage, Unit* attacker, s32 attackingPlayer, bool notify) {
	assert(this);

	// Logging
	/*
	u32 target_id = this->id;
	u32  target_player_id = this->player;
	if (attacker) {
		GPTP::logger << "damage_hp(damage (" << damage / 256 << "), attackerId (" << attacker->id << "), attacking_player_id (" << attackingPlayer << "), notify (" << notify
			<< "), target (" << target_id << "), target_player_id (" << target_player_id << "))" << std::endl;
	}
	*/

	if (attacker) {
		bw::update_on_damage_dealt(attacker, damage);
	}
	bw::update_on_damage_taken(this, damage);

	if (attacker) {
		TriggerEvent event = {};
		for (u32 i = 0; i < PlayerLimit; ++i) {
			if (is_visible_to(i)) {
				event.seen_by |= 1 << i;
			}
		}
		event.position = this->position;
		event.source_player = attackingPlayer;
		event.target_player = this->player;
		event.source_unit_id = attacker->id;
		event.target_unit_id = this->id;
		event.timestamp = g_elapsed_time_frames;
		combat_events.append(event);
	}

	u32 notify_xd = notify ? 1 : 0;
	const u32 func_address = 0x004797B0;
	__asm {
		PUSHAD
		PUSH notify_xd
		PUSH attackingPlayer
		PUSH attacker
		MOV EAX, damage
		MOV ECX, this
		CALL func_address
		POPAD
	}

	if (this->is_dead()) {
		TriggerEvent event = {};
		for (u32 i = 0; i < PlayerLimit; ++i) {
			if (is_visible_to(i)) {
				event.seen_by |= 1 << i;
			}
		}
		event.position = this->position;
		event.source_player = attackingPlayer;
		event.target_player = this->player;
		event.source_unit_id = attacker ? attacker->id : UnitId::None;
		event.target_unit_id = this->id;
		event.timestamp = g_elapsed_time_frames;
		kill_events.append(event);
	}
}

// Logically equivalent to function @ 0x00454ED0
void Unit::reduce_defensive_matrix_hp(s32 amount) {
	assert(this);
	if (this->defensive_matrix_hp > amount) {
		this->defensive_matrix_hp -= amount;
	} else {
		Image* current_image;
		this->defensive_matrix_hp = 0;
		this->defensive_matrix_timer = 0;
		this->remove_overlay(ImageId::DefensiveMatrixFront_Small, ImageId::DefensiveMatrixFront_Large);
		current_image = this->sprite->images.head;

		while (current_image) {
			if (current_image->id < ImageId::DefensiveMatrixBack_Small || current_image->id > ImageId::DefensiveMatrixBack_Large) {
				current_image = current_image->link.next;
			} else {
				current_image->free();
				current_image = nullptr; // to exit loop
			}
		}
	}

	if (this->defensive_matrix_timer != 0 && !(this->status & UnitStatus::Burrowed)) {
		Unit* unit = this->subunit ? this->subunit : this;
		// get_unit_overlay_adjustment code was hardcoded into function in original code)
		unit->sprite->create_top_overlay(bw::get_unit_overlay_adjustment(unit) + ImageId::DefensiveMatrixHit_Small);
	}
}

void Unit::remove() {
	assert(this);
	const u32 func_address = 0x00475710; // AKA orders_self_destructing()
	__asm {
		PUSHAD
		MOV EAX, this
		CALL func_address
		POPAD
	}
}

void Unit::remove_lockdown() {
	assert(this);
	const u32 func_address = 0x00454D90;
	__asm {
		PUSHAD
		MOV ESI, this
		CALL func_address
		POPAD
	}
}

void Unit::remove_maelstrom() {
	assert(this);
	const u32 func_address = 0x00454D20;
	__asm {
		PUSHAD
		MOV ESI, this
		CALL func_address
		POPAD
	}
}

void Unit::freeze() {
	assert(this);
	const u32 func_address = 0x00492CC0;
	__asm {
		PUSHAD
		MOV EAX, this
		CALL func_address
		POPAD
	}
	this->status |= UnitStatus::Frozen;
}

void Unit::unfreeze() {
	assert(this);
	const u32 func_address = 0x004F62D0;
	__asm {
		PUSHAD
		MOV ESI, this
		CALL func_address
		POPAD
	}
	this->status &= ~UnitStatus::Frozen;
	helper::change_unit_buttonset(this, this->id);
	bw::refresh_console();
}

void Unit::remove_acid_spores() {
	assert(this);
	const u32 func_address = 0x004F4160;
	__asm {
		PUSHAD
		MOV EDX, this
		CALL func_address
		POPAD
	}
}

/// Identical to Unit__SpendEnergy @ 0x00491460
/// May underflow if alone
void Unit::spend_energy(u32 energy_spent) {
	if (this->energy > energy_spent) {
		this->energy -= energy_spent;
	} else {
		this->energy = 0;
	}
	UnitExt* unit_ext = get_unit_ext(this);
	unit_ext->energy_regen_timer = 0;
}

void Unit::add_unit_energy(u32 energy) {
	this->energy = MathUtils::clamp<u32>(this->energy + energy, 0, this->get_max_energy());
}

void Unit::add_unit_energy_capped(u32 energy, u32 energyMax) {
	this->energy = MathUtils::clamp<u32>(this->energy + energy, 0, energyMax);
}

// Identical to get_max_vitality @ 00402710
s32 Unit::get_total_hp_plus_shields() {
	assert(this);
	s32 id = this->id;
	s32 shields = 0;
	s32 hp = units_dat::max_hitpoints[id] >> 8;
	if (bw::unit_has_shields(this)) {
		shields = units_dat::max_shield_points[id];
	}
	if (!hp) {
		hp = (this->hitpoints + 255) >> 8;
		if (!hp) {
			hp = 1;
		}
	}
	return shields + hp;
}

// Identical to isAIControllerNotABuilding @ 004381D0
s32 Unit::is_unit_ai_non_building() {
	assert(this);
	UnitAi* ai = (UnitAi*)this->unit_ai;
	MilitaryAi* mai = (MilitaryAi*)ai;
	s32 result = 1;
	if (!ai
	||  ai->type != AiType::Military
	||  mai->unit != this
	||  units_dat::base_property[this->id] & UnitProperty::Building)
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
void Unit::order_to(u8 order_id, Unit* target) {
	assert(this);
	this->user_action_flags |= UserActionFlags_IscriptGoToIdle;
	if (target) {
		this->order(order_id, target->get_x(), target->get_y(), target, UnitId::None, true);
	} else {
		this->order(order_id, 0, 0, target, UnitId::None, true);
	}
	prepare_for_next_order(this);
}

// Logically equivalent to function @ 0x00475260
void Unit::order_to(u8 order_id, u16 x, u16 y) {
	assert(this);
	this->user_action_flags |= UserActionFlags_IscriptGoToIdle;
	this->order(order_id, x, y, 0, UnitId::None, true);
	prepare_for_next_order(this);
}

// Logically equivalent to function orderComputer_cl @ 0x00475310
// used in hooks so not doing assert, mind it if used in mods
void Unit::order_computer(u8 order_id) {
	this->user_action_flags |= UserActionFlags_IscriptGoToIdle;
	this->order(order_id, 0, 0, 0, UnitId::None, true);
	prepare_for_next_order(this);
}

// Equivalent to 00474D60 __order function
void Unit::order_simple(u8 order_id, bool stopPreviousOrders) {
	this->order(order_id, 0, 0, 0, UnitId::None, stopPreviousOrders);
}

// Equivalent to 00474D10 issue_queued_order_target function
void Unit::issue_queued_order_target(u8 order_id, Unit* target, bool stopPreviousOrders) {
	if (!target) {
		this->order(order_id, 0, 0, 0, UnitId::None, stopPreviousOrders);
	} else {
		this->order(order_id, target->sprite->position.x, target->sprite->position.y, target, UnitId::None, stopPreviousOrders);
	}
}

// Logically equivalent to function to_idle @ 0x004753A0
void Unit::order_to_idle() {
	assert(this);
	if (this->order_queue_head) {
		this->user_action_flags |= UserActionFlags_IscriptGoToIdle;
		helper::do_next_queued_order(this);
	} else if (this->unit_ai) {
		this->order_to(OrderId::ComputerAI);
	} else {
		this->order_to(units_dat::return_to_idle_order[this->id]);
	}
}

void Unit::order(u8 order_id, u16 x, u16 y, Unit* target, u16 target_unit_id, bool stop_previous_orders) {
	assert(this);
	const u32 Func_Order = 0x00474810;
	static Point16 pos;
	static u32 target_unit_id_xd;
	pos.x = x, pos.y = y;
	target_unit_id_xd = target_unit_id;
	__asm {
		PUSHAD
		PUSH target_unit_id_xd
		PUSH target
		PUSH pos
		MOVZX EAX, stop_previous_orders
		MOV CL, order_id
		MOV EDX, this
		CALL Func_Order
		POPAD
	}
}

void Unit::perform_another_order(u8 order_id, s16 x, s16 y, Unit* target, u16 target_unit_id, u32 unk) {
	const u32 func_address = 0x004745F0;
	static Point16 pos;
	pos.x = x; pos.y = y;
	__asm {
		PUSHAD
		PUSH target
		PUSH pos
		MOV BL, order_id
		MOVZX EDX, target_unit_id
		MOV ESI, this
		MOV EDI, unk
		CALL func_address
		POPAD
	}
}

// Identical to @ 0x004743D0
void Unit::set_secondary_order(u8 order_id) {
	assert(this);
	if (this->secondary_order_id == order_id) {
		return;
	}
	this->secondary_order_id	= order_id;
	this->secondary_order_pos.x = 0;
	this->secondary_order_pos.y = 0;
	this->current_build_unit	= 0;
	this->secondary_order_state = 0;
}

//-------- Graphics and animations --------//

// Similar to function @ 0x004E5C90
Image* Unit::get_overlay(u16 imageId) const {
	assert(this);
	Image* image = this->sprite->get_overlay(imageId);
	if (!image && this->subunit) {
		image = this->subunit->sprite->get_overlay(imageId);
	}
	return image;
}

void Unit::play_iscript_animation(IscriptAnimation::Enum animation) {
	assert(this);
	this->sprite->play_iscript_animation(animation);
}

void Unit::remove_overlay(u32 imageIdStart, u32 imageIdEnd) {
	assert(this);
	this->sprite->remove_overlay(imageIdStart, imageIdEnd);
	if (this->subunit) {
		this->subunit->sprite->remove_overlay(imageIdStart, imageIdEnd);
	}
}

void Unit::remove_overlay(u32 imageId) {
	assert(this);
	this->sprite->remove_overlay(imageId);
	if (this->subunit) {
		this->subunit->sprite->remove_overlay(imageId);
	}
}

// -----------------------------------------------------------------------//
//  Return the proper ColorId to use an unit color with graphics function
// -----------------------------------------------------------------------//

graphics::ColorId Unit::get_faction_color() {
	assert(this);
	return get_faction_color(this);
}

graphics::ColorId Unit::get_faction_color(Unit* unit) {
	// store a player until the end where it will properly store a ColorId
	graphics::ColorId return_value;
	if (unit != nullptr) {
		if (unit->sprite != nullptr) {
			// note: using the sprite player to preserve recruitable units color
			return_value = unit->sprite->player;
		} else {
			// note: using player color for spriteless units (if that exist)
			return_value = unit->player;
		}
		// for normal players, apply colors switching
		if (unit->player < PLAYABLE_PLAYER_COUNT) {
			// get the proper player in multiplayer game (don't break campaigns/UMS)
			return_value = g_factions_colors_ordering[return_value];
			// fix BROWN to GREEN color swap in Desert map
			if (*g_current_tileset == TilesetType::Desert && return_value == 5) {
				return_value = 8;
			}
			// fix WHITE to GREEN color swap in Ice map
			if (*g_current_tileset == TilesetType::Ice && return_value == 6) {
				return_value = 8;
			}
		}
		if (return_value < PLAYER_COUNT) {
			return_value = graphics::standardColors[return_value];
		} else {
			return_value = graphics::BLACK; // will return 0/black if abnormal player
		}
	} else {
		return_value = graphics::BLACK; // will return 0/black if null pointer is the argument
	}
	return return_value;
}

graphics::ColorId Unit::get_color() {
	assert(this);
	return get_color(this);
}

graphics::ColorId Unit::get_color(Unit* unit) {
	// getColourID internal function
	const u32 Func_getColourID = 0x0049B0E0;
	graphics::ColorId return_value;
	if (unit != nullptr) {
		if (unit->sprite != nullptr) {
			// note: using the sprite player to preserve recruitable units color
			return_value = unit->sprite->player;
		} else {
			// note: using player color for spriteless units (if that exist)
			return_value = unit->player;
		}
		if (return_value < PLAYABLE_PLAYER_COUNT) {
			__asm {
				PUSHAD
				MOV AL, return_value
				CALL Func_getColourID
				MOV return_value, AL
				POPAD
			}
			return_value = g_player_minimap_colors[return_value];
		} else {
			return_value = graphics::BLACK; // will return 0/black if abnormal player
		}
	} else {
		return_value = graphics::BLACK; // will return 0/black if unit is nullptr
	}
	return return_value;
}

//-------- Distances and terrain --------//

u32 Unit::get_distance_to_target(Unit* target) const {
	assert(this);
	assert(target);
	Unit* unit = (Unit*)this;
	if (this->is_subunit()) {
		unit = this->subunit; // Current unit is a turret, so use it's base instead
	}
	s32 dx = unit->get_left() - target->get_right() - 1;
	if (dx < 0) {
		dx = target->get_left() - unit->get_right() - 1;
		if (dx < 0) {
			dx = 0;
		}
	}
	s32 dy = unit->get_top() - target->get_bottom() - 1;
	if (dy < 0) {
		dy = target->get_top() - unit->get_bottom() - 1;
		if (dy < 0) {
			dy = 0;
		}
	}
	return bw::distance_between(0, 0, dx, dy);
}

u32 Unit::get_distance_to_point(Point16 point) const {
	assert(this);
	Unit* unit = (Unit*)this;
	if (this->is_subunit()) {
		unit = this->subunit; // Current unit is a turret, so use it's base instead
	}
	s32 dx = unit->get_left() - point.x - 1;
	if (dx < 0) {
		dx = point.x - unit->get_right() - 1;
		if (dx < 0) {
			dx = 0;
		}
	}
	s32 dy = unit->get_top() - point.y - 1;
	if (dy < 0) {
		dy = point.y - unit->get_bottom() - 1;
		if (dy < 0) {
			dy = 0;
		}
	}
	return bw::distance_between(0, 0, dx, dy);
}

bool Unit::has_path_to_pos(u32 x, u32 y) const {
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

bool Unit::has_path_to_unit(Unit* target) const {
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

bool Unit::is_target_within_min_range(Unit* target, u32 range) {
	if (!this->sprite) {
		return false;
	}
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

s32 Unit::can_make_unit(u16 unit_id, u8 player) const {
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

s32 Unit::can_use_tech(u8 techId, u8 player) const {
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

bool Unit::can_attack_target(Unit* target, bool check_visibility) const {
	assert(this);
	assert(target);
	const u32 func_address = 0x00476730;
	u32 check_visibility_xd = check_visibility ? 1 : 0;
	static u32 result;
	__asm {
		PUSHAD
		PUSH check_visibility_xd
		MOV EBX, target
		MOV ESI, this
		CALL func_address
		MOV result, EAX
		POPAD
	}

	return result;
}

void Unit::fire_weapon(u8 weapon_id) const {
	assert(this);
	const u32 func_address = 0x00479C90;
	static u32 weapon_id_xd;
	weapon_id_xd = weapon_id;
	__asm {
		PUSHAD
		PUSH weapon_id_xd
		MOV ESI, this
		CALL func_address
		POPAD
	}
}

Unit* Unit::get_unit_from_index(u16 index) {
	if (1 <= index && index <= UNIT_ARRAY_LENGTH) {
		return &g_unit_table[index - 1];
	}
	return nullptr;
}

u16 Unit::get_unit_index() const {
	assert(this);
	return this - g_unit_table + 1;
}

u8 Unit::get_last_owner_id() const {
	assert(this);
	assert(this->sprite);
	if (this->player == 11) { // The owner has left the game
		return this->sprite->player;
	} else {
		return this->player;
	}
}

Unit* Unit::get_loaded_unit(s32 index) {
	return hooks::get_loaded_unit_from_index(this, index);
}

// Logically equivalent to function @ 0x004E6C90
Unit* Unit::get_first_loaded_unit() {
	assert(this);
	return hooks::get_first_loaded_unit(this);
}

// Logically equivalent to function @ 0x004E7110
bool Unit::has_loaded_unit() {
	assert(this);
	for (s32 i = 0; i < 8; ++i) {
		if (!get_loaded_unit(i))
		{
			return true;
		}
	}
	return false;
}

namespace {
	typedef u32(__stdcall* GiveUnitToPlayerFunc)(Unit* unit, u32 player);
	GiveUnitToPlayerFunc const give_unit_to_player = (GiveUnitToPlayerFunc)0x004C8040;
}

bool Unit::give_to(u8 player) {
	assert(this);
	assert(player < 12);
	if (give_unit_to_player(this, player) != 0) {
		if ((this->id == UnitId::ZergNydusCesiant || !(units_dat::base_property[this->id] & UnitProperty::Building))
			&& this->id != UnitId::ZergEgg
			&& this->id != UnitId::ZergLarva
			&& this->id != UnitId::ZergOcsalLarva)
		{
			this->order_to_idle();
		}
		UnitExt* unit_ext = get_unit_ext(this);
		unit_ext->timers[TimerOwnerChangedRecently] = 2;
		unit_ext->values[ValueAlliedInvPlayer] = player;
		unit_ext->timers[TimerAlliedInvFrames] = 24;
		return true;
	}
	return false;
}

// Identical to function @ 00475A50
bool Unit::is_dead() const {
	assert(this);
	return (!this->sprite) || (this->main_order_id == OrderId::Die);
}

// Probably equivalent to function @ 0047B910
bool Unit::is_target_enemy(Unit* target) const {
	assert(this);
	return hooks::is_unit_enemy((Unit*)this, target);
}

bool Unit::is_visible_to(u8 player) const {
	assert(this);
	return (!(this->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) || this->detection_status & (1 << player)) && this->sprite && this->sprite->visibility_status & (1 << player);
}

void Unit::update_speed() {
	assert(this);
	const u32 func_address = 0x00454310;
	__asm {
		PUSHAD
		MOV EAX, this
		CALL func_address
		POPAD
	}
}

u32 Unit::get_supply_provided() { // overwrites previous supply provided
	UnitExt* unit_ext = get_unit_ext(this);
	if (unit_ext->values[ValueSupplyProvidedSet]) {
		if (unit_ext->values[ValueSupplyProvidedId] == id) {
			return unit_ext->values[ValueSupplyProvided];
		} else {
			g_race_supply[RaceId::Terran].provided[player] -= unit_ext->values[ValueSupplyProvided];
			g_race_supply[RaceId::Terran].provided[player] += units_dat::supply_provided[unit_ext->values[ValueSupplyProvidedId]];
			unit_ext->values[ValueSupplyProvidedSet] = 0;
			unit_ext->values[ValueSupplyProvidedId] = 0;
			unit_ext->values[ValueSupplyProvided] = 0;
		}
	}

	u32 supply = units_dat::supply_provided[id];
	if (bw::is_zerg_egg_new(this, false, false)) {
		if (build_queue[build_queue_slot % 5] == UnitId::ZergIroleth)
		{
			supply = units_dat::supply_provided[UnitId::ZergOvileth];
		}
	}
	return supply;
}

void Unit::set_supply_provided(u8 amount) { // overwrites previous supply provided
	UnitExt* unit_ext = get_unit_ext(this);
	g_race_supply[RaceId::Terran].provided[player] -= get_supply_provided();
	g_race_supply[RaceId::Terran].provided[player] += amount;
	unit_ext->values[ValueSupplyProvidedSet] = 1;
	unit_ext->values[ValueSupplyProvidedId] = id;
	unit_ext->values[ValueSupplyProvided] = amount;
}

void Unit::reset_supply_provided() { // overwrites previous supply provided
	UnitExt* unit_ext = get_unit_ext(this);
	set_supply_provided(units_dat::supply_provided[id]);
	unit_ext->values[ValueSupplyProvidedSet] = 0;
	unit_ext->values[ValueSupplyProvidedId] = 0;
	unit_ext->values[ValueSupplyProvided] = 0;
}

bool Unit::is_attempting_protoss_build() {
	assert(this);
	return this->main_order_id == OrderId::BuildProtoss1
		&& this->status & UnitStatus::GroundedBuilding
		&& this->order_target.unit != NULL
		&& !(this->order_target.unit->status & UnitStatus::Completed);
}

bool Unit::is_constructing_addon() {
	assert(this);
	return this->secondary_order_id == OrderId::BuildAddon
		&& this->status & UnitStatus::GroundedBuilding
		&& this->current_build_unit != NULL
		&& !(this->current_build_unit->status & UnitStatus::Completed);
}

u8 Unit::get_hangar_train_count() {
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
	return result;
}

u8 Unit::get_max_hangar_space() {
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
	return result;
}

bool Unit::is_reaver() {
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

bool Unit::is_carrier() {
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

bool Unit::unit_is_training_or_morphing() {
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

bool Unit::is_queue_slot_active(int slot) {
	assert(this);
	return this->build_queue[(slot + (unsigned __int8)this->build_queue_slot) % 5] != UnitId::None;
}

u8 Unit::get_right_click_action_order() {
	u8 result;
	if (unit_id_is_burrowed_attacker(this->id) && this->status & UnitStatus::Burrowed) {
		result = RightClickActions::NoMove_NormalAttack;
	} else {
		u8 temp_result = units_dat::right_click_action[this->id];
		if (temp_result == RightClickActions::NoCommand_AutoAttack &&
			this->status & UnitStatus::GroundedBuilding &&
			this->is_factory()) {
			result = RightClickActions::NormalMove_NoAttack;
		} else {
			result = temp_result;
		}
	}
	return result;
}

UnitExt& Unit::ext() {
	return *get_unit_ext(this);
}

bool Unit::has_energy_for(u32 tech_id) {
	return this->energy >= techdata_dat::energy_cost[tech_id] * 256;
}

bool Unit::can_attack_transport_checks() {
	if (!(this->status & UnitStatus::InTransport)) {
		return true;
	}

	if (this->status & UnitStatus::InBuilding && this->connected_unit) {
		if (this->connected_unit->id == UnitId::TerranTencendur) {
			return true;
		}
		if (this->connected_unit->status & UnitStatus::InAir) {
			return false;
		}
		return true;
	}
	return false;
}