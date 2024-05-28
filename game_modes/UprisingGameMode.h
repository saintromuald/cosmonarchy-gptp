#pragma once
#include "BW/api.h"
#include "CM/cm.h"

#include "IGameMode.h"

struct UpstartSpawnLocation {
	Point16 position;
	u32 resarea;
};

class UprisingGameMode : public IGameMode {
public:
	UprisingGameMode();
	~UprisingGameMode() override = default;
	void tick_unit(Unit* unit) override;

private:
	// NOTE: not using unordered_map, since ordering could be different and desync
	inline static std::map<UnitId::Enum, std::vector<UnitId::Enum>> s_spawner_types{
		{
			UnitId::TerranStockade,
			{
				UnitId::TerranMaverick,
				UnitId::TerranHarakan,
				UnitId::TerranCyprian,
			},
		},
		{
			UnitId::TerranFulcrum,
			{
				UnitId::TerranVulture,
				UnitId::TerranCyclops,
				UnitId::TerranGoliath,
			},
		},
		{
			UnitId::ProtossGateway,
			{
				UnitId::ProtossZealot,
				UnitId::ProtossDracadin,
				UnitId::ProtossEcclesiast,
				UnitId::ProtossLegionnaire,
				UnitId::ProtossHierophant,
			},
		},
		{
			UnitId::ProtossLattice,
			{
				UnitId::ProtossSimulacrum,
				UnitId::ProtossManifold,
				UnitId::ProtossVassal,
				UnitId::ProtossIdol,
				UnitId::ProtossGolem,
			},
		},
		{
			UnitId::ZergQuazethPool,
			{
				UnitId::ZergZethrokor,
				UnitId::ZergQuazilisk,
			},
		},
		{
			UnitId::ZergHydrithDen,
			{
				UnitId::ZergHydralisk,
				UnitId::ZergIzirokor,
			},
		},
	};
};
