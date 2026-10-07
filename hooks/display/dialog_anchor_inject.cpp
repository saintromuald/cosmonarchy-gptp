#include "dialog_anchor.h"

#include "../../hook_tools.h"

namespace {
void __declspec(naked) initialize_popup_dialog_wrapper() {
    static BinDlg* popup;
    static BinDlgProc popup_proc;
    __asm {
        MOV popup, ESI
        MOV popup_proc, EAX
        PUSHAD
    }
    hooks::initialize_popup_dialog(popup, popup_proc);
    __asm {
        POPAD
        RETN
    }
}
} // namespace

namespace hooks {

void inject_dialog_anchor_hooks() {
    call_patch(initialize_popup_dialog_wrapper, 0x004F5920);
}

} // hooks
