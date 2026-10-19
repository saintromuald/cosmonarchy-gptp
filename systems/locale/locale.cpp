#include "locale.h"

#include "BW/api.h"
#include "config.h"
#include "storm/sfile.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <format>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <rfl/yaml.hpp>

namespace {
bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_alphanumeric(char c) {
    return is_alpha(c) || (c >= '0' && c <= '9');
}

// Subtags are 1-8 alphanumeric characters; the language subtag is 2-8 alphabetic.
bool is_valid_subtag(std::string_view subtag, size_t position) {
    if (subtag.empty() || subtag.size() > 8) {
        return false;
    }
    if (position == 0) {
        return subtag.size() >= 2 && std::ranges::all_of(subtag, is_alpha);
    }
    return std::ranges::all_of(subtag, is_alphanumeric);
}

// BCP47 casing conventions: language lowercase, script Titlecase, region UPPERCASE.
std::string canonicalize_subtag(std::string_view subtag, size_t position) {
    std::string result{subtag};
    std::ranges::transform(result, result.begin(), [](unsigned char c) { return std::tolower(c); });
    if (position > 0 && subtag.size() == 4 && std::ranges::all_of(subtag, is_alpha)) {
        result[0] = std::toupper((unsigned char)result[0]);
    } else if (position > 0 && subtag.size() == 2 && std::ranges::all_of(subtag, is_alpha)) {
        std::ranges::transform(result, result.begin(), [](unsigned char c) { return std::toupper(c); });
    }
    return result;
}

// Collapsed <0> entries start with a NUL, making them empty strings.
bool is_untranslated(const std::string& line) {
    return line.empty() || line[0] == '\0';
}

void collapse_control_codes(std::string& line) {
    for (size_t i = 0; i < line.size(); i++) {
        switch (line[i]) {
            case '<': {
                for (size_t j = i + 1; j < line.size(); j++) {
                    if (line[j] == '>') {
                        u32 control_char = 0;
                        const auto first = line.c_str() + i + 1;
                        const auto last = line.c_str() + j;
                        const auto result = std::from_chars(first, last, control_char, 10);
                        if (result.ec == std::errc{} && result.ptr == last) {
                            line[i] = char(control_char);
                            line.erase(i + 1, j - i);
                        }
                        break;
                    }
                }
            } break;
            case '\\': {
                // Safe to check i+1, since we have the ending null char as buffer
                switch (line[i+1]) {
                    case 'n': {
                        line[i] = '\n';
                        line.erase(i + 1, 1);
                    } break;
                    case '\\': {
                        line.erase(i + 1, 1);
                    } break;
                }
            } break;
        }
    }
}
} // namespace

namespace locale {

std::optional<LanguageTag> LanguageTag::parse(std::string_view tag) {
    LanguageTag result;
    size_t start = 0;
    while (true) {
        const size_t end = std::min(tag.find('-', start), tag.size());
        const auto subtag = tag.substr(start, end - start);
        if (!is_valid_subtag(subtag, result.subtags.size())) {
            return std::nullopt;
        }
        result.subtags.push_back(canonicalize_subtag(subtag, result.subtags.size()));
        if (end == tag.size()) {
            break;
        }
        start = end + 1;
    }
    return result;
}

std::string LanguageTag::to_string() const {
    std::string result;
    for (const auto& subtag : subtags) {
        if (!result.empty()) {
            result += '-';
        }
        result += subtag;
    }
    return result;
}

std::vector<std::string> LanguageTag::lookup_chain() const {
    std::vector<std::string> chain;
    std::string candidate;
    for (const auto& subtag : subtags) {
        if (!candidate.empty()) {
            candidate += '-';
        }
        candidate += subtag;
        chain.push_back(candidate);
    }
    return chain;
}

LanguageList load_languages() {
    LanguageList fallback;
    fallback.languages.push_back({.tag = "en-US", .name = "English"});

    const auto file = sfile::try_load_file(LanguagesYamlPath);
    if (!file) {
        bw::println("Localization: {} not found, language options limited to English", LanguagesYamlPath);
        return fallback;
    }

    auto parsed = rfl::yaml::read<LanguageList>(std::string{file.data.get(), file.size});
    if (!parsed) {
        bw::println("Localization: failed to parse {} ({}), language options limited to English",
            LanguagesYamlPath, parsed.error().what());
        return fallback;
    }

    LanguageList list = std::move(parsed.value());
    for (auto it = list.languages.begin(); it != list.languages.end();) {
        auto tag = LanguageTag::parse(it->tag);
        if (!tag) {
            bw::println("Localization: skipping language '{}' with malformed tag '{}'", it->name, it->tag);
            it = list.languages.erase(it);
            continue;
        }
        it->tag = tag->to_string();
        ++it;
    }
    if (list.languages.empty()) {
        bw::println("Localization: {} has no valid entries, language options limited to English", LanguagesYamlPath);
        return fallback;
    }
    return list;
}

std::optional<std::vector<std::string>> read_stat_file(std::string_view path) {
    const std::string path_str{path};
    const auto file = sfile::try_load_file(path_str.c_str());
    if (!file) {
        return std::nullopt;
    }

    std::istringstream stream{std::string{file.data.get(), file.size}};
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        collapse_control_codes(line);
        lines.push_back(std::move(line));
    }
    return lines;
}

void apply_translations(std::vector<std::string>& lines) {
    auto tag = LanguageTag::parse(g_config.language);
    if (!tag) {
        if (!g_config.language.empty()) {
            bw::println("Localization: malformed language tag '{}' in config, using English", g_config.language);
        }
        return;
    }

    for (const auto& candidate : tag->lookup_chain()) {
        const auto path = std::format("rez\\localizations\\stat_{}.txt", candidate);
        auto overlay = read_stat_file(path);
        if (!overlay) {
            continue;
        }
        if (overlay->size() > lines.size()) {
            bw::println("Localization: {} has {} strings but stat.txt has {}; ignoring the extra translations",
                path, overlay->size(), lines.size());
            overlay->resize(lines.size());
        }
        for (size_t i = 0; i < overlay->size(); i++) {
            std::string& translation = (*overlay)[i];
            if (is_untranslated(translation)) {
                continue;
            }
            lines[i] = std::move(translation);
        }
    }
}

} // namespace locale