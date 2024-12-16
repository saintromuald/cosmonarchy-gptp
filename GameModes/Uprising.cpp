#include "Uprising.h"
#include "BW/api.h"
#include "trigger_system/trigger_system.h"
#include "hooks/limits/sfx_extender.h"
#include "ai_layouts.h"
#include "maths.h"
#include "hooks/tactical_ai.h"

// Originally Pr0 proposed camps expanding with more structures, defenses, etc
// might be something to consider for expanding the current attack wave methodology.
// Perhaps each attack wave spawner after some time gets another spawner
// spawned nearby, same for defenses. Also maybe we need to eventually add tiers to spawners
// since some maybe shouldn't be available in early game.

static u32 get_spawner_train_time(UnitId::Enum unit_id) {
    u32 time = units_dat::time_cost[unit_id];
    switch (unit_id) {
        case UnitId::ZergRililisk:  time = 12 * 24; break;
        case UnitId::ZergRilirokor: time = 20 * 24; break;
    }
    return Map(g_elapsed_time_frames, 0, 40 * 60 * 24, time * 2, time / 2, true);
}

static UnitId::Enum get_next_unit_to_train(UprisingSpawner& spawner) {
    std::erase_if(spawner.units_to_send, [](UnitRef ref) { return !ref.get(); });

    const auto& wave = spawner.get_wave(spawner.wave_index);
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

void UprisingGameMode::on_tick() {
    for (auto& creator : m_creators) {
        creator.tick(m_spawners);
    }

    for (auto& spawner : m_spawners) {
        Unit* unit = spawner.spawner_unit.get();
        if (!unit) {
            continue;
        }
        if (unit->id != spawner.info->unit_id || unit->player != NeutralHostilePlayer) {
            unit->remove();
            continue;
        }

        if (spawner.spawn_timer.elapsed()) {
            spawner.spawn_timer.stop();

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

        if (!spawner.spawn_timer.started()) {
            UnitId::Enum unit_id = get_next_unit_to_train(spawner);
            if (unit_id != UnitId::None) {
                spawner.spawn_id = unit_id;
                spawner.spawn_timer.start(get_spawner_train_time(unit_id));
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
    std::erase_if(m_spawners, [](const auto& spawner) { return !spawner.spawner_unit.get(); });

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

        Unit* target = g_unit_search.all().not_neutral().filter().filter_by([&wave](Unit* target) {
            for (UnitRef attacker_ref : wave.units) {
                Unit* attacker = attacker_ref.get();
                assert(attacker); // since we cleared wave.units from empty refs

                if (attacker->can_attack_target(target)) {
                    return true;
                }
            }
            return false;
        }).sort_by_distance(center).first(); // TODO: consider only targets the wave can attack
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

void UprisingGameMode::on_unit_tick(Unit* unit) {
    if (unit->player != NeutralHostilePlayer) {
        return;
    }
    UnitExt* unit_ext = get_unit_ext(unit);
    if (unit_ext->timers[TimerAI_KitingActive]) {
        return;
    }

    Unit* order_target = unit->order_target.unit;
    if (unit->main_order_id == OrderId::AttackUnit) {
        if (!order_target || bw::distance_between(unit->position, order_target->position) > unit->get_sight_range() * 32 * 1.5) {
            unit->order_to_idle();
        }
    }

    // Note: this targeting is a fallback for when kiting code wouldn't work
    if (Unit* target = g_unit_search.radial(unit->position, unit->get_sight_range() * 32).not_neutral().filter().sort([unit](Unit* target) {
        return -get_targeting_cost(unit, target);
    }).first()) {
        if (unit->main_order_id != OrderId::AttackUnit || target != order_target) {
            unit->order_to(OrderId::AttackUnit, target);
        }
    } else if (!unit_ext->timers[TimerTurningRoam] && !unit_ext->values[ValueUprisingInAttackWave]) {
        const auto target_pos = bw::random_polar_point(unit->position, 1 * 32);
        unit->order_to(OrderId::Move, target_pos);
        unit_ext->timers[TimerTurningRoam] = 4 * 24;
    }
}

void UprisingCreator::tick(std::vector<UprisingSpawner>& spawners) {
    if (m_timer.elapsed()) {
        if (m_infos.empty()) {
            return; // They shouldn't be, but we don't wanna crash
        }

        const auto& info = bw::rand_one(m_infos);
        const auto position = utils::pick_random_spawn_point(info.unit_id, 100);
        if (!position) {
            return; // We don't want to restart the timer if spawn failed
        }

        Unit* unit = bw::create_unit_in_game((u16)info.unit_id, NeutralHostilePlayer, position->x, position->y);
        if (!unit) {
            return; // We don't want to restart the timer if spawn failed
        }

        UnitExt* unit_ext = get_unit_ext(unit);
        unit_ext->values[ValueUprisingSpawner] = true;
        unit_ext->custom_name = info.name;
        if (unit->get_race() == RaceId::Zerg) {
            bw::spawn_custom_creep(unit);
        }

        UprisingSpawner spawner{
            .info = &info,
            .spawner_unit = unit,
        };
        spawners.push_back(std::move(spawner));

        bw::minimap_ping(unit->position.x, unit->position.y, 0);
        unit->play_iscript_animation(IscriptAnimation::IsWorking);
    }
    if (!m_timer.running()) {
        m_timer.start(Map(g_elapsed_time_frames, 0, 40 * 60 * 24, m_default_timer, m_default_timer / 4, true) / bw::active_player_count());
    }
}