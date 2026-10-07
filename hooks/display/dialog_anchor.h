#pragma once
#include "../../BW/bwdata.h"

namespace hooks {

// 004F5920 (tail of load_bin_dlg). Anchors popups to surface center before initialize_dialog.
void initialize_popup_dialog(BinDlg* dialog, BinDlgProc dialogproc);

void inject_dialog_anchor_hooks();

} // hooks
