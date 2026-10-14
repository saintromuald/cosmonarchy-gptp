#pragma once
#include "types.h"

struct BinDlg;

namespace hooks {

// 0x0041F920's replacement body (text in EAX): the walk-based wrap point.
// g_current_text_x is the start x on entry and the line's end x on return.
const u8* __stdcall font_find_line_break_impl(const u8* text);

// 0x0041FC20's replacement body (text in EAX, max width on the stack):
// the codepoint-snapped fit offset.
u32 __stdcall font_fit_text_to_width_impl(const u8* text, s32 max_width);

// 0x0041FC80's replacement body (text in EAX): the widest line's width.
s32 __stdcall font_max_line_width_impl(const u8* text);

// 0x00416240's replacement body (dlg in ESI): edit-field horizontal scroll,
// walking codepoints instead of bytes so the view offset never splits one.
void __stdcall textbox_scroll_to_cursor_impl(BinDlg* dlg);

void inject_text_width_hooks();

} // hooks
