#include "text.h"

#include "BW/bwdata.h"
#include "BW/bwhelper.h"
#include "graphics/Font.h"
#include "logger.h"

#include <algorithm>
#include <cassert>
#include <format>
#include <string>
#include <string_view>
#include <utility>

#include <windows.h>

namespace text {
namespace {

const graphics::Font* slot_font(u8 slot) {
    return slot < SlotTable.size() ? font_base[slot] : nullptr;
}


const SlotConfig& slot_config(u8 slot) {
    assert(slot < SlotTable.size());
    return SlotTable[slot < SlotTable.size() ? slot : DefaultSlot];
}

}

TextSystem::TextSystem() {
    // Empty stacks keep stack() and the font getters dereferenceable before
    // init() runs and when it fails; init() replaces them.
    for (auto& font_stack : m_stacks) {
        font_stack = std::make_unique<FontStack>();
    }
}

bool TextSystem::init() {
    if (m_library) {
        return true;
    }
    if (FT_Init_FreeType(&m_library)) {
        m_library = nullptr;
        char error_type[] = "TextSystem: FreeType initialization failed";
        gptp::g_logger << error_type << '\n';
        helper::fatal_error(error_type);
        return false;
    }

    FontFace* exo2 = load_mpq_face("font\\Exo2-Regular.ttf");
    if (!exo2) {
        // Text renders exclusively through this pipeline: a broken body font
        // is fatal, like any other missing core asset.
        helper::fatal_file_not_found(SErrGetLastError(), "font\\Exo2-Regular.ttf");
    }
    FontFace* michroma = load_mpq_face("font\\Michroma-Regular.ttf");
    FontFace* noto_jp  = load_mpq_face("font\\NotoSansJP-Regular.ttf");
    FontFace* noto_kr  = load_mpq_face("font\\NotoSansKR-Regular.ttf");
    FontFace* noto_sc  = load_mpq_face("font\\NotoSansSC-Regular.ttf");
    FontFace* emoji    = load_mpq_face("font\\NotoEmoji-Regular.ttf");
    FontFace* symbols  = load_mpq_face("font\\NotoSansSymbols2-Regular.ttf");

    // Optional system fonts to use if available
    FontFace* meiryo = probe_system_face("meiryo.ttc", "Meiryo");
    FontFace* dotum  = probe_system_face("gulim.ttc", "Dotum");
    FontFace* yahei  = probe_system_face("msyh.ttc", "Microsoft YaHei");

    FontStack::Faces body;
    body.primary = {exo2};
    body.script_probe[(u32)Script::Japanese] = meiryo;
    body.script_probe[(u32)Script::Korean] = dotum;
    body.script_probe[(u32)Script::SimplifiedChinese] = yahei;
    body.script_fallback[(u32)Script::Japanese] = noto_jp;
    body.script_fallback[(u32)Script::Korean] = noto_kr;
    body.script_fallback[(u32)Script::SimplifiedChinese] = noto_sc;
    body.emoji = emoji;
    body.symbols = symbols;

    FontStack::Faces title = body;
    title.primary.clear();
    if (michroma) {
        title.primary.push_back(michroma);
    }
    title.primary.push_back(exo2);

    m_stacks[(u32)StackId::Body]  = std::make_unique<FontStack>(std::move(body));
    m_stacks[(u32)StackId::Title] = std::make_unique<FontStack>(std::move(title));
    m_base_fonts.clear();  // anything queried before init cached empty-stack metrics

    return true;
}

FontFace* TextSystem::load_mpq_face(const char* path) {
    sfile::LoadedFile file = sfile::try_load_file(path);
    if (!file) {
        gptp::g_logger << std::format("TextSystem: {} missing from the mpq\n", path);
        return nullptr;
    }
    auto face = std::make_unique<FontFace>(m_library, std::move(file));
    if (!face->valid()) {
        gptp::g_logger << std::format("TextSystem: {} is not a usable font\n", path);
        return nullptr;
    }
    m_faces.push_back(std::move(face));
    return m_faces.back().get();
}

FontFace* TextSystem::probe_system_face(const char* filename, const char* family) {
    char directory[MAX_PATH] = {};
    const u32 length = GetSystemWindowsDirectoryA(directory, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return nullptr;
    }
    const std::string path = std::format("{}\\Fonts\\{}", directory, filename);

    // Face 0 gives the collection size; match the rest by family name.
    FontFace first(m_library, path.c_str(), 0);
    if (!first.valid()) {
        return nullptr;
    }
    const u32 count = std::max(first.faces_in_collection(), 1u);
    for (u32 index = 0; index < count; ++index) {
        auto face = std::make_unique<FontFace>(m_library, path.c_str(), index);
        if (face->valid() && std::string_view(face->family_name()) == family) {
            m_faces.push_back(std::move(face));
            return m_faces.back().get();
        }
    }
    return nullptr;
}

// Everything the (stack, pixel_height) pair derives from its faces, memoized:
// the plot anchors (ascent, and the visible band [ascent - cap, ascent] that
// the vertical adjust centers, cap = the 'O' bitmap top) plus the shaped
// space and cursor advances. Live-global metrics are layered on by the
// font_for_* callers, so the per-print hot path is pure field assembly.
PrintFont TextSystem::base_font(const SlotConfig& config) {
    const u64 key = (u64(config.stack) << 32) | config.pixel_height;
    if (const auto it = m_base_fonts.find(key); it != m_base_fonts.end()) {
        return it->second;
    }
    PrintFont font;
    font.stack = config.stack;
    font.pixel_height = config.pixel_height;
    font.font_height = s32(config.pixel_height);
    font.cap_height = s32(config.pixel_height) / 2;

    FontStack& stack = *m_stacks[(u32)config.stack];
    if (FontFace* primary = stack.primary_face()) {
        font.ascent = primary->ascender(config.pixel_height);
        font.cap_height = font.ascent;
        const char32_t capital_o = U'O';
        const auto shaped = m_shaper.shape(*primary, config.pixel_height, {&capital_o, 1});
        if (!shaped.empty()) {
            const GlyphBitmap& glyph = m_glyph_cache.lookup(*primary, config.pixel_height, shaped.front().glyph_index);
            if (glyph.bitmap_top > 0 && glyph.bitmap_top <= font.ascent) {
                font.cap_height = glyph.bitmap_top;
            }
        }
        const char32_t space = U' ';
        if (FontFace* face = stack.resolve(space)) {
            const auto space_shaped = m_shaper.shape(*face, config.pixel_height, {&space, 1});
            if (!space_shaped.empty()) {
                font.space_advance = space_shaped.front().advance;
            }
        }
        const char32_t marker = 0xA6;
        if (FontFace* face = stack.resolve(marker)) {
            const auto marker_shaped = m_shaper.shape(*face, config.pixel_height, {&marker, 1});
            if (!marker_shaped.empty()) {
                font.cursor_advance = marker_shaped.front().advance;
            }
        }
    }
    m_base_fonts.emplace(key, font);
    return font;
}

PrintFont TextSystem::font_for_slot(u8 slot) {
    PrintFont font = base_font(slot_config(slot));
    font.line_advance = line_advance(slot);
    font.line_gap = s32(ui_scale::Factor) * g_line_spacing;
    font.space_advance = space_advance(slot);
    font.tab_width = tab_advance(slot);
    font.char_spacing = char_spacing();
    return font;
}

PrintFont TextSystem::font_for_px(u32 pixel_height) {
    if (!pixel_height) {
        return {};
    }
    // px fonts use vanilla's default gap (3) and spacing (1), not the live
    // globals that vanilla callers toggle per draw.
    const s32 gap = s32(ui_scale::Factor) * 3;

    PrintFont font = base_font({StackId::Body, pixel_height});
    // The .fnt recipe's analog at any size: advance = em + scaled gap.
    font.line_advance = s32(pixel_height) + gap;
    font.line_gap = gap;
    font.char_spacing = s32(ui_scale::Factor);
    if (!font.space_advance) {
        font.space_advance = s32(pixel_height) / 2;
    }
    font.tab_width = font.space_advance * 4;
    return font;
}

s32 TextSystem::line_advance(u8 slot) const {
    if (const graphics::Font* font = slot_font(slot)) {
        return s32(ui_scale::Factor) * (font->yMax + g_line_spacing);
    }
    return s32(slot_config(slot).pixel_height);
}

s32 TextSystem::space_advance(u8 slot) const {
    if (const graphics::Font* font = slot_font(slot)) {
        return s32(ui_scale::Factor) * (font->xMax >> 1);
    }
    return s32(slot_config(slot).pixel_height) / 2;
}

s32 TextSystem::tab_advance(u8 slot) const {
    if (const graphics::Font* font = slot_font(slot)) {
        return s32(ui_scale::Factor) * (font->xMax * 2);
    }
    return s32(slot_config(slot).pixel_height);
}

s32 TextSystem::char_spacing() const {
    return s32(ui_scale::Factor) * g_font_spacing;
}

FontStack& TextSystem::stack(StackId id) {
    return *m_stacks[(u32)id];
}

} // namespace text