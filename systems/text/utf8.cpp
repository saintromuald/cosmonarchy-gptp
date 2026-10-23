#include "utf8.h"

#include "logger.h"

#include <algorithm>
#include <format>
#include <utility>

#include <simdutf.h>

namespace text {
namespace {

void log_conversion_failure(bool& logged, std::string_view direction, const simdutf::result& conversion) {
    if (std::exchange(logged, true)) {
        return;
    }
    gptp::g_logger << std::format(
        "utf8: {} conversion failed: {} at code unit {}\n",
        direction, simdutf::error_to_string(conversion.error), conversion.count);
}

bool g_utf16_to_utf8_logged = false;
bool g_utf8_to_utf16_logged = false;

} // namespace

size_t truncate_to_boundary(std::string_view text, size_t cap) {
    return simdutf::trim_partial_utf8(text.data(), std::min(text.size(), cap));
}

std::optional<std::string> utf16_to_utf8(std::wstring_view text) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "Windows wchar_t is UTF-16");
    if (text.empty()) {
        return std::string{};
    }
    const auto* input = reinterpret_cast<const char16_t*>(text.data());
    std::string output(simdutf::utf8_length_from_utf16le(input, text.size()), '\0');
    const simdutf::result conversion =
        simdutf::convert_utf16le_to_utf8_with_errors(input, text.size(), output.data());
    if (conversion.error != simdutf::error_code::SUCCESS) {
        log_conversion_failure(g_utf16_to_utf8_logged, "UTF-16 to UTF-8", conversion);
        return std::nullopt;
    }
    output.resize(conversion.count);
    return output;
}

std::optional<std::wstring> utf8_to_utf16(std::string_view text) {
    if (text.empty()) {
        return std::wstring{};
    }
    std::wstring output(simdutf::utf16_length_from_utf8(text.data(), text.size()), L'\0');
    const simdutf::result conversion = simdutf::convert_utf8_to_utf16le_with_errors(
        text.data(), text.size(), reinterpret_cast<char16_t*>(output.data()));
    if (conversion.error != simdutf::error_code::SUCCESS) {
        log_conversion_failure(g_utf8_to_utf16_logged, "UTF-8 to UTF-16", conversion);
        return std::nullopt;
    }
    output.resize(conversion.count);
    return output;
}

bool is_valid_utf8(std::string_view text) {
    return simdutf::validate_utf8_with_errors(text.data(), text.size()).error
        == simdutf::error_code::SUCCESS;
}

} // namespace text