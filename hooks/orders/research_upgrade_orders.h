#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_Upgrade(CUnit* unit);		//004546A0
	void orders_ResearchTech(CUnit* unit);	//004548B0

	void injectResearchUpgradeOrdersHooks();

} //hooks