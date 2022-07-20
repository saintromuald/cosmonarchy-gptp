#pragma once
#include "SCBW/scbwdata.h"

namespace hooks
{
	void show_health_bar(Unit* unit);
	void compile_health_bar(Image* image, Sprite* sprite);

	void inject_hp_bars_hooks();

	extern Unit* _unitTemp;
}