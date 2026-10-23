#pragma once
#include "types.h"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>

// Legacy pre-filled fields may contain malformed UTF-8; sequence walks must stay bounded

namespace text {

constexpr bool is_continuation(u8 byte) {
    return (byte & 0xC0) == 0x80;
}

// Continuations and invalid leads (0x80-0xC1, 0xF5-0xFF) count as one byte
constexpr size_t sequence_length(u8 lead) {
    if (lead >= 0xF0 && lead <= 0xF4) {
        return 4;
    }
    if (lead >= 0xE0 && lead <= 0xEF) {
        return 3;
    }
    if (lead >= 0xC2 && lead <= 0xDF) {
        return 2;
    }
    return 1;
}

// Stops at the text boundary or the first non-continuation byte
constexpr size_t sequence_length_at(std::string_view text, size_t offset) {
    if (offset >= text.size()) {
        return 0;
    }
    const size_t announced = sequence_length((u8)text[offset]);
    size_t length = 1;
    while (length < announced
        && offset + length < text.size()
        && is_continuation((u8)text[offset + length])) {
        ++length;
    }
    return length;
}

// Uses the same sequence boundaries as sequence_length_at, including for malformed input
constexpr size_t prev_sequence_start(std::string_view text, size_t cursor) {
    const size_t end = cursor < text.size() ? cursor : text.size();
    size_t start = end;
    for (size_t step = 0; step < 4 && start > 0; ++step) {
        --start;
        if (!is_continuation((u8)text[start])) {
            break;
        }
    }
    if (start + sequence_length_at(text, start) < end) {
        return end - 1;
    }
    return start;
}

constexpr size_t snap_to_sequence_start(std::string_view text, size_t offset) {
    if (offset >= text.size() || !is_continuation((u8)text[offset])) {
        return offset;
    }
    return prev_sequence_start(text, offset + 1);
}

// Returns 0 for surrogates and values above U+10FFFF
constexpr size_t encode_utf8(char32_t codepoint, u8 out[4]) {
    if (codepoint <= 0x7F) {
        out[0] = (u8)codepoint;
        return 1;
    }
    if (codepoint <= 0x7FF) {
        out[0] = (u8)(0xC0 | (codepoint >> 6));
        out[1] = (u8)(0x80 | (codepoint & 0x3F));
        return 2;
    }
    if ((codepoint >= 0xD800 && codepoint <= 0xDFFF) || codepoint > 0x10FFFF) {
        return 0;
    }
    if (codepoint <= 0xFFFF) {
        out[0] = (u8)(0xE0 | (codepoint >> 12));
        out[1] = (u8)(0x80 | ((codepoint >> 6) & 0x3F));
        out[2] = (u8)(0x80 | (codepoint & 0x3F));
        return 3;
    }
    out[0] = (u8)(0xF0 | (codepoint >> 18));
    out[1] = (u8)(0x80 | ((codepoint >> 12) & 0x3F));
    out[2] = (u8)(0x80 | ((codepoint >> 6) & 0x3F));
    out[3] = (u8)(0x80 | (codepoint & 0x3F));
    return 4;
}

// Drops an incomplete trailing sequence
size_t truncate_to_boundary(std::string_view text, size_t cap);

// Capacity includes the NUL terminator; returns bytes copied, excluding the terminator, without splitting a codepoint
inline size_t copy_truncated(char* to, size_t capacity, const char* from) {
    if (!capacity) {
        return 0;
    }
    const std::string_view source = from ? std::string_view{from} : std::string_view{};
    const size_t length = truncate_to_boundary(source, capacity - 1);
    std::copy_n(source.data(), length, to);
    to[length] = '\0';
    return length;
}

template <size_t Capacity>
size_t copy_truncated(char (&to)[Capacity], const char* from) {
    return copy_truncated(to, Capacity, from);
}

// Invalid input returns nullopt; the first failure per direction is logged
std::optional<std::string> utf16_to_utf8(std::wstring_view text);
std::optional<std::wstring> utf8_to_utf16(std::string_view text);

bool is_valid_utf8(std::string_view text);

} // namespace text