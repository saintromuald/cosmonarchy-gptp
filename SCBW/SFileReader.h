#pragma once
#include <SCBW/api.h>
#include <Windows.h>

typedef void* HANDLE;
typedef void* (__stdcall* SMemAlloc_type)(int size, char* filename, int line, int value);
typedef u32(__stdcall* SMemFree_type)(void* ptr, char* filename, int line, int idk);
typedef u32(__stdcall* SFileGetFileSize_type)(HANDLE hFile, LPDWORD lpFileSizeHigh);
typedef u32(__stdcall* SFileCloseFile_type)(HANDLE hFile);
typedef u32(__stdcall* SFileOpenFileEx_type)(HANDLE hArchive, char* filename, int scope, HANDLE* hFile);
typedef u32(__stdcall* SFileReadFile_type)(HANDLE hFile, void* buffer, int toRead, int* read, int overlapped);

extern SMemAlloc_type SMemAlloc;
extern SMemFree_type SMemFree;
extern SFileGetFileSize_type SFileGetFileSize;
extern SFileCloseFile_type SFileCloseFile;
extern SFileOpenFileEx_type SFileOpenFileEx;
extern SFileReadFile_type SFileReadFile;