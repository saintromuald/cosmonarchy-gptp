#pragma once
#include "IGameMode.h"
#include "Utils.h"

struct SpawnCandidate {
    UnitId::Enum unit_id;
    u32 count;
};

class TheTurningGameMode : public IGameMode {
public:
    static constexpr u32 DefaultSpawnTimer = 60 * 24;
    static constexpr u32 WaveTimer = 3 * 60 * 24;

    void on_init() override;
    void on_tick() override;
    void on_unit_tick(Unit* unit) override;

    void on_unit_death(Unit* unit) override;

private:
    Unit* spawn();
    f32 calculate_spawn_time_factor(UnitId::Enum unit_id = UnitId::None) const;

private:
    u32 m_spawn_pool_size = 0;
    std::vector<SpawnCandidate> m_spawn_pool;
    Timer m_spawn_timer;
    Timer m_wave_timer;
};
