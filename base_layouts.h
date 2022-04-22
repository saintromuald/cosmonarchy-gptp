#pragma once
#include "SCBW\api.h"
#include "unit_constants.h"

struct Placeholder {
	Point16 position;
	u8 unit_id;
	u8 priority;
	b8 should_request;
	b8 was_preplaced;
	u16 wait_build_start_count;
};

#define PlaceholderLimit 512

struct BaseLayouts {
	AiTown* town;
	u32 placeholders_count;
	bool placeholders_requested[PlaceholderLimit];
	Placeholder placeholders[PlaceholderLimit];
	u32 request_count[UnitId::Count]; // First used to set wait_build_start_count, then used to track request counts
};

void tick_base_layouts();

