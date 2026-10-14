#pragma once

namespace graphics {
struct Font;
}

namespace hooks {

// 0x004202B0's replacement body (text in EAX, x in ESI, y on the stack).
const char* __stdcall bw_print_xy_impl(const char* text, s32 x, s32 y);

// 0x0041FB30's replacement body (font in ECX): the vanilla state reset with
// the mod's expanded ramp buffer and the height scaled by ui_scale::Factor.
void __stdcall set_font_handle_impl(graphics::Font* font);

void inject_text_print_hooks();

} // hooks
