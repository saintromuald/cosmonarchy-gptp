#include "SFileReader.h"
#include <SCBW/api.h>
#include <Windows.h>

SMemAlloc_type SMemAlloc = (SMemAlloc_type)0x0041006A;
SMemFree_type SMemFree = (SMemFree_type)0x00410070;
SFileGetFileSize_type SFileGetFileSize = (SFileGetFileSize_type)0x00410142;
SFileCloseFile_type SFileCloseFile = (SFileCloseFile_type)0x004100B8;
SFileOpenFileEx_type SFileOpenFileEx = (SFileOpenFileEx_type)0x004100C4;
SFileReadFile_type SFileReadFile = (SFileReadFile_type)0x00410148;