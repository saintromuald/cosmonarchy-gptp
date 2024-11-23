#pragma once
#include "BW/api.h"

struct QuickCasting {
	u8 timeout = 0;
	u8 building_timeout = 0;
	bool signal = false;
	u32 command_card = UnitId::None;
};

struct Game {
	QuickCasting m_quick_casting;
	Unit* m_transport_ui_unit{};
	std::vector<TileCoord> m_collision_set_tiles;
	std::vector<TileCoord> m_collision_fill_tiles;

	// TODO: move most in-game global state here, since here we know the lifetime of the state will be reset for each game

	void first_frame();
	void tick();
};

inline std::unique_ptr<Game> g_game;
