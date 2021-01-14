#include "file_operations.h"
#include <SCBW/api.h>
#include <windows.h>

namespace {
    // for openMapFile
    s32 SStrCopy(char* dest, const char* src, s32 dwSize);                              // 0041008E
    s32 SStrNCat(char* dest, const char* src, s32 maxLength);                           // 0041007C // also, N
    char* strrchr(const char* str, s32 ch);                                             // 0040C530
    s32 createDirectory(const char* folder);                                            // 0041EAB0
    FILE* pfOpen(s32 a1, const char* a2);                                               // 00420540
    size_t* lockwrite(const void* data, size_t* count, size_t* sizeArg, FILE* file);    // 00411931
    
}

namespace hooks {
	bool openMapFile(s32 vector) {							                            // 0045A290
        FILE* map; // edi
        bool result; // eax
        char FileName; // [esp+8h] [ebp-104h]

        SStrCopy(&FileName, mapsFolder, 0x104u);
        SStrNCat(&FileName, (const char*)(vector + 20), 0x104u);
        strrchr(&FileName, 92);
        SStrCopy(CurrentMapFileName, &FileName, 0x104u);
        *campaignIndex = 0;
        *dword_5994DC = 1;
        if (!createDirectory(mapsFolder))
            return 0;
        map = pfOpen(23, &FileName);
        if (!map)
            return 0;
        if (fclose(map)) {
            DeleteFileA(&FileName);
            result = 0;
        }
        else {
            if (!lockwrite(*(const void**)(vector + 68), (size_t*)(vector + 12), (size_t*)1u, map))
                DeleteFileA(&FileName);
            result = 0;
        }
        return result;
	}
}

namespace {
    // for openMapFile
    const u32 Func_SStrCopy = 0x0041008E;
    s32 SStrCopy(char* dest, const char* src, s32 dwSize) {
        static s32 result;
        __asm {
            PUSHAD
            PUSH dwSize
            PUSH src
            PUSH dest
            CALL Func_SStrCopy
            MOV result, EAX
            POPAD
        }
        return result;
    }

    const u32 Func_SStrNCat = 0x0041007C;
    s32 SStrNCat(char* dest, const char* src, s32 maxLength) { // N's cat lives on
        static s32 result;
        __asm {
            PUSHAD
            PUSH maxLength
            PUSH src
            PUSH dest
            CALL Func_SStrNCat
            MOV result, EAX
            POPAD
        }
        return result;
    }

    const u32 Func_strrchr = 0x0040C530;
    char* strrchr(const char* str, s32 ch) {
        static s32* result;
        __asm {
            PUSHAD
            PUSH ch
            PUSH str
            CALL Func_strrchr
            MOV result, EAX
            POPAD
        }
        return (char*)result;
    }

    const u32 Func_createDirectory = 0x0041EAB0;
    s32 createDirectory(const char* folder) {
        static s32 result;
        __asm {
            PUSHAD
            MOV EDI, folder
            CALL Func_createDirectory
            MOV result, EAX
            POPAD
        }
        return result;
    }

    const u32 Func_pfOpen = 0x00420540;
    FILE* pfOpen(s32 a1, const char* a2) {
        static FILE* result;
        __asm {
            PUSHAD
            PUSH a2
            MOV EAX, a1
            CALL Func_pfOpen
            MOV result, EAX
            POPAD
        }
        return result;
    }

    const u32 Func_lockwrite = 0x00411931;
    size_t* lockwrite(const void* data, size_t* count, size_t* sizeArg, FILE* file) {
        static size_t* result;
        __asm {
            PUSHAD
            PUSH file
            PUSH sizeArg
            PUSH count
            PUSH data
            CALL Func_lockwrite
            MOV result, EAX
            POPAD
        }
        return result;
    }
}