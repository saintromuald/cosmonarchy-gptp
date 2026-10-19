#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <vector>

static constexpr auto LanguagesYamlPath = "rez\\localizations\\languages.yml";

namespace locale {

// A BCP47 language tag split into canonical subtags: language, script, region, variants.
struct LanguageTag {
    std::vector<std::string> subtags;

    // Parses and case-canonicalizes a tag ("ko-kr" -> {"ko", "KR"}); nullopt when malformed.
    static std::optional<LanguageTag> parse(std::string_view tag);

    std::string to_string() const;

    // Lookup chain ordered least specific first: {"zh", "Hans", "TW"} -> "zh", "zh-Hans", "zh-Hans-TW".
    std::vector<std::string> lookup_chain() const;
};

// Entry in rez\localizations\languages.yml.
struct Language {
    std::string tag;
    std::string name;
};

struct LanguageList {
    std::vector<Language> languages;
};

// Load rez/localizations/languages.yml
LanguageList load_languages();

std::optional<std::vector<std::string>> read_stat_file(std::string_view path);

void apply_translations(std::vector<std::string>& lines);

} // namespace locale