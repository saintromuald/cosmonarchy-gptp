#include <BW/structures/Unit.h>

namespace hooks {
	u32 get_tech_use_error_message_hook(Unit* target, u8 player, u16 tech_id);	// 00491E80

	void inject_tech_target_check_hooks();
}