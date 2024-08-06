#pragma once
#include "IGameMode.h"

class EmbracedGameMode : public IGameMode {
public:
	void OnUnitTick(Unit* unit) override {
		const auto& flags = units_dat::group_flags[unit->id];
		if (!flags.is_building && !flags.is_factory && !flags.is_factory 
			&& !flags.is_terran && !flags.is_zerg && !flags.is_protoss) {
			return;
		}
		UnitExt* unit_ext = get_unit_ext(unit);
		if (!unit_ext->values[ValueInfested]) {
			blissful_embrace(unit, unit->player, true);
		}
	}
};
