#include "text.h"

#include "BW/bwdata.h"
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
    return slot < 4 ? font_base[slot] : nullptr;
}

}

bool TextSystem::init() {
    if (m_library) {
        return m_available;
    }
    if (FT_Init_FreeType(&m_library)) {
        m_library = nullptr;
        gptp::g_logger << "TextSystem: FreeType initialization failed; text stays legacy\n";
        return false;
    }

    FontFace* exo2 = load_mpq_face("font\\Exo2-Regular.ttf");
    if (!exo2) {
        // Everything would render tofu without the body face.
        return false;
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

    m_available = true;
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

const SlotConfig& TextSystem::slot(u8 slot) const {
    assert(slot < SlotTable.size());
    return SlotTable[slot];
}

s32 TextSystem::line_advance(u8 slot) const {
    if (const graphics::Font* font = slot_font(slot)) {
        return s32(ui_scale::Factor) * (font->yMax + g_line_spacing);
    }
    return s32(SlotTable[slot].pixel_height);
}

s32 TextSystem::space_advance(u8 slot) const {
    if (const graphics::Font* font = slot_font(slot)) {
        return s32(ui_scale::Factor) * (font->xMax >> 1);
    }
    return s32(SlotTable[slot].pixel_height) / 2;
}

s32 TextSystem::tab_advance(u8 slot) const {
    if (const graphics::Font* font = slot_font(slot)) {
        return s32(ui_scale::Factor) * (font->xMax * 2);
    }
    return s32(SlotTable[slot].pixel_height);
}

s32 TextSystem::char_spacing() const {
    return s32(ui_scale::Factor) * g_font_spacing;
}

FontStack& TextSystem::stack(StackId id) {
    return *m_stacks[(u32)id];
}

} // namespace text