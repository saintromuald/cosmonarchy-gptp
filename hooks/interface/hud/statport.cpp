#include "statport.h"

#include "BW/bwdata.h"
#include "BW/bwhelper.h"
#include "BW/structures.h"
#include "BW/ui.h"

namespace hooks {

void load_statport() {
    // statport_dlg_create sizes the portrait decode buffer (g_portrait_buffer) from
    // the portrait child's bounds, so 2x portrait smks crash when the tree is
    // created at 1x — the frame decode overruns the destination and smackw32 faults
    // mid-blit. The portdata.tbl half of vanilla's loader must stay vanilla because
    // DatExtend memory-patches its fixup loops, so only the dialog is rebuilt.
    helper::load_statport();

    if (g_statport_dialog) {
        helper::destroy_dialog(g_statport_dialog); // Destroy frees the 1x portrait + conover buffers
        g_statport_dialog = nullptr;
    }

    bw::Dialog* dialog = load_bin_file_scaled("rez\\statport.bin").release();
    g_statport_dialog = dialog->to_bindlg();
    initialize_bin_dialog(dialog, helper::statport_dlgproc);
}

} // hooks
