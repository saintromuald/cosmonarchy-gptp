#pragma once
#include "SCBW\api.h"

enum
{
	PlayerTintInvalid = -1,
	PlayerTintDarkRed,

	PlayerTintCount,
};

const std::vector<std::vector<u32>> tints =
{
	{
		PlayerColor::Red,
		PlayerColor::Redwood,
		PlayerColor::VividRed,
	},
	{

	},
};

u32 get_random_tint();
u32 get_random_color_from_tint(u32 tint);
u32 get_player_tint(u32 color);
bool can_use_player_tint(u32 color_tint, u32 last_player);
bool can_use_player_tint_ai(u32 color_tint, u32 last_player);
u32 get_color_for_personality(u32 player_id);

void update_player_colors();
void update_ai_player_colors();