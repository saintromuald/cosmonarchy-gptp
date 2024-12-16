#include "TheTurning.h"
#include "BW/api.h"

#include <ranges>

static f32 Lerp(f32 t, f32 min, f32 max) {
    return std::clamp((1 - t) * min + t * max, min, max);
}

static f32 InvLerp(f32 value, f32 min, f32 max) {
    return std::clamp<f32>((value - min) / (max - min), 0, 1);
}

static f32 Map(f32 value, f32 in_min, f32 in_max, f32 out_min, f32 out_max) {
    return Lerp(InvLerp(value, in_min, in_max), out_min, out_max);
}

void TheTurningGameMode::on_init() {
    m_spawn_timer.start(DefaultSpawnTimer);
    m_wave_timer.start(WaveTimer);
}

void TheTurningGameMode::on_tick() {
    if (m_spawn_timer.elapsed()) {
        if (Unit* unit = spawn()) {
            u32 cost = units_dat::mineral_cost[unit->id] + units_dat::vespene_cost[unit->id];
            if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg) {
                cost /= 2;
            }

            f32 factor = 1;
            factor += 0.05 * (g_elapsed_time_frames / 24.0 / 60.0); // 5% faster spawn every minute
            factor += 0.01 * m_spawn_pool_size; // 1% faster for each unit in the pool
            
            factor *= cost / 200.0; // factor spawn time relative to 200 combined cost (e.g. maverick at 50 is 50/200 = 0.25)
            factor /= bw::active_player_count(); // times the amount of players

            m_spawn_timer.start(DefaultSpawnTimer * factor);
        }
    }

    if (m_wave_timer.elapsed()) {
        m_wave_timer.start(WaveTimer);

        f32 fraction = Map(g_elapsed_time_frames, WaveTimer, 5 * WaveTimer, 0.1, 0.6);
        auto units = g_unit_search.all().owned_by(NeutralHostilePlayer).filter_by([](Unit* unit) {
            UnitExt* unit_ext = get_unit_ext(unit);
            return !unit_ext->values[ValueTurningAggressiveMode];
        }).shuffle().take_fraction(fraction);

        for (Unit* unit : units) {
            UnitExt* unit_ext = get_unit_ext(unit);
            unit_ext->values[ValueTurningAggressiveMode] = true;
            unit_ext->timers[TimerTurningAggressiveModeThrottle] = bw::rand_between(0, 3 * 24);
        }
    }
}

void TheTurningGameMode::on_unit_tick(Unit* unit) {
    if (unit->player != NeutralHostilePlayer) {
        return;
    }

    UnitExt* unit_ext = get_unit_ext(unit);
    Unit* order_target = unit->order_target.unit;
    if (unit_ext->values[ValueTurningAggressiveMode]) {
        if (unit->main_order_id == OrderId::AttackUnit) {
            return;
        }

        if (!unit_ext->timers[TimerTurningAggressiveModeThrottle]) {
            unit_ext->timers[TimerTurningAggressiveModeThrottle] = bw::rand_between(5 * 24, 8 * 24);
            if (Unit* target = g_unit_search.all().not_neutral().filter().closest_to(unit->position)) {
                unit->order_to(OrderId::AttackUnit, target);
                return;
            }
        }
        if (!unit_ext->timers[TimerTurningRoam]) {
            const auto target_pos = bw::random_polar_point(unit->position, 1 * 32);
            unit->order_to(OrderId::Move, target_pos);
            unit_ext->timers[TimerTurningRoam] = 6;
        }
        return;
    }

    if (unit->main_order_id == OrderId::AttackUnit) {
        if (!order_target || bw::distance_between(unit->position, order_target->position) > unit->get_sight_range() * 32) {
            unit->order_to_idle();
        } else {
            return;
        }
    }

    if (Unit* target = g_unit_search.radial(unit->position, unit->get_sight_range() * 32).not_neutral().filter().sort_by_distance(unit->position).first()) {
        unit->order_to(OrderId::AttackUnit, target);
    } else if (!unit_ext->timers[TimerTurningRoam]) {
        const auto target_pos = bw::random_polar_point(unit->position, 6 * 32);
        unit->order_to(OrderId::Move, target_pos);
        unit_ext->timers[TimerTurningRoam] = 6 * 24;
    }
    
}

void TheTurningGameMode::on_unit_death(Unit* unit) {
    if (unit->player == NeutralHostilePlayer) {
        return;
    }
    const auto unit_id = static_cast<UnitId::Enum>(unit->id);
    if (units_dat::base_property[unit_id] & UnitProperty::Building
        || units_dat::base_property[unit_id] & UnitProperty::Worker
        || !unit_id_should_give_gas_in_massacre_mode(unit_id)) {
        return;
    }
    
    m_spawn_pool_size++;
    if (auto it = std::ranges::find_if(m_spawn_pool, [unit_id](const auto& candidate) { return candidate.unit_id == unit_id; }); it != m_spawn_pool.end()) {
        it->count++;
    } else {
        m_spawn_pool.emplace_back(unit_id, 1);
    }
}

Unit* TheTurningGameMode::spawn() {
    UnitId::Enum unit_id = UnitId::None;
    u32 rand = bw::rand_between(0, m_spawn_pool_size - 1);
    for (const auto& candidate : m_spawn_pool) {
        u32 count = candidate.count;
        if (count < rand) {
            unit_id = candidate.unit_id;
            break;
        }
        rand -= count;
    }
    if (unit_id == UnitId::None) {
        return 0;
    }

    std::optional<Point16> spawn_position = utils::pick_random_spawn_point(unit_id);
    if (!spawn_position) {
        return 0;
    }

    return spawn_reclaimed_unit(NeutralHostilePlayer, *spawn_position, unit_id);
}