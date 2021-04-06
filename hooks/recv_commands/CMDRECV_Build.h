#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

	bool CMDRECV_PlaceBuildingAllowed(CUnit* builder, uint8 orderId, uint16 builtUnitId);	//0048DBD0
	void cmdRECV_PlaceBuildingNormal(int16 x, int16 y, uint32 orderId, uint32 builtUnitId);	//0048E010
	void cmdRECV_PlaceBuildingAddon(int16 x, int16 y, uint32 orderId, uint32 builtUnitId);	//0048E0A0
	void CMDRECV_Build(uint8 orderId, int16 x, int16 y, uint16 builtUnitId);					//004C23C0

	void injectCMDRECV_BuildHooks();

} //hooks
