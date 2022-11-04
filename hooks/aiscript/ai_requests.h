#pragma once
#include "SCBW\structures.h"

namespace hooks
{
	void ai_build_tech_upgrade(AiTown* town, char method, char amount, s16 id, char priority, bool need);					// 00431F90
	AiTown* ai_progress_build(s32 player, s32* minerals, s32* gas);															// 00434480
	char ai_perform_task(Unit* unit);																						// 004361A0
	char ai_plan_do_merge_morph(s32 player, Unit* unit, s32 boolSomething);													// 00435F10
	void ai_train_defensive_military(u8 player_id);																			// 0043F990
	void ai_build(u32 unit_id, bool need, u32 count, AiTown* town, u8 priority);											// 004323D0
	char ai_build_refinery(AiTown* town);																					// 00433660
	void ai_request_guard(GuardAi* guard_ai, u32 player_id, u32 priority = 70);												// 00448630
	void ai_region_train_attack_force(AiRegion* region);																	// 0043E670
	void ai_add_town_spending_request(u32 player, u32 priority, u32 unit_id, AiRegion* region);								// 00448400
	void ai_update_guard_needs(u32 player);																					// 004630C0
	void ai_make_guard(u32 player_id, Unit* unit);																			// 0043A010
	void ai_add_spending_request_from_towns(u32 player);																	// 00436320
	void pop_spending_request(u32 player);																					// 004476D0
	s32 ai_try_progress_spending_queue_worker(AiTown* town, Unit* unit);													// 004348C0
	bool ai_add_spending_request(u32 id, u32 player, u32 type, u32 priority, void* value);									// 00447980
	s32 ai_make_detector_behavior(Unit* unit, u32 player_id);																// 00435130
	bool has_resources_for_unit(u16 unit_id, u32 player);																	// 004487B0

	void inject_ai_request_hooks();
}