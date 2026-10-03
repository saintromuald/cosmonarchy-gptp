#include <cassert>
#include "Unit.h"
#include "../api.h"
#include "../enumerations.h"
#include "globals.h"
#include "hooks\limits\ccmu.h"
#include "hooks\weapons\weapon_damage.h"
#include "hooks\load_unload_proc.h"
#include "systems/triggers/trigger_events.h"
#include "systems/triggers/trigger_system.h"
#include <DatExtend/datext.h>
#include "hooks/unit_stats/armor_bonus.h"
#include "hooks/target_related.h"
#include "data_ext.h"
#include "BW/stat_txt_spec.h"
#include "hooks\weapons\weapon_cooldown.h"
#include "hooks\stim_packs.h"

UnitRef::UnitRef(Unit* unit) : index{unit->get_unit_index()}, unique_id{unit->unique_id} {}

Unit* UnitRef::get() const {
    Unit* unit = Unit::get_unit_from_index(index);
    return (unit && unit->unique_id == unique_id) ? unit : 0;
}

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
WeaponId::Enum Unit::get_ground_weapon() const {
    if (unit_id_is_burrowed_attacker(this->id) && !(this->status & (UnitStatus::Burrowed | UnitStatus::InBuilding))) {
        return WeaponId::None;
    } else {
        return units_dat::ground_weapon[this->id];
    }
}

WeaponId::Enum Unit::get_air_weapon() const {
    if (unit_id_is_burrowed_attacker(this->id) && !(this->status & (UnitStatus::Burrowed | UnitStatus::InBuilding))) {
        return WeaponId::None;
    }
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
        if (!max_hp) {
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

u32 Unit::get_max_weapon_range(WeaponId::Enum weapon_id) const {
    static u32 func_address = 0x00475870;
    assert(this);

    if (weapon_id == WeaponId::None) {
        return 0;
    }

    assert(weapon_id < WEAPON_TYPE_COUNT);

    static u32 max_weapon_range;
    __asm {
        PUSHAD
        MOV BX, weapon_id
        MOV EAX, this
        CALL func_address
        MOV max_weapon_range, EAX
        POPAD
    }
    return max_weapon_range;
}

const char* Unit::get_name() {
    auto& unit_ext = this->ext();
    if (unit_ext.custom_name) {
        return unit_ext.custom_name;
    }
    if (this->player < PlayerLimit) {
        const auto& custom_unit_names = g_globals->players[this->player].custom_unit_names;
        if (auto it = custom_unit_names.find(UnitId::Enum(this->id)); it != custom_unit_names.end()) {
            return it->second.data();
        }
    }
    return get_name(this->id);
}

// Based on function @ 0x0047B090
const char* Unit::get_name(u16 unit_id, bool ignore_map_name) {
    assert(unit_id < UnitsIdLimit);
    if (unit_id >= UnitsIdLimit) {
        return (char*)g_string_empty;
    }
    if (!ignore_map_name && units_dat::map_string_id[unit_id] != 0) {
        return (char*)g_map_string_tbl->get_string(units_dat::map_string_id[unit_id]);
    }
    if (DatExt::g_unit_names_tbl) {
        return (char*)((StringTbl*)&DatExt::g_unit_names_tbl)->get_string(unit_id + 1); // must be a pointer to a pointer, rather than just a pointer
    }
    return g_stat_txt_tbl_new->get_string(unit_id + 1);
}

RaceId::Enum Unit::get_race() const {
    assert(this);
    return get_race(this->id);
}

// Based on function at 0x0047B150
RaceId::Enum Unit::get_race(u16 unit_id) {
    assert(unit_id < UnitsIdLimit);
    if (units_dat::group_flags[unit_id].is_zerg) {
        return UnitId::is_bloodsummon((UnitId::Enum)unit_id) ? RaceId::Bloodsummon : RaceId::Zerg;
    } else if (units_dat::group_flags[unit_id].is_protoss) {
        return RaceId::Protoss;
    } else if (units_dat::group_flags[unit_id].is_terran) {
        return UnitId::is_askosi(static_cast<UnitId::Enum>(unit_id)) ? RaceId::Askosi : RaceId::Terran;
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
    if (this->move_target.pt.x != this->position.x || this->move_target.pt.y != this->position.y) {
        return MovableState_NotReachedDestination;
    } else {
        if (!(this->status & UnitStatus::Unmovable)) {
            return MovableState_ReachedDestination;
        }
        return MovableState_UnmovableAtDestination;
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
    Box16 box = {};
    box.left   = this->get_left();
    box.top    = this->get_top();
    box.right  = this->get_right();
    box.bottom = this->get_bottom();
    return box;
}

Box32 Unit::get_box32() const {
    Box32 box = {};
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

    hitpoints = std::clamp<s32>(hitpoints, 0, units_dat::max_hitpoints[this->id]);

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
    if (this->plague_timer) {
        value *= 0.5fp;
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
    if (this->plague_timer) {
        value *= 0.5fp;
    }

    this->shields = cap_value(this->shields + value, this->get_max_shields_raw(), &value);
    this->set_hitpoints(this->hitpoints); // Also updates unit strength

    // If heals to full, clear debuffs
    if (restore && this->shields == this->get_max_shields_raw()) {
    //	bw::restore_func(this);
    }

    // Activate flash shielding
    if (proc_flash_shielding) {
        this->ext().shield_regen_timer = UINT_MAX;
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

void Unit::damage_with(WeaponId::Enum weapon_id, s32 damage, Unit* attacker, u8 attacking_player, s8 direction, u8 damage_divisor) {
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
        MOVSX EBX, direction
        PUSH EBX
        MOVZX EBX, damage_divisor
        PUSH EBX
        MOVZX EBX, weapon_id
        PUSH EBX
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

void Unit::bwfunc_remove() {
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
    this->energy = std::clamp<u16>(this->energy + energy, 0, this->get_max_energy());
}

void Unit::add_unit_energy_capped(u32 energy, u32 energyMax) {
    this->energy = std::clamp<u16>(this->energy + energy, 0, energyMax);
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
void Unit::order_simple(u8 order_id, bool stop_previous_orders) {
    this->order(order_id, 0, 0, 0, UnitId::None, stop_previous_orders);
}

// Equivalent to 00474D10 issue_queued_order_target function
void Unit::issue_queued_order_target(u8 order_id, Unit* target, bool stop_previous_orders) {
    if (!target) {
        this->order(order_id, 0, 0, 0, UnitId::None, stop_previous_orders);
    } else {
        this->order(order_id, target->sprite->position.x, target->sprite->position.y, target, UnitId::None, stop_previous_orders);
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

void Unit::order(u8 order_id, u16 x, u16 y, Unit* target, u16 fow_unit_id, bool stop_previous_orders) {
    assert(this);
    const u32 Func_Order = 0x00474810;
    static v2u16 pos; pos.x = x, pos.y = y;
    __asm {
        PUSHAD
        MOVZX EBX, fow_unit_id
        PUSH EBX
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
    static v2u16 pos;
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
    graphics::ColorId color;
    if (unit != nullptr) {
        if (unit->sprite != nullptr) {
            // note: using the sprite player to preserve recruitable units color
            color = unit->sprite->player;
        } else {
            // note: using player color for spriteless units (if that exist)
            color = unit->player;
        }
        // for normal players, apply colors switching
        if (unit->player < PlayerLimit) {
            // get the proper player in multiplayer game (don't break campaigns/UMS)
            color = g_factions_colors_ordering[color];
            // fix BROWN to GREEN color swap in Desert map
            if (*g_current_tileset == TilesetType::Desert && color == 5) {
                color = 8;
            }
            // fix WHITE to GREEN color swap in Ice map
            if (*g_current_tileset == TilesetType::Ice && color == 6) {
                color = 8;
            }
        }
        if (color < FullPlayerLimit) {
            color = graphics::standardColors[color];
        } else {
            color = graphics::BLACK; // will return 0/black if abnormal player
        }
    } else {
        color = graphics::BLACK; // will return 0/black if null pointer is the argument
    }
    return color;
}

graphics::ColorId Unit::get_color() {
    assert(this);
    return get_color(this);
}

graphics::ColorId Unit::get_color(Unit* unit) {
    // get_color internal function
    graphics::ColorId return_value;
    if (unit) {
        return_value = unit->player; // Using player color for spriteless units (if that exist)
        if (unit->sprite) {
            return_value = unit->sprite->player; // Using the sprite player to preserve rescuable unit color
        }
        if (return_value < PlayerLimit) {
            const u32 func_address = 0x0049B0E0;
            __asm {
                PUSHAD
                MOV AL, return_value
                CALL func_address
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

u32 Unit::get_distance_to_point(v2u16 point) const {
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

s32 Unit::can_use_tech(u8 tech_id, u8 player) const {
    assert(this);
    const u32 func_address = 0x0046DD80;
    s32 player_id_xd = player;
    u32 tech_id_xd = tech_id;
    static s32 result;
    __asm {
        PUSHAD
        PUSH player_id_xd
        MOVZX EDI, tech_id
        MOV EAX, this
        CALL func_address
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

void Unit::fire_weapon(WeaponId::Enum weapon_id) const {
    assert(this);
    const u32 func_address = 0x00479C90;
    __asm {
        PUSHAD
        MOVZX EBX, weapon_id
        PUSH EBX
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
    if (give_unit_to_player(this, player)) {
        if ((this->id == UnitId::ZergNydusCesiant || !this->is_building())
            && this->id != UnitId::ZergEgg
            && this->id != UnitId::ZergLarva
            && this->id != UnitId::ZergOcsalLarva)
        {
            this->order_to_idle();
        }
        auto& unit_ext = this->ext();
        unit_ext.timers[TimerOwnerChangedRecently] = 2;
        unit_ext.values[ValueAlliedInvPlayer] = player;
        unit_ext.timers[TimerAlliedInvFrames] = 24;
        clear_maledictus_if_needed(this);
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
    if (player == NeutralHostilePlayer) {
        return true;
    }
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
        if (build_queue[build_queue_slot % 5] == UnitId::ZergIroleth) {
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
    if (unit_id_is_burrowed_attacker(this->id) && this->status & (UnitStatus::Burrowed | UnitStatus::InBuilding)) {
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
    if (!this->is_in_transport()) {
        return true;
    }

    if (this->is_garrisoned() && this->connected_unit) {
        if (this->connected_unit->is_in_air()) {
            return false;
        }
        return this->connected_unit->is_valid_garrison();
    }
    return false;
}

void Unit::issue_custom_order(u32 order_id) {
    switch (order_id) {
        case OrderId::GriefOfAllGods: {
            if (this->id == UnitId::ProtossStarSovereign) {
                this->order_computer(OrderId::GriefOfAllGods);
                this->order_target.pt = this->sprite->position;
                this->main_order_state = 0;
            }
        } break;
    }
}

bool Unit::burrowed_attacker_can_attack() {
    if (unit_id_is_burrowed_attacker(this->id)) {
        return this->status & (UnitStatus::Burrowed | UnitStatus::InBuilding);
    }
    return false;
}

void Unit::count_connected_larva() {
    *g_larva_counter = 0;
    Box16 coords = {};
    coords.left = this->sprite->position.x - 256;
    coords.top = this->sprite->position.y - 256;
    coords.right = this->sprite->position.x + 256;
    coords.bottom = this->sprite->position.y + 256;
    // Count existing larvas in g_larva_counter
    helper::iterate_units_at_location_target_proc_larva_count(this, &coords);
}

Faction Unit::faction() {
    return player < PlayerLimit ? g_globals->players[player].faction : Faction::None;
}

fp32 Unit::total_cost() {
    auto& unit_ext = this->ext();
    return bw::unit_id_total_cost(id, faction(), unit_ext.values[ValueRegressionUnstable]);
}

u32 Unit::standard_sprite_id() {
    if (this->id == UnitId::TerranBlackjack || this->id == UnitId::TerranBlackjackGolden) {
        auto& unit_ext = this->ext();
        if (unit_ext.values[ValueWearAndTear]) {
            return this->id == UnitId::TerranBlackjack ? SpriteId::TerranBlackjackDamaged : SpriteId::TerranBlackjackGoldenDamaged;
        } else {
            return this->id == UnitId::TerranBlackjack ? SpriteId::TerranBlackjack : SpriteId::TerranBlackjackGolden;
        }
    }

    u32 flingy_id = units_dat::graphic[this->id];
    u32 sprite_id = flingy_dat::sprite_id[flingy_id];
    return sprite_id;
}

bool Unit::is_building() {
    return units_dat::base_property[this->id] & UnitProperty::Building;
}

bool Unit::is_detector() {
    return units_dat::base_property[this->id] & UnitProperty::Detector;
}

bool Unit::is_neutral_accessory() {
    return units_dat::base_property[this->id] & UnitProperty::NeutralAccessories;
}

bool Unit::has_shields() {
    return units_dat::shields_enabled[this->id];
}

bool Unit::is_worker() {
    return units_dat::base_property[this->id] & UnitProperty::Worker;
}

bool Unit::is_resource_depot() {
    return units_dat::base_property[this->id] & UnitProperty::ResourceDepot;
}

bool Unit::is_addon() {
    return units_dat::base_property[this->id] & UnitProperty::Addon;
}

bool Unit::is_upgrading() {
    return this->is_building() ? this->building.upgrade_id != UpgradeId::None : false;
}

bool Unit::is_researching() {
    return this->is_building() ? this->building.tech_id != TechId::None : false;
}

bool Unit::is_upgrading_or_researching() {
    return this->is_upgrading() || this->is_researching();
}

bool Unit::is_completed() {
    return this->status & UnitStatus::Completed;
}

bool Unit::is_garrisoned() {
    return this->status & UnitStatus::InBuilding;
}

bool Unit::is_burrowed() {
    return this->status & UnitStatus::Burrowed;
}

bool Unit::can_burrow() {
    return units_dat::base_property[this->id] & UnitProperty::Burrowable;
}

bool Unit::is_computer_controlled() {
    return g_player_table[this->player].type == PlayerType::Computer;
}

bool Unit::is_in_air() {
    return this->status & UnitStatus::InAir;
}

bool Unit::is_flyer() {
    return units_dat::base_property[this->id] & UnitProperty::Flyer;
}

bool Unit::is_invincible() {
    return this->status & UnitStatus::Invincible || units_dat::base_property[this->id] & UnitProperty::Invincible;
}

bool Unit::is_spellcaster() {
    return units_dat::base_property[this->id] & UnitProperty::Spellcaster;
}

bool Unit::is_hallucination() {
    return this->status & UnitStatus::IsHallucination;
}

bool Unit::is_grounded_building() {
    return this->status & UnitStatus::GroundedBuilding;
}
bool Unit::is_in_transport() {
    return this->status & UnitStatus::InTransport;
}

bool Unit::is_dual_requisition() {
    return units_dat::base_property[this->id] & UnitProperty::TwoUnitsIn1Egg;
}

bool Unit::is_disabled() {
    return this->status & UnitStatus::Disabled;
}

SpriteId::Enum Unit::get_custom_corpse(SpriteId::Enum id) {
    switch (id) {
        case SpriteId::Corpse_Dracadin: {
            if (this->id == UnitId::ProtossDracadinOrder) {
                return SpriteId::Corpse_DracadinOrder;
            }
        } break;
        case SpriteId::Corpse_Hydralisk: {
            if (this->id == UnitId::ZergHydraliskOrkoti) {
                return SpriteId::Corpse_HydraliskOrkoti;
            }
        } break;
    }
    return id;
}

bool Unit::has_last_orders() {
    return this->id == UnitId::ProtossAurora
        || this->id == UnitId::ProtossSimulacrumKhalendric;
}

void Unit::autophagy_spawn() {
    auto& unit_ext = this->ext();
    if (unit_ext.timers[TimerAutophagySpawn]) {
        return;
    }
    u32 timer = this->id == UnitId::ZergVilgoleth ? Vilgoleth::SpawnDelay : Yblyleth::SpawnDelay;
    Thingy* corpse = get_closest_active_corpse(this->position, Vilgoleth::Range);
    if (corpse && corpse->sprite->id != SpriteId::Corpse_Rilirokor) {
        unit_ext.timers[TimerAutophagySpawn] = timer;
        v2u16 pos = corpse->sprite->position;
        bw::create_unit_from_ability(UnitId::ZergRilirokor, 2, this->player, pos);
        bw::create_and_init_thingy(SpriteId::ReconstitutionConsumption, pos.x, pos.y - 12, this->player, corpse->sprite->elevation + 1);
        consume_corpse(corpse);
        this->sprite->create_overlay(ImageId::LazarusAgentProc_Large);
    }
}

bool Unit::can_be_displaced() {
    return !this->is_building() || (this->is_in_air() && this->main_order_id != OrderId::Liftoff && this->main_order_id != OrderId::BuildingLand) && !this->is_burrowed();
}

bool Unit::is_plainly_visible() {
    if (this->is_burrowed()) {
        return false;
    }
    auto& unit_ext = this->ext();
    if ((bw::is_passive_decloakable(this) || bw::is_proximity_decloakable(this)) && !unit_ext.timers[TimerDecloak]) {
        return false;
    }
    return true;
}

bool Unit::can_use_passive_ability() {
    return this->is_completed() && !this->is_frozen() && !this->is_in_transport() && !this->ext().values[ValueDisablePassive] && !this->ext().timers[TimerSalvageStun];
}

u32 Unit::get_adaptive_amitosis_range() {
    u32 timer = this->ext().timers[TimerAdaptiveAmitosisGrowth];
    u32 radius = 2 * 32; // Initial range
    if (timer < 49) {
        radius = timer < 25 ? 6 * 32 : 4 * 32; // Grows to 6 range over time
    }
    return radius;
}

bool Unit::can_proc_autovitality(u32 attacking_player, u32 shield_reduce, u32 damage) {
    return !this->is_building()
        && !unit_id_is_larva(this->id)
        && !unit_id_is_zerg_egg(this->id)
        &&  this->id != UnitId::ZergVilgoleth
        &&  this->id != UnitId::VolatileEgg
        && !bw::is_allied_to(this->player, attacking_player)
        && !bw::is_under_carapace_swarm(this)
        && (shield_reduce >= this->shields || !this->has_shields())
        &&  damage
        &&  this->is_completed()
        &&  this->ext().timers[TimerAutovitalityClone] == 0;
}

bool Unit::can_be_passive_cloaked(Unit* source) {
    return this->is_completed() && this != source && !this->is_neutral_accessory() && this->id != UnitId::TerranMorningstar
        && bw::are_mutual_allies(this->player, source->player) && this->player != 11 && !this->is_frozen() && !this->is_burrowed();
}

bool Unit::can_transport_air() {
    return this->ext().values[ValueAirTransport] || this->id == UnitId::AskosiGrandstandHoldout || this->id == UnitId::BloodsummonGrimoire;
}

bool Unit::can_play_training_sfx() {
    if (!this->is_completed() && !unit_id_is_zerg_egg(this->id)) {
        return false;
    }
    return !this->is_frozen() && this->build_queue[this->build_queue_slot] != UnitId::None && !this->ext().timers[TimerAmbientSFX];
}

void Unit::play_training_sfx() {
    auto& unit_ext = this->ext();
    s32 delay = 0;
    SoundId::Enum sound_id = SoundId::None;
    switch (this->id) {
        case UnitId::TerranMinistry: {
            sound_id = SoundId::MinistryAmbient01;
            delay = TerranMinistry::AmbientSFXDelay;
        } break;
        case UnitId::TerranTreasury: {
            sound_id = SoundId::TreasuryAmbient01;
            delay = TerranTreasury::AmbientSFXDelay;
        } break;
        case UnitId::TerranStockade: {
            sound_id = SoundId::StockadeAmbient01;
            delay = TerranStockade::AmbientSFXDelay;
        } break;
        case UnitId::TerranFulcrum: {
            sound_id = SoundId::FulcrumAmbient01;
            delay = TerranFulcrum::AmbientSFXDelay;
        } break;
        case UnitId::TerranStarpad: {
            sound_id = SoundId::StarpadAmbient01;
            delay = TerranStarpad::AmbientSFXDelay;
        } break;

        case UnitId::ProtossNexus: {
            sound_id = SoundId::NexusAmbient01;
            delay = ProtossNexus::AmbientSFXDelay;
        } break;
        case UnitId::ProtossEmbassy: {
            sound_id = SoundId::EmbassyAmbient01;
            delay = ProtossEmbassy::AmbientSFXDelay;
        } break;
        case UnitId::ProtossGateway: {
            sound_id = SoundId::GatewayAmbient01;
            delay = ProtossGateway::AmbientSFXDelay;
        } break;
        case UnitId::ProtossGrandLibrary: {
            sound_id = SoundId::GrandLibraryAmbient01;
            delay = ProtossGrandLibrary::AmbientSFXDelay;
        } break;
        case UnitId::ProtossRogueGallery: {
            sound_id = SoundId::RogueGalleryAmbient01;
            delay = ProtossRogueGallery::AmbientSFXDelay;
        } break;
        case UnitId::ProtossProstrationStage: {
            sound_id = SoundId::ProstrationStageAmbient01;
            delay = ProtossProstrationStage::AmbientSFXDelay;
        } break;
        case UnitId::ProtossAncestralArchives: {
            sound_id = SoundId::AncestralArchivesAmbient01;
            delay = ProtossAncestralArchives::AmbientSFXDelay;
        } break;
        case UnitId::ProtossLattice: {
            sound_id = SoundId::LatticeAmbient01;
            delay = ProtossLattice::AmbientSFXDelay;
        } break;
        case UnitId::ProtossArdentAuthority: {
            sound_id = SoundId::ArdentAuthorityAmbient01;
            delay = ProtossArdentAuthority::AmbientSFXDelay;
        } break;
        case UnitId::ProtossUnitaryUnion: {
            sound_id = SoundId::UnitaryUnionAmbient01;
            delay = ProtossUnitaryUnion::AmbientSFXDelay;
        } break;
        case UnitId::ProtossStargate: {
            sound_id = SoundId::StargateAmbient01;
            delay = ProtossStargate::AmbientSFXDelay;
        } break;
        case UnitId::ProtossArgosy: {
            sound_id = SoundId::ArgosyAmbient01;
            delay = ProtossArgosy::AmbientSFXDelay;
        } break;
        case UnitId::ProtossMonumentOfSin: {
            sound_id = SoundId::MonumentofSinAmbient01;
            delay = ProtossMonumentOfSin::AmbientSFXDelay;
        } break;

        case UnitId::ZergEgg:
        case UnitId::ZergCocoon: {
            sound_id = static_cast<SoundId::Enum>(bw::rand_between(SoundId::EggAmbient01, SoundId::EggAmbient04));
            delay = bw::rand_between(ZergEgg::AmbientSFXDelayMin, ZergEgg::AmbientSFXDelayMax);
        } break;

        case UnitId::AskosiLandlock: {
            sound_id = SoundId::LandlockAmbient01;
            delay = AskosiLandlock::AmbientSFXDelay;
        } break;
        case UnitId::AskosiFunction: {
            sound_id = SoundId::FunctionAmbient01;
            delay = AskosiFunction::AmbientSFXDelay;
        } break;
        case UnitId::AskosiCosmoscope: {
            sound_id = SoundId::CosmoscopeAmbient01;
            delay = AskosiCosmoscope::AmbientSFXDelay;
        } break;
    }
    unit_ext.timers[TimerAmbientSFX] = delay;
    bw::play_sound(sound_id, this);
}

void Unit::play_completion_sfx() {
    SoundId::Enum sound_id = SoundId::None;
    switch (this->get_race()) {
        case RaceId::Terran: {
            sound_id =  SoundId::TerranUnitDeployment01;
        } break;
        case RaceId::Protoss: {
            sound_id = static_cast<SoundId::Enum>(bw::rand_between(SoundId::ProtossUnitDeployment01, SoundId::ProtossUnitDeployment04));
        } break;
        case RaceId::Askosi: {
            sound_id = SoundId::AskosiUnitDeployment01;
        } break;
    }
    bw::play_sound(sound_id, this);
}

SoundId::Enum Unit::get_error_sfx() {
    switch (this->get_race()) {
        case RaceId::Terran:  return SoundId::Misc_Buzz_wav_2;
        case RaceId::Protoss: return SoundId::Misc_PError_wav;
        case RaceId::Zerg:    return SoundId::Zerg_DRONE_ZDrErr00_WAV_2;
        case RaceId::Askosi:  return SoundId::Misc_Buzz_wav_2;
    }
    return SoundId::Misc_Buzz_wav_2;
}

Unit* Unit::corpse_cocoon_search() {
    for (Thingy* corpse : g_corpse_search.rect(this->get_box32())) {
        auto* corpse_ext = get_thingy_ext(corpse);
        if (!(corpse->sprite->flags & SpriteFlags::RemoveAfterDeath) && !unit_id_is_broodspawn(corpse_ext->unit_id)) {
            Unit* egg = bw::create_unit_in_game(UnitId::ZergEgg, this->player, corpse->sprite->position.x, corpse->sprite->position.y, 0, false);
            if (!egg) {
                continue;
            }
            auto unit_id = this->id;
            if (bw::is_allied_to(this->player, corpse->sprite->player)) {
                unit_id = units_dat::base_property[corpse->unit_id] & UnitProperty::Flyer ? UnitId::ZergRililisk : UnitId::ZergRilirokor;
            }
            auto time_cost = units_dat::time_cost[unit_id] / 2;
            if (units_dat::base_property[unit_id] & UnitProperty::TwoUnitsIn1Egg) {
                time_cost /= 2;
            }
            set_custom_egg(egg, unit_id, time_cost);
            auto& egg_ext = egg->ext();
            egg_ext.values[ValueNoCorpse] = true;
            egg_ext.values[ValueRegressionUnstable] = true;
            corpse->sprite->flags |= SpriteFlags::RemoveAfterDeath;
            corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
            return egg;
        }
    }
    return nullptr;
}

bool Unit::is_valid_nydus() {
    auto& unit_ext = this->ext();
    return (unit_ext.values[ValueIsNydusEntry] || unit_ext.values[ValueIsNydusExit]) && this->building.nydus_exit;
}

bool Unit::is_rescuable() {
    u32 player_type = g_player_table[this->player].type;
    return player_type == PlayerType::Rescuable || player_type == PlayerType::UnusedRescueActive;
}

bool Unit::recharges_shields() {
    // Maybe we add this mechanic back in later, for Golden Republic Terrans or some other shield-user. -Pr0nogo
    return false;
}

bool Unit::is_constructor() {
    return this->is_worker() || this->id == UnitId::TerranOgunWorkerMode || this->id == UnitId::AskosiDyad;
}

bool Unit::is_askosi_embedded() {
    return this->ext().values[ValueReassemblyActive];
}

void Unit::pull_towards(Unit* source) {
    auto& pull_ext = this->ext();
    pull_ext.units[UnitPullTowardsSource] = source;
    const u32 displacement_time = 5;
    v2 diff = v2(source->sprite->position).vec_to(v2(this->sprite->position));
    if (diff.len() > 12.0fp) {
        v2 displacement = diff.norm();
        displacement *= -32.0fp / fp32(displacement_time);
        pull_ext.timers[TimerPullTowards] = displacement_time;
        pull_ext.repulsion_vector = displacement;
        pull_ext.pull_pos = source->position;
    }
}

void Unit::tick_burrowed_cagra_spread() {
    if (this->order_signal & 4) {
        if (!this->is_burrowed()) {
            bw::spawn_custom_creep(this);
        } else {
            bw::update_custom_creep_disappearance(this);
        }
    }
}

void Unit::proc_booming_blast(bool damage_self) {
    auto& unit_ext = this->ext();
    u32 attacking_player = unit_ext.values[ValueBoomingBlastPlayer];
    unit_ext.values[ValueBoomingBlastPlayer] = 0;
    unit_ext.timers[TimerBoomingBlast] = 0;
    if (damage_self) {
        this->damage_with(WeaponId::BoomingBlast, -1, 0, attacking_player);
    }
    for (Unit* nearby : g_unit_search.radial_ex(this, 2 * 32, FilterNotSelf)) {
        nearby->damage_with(WeaponId::BoomingBlast, -1, 0, attacking_player);
        nearby->sprite->create_top_overlay(ImageId::ArchitectWeaponChannel); // TODO: Own visuals
    }
    // TODO: Own audiovisuals
    this->sprite->create_top_overlay(ImageId::ArchitectWeaponChannel);
    bw::play_sound({SoundId::AskosiDeploy01, SoundId::AskosiDeploy03}, this->position);
}

bool Unit::is_valid_garrison() {
    if (unit_id_is_ground_garrison(this->id) && this->status & (UnitStatus::InAir | UnitStatus::InTransport)) {
        return false;
    }
    if (!unit_id_is_garrison(this->id)) {
        return false;
    }
    if (this->id == UnitId::ZergScrylyleth && !this->is_burrowed()) {
        return false;
    }
    if (this->id == UnitId::AskosiPetract && !this->ext().values[ValueReassemblyActive]) {
        return false;
    }
    return true;
}

void Unit::cast_rematerialize() {
    auto& unit_ext = this->ext();
    auto& try_cast = unit_ext.values[ValueTryCastCustomSpell];
    if (try_cast && (this->main_order_id != OrderId::AskosiRematerialize || this->is_frozen())) {
        try_cast = false;
    }
    if (!try_cast || unit_ext.timers[TimerTryCastCustomSpell]) {
        return;
    }
    Unit* target = this->order_target.unit;
    if (!target) {
        this->order_to_idle();
        return;
    }
    if (!bw::are_mutual_allies(this->player, target->player) || target->is_building()) {
        this->order_to_idle();
        helper::display_error_message_id_with_sfx(this->player, 968, get_racial_error_sfx(RaceId::Enum(g_player_table[this->player].race)));
        return;
    }
    v2u16 target_pos{this->order_target.unit->get_x(), this->order_target.unit->get_y()};
    u32 range = Askosi::RematerializeCastRange;
    if (!helper::is_target_within_min_range(this, range, target)) {
        if (this->order_target.unit) {
            helper::spell_order_move_towards_target(this, range);
        } else {
            helper::set_move_target_xy(this, target_pos.x, target_pos.y);
        }
        unit_ext.timers[TimerTryCastCustomSpell] = 3;
        return;
    }
    u16 energy_cost = 0;
    if (helper::spell_order(this, TechId::AskosiRematerialize, &energy_cost, range, 1330)) {
        bw::decloak_passive_proc(this);
        this->spend_energy(energy_cost);
        for (Unit* ally : g_unit_search.radial_ex(target, Askosi::RematerializeTeleportRange, FilterAlly | FilterIncludingSelf | FilterNoBuilding)) {
            if (ally == this) {
                continue;
            }
            bw::teleport_to_unit(ally, this, 24, {.check_distance = false}, SpriteId::LegionnaireTeleportEffect);
            ally->ext().timers[TimerImpactDissipation] = Askosi::ImpactDissipationDuration;
            ally->ext().values[ValueImpactDissipationBlock] = 1;
            ally->update_speed();
            bw::add_unit_overlay(ally, ImageId::ImpactDissipationActive);
        }
        try_cast = false;
    }
}

bool Unit::cast_rapid_rendezvous() {
    auto& unit_ext = this->ext();
    auto& try_cast = unit_ext.values[ValueTryCastCustomSpell];
    if (try_cast && (this->main_order_id != OrderId::AskosiRapidRendezvous || this->is_frozen())) {
        try_cast = false;
    }
    if (!try_cast || unit_ext.timers[TimerTryCastCustomSpell]) {
        return false;
    }
    Unit* target = this->order_target.unit;
    if (!target) {
        this->order_to_idle();
        return false;
    }
    if (!bw::are_mutual_allies(this->player, target->player) || target->is_in_air()) {
        this->order_to_idle();
        helper::display_error_message_id_with_sfx(this->player, 969, get_racial_error_sfx(RaceId::Enum(g_player_table[this->player].race)));
        return false;
    }
    v2u16 target_pos{this->order_target.unit->get_x(), this->order_target.unit->get_y()};
    u32 range = Askosi::RapidRendezvousCastRange;
    if (!helper::is_target_within_min_range(this, range, target)) {
        if (this->order_target.unit) {
            helper::spell_order_move_towards_target(this, range);
        } else {
            helper::set_move_target_xy(this, target_pos.x, target_pos.y);
        }
        unit_ext.timers[TimerTryCastCustomSpell] = 3;
        return false;
    }
    u16 energy_cost = 0;
    if (helper::spell_order(this, TechId::AskosiRapidRendezvous, &energy_cost, range, 1330)) {
        bw::decloak_passive_proc(this);
        this->spend_energy(energy_cost);
        bw::teleport_to_unit(this, target, 24, {.check_distance = false}, SpriteId::LegionnaireTeleportEffect);
        unit_ext.timers[TimerImpactDissipation] = Askosi::ImpactDissipationDuration;
        unit_ext.values[ValueImpactDissipationBlock] = 1;
        bw::add_unit_overlay(this, ImageId::ImpactDissipationActive);
        for (Unit* ally : g_unit_search.radial_ex(target, Askosi::RapidRendezvousBuffRange, FilterAlly | FilterIncludingSelf)) {
            if (ally == this) {
                continue;
            }
            ally->ext().timers[TimerImpactDissipation] = Askosi::ImpactDissipationDuration;
            ally->ext().values[ValueImpactDissipationBlock] = 1;
            ally->update_speed();
            bw::add_unit_overlay(ally, ImageId::ImpactDissipationActive);
        }
        try_cast = false;
        return true;
    }
}

void Unit::cast_hijack() {
    auto& unit_ext = this->ext();
    auto& try_cast = unit_ext.values[ValueTryCastCustomSpell];
    if (try_cast && (this->main_order_id != OrderId::AskosiHijack || this->is_frozen()) || unit_ext.units[UnitHijackTarget]) {
        try_cast = false;
    }
    if (!try_cast || unit_ext.timers[TimerTryCastCustomSpell]) {
        return;
    }
    Unit* target = this->order_target.unit;
    if (!target) {
        this->order_to_idle();
        return;
    }
    if (target->is_building() || (bw::are_mutual_allies(this->player, target->player) && !target->is_askosi_embedded())) {
        this->order_to_idle();
        helper::display_error_message_id_with_sfx(this->player, 970, get_racial_error_sfx(RaceId::Enum(g_player_table[this->player].race)));
        return;
    }
    v2u16 target_pos{this->order_target.unit->get_x(), this->order_target.unit->get_y()};
    u32 range = Askosi::HijackCastRange;
    if (!helper::is_target_within_min_range(this, range, target)) {
        if (this->order_target.unit) {
            helper::spell_order_move_towards_target(this, range);
        } else {
            helper::set_move_target_xy(this, target_pos.x, target_pos.y);
        }
        unit_ext.timers[TimerTryCastCustomSpell] = 3;
        return;
    }
    auto& target_ext = target->ext();
    if (target_ext.units[UnitHijacker]) {
        this->order_to_idle();
        try_cast = false;
        helper::display_error_message_id_with_sfx(this->player, 971, get_racial_error_sfx(RaceId::Enum(g_player_table[this->player].race)));
        return;
    }
    u16 energy_cost = 0;
    if (helper::spell_order(this, TechId::AskosiHijack, &energy_cost, range, 1330)) {
        bw::decloak_passive_proc(this);
        this->status |= UnitStatus::NoCollide;
        this->status |= UnitStatus::CanNotReceiveOrders;
        bw::teleport_to_unit(this, target, 24, {.check_distance = false}, SpriteId::LegionnaireTeleportEffect);
        target_ext.timers[TimerIsBeingHijacked] = Askosi::HijackTimer;
        target_ext.units[UnitHijacker] = this;
        unit_ext.units[UnitHijackTarget] = target;
        try_cast = false;
    }
}

void Unit::cast_remote_hijack() {
    auto& unit_ext = this->ext();
    auto& try_cast = unit_ext.values[ValueTryCastCustomSpell];
    if (try_cast && (this->main_order_id != OrderId::AskosiRemoteHijack || this->is_frozen())) {
        try_cast = false;
    }
    if (!try_cast || unit_ext.timers[TimerTryCastCustomSpell]) {
        return;
    }
    Unit* target = this->order_target.unit;
    if (!target) {
        this->order_to_idle();
        return;
    }
    if (target->is_building() || (bw::are_mutual_allies(this->player, target->player) && !target->is_askosi_embedded())) {
        this->order_to_idle();
        helper::display_error_message_id_with_sfx(this->player, 970, get_racial_error_sfx(RaceId::Enum(g_player_table[this->player].race)));
        return;
    }
    v2u16 target_pos{this->order_target.unit->get_x(), this->order_target.unit->get_y()};
    u32 range = Askosi::RemoteHijackCastRange;
    if (!helper::is_target_within_min_range(this, range, target)) {
        if (this->order_target.unit) {
            helper::spell_order_move_towards_target(this, range);
        } else {
            helper::set_move_target_xy(this, target_pos.x, target_pos.y);
        }
        unit_ext.timers[TimerTryCastCustomSpell] = 3;
        return;
    }
    auto& target_ext = target->ext();
    if (target_ext.units[UnitHijacker]) {
        this->order_to_idle();
        try_cast = false;
        helper::display_error_message_id_with_sfx(this->player, 971, get_racial_error_sfx(RaceId::Enum(g_player_table[this->player].race)));
        return;
    }
    u16 energy_cost = 0;
    if (helper::spell_order(this, TechId::AskosiRemoteHijack, &energy_cost, range, 1330)) {
        bw::decloak_passive_proc(this);
        this->spend_energy(energy_cost);
        fire_bullet(this, WeaponId::RemoteHijack, this->order_target);
        bw::play_sound(SoundId::ParacleteCast01, this->position);
        this->order_to_idle();
        try_cast = false;
    }
}

void Unit::cast_preylock() {
    auto& unit_ext = this->ext();
    auto& try_cast = unit_ext.values[ValueTryCastCustomSpell];
    if (try_cast && (this->main_order_id != OrderId::AskosiPreylock || this->is_frozen())) {
        try_cast = false;
    }
    if (!try_cast || unit_ext.timers[TimerTryCastCustomSpell]) {
        return;
    }
    Unit* target = this->order_target.unit;
    if (!target) {
        this->order_to_idle();
        return;
    }
    if (bw::are_mutual_allies(this->player, target->player) || target->is_building()) {
        this->order_to_idle();
        helper::display_error_message_id_with_sfx(this->player, 967, get_racial_error_sfx(RaceId::Enum(g_player_table[this->player].race)));
        return;
    }
    v2u16 target_pos{this->order_target.unit->get_x(), this->order_target.unit->get_y()};
    u32 range = Askosi::PreylockCastRange;
    if (!helper::is_target_within_min_range(this, range, target)) {
        if (this->order_target.unit) {
            helper::spell_order_move_towards_target(this, range);
        } else {
            helper::set_move_target_xy(this, target_pos.x, target_pos.y);
        }
        unit_ext.timers[TimerTryCastCustomSpell] = 3;
        return;
    }
    u16 energy_cost = 0;
    if (helper::spell_order(this, TechId::AskosiPreylock, &energy_cost, range, 1330)) {
        bw::decloak_passive_proc(this);
        this->spend_energy(energy_cost);
        bw::teleport_to_unit(target, this, 24, {.check_distance = false}, SpriteId::LegionnaireTeleportEffect);
        auto& target_ext = target->ext();
        if (!target_ext.timers[TimerBoomingBlast]) {
            target_ext.timers[TimerBoomingBlast] = Askosi::BoomingBlastTimer;
            target_ext.values[ValueBoomingBlastPlayer] = this->player;
        }
        try_cast = false;
    }
}

void Unit::cast_lobotomy_warhead() {
    auto& unit_ext = this->ext();
    auto& try_cast = unit_ext.values[ValueTryCastCustomSpell];
    if (try_cast && (this->main_order_id != OrderId::LobotomyWarhead || this->is_frozen())) {
        try_cast = false;
    }
    if (!try_cast || unit_ext.timers[TimerTryCastCustomSpell]) {
        return;
    }
    Unit* target = this->order_target.unit;
    if (!target) {
        try_cast = false;
        return;
    }
    v2u16 target_pos{this->order_target.unit->get_x(), this->order_target.unit->get_y()};
    u32 range = TerranTriton::LobotomyWarheadRange;
    if (!helper::is_target_within_min_range(this, range, target)) {
        if (this->order_target.unit) {
            helper::spell_order_move_towards_target(this, range);
        } else {
            helper::set_move_target_xy(this, target_pos.x, target_pos.y);
        }
        unit_ext.timers[TimerTryCastCustomSpell] = 3;
        return;
    }
    u16 energy_cost = 0;
    if (helper::spell_order(this, TechId::TritonLobotomyWarhead, &energy_cost, range, 1330)) {
        bw::decloak_passive_proc(this);
        this->spend_energy(energy_cost);
        fire_bullet(this, WeaponId::TritonLobotomyWarhead, this->order_target);
        try_cast = false;
    }
}

void Unit::hijack(Unit* hijacker) {
    this->set_hitpoints(std::max((u32)100 * 256, this->get_current_hp_ingame() * 256));
    auto& unit_ext = this->ext();
    unit_ext.values[ValueHijackOriginalPlayer] = this->player;
    this->give_to(hijacker->player);
    helper::hide_and_disable_unit(hijacker);
    // TODO: Own audiovisuals
    bw::play_sound({SoundId::AskosiDeploy01, SoundId::AskosiDeploy03}, this->position);
    this->sprite->create_top_overlay(ImageId::ArchitectWeaponChannel);
}

void Unit::proc_askosi_embed() {
    this->ext().values[ValueReassemblyActive] = true;
    this->heal_hp(this->get_max_hp_raw());
    this->play_iscript_animation(IscriptAnimation::SpecialState1);
    this->update_speed();
    bw::play_sound({SoundId::AskosiDeploy01, SoundId::AskosiDeploy03}, this->position);
    this->sprite->create_top_overlay(ImageId::ArchitectWeaponChannel);
    if (this->id == UnitId::AskosiObject) {
        this->status |= UnitStatus::CanNotReceiveOrders;
    }
}

void Unit::cancel_psi_storm() {
    if (bw::get_generic_rust_value(this, RustId::CancelPsiStorm)) {
        bw::set_generic_rust_value(this, RustId::CancelPsiStorm, 0);
        auto& unit_ext = this->ext();
        if (auto effect = g_iquare_globals->get_effect(unit_ext.current_storm)) {
            effect->destroy();
        }
        unit_ext.current_storm = 0;
        this->spell_cooldown = u8(hooks::get_modified_weapon_cooldown(this, WeaponId::PsiStorm) * 1.25fp);
    }
}

void Unit::proc_ensnaring_brood() {
    if (this->is_frozen()) {
        return;
    }
    for (Unit* target : g_unit_search.radial_ex(this, Matraleth::EnsnaringBroodRange, FilterAlly | FilterNoBuilding | FilterCompleted)) {
        if (!unit_is_combatant(target)) {
            continue;
        }
        auto& target_ext = target->ext();
        target_ext.timers[TimerEnsnaringBrood] = 3;
        if (!target->is_burrowed()) {
            bw::add_unit_overlay(target, ImageId::EnsnaringBroodOverlay);
        }
    }
}

bool Unit::proc_sermon_salve_cloak(Unit* cloaker) {
    auto& unit_ext = this->ext();
    if (bw::unit_in_combat_within_seconds(this, 2)) {
        return false;
    }
    if (this->is_building() || !this->can_be_passive_cloaked(cloaker) || unit_ext.timers[TimerDecloak] || unit_ext.values[ValueSermonSalveCloak]) {
        return false;
    }
    unit_ext.values[ValueSermonSalveCloak] = true;
    unit_ext.timers[TimerDecloak] = 3;
    helper::secondary_order_cloak(this);
    if (!(this->status & UnitStatus::CloakingForFree)) {
        this->status |= UnitStatus::CloakingForFree;
    }
    return true;
}

void Unit::cast_puppeteer(Unit* victim) {
    if (!bw::get_generic_rust_value(this, RustId::CastPuppeteer)) {
        return;
    }
    bw::set_generic_rust_value(this, RustId::CastPuppeteer, 0);
    auto energy_cost = TechId::energy_cost(TechId::AskosiPuppeteer);
    if (this->energy >= energy_cost && victim) {
        // TODO: Own audiovisuals
        bw::play_sound({ SoundId::AkrenoPassiveProc01, SoundId::AkrenoPassiveProc03 }, victim->position);
        Thingy* recall_effect_sprite = helper::create_thingy(SpriteId::LegionnaireTeleportEffect, victim->position.x, victim->position.y, 0);
        if (recall_effect_sprite) {
            recall_effect_sprite->sprite->elevation = this->sprite->elevation + 1;
            helper::set_thingy_visibility_flags(recall_effect_sprite);
        }

        bw::decloak_passive_proc(this);
        this->spend_energy(energy_cost);
        for (Unit* target : g_unit_search.radial_ex(victim, Askosi::PuppeteerPullRange)) {
            if (bw::is_allied_to(target->player, this->player)) {
                continue;
            }
            if (unit_can_be_repulsed(target)) {
                target->pull_towards(victim);
            }
        }
        victim->ext().timers[TimerStun] = 1secs;
        victim->order_to_idle();
        this->ext().units[UnitPuppeteerTarget] = 0;
    }
}

void Unit::cast_ashbind() {
    auto& unit_ext = this->ext();
    auto& try_cast_ref = unit_ext.values[ValueTryCastCustomSpell];
    if (try_cast_ref && (this->main_order_id != OrderId::AskosiAshbind || this->is_frozen())) {
        try_cast_ref = false;
    }
    if (!try_cast_ref || unit_ext.timers[TimerTryCastCustomSpell]) {
        return;
    }
    Unit* target = this->order_target.unit;
    if (!target) {
        this->order_to_idle();
        return;
    }
    if (bw::are_mutual_allies(this->player, target->player) || target->is_in_air()) {
        this->order_to_idle();
        helper::display_error_message_id_with_sfx(this->player, 967, get_racial_error_sfx(RaceId::Enum(g_player_table[this->player].race)));
        return;
    }
    v2u16 target_pos{this->order_target.unit->get_x(), this->order_target.unit->get_y()};
    u32 range = Askosi::RapidRendezvousCastRange;
    if (!helper::is_target_within_min_range(this, range, target)) {
        if (this->order_target.unit) {
            helper::spell_order_move_towards_target(this, range);
        } else {
            helper::set_move_target_xy(this, target_pos.x, target_pos.y);
        }
        unit_ext.timers[TimerTryCastCustomSpell] = 3;
        return;
    }
    u16 energy_cost = 0;
    if (helper::spell_order(this, TechId::AskosiAshbind, &energy_cost, range, 1330)) {
        bw::decloak_passive_proc(this);
        this->spend_energy(energy_cost);
        bw::teleport_to_unit(this, target, 24, {.check_distance = false}, SpriteId::LegionnaireTeleportEffect);
        for (Unit* enemy : g_unit_search.radial_ex(target, Askosi::AshbindDebuffRange, FilterIncludingSelf)) {
            if (bw::are_mutual_allies(this->player, enemy->player)) {
                continue;
            }
            auto& enemy_ext = enemy->ext();
            enemy_ext.timers[TimerAshHeap] = Askosi::ScrapworldAshHeapDuration;
            enemy_ext.timers[TimerAshHeapArmorRend] += Askosi::ScrapworldAshHeapDuration;
            enemy_ext.values[ValueAshHeapPlayer] = this->player;
        }
        try_cast_ref = false;
        return;
    }
}

void Unit::tick_mimicry() {
    if (!this->can_use_passive_ability()) {
        return;
    }
    for (Unit* ally : g_unit_search.radial(this->position, Askosi::MimicryRange).filter(this, FilterAlly | FilterCombatant)) {
        ally->ext().timers[TimerMimicryCheck] = 3;
        if (ally->subunit && ally->subunit->is_subunit()) {
            ally->subunit->ext().timers[TimerMimicryCheck] = 3;
        }
    }
}

void Unit::tick_vital_remains() {
    auto& unit_ext = this->ext();
    if (unit_ext.timers[TimerCorpseConsumptionDelay]) {
        return;
    }
    bool consumed = false;
    for (Thingy* corpse : g_corpse_search.radial(this->position, 32)) {
        bw::create_and_init_thingy(SpriteId::ReconstitutionConsumption, corpse->sprite->position.x, corpse->sprite->position.y);
        corpse->sprite->flags |= SpriteFlags::RemoveAfterDeath;
        corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
        bw::play_sound({ SoundId::AskosiScrap01, SoundId::AskosiScrap03 }, corpse->sprite->position);
        consumed = true;
        unit_ext.timers[TimerCorpseConsumptionDelay] = 1secs;
        return;
    }
    if (consumed) {
        for (Unit* ally : g_unit_search.radial(this->position, Askosi::VitalRemainsRange).filter(this, FilterAlly | FilterCompleted | FilterNoBuilding)) {
            if (ally->hitpoints < ally->get_max_hp_raw()) {
                ally->heal_hp(Askosi::VitalRemainsHeal, this);
                helper::add_overlay_below_main(ally->sprite, ImageId::HealingPulseUnderlay, 0, 0, 0);
            }
            ally->ext().timers[TimerVitalRemainsArmorPen] = Askosi::VitalRemainsArmorPenDuration;
        }
    }
}

bool Unit::has_coagulation() {
    return this->id == UnitId::BloodsummonThrombocyte;
}

void Unit::cast_bloody_burst() {
    if (!bw::get_generic_rust_value(this, RustId::CastBloodyBurst)) {
        return;
    }
    bw::set_generic_rust_value(this, RustId::CastBloodyBurst, 0);
    for (Unit* ally : g_unit_search.radial_ex(this, Bloodsummon::BloodyBurstRange, FilterAlly | FilterCompleted)) {
        ally->heal_hp_then_shields(this->hitpoints, this);
        helper::add_overlay_below_main(ally->sprite, ImageId::SmallExplosion_Unused, 0, 0, 0);
    }
    // TODO: Own audiovisuals
    helper::create_thingy(SpriteId::ZergAirExplosionLarge, this->position.x, this->position.y, this->player);
    bw::play_sound({ SoundId::AkrenoPassiveProc01, SoundId::AkrenoPassiveProc03 }, this->position);
    this->bwfunc_remove();
}

bool Unit::check_neutral_garrison_death() {
    auto& unit_ext = this->ext();
    for (u32 i = 0; i < TransportCap; ++i) {
        unload_unit_on_transport_death(this, i);
    }
    if (unit_ext.timers[TimerStun]) {
        return true; // Kill the neutral garrison if it takes fatal damage while stunned. -Pr0nogo
    }
    bw::play_sound(SoundId::Misc_Explo1_wav, this->position);
    helper::create_thingy(SpriteId::CuirassStonewallProtocolOverlay, this->position.x, this->position.y, this->player);
    unit_ext.timers[TimerStun] = 10secs + 1;
    unit_ext.timers[TimerZapOverlay] = 10secs + 1;
    this->set_hitpoints(this->get_max_hp_raw());
    return false;
}

void Unit::apply_ash_heap(u32 attacking_player) {
    auto& unit_ext = this->ext();
    unit_ext.timers[TimerAshHeap] = Askosi::ScrapworldAshHeapDuration;
    unit_ext.timers[TimerAshHeapArmorRend] += Askosi::ScrapworldAshHeapDuration;
    unit_ext.values[ValueAshHeapPlayer] = attacking_player;
}

void Unit::apply_booming_blast(u32 attacking_player) {
    auto& unit_ext = this->ext();
    if (!unit_ext.timers[TimerBoomingBlast]) {
        unit_ext.timers[TimerBoomingBlast] = Askosi::BoomingBlastTimer;
        unit_ext.values[ValueBoomingBlastPlayer] = attacking_player;
    }
}

void Unit::proc_nanite_field() { // Maybe search_nanite_field? -Pr0nogo
    auto& unit_ext = this->ext();
    if (this->is_frozen() || this->is_in_transport() || unit_ext.values[ValueDisablePassive]) {
        return;
    }
    u32 timer = unit_ext.timers[TimerNaniteField];
    if (timer) {
        return;
    }
    unit_ext.timers[TimerNaniteField] = 1secs;
    bw::add_unit_overlay(this, ImageId::RestorationHit_Small + bw::get_overlay_offset(this->id));

    u32 range = this->id == UnitId::TerranShamanVisionaryMode ? Shaman::NaniteFieldVisionaryRange : Shaman::NaniteFieldSupportRange;
    for (Unit* target : g_unit_search.radial(this->position, range).add_units_from_connected_transport(this).filter(this, FilterAlly | FilterNoBuilding)) {
        auto& target_ext = target->ext();
        if (!target_ext.timers[TimerNaniteFieldLockout]) {
            target_ext.timers[TimerNaniteFieldLockout] = Shaman::NaniteFieldDuration;
            proc_nanite_field_heal(this, Shaman::NaniteFieldHealAmount, target);
        }
    }
}

void Unit::proc_adrenaline_field() {
    auto& unit_ext = this->ext();
    if (this->is_frozen() || this->is_in_transport() || unit_ext.values[ValueDisablePassive]) {
        return;
    }
    u32 timer = unit_ext.timers[TimerNaniteField];
    if (timer) {
        return;
    }
    unit_ext.timers[TimerNaniteField] = 10secs;
    bw::add_unit_overlay(this, ImageId::RestorationHit_Small + bw::get_overlay_offset(this->id));

    for (Unit* target : g_unit_search.radial(this->position, Shaman::NaniteFieldSupportRange).filter(this, FilterAlly | FilterNoBuilding)) {
        if (target->hitpoints > target->get_stimpack_hp_cost() && target->stim_timer == 0 && target->ext().timers[TimerStimpackDelay] <= 1) {
            proc_stimpack_internal(target);
        }
    }
}

s32 Unit::get_stimpack_hp_cost() {
    switch (this->id) {
        case UnitId::TerranMaverickUpstart: return UpstartMaverick::StimpackMinimumHP;
    }
    return Terran::StimpackMinimumHP;
}