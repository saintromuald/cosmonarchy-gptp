#pragma once
#include "../../BW/bwdata.h"

namespace hooks {

void scale_and_initialize_popup_dialog(bw::Dialog* dialog, bw::DialogInteraction* dialogproc);

void __fastcall dialog_center_on_screen(bw::Dialog* dialog); // 004CD9C0

void inject_popup_dialog_hooks();

} // hooks
