#include "rasterizer.h"

#include <algorithm>
#include <span>
#include <utility>

namespace text {

FontFace::FontFace(FT_Library library, sfile::LoadedFile&& file, u32 face_index)
    : m_file(std::move(file)) {
    if (!m_file) {
        return;
    }
    FT_New_Memory_Face(library, (const FT_Byte*)m_file.data.get(), (FT_Long)m_file.size, (FT_Long)face_index, &m_face);
}

FontFace::FontFace(FT_Library library, const char* filepath, u32 face_index) {
    FT_New_Face(library, filepath, (FT_Long)face_index, &m_face);
}

FontFace::~FontFace() {
    if (m_face) {
        FT_Done_Face(m_face);
    }
}

u32 FontFace::faces_in_collection() const {
    return m_face ? (u32)m_face->num_faces : 0;
}

u32 FontFace::glyph_index(char32_t codepoint) const {
    return m_face ? FT_Get_Char_Index(m_face, (FT_ULong)codepoint) : 0;
}

const char* FontFace::family_name() const {
    return m_face && m_face->family_name ? m_face->family_name : "";
}

s32 FontFace::ascender(u32 pixel_height) {
    if (!m_face) {
        return 0;
    }
    if (FT_Set_Pixel_Sizes(m_face, 0, pixel_height)) {
        return 0; // stale metrics would silently misplace baselines
    }
    return (s32)(m_face->size->metrics.ascender >> 6);
}

GlyphBitmap rasterize_glyph(FontFace& face, u32 pixel_height, u32 glyph_index) {
    GlyphBitmap bitmap;
    FT_Face ft = face.ft();
    if (!ft) {
        return bitmap;
    }
    if (FT_Set_Pixel_Sizes(ft, 0, pixel_height)) {
        return bitmap; // stale metrics would silently misplace glyphs
    }
    if (FT_Load_Glyph(ft, (FT_UInt)glyph_index, FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT)) {
        return bitmap;
    }
    const FT_Bitmap& source = ft->glyph->bitmap;
    if (source.pixel_mode != FT_PIXEL_MODE_GRAY || !source.buffer) {
        return bitmap;
    }
    bitmap.width = source.width;
    bitmap.height = source.rows;
    bitmap.bitmap_left = ft->glyph->bitmap_left;
    bitmap.bitmap_top = ft->glyph->bitmap_top;
    bitmap.pixels.resize(size_t(source.width) * source.rows);
    // The live ramps (glue\dynamic\tfont.pcx + game\tfontgam.pcx) carry four
    // coverage shades at bytes 1-4 - byte 1 is the bright core, byte 4 the
    // faintest fade - while bytes 5-6 are unused and byte 7 is the shadow
    // color. So quantize into masks 1-4 only, heaviest coverage on the core
    // byte, and keep 0 for transparent pixels.
    const auto quantize = [](u8 coverage) {
        return coverage ? u8(4 - (coverage >> 6)) : 0;
    };
    for (u32 y = 0; y < source.rows; ++y) {
        const auto row = std::span{source.buffer + y * source.pitch, source.width};
        const auto destination = std::span{bitmap.pixels}.subspan(y * source.width, source.width);
        std::ranges::transform(row, destination.begin(), quantize);
    }
    return bitmap;
}

} // namespace text