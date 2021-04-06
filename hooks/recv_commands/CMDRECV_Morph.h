#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

	void CMDRECV_BuildingMorph(uint16 wUnitType);	//004C1910
	void CMDRECV_UnitMorph(uint16 wUnitType);		//004C1990

	void injectCMDRECV_MorphHooks();

} //hooks
