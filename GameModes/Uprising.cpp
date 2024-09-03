#include "Uprising.h"
#include "BW/api.h"
#include "trigger_system/trigger_system.h"
#include "hooks/limits/sfx_extender.h"
#include "ai_layouts.h"

// Originally Pr0 proposed camps expanding with more structures, defenses, etc
// might be something to consider for expanding the current attack wave methodology.
// Perhaps each attack wave spawner after some time gets another spawner
// spawned nearby, same for defenses. Also maybe we need to eventually add tiers to spawners
// since some maybe shouldn't be available in early game.

static UnitId::Enum GetNextUnitToTrain(UprisingSpawner& spawner) {
    std::erase_if(spawner.units_to_send, [](UnitRef ref) { return !ref.get(); });

    const auto& wave = spawner.GetWave(spawner.wave_index);
    for (const auto& [count_quota, unit_id] : wave) {
        u32 count = 0;
        for (UnitRef ref : spawner.units_to_send) {
            Unit* unit = ref.get();
            assert(unit); // since we cleared units_to_send from empty refs

            if (unit->id == unit_id) {
                count++;
            }
        }
        if (count < count_quota) {
            return unit_id;
        }
    }
    return UnitId::None;
}

void UprisingGameMode::OnTick() {
    if (m_create_spawner_timer.HasElapsed()) {
        CreateSpawner();
    }
    if (!m_create_spawner_timer.IsRunning()) {
        m_create_spawner_timer.Start(90 * 24 / bw::active_player_count());
    }

    for (auto& spawner : m_spawners) {
        Unit* unit = spawner.spawner_unit;

        if (spawner.spawn_timer.HasElapsed()) {
            spawner.spawn_timer.Stop();

            if (spawner.spawn_id != UnitId::None) {
                bool two_per_egg = units_dat::base_property[spawner.spawn_id] & UnitProperty::TwoUnitsIn1Egg;
                u32 spawn_count = two_per_egg ? 2 : 1;
                for (u32 i = 0; i < spawn_count; i++) {
                    Unit* spawn = bw::create_unit_in_game(spawner.spawn_id, NeutralHostilePlayer, unit->position.x, unit->position.y, unit);
                    if (spawn) {
                        spawner.units_to_send.emplace_back(spawn);
                    }
                }
            }
        }

        if (!spawner.spawn_timer.HasStarted()) {
            UnitId::Enum unit_to_train = GetNextUnitToTrain(spawner);
            if (unit_to_train != UnitId::None) {
                spawner.spawn_id = unit_to_train;
                spawner.spawn_timer.Start(units_dat::time_cost[unit_to_train]);
            } else {
                for (UnitRef ref : spawner.units_to_send) {
                    Unit* wave_unit = ref.get();
                    assert(wave_unit); // since we cleared units_to_send from empty refs

                    UnitExt* wave_unit_ext = get_unit_ext(wave_unit);
                    wave_unit_ext->values[ValueUprisingInAttackWave] = true;
                }

                UprisingWave wave;
                wave.units = std::move(spawner.units_to_send);
                m_waves.push_back(std::move(wave));
            }
        }
    }

    for (auto& wave : m_waves) {
        std::erase_if(wave.units, [](UnitRef ref) { return !ref.get(); });
        if (wave.units.empty()) {
            continue;
        }

        Vec32 pos_sum{};
        for (UnitRef ref : wave.units) {
            Unit* unit = ref.get();
            assert(unit); // since we cleared wave.units from empty refs

            pos_sum.x += unit->position.x; 
            pos_sum.y += unit->position.y; 
        }

        Point16 center{
            pos_sum.x / wave.units.size(),
            pos_sum.y / wave.units.size(),
        };

        Unit* target = g_unit_search.all().not_neutral().filter().sort_by_distance(center).first(); // TODO: consider only targets the wave can attack
        if (target) {
            for (UnitRef ref : wave.units) {
                Unit* unit = ref.get();
                assert(unit); // since we cleared wave.units from empty refs

                if (unit->main_order_id != OrderId::AttackUnit) {
                    unit->order_to(OrderId::AttackMove, target->position);
                }
            }
        }
    }
    std::erase_if(m_waves, [](const auto& wave) { return wave.units.empty(); });
}

void UprisingGameMode::OnUnitTick(Unit* unit) {
    if (unit->player != NeutralHostilePlayer) {
        return;
    }
    UnitExt* unit_ext = get_unit_ext(unit);

    Unit* order_target = unit->order_target.unit;
    if (unit->main_order_id == OrderId::AttackUnit) {
        if (!order_target || bw::distance_between(unit->position, order_target->position) > unit->get_sight_range() * 32 * 1.5) {
            unit->order_to_idle();
        } else {
            return;
        }
    }

    if (Unit* target = g_unit_search.radial(unit->position, unit->get_sight_range() * 32).not_neutral().filter().sort_by_distance(unit->position).first()) {
        unit->order_to(OrderId::AttackUnit, target);
    } else if (!unit_ext->timers[TimerTurningRoam] && !unit_ext->values[ValueUprisingInAttackWave]) {
        const auto target_pos = bw::random_polar_point(unit->position, 1 * 32);
        unit->order_to(OrderId::Move, target_pos);
        unit_ext->timers[TimerTurningRoam] = 4 * 24;
    }
}

void UprisingGameMode::CreateSpawner() {
    auto spawner = bw::rand_one(s_spawner_types);
    const auto position = pick_random_spawn_position(spawner.unit_id, 50);
    if (!position) {
        return;
    }

    Unit* unit = bw::create_unit_in_game((u16)spawner.unit_id, NeutralHostilePlayer, position->x, position->y);
    if (!unit) {
        return;
    }

    UnitExt* unit_ext = get_unit_ext(unit);
    unit_ext->values[ValueUprisingSpawner] = true;
    unit_ext->custom_name = spawner.name;
    if (unit->get_race() == RaceId::Zerg) {
        bw::spawn_custom_creep(unit);
    }

    spawner.spawner_unit = unit;
    m_spawners.push_back(std::move(spawner));

    bw::minimap_ping(unit->position.x, unit->position.y, 0);
    unit->play_iscript_animation(IscriptAnimation::IsWorking);
}