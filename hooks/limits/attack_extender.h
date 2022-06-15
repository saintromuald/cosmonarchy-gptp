#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	u32 AI_TrainAttackForce(AiRegion* region);																		// 0043E670
	void AI_GroupingForAttack(AiRegion* region);																	// 0043EEC0
	u32 AI_GetMissingAttackForceUnits(AiRegion* region, u32* unitIds, u32 isCampaign);								// 00438050
	void AI_AddToAttack(u32 result, u32 player, u32 unit_id);														// 00447230
	bool ai_attack_manager(u32 _player, s32 x1, s32 y1, u32 alywaysOverwriteLastAttack, u32 allowFallbackAirOnly);	// 0043ABB0		
	s32 AI_HasUnitsForAttackForce(u32 player);																		// 00437F20
	void AI_RemoveFromAttack(s32 unit_id, s32 player);																// 00447040

	void inject_ai_attack_extender();
}

constexpr s32 ATTACK_LIMIT = 64;