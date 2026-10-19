#include "armor_bonus.h"
#include <BW/api.h>
#include "globals.h"
#include <algorithm>
#include <cmath>

namespace hooks {
    u8 get_armor_bonus(Unit* unit) { // 00453FC0
        auto& unit_ext = unit->ext();
        s32 armor_modifier = bw::get_upgrade_level(unit->player, units_dat::armor_upgrade[unit->id]);
        switch (unit->id) {
            case UnitId::TerranMatadorCinderMode: {
                if (!unit_ext.values[ValueTriggerArmorBaseSet] || unit_ext.values[ValueTriggerArmorBaseAmount]) {
                    armor_modifier += 2;
                }
            } break;
            case UnitId::TerranPhalanxTankMode: {
                if (!unit_ext.values[ValueTriggerArmorBaseSet] || unit_ext.values[ValueTriggerArmorBaseAmount]) {
                    armor_modifier += 4;
                }
            } break;
            case UnitId::ProtossZealot: {
                if (unit->shields > 256) {
                    armor_modifier += 1;
                }
            } break;
        }
        if (unit_ext.values[ValueInfested]) {
            armor_modifier += 1;
        }
        if (unit_ext.timers[TimerClericHeal]) {
            armor_modifier += 1;
        }
        if (unit_ext.values[ValueReassemblyActive]) {
            switch (unit->id) {
                case UnitId::AskosiSilversoul:
                case UnitId::AskosiTirade: {
                    armor_modifier += 1;
                } break;
                case UnitId::AskosiThaumalogue:
                case UnitId::AskosiObject:
                case UnitId::AskosiScepter:
                case UnitId::AskosiPetract:
                case UnitId::AskosiTortare: {
                    armor_modifier += 2;
                } break;
            }
        }
        if (unit_ext.timers[TimerMechanicalRage] > 1) {
            armor_modifier += 10;
        }
        armor_modifier += unit_ext.values[ValuePowerOverloadBuffStacks];
        return armor_modifier;
    }
}

s32 get_armor(Unit* unit) {
    s32 armor = units_dat::armor_amount[unit->id] + hooks::get_armor_bonus(unit);
    auto& unit_ext = unit->ext();

    // Override base armor
    if (unit_ext.values[ValueTriggerArmorBaseSet]) {
        armor = unit_ext.values[ValueTriggerArmorBaseAmount];
    }

    // Armor rends
    armor -= unit_ext.values[ValueApollyoidAdhesiveStacks];
    armor -= unit_ext.values[ValueAutocratControlledOppositionStacks];
    armor -= unit_ext.values[ValueObservanceRend];
    armor -= unit_ext.values[ValueVorvalHidebreaker];
    armor -= unit_ext.values[ValueGarvrolHidebreaker];
    armor -= unit_ext.values[ValueNathrokHidebreaker];
    armor -= unit_ext.timers[TimerCorrosiveTouch] > 1;
    armor -= unit_ext.values[ValueWendigoShatteredWindsStacks];
    armor -= unit_ext.timers[TimerDiscordantChorus] > 1;
    armor -= unit_ext.timers[TimerIncinerateArmor] > 1;
    armor -= unit_ext.values[ValueAshHeap];
    armor -= unit_ext.values[ValuePowerOverloadDebuffStacks];

    if (armor < 0) {
        armor = 0;
    }
    return armor;
}

s32 get_armor_bonus_display(Unit* unit) {
    s32 bonus = get_armor(unit);
    if (!unit->ext().values[ValueTriggerArmorBaseSet]) {
        bonus -= units_dat::armor_amount[unit->id];
    }
    return bonus;
}