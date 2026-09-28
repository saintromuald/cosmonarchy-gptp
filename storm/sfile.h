#pragma once

#include "storm/storm.h"
#include "storm/smem.hpp"

#include <algorithm>
#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <filesystem>
namespace fs = std::filesystem;

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

struct LoadedFile {
    smem::ptr<char[]> data;
    size_t            size{};
    std::string       filename;

    explicit operator bool() const noexcept { return static_cast<bool>(data); }

    template <typename T>
    smem::ptr<T> cast() { return cast_impl<T, true>();}

    template <typename T>
    smem::ptr<T> try_cast() { return cast_impl<T, false>(); }

    template <typename T>
    smem::ptr<T> unsafe_cast() { return unsafe_cast_impl<T, true>();}

    template <typename T>
    smem::ptr<T> try_unsafe_cast() { return unsafe_cast_impl<T, false>();}

private:
    template <typename T, bool fatal_on_fail>
    smem::ptr<T> cast_impl() {
        if (size != sizeof(T)) {
            if constexpr (fatal_on_fail) {
                helper::fatal_file_not_found(ErrorBadLength, filename.c_str());
            }
            return {};
        }
        return smem::ptr<T>{reinterpret_cast<T*>(data.release())};
    }

    template <typename T, bool fatal_on_fail>
    smem::ptr<T> unsafe_cast_impl() {
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

// Streaming file handle; reads incrementally instead of loading the whole file
class StreamFile {
public:
    StreamFile() = default;
    ~StreamFile() {
        if (m_handle) {
            SFileCloseFile(m_handle);
        }
    }

    StreamFile(StreamFile&& other) noexcept : m_handle{std::exchange(other.m_handle, nullptr)} {}
    StreamFile& operator=(StreamFile&& other) noexcept {
        if (this != &other) {
            if (m_handle) {
                SFileCloseFile(m_handle);
            }
            m_handle = std::exchange(other.m_handle, nullptr);
        }
        return *this;
    }

    StreamFile(const StreamFile&) = delete;
    StreamFile& operator=(const StreamFile&) = delete;

    [[nodiscard]] static std::optional<StreamFile> open(std::string_view filename, SFileScope scope = SFileScope::DiskAndMpq) {
        std::string path{filename};
        std::replace(path.begin(), path.end(), '/', '\\');

        void* handle = nullptr;
        if (!SFileOpenFileEx(0, path.c_str(), scope, &handle)) {
            return {};
        }

        StreamFile file;
        file.m_handle = handle;
        return file;
    }

    explicit operator bool() const { return m_handle != nullptr; }

    // Reads up to count bytes, returning how many were actually read (a short read marks the end of the file)
    u64 read(void* destination, u64 count) {
        u32 bytes_read = 0;
        const u32 chunk = (u32)std::min<u64>(count, 0xFFFFFFFF);
        if (!SFileReadFile(m_handle, destination, chunk, &bytes_read, 0) && bytes_read == 0) {
            return 0;
        }
        return bytes_read;
    }

    bool seek(u64 position) {
        s32 position_high = (s32)(position >> 32);
        const u32 position_low = SFileSetFilePointer(m_handle, (s32)(u32)position, &position_high, SFileSeekMethod::Begin);
        return position_low != 0xFFFFFFFF || SErrGetLastError() == 0;
    }

    u64 position() {
        const u32 position_low = SFileSetFilePointer(m_handle, 0, nullptr, SFileSeekMethod::Current);
        return position_low;
    }

    u64 size() const {
        u32 size_high = 0;
        const s32 size_low = SFileGetFileSize(m_handle, &size_high);
        return size_low == -1 ? 0 : ((u64)size_high << 32) | (u32)size_low;
    }

private:
    void* m_handle = nullptr;
};

namespace detail {

template <bool fatal_on_fail>
[[nodiscard]] inline LoadedFile load_file_impl(const char* filename, SFileScope scope = SFileScope::MpqOnly) {
    LoadedFile out{};
    out.filename = filename;

    // Storm's absolute-path detection in SFileOpenFileEx only recognises backslashes (':\', '\\', leading '\').
    std::string path{filename};
    std::replace(path.begin(), path.end(), '/', '\\');

    void* file_handle = nullptr;
    char* buffer = nullptr;
    u32 file_size = 0;
    u32 bytes_read = 0;
    u32 fatal_error = 0;
    s32 raw_size = 0;

    if (!SFileOpenFileEx(0, path.c_str(), scope, &file_handle)) {
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
[[nodiscard]] inline LoadedFile load_file(const char* filename, SFileScope scope = SFileScope::MpqOnly) {
    return detail::load_file_impl<true>(filename, scope);
}

// Load a file, returning an empty `LoadedFile` if it can't be found.
[[nodiscard]] inline LoadedFile try_load_file(const char* filename, SFileScope scope = SFileScope::MpqOnly) {
    return detail::load_file_impl<false>(filename, scope);
}

// Check whether a file exists (disk and/or MPQ per scope) without loading it
[[nodiscard]] inline bool file_exists(std::string_view filename, SFileScope scope = SFileScope::DiskAndMpq) {
    fs::path path{filename};
    path.make_preferred();

    void* file_handle = nullptr;
    if (!SFileOpenFileEx(0, path.string().c_str(), scope, &file_handle)) {
        return false;
    }
    SFileCloseFile(file_handle);
    return true;
}

} // namespace sfile
