#pragma once
#include "SCBW/scbwdata.h"

namespace hooks
{
	void show_health_bar(Unit* unit);
	void compile_healthbar(Image* image, Sprite* sprite);	// 004D6010

	void inject_hp_bars_hooks();

	extern Unit* _unitTemp;
}