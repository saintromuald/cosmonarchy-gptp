#include <SCBW/api.h>

// Equivalent to HANDLE, but different so that Windows.h doesn't have to be included
typedef void *SHANDLE;

void* SMemAlloc(u32 size, const char* filename, u32 line, int idk);
u32 SMemFree(void* ptr, const char* filename, u32 line, int idk);
u32 SFileCloseFile(SHANDLE hFile);
u32 SFileGetFileSize(SHANDLE hFile, u32* lpFileSizeHigh);
u32 SFileOpenFileEx(SHANDLE hArchive, const char* filename, u32 scope, SHANDLE* hFile);
u32 SFileReadFile(SHANDLE hFile, void* buffer, u32 toRead, u32* read, int idk);

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