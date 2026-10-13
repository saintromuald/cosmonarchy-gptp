#pragma once
#include "types.h"
#include "storm/sfile.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>

namespace text {

// One FreeType face. Memory faces read from the owned buffer for the face's lifetime, system faces stream from disk.
class FontFace {
public:
    FontFace(FT_Library library, sfile::LoadedFile&& file, u32 face_index = 0);
    FontFace(FT_Library library, const char* filepath, u32 face_index = 0);
    ~FontFace();
    FontFace(const FontFace&) = delete;
    FontFace& operator=(const FontFace&) = delete;

    bool valid() const { return m_face != nullptr; }
    FT_Face ft() const { return m_face; }

    u32 faces_in_collection() const;
    u32 glyph_index(char32_t codepoint) const;
    bool has_codepoint(char32_t codepoint) const { return glyph_index(codepoint) != 0; }
    const char* family_name() const;

    s32 ascender(u32 pixel_height);

private:
    sfile::LoadedFile m_file;
    FT_Face m_face = nullptr;
};

struct GlyphBitmap {
    u32 width = 0;
    u32 height = 0;
    s32 bitmap_left = 0;     // pen-relative x of the bitmap's left edge
    s32 bitmap_top = 0;      // distance up from the baseline to the bitmap's top edge
    std::vector<u8> pixels;  // width * height; 0 = transparent, 1-4 = coverage masks (byte 1 = core)
};

// Light grayscale rasterization.
GlyphBitmap rasterize_glyph(FontFace& face, u32 pixel_height, u32 glyph_index);

} // namespace text