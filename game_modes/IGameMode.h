#pragma once
#include <BW/api.h>
#include <memory>

class IGameMode {
public:
	virtual ~IGameMode() {}
	virtual void tick() {}
	virtual void tick_unit(Unit* unit) {}
};

inline std::unique_ptr<IGameMode> g_active_game_mode;

class EmbracedGameMode : public IGameMode {
public:
	void tick_unit(Unit* unit) override {
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
