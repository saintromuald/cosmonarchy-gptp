#pragma once
#include "types.h"
#include "BW/structures/common.h"
#include "BW/enumerations.h"
#include "graphics/Shape.h"

#include <optional>
#include <string_view>

namespace graphics {
struct Bitmap;
}

namespace text {

// Horizontal placement. Every combination with any VAlign is valid.
enum class HAlign : u8 {
    Left,
    Center,
    Right,
    Adaptive,   // Center when the text fits one line, Left when it wraps
};

// Vertical placement inside the clip box.
enum class VAlign : u8 {
    None,       // draw at y (default)
    Center,     // vertically centered in the clip box
    Bottom,     // bottom-aligned in the clip box
};

struct PrintResult {
    std::string_view remaining;   // unprinted text; empty when everything printed
    u16 line_count = 0;
    Box32 bounds = {};            // inclusive ltrb ink box of what was drawn; w/h alias right/bottom
    s32 final_y = 0;
};

class PrintOptions {
public:
    PrintOptions& on(graphics::Bitmap& surface) {
        m_target = &surface;
        return *this;
    }

    PrintOptions& clip(Box32 box) {
        m_clip = box;
        return *this;
    }

    // The vanilla font slots (their metrics match the dialogs' hardcoded layout).
    PrintOptions& size(graphics::FontSize slot) {
        m_slot = slot;
        m_px = 0;
        return *this;
    }

    // An arbitrary pixel height on the body stack (mod-authored layout).
    PrintOptions& px(u32 height) {
        m_px = height;
        return *this;
    }

    PrintOptions& halign(HAlign alignment) {
        m_halign = alignment;
        return *this;
    }

    PrintOptions& valign(VAlign alignment) {
        m_valign = alignment;
        return *this;
    }

    PrintOptions& wrap(bool wrapped = true) {
        m_word_wrap = wrapped;
        return *this;
    }

    // The color code is palette-relative: TextColor for in-game drawing,
    // LobbyTextColor for menu drawing (the ramp rows hold different colors).
    PrintOptions& color(TextColor text_color) {
        m_color = text_color;
        return *this;
    }

    PrintOptions& color(LobbyTextColor lobby_color) {
        return color(static_cast<TextColor>(lobby_color));
    }

private:
    friend PrintResult print(std::string_view, s32, s32, const PrintOptions&);

    graphics::Bitmap* m_target = nullptr;   // null: the active compositor draw target
    std::optional<Box32> m_clip;            // nullopt: the target's full bounds
    graphics::FontSize m_slot = graphics::FONT_MEDIUM;
    u32 m_px = 0;                           // nonzero: overrides m_slot
    HAlign m_halign = HAlign::Left;
    VAlign m_valign = VAlign::None;
    bool m_word_wrap = false;
    std::optional<TextColor> m_color;      // nullopt: inherit the ambient print color state
};

// The simple call: active draw target, whole-target clip, medium slot, left, and
// whatever color state the ambient globals hold (set by get_color_handle).
PrintResult print(std::string_view text, s32 x, s32 y);
// Everything else, fluent:
PrintResult print(std::string_view text, s32 x, s32 y, const PrintOptions& options);

// The vanilla bw_print_xy boundary (0x004202B0): reads the print globals into
// engine params and writes the results back. Workstep 5's hook calls this.
const char* print_vanilla(const char* text, s32 x, s32 y);

} // namespace text