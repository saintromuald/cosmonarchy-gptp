#include "weapon_armor_tooltip.h"
#include <hook_tools.h>
#include <BW/bwdata.h>

namespace {
	// Replaces function @ 0x004576A0
	void __stdcall statdata_luser_interact_wrapper(BinDlg* consoleIcon) {
		GuiOverlay* gui = (GuiOverlay*)consoleIcon->user;
		switch (gui->overlay_type) {
			case 0: {
				helper::create_context_help_from_dialog(consoleIcon, hooks::get_weapon_tooltip_string((WeaponId::Enum)(gui->id & 255), *g_active_portrait_unit));
			} break;
			case 1: {
				helper::create_context_help_from_dialog(consoleIcon, hooks::get_armor_tooltip_string(*g_active_portrait_unit));
			} break;
			case 2: {
				helper::create_context_help_from_dialog(consoleIcon, hooks::get_shield_tooltip_string(*g_active_portrait_unit));
			} break;
			case 3: {
				helper::create_context_help_from_dialog(consoleIcon, hooks::get_special_tooltip_string(gui->id, *g_active_portrait_unit));
			} break;
		}
	}
}

namespace hooks {
	void inject_unit_tooltip_hooks() {
		call_patch(&statdata_luser_interact_wrapper, 0x00457D4B);
	}
}