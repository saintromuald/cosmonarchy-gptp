#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

void CMDRECV_Siege(uint8 bCommandType);	//004C1E80
void CMDRECV_Unsiege(uint8 bCommandType);	//004C1F10

void injectCMDRECV_SiegeTankHooks();

} //hooks
