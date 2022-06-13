#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	void ai_unit_was_hit(Unit* self, Unit* attacker, s32 mainTargetReactions, s32 dontCallHelp);						// 0043F320
	bool get_best_worker_for_repair_proc(Unit* repairman, Unit* repairTarget);											// 004402B0
	b32 ai_military_should_wait_for_group(Unit* unit);																	// 0043AA10
	AiTown* delete_ai_town_if_needed(AiTown* town);																		// 004347E0
	bool ai_can_mine_extra(Unit* unit);																					// 00447A70
	void ai_order_unit_cloaking(Unit* unit);																			// 0043B970
	void ai_send_suicide(char argType, s32 player);																		// 0043E050
	u32 ai_recalculate_region_strength(AiRegion* region);																// 0043A390
	Unit* get_closest_enemy_structure(s32 x, s32 y, s32 player);														// 004A0F90
	s16 get_region_id_from_unit(Unit* unit);																			// 0049CB40
	bool ai_is_building_safe(Unit* unit);																				// 00438910
	bool ai_are_on_connected_regions(Unit* unit1, Unit* unit2);															// 00438E10

	s32 get_bunker_placement_position(u32 placementId, u32* regionId);													// 004442E0
	void ai_get_build_location(u32 unit_id, char* placementData, u32 player, Point16 desiredPos, s32 maxRange);		// 004461B0
	u32 get_turret_placement_position(char* placementData, u32 player, Point16 desiredPos, u16 unit_id, u32 unkFlag);	// 00443AC0
	bool ai_check_military_grouping(Unit* unit);																		// 00439550
	u32 __stdcall ai_remove_different_height(char* placement_data, Point16 desired_pos);								// 004BD0F0
	s32 attack_recent_attacker(Unit* unit);																				// 004770E0
	u32 ai_get_vespene_type(Unit* unit, u32 index, bool for_harvesting = false);										// 004326E0
	Unit* get_ai_refinery(AiTown* town, u32 gas_state, u32 index, bool for_harvesting = false);							// 004326E0
	void ai_worker_controller(Unit* worker);																			// 00435210
	char ai_reassign_worker_resources(AiTown* town);																	// 00435900
	void ai_auto_harvest(Unit* resource, Unit* worker, Unit* fallback);													// 00433B90
	u8 ai_get_build_order_id(Unit* unit, u32 unit_id);																	// 00434C31

	Unit* ai_find_best_repair_unit(Unit* unit);																			// 00443160
	s32 ai_scv_repair_controller(Unit* repairer, AiTown* town);															// 00433C70

	void ai_expand(AiScript* aiscript, u32 block, u32 amount);															// 0045B530

	struct PlacementData {
		bool tile_valid[64][64];
	};
	C_ASSERT(sizeof(PlacementData) == 0x1000);
	void __stdcall ai_remove_addon_placements(u16 unit_id, PlacementData* placement_data, u32 player, Point16 desired_position);	// 00443CC0

	void ai_manage_military_controller(Unit* unit);																		// 0043D910
	void ai_assign_region_to_slowest_unit(AiRegion* region);															// 00436F70
	void ai_respond_to_cloaked_threat(Unit* victim, Unit* threat);														// 0043C580
	u32 ai_try_return_home(Unit* unit, u32 dont_issue_order);															// 00462EA0

	void injectAIOrdersHooks();
}