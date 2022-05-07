#pragma once
#include "SCBW\api.h"
#include "unit_constants.h"

enum ChkUnitFlags {
	ChkUnitFlags_Cloaked      = 0x01,
	ChkUnitFlags_Burrowed     = 0x02,
	ChkUnitFlags_Lifted       = 0x04,
	ChkUnitFlags_Hallucinated = 0x08,
	ChkUnitFlags_Invincible   = 0x10,
};

#define ChkCacheLimit UNIT_ARRAY_LENGTH
extern u32 chk_cache_size;
extern CHKUnit chk_cache[ChkCacheLimit];

// This structure is mirrored in aise, be careful when you change it 
struct Placeholder {
	Point16 position;
	u8 unit_id;
	u8 priority;
	u8 guard_rebuilds;
	u8 wait_build_start_count;
	b8 was_preplaced;
	b8 should_request;
};

#define PlaceholderLimit 512
struct BaseLayouts {
	AiTown* town;
	Point16 resarea_pos;
	u32 placeholders_count;
	Placeholder placeholders[PlaceholderLimit];
	bool placeholders_requested[PlaceholderLimit];
	u32 request_count[UnitId::Count]; // First used to set wait_build_start_count, then used to track request counts
};

b32 is_tech_unitid(u32 unit_id);
void tick_base_layouts();