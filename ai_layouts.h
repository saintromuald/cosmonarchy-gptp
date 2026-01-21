#pragma once
#include "BW\api.h"
#include "unit_constants.h"
#include <array>

enum ChkUnitFlags {
	ChkUnitFlags_Cloaked      = 0x01, // Ai Layout
	ChkUnitFlags_Burrowed     = 0x02, 
	ChkUnitFlags_Lifted       = 0x04,
	ChkUnitFlags_Hallucinated = 0x08, // Dont Request
	ChkUnitFlags_Invincible   = 0x10, // Main Town
};

// This structure is mirrored in aise, be careful when you change it 
// TODO: Make a SetBaseLayoutRequest type instead, and allow this to be modified freely
struct AiPlaceholder {
	Point16 position;
	UnitId::Enum unit_id;
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

struct PlayerLayouts {
	std::vector<AiPlaceholder> placeholders;
	bool should_sort;
};

struct TownLayouts {
	bool inited;
	bool wants_no_resarea;
	u32 worker_limit;
	u32 worker_maynard;
	std::unordered_map<UnitId::Enum, u32> request_count; // First used to set wait_build_start_count, then used to track request counts
};

struct AiUnitEquivalent {
	UnitId::Enum from{UnitId::None};
	UnitId::Enum to{UnitId::None};
	s32 priority_decrease{0};
	bool merge_requests{false};
};

inline const AiUnitEquivalent equivalents[] = {
	{UnitId::ZergHachirosk, UnitId::ZergCaphrolosk, 1},
	{UnitId::ZergHachirosk, UnitId::ZergGathtelosk, 1},

	{UnitId::ZergExcisant, UnitId::ZergEzigrisant, 1},

	{UnitId::ZergIrolIris, UnitId::ZergOthstolOviform, 2},

	{UnitId::ZergQuazethPool, UnitId::ZergVornathPond, 1},
	{UnitId::ZergQuazethPool, UnitId::ZergKalkiirLake, 1},
	{UnitId::ZergVornathPond, UnitId::ZergKalkiirLake, 1},
	{UnitId::ZergMutathSpire, UnitId::ZergTosvrolHaunt, 1},
	{UnitId::ZergAxtothAxis, UnitId::ZergBegrozithBarrow, 1},
	{UnitId::ZergHydrithDen, UnitId::ZergSkibactScab, 1},
	{UnitId::ZergHydrithDen, UnitId::ZergTethvithSwamp, 1},
	{UnitId::ZergSkibactScab, UnitId::ZergTethvithSwamp, 1},
	{UnitId::ZergAlkagIteth, UnitId::ZergChokralChazsal, 1},
	{UnitId::ZergGeszkathGrotto, UnitId::ZergKethrajKeep, 1},
	{UnitId::ZergZorkizShroud, UnitId::ZergUlgorgCavern, 1},
	{UnitId::ZergUlgorgCavern, UnitId::ZergZozarcGorge, 1},
	{UnitId::ZergMatravilNest, UnitId::ZergAlmakisAntre, 1},
	{UnitId::ZergRakzogthRoost, UnitId::ZergRakzogthRoost, 1},

	{UnitId::ZergCagrantCesiant, UnitId::ZergSurkithCesiant, 0, true},
	{UnitId::ZergCagrantCesiant, UnitId::ZergSpraithCesiant, 0, true},
	{UnitId::ZergCagrantCesiant, UnitId::ZergLarvoskCesiant, 0, true},
	{UnitId::ZergCagrantCesiant, UnitId::ZergKazitrithCesiant, 0, true},
	{UnitId::ZergCagrantCesiant, UnitId::ZergOcsalCesiant, 0, true},
	{UnitId::ZergCagrantCesiant, UnitId::ZergSkortrithCesiant, 0, true},
	{UnitId::ZergCagrantCesiant, UnitId::ZergSpirtithCesiant, 0, true},
	{UnitId::ZergCagrantCesiant, UnitId::ZergNydusCesiant, 0, true},

	{UnitId::ZergSurkithCesiant, UnitId::ZergSkortrithCesiant, 0, true},
	{UnitId::ZergSpraithCesiant, UnitId::ZergSpirtithCesiant, 0, true},

	{UnitId::ZergDroleth, UnitId::ZergGosvileth},
	{UnitId::ZergOvileth, UnitId::ZergIroleth},

	{UnitId::ZergZethrokor, UnitId::ZergVorvrokor},
	{UnitId::ZergSovrokor, UnitId::ZergGarvrokor},
	{UnitId::ZergQuazilisk, UnitId::ZergLiiralisk},
	{UnitId::ZergQuazrokor, UnitId::ZergLiiralisk},
	{UnitId::ZergNathrokor, UnitId::ZergKalkalisk},
	{UnitId::ZergNathrelisk, UnitId::ZergKalkalisk},
	{UnitId::ZergCikralisk, UnitId::ZergGhitorokor},
	{UnitId::ZergProtathalor, UnitId::ZergSovroleth},
	{UnitId::ZergMutalisk, UnitId::ZergTosgrilisk},
	{UnitId::ZergAxitrilisk, UnitId::ZergBegrothilisk},
	{UnitId::ZergMortothrokor, UnitId::ZergOztukrokor},
	{UnitId::ZergHydralisk, UnitId::ZergBactalisk},
	{UnitId::ZergHydraliskOrkoti, UnitId::ZergBactalisk},
	{UnitId::ZergIzirokor, UnitId::ZergTethzorokor},
	{UnitId::ZergIziralisk, UnitId::ZergTethzorokor},
	{UnitId::ZergSkithrokor, UnitId::ZergVithrilisk},
	{UnitId::ZergKagralisk, UnitId::ZergCherbrathalor},
	{UnitId::ZergEvigrilisk, UnitId::ZergOkrimnilisk},
	{UnitId::ZergAlmaksalisk, UnitId::ZergAlgoztalisk},
	{UnitId::ZergKeskathalor, UnitId::ZergKethrostalor},
	{UnitId::ZergGeszithalor, UnitId::ZergAlkajelisk},
	{UnitId::ZergZoryusthaleth, UnitId::ZergUltrokor},
	{UnitId::ZergInzorthrokor, UnitId::ZergUltrokor},
	{UnitId::ZergLakizilisk, UnitId::ZergZobriolisk},
	{UnitId::ZergGorgrokor, UnitId::ZergZarcavrokor},
	{UnitId::ZergAzanthrokor, UnitId::ZergZarcavrokor},
	{UnitId::ZergKonvilisk, UnitId::ZergTetcaerokor},
	{UnitId::ZergVilgorokor, UnitId::ZergIsthrathaleth},
	{UnitId::ZergMatraleth, UnitId::ZergAkistrokor},
	{UnitId::ZergIsthrathaleth, UnitId::ZergOgroszaleth},
	{UnitId::ZergElthrazalisk, UnitId::ZergRakzilisk},
};

inline std::vector<CHKUnit> g_chk_cache;

inline bool g_show_player_layouts[PlayerLimit];
inline u32 g_default_ai_expansion_block[PlayerLimit];

class AiLayouts {
public:
	void init();
	void update();

	void add_placeholder_unit_id(PlayerLayouts& layouts, AiPlaceholder& placeholder, UnitId::Enum unit_id);
	void add_placeholder_chk(const CHKUnit& unit, u32 player);

	Point16 get_ai_town_position(AiTown& town);
	PlayerLayouts& get_player_layouts(u32 player);
	TownLayouts& get_town_layouts(AiTown& town);
	u32 ai_get_next_custom_expo(AiTown& main_town);
	bool has_expanding_town(u32 player);

private:
	void init_first_tick();
	bool can_build_placeholder(AiTown& town, AiPlaceholder& placeholder);
	void init_placeholder(AiTown& town, AiPlaceholder& placeholder);
	void request_placeholder(AiTown& town, TownLayouts& layouts_town, AiPlaceholder& placeholder);
	void sort_layouts(PlayerLayouts& layouts);

private:
	std::array<PlayerLayouts, PlayerLimit> m_player_layouts;
	std::array<TownLayouts, AiTownLimit> m_town_layouts;
};

inline AiLayouts g_ai_layouts;

UnitId::Enum find_equivalent(UnitId::Enum unit_id);
UnitId::Enum find_equivalent_merge(UnitId::Enum unit_id);
UnitId::Enum find_equivalent_root_merge(UnitId::Enum unit_id);
AiUnitEquivalent find_unit_equivalent_down(UnitId::Enum unit_id);
AiUnitEquivalent find_unit_equivalent_up(UnitId::Enum unit_id, u32* index = 0);

u32 town_count_units(AiTown& town, UnitId::Enum unit_id, bool completed);
u32 town_has_down_equivalent(AiTown& town, UnitId::Enum unit_id);
u32 town_count_up_equivalents(AiTown& town, UnitId::Enum unit_id);

u32 player_count_units(u32 player, UnitId::Enum unit_id, bool completed);
u32 player_count_up_equivalents(u32 player, UnitId::Enum unit_id);

bool unit_has_creep(UnitId::Enum unit_id, u32 x, u32 y);
bool unit_has_no_or_allied_creep(UnitId::Enum unit_id, u32 x, u32 y, u32 player);
bool can_place_unit(UnitId::Enum unit_id, Point16 position, u32 player);