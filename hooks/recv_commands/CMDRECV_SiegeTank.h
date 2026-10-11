#pragma once
#include "../../BW/structures/Unit.h"

namespace hooks {
	char __fastcall CMDACT_Siege(u32 unused, char siege);	// 00423450
	void CMDRECV_Siege(u8 is_queued);						// 004C1E80
	void CMDRECV_Unsiege(u8 is_queued);						// 004C1F10

	void inject_cmdrecv_siege_tank_hooks();
}
