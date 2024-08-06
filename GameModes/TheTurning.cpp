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

void TheTurningGameMode::OnInit() {
    m_spawn_timer.Start(DefaultSpawnTimer);
    m_wave_timer.Start(WaveTimer);
}

void TheTurningGameMode::OnTick() {
    if (m_spawn_timer.HasElapsed()) {
        if (Unit* unit = Spawn()) {
            u32 cost = units_dat::mineral_cost[unit->id] + units_dat::vespene_cost[unit->id];
            if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg) {
                cost /= 2;
            }

            f32 factor = 1;
            factor *= cost / 200.0; // factor spawn time relative to 200 combined cost (e.g. maverick at 50 is 50/200 = 0.25)
            factor /= 1 + 0.2 * (g_elapsed_time_frames / 24.0 / 60.0); // 10% faster spawn every minute

            m_spawn_timer.Start(DefaultSpawnTimer * factor);
        }
    }

    if (m_wave_timer.HasElapsed()) {
        m_wave_timer.Start(WaveTimer);

        f32 fraction = Map(g_elapsed_time_frames, WaveTimer, 5 * WaveTimer, 0.2, 0.8);
        auto units = g_unit_search.all().owned_by(NeutralHostilePlayer).filter_by([](Unit* unit) {
            UnitExt* unit_ext = get_unit_ext(unit);
            return !unit_ext->values[ValueTurningAggressiveMode];
        }).shuffle().take_fraction(fraction);

        for (Unit* unit : units) {
            UnitExt* unit_ext = get_unit_ext(unit);
            unit_ext->values[ValueTurningAggressiveMode] = true;
        }
    }
}

void TheTurningGameMode::OnUnitInit(Unit* unit) {
    if (unit->player != NeutralHostilePlayer) {
        return;
    }
    blissful_embrace(unit, unit->player, true);
}

void TheTurningGameMode::OnUnitTick(Unit* unit) {
    if (unit->player != NeutralHostilePlayer) {
        return;
    }
    UnitExt* unit_ext = get_unit_ext(unit);

    if (unit_ext->values[ValueTurningAggressiveMode]) {
        Unit* order_target = unit->order_target.unit;
        if (unit->main_order_id == OrderId::AttackUnit) {
            return;
        }

        if (Unit* target = g_unit_search.all().not_neutral().filter(0).sort_by_distance(unit->position).first()) {
            unit->order_to(OrderId::AttackUnit, target);
        } else if (!unit_ext->timers[TimerTurningRoam]) {
            const auto target_pos = bw::random_polar_point(unit->position, 1 * 32);
            unit->order_to(OrderId::Move, target_pos);
            unit_ext->timers[TimerTurningRoam] = 6;
        }
    } else {
        Unit* order_target = unit->order_target.unit;
        if (unit->main_order_id == OrderId::AttackUnit) {
            if (!order_target || bw::distance_between(unit->position, order_target->position) > unit->get_sight_range() * 32) {
                unit->order_to_idle();
            } else {
                return;
            }
        }

        if (Unit* target = g_unit_search.radial(unit->position, unit->get_sight_range() * 32).not_neutral().filter(0).sort_by_distance(unit->position).first()) {
            unit->order_to(OrderId::AttackUnit, target);
        } else if (!unit_ext->timers[TimerTurningRoam]) {
            const auto target_pos = bw::random_polar_point(unit->position, 6 * 32);
            unit->order_to(OrderId::Move, target_pos);
            unit_ext->timers[TimerTurningRoam] = 6 * 24;
        }
    }
}

void TheTurningGameMode::OnUnitDeath(Unit* unit) {
    const auto unit_id = static_cast<UnitId::Enum>(unit->id);
    if (units_dat::base_property[unit_id] & UnitProperty::Building || !unit_id_should_give_gas_in_massacre_mode(unit_id)) {
        return;
    }
    
    m_spawn_pool_size++;
    if (auto it = std::ranges::find_if(m_spawn_pool, [unit_id](const auto& candidate) { return candidate.unit_id == unit_id; }); it != m_spawn_pool.end()) {
        it->count++;
    } else {
        m_spawn_pool.emplace_back(unit_id, 1);
    }
}

Unit* TheTurningGameMode::Spawn() {
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

    auto buildings = g_unit_search.all().not_neutral().filter(0, FilterBuilding);
    std::optional<Point16> spawn_position = SampleBest(
        [unit_id]() -> std::optional<Point16> {
            Point16 point;
            point.x = bw::random_float() * *g_map_width_pixels;
            point.y = bw::random_float() * *g_map_height_pixels;

            if (!(units_dat::base_property[unit_id] & UnitProperty::Flyer)) {
                if (!helper::can_unit_id_fit_at_xy(point.x, point.y, unit_id)) {
                    return {};
                }
            }

            return point;
        },
        [&buildings](const Point16& point) -> s32 {
            s32 score = 0;
            for (Unit* building : buildings) {
                score += bw::distance_between(building->position, point);
            }
            return score;
        },
        10,
        100
    );
    if (!spawn_position) {
        return 0;
    }

    return spawn_reclaimed_unit(NeutralHostilePlayer, *spawn_position, unit_id);
}
