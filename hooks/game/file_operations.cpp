#include "file_operations.h"
#include <SCBW/api.h>
#include <windows.h>

namespace {
    // for openMapFile
    int32 SStrCopy(char* dest, const char* src, int32 dwSize);                              // 0041008E
    int32 SStrNCat(char* dest, const char* src, int32 maxLength);                           // 0041007C // also, N
    char* strrchr(const char* str, int32 ch);                                             // 0040C530
    int32 createDirectory(const char* folder);                                            // 0041EAB0
    FILE* pfOpen(int32 a1, const char* a2);                                               // 00420540
    size_t* lockwrite(const void* data, size_t* count, size_t* sizeArg, FILE* file);    // 00411931
    
    // for loadMap
    int32 readMapChunks(int32 a1, int32 a2, int32* a3, int32 sizeArg);                               // 004CC060
    int32 readChunkNodes(int32 count, int32 a2, chkStruct* nodes, void* buffer, int32 a5);      // 00413710
    int32 unkNetworkFunc(char* a1, char* a2, int32 a3);                                     // 004CC7F0
}

namespace hooks {
	bool openMapFile(int32 vector) {							                            // 0045A290
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

    // 2
    int32 loadMap(int32 a1, char* a2, int32 a3) {                                             // 004BF520
        int32 v3; // esi
        void* v4; // edi
        int32 result; // eax
        scenarioFormat* v6; // ecx
        int32 v7; // [esp+0h] [ebp-4h]

        v7 = a1;
        if (IS_IN_REPLAY)
        {
            v3 = *scenarioCHKSize;
            v4 = scenarioChk;
            v7 = 0;
            result = readMapChunks(0, (int32)scenarioChk, (int32*)&v7, *scenarioCHKSize);
            if (result) {
                if (gameData->victoryCondition || gameData->startingUnits || gameData->tournamentModeEnabled)
                    v6 = &stru_500574[5 * v7];
                else
                    v6 = &stru_50057C[5 * v7];
                result = readChunkNodes(v6->chkStructCount, v3, v6->chkStruct, v4, 0);
            }
        }
        else if (CurrentMapFileName[0]) {
            result = unkNetworkFunc(CurrentMapFileName, a2, a3) != 0;
        }
        else {
            result = 0;
        }
        return result;
    }
}

namespace {
    // for openMapFile
    const uint32 Func_SStrCopy = 0x0041008E;
    int32 SStrCopy(char* dest, const char* src, int32 dwSize) {
        static int32 result;
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

    const uint32 Func_SStrNCat = 0x0041007C;
    int32 SStrNCat(char* dest, const char* src, int32 maxLength) { // N's cat lives on
        static int32 result;
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

    const uint32 Func_strrchr = 0x0040C530;
    char* strrchr(const char* str, int32 ch) {
        static int32* result;
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

    const uint32 Func_createDirectory = 0x0041EAB0;
    int32 createDirectory(const char* folder) {
        static int32 result;
        __asm {
            PUSHAD
            MOV EDI, folder
            CALL Func_createDirectory
            MOV result, EAX
            POPAD
        }
        return result;
    }

    const uint32 Func_pfOpen = 0x00420540;
    FILE* pfOpen(int32 a1, const char* a2) {
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

    const uint32 Func_lockwrite = 0x00411931;
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

    // for loadMap
    const uint32 Func_readMapChunks = 0x004CC060;
    int32 readMapChunks(int32 a1, int32 a2, int32* a3, int32 sizeArg) {
        static int32 result;
        __asm {
            PUSHAD
            PUSH sizeArg
            MOV EBX, a3
            MOV ECX, a2
            MOV EAX, a1
            CALL Func_readMapChunks
            MOV result, EAX
            POPAD
        }
        return result;
    }

    const uint32 Func_readChunkNodes = 0x00413710;
    int32 readChunkNodes(int32 count, int32 a2, chkStruct* nodes, void* buffer, int32 a5) {
        static int32 result;
        __asm {
            PUSHAD
            PUSH a5
            PUSH buffer
            MOV ECX, nodes
            MOV EDX, a2
            MOV EAX, count
            CALL Func_readChunkNodes
            MOV result, EAX
            POPAD
        }
        return result;
    }

    const uint32 Func_unkNetworkFunc = 0x004CC7F0;
    int32 unkNetworkFunc(char* a1, char* a2, int32 a3) {
        static int32 result;
        __asm {
            PUSHAD
            MOV ESI, a3
            MOV EDI, a2
            MOV EAX, a1
            CALL Func_unkNetworkFunc
            MOV result, EAX
            POPAD
        }
        return result;
    }
}