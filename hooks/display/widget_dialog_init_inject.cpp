#include "widget_dialog_init.h"

#include "../../hook_tools.h"

namespace {

// glu_load_bin_dialog reaches initialize_dialog with the dialog in ESI and its
// proc in EAX, the convention 00419D20 shipped with. The call it replaces is
// five bytes and so is this one; the caller only relies on callee-saved
// registers past this point, which PUSHAD/POPAD preserves.
void __declspec(naked) initialize_glu_dialog_wrapper() {
    static bw::Dialog* dialog;
    static bw::DialogInteraction* dialogproc;
    __asm {
        MOV dialog, ESI
        MOV dialogproc, EAX
        PUSHAD
    }
    hooks::initialize_glu_dialog(dialog, dialogproc);
    __asm {
        POPAD
        RETN
    }
}

} // namespace

namespace hooks {

void inject_widget_dialog_init_hooks() {
    call_patch(initialize_glu_dialog_wrapper, 0x0041A0A0);
}

} // hooks