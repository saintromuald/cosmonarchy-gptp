#pragma once
#include "SCBW\api.h"
#include "unit_constants.h"

enum ChkUnitFlags {
	ChkUnitFlags_Cloaked      = 0x01, // Ai Layout
	ChkUnitFlags_Burrowed     = 0x02, 
	ChkUnitFlags_Lifted       = 0x04,
	ChkUnitFlags_Hallucinated = 0x08, // Dont Request
	ChkUnitFlags_Invincible   = 0x10, // Main Town
};

// This structure is mirrored in aise, be careful when you change it 
struct AiPlaceholder {
	Point16 position;
	u16 unit_id;
	u8 priority;
	u8 guard_rebuilds;
	u8 resarea;
	b8 was_preplaced;
	b8 should_request;
	b8 is_townhall_marker;

	// Runtime data
	b8 disabled;
	b8 inited;
	b8 requested;
	u8 placement_timeout;
	u32 yield_score;
	AiTown* town;
	GuardAi* guard_ai;
	graphics::ColorId color;
};

#define PlaceholderLimit 2048
struct AiLayouts {
	u32 placeholders_count;
	AiPlaceholder placeholders[PlaceholderLimit];
	bool should_sort;
};

struct AiLayoutsTown {
	bool inited;
	bool wants_no_resarea;
	u32 worker_limit;
	u32 worker_maynard;
	u32 request_count[UnitId::Count]; // First used to set wait_build_start_count, then used to track request counts
};

struct AiUnitEquivalent {
	u32 from;
	u32 to;
	s32 priority_decrease;
	b32 merge_requests;
};

const AiUnitEquivalent equivalents[] = {
	{UnitId::ZergHachirosk, UnitId::ZergCaphrolosk, 1},
	{UnitId::ZergHachirosk, UnitId::ZergGathtelosk, 1},
	{UnitId::ZergCaphrolosk, UnitId::ZergSibraelosk, 1},
	{UnitId::ZergGathtelosk, UnitId::ZergNephrilosk, 1},

	{UnitId::ZergIrolIris, UnitId::ZergIrulentIvoa, 2},
	{UnitId::ZergIrolIris, UnitId::ZergOthstolOviform, 2},
	{UnitId::ZergIrulentIvoa, UnitId::ZergOthstolOviform, 2},
	{UnitId::ZergOthstolOviform, UnitId::ZergOrbothOmlia, 2},

	{UnitId::ZergQuazrokPool, UnitId::ZergLiivralPond, 1},
	{UnitId::ZergMuthrokSpire, UnitId::ZergVithrathHaunt, 1},
	{UnitId::ZergAxtothAxis, UnitId::ZergBegrozithBarrow, 1},
	{UnitId::ZergHydrokDen, UnitId::ZergGorgralSwamp, 1},
	{UnitId::ZergIzkagIteth, UnitId::ZergChokralgantChazsal, 1},
	{UnitId::ZergGeszkathGrotto, UnitId::ZergAlkajChasm, 1},
	{UnitId::ZergZorkizShroud, UnitId::ZergUltrokCavern, 1},
	{UnitId::ZergMatravilNest, UnitId::ZergAlmakisAntre, 1},
	{UnitId::ZergElthisthMound, UnitId::ZergRakzogthRoost, 1},

	{UnitId::ZergCagrantColony, UnitId::ZergSurkithColony, 0, true},
	{UnitId::ZergCagrantColony, UnitId::ZergSpraithColony, 0, true},
	{UnitId::ZergCagrantColony, UnitId::ZergLarvoskColony, 0, true},
	{UnitId::ZergCagrantColony, UnitId::ZergKazitrithColony, 0, true},
	{UnitId::ZergCagrantColony, UnitId::ZergOcsalColony, 0, true},

	{UnitId::ZergDroleth, UnitId::ZergGosvileth},
	{UnitId::ZergOvileth, UnitId::ZergIroleth},
	{UnitId::ZergOthstoleth, UnitId::ZergAlaszileth},

	{UnitId::ZergZethrokor, UnitId::ZergVorvrokor},
	{UnitId::ZergQuazilisk, UnitId::ZergLiiralisk},
	{UnitId::ZergNathrokor, UnitId::ZergKalkalisk},
	{UnitId::ZergCikralisk, UnitId::ZergGhitorokor},
	{UnitId::ZergProtathalor, UnitId::ZergSovroleth},
	{UnitId::ZergMutalisk, UnitId::ZergTosgrilisk},
	{UnitId::ZergAxitrilisk, UnitId::ZergBegrothilisk},
	{UnitId::ZergMortothrokor, UnitId::ZergOztukrokor},
	{UnitId::ZergHydralisk, UnitId::ZergBactalisk},
	{UnitId::ZergIzirokor, UnitId::ZergTethzorokor},
	{UnitId::ZergSkithrokor, UnitId::ZergVithrilisk},
	{UnitId::ZergKagralisk, UnitId::ZergCherbrathalor},
	{UnitId::ZergEvigrilisk, UnitId::ZergOkrimnilisk},
	{UnitId::ZergAlmaksalisk, UnitId::ZergAlgoztalisk},
	{UnitId::ZergKeskathalor, UnitId::ZergKethrostalor},
	{UnitId::ZergGeszithalor, UnitId::ZergAlkajelisk},
	{UnitId::ZergZoryusthaleth, UnitId::ZergUltrokor},
	{UnitId::ZergLakizilisk, UnitId::ZergZobriolisk},
	{UnitId::ZergGorgrokor, UnitId::ZergZarcavrokor},
	{UnitId::ZergKonvilisk, UnitId::ZergTetcaerokor},
	{UnitId::ZergVilgorokor, UnitId::ZergSelkerilisk},
	{UnitId::ZergMatraleth, UnitId::ZergAkistrokor},
	{UnitId::ZergIsthrathaleth, UnitId::ZergOgroszaleth},
	{UnitId::ZergElthrazalisk, UnitId::ZergRakzilisk},
};

#define ChkCacheLimit UNIT_ARRAY_LENGTH
extern u32 chk_cache_size;
extern CHKUnit chk_cache[ChkCacheLimit];

extern bool show_player_layouts[PlayerLimit];
extern u32 default_ai_expansion_block[PlayerLimit];

extern AiLayouts player_layouts[PlayerLimit];
extern AiLayoutsTown layouts_towns[AiTownLimit];

void init_layouts();
void update_layouts();

Point16 get_ai_town_position(AiTown* town);
AiLayoutsTown* get_layouts_town(AiTown* town);
u32 ai_get_next_custom_expo(AiTown* main_town);

u32 find_unit_base(u32 unit_id);
AiUnitEquivalent find_unit_equivalent_down(u32 unit_id);
AiUnitEquivalent find_unit_equivalent_up(u32 unit_id, u32* index = 0);
u32 find_unit_merge(u32 unit_id);
u32 find_unit_root_merge(u32 unit_id);

u32 town_count_units(AiTown* town, u32 unit_id, bool completed);
u32 town_has_down_equivalent(AiTown* town, u32 unit_id);
u32 town_count_up_equivalents(AiTown* town, u32 unit_id);

u32 player_count_units(u32 player, u32 unit_id, bool completed);
u32 player_count_up_equivalents(u32 player, u32 unit_id);

void add_placeholder_by_unit_id(AiLayouts* layouts, AiPlaceholder* placeholder, u32 unit_id);
void add_placeholder(CHKUnit* unit, u32 player);
void sort_layouts(AiLayouts* layouts);

bool unit_has_creep(u32 unit_id, u32 x, u32 y);
bool unit_has_no_or_allied_creep(u32 unit_id, u32 x, u32 y, u32 player);
bool is_basic_townhall(u32 unit_id);

bool can_place_unit(u32 unit_id, Point16 position, u32 player);
bool can_build_placeholder(AiLayouts* layouts, AiTown* town, AiPlaceholder* placeholder);
bool has_expanding_town(u32 player);
void init_placeholder(AiTown* town, AiPlaceholder* placeholder);
void request_placeholder(AiTown* town, AiLayoutsTown* layouts_town, AiPlaceholder* placeholder);