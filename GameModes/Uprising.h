#pragma once
#include "BW/structures.h"
#include "CM/cm.h"

#include "IGameMode.h"

using UprisingWaveUnits = std::vector<std::pair<u32, UnitId::Enum>>;

struct UprisingSpawnerInfo {
	const char* name;
	UnitId::Enum unit_id;
	std::vector<UprisingWaveUnits> starting_waves;
	std::vector<UprisingWaveUnits> repeating_waves;
};

struct UprisingSpawner {
	const UprisingSpawnerInfo* info;

	// Runtime Data
	UnitRef spawner_unit;
	u32 wave_index = 0;
	UnitId::Enum spawn_id = UnitId::None;
	Timer spawn_timer;
	std::vector<UnitRef> units_to_send;

	const UprisingWaveUnits& GetWave(u32 index) {
		if (index < info->starting_waves.size()) {
			return info->starting_waves[index];
		}
		index -= info->starting_waves.size();
		index %= info->repeating_waves.size();
		return info->repeating_waves[index];
	}
};

struct UprisingWave {
	std::vector<UnitRef> units;
};

class UprisingCreator {
public:
	UprisingCreator(u32 default_timer, std::vector<UprisingSpawnerInfo> infos)
		: m_default_timer{default_timer}, m_infos{std::move(infos)} {}

	void Tick(std::vector<UprisingSpawner>& spawners);

private:
	u32 m_default_timer;
	Timer m_timer;
    std::vector<UprisingSpawnerInfo> m_infos;
};

class UprisingGameMode : public IGameMode {
public:
	void on_tick() override;
	void on_unit_tick(Unit* unit) override;

private:
	Timer m_create_spawner_timer;
	std::vector<UprisingSpawner> m_spawners;
	std::vector<UprisingWave> m_waves;
	std::vector<UprisingCreator> m_creators{
        // Tier 1
		UprisingCreator{static_cast<u32>(1.5 * 60 * 24), {
            UprisingSpawnerInfo{
                .name = "Ackmed's BBQ",
                .unit_id = UnitId::TerranStockade,
                .starting_waves = {
                    {
                        {3, UnitId::TerranHarakan},
                    },
                    {
                        {4, UnitId::TerranHarakan},
                        {1, UnitId::TerranMaverick},
                    },
                    {
                        {8, UnitId::TerranHarakan},
                    },
                    {
                        {6, UnitId::TerranHarakan},
                        {3, UnitId::TerranMaverick},
                    },
                },
                .repeating_waves = {
                    {
                        {12, UnitId::TerranHarakan},
                    },
                    {
                        {9, UnitId::TerranHarakan},
                        {5, UnitId::TerranMaverick},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "The Revolutionaries",
                .unit_id = UnitId::TerranStockade,
                .starting_waves = {
                    {
                        {4, UnitId::TerranMaverick},
                    },
                    {
                        {4, UnitId::TerranCyprian},
                    },
                    {
                        {8, UnitId::TerranMaverick},
                        {2, UnitId::TerranCyprian},
                    },
                    {
                        {10, UnitId::TerranCyprian},
                    },
                },
                .repeating_waves = {
                    {
                        {8, UnitId::TerranMaverick},
                        {8, UnitId::TerranCyprian},
                    },
                    {
                        {12, UnitId::TerranCyprian},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Swords & Swordsmen",
                .unit_id = UnitId::ProtossGateway,
                .starting_waves = {
                    {
                        {1, UnitId::ProtossZealot},
                        {3, UnitId::ProtossLegionnaire},
                    },
                    {
                        {1, UnitId::ProtossZealot},
                        {6, UnitId::ProtossLegionnaire},
                    },
                    {
                        {2, UnitId::ProtossZealot},
                        {8, UnitId::ProtossLegionnaire},
                        {1, UnitId::ProtossEcclesiast},
                    },
                    {
                        {2, UnitId::ProtossZealot},
                        {11, UnitId::ProtossLegionnaire},
                        {2, UnitId::ProtossEcclesiast},
                    },
                },
                .repeating_waves = {
                    {
                        {3, UnitId::ProtossZealot},
                        {12, UnitId::ProtossLegionnaire},
                        {3, UnitId::ProtossEcclesiast},
                    },
                    {
                        {18, UnitId::ProtossZealot},
                        {4, UnitId::ProtossEcclesiast},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Zerg, here?",
                .unit_id = UnitId::ZergQuazethPool,
                .starting_waves = {
                    {
                        {8, UnitId::ZergZethrokor},
                    },
                    {
                        {8, UnitId::ZergQuazilisk},
                    },
                    {
                        {12, UnitId::ZergZethrokor},
                        {4, UnitId::ZergQuazilisk},
                    },
                    {
                        {20, UnitId::ZergZethrokor},
                    },
                },
                .repeating_waves = {
                    {
                        {18, UnitId::ZergZethrokor},
                        {8, UnitId::ZergQuazilisk},
                    },
                    {
                        {30, UnitId::ZergZethrokor},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Connor's Revenge",
                .unit_id = UnitId::ZergHydrithDen,
                .starting_waves = {
                    {
                        {4, UnitId::ZergHydralisk},
                    },
                    {
                        {6, UnitId::ZergHydralisk},
                    },
                    {
                        {5, UnitId::ZergHydralisk},
                        {3, UnitId::ZergBactalisk},
                    },
                    {
                        {8, UnitId::ZergHydralisk},
                    },
                },
                .repeating_waves = {
                    {
                        {4, UnitId::ZergHydralisk},
                        {8, UnitId::ZergBactalisk},
                    },
                    {
                        {14, UnitId::ZergHydralisk},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Nekron's Wet Dream",
                .unit_id = UnitId::ZergNydusCesiant,
                .starting_waves = {
                    {
                        {4, UnitId::ZergRililisk},
                    },
                    {
                        {8, UnitId::ZergRililisk},
                    },
                    {
                        {6, UnitId::ZergRililisk},
                        {2, UnitId::ZergRilirokor},
                    },
                    {
                        {12, UnitId::ZergRililisk},
                    },
                    {
                        {8, UnitId::ZergRilirokor},
                    },
                },
                .repeating_waves = {
                    {
                        {10, UnitId::ZergRililisk},
                        {6, UnitId::ZergRilirokor},
                    },
                    {
                        {20, UnitId::ZergRililisk},
                    },
                    {
                        {12, UnitId::ZergRilirokor},
                    },
                },
            },
            // TODO @Pr0nogo: Add more please -Veeq7
        }},
        // Tier 2
        UprisingCreator{4 * 60 * 24, {
            UprisingSpawnerInfo{
                .name = "Uprising",
                .unit_id = UnitId::TerranCaptaincy,
                .starting_waves = {
                    {
                        {5, UnitId::TerranMadcap},
                        {2, UnitId::TerranHeracles},
                    },
                    {
                        {9, UnitId::TerranMadcap},
                    },
                },
                .repeating_waves = {
                    {
                        {7, UnitId::TerranMadcap},
                        {4, UnitId::TerranHeracles},
                    },
                    {
                        {12, UnitId::TerranMadcap},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Skymass",
                .unit_id = UnitId::ZergMutathSpire,
                .starting_waves = {
                    {
                        {6, UnitId::ZergMutalisk},
                    },
                    {
                        {8, UnitId::ZergMutalisk},
                    },
                },
                .repeating_waves = {
                    {
                        {10, UnitId::ZergMutalisk},
                    },
                    {
                        {14, UnitId::ZergMutalisk},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Assembly of Iquaria",
                .unit_id = UnitId::ProtossArdentAuthority,
                .starting_waves = {
                    {
                        {1, UnitId::ProtossAccantor},
                    },
                    {
                        {2, UnitId::ProtossAccantor},
                    },
                    {
                        {1, UnitId::ProtossAccantor},
                        {4, UnitId::ProtossServitor},
                    },
                },
                .repeating_waves = {
                    {
                        {4, UnitId::ProtossAccantor},
                    },
                    {
                        {3, UnitId::ProtossAccantor},
                        {8, UnitId::ProtossServitor},
                    },
                },
            },
            // TODO @Pr0nogo: Add more please -Veeq7
        }},
        // Tier 3 / 4
        UprisingCreator{12 * 60 * 24, {
            UprisingSpawnerInfo{
                .name = "The Grief of All Gods",
                .unit_id = UnitId::ProtossMonumentOfSin,
                .starting_waves = {
                    {
                        {1, UnitId::ProtossStarSovereign},
                    },
                    {
                        {1, UnitId::ProtossStarSovereign},
                    },
                },
                .repeating_waves = {
                    {
                        {2, UnitId::ProtossStarSovereign},
                    },
                    {
                        {2, UnitId::ProtossStarSovereign},
                        {1, UnitId::ProtossDidact},
                    },
                    {
                        {3, UnitId::ProtossStarSovereign},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Skyfall",
                .unit_id = UnitId::TerranIronFoundry,
                .starting_waves = {
                    {
                        {3, UnitId::TerranPaladin},
                    },
                    {
                        {4, UnitId::TerranPaladin},
                    },
                },
                .repeating_waves = {
                    {
                        {6, UnitId::TerranPaladin},
                    },
                    {
                        {4, UnitId::TerranPaladin},
                        {2, UnitId::TerranCuirassTankMode},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Jayborino's Cheat Code",
                .unit_id = UnitId::ZergGeszkathGrotto,
                .starting_waves = {
                    {
                        {3, UnitId::ZergGeszithalor},
                    },
                    {
                        {1, UnitId::ZergAlkajelisk},
                    },
                    {
                        {5, UnitId::ZergGeszithalor},
                        {1, UnitId::ZergAlkajelisk},
                    },
                    {
                        {2, UnitId::ZergAlkajelisk},
                    },
                },
                .repeating_waves = {
                    {
                        {8, UnitId::ZergGeszithalor},
                    },
                    {
                        {6, UnitId::ZergGeszithalor},
                        {2, UnitId::ZergAlkajelisk},
                    },
                    {
                        {4, UnitId::ZergAlkajelisk},
                    },
                },
            },
            // TODO @Pr0nogo: Add more please -Veeq7
        }},
	};
};
