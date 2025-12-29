#include "fraud_launcher.h"
#include "hook_tools.h"

namespace hooks {
void glu_wrapper(u32 original_address) {
	__asm {
		PUSHAD
	}
	if (fraud_launcher.connected() && fraud_launcher.handle_messages()) {
		__asm {
			POPAD
		}
		return;
	}
	__asm {
		POPAD
		CALL original_address
	}
}

void __stdcall glu_main_wrapper() { // 0x004de4a4 -> 0x004db930
	g_is_brood_war = true;
	glu_wrapper(0x004db930);
}

void __stdcall glu_connect_wrapper() { // 0x004de4e1 -> 0x004baf30
	glu_wrapper(0x004baf30);
}

void __stdcall glu_login_wrapper() { // 0x004de4b4 -> 0x004edc30
	glu_wrapper(0x004edc30);
}

void __stdcall glu_join_wrapper() { // 0x004de4ef -> 0x004b86c0
	glu_wrapper(0x004b86c0);
}

void __stdcall glu_create_wrapper() { // 0x004de4f8 and 0x004de504 -> 0x004af6d0
	glu_wrapper(0x004af6d0);
}

void __stdcall glu_ex_campaign_wrapper() { // 0x004de4da -> 0x004b52a0
	glu_wrapper(0x004b52a0);
}

void inject_menu_hooks() {
	return; // Disable hooks until this is ready
	fraud_launcher.connect();
	if (fraud_launcher.connected()) {
		call_patch(glu_main_wrapper, 0x004de4a4);
		call_patch(glu_connect_wrapper, 0x004de4e1);
		call_patch(glu_login_wrapper, 0x004de4b4);
		call_patch(glu_join_wrapper, 0x004de4ef);
		call_patch(glu_create_wrapper, 0x004de4f8); // Singleplayer
		call_patch(glu_create_wrapper, 0x004de504); // Multiplayer
		call_patch(glu_ex_campaign_wrapper, 0x004de4da);
	}
}
} // namespace hooks