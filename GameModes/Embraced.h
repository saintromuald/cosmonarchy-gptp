#pragma once
#include "IGameMode.h"

class EmbracedGameMode : public IGameMode {
public:
	void OnUnitInit(Unit* unit) override {
		const auto& flags = units_dat::group_flags[unit->id];
		if (!flags.is_building && !flags.is_factory && !flags.is_men
			&& !flags.is_terran && !flags.is_zerg && !flags.is_protoss) {
			return;
		}
        blissful_embrace(unit, unit->player, true);
	}
};
