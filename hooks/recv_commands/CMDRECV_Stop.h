#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void cmdrecv_reaver_stop();			// 004C1240
	void cmdrecv_carrier_stop();		// 004C1430
	void cmdrecv_stop(u8 bCommandType);	// 004C2190

	void inject_cmdrecv_stop_hooks();
}