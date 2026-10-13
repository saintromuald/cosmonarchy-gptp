#pragma once
#include "types.h"
#include "rasterizer.h"

#include <cstddef>
#include <list>
#include <unordered_map>

namespace text {

// Byte-budgeted LRU of rasterized glyphs keyed (face, size, glyph).
class GlyphCache {
public:
    explicit GlyphCache(size_t budget_bytes) : m_budget_bytes(budget_bytes) {}

    // Rasterizes on a miss. The reference is valid until the next lookup.
    const GlyphBitmap& lookup(FontFace& face, u32 pixel_height, u32 glyph_index);

    size_t bytes_used() const { return m_bytes_used; }

private:
    struct Key {
        FontFace* face;
        u32 pixel_height;
        u32 glyph_index;
        bool operator==(const Key& other) const = default;
    };
    struct KeyHash {
        size_t operator()(const Key& key) const;
    };
    struct Entry {
        GlyphBitmap glyph;
        std::list<Key>::iterator lru;
    };

    size_t entry_cost(const GlyphBitmap& glyph) const;
    void evict_over_budget();

    size_t m_budget_bytes;
    size_t m_bytes_used = 0;
    std::unordered_map<Key, Entry, KeyHash> m_entries;
    std::list<Key> m_lru;  // front = most recently used
};

} // namespace text