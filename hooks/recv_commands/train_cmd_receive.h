#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void cmdrecv_train_fighter();		// 004C1800
	void cmdrecv_train(u16 unit_id);	// 004C1C20

	void inject_train_cmdrecv_hooks();
}