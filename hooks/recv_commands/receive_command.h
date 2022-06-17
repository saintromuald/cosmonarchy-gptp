#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void receive_command(s32 x, s32 y, Unit* unitParam, u32 wUnitType, u32 bActionOrder, u32 bCommandType); // 0049AB00

	void inject_receive_command_hooks();
}