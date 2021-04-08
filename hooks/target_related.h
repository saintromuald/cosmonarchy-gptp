#pragma once
#include "SCBW/api.h"
#include "SCBW/scbwdata.h"
namespace hooks {
	/*	int32 __fastcall setUnderDisruptionWeb(int32 a1);
		*/
		//	int32 setUnderDisruptionWeb(CUnit* a1);
		//	int32 updateDisruptionWebStatus();
	int32 genericRightClickUnitProc2(CUnit* unit, int32 a2, int16 a3, CUnit* target, int16 unitID);		// 00455E00
	CUnit* CMDRECV_HoldPosition(int32 a1);																// 004C20C0
	uint32 unitCanSeeCloakedTarget(CUnit* target, CUnit* unit);											// 00401D60
	uint32 isTargetVisible(CUnit* target, CUnit* unit);													// 004E5E30
	uint32 isUnitVisible(CUnit* unit);																	// 004E5DB0
	void checkUnitVisibility(CUnit* unit);																// 00443390
	extern void injectTargetRelatedHooks();
}