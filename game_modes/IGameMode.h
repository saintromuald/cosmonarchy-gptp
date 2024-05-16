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
