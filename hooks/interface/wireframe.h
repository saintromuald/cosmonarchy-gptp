#pragma once
#include "../../BW/structures/Unit.h"

struct BinDlg;

namespace hooks {
	u16 stat_data_unit_wireframe_update_fragment(Unit* unit);
	u8 wireframe_shields(Unit* unit);
	void __fastcall remap_grpwire_button(u16 graphic, BinDlg* dialog);
	
	void inject_wireframe_hooks();
} //hooks