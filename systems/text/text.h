#pragma once
#include "types.h"
#include "BW/ui_scale.h"

#include "font_stack.h"
#include "glyph_cache.h"
#include "rasterizer.h"
#include "shaper.h"

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

namespace text {

enum class StackId : u8 {
    Body,
    Title,
    Count,
};

// A print's resolved font: the pixel height plus every metric the walk/print
// core needs, so callers resolve once instead of re-querying per metric.
struct PrintFont {
    StackId stack = StackId::Body;
    u32 pixel_height = 0;
    s32 line_advance = 0;
    s32 font_height = 0;   // the em: the gate/fit height (descender room)
    s32 line_gap = 0;
    s32 ascent = 0;        // the baseline anchor the renderer plots with
    s32 cap_height = 0;     // 'O' bitmap_top; the vertical adjust centers [ascent - cap, ascent]
    s32 space_advance = 0;
    s32 tab_width = 0;
    s32 char_spacing = 0;
    s32 cursor_advance = 0;   // the 0xA6 marker's shaped advance, without char spacing
};

// Reusable scratch buffers
struct WalkScratch {
    std::vector<char32_t> run_codepoints;
    std::vector<const u8*> run_starts;
    std::vector<ShapedGlyph> shaped;
};

// Indexed by graphics::FontSize (0-3).
struct SlotConfig {
    StackId stack;
    u32 pixel_height;
};

constexpr std::array<SlotConfig, 4> SlotTable{{
    {StackId::Body,  16},
    {StackId::Body,  22},
    {StackId::Body,  26},
    {StackId::Title, 26},
}};

constexpr u8 DefaultSlot = 1;

constexpr size_t GlyphCacheBudgetBytes = 16 * 1024 * 1024;

class TextSystem {
public:
    static TextSystem& instance() {
        static TextSystem instance;
        return instance;
    }

    bool init();

    // The vanilla slot metrics (the .fnt recipe, scaled) or FreeType-derived
    // metrics for an arbitrary pixel height on the body stack.
    PrintFont font_for_slot(u8 slot);
    PrintFont font_for_px(u32 pixel_height);

    // The vanilla text metrics, scaled by ui_scale::Factor and derived from
    // the live .fnt slot and font globals.
    s32 line_advance(u8 slot) const;
    s32 space_advance(u8 slot) const;
    s32 tab_advance(u8 slot) const;
    s32 char_spacing() const;

    FontStack& stack(StackId id);
    Shaper& shaper() { return m_shaper; }
    GlyphCache& glyph_cache() { return m_glyph_cache; }
    WalkScratch& walk_scratch() { return m_walk_scratch; }

private:
    TextSystem();

    FontFace* load_mpq_face(const char* path);
    FontFace* probe_system_face(const char* filename, const char* family);
    PrintFont base_font(const SlotConfig& config);

    FT_Library m_library = nullptr;
    std::vector<std::unique_ptr<FontFace>> m_faces;
    std::array<std::unique_ptr<FontStack>, (u32)StackId::Count> m_stacks;
    Shaper m_shaper;
    GlyphCache m_glyph_cache{GlyphCacheBudgetBytes};
    WalkScratch m_walk_scratch;
    std::unordered_map<u64, PrintFont> m_base_fonts;  // memoized (stack, pixel_height) face metrics
};

} // namespace text