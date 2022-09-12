#pragma once
#include <SCBW/api.h>

namespace hooks
{
	void update_buttonset_ex();							//	00458BC0
	void highlight_button();							//	00458D50 sub_458D50
	void update_buttonset();							//	00458DE0
	void update_command_card();							//	004591D0
	void update_current_buttonset();					//	004599A0
	void statbtn_bin_custom_ctrl_id(BinDlg* dialog);	//	00459AD0
	ButtonSet* get_buttonset(s32 index);

	void inject_buttonset_hooks();
}