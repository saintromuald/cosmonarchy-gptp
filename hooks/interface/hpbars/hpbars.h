#pragma once
#include "BW/bwdata.h"

namespace hooks
{
	void show_health_bar(Unit* unit);
	void compile_healthbar(Image* image, Sprite* sprite);	// 004D6010
	void remove_healthbar(Image* image);					// 004D5030
	Image* create_healthbar(u32 unused, Sprite* sprite);	// 004D6420

	void inject_hp_bars_hooks();

	extern Unit* temp_unit;
}