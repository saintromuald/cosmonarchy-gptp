#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	bool canEnterNydusCanal(Unit* unit, Unit* nydusCanal);		//004E8C20
	void enterNydusCanal_Effect(Unit* unit, Unit* nydusCanal);	//004EA180
	void orders_EnterNydusCanal(Unit* unit);						//004EA3E0

	void inject_enter_nydus_hooks();

} //hooks
