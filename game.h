#pragma once
#include "BW/api.h"

struct QuickCasting {
	u8 timeout = 0;
	u8 building_timeout = 0;
	bool signal = false;
	u32 command_card = UnitId::None;

	void tick();
};

struct Game {
	QuickCasting quick_casting{};
	Unit* transport_ui_unit = nullptr;

	// TODO: move most in-game global state here, since here we know the lifetime of the state will be reset for each game

	Game();
	void tick();
};

inline std::unique_ptr<Game> g_game;
