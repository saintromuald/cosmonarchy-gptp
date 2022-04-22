#pragma once
#include "SCBW\api.h"
#include "unit_constants.h"

#define BaseLayoutLimit 256

struct BaseLayout {
	Point16 position;
	u8 unit_id;
	u8 priority;
	b8 should_request;
	b8 is_placeholder;
};

struct BaseLayouts {
	AiTown* town;
	u32 structures_count;
	bool structures_active[BaseLayoutLimit];
	BaseLayout structures[BaseLayoutLimit];
	u32 units_count;
	bool units_active[BaseLayoutLimit];
	BaseLayout units[BaseLayoutLimit];
	u32 request_count[UnitId::Count];
	u32 preplaced_request_count[UnitId::Count];
};

void tick_base_layouts();

