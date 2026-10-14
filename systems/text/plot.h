#pragma once
#include "types.h"
#include "BW/structures/common.h"
#include "rasterizer.h"
#include "systems/graphics/grp.h"
#include "text.h"

namespace graphics {
struct Bitmap;
}

namespace text {

// True for the control codes that select a ramp row. Revert, Tab, Newline,
// the player-color prefixes, the aligns and the print ends are structural:
// color_slot maps them to unrelated rows (Revert wraps past the buffer).
bool is_color_code(u8 code);

u8 color_slot(u8 code);

// The coverage ramp for a color slot in the shared font ramp buffer. Color
// state throughout the print engine is the ramp pointer (vanilla's own model)
// because callers can point it anywhere - FUN_00417a10 synthesizes ramps for
// dialog color schemes, the wireframe font uses slot 40 directly.
const u8* ramp(u8 slot);

// Ramp layout: bytes 1-4 are the coverage shades (byte 1 = bright core), byte 7
// is the shadow color baked into .fnt glyphs as mask-7 pixels.
constexpr u32 ShadowRampByte = 7;

// The grey override (color_slot(0x05)): while it is current, the legacy color
// codes, Revert and the player pairs are no-ops; the extended colors bypass
// the grey check, like writeFontApplyColour's off_4 jump.
constexpr u8 GreyColorSlot = 3;

// Clip box is inclusive ltrb, intersected with the bitmap's own bounds.
void plot_pixel(graphics::Bitmap& bitmap, const Box32& clip, s32 x, s32 y, u8 value);
void plot_glyph(graphics::Bitmap& bitmap, const Box32& clip, const GlyphBitmap& glyph, s32 x, s32 y, const u8* ramp);

// The text cursor: the U+00A6 glyph at the cursor pair's position, like vanilla's .fnt 0xA6.
void plot_cursor(graphics::Bitmap& bitmap, const Box32& clip, const PrintFont& font, s32 x, s32 line_y, const u8* ramp);

// Vertically centers the frame on the line box; returns the blitted size.
// Blits clip to the clip box, like the glyph plotters.
graphics::FrameSize plot_icon(graphics::Bitmap& bitmap, const Box32& clip, u16 unit_id, s32 x, s32 line_y, s32 line_advance, u32 pixel_height, const u8* ramp);

} // namespace text
