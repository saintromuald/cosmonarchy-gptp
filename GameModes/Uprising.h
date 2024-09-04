#pragma once
#include "BW/structures.h"
#include "CM/cm.h"

#include "IGameMode.h"

using UprisingWaveUnits = std::vector<std::pair<u32, UnitId::Enum>>;

struct UprisingSpawner {
	const char* name;
	UnitId::Enum unit_id;
	std::vector<UprisingWaveUnits> starting_waves;
	std::vector<UprisingWaveUnits> repeating_waves;

	// Runtime Data
	Unit* spawner_unit;
	u32 wave_index = 0;
	UnitId::Enum spawn_id = UnitId::None;
	Timer spawn_timer;
	std::vector<UnitRef> units_to_send;

	UprisingWaveUnits& GetWave(u32 index) {
		if (index < starting_waves.size()) {
			return starting_waves[index];
		}
		index -= starting_waves.size();
		index %= repeating_waves.size();
		return repeating_waves[index];
	}
};

struct UprisingWave {
	std::vector<UnitRef> units;
};

class UprisingGameMode : public IGameMode {
public:
	void OnTick() override;
	void OnUnitTick(Unit* unit) override;

	void CreateSpawner();

private:
	Timer m_create_spawner_timer;

	std::vector<UprisingSpawner> m_spawners;
	std::vector<UprisingWave> m_waves;

	inline static const std::vector<UprisingSpawner> s_spawner_types{
		UprisingSpawner{
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
		UprisingSpawner{
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
		UprisingSpawner{
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
		UprisingSpawner{
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
		// TODO @Pr0nogo: Add more please -Veeq7
	};
};
