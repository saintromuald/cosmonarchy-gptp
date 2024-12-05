#include "weapon_range.h"
#include "hooks/cosmonarchy_finder.h"
#include <BW/api.h>
#include "globals.h"
namespace {
u32 estimate_max_weapon_range(Unit* unit, Unit* target) {
    if (target) {
        const bool in_air = target->status & UnitStatus::InAir;
        if (!in_air && unit->get_ground_weapon() != WeaponId::None) {
            return weapons_dat::max_range[unit->get_ground_weapon()];
        }
        if (in_air && unit->get_air_weapon() != WeaponId::None) {
            return weapons_dat::max_range[unit->get_air_weapon()];
        }
    }
    if (unit->get_ground_weapon() != WeaponId::None && unit->get_air_weapon() != WeaponId::None) {
        return std::max(weapons_dat::max_range[unit->get_ground_weapon()], weapons_dat::max_range[unit->get_air_weapon()]);
    }
    if (unit->get_ground_weapon() != WeaponId::None) {
        return weapons_dat::max_range[unit->get_ground_weapon()];
    }
    if (unit->get_air_weapon() != WeaponId::None) {
        return weapons_dat::max_range[unit->get_air_weapon()];
    }
    return 0;
}
} // namespace

namespace hooks {
// Returns the modified seek distance (AKA target acquisition distance) for the unit.
// This hook affects the behavior of Unit::get_seek_range().
// Equivalent to GetTargetAckRangeUpgrade @ 0x00476000  
u8 get_seek_range(Unit* unit, Unit* target) { // seek distance is measured in tiles, unfortunately
    UnitExt* unit_ext = get_unit_ext(unit);

    s16 bonus_amount = 0;
    if (bw::get_game_kind() == GameKind::DarkAges) {
        bonus_amount -= 1;
    }

    if (target) {
        UnitExt* target_ext = get_unit_ext(target);

        // Stellar Enforcement - Magister behavior
        if (target_ext->timers[TimerStellarEnforcement]) {
            bonus_amount += 2;
        }
    }

    f32 scale = 1;
    // Sovroleth - Writhing Web
    if (unit->id == UnitId::ZergSovroleth) {
        scale += 0.05 * unit_ext->values[ValueSovrolethLinkCount];
    }

    // Height advantage
    bonus_amount += bw::get_cliff_advantage(unit, target, 0, true);

    // Ignore bonus distance for melee units
    u32 weapon_range = estimate_max_weapon_range(unit, target);
    if (weapon_range <= 32 || unit_id_is_trap(unit->id)) {
        scale = 1;
        bonus_amount = 0;
    }

    u32 final_range = units_dat::seek_range[unit->id];
    if (unit_ext->values[ValueTriggerWeaponRangeBaseSet]) {
        final_range = unit_ext->values[ValueTriggerWeaponRangeBaseAmount];
    }
    if (target && target->status & UnitStatus::InAir) {
        u32 weapon_id = unit->get_air_weapon();
        if (unit_id_has_custom_air_weapon(unit->id)) {
            weapon_id = bw::get_custom_secondary_weapon_from_unit_id(unit->id);
        }
        if (weapon_id != WeaponId::None && weapons_dat::max_range[weapon_id] / 32 < final_range) {
            final_range = weapons_dat::max_range[weapon_id] / 32;
        }
    }

    return final_range * scale + bonus_amount;
};

// Returns the modified max distance for the weapon, which is assumed to be attached to the given unit.
// This hook affects the behavior of Unit::get_max_weapon_range().
// Note: Weapon ranges are measured in pixels.
u32 get_max_weapon_range(Unit* unit, u8 weapon_id, Unit* target, bool ignore_cliff_advantage) {
    UnitExt* unit_ext = get_unit_ext(unit);

    s32 bonus_amount = 0;
    if (bw::get_game_kind() == GameKind::DarkAges) {
        bonus_amount -= 32;
    }

    // Stellar Enforcement - Magister behavior
    if (target) {
        UnitExt* target_ext = get_unit_ext(target);
        if (target_ext->timers[TimerStellarEnforcement]) {
            bonus_amount += 64;
        }
    }

    f32 scale = 1;
    // Sovroleth - Writhing Web
    if (unit->id == UnitId::ZergSovroleth) {
        scale += 0.05 * unit_ext->values[ValueSovrolethLinkCount];
    }

    // Height advantage
    if (!ignore_cliff_advantage) {
        bonus_amount += bw::get_cliff_advantage(unit, target, weapon_id, false) * 32;
    }

    // Ignore bonus distance for melee units
    if (weapons_dat::max_range[weapon_id] <= 32 || unit_id_is_trap(unit->id)) {
        scale = 1;
        bonus_amount = 0;
    }

    // Give bonus distance to garrisoned units - after melee unit calcs so flamethrower units can attack
    if (unit->status & UnitStatus::InBuilding) {
        bonus_amount += 64;
    }

    if (unit_ext->timers[TimerAuralAssaultBlind] > 1) {
        return std::min(weapons_dat::max_range[weapon_id], 64u);
    }

    u32 final_range = weapons_dat::max_range[weapon_id];
    if (unit_ext->values[ValueTriggerWeaponRangeBaseSet]) {
        final_range = unit_ext->values[ValueTriggerWeaponRangeBaseAmount];
    }

    return final_range * scale + bonus_amount;
}
} // namespace hooks