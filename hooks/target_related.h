#pragma once
#include "SCBW/api.h"
#include "SCBW/scbwdata.h"
namespace hooks
{
	//	s32 __fastcall setUnderDisruptionWeb(s32 a1);		
	//	s32 setUnderDisruptionWeb(Unit* a1);
	//	s32 updateDisruptionWebStatus();

	s32 genericRightClickUnitProc2(Unit* unit, s32 a2, s16 a3, Unit* target, s16 unitID);				// 00455E00
	Unit* CMDRECV_HoldPosition(s32 a1);																	// 004C20C0
	u32 unitCanSeeCloakedTargetHook(Unit* target, Unit* unit);											// 00401D60
	u32 isTargetVisibleHook(Unit* target, Unit* unit);													// 004E5E30
	u32 isUnitVisibleHook(Unit* unit);																	// 004E5DB0
	void checkUnitVisibilityHook(Unit* unit);															// 00443390
	s32 isUnitCloakingHook(Unit* unit);																	// 004916E0
	void updateUnitVisibility(Unit* unit);																// Extracted part of checkUnitVisibility
	bool unitCanAttackTarget(Unit* target, Unit* unit, int checkVisibility);							// 
	s32 orderAttackBehavior(Unit *a1);																	// 00478540
	void AI_AttackUnit(Unit* unit);																		// 0043FFD0

	void injectTargetRelatedHooks();
}