#include <SCBW/structures/Unit.h>

namespace hooks {
	u16 get_tech_use_error_message_hook(Unit* target, u8 castingPlayer, u16 techId);	// 00491E80

	void inject_tech_target_check_hooks();
}