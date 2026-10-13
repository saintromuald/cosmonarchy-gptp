#pragma once
#include "types.h"

#include <span>
#include <unordered_map>
#include <vector>

namespace text {

class FontFace;

struct ShapedGlyph {
    u32 glyph_index = 0;
    s32 advance = 0;   // horizontal advance in pixels, kerning included
    s32 offset_x = 0;  // Always 0 in the FreeType shaper; reserved for HarfBuzz.
    s32 offset_y = 0;
};

// FreeTypeonly today, replace body of this function if/when we add HarfBuzz
class Shaper {
public:
    // Shapes a run with no control bytes.
    std::vector<ShapedGlyph> shape(FontFace& face, u32 pixel_height, std::span<const char32_t> codepoints);

private:
    struct Key {
        FontFace* face;
        u32 pixel_height;
        u64 hash;
        bool operator==(const Key& other) const = default;
    };
    struct KeyHash {
        size_t operator()(const Key& key) const;
    };
    struct Run {
        std::vector<char32_t> codepoints;  // stored for exact-match verification
        std::vector<ShapedGlyph> glyphs;
    };

    std::unordered_map<Key, Run, KeyHash> m_runs;
};

} // namespace text