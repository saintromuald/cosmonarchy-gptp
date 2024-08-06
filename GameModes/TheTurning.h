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

    void OnInit() override;
    void OnTick() override;
    void OnUnitTick(Unit* unit) override;

    void OnUnitDeath(Unit* unit) override;

private:
    Unit* Spawn();

private:
    u32 m_spawn_pool_size = 0;
    std::vector<SpawnCandidate> m_spawn_pool;
    Timer m_spawn_timer;
    Timer m_wave_timer;
};
