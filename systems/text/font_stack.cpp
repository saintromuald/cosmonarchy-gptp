#include "font_stack.h"
#include "rasterizer.h"

namespace text {
namespace {

FontFace* pick(FontFace* face, char32_t codepoint) {
    return face && face->has_codepoint(codepoint) ? face : nullptr;
}

}

std::optional<Script> classify_script(char32_t codepoint) {
    if ((codepoint >= 0x1100 && codepoint <= 0x11FF)       // Hangul Jamo
        || (codepoint >= 0x3130 && codepoint <= 0x318F)    // Hangul Compatibility Jamo
        || (codepoint >= 0xAC00 && codepoint <= 0xD7A3)    // Hangul Syllables
        || (codepoint >= 0xFFA0 && codepoint <= 0xFFDC)) { // Halfwidth Hangul Jamo
        return Script::Korean;
    }
    if ((codepoint >= 0x3040 && codepoint <= 0x309F)       // Hiragana
        || (codepoint >= 0x30A0 && codepoint <= 0x30FF)    // Katakana
        || (codepoint >= 0xFF61 && codepoint <= 0xFF9F)) { // Halfwidth Katakana
        return Script::Japanese;
    }
    return std::nullopt;
}

bool is_shared_cjk(char32_t codepoint) {
    return (codepoint >= 0x2E80 && codepoint <= 0x2FDF)       // CJK Radicals, Kangxi
        || (codepoint >= 0x3000 && codepoint <= 0x303F)       // CJK Symbols and Punctuation
        || (codepoint >= 0x31C0 && codepoint <= 0x31EF)       // CJK Strokes
        || (codepoint >= 0x3400 && codepoint <= 0x4DBF)       // Ideographs Extension A
        || (codepoint >= 0x4E00 && codepoint <= 0x9FFF)      // CJK Unified Ideographs
        || (codepoint >= 0xF900 && codepoint <= 0xFAFF)      // CJK Compatibility Ideographs
        || (codepoint >= 0xFF00 && codepoint <= 0xFF60)      // Fullwidth Forms
        || (codepoint >= 0x20000 && codepoint <= 0x3FFFF);   // Ideograph extensions, planes 2-3
}

FontStack::FontStack(Faces faces) : m_faces(std::move(faces)) {}

std::array<FontStack::ScriptFace, (u32)Script::Count> FontStack::script_faces() const {
    std::array<ScriptFace, (u32)Script::Count> faces{};
    for (u32 script = 0; script < (u32)Script::Count; ++script) {
        const FontFace* probe = m_faces.script_probe[script];
        const FontFace* fallback = m_faces.script_fallback[script];
        if (probe && probe->valid()) {
            faces[script] = {true, true, probe->family_name()};
        } else if (fallback && fallback->valid()) {
            faces[script] = {true, false, fallback->family_name()};
        }
    }
    return faces;
}

FontFace* FontStack::resolve(char32_t codepoint, FontFace* prefer) {
    // A shared CJK codepoint keeps the surrounding run's face when it covers it.
    if (prefer && is_shared_cjk(codepoint) && prefer->has_codepoint(codepoint)) {
        return prefer;
    }
    // Non-BMP traffic (emoji, extended ideographs) is rare enough that the
    // uncached walk beats carrying a second tier of cache machinery.
    if (codepoint >= 0x10000) {
        return resolve_uncached(codepoint);
    }
    if (FontFace* face = m_bmp_cache[codepoint]) {
        return face;
    }
    FontFace* face = resolve_uncached(codepoint);
    if (face) {
        m_bmp_cache[codepoint] = face;
    }
    return face;
}

FontFace* FontStack::resolve_uncached(char32_t codepoint) {
    const std::optional<Script> script = is_shared_cjk(codepoint)
        ? std::optional<Script>{Script::SimplifiedChinese}
        : classify_script(codepoint);
    if (script) {
        const u32 index = (u32)*script;
        if (FontFace* face = pick(m_faces.script_probe[index], codepoint)) {
            return face;
        }
        if (FontFace* face = pick(m_faces.script_fallback[index], codepoint)) {
            return face;
        }
    } else {
        for (FontFace* face : m_faces.primary) {
            if (face->has_codepoint(codepoint)) {
                return face;
            }
        }
    }
    if (FontFace* face = pick(m_faces.emoji, codepoint)) {
        return face;
    }
    if (FontFace* face = pick(m_faces.symbols, codepoint)) {
        return face;
    }
    // Misses render .notdef tofu from the first primary face.
    return m_faces.primary.empty() ? m_faces.emoji : m_faces.primary.front();
}

} // namespace text