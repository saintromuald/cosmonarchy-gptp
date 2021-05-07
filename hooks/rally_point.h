#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {

void setRallyUnit(Unit* unit, Unit* target);			//0x00466B40
void orderNewUnitToRallyHook(Unit* unit, Unit* factory);	//0x00466F50
void setRallyPosition(Unit* unit, u16 x, u16 y);		//0x00466910

void injectRallyHooks();

} //hooks
