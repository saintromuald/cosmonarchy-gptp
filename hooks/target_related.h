#pragma once
#include "SCBW/api.h"
#include "SCBW/scbwdata.h"
namespace hooks {
/*	s32 __fastcall setUnderDisruptionWeb(s32 a1);
	*/
//	s32 setUnderDisruptionWeb(CUnit* a1);
//	s32 updateDisruptionWebStatus();
	s32 genericRightClickUnitProc2(CUnit* unit, s32 a2, s16 a3, CUnit* target, s16 unitID); // 00455E00
	CUnit* CMDRECV_HoldPosition(s32 a1); // 004C20C0
	bool unitCanSeeCloakedTarget(CUnit* target, CUnit* unit); // 00401D60
	extern void injectTargetRelatedHooks();
}