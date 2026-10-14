#include "text_print.h"

#include "BW/bwdata.h"
#include "BW/ui_scale.h"
#include "graphics/Font.h"
#include "hooks/interface/mainmenu/glucustom.h"
#include "systems/text/print.h"

namespace hooks {

const char* __stdcall bw_print_xy_impl(const char* text, s32 x, s32 y) {
    return text::print_vanilla(text, x, y);
}

void __stdcall set_font_handle_impl(graphics::Font* font) {
    g_current_font = font;
    if (!font) {
        return;
    }
    g_current_font_color = (u32)g_tfontgame_image_data;
    g_saved_font_color = (u32)g_tfontgame_image_data;
    g_font_max_advance = font->xMax;
    g_font_tab_width = u8(font->xMax << 1);
    g_current_font_height = u8(s32(ui_scale::Factor) * font->yMax);
    g_line_spacing = 3;
    g_print_xy_alignment = 0x11;
    g_font_spacing = 1;
    g_draw_text.left = 0;
    g_draw_text.top = 0;
    g_draw_text.right = 0x7FFF;
    g_draw_text.bottom = 0x7FFF;
}

} // hooks
