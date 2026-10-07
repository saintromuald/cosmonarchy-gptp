#pragma once
#include "../../BW/bwdata.h"

namespace hooks {

// 0041A0A0 (glu_load_bin_dialog's call to 00419D20). Widget-framework dialogs
// carry absolute l/t/r/b bounds, so they must skip vanilla's x/y/w/h conversion
void initialize_glu_dialog(bw::Dialog* dialog, bw::DialogInteraction* dialogproc);

void inject_widget_dialog_init_hooks();

} // hooks