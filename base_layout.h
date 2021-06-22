#pragma once
#include "SCBW\api.h"
#include "unit_constants.h";

struct BaseLayout
{
	Point16 position;
	u8 playerId;
	u8 unitId;
	u8 priority;
	b8 active;
	AiTown* town;
};

void InitBaseLayouts();
void UpdateBaseLayouts();

