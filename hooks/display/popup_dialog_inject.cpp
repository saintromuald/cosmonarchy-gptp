#include "popup_dialog.h"

#include "../../hook_tools.h"

namespace {

void __declspec(naked) scale_and_initialize_popup_dialog_wrapper() {
    static bw::Dialog* dialog;
    static bw::DialogInteraction* dialogproc;
    __asm {
        MOV dialog, ESI
        MOV dialogproc, EAX
        PUSHAD
    }
    hooks::scale_and_initialize_popup_dialog(dialog, dialogproc);
    __asm {
        POPAD
        RETN
    }
}

} // namespace

namespace hooks {

void inject_popup_dialog_hooks() {
    call_patch(scale_and_initialize_popup_dialog_wrapper, 0x004F5920);
    jump_patch(dialog_center_on_screen, 0x004CD9C0, 3);
}

} // hooks
