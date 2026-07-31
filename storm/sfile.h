#pragma once

#include "storm/storm.h"
#include "storm/smem.hpp"

#include <concepts>
#include <string>
#include <type_traits>
#include <utility>

// So we don't need to include bwhelper.
namespace helper {
[[noreturn]] void fatal_file_not_found(u32 code, const char* filename); // 004212C0
} // namespace helper

namespace sfile {

// Windows error codes, so we don't need to include windows.h.
inline constexpr u32 ErrorFileNotFound = 2; // ERROR_FILE_NOT_FOUND
inline constexpr u32 ErrorInvalidData = 13; // ERROR_INVALID_DATA
inline constexpr u32 ErrorBadLength = 24;   // ERROR_BAD_LENGTH
inline constexpr u32 ErrorHandleEOF = 38;   // ERROR_HANDLE_EOF
inline constexpr u32 ErrorUnrecognizedVolume = 1006; // ERROR_UNRECOGNIZED_VOLUME

template <bool fatal_on_fail>
struct LoadedFile {
    smem::ptr<char[]> data;
    size_t            size{};
    std::string       filename;

    explicit operator bool() const noexcept { return static_cast<bool>(data); }

    // Cast when size matches exactly
    template <typename T>
    smem::ptr<T> cast() {
        if (size != sizeof(T)) {
            if constexpr (fatal_on_fail) {
                helper::fatal_file_not_found(ErrorBadLength, filename.c_str());
            }
            return {};
        }
        return smem::ptr<T>{reinterpret_cast<T*>(data.release())};
    }

    // Use for dynamically sized types e.g. .bin and .grp sides
    // Ideally provide a bool validate_cast(type) function for further validation
    template <typename T>
    smem::ptr<T> unsafe_cast() {
        if (size < sizeof(T)) {
            if constexpr (fatal_on_fail) {
                helper::fatal_file_not_found(ErrorBadLength, filename.c_str());
            }
            return {};
        }
        auto result = smem::ptr<T>{reinterpret_cast<T*>(data.release())};
        if constexpr (requires { { validate_cast(*result) } -> std::convertible_to<bool>; }) {
            if (!validate_cast(*result)) {
                if constexpr (fatal_on_fail) {
                    helper::fatal_file_not_found(ErrorInvalidData, filename.c_str());
                }
                return {};
            }
        }
        return result;
    }
};

namespace detail {

template <bool fatal_on_fail>
[[nodiscard]] inline LoadedFile<fatal_on_fail> load_file_impl(const char* filename) {
    LoadedFile<fatal_on_fail> out{};
    out.filename = filename;

    void* file_handle = nullptr;
    char* buffer = nullptr;
    u32 file_size = 0;
    u32 bytes_read = 0;
    u32 fatal_error = 0;
    s32 raw_size = 0;

    if (!SFileOpenFileEx(0, filename, 0, &file_handle)) {
        auto err = SErrGetLastError();
        if constexpr (!fatal_on_fail) {
            if (err == ErrorFileNotFound || err == ErrorUnrecognizedVolume) {
                goto cleanup;
            }
        }
        fatal_error = err;
        goto cleanup;
    }

    raw_size = SFileGetFileSize(file_handle, 0);
    if (raw_size == -1) {
        fatal_error = SErrGetLastError();
        goto cleanup;
    }

    if (!raw_size) {
        if constexpr (fatal_on_fail) {
            fatal_error = ErrorBadLength;
        }
        goto cleanup;
    }
    file_size = static_cast<u32>(raw_size);

    buffer = static_cast<char*>(SMemAlloc(file_size));
    if (!buffer) {
        goto cleanup;
    }

    if (!SFileReadFile(file_handle, buffer, file_size, &bytes_read, 0)) {
        auto err    = SErrGetLastError();
        fatal_error = (err == ErrorHandleEOF) ? ErrorBadLength : err;
        goto cleanup;
    }

    if (bytes_read != file_size) {
        fatal_error = ErrorBadLength;
        goto cleanup;
    }

    out.data = smem::ptr<char[]>{buffer};
    out.size = file_size;
    buffer   = nullptr;

cleanup:
    if (buffer)      SMemFree(buffer);
    if (file_handle) SFileCloseFile(file_handle);
    if (fatal_error) helper::fatal_file_not_found(fatal_error, filename);
    return out;
}

} // namespace detail

// Load a file, treating any failure as fatal.
[[nodiscard]] inline LoadedFile<true> load_mpq_file(const char* filename) {
    return detail::load_file_impl<true>(filename);
}

// Load a file, returning an empty `LoadedFile` if it can't be found.
[[nodiscard]] inline LoadedFile<false> try_load_mpq_file(const char* filename) {
    return detail::load_file_impl<false>(filename);
}

} // namespace sfile
