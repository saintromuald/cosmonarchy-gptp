#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_tank_mode(Unit* unit);										// 00464AE0
	void orders_siege_mode(Unit* unit);										// 00464BD0
	s32 __fastcall siege_tank_self_destruct_proc(Unit* other, Unit* self);	// 00463F90

	void inject_siege_transform_hooks();
}