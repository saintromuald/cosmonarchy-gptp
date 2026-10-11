#pragma once
#include "BW/api.h"

struct CustomAiPlayer {
};

struct CustomAiTown {
	bool active;
	u32 gas_timer;
	u32 gas_cap;
};

inline CustomAiPlayer custom_ai_players[PlayerLimit];
inline CustomAiTown custom_ai_towns[AiTownLimit];

void init_custom_ai();
void update_custom_ai();