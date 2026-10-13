#pragma once
#include "types.h"
#include "BW/ui_scale.h"

#include "font_stack.h"
#include "glyph_cache.h"
#include "rasterizer.h"
#include "shaper.h"

#include <array>
#include <memory>
#include <vector>

namespace text {

enum class StackId : u8 {
    Body,
    Title,
    Count,
};

// Indexed by graphics::FontSize (0-3).
struct SlotConfig {
    StackId stack;
    u32 pixel_height;
};

constexpr std::array<SlotConfig, 4> SlotTable{{
    {StackId::Body, 10 * ui_scale::Factor},
    {StackId::Body, 14 * ui_scale::Factor},
    {StackId::Body, 16 * ui_scale::Factor},
    {StackId::Title, 16 * ui_scale::Factor},
}};

constexpr size_t GlyphCacheBudgetBytes = 16 * 1024 * 1024;

class TextSystem {
public:
    static TextSystem& instance() {
        static TextSystem instance;
        return instance;
    }

    // Loads the bundled faces from the mpq and probes the system CJK fonts
    // Only the body face is required; other misses degrade to tofu
    bool init();

    // Renderers can fall back to legacy .fnt if unavailable
    bool available() const { return m_available; }

    const SlotConfig& slot(u8 slot) const;

    // The vanilla text metrics, scaled by ui_scale::Factor and derived from
    // the live .fnt slot and font globals.
    s32 line_advance(u8 slot) const;
    s32 space_advance(u8 slot) const;
    s32 tab_advance(u8 slot) const;
    s32 char_spacing() const;

    FontStack& stack(StackId id);
    Shaper& shaper() { return m_shaper; }
    GlyphCache& glyph_cache() { return m_glyph_cache; }

private:
    FontFace* load_mpq_face(const char* path);
    FontFace* probe_system_face(const char* filename, const char* family);

    FT_Library m_library = nullptr;
    bool m_available = false;
    std::vector<std::unique_ptr<FontFace>> m_faces;
    std::array<std::unique_ptr<FontStack>, (u32)StackId::Count> m_stacks;
    Shaper m_shaper;
    GlyphCache m_glyph_cache{GlyphCacheBudgetBytes};
};

} // namespace text