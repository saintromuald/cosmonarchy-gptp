#pragma once

void augurSacrifice(Unit* unit, s32 radius = 6 * 24);
namespace hooks
{
	void injectUnitDestructorSpecial();
}