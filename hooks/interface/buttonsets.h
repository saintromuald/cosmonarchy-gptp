#pragma once
#include <SCBW/api.h>

namespace hooks
{
	void update_buttonset_ex();							//	00458BC0  update_buttonset_ex
	void update_buttonset_sub458D50();					//	00458D50  sub_458D50
	void update_buttonset();							//	00458DE0  update_buttonset
	void update_buttonset_sub4591D0();					//	004591D0  sub_4591D0
	void update_current_buttonset();					//	004599A0  update_current_buttonset
	void statbtn_bin_custom_ctrl_id(BinDlg* dialog);	//	00459AD0 statbtn_bin_custom_ctrl_id
	ButtonSet* get_buttonset(s32 index);

	void inject_buttonset_hooks();
}