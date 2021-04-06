#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

	void receive_command(int x, int y, CUnit* unitParam, uint32 wUnitType, uint32 bActionOrder, uint32 bCommandType); //0x0049AB00

	void injectRecvCmdHook();

} //hooks
