#pragma once
#include "../../BW/structures/Unit.h"

namespace hooks
{

	void orders_upgrade(Unit* unit);		//004546A0
	void orders_research_tech(Unit* unit);	//004548B0

	void injectResearchUpgradeOrdersHooks();

} //hooks