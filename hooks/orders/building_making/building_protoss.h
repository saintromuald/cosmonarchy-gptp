#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_ProbeBuild(Unit* unit);	//004E4D00
	void orders_BuildSelf2(Unit* building);//004E4F40

	void injectBuildingProtossHooks();

} //hooks