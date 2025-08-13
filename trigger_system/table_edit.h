#pragma once
#include "trigger_parser.h"

class TableEditor {
public:
    TableEditor(Parser& parser) : m_parser{parser} {}

    void run() {
        const auto target = m_parser.get_word();
        if (target == "unit") {
            const auto unit_ids = m_parser.get_unit_id_bitset();
            if (unit_ids.none()) {
                m_parser.error("No valid unit ids specified");
            }
            const auto stat = m_parser.get_word();
            if      (stat == "health")              alter_stat(unit_ids, units_dat::max_hitpoints);
            else if (stat == "armor")               alter_stat(unit_ids, units_dat::armor_amount);
            else if (stat == "shields")             alter_stat(unit_ids, units_dat::max_shield_points);
            else if (stat == "shields_enabled")     set_bool(unit_ids, units_dat::shields_enabled);
            else if (stat == "ground_weapon")       set_weapon(unit_ids, units_dat::ground_weapon);
            else if (stat == "air_weapon")          set_weapon(unit_ids, units_dat::air_weapon);
            else if (stat == "seek_range")          alter_stat(unit_ids, units_dat::seek_range);
            else if (stat == "sight_range")         alter_stat(unit_ids, units_dat::sight_range);
            else if (stat == "portrait_id")         set_stat(unit_ids, units_dat::portrait_id);
            else if (stat == "mineral_cost")        alter_stat(unit_ids, units_dat::mineral_cost);
            else if (stat == "vespene_cost")        alter_stat(unit_ids, units_dat::vespene_cost);
            else if (stat == "time_cost")           alter_stat(unit_ids, units_dat::time_cost);
            else if (stat == "supply_cost")         alter_stat(unit_ids, units_dat::supply_required);
            else if (stat == "supply_provided")     alter_stat(unit_ids, units_dat::supply_provided);
            else if (stat == "space_cost")          alter_stat(unit_ids, units_dat::space_required);
            else if (stat == "space_provided")      alter_stat(unit_ids, units_dat::space_provided);
            else if (stat == "build_score")         alter_stat(unit_ids, units_dat::build_score);
            else if (stat == "destroy_score")       alter_stat(unit_ids, units_dat::destroy_score);
            else if (stat == "start_direction")     alter_stat(unit_ids, units_dat::start_direction);
            else if (stat == "elevation")           alter_stat(unit_ids, units_dat::elevation);
            else if (stat == "movement_flags")      alter_flags<MovementFlags>(unit_ids, units_dat::movement_flags);
            else if (stat == "base_property")       alter_flags<UnitProperty::Enum>(unit_ids, units_dat::base_property);
            else if (stat == "group_flags")         alter_flags_value<GroupFlags>(unit_ids, units_dat::group_flags);
            else if (stat == "seek_range")          alter_stat(unit_ids, units_dat::seek_range);
            else if (stat == "sight_range")         alter_stat(unit_ids, units_dat::sight_range);
            else if (stat == "sound_ready")         set_stat(unit_ids, units_dat::sound_ready);
            else if (stat == "sound_what_first")    set_stat(unit_ids, units_dat::sound_what_first);
            else if (stat == "sound_what_last")     set_stat(unit_ids, units_dat::sound_what_last);
            else if (stat == "sound_annoyed_first") set_stat(unit_ids, units_dat::sound_annoyed_first);
            else if (stat == "sound_annoyed_last")  set_stat(unit_ids, units_dat::sound_annoyed_last);
            else if (stat == "sound_yes_first")     set_stat(unit_ids, units_dat::sound_yes_first);
            else if (stat == "sound_yes_last")      set_stat(unit_ids, units_dat::sound_yes_last);
            else if (stat == "building_dimensions") alter_point(unit_ids, units_dat::building_dimensions);
            else if (stat == "addon_placement")     alter_point(unit_ids, units_dat::addon_placement);
            else if (stat == "unit_bounds")         alter_box(unit_ids, units_dat::unit_bounds);
            else m_parser.error("Invalid stat", stat.sv);
        } else if (target == "weapon") {
            const auto weapon_ids = m_parser.get_weapon_id_bitset();
            if (weapon_ids.none()) {
                m_parser.error("No valid weapon ids specified");
            }
            const auto stat = m_parser.get_word();
            if      (stat == "damage")          alter_stat(weapon_ids, weapons_dat::damage_amount);
            else if (stat == "armor_pen")       alter_stat(weapon_ids, weapons_dat::armor_pen);
            else if (stat == "cooldown")        alter_stat(weapon_ids, weapons_dat::cooldown);
            else if (stat == "factor")          alter_stat(weapon_ids, weapons_dat::damage_factor);
            else if (stat == "inner_splash")    alter_stat(weapon_ids, weapons_dat::inner_splash_radius);
            else if (stat == "medium_splash")   alter_stat(weapon_ids, weapons_dat::medium_splash_radius);
            else if (stat == "outer_splash")    alter_stat(weapon_ids, weapons_dat::outer_splash_radius);
            else if (stat == "attack_angle")    alter_stat(weapon_ids, weapons_dat::attack_angle);
            else if (stat == "launch_spin")     alter_stat(weapon_ids, weapons_dat::launch_spin);
            else if (stat == "forward_offset")  alter_stat(weapon_ids, weapons_dat::forward_offset);
            else if (stat == "vertical_offset") alter_stat(weapon_ids, weapons_dat::vertical_offset);
            else if (stat == "label")           set_stat(weapon_ids, weapons_dat::label);
            else if (stat == "weapon_icon")     set_stat(weapon_ids, weapons_dat::weapon_icon);
            else if (stat == "flingy_id")       set_stat(weapon_ids, weapons_dat::flingy_id);
            else if (stat == "target_flags")    alter_flags_value<TargetFlag>(weapon_ids, weapons_dat::target_flags);
            else if (stat == "min_range")       alter_stat(weapon_ids, weapons_dat::min_range);
            else if (stat == "range")           alter_stat(weapon_ids, weapons_dat::max_range);
            else if (stat == "behavior")        set_enum<WeaponBehaviors>(weapon_ids, weapons_dat::behavior);
            else if (stat == "remove_after")    alter_stat(weapon_ids, weapons_dat::remove_after);
            else if (stat == "explosion_type")  set_enum<WeaponEffect::Enum>(weapon_ids, weapons_dat::explosion_type);
            else m_parser.error("Invalid stat", stat.sv);
        } else {
            m_parser.error("Invalid token", target.sv);
        }
    }

    template <size_t N, typename T>
    void set_stat(const std::bitset<N>& bitset, T* table) {
        const auto op = m_parser.get_stat_operator();
        const auto value = m_parser.get_s32();
        for (const auto i : IterateBitset<size_t, N>{bitset}) {
            switch (op) {
                case StatOperator::Set: table[i] = value; break;
                default: m_parser.error("Invalid operator type", op); break;
            }
        }
    }

    template <size_t N, typename T>
    void alter_stat(const std::bitset<N>& bitset, T* table) {
        const auto op = m_parser.get_stat_operator();
        const auto value = m_parser.get_s32();
        for (const auto i : IterateBitset<size_t, N>{bitset}) {
            switch (op) {
                case StatOperator::Set:      table[i] = value; break;
                case StatOperator::Add:      table[i] += value; break;
                case StatOperator::Subtract: table[i] -= value; break;
                case StatOperator::Multiply: table[i] *= value; break;
                case StatOperator::Divide:   table[i] /= value; break;
                default: m_parser.error("Invalid operator type", op); return;
            }
        }
    }

    template <typename E>
    u32 get_enum() {
        if constexpr (std::is_enum_v<E>) {
            return u32(m_parser.get_magic_enum<E>());
        } else {
            return m_parser.get_u32();
        }
    }

    template <typename E>
    u32 get_flags() {
        if constexpr (std::is_enum_v<E>) {
            return u32(m_parser.get_magic_flags<E>());
        } else {
            return m_parser.get_u32();
        }
    }

    template <typename E = void, size_t N, typename T>
    void set_enum(const std::bitset<N>& bitset, T* table) {
        const auto op = m_parser.get_stat_operator();
        const auto value = get_enum<T>();
        for (const auto i : IterateBitset<size_t, N>{bitset}) {
            switch (op) {
                case StatOperator::Set: table[i] = value; break;
                default: m_parser.error("Invalid operator type", op); break;
            }
        }
    }

    template <typename E = void, size_t N, typename T>
    void alter_flags(const std::bitset<N>& bitset, T* table) {
        const auto op = m_parser.get_stat_operator();
        const auto value = get_flags<E>();
        for (const auto i : IterateBitset<size_t, N>{bitset}) {
            switch (op) {
                case StatOperator::Set:        table[i] = value; break;
                case StatOperator::Or:         table[i] |= value; break;
                case StatOperator::InverseAnd: table[i] &= ~value; break;
                case StatOperator::Xor:        table[i] ^= value; break;
                default: m_parser.error("Invalid operator type", op); return;
            }
        }
    }

    template <typename E = void, size_t N, typename T>
    void alter_flags_value(const std::bitset<N>& bitset, T* table) {
        const auto op = m_parser.get_stat_operator();
        const auto value = get_flags<E>();
        for (const auto i : IterateBitset<size_t, N>{bitset}) {
            switch (op) {
                case StatOperator::Set:        table[i].value = value; break;
                case StatOperator::Or:         table[i].value |= value; break;
                case StatOperator::InverseAnd: table[i].value &= ~value; break;
                case StatOperator::Xor:        table[i].value ^= value; break;
                default: m_parser.error("Invalid operator type", op); return;
            }
        }
    }

    template <size_t N, typename T>
    void set_bool(const std::bitset<N>& bitset, T* table) {
        const auto op = m_parser.get_stat_operator();
        const auto value = op != StatOperator::Negate ? m_parser.get_bool() : false;
        for (const auto i : IterateBitset<size_t, N>{bitset}) {
            switch (op) {
                case StatOperator::Set:    table[i] = value; break;
                case StatOperator::Negate: table[i] = !table[i]; break;
                default: m_parser.error("Invalid operator type", op); return;
            }
        }
    }

    template <size_t N, typename T>
    void set_weapon(const std::bitset<N>& bitset, T* table) {
        const auto value = m_parser.get_weapon_id();
        for (const auto i : IterateBitset<size_t, N>{bitset}) {
            table[i] = value;
        }
    }

    template <size_t N, typename T>
    void alter_point(const std::bitset<N>& bitset, T* table) {
        const auto x_op = m_parser.get_stat_operator();
        const auto x_value = m_parser.get_s32();
        const auto y_op = m_parser.get_stat_operator();
        const auto y_value = m_parser.get_s32();
        for (const auto i : IterateBitset<size_t, N>{bitset}) {
            switch (x_op) {
                case StatOperator::Set:      table[i].x = x_value; break;
                case StatOperator::Add:      table[i].x += x_value; break;
                case StatOperator::Subtract: table[i].x -= x_value; break;
                case StatOperator::Multiply: table[i].x *= x_value; break;
                case StatOperator::Divide:   table[i].x /= x_value; break;
                default: m_parser.error("Invalid operator type", x_op); return;
            }
            switch (y_op) {
                case StatOperator::Set:      table[i].y = y_value; break;
                case StatOperator::Add:      table[i].y += y_value; break;
                case StatOperator::Subtract: table[i].y -= y_value; break;
                case StatOperator::Multiply: table[i].y *= y_value; break;
                case StatOperator::Divide:   table[i].y /= y_value; break;
                default: m_parser.error("Invalid operator type", y_op); return;
            }
        }
    }

    template <size_t N, typename T>
    void alter_box(const std::bitset<N>& bitset, T* table) {
        const auto x_op = m_parser.get_stat_operator();
        const auto x_value = m_parser.get_s32();
        const auto y_op = m_parser.get_stat_operator();
        const auto y_value = m_parser.get_s32();
        const auto w_op = m_parser.get_stat_operator();
        const auto w_value = m_parser.get_s32();
        const auto h_op = m_parser.get_stat_operator();
        const auto h_value = m_parser.get_s32();
        for (const auto i : IterateBitset<size_t, N>{bitset}) {
            switch (x_op) {
                case StatOperator::Set:      table[i].x = x_value; break;
                case StatOperator::Add:      table[i].x += x_value; break;
                case StatOperator::Subtract: table[i].x -= x_value; break;
                case StatOperator::Multiply: table[i].x *= x_value; break;
                case StatOperator::Divide:   table[i].x /= x_value; break;
                default: m_parser.error("Invalid operator type", x_op); return;
            }
            switch (y_op) {
                case StatOperator::Set:      table[i].y = y_value; break;
                case StatOperator::Add:      table[i].y += y_value; break;
                case StatOperator::Subtract: table[i].y -= y_value; break;
                case StatOperator::Multiply: table[i].y *= y_value; break;
                case StatOperator::Divide:   table[i].y /= y_value; break;
                default: m_parser.error("Invalid operator type", y_op); return;
            }
            switch (w_op) {
                case StatOperator::Set:      table[i].w = w_value; break;
                case StatOperator::Add:      table[i].w += w_value; break;
                case StatOperator::Subtract: table[i].w -= w_value; break;
                case StatOperator::Multiply: table[i].w *= w_value; break;
                case StatOperator::Divide:   table[i].w /= w_value; break;
                default: m_parser.error("Invalid operator type", w_op); return;
            }
            switch (h_op) {
                case StatOperator::Set:      table[i].h = h_value; break;
                case StatOperator::Add:      table[i].h += h_value; break;
                case StatOperator::Subtract: table[i].h -= h_value; break;
                case StatOperator::Multiply: table[i].h *= h_value; break;
                case StatOperator::Divide:   table[i].h /= h_value; break;
                default: m_parser.error("Invalid operator type", h_op); return;
            }
        }
    }
private:
    Parser& m_parser;
};
