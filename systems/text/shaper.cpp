#include "shaper.h"
#include "rasterizer.h"

#include <algorithm>
#include <functional>

namespace text {
namespace {

// Wholesale clear on overflow: chat refills the working set quickly, and
// per-entry eviction isn't worth the bookkeeping.
constexpr size_t RunCacheLimit = 4096;

u64 run_hash(std::span<const char32_t> codepoints) {
    u64 hash = 1469598103934665603ull;  // FNV-1a 64
    for (const char32_t codepoint : codepoints) {
        hash ^= (u64)codepoint;
        hash *= 1099511628211ull;
    }
    return hash;
}

}

size_t Shaper::KeyHash::operator()(const Key& key) const {
    size_t hash = std::hash<const void*>{}(key.face);
    hash ^= std::hash<u32>{}(key.pixel_height) << 3;
    hash ^= std::hash<u64>{}(key.hash) * 2654435761u;
    return hash;
}

std::vector<ShapedGlyph> Shaper::shape(FontFace& face, u32 pixel_height, std::span<const char32_t> codepoints) {
    if (codepoints.empty()) {
        return {};
    }
    const Key key{&face, pixel_height, run_hash(codepoints)};
    if (const auto it = m_runs.find(key); it != m_runs.end()) {
        const Run& run = it->second;
        if (run.codepoints.size() == codepoints.size()
            && std::equal(run.codepoints.begin(), run.codepoints.end(), codepoints.begin())) {
            return run.glyphs;
        }
    }

    FT_Face ft = face.ft();
    if (!ft) {
        return {};
    }
    FT_Set_Pixel_Sizes(ft, 0, pixel_height);
    const bool kerns = FT_HAS_KERNING(ft) != 0;

    std::vector<ShapedGlyph> glyphs;
    glyphs.reserve(codepoints.size());
    u32 previous_index = 0;
    for (const char32_t codepoint : codepoints) {
        ShapedGlyph shaped;
        shaped.glyph_index = face.glyph_index(codepoint);
        if (FT_Load_Glyph(ft, shaped.glyph_index, FT_LOAD_DEFAULT)) {
            shaped.advance = 0;
        } else {
            shaped.advance = ft->glyph->advance.x >> 6;
        }
        if (kerns && previous_index && shaped.glyph_index) {
            FT_Vector kerning = {};
            if (!FT_Get_Kerning(ft, previous_index, shaped.glyph_index, FT_KERNING_DEFAULT, &kerning)) {
                shaped.advance += kerning.x >> 6;
            }
        }
        previous_index = shaped.glyph_index;
        glyphs.push_back(shaped);
    }

    if (m_runs.size() >= RunCacheLimit) {
        m_runs.clear();
    }
    auto [it, inserted] = m_runs.emplace(key, Run{{codepoints.begin(), codepoints.end()}, std::move(glyphs)});
    return it->second.glyphs;
}

} // namespace text