#include "weapon_armor_tooltip.h"
#include <hook_tools.h>
#include <SCBW/scbwdata.h>

namespace
{
	const u32 Func_CreateContextHelp = 0x00481510;
	void create_context_help_from_dialog(BinDlg* dialog, const char* text)
	{
		u32 x = dialog->bounds.right, y = dialog->bounds.bottom;
		if (dialog->control_type != 0)
		{
			x += dialog->parent->bounds.left;
			y += dialog->parent->bounds.top;
		}
		__asm
		{
			PUSHAD
			PUSH text
			PUSH y
			MOV EAX, x
			CALL Func_CreateContextHelp
			POPAD
		}
	}

	// Replaces function @ 0x004576A0
	void __stdcall statdata_luser_interact_wrapper(BinDlg* consoleIcon)
	{
		GuiOverlay* gui = (GuiOverlay*)consoleIcon->user;
		switch (gui->overlay_type)
		{
			case 0:
			{
				create_context_help_from_dialog(consoleIcon, hooks::get_weapon_tooltip_string((u8)gui->id, *active_portrait_unit));
			} break;
			case 1:
			{
				create_context_help_from_dialog(consoleIcon, hooks::get_armor_tooltip_string(*active_portrait_unit));
			} break;
			case 2:
			{
				create_context_help_from_dialog(consoleIcon, hooks::get_shield_tooltip_string(*active_portrait_unit));
			} break;
			case 3:
			{
				create_context_help_from_dialog(consoleIcon, hooks::get_special_tooltip_string(gui->id, *active_portrait_unit));
			} break;
		}
	}
}

namespace hooks
{
	void inject_unit_tooltip_hooks()
	{
		call_patch(&statdata_luser_interact_wrapper, 0x00457D4B);
	}
}
