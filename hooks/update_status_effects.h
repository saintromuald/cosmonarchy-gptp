#pragma once
#include <BW/structures/Unit.h>

namespace hooks {
	void update_status_effects(Unit* unit);						// 00492F70
	void ai_burrower(Unit* unit);								// 004A1340
	bool unit_is_always_detected(Unit* unit);					// 00491810
	b32 unit_is_clean_detector(Unit* unit);						// 00403430
	void refresh_carryable_sprite(Sprite* sprite);				// 00497750
	void update_carryable_sprite_flags(Sprite* sprite);			// 004981B0

	void inject_update_status_effects_hooks();
}