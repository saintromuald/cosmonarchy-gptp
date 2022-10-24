#pragma once
#include "SCBW/api.h"

struct CustomAiPlayer {
	u32 temp;
};

struct CustomAiTown {
	bool active;
	u32 gas_timer;
	u8 gas_requested_count;
	bool gas_requested[3];
};

extern CustomAiPlayer custom_ai_player[PlayerLimit];
extern CustomAiTown custom_ai_towns[AiTownLimit];

void init_custom_ai();
void update_custom_ai();