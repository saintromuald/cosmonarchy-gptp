#pragma once
#include "SCBW\api.h"
#include "unit_constants.h";

#define BaseLayoutLimit 256

struct BaseLayout {
	Point16 position;
	u8 unit_id;
	u8 priority;
	b8 should_request;
};

struct BaseLayouts {
	AiTown* town;
	u32 structures_count;
	BaseLayout structures[BaseLayoutLimit];
	u32 units_count;
	BaseLayout units[BaseLayoutLimit];
};

void tick_base_layouts();

