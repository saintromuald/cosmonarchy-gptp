#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks {
	char __fastcall CMDACT_Siege(u32 unused, char siege);	// 00423450
	void CMDRECV_Siege(u8 command_type);					// 004C1E80
	void CMDRECV_Unsiege(u8 command_type);					// 004C1F10

	void injectCMDRECV_SiegeTankHooks();
}
