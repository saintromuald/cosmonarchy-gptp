#pragma once
#include "SCBW/api.h"
#include "SCBW/scbwdata.h"
namespace hooks
{
	//	s32 __fastcall setUnderDisruptionWeb(s32 a1);		
	//	s32 setUnderDisruptionWeb(CUnit* a1);
	//	s32 updateDisruptionWebStatus();

	s32 genericRightClickUnitProc2(CUnit* unit, s32 a2, s16 a3, CUnit* target, s16 unitID);		// 00455E00
	CUnit* CMDRECV_HoldPosition(s32 a1);																// 004C20C0
	u32 unitCanSeeCloakedTargetHook(CUnit* target, CUnit* unit);											// 00401D60
	u32 isTargetVisibleHook(CUnit* target, CUnit* unit);												// 004E5E30
	u32 isUnitVisibleHook(CUnit* unit);																// 004E5DB0
	void checkUnitVisibilityHook(CUnit* unit);																// 00443390
	s32 isUnitCloakingHook(CUnit* unit);																	// 004916E0
	void updateUnitVisibility(CUnit* unit);																// Extracted part of checkUnitVisibility
	extern void injectTargetRelatedHooks();
}