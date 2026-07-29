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

inline std::ostream& operator<<(std::ostream& os, const UprisingSpawnerInfo& info) {
    return os << info.name;
}

struct UprisingSpawner {
	const UprisingSpawnerInfo* info;

	// Runtime Data
	UnitRef spawner_unit;
	u32 wave_index = 0;
	UnitId::Enum spawn_id = UnitId::None;
	Timer spawn_timer;
	std::vector<UnitRef> units_to_send;

	const UprisingWaveUnits& get_wave(u32 index) {
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

	void tick(std::vector<UprisingSpawner>& spawners);

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
		UprisingCreator{1.5mins, {
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
                        {1, UnitId::Ackmed},
                    },
                    {
                        {12, UnitId::TerranHarakan},
                    },
                    {
                        {9, UnitId::TerranHarakan},
                        {5, UnitId::TerranMaverick},
                    },
                    {
                        {1, UnitId::Ackmed},
                        {8, UnitId::TerranHarakan},
                        {3, UnitId::TerranCleric},
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
                        {12, UnitId::TerranMaverick},
                        {8, UnitId::TerranCyprian},
                    },
                    {
                        {16, UnitId::TerranMaverick},
                    },
                    {
                        {12, UnitId::TerranCyprian},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Medicine Men",
                .unit_id = UnitId::TerranStockade,
                .starting_waves = {
                    {
                        {3, UnitId::TerranMaverick},
                        {1, UnitId::TerranCleric},
                    },
                    {
                        {6, UnitId::TerranMaverick},
                        {1, UnitId::TerranShamanSupportMode},
                    },
                    {
                        {12, UnitId::TerranMaverick},
                        {2, UnitId::TerranShamanSupportMode},
                    },
                    {
                        {12, UnitId::TerranMaverick},
                        {2, UnitId::TerranHarakan},
                        {2, UnitId::TerranShamanSupportMode},
                    },
                },
                .repeating_waves = {
                    {
                        {12, UnitId::TerranMaverick},
                        {8, UnitId::TerranCyprian},
                    },
                    {
                        {16, UnitId::TerranMaverick},
                    },
                    {
                        {12, UnitId::TerranCyprian},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Good Juju",
                .unit_id = UnitId::TerranStockade,
                .starting_waves = {
                    {
                        {2, UnitId::TerranSavantEntropyGauntlets},
                    },
                    {
                        {2, UnitId::TerranMaverick},
                        {4, UnitId::TerranSavantEntropyGauntlets},
                    },
                    {
                        {2, UnitId::TerranMaverick},
                        {2, UnitId::TerranHarakan},
                        {6, UnitId::TerranSavantEntropyGauntlets},
                    },
                    {
                        {12, UnitId::TerranSavantEntropyGauntlets},
                    },
                },
                .repeating_waves = {
                    {
                        {12, UnitId::TerranMaverick},
                        {6, UnitId::TerranSavantEntropyGauntlets},
                    },
                    {
                        {8, UnitId::TerranMaverick},
                        {9, UnitId::TerranSavantEntropyGauntlets},
                    },
                    {
                        {15, UnitId::TerranSavantEntropyGauntlets},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Trinity's Touch",
                .unit_id = UnitId::TerranStockade,
                .starting_waves = {
                    {
                        {1, UnitId::TerranMaverick},
                        {2, UnitId::TerranCohort},
                    },
                    {
                        {2, UnitId::TerranMaverick},
                        {4, UnitId::TerranCohort},
                    },
                    {
                        {3, UnitId::TerranMaverick},
                        {8, UnitId::TerranCohort},
                    },
                    {
                        {14, UnitId::TerranCohort},
                    },
                },
                .repeating_waves = {
                    {
                        {1, UnitId::Goska},
                        {6, UnitId::TerranMaverick},
                        {12, UnitId::TerranCohort},
                    },
                    {
                        {8, UnitId::TerranMaverick},
                        {18, UnitId::TerranCohort},
                    },
                    {
                        {25, UnitId::TerranCohort},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Speed Demons",
                .unit_id = UnitId::TerranFulcrum,
                .starting_waves = {
                    {
                        {2, UnitId::TerranVulture},
                    },
                    {
                        {2, UnitId::TerranVulture},
                        {2, UnitId::TerranMatadorTankMode},
                    },
                    {
                        {5, UnitId::TerranVulture},
                        {3, UnitId::TerranMatadorTankMode},
                    },
                    {
                        {8, UnitId::TerranVulture},
                        {2, UnitId::TerranMatadorTankMode},
                    },
                },
                .repeating_waves = {
                    {
                        {6, UnitId::TerranVulture},
                        {6, UnitId::TerranMatadorTankMode},
                    },
                    {
                        {12, UnitId::TerranVulture},
                    },
                    {
                        {4, UnitId::TerranVulture},
                        {10, UnitId::TerranMatadorTankMode},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Daywalkers",
                .unit_id = UnitId::TerranFulcrum,
                .starting_waves = {
                    {
                        {2, UnitId::TerranCyclops},
                    },
                    {
                        {2, UnitId::TerranCyclops},
                        {2, UnitId::TerranBlackjack},
                    },
                    {
                        {2, UnitId::TerranCyclops},
                        {4, UnitId::TerranBlackjack},
                        {1, UnitId::TerranGoliath},
                    },
                    {
                        {3, UnitId::TerranCyclops},
                        {6, UnitId::TerranBlackjack},
                        {2, UnitId::TerranGoliath},
                    },
                },
                .repeating_waves = {
                    {
                        {4, UnitId::TerranCyclops},
                        {4, UnitId::TerranBlackjack},
                        {4, UnitId::TerranGoliath},
                    },
                    {
                        {2, UnitId::TerranCyclops},
                        {10, UnitId::TerranBlackjack},
                        {2, UnitId::TerranGoliath},
                    },
                    {
                        {10, UnitId::TerranCyclops},
                        {10, UnitId::TerranBlackjack},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Thermopylae",
                .unit_id = UnitId::TerranFulcrum,
                .starting_waves = {
                    {
                        {1, UnitId::TerranPhalanxTankMode},
                    },
                    {
                        {1, UnitId::TerranGoliath},
                        {2, UnitId::TerranPhalanxTankMode},
                    },
                    {
                        {2, UnitId::TerranGoliath},
                        {4, UnitId::TerranPhalanxTankMode},
                    },
                },
                .repeating_waves = {
                    {
                        {9, UnitId::TerranPhalanxTankMode},
                    },
                    {
                        {4, UnitId::TerranGoliath},
                        {6, UnitId::TerranPhalanxTankMode},
                    },
                    {
                        {7, UnitId::TerranGoliath},
                        {5, UnitId::TerranPhalanxTankMode},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "The Reverse-Sweep",
                .unit_id = UnitId::TerranStarpad,
                .starting_waves = {
                    {
                        {3, UnitId::TerranWraith},
                    },
                    {
                        {6, UnitId::TerranWraith},
                    },
                    {
                        {8, UnitId::TerranWraith},
                        {1, UnitId::TerranAncile},
                    },
                },
                .repeating_waves = {
                    {
                        {12, UnitId::TerranWraith},
                        {1, UnitId::TerranAncile},
                    },
                    {
                        {14, UnitId::TerranWraith},
                    },
                    {
                        {10, UnitId::TerranWraith},
                        {2, UnitId::TerranAncile},
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
                .name = "Strident Signs",
                .unit_id = UnitId::ProtossGateway,
                .starting_waves = {
                    {
                        {1, UnitId::ProtossHierophant},
                        {1, UnitId::ProtossDracadin},
                    },
                    {
                        {1, UnitId::ProtossHierophant},
                        {3, UnitId::ProtossDracadin},
                    },
                    {
                        {2, UnitId::ProtossHierophant},
                        {4, UnitId::ProtossDracadin},
                    },
                },
                .repeating_waves = {
                    {
                        {6, UnitId::ProtossHierophant},
                        {6, UnitId::ProtossDracadin},
                    },
                    {
                        {4, UnitId::ProtossHierophant},
                        {12, UnitId::ProtossDracadin},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Autonnihilation",
                .unit_id = UnitId::ProtossLattice,
                .starting_waves = {
                    {
                        {4, UnitId::ProtossSimulacrum},
                    },
                    {
                        {6, UnitId::ProtossSimulacrum},
                        {3, UnitId::ProtossManifold},
                    },
                    {
                        {8, UnitId::ProtossSimulacrum},
                        {2, UnitId::ProtossManifold},
                        {2, UnitId::ProtossGolem},
                    },
                },
                .repeating_waves = {
                    {
                        {16, UnitId::ProtossSimulacrum},
                        {2, UnitId::ProtossManifold},
                        {2, UnitId::ProtossGolem},
                    },
                    {
                        {12, UnitId::ProtossSimulacrum},
                        {4, UnitId::ProtossManifold},
                        {6, UnitId::ProtossGolem},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Brobots",
                .unit_id = UnitId::ProtossLattice,
                .starting_waves = {
                    {
                        {2, UnitId::ProtossIdol},
                    },
                    {
                        {2, UnitId::ProtossGolem},
                        {3, UnitId::ProtossIdol},
                    },
                    {
                        {4, UnitId::ProtossIdol},
                        {4, UnitId::ProtossGolem},
                        {2, UnitId::ProtossManifold},
                    },
                },
                .repeating_waves = {
                    {
                        {9, UnitId::ProtossIdol},
                        {6, UnitId::ProtossGolem},
                        {3, UnitId::ProtossManifold},
                    },
                    {
                        {12, UnitId::ProtossIdol},
                        {4, UnitId::ProtossGolem},
                        {4, UnitId::ProtossManifold},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "More than Three Crows",
                .unit_id = UnitId::ProtossLattice,
                .starting_waves = {
                    {
                        {3, UnitId::ProtossVassal},
                    },
                    {
                        {9, UnitId::ProtossVassal},
                    },
                    {
                        {14, UnitId::ProtossVassal},
                    },
                },
                .repeating_waves = {
                    {
                        {28, UnitId::ProtossVassal},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Zerg, here?",
                .unit_id = UnitId::ZergQuazethPool,
                .starting_waves = {
                    {
                        {10, UnitId::ZergZethrokor},
                    },
                    {
                        {6, UnitId::ZergQuazilisk},
                    },
                    {
                        {12, UnitId::ZergZethrokor},
                        {6, UnitId::ZergQuazilisk},
                    },
                    {
                        {24, UnitId::ZergZethrokor},
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
                .name = "Lester's Nightmare",
                .unit_id = UnitId::ZergVornathPond,
                .starting_waves = {
                    {
                        {4, UnitId::ZergNathrokor},
                    },
                    {
                        {6, UnitId::ZergVorvrokor},
                        {2, UnitId::ZergNathrokor},
                    },
                    {
                        {6, UnitId::ZergVorvrokor},
                        {8, UnitId::ZergNathrokor},
                    },
                    {
                        {12, UnitId::ZergVorvrokor},
                        {10, UnitId::ZergNathrokor},
                    },
                },
                .repeating_waves = {
                    {
                        {18, UnitId::ZergVorvrokor},
                        {10, UnitId::ZergNathrokor},
                    },
                    {
                        {20, UnitId::ZergNathrokor},
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
                        {5, UnitId::ZergIzirokor},
                    },
                    {
                        {12, UnitId::ZergHydralisk},
                    },
                },
                .repeating_waves = {
                    {
                        {6, UnitId::ZergHydralisk},
                        {10, UnitId::ZergIzirokor},
                    },
                    {
                        {14, UnitId::ZergHydralisk},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Scatman's Scabmen",
                .unit_id = UnitId::ZergSkibactScab,
                .starting_waves = {
                    {
                        {3, UnitId::ZergSkithrokor},
                    },
                    {
                        {4, UnitId::ZergBactalisk},
                    },
                    {
                        {5, UnitId::ZergSkithrokor},
                        {5, UnitId::ZergBactalisk},
                    },
                    {
                        {9, UnitId::ZergSkithrokor},
                        {3, UnitId::ZergBactalisk},
                    },
                },
                .repeating_waves = {
                    {
                        {5, UnitId::ZergSkithrokor},
                        {9, UnitId::ZergBactalisk},
                    },
                    {
                        {12, UnitId::ZergSkithrokor},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Zerg Krankiz Shroud",
                .unit_id = UnitId::ZergZorkizShroud,
                .starting_waves = {
                    {
                        {2, UnitId::ZergLakizilisk},
                    },
                    {
                        {2, UnitId::ZergZoryusthaleth},
                        {2, UnitId::ZergLakizilisk},
                    },
                    {
                        {2, UnitId::ZergZoryusthaleth},
                        {4, UnitId::ZergLakizilisk},
                    },
                    {
                        {4, UnitId::ZergZoryusthaleth},
                        {6, UnitId::ZergLakizilisk},
                    },
                },
                .repeating_waves = {
                    {
                        {9, UnitId::ZergLakizilisk},
                    },
                    {
                        {6, UnitId::ZergZoryusthaleth},
                        {5, UnitId::ZergLakizilisk},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Engorgedrokor",
                .unit_id = UnitId::ZergZorkizShroud,
                .starting_waves = {
                    {
                        {2, UnitId::ZergGorgrokor},
                    },
                    {
                        {2, UnitId::ZergZoryusthaleth},
                        {2, UnitId::ZergGorgrokor},
                    },
                    {
                        {2, UnitId::ZergZoryusthaleth},
                        {4, UnitId::ZergGorgrokor},
                    },
                    {
                        {4, UnitId::ZergZoryusthaleth},
                        {6, UnitId::ZergGorgrokor},
                    },
                },
                .repeating_waves = {
                    {
                        {9, UnitId::ZergGorgrokor},
                    },
                    {
                        {6, UnitId::ZergZoryusthaleth},
                        {5, UnitId::ZergGorgrokor},
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
        }},
        // Tier 2
        UprisingCreator{4 * 60 * 24, {
            UprisingSpawnerInfo{
                .name = "Peacekeepers",
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
                .name = "From Egypt, With Love",
                .unit_id = UnitId::TerranMantle,
                .starting_waves = {
                    {
                        {4, UnitId::TerranRamesses},
                        {1, UnitId::TerranDurendal},
                    },
                    {
                        {8, UnitId::TerranRamesses},
                        {1, UnitId::TerranCataphract},
                    },
                    {
                        {6, UnitId::TerranRamesses},
                        {2, UnitId::TerranDurendal},
                        {2, UnitId::TerranCataphract},
                    },
                },
                .repeating_waves = {
                    {
                        {10, UnitId::TerranRamesses},
                        {4, UnitId::TerranDurendal},
                        {2, UnitId::TerranCataphract},
                    },
                    {
                        {16, UnitId::TerranRamesses},
                        {2, UnitId::TerranCataphract},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "The Twiceborn",
                .unit_id = UnitId::ProtossGrandLibrary,
                .starting_waves = {
                    {
                        {4, UnitId::ProtossAmaranth},
                    },
                    {
                        {6, UnitId::ProtossAmaranth},
                        {2, UnitId::ProtossAtreus},
                    },
                    {
                        {5, UnitId::ProtossAmaranth},
                        {3, UnitId::ProtossAtreus},
                        {1, UnitId::ProtossHerald},
                    },
                },
                .repeating_waves = {
                    {
                        {9, UnitId::ProtossAmaranth},
                        {3, UnitId::ProtossAtreus},
                        {2, UnitId::ProtossHerald},
                    },
                    {
                        {6, UnitId::ProtossAmaranth},
                        {6, UnitId::ProtossAtreus},
                        {1, UnitId::ProtossHerald},
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
                        {1, UnitId::ProtossServitor},
                    },
                    {
                        {1, UnitId::ProtossAccantor},
                        {5, UnitId::ProtossServitor},
                    },
                },
                .repeating_waves = {
                    {
                        {5, UnitId::ProtossAccantor},
                        {2, UnitId::ProtossServitor},
                    },
                    {
                        {3, UnitId::ProtossAccantor},
                        {12, UnitId::ProtossServitor},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "Skysinew",
                .unit_id = UnitId::ZergMutathSpire,
                .starting_waves = {
                    {
                        {6, UnitId::ZergMutalisk},
                    },
                    {
                        {9, UnitId::ZergMutalisk},
                    },
                },
                .repeating_waves = {
                    {
                        {11, UnitId::ZergMutalisk},
                    },
                    {
                        {15, UnitId::ZergMutalisk},
                    },
                },
            },
            UprisingSpawnerInfo{
                .name = "The Evig Within",
                .unit_id = UnitId::ZergAlkagIteth,
                .starting_waves = {
                    {
                        {5, UnitId::ZergKagralisk},
                        {1, UnitId::ZergEvigrilisk},
                    },
                    {
                        {4, UnitId::ZergKagralisk},
                        {4, UnitId::ZergEvigrilisk},
                    },
                    {
                        {8, UnitId::ZergKagralisk},
                        {4, UnitId::ZergEvigrilisk},
                    },
                },
                .repeating_waves = {
                    {
                        {10, UnitId::ZergKagralisk},
                        {6, UnitId::ZergEvigrilisk},
                    },
                    {
                        {13, UnitId::ZergKagralisk},
                        {4, UnitId::ZergEvigrilisk},
                    },
                },
            },
        }},
        // Tier 3 / 4
        UprisingCreator{12 * 60 * 24, {
            UprisingSpawnerInfo{
                .name = "Skyfall Protocol",
                .unit_id = UnitId::TerranIronFoundry,
                .starting_waves = {
                    {
                        {2, UnitId::TerranPaladin},
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
                    },
                    {
                        {3, UnitId::ProtossStarSovereign},
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
        }},
	};
};
