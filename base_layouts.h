#pragma once
#include "SCBW\api.h"
#include "unit_constants.h"

struct Placeholder {
	Point16 position;
	u8 unit_id;
	u8 priority;
	b8 should_request;
	b8 is_placeholder;
};

#define PlaceholderLimit 1024

struct BaseLayouts {
	AiTown* town;
	u32 units_count;
	bool units_requested[PlaceholderLimit];
	Placeholder units[PlaceholderLimit];
	u32 placeholder_request_count[UnitId::Count];
	u32 request_count[UnitId::Count];
};

void tick_base_layouts();

