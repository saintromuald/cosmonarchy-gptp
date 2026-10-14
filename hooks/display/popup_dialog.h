#pragma once
#include "../../BW/bwdata.h"

namespace hooks {

void scale_and_initialize_popup_dialog(bw::Dialog* dialog, bw::DialogInteraction* dialogproc);

void inject_popup_dialog_hooks();

} // hooks
