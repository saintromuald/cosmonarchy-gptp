#pragma once
#include "SCBW\structures.h"

namespace hooks
{
	void AI_BuildTechUpgrade(AiTown* town, char method, char amount, s16 id, char priority, bool need);	        // 00431F90
	AiTown* AI_ProgressBuild(s32 playerId, s32* minerals, s32* gas);											// 00434480
	s32 AIManageBuildingPlacement(Unit* builder, u32 unitId, Point16 tryPosition, Point16* outPosition, s32 areaTiles);
	char AI_PerformTask(Unit* unit);																			// 004361A0
	char AI_PlanDoMergeMorph(s32 player, Unit* unit, s32 boolSomething);										// 00435F10
	void ai_train_defensive_military(BYTE player_id);															// 0043F990
	void AI_Build(u32 unit_id, bool need, u32 count, AiTown* town, u8 priority);								// 004323D0

	void injectAIRequestsHooks();
}