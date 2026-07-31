#pragma once

#include "types.h"

struct Palette;
struct Caps;
struct LobbyInfo;

using HANDLE = void*;

enum class SNetGetGameInfoType : u32 {
    Name            = 1,
    Password        = 2,
    Mode            = 3,
    StatString      = 4,
    InitDataBytes   = 5,
    MaxPlayers      = 6,
};

extern "C" {
// ---- SNet ----
__declspec(dllimport) b32   __stdcall SNetDestroy();
__declspec(dllimport) b32   __stdcall SNetEnumProviders(u32 unknown, b32(__stdcall* callback)(u32 provider_id, const char* provider_name, const char* provider_description, Caps* caps));
__declspec(dllimport) b32   __stdcall SNetGetGameInfo(SNetGetGameInfoType type, char* output, u32 output_size, u32* bytes_written);
__declspec(dllimport) b32   __stdcall SNetGetPlayerName(u32 player_id, char* buffer, u32 buffer_size);
__declspec(dllimport) b32   __stdcall SNetGetProviderCaps(Caps* caps);
__declspec(dllimport) b32   __stdcall SNetGetTurnsInTransit(u32* turns);
__declspec(dllimport) b32   __stdcall SNetSendMessage(u32 player_id, void* data, u32 bytes);
__declspec(dllimport) b32   __stdcall SNetEnumGamesEx(u32 a1, u32 a2, bool(__stdcall* callback)(LobbyInfo* lobby_info), u32 a4);
__declspec(dllimport) s32   __stdcall SNetSendServerChatCommand(const char* command);
__declspec(dllimport) b32   __stdcall SNetGetLeaguePlayerName(char* output, u32 size);

// ---- SFile ----
__declspec(dllimport) b32   __stdcall SFileCloseArchive(HANDLE archive);
__declspec(dllimport) u32   __stdcall SFileCloseFile(HANDLE file);
__declspec(dllimport) b32   __stdcall SFileDdaBeginEx(HANDLE directsound, u32 flags, u32 mask, u32 distance_to_move, s32 volume, s32 a6, int a7);
__declspec(dllimport) b32   __stdcall SFileDdaEnd(HANDLE directsound);
__declspec(dllimport) b32   __stdcall SFileDdaInitialize(HANDLE directsound);
__declspec(dllimport) s32   __stdcall SFileGetFileSize(HANDLE file, u32* size);
__declspec(dllimport) b32   __stdcall SFileOpenArchive(const char* archive_name, u32 priority, u32 flags, HANDLE* archive);
__declspec(dllimport) b32   __stdcall SFileOpenFile(const char* filename, HANDLE* handle);
__declspec(dllimport) b32   __stdcall SFileOpenFileEx(HANDLE archive, const char* filename, u32 scope, HANDLE* file);
__declspec(dllimport) b32   __stdcall SFileReadFile(HANDLE file, void* buffer, u32 to_read, u32* bytes_read, int unknown);

// ---- SBmp ----
__declspec(dllimport) b32   __stdcall SBmpLoadImage(const char* filename, Palette* palette, u8* buffer, u32 buffer_size, u32* width, u32* height, u32* bitdepth);
__declspec(dllimport) b32   __stdcall SBmpAllocLoadImage(const char* filename, Palette* palette, u8** buffer, u32* width, u32* height, u32 a6, u32 a7, u8*(__stdcall* malloc_function)(u32 size));
// ---- SMem ----
__declspec(dllimport) void* __stdcall SMemAlloc(u32 size,      const char* filename = "", u32 line = 0, int unknown = 0);
__declspec(dllimport) u32   __stdcall SMemFree (void* pointer, const char* filename = "", u32 line = 0, int unknown = 0);

// ---- SReg ----
__declspec(dllimport) b32   __stdcall SRegLoadValue(const char* key_name, const char* value_name, u8 flags, u32* value);

// ---- STrans ----
__declspec(dllimport) b32   __stdcall STransCreateE(u8* bitmap_buffer, u32 width, u32 height, u32 color_depth, u32 unknown1, u32 unknown2, void** surface);

// ---- SVid ----
__declspec(dllimport) b32   __stdcall SVidInitialize(HANDLE video);
__declspec(dllimport) b32   __stdcall SVidPlayEnd(HANDLE video);

// ---- SErr ----
__declspec(dllimport) b32   __stdcall SErrGetErrorStr(u32 error_code, char* buffer, u32 buffer_size);
__declspec(dllimport) u32   __stdcall SErrGetLastError();
__declspec(dllimport) void  __stdcall SErrSetLastError(u32 error_code);
}
