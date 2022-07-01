#pragma once
#include "SCBW/api.h"

struct CustomAiPlayer {
	u32 temp;
};

struct CustomAiTown {
	bool active;
	u32 gas_cooldown;
	u32 gas_requested;
};

extern CustomAiPlayer custom_ai_player[PlayerLimit];
extern CustomAiTown custom_ai_towns[AiTownLimit];

void init_custom_ai();
void update_custom_ai();