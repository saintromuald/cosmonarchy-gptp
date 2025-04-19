#include "weapon_armor_tooltip.h"
#include <hook_tools.h>
#include <BW/bwdata.h>

namespace {
	// Replaces function @ 0x004576A0
	void __stdcall statdata_luser_interact_wrapper(BinDlg* dialog) {
		bw::statdata_user_interact_internal(dialog);
	}
}

namespace hooks {
	void inject_unit_tooltip_hooks() {
		call_patch(&statdata_luser_interact_wrapper, 0x00457D4B);
	}
}