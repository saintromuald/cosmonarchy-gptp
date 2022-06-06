#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void CMDRECV_BuildingMorph(u16 wUnitType);	// 004C1910
	void CMDRECV_UnitMorph(u16 wUnitType);		// 004C1990
	void cmdrecv_train(char* buffer);			// 004C1C20

	void injectCMDRECV_MorphHooks();
}