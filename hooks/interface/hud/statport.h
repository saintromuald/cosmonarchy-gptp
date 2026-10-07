#pragma once

namespace hooks {

// Loads the statport dialog scaled to the presentation size. Vanilla's loader runs
// first (the portdata.tbl fixup must stay vanilla — DatExtend patches its loops),
// then the dialog is rebuilt from rez\statport.bin at 2x so statport_dlg_create
// sizes the portrait decode buffer from the scaled child bounds.
void load_statport();

} // hooks
