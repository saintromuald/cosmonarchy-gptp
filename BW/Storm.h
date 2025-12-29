#pragma once

#include <BW/api.h>
#include <utility>

// Loads an MPQ file to memory (SC function 0x004D2D10)
// - Returns the data pointer on success (see `buffer` argument)
// - Returns NULL only in the cases specified by the `returnIfNotFound` argument
// - Throws a file read error if the file cannot be read, regardless of `returnIfNotFound`
// Arguments:
//     *filename				- MPQ path to file
//     *buffer					- Pointer to where memory will be written, or NULL to allocate memory with SMemAlloc
//     returnIfNotFound	- If true then function returns NULL if the file is not found or has a size of 0, otherwise a fatal error is thrown
//     scope						- Passed directly to SFileOpenFileEx (usually 0)
//     *filesize					- Returns filesize, can be NULL
void* FastFileRead(const char* filename, void* buffer, bool returnIfNotFound, u32 scope, u32* filesize);

inline auto g_storm_dll_handle = GetModuleHandleA("storm.dll");

template <typename>
class StormStdCall;

template <typename Result, typename... Args>
class StormStdCall<Result(Args...)> {
public:
    using FuncPtr = Result(__stdcall*)(Args...);

    StormStdCall(const char* name) : m_func{reinterpret_cast<FuncPtr>(GetProcAddress(g_storm_dll_handle, name))} {}

    StormStdCall(u32 ordinal)
        : m_func{reinterpret_cast<FuncPtr>(GetProcAddress(g_storm_dll_handle, MAKEINTRESOURCE(ordinal)))} {
    }

    Result operator()(Args... args) const { return m_func(args...); }

private:
    FuncPtr m_func;
};

inline const StormStdCall<b32()> SNetDestroy{102};
inline const StormStdCall<b32(u32, b32(__stdcall* callback)(u32 provider_id, const char* provider_name, const char* provider_description, Caps* caps))> SNetEnumProviders{105};
inline const StormStdCall<b32(Caps* caps)> SNetGetProviderCaps{114};
inline const StormStdCall<b32(u32, u32, bool(__stdcall* callback)(LobbyInfo* lobby_info), u32)> SNetEnumGamesEx{133};
inline const StormStdCall<void*(u32 size, const char* filename, u32 line, int unknown)> SMemAlloc{401};
inline const StormStdCall<u32(void* ptr, const char* filename, u32 line, int unknown)> SMemFree{403};
inline const StormStdCall<u32(HANDLE file)> SFileCloseFile{253};
inline const StormStdCall<u32(HANDLE file, u32* size)> SFileGetFileSize{256};
inline const StormStdCall<u32(HANDLE archive, const char* filename, u32 scope, HANDLE* file)> SFileOpenFileEx{268};
inline const StormStdCall<u32(HANDLE file, void* buffer, u32 to_read, u32* bytes_read, int unknown)> SFileReadFile{269};