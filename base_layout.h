#pragma once
#include "SCBW\api.h"
#include "unit_constants.h";

struct BaseLayout
{
	Point16 position;
	u8 player_id;
	u8 unit_id;
	u8 priority;
	b8 active;
	AiTown* town;
};

void init_base_layouts();
void update_base_layouts();

