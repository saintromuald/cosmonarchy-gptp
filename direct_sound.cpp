#include <windows.h>
#include <dsound.h>
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

// These hooks facilitate a new DirectSound implementation, which will enable arbitrary sound channels and other features. -Pr0nogo
namespace hooks
{

    s32 soundThread(s32 edi0, s32 a2) // 004BC620
    {
        s32* v2;
        s32 v3;
        char* v4;
        const void* v5;
        s32 v6;
        s32* v7;
        s32 v8;
        u32 v9;
        s32* v10;
        s32* v11;
        s32 v12;
        char v13;
        char v14;
        s32 v15;
        s32 v17;
        char fileName;
        char a3;
        s32 v20;
        s32 v21;
        s32 v22;
        s32 v23;
        char* v24;
        char* v25;
        void* v26;
        u32 v27;
        s32 v28;
        s32 v29;
        s32 fileSize;
        s32* v32;

        void* soundFile = 0;
        WaitForSingleObject(soundThreadEvent, 0xFFFFFFFF);
        if (*unkSoundCore)
        {
            v17 = edi0;
            do
            {
                v2 = soundVolume;
                v28 = 0;
                v32 = soundVolume;
                do
                {
                    if (!(*(v2 - 10) & 8))
                    {
                        goto LABEL_20;
                    }
                    v4 = &soundData[16 * *(v2 - 1)];
                    v29 = *(v2 - 1);
                    v3 = v29;
                    v25 = v4;
                    *(v4 + 1) = 81 - GetTickCount();
                    if (!SFXDAT_SoundFile[v3])
                    {
                        goto LABEL_17;
                    }
                    sprintf_s(&fileName, 260, "sound\\%s", SFXDAT_SoundFile[v3]);
                    soundFile = helper::fastFileRead(fileSize, 1, &fileName, fileName, 0, 1, "Starcraft\\SWAR\\lang\\snd.cpp", 737);
                    if (soundFile)
                    {
                        if (fileSize)
                        {
                            v5 = helper::parseWaveFile(*(s32*)soundFile, fileSize, a3);
                            if (v5)
                            {
                                v6 = fileSize;
                                v23 = 0;
                                *v4 = fileSize;
                                v7 = (v4 + 12);
                                v24 = &a3;
                                v20 = 20;
                                v21 = 194;
                                v22 = v6;
                                if ((*(directSoundInterface->lpVtbl + 12))(directSoundInterface->lpVtbl, &v20, v7, 0))
                                {
                                    *v7 = 0;
                                }
                                else if ((*(**v7 + 44))(*v7, 0, v6, &v26, &v27, 0, 0, 0, v17))
                                {
                                    v17 = *v7;
                                    (*(**v7 + 8))();
                                    *v7 = 0;
                                }
                                else
                                {
                                    memcpy(v26, v5, v27); // was qmemcpy
                                    v17 = 0;
                                    (*(**v7 + 76))(*v7, v26, v27, 0);
                                    if (!(SFXDAT_Flags[v29] & 1))
                                    {
                                        v28 += fileSize;
                                    }
                                    v8 = v25;
                                    v9 = helper::getSoundDuration(v25);
                                    v10 = v32;
                                    *(v8 + 8) = v9;
                                    if (*(v10 - 10) & 8)
                                    {
                                        v11 = v10;
                                        (*(**v7 + 60))(*v7, *v10 - *volumeReduction);
                                        (*(**v7 + 64))(*v7, v11[1]);
                                        (*(**v7 + 48))(*v7, 0, 0, 0);
                                        v12 = v29;
                                        v13 = SFXDAT_Flags[v29];
                                        *(v11 - 4) = *v7;
                                        v14 = SFXDAT_Priority[v12];
                                        *(v11 - 10) = v13;
                                        *(v11 - 9) = v14;
                                        *(v11 - 8) = 0;
                                    }
                                }
                            }
                        }
LABEL_17:
                        if (soundFile)
                        {
                            helper::SMemFree(soundFile, "Starcraft\\SWAR\\lang\\snd.cpp", 823, 0);
                            soundFile = 0;
                        }
                    }
                    SFXDAT_Flags[v29] &= 0xF7u;
                    v2 = v32;
                    *(v2 - 10) &= 0xF7u;
LABEL_20:
                    v2 += 6;
                    v32 = v2;
                } while (v2 < unkSoundCore2);
                v15 = v28;
                if (v28)
                {
                    EnterCriticalSection(soundLock);
                    *currentSoundMemory += v15;
                    LeaveCriticalSection(soundLock);
                }
                WaitForSingleObject(soundThreadEvent, 0xFFFFFFFF);
            } while (*unkSoundCore);
        }
        return 0;
    }

    s32 soundInit(s32 a1, s32 a2) // 004BCB70
    {
        s32* v2;
        s32 result;

        *v2 = a1;
        if (directSoundInterface->lpVtbl)
        {
            return 1;
        }
        a1 = 0;
        (a1 + 8) = 0;
        (a1 + 4) = 138;
        memset(soundChannels, 0, 0xC0u);
        if (!hdSoundModule)
        {
            *hdSoundModule = LoadLibraryA("dsound.dll");
            if (!hdSoundModule)
            {
                v2[1] = 133;
                *v2 = GetLastError();
                v2[2] = "LoadLibrary";
LABEL_6:
                helper::dSoundDestroy();
                return 0;
            }
        }
        if (!helper::dSoundCreate(v2) || !helper::setCoopLevel(a2, v2) || !helper::createSoundBuffer(v2))
        {
            goto LABEL_6;
        }
        helper::unkSoundCoreFunc();
        helper::sFileDdaInitialize(directSoundInterface->lpVtbl);
        *ddaInitialized = 1;
        helper::sVidInitialize(directSoundInterface->lpVtbl);
        *vidInitialized = 1;
        helper::loadButtonSFXFile();
        helper::initVolume();
        if (helper::sRegLoadValue("Starcraft", "Sound Memory Cache", 0, *soundMemoryCache)) // was &soundMemoryCache
        {
            if (*soundMemoryCache < 0x100000)
            {
                *soundMemoryCache = 0x100000;
            }
        }
        else
        {
            *soundMemoryCache = 0x400000;
        }
        *currentSoundMemory = 0;
        *soundThreadEvent = CreateEventA(0, 0, 0, 0);
        if (soundThreadEvent && (*unkSoundCore = 1, (*soundHandle = helper::beginThreadEx(0, 0, soundThread, 0, 0, *unkSoundCore3)) != 0)) // was &unkSoundCore3
        {
            *unkSoundBoolCore = 1;
            result = 1;
        }
        else
        {
            helper::dSoundDestroy();
            result = 0;
        }
        return result;
    }
}