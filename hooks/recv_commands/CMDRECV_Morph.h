#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void CMDRECV_BuildingMorph(u16 wUnitType);	// 004C1910
	void CMDRECV_UnitMorph(u16 wUnitType);		// 004C1990
	void cmdrecv_train(char* buffer);			// 004C1C20
	void cmdrecv_liftoff(char* buffer);			// 004C1620
	void btnsact_rally_point(u32 xd);			// 004244A0

	void injectCMDRECV_MorphHooks();
}