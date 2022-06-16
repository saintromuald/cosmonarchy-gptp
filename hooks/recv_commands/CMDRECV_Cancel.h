#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void cmdrecv_cancel_nuke();					//004BFCD0
	void cmdrecv_cancel_addon();				//004BFF30
	void cmdrecv_cancel_upgrade();				//004BFFC0
	void cmdrecv_cancel_research();				//004C0070
	void cmdrecv_cancel_train(u16 wUnitID);		//004C0100
	void cmdrecv_cancel_unit_morph();			//004C2EC0
	void cmdrecv_cancel_construction();			//004C2EF0

	void inject_cmdrecv_cancel_hooks();
}