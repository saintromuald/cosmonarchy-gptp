#include "glyph_cache.h"

namespace text {
namespace {

// Map node + LRU node + bitmap metadata.
constexpr size_t EntryOverheadBytes = 128;

}

size_t GlyphCache::KeyHash::operator()(const Key& key) const {
    size_t hash = std::hash<const void*>{}(key.face);
    hash ^= std::hash<u32>{}(key.pixel_height) * 31;
    hash ^= std::hash<u32>{}(key.glyph_index) * 2654435761u;
    return hash;
}

size_t GlyphCache::entry_cost(const GlyphBitmap& glyph) const {
    return glyph.pixels.size() + EntryOverheadBytes;
}

void GlyphCache::evict_over_budget() {
    while (m_bytes_used > m_budget_bytes && m_lru.size() > 1) {
        const Key key = m_lru.back();
        const auto it = m_entries.find(key);
        m_bytes_used -= entry_cost(it->second.glyph);
        m_entries.erase(it);
        m_lru.pop_back();
    }
}

const GlyphBitmap& GlyphCache::lookup(FontFace& face, u32 pixel_height, u32 glyph_index) {
    const Key key{&face, pixel_height, glyph_index};
    if (auto it = m_entries.find(key); it != m_entries.end()) {
        m_lru.splice(m_lru.begin(), m_lru, it->second.lru);
        return it->second.glyph;
    }
    GlyphBitmap glyph = rasterize_glyph(face, pixel_height, glyph_index);
    const size_t cost = entry_cost(glyph);
    auto [it, inserted] = m_entries.emplace(key, Entry{std::move(glyph), {}});
    m_lru.push_front(key);
    it->second.lru = m_lru.begin();
    m_bytes_used += cost;
    evict_over_budget();
    return it->second.glyph;
}

} // namespace text