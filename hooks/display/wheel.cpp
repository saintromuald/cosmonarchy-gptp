#include "wheel.h"
#include "../../BW/bwhelper.h"
#include "../../systems/display/zoom.h"

namespace hooks {

void __cdecl mouse_wheel(u32 event_type, u32 packed_position, InputProcsFunc* input_proc) {
    // Uses g_mouse (surface client coords) since packed_position is in screen space.
    // 004D2487 thresholds WHEEL_DELTA into discrete events.
    const s32 notches = event_type == DialogEventType_MWheelUp ? 1 : -1;
    if (g_zoom.adjust_by_wheel(notches, { g_mouse->x, g_mouse->y })) {
        return;
    }

    // Fallback event dispatch to dialogs and input proc (004D1900).
    DialogEvent event{};
    event.param = u16(packed_position);
    event.field_6 = u16(packed_position >> 16);
    event.number_order = DialogEventType(u16(event_type));

    if (helper::send_input_to_all_dialogs(&event) != 0) {
        return;
    }
    if (input_proc) {
        input_proc(s32(&event));
    }
}

} // hooks
