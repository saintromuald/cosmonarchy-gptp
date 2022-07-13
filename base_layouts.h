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

// This structure is mirrored in aise, be careful when you change it 
struct Placeholder {
	Point16 position;
	u8 unit_id;
	u8 priority;
	u8 guard_rebuilds;
	u8 resarea;
	b8 was_preplaced;
	b8 should_request;

	// Runtime data
	b8 disabled;
	b8 inited;
	b8 requested;
	u8 placement_timeout;
	AiTown* town;
};

#define PlaceholderLimit 2048
struct Layouts {
	u32 placeholders_count;
	Placeholder placeholders[PlaceholderLimit];
};

struct LayoutsTown {
	bool inited;
	bool can_ignore_resarea;
	u32 request_count[UnitId::Count]; // First used to set wait_build_start_count, then used to track request counts
};

struct Equivalent {
	u32 from;
	u32 to;
	s32 priority_decrease;
	b32 merge_requests;
};

const Equivalent equivalents[] = {
	{UnitId::ZergHatchery, UnitId::ZergLair, 1},
	{UnitId::ZergLair, UnitId::ZergHive, 1},
	{UnitId::ZergHive, UnitId::ZergSire, 1},

	{UnitId::ZergIrolIris, UnitId::ZergOthstolOviform, 2},
	{UnitId::ZergOthstolOviform, UnitId::ZergAlaszilArbor, 2},

	{UnitId::ZergQuazrokPool, UnitId::ZergLiivralPond, 1},
	{UnitId::ZergHydrokDen, UnitId::ZergGorgralSwamp, 1},
	{UnitId::ZergMuthrokSpire, UnitId::ZergVithrathHaunt, 1},
	{UnitId::ZergMatravilNest, UnitId::ZergAlmakisAntre, 1},
	{UnitId::ZergZorkizShroud, UnitId::ZergUltrokCavern, 1},
	{UnitId::ZergGeszkathGrotto, UnitId::ZergAlkajChasm, 1},

	{UnitId::ZergCreepColony, UnitId::ZergSunkenColony, 0, true},
	{UnitId::ZergCreepColony, UnitId::ZergSporeColony, 0, true},
	{UnitId::ZergCreepColony, UnitId::ZergLarvalColony, 0, true},

	{UnitId::ZergDroleth, UnitId::ZergGosvileth},
	{UnitId::ZergOvileth, UnitId::ZergIroleth},
	{UnitId::ZergOthstoleth, UnitId::ZergAlaszileth},

	{UnitId::ZergZethrokor, UnitId::ZergVorvrokor},
	{UnitId::ZergQuazilisk, UnitId::ZergLiiralisk},
	{UnitId::ZergHydralisk, UnitId::ZergBactalisk},
	{UnitId::ZergSkithrokor, UnitId::ZergGorgrokor},
	{UnitId::ZergMutalisk, UnitId::ZergVithrilisk},
	{UnitId::ZergNathrokor, UnitId::ZergKalkalisk},
	{UnitId::ZergKonvilisk, UnitId::ZergAlmaksalisk},
	{UnitId::ZergMatraleth, UnitId::ZergAkistrokor},
	{UnitId::ZergZoryusthaleth, UnitId::ZergUltrokor},
	{UnitId::ZergKeskathalor, UnitId::ZergAlkajelisk},

	{UnitId::ProtossCantavis, UnitId::ProtossAugur},
};

#define ChkCacheLimit UNIT_ARRAY_LENGTH
extern u32 chk_cache_size;
extern CHKUnit chk_cache[ChkCacheLimit];

extern bool log_placeholder_requests[PlayerLimit];
extern u32 default_ai_expansion_block[PlayerLimit];

extern Layouts player_layouts[PlayerLimit];
extern LayoutsTown layouts_towns[AiTownLimit];

void init_layouts();
void update_layouts();

LayoutsTown* get_layouts_town(AiTown* town);
u32 ai_get_next_custom_expo(AiTown* main_town);

Equivalent find_unit_equivalent_down(u32 unit_id);
Equivalent find_unit_equivalent_up(u32 unit_id);
u32 has_down_equivalent(AiTown* town, u32 unit_id);
u32 number_of_equivalents_up(AiTown* town, u32 unit_id);
u32 find_unit_merge(u32 unit_id);
u32 find_unit_base_merge(u32 unit_id);

void add_placeholder_by_unit_id(Layouts* layouts, Placeholder* placeholder, u32 unit_id);
void add_placeholder(CHKUnit* unit, u32 player);
void sort_layouts(Layouts* layouts);

u32 count_units_in_town(AiTown* town, u32 unit_id);
bool unit_has_creep(u32 unit_id, u32 x, u32 y);
bool unit_has_no_or_allied_creep(u32 unit_id, u32 x, u32 y, u32 player);
bool is_basic_townhall(u32 unit_id);

bool can_place_unit(u32 unit_id, Point16 position, u32 player);
bool can_build_placeholder(Layouts* layouts, AiTown* town, Placeholder* placeholder);
bool has_expanding_town(u32 player);
void init_placeholder(AiTown* town, Placeholder* placeholder);
void request_placeholder(AiTown* town, LayoutsTown* layouts_town, Placeholder* placeholder);