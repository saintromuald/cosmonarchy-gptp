#pragma once
#include "types.h"

#include <array>
#include <optional>
#include <vector>

namespace text {

class FontFace;

enum class Script : u8 {
    Japanese,
    Korean,
    SimplifiedChinese,
    Count,
};

std::optional<Script> classify_script(char32_t codepoint);

bool is_shared_cjk(char32_t codepoint);

// Font resolution chain, CJK -> per-script -> fallback -> emoji -> symbols -> tofu
class FontStack {
public:
    struct Faces {
        std::vector<FontFace*> primary;
        FontFace* script_probe[(u32)Script::Count] = {};
        FontFace* script_fallback[(u32)Script::Count] = {};
        FontFace* emoji = nullptr;
        FontFace* symbols = nullptr;
    };

    explicit FontStack(Faces faces);
    FontStack() = default;

    FontFace* resolve(char32_t codepoint, FontFace* prefer = nullptr);

    // Per-script face report for diagnostics: which family the script's text
    // renders from, and whether the system probe or the bundled face
    // provided it. {false, false, ""} when the script renders tofu.
    struct ScriptFace {
        bool available = false;
        bool system = false;
        const char* family = "";
    };
    std::array<ScriptFace, (u32)Script::Count> script_faces() const;

    FontFace* primary_face() const { return m_faces.primary.empty() ? nullptr : m_faces.primary.front(); }

private:
    FontFace* resolve_uncached(char32_t codepoint);

    Faces m_faces;
    // Unicode Basic Multilingual Plane contains the majority of all common characters. Don't bother caching non-bmp.
    std::array<FontFace*, 0x10000> m_bmp_cache = {};
};

} // namespace text