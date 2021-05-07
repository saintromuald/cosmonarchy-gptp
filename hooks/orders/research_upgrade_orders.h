#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_Upgrade(Unit* unit);		//004546A0
	void orders_ResearchTech(Unit* unit);	//004548B0

	void injectResearchUpgradeOrdersHooks();

} //hooks