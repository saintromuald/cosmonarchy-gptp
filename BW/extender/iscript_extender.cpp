#include "hook_tools.h"
#include "BW/bwdata.h"
#include "BW/bwhelper.h"

char* g_iscript_filenames[] = {
    (char*)"scripts\\iscript.txt",
  //(char*)"scripts\\for_example.bin",
  //(char*)"scripts\\iscript_askosiO_O!!!.bin"
};

constexpr u32 ISCRIPT_FILECOUNT = sizeof(g_iscript_filenames) / 4;
char* g_iscript_handles[ISCRIPT_FILECOUNT];
u32 g_temp_iscript_filekey;

namespace hooks {
    void init_iscript_bin() {
        for (u32 i_0 = 0; i_0 < ISCRIPT_FILECOUNT; ++i_0) {
            HANDLE* file = nullptr;
            u32 read = 0;

            if (!helper::sfile_open_file_ex(nullptr, g_iscript_filenames[i_0], 0, &file)) {
                helper::syswarn_file_not_found(helper::serr_get_last_error(), g_iscript_filenames[i_0]);
            }

            u32 file_size = helper::SFileGetFileSize(file, 0);

            if (file_size == -1) {
                helper::file_fatal(file, GetLastError());
            }

            if (!file_size) {
                helper::syswarn_file_not_found(24, g_iscript_filenames[i_0]);
            }

            void* item = helper::SMemAlloc(file_size, "Starcraft\\SWAR\\lang\\gamedata.cpp", 210, 0);

            if (!helper::sfile_read_file(file, item, file_size, &read, 0)) {
                if (GetLastError() == 38) {
                    helper::file_fatal(file, 24);
                }

                helper::file_fatal(file, GetLastError());
            }

            if (read != file_size) {
                helper::file_fatal(file, 24);
            }

            helper::SFileCloseFile(file);
            g_iscript_handles[i_0] = (char*)item;
        }
    }

    void cleanup_iscript_handles() {
        for (u32 i_0 = 0; i_0 < ISCRIPT_FILECOUNT; ++i_0) {
            if (g_iscript_handles[i_0]) {
                helper::SMemFree(g_iscript_handles[i_0], "Starcraft\\SWAR\\lang\\CImage.cpp", 1626, 0);
                g_iscript_handles[i_0] = nullptr;
            }
        }
    }

    const u32 iscript_update_speed_retn = 0x0042D8E4;
    __declspec(naked) void iscript_update_speed() {
        __asm {
            MOV EDX, [EAX + 0x14]
            MOVZX EAX, byte ptr[EAX + 0xD]
            MOV g_temp_iscript_filekey, EAX
            MOV AL, 11
            JMP iscript_update_speed_retn
        }
    }

    const u32 sub_42DB50_retn = 0x0042DC22;
    __declspec(naked) void sub_42DB50() {
        __asm {
            MOV ds : [0x006C2314] , EAX//g_temp_iscript_return
            MOVZX EAX, byte ptr[ECX + 0xD]
            MOV g_temp_iscript_filekey, EAX
            JMP sub_42DB50_retn
        }
    }

    const u32 iscript_has_animation_retn = 0x0042D60D;
    __declspec(naked) void iscript_has_animation() {
        __asm {
            MOV EDX, g_temp_iscript_filekey
            SHR EDX, 4
            MOV EDX, [EDX * 4 + g_iscript_handles]
            JMP iscript_has_animation_retn
        }
    }

    const u32 ground_attack_init_retn = 0x004886A0;
    __declspec(naked) void ground_attack_init() {
        __asm {
            MOVZX EAX, byte ptr[ESI + 0xD]
            SHR EAX, 4
            MOV EDX, [EAX * 4 + g_iscript_handles]
            JMP ground_attack_init_retn
        }
    }

    const u32 is_valid_iscript_retn = 0x004D6645;
    __declspec(naked) void is_valid_iscript() {
        __asm {
            MOV EAX, EDI							// u32 iscript_key = imagesdat_iscript_id[image->id] >> 4;
            MOVZX EDI, DI							// u32 iscript_id = (u16)imagesdat_iscript_id[image->id];
            SHR EAX, 12
            AND[EBX + 0xD], 0xF					// image->flags &= 0x0FFF;
            OR[EBX + 0xD], AL						// image->flags |= iscript_key;
            MOVZX EAX, AL
            SHR EAX, 4								// iscript_key = (u16)iscript_key >> 12;
            MOV EAX, [EAX * 4 + g_iscript_handles]	// char* data = g_iscript_handles[iscript_key];
            JMP is_valid_iscript_retn
        }
    }

    const u32 iscript_cases_D74DE_retn = 0x004D74E3;
    __declspec(naked) void iscript_cases_D74DE() {
        __asm {
            MOV EAX, [EBP - 0xC]
            MOVZX EAX, byte ptr[EAX + 0xD]
            SHR EAX, 4
            MOV EAX, [EAX * 4 + g_iscript_handles]
            JMP iscript_cases_D74DE_retn
        }
    }

    const u32 iscript_cases_waitrand_retn = 0x004D761E;
    __declspec(naked) void iscript_cases_waitrand() {
        __asm {
            MOV EAX, [EBP - 0xC]
            MOVZX EAX, byte ptr[EAX + 0xD]
            SHR EAX, 4
            MOV ECX, [EAX * 4 + g_iscript_handles]
            JMP iscript_cases_waitrand_retn
        }
    }

    const u32 iscript_cases_goto_retn = 0x004D7640;
    __declspec(naked) void iscript_cases_goto() {
        __asm {
            MOV EDX, [EBP - 0xC]
            MOVZX EDX, byte ptr[EDX + 0xD]
            SHR EDX, 4
            MOV EDX, [EDX * 4 + g_iscript_handles]
            ADD EAX, EDX
            JMP iscript_cases_goto_retn
        }
    }

    const u32 iscript_cases_D7BAF_retn = 0x004D7BB5;
    __declspec(naked) void iscript_cases_D7BAF() {
        __asm {
            MOV EDX, [EBP - 0xC]
            MOVZX EDX, byte ptr[EDX + 0xD]
            SHR EDX, 4
            MOV EDX, [EDX * 4 + g_iscript_handles]
            ADD EAX, EDX
            JMP iscript_cases_D7BAF_retn
        }
    }

    const u32 iscript_cases_end_retn = 0x004D7BD7;
    __declspec(naked) void iscript_cases_end() {
        __asm {
            MOVZX EAX, byte ptr[ESI + 0xD]
            SHR EAX, 4
            MOV EAX, [EAX * 4 + g_iscript_handles]
            JMP iscript_cases_end_retn
        }
    }

    const u32 iscript_cases_randcondjmp_retn = 0x004D7F55;
    __declspec(naked) void iscript_cases_randcondjmp() {
        __asm {
            MOV EAX, [EBP - 0xC]
            MOVZX EAX, byte ptr[EAX + 0xD]
            SHR EAX, 4
            MOV ECX, [EAX * 4 + g_iscript_handles]
            JMP iscript_cases_randcondjmp_retn
        }
    }

    const u32 iscript_cases_trgtrangecondjmp_retn = 0x004D7FC0;
    __declspec(naked) void iscript_cases_trgtrangecondjmp() {
        __asm {
            MOV EAX, [EBP - 0xC]
            MOVZX EAX, byte ptr[EAX + 0xD]
            SHR EAX, 4
            MOV ECX, [EAX * 4 + g_iscript_handles]
            JMP iscript_cases_trgtrangecondjmp_retn
        }
    }

    const u32 iscript_cases_trgtarccondjmp_retn = 0x004D8048;
    __declspec(naked) void iscript_cases_trgtarccondjmp() {
        __asm {
            MOV EAX, [EBP - 0xC]
            MOVZX EAX, byte ptr[EAX + 0xD]
            SHR EAX, 4
            MOV EAX, [EAX * 4 + g_iscript_handles]
            ADD EDX, EAX
            JMP iscript_cases_trgtarccondjmp_retn
        }
    }

    const u32 iscript_cases_call_retn = 0x004D82CF;
    __declspec(naked) void iscript_cases_call() {
        __asm {
            MOV ECX, [EBP - 0xC]
            MOVZX ECX, byte ptr[ECX + 0xD]
            SHR ECX, 4
            MOV ECX, [ECX * 4 + g_iscript_handles]
            JMP iscript_cases_call_retn
        }
    }

    const u32 iscript_cases_return_retn = 0x004D82F7;
    __declspec(naked) void iscript_cases_return() {
        __asm {
            MOV EDX, [EBP - 0xC]
            MOVZX EDX, byte ptr[EDX + 0xD]
            SHR EDX, 4
            MOV EDX, [EDX * 4 + g_iscript_handles]
            ADD EAX, EDX
            JMP iscript_cases_return_retn
        }
    }

    const u32 iscript_cases_wait_retn = 0x004D8331;
    __declspec(naked) void iscript_cases_wait() {
        __asm {
            MOV EDX, [EBP - 0xC]
            MOVZX EDX, byte ptr[EDX + 0xD]
            SHR EDX, 4
            MOV EDX, [EDX * 4 + g_iscript_handles]
            SUB EDI, EDX
            JMP iscript_cases_wait_retn
        }
    }

    const u32 iscript_cases_ignorerest_retn = 0x004D8344;
    __declspec(naked) void iscript_cases_ignorerest() {
        __asm {
            MOV EAX, [EBP - 0xC]
            MOVZX EAX, byte ptr[EAX + 0xD]
            SHR EAX, 4
            MOV EDX, [EAX * 4 + g_iscript_handles]
            JMP iscript_cases_ignorerest_retn
        }
    }

    const u32 play_image_iscript_retn = 0x004D84D3;
    __declspec(naked) void play_image_iscript() {
        __asm {
            MOVZX EDI, byte ptr[ECX + 0xD]
            SHR EDI, 4
            MOV EDI, [EDI * 4 + g_iscript_handles]
            JMP play_image_iscript_retn
        }
    }

    const u32 play_warp_in_overlay_retn = 0x004D8519;
    __declspec(naked) void play_warp_in_overlay() {
        __asm {
            MOVZX EAX, byte ptr[EBX + 0xD]
            SHR EAX, 4
            MOV EDX, [EAX * 4 + g_iscript_handles]
            JMP play_warp_in_overlay_retn
        }
    }

    const u32 updateFunc_WarpFlash_retn = 0x004D85D0;
    __declspec(naked) void updateFunc_WarpFlash() {
        __asm {
            MOVZX EAX, byte ptr[EBX + 0xD]
            SHR EAX, 4
            MOV EDI, [EAX * 4 + g_iscript_handles]
            JMP updateFunc_WarpFlash_retn
        }
    }

    const u32 orders_ProtossBuildSelf_0_retn = 0x004E4F86;
    __declspec(naked) void orders_ProtossBuildSelf_0() {
        __asm {
            MOVZX EAX, byte ptr[EDI + 0xD]
            SHR EAX, 4
            MOV EDX, [EAX * 4 + g_iscript_handles]
            JMP orders_ProtossBuildSelf_0_retn
        }
    }

    const u32 orders_ProtossBuildSelf_1_retn = 0x004E5043;
    __declspec(naked) void orders_ProtossBuildSelf_1() {
        __asm {
            MOVZX EAX, byte ptr[EDI + 0xD]
            SHR EAX, 4
            MOV EDX, [EAX * 4 + g_iscript_handles]
            JMP orders_ProtossBuildSelf_1_retn
        }
    }

    const u32 orders_ProtossBuildSelf_2_retn = 0x004E50C2;
    __declspec(naked) void orders_ProtossBuildSelf_2() {
        __asm {
            MOVZX EAX, byte ptr[EDI + 0xD]
            SHR EAX, 4
            MOV EDX, [EAX * 4 + g_iscript_handles]
            JMP orders_ProtossBuildSelf_2_retn
        }
    }

    void inject_iscript_extender() {
        memory_patch(0x004D739C, (u8)0xE9);//JMP...
        memory_patch(0x004D739D, (u32)0xB1);//...0x004D7452;
        memory_patch(0x004D50C5, (u16)0x14EB);//JMP 0x004D50DB

        call_patch(init_iscript_bin,               0x004D7397, 0);	// 004D7390
        call_patch(cleanup_iscript_handles,        0x004D50C0, 0);	// 004D50C0
        jump_patch(iscript_update_speed,           0x0042D8DF, 0);	// 0042D8C0
        jump_patch(sub_42DB50,                     0x0042DC1D, 0);	// 0042DB50
        jump_patch(iscript_has_animation,          0x0042D607, 1);	// 0042D600
     // jump_patch(ground_attack_init,             0x0048869A, 1);	// 00488660
        jump_patch(is_valid_iscript,               0x004D6640, 0);	// 004D6640
        jump_patch(iscript_cases_D74DE,            0x004D74DE, 0);	// 004D74C0
        jump_patch(iscript_cases_waitrand,         0x004D7618, 1);
        jump_patch(iscript_cases_goto,             0x004D763A, 1);
        jump_patch(iscript_cases_D7BAF,            0x004D7BAF, 1);
        jump_patch(iscript_cases_end,              0x004D7BD2, 0);
        jump_patch(iscript_cases_randcondjmp,      0x004D7F4F, 1);
        jump_patch(iscript_cases_trgtrangecondjmp, 0x004D7FBA, 1);
        jump_patch(iscript_cases_trgtarccondjmp,   0x004D8042, 1);
        jump_patch(iscript_cases_call,             0x004D82C9, 1);
        jump_patch(iscript_cases_return,           0x004D82F1, 1);
        jump_patch(iscript_cases_wait,             0x004D832B, 1);
        jump_patch(iscript_cases_ignorerest,       0x004D833E, 1);
     // jump_patch(play_image_iscript,             0x004D84CD, 1);	// 004D8470
     // jump_patch(play_warp_in_overlay,           0x004D8513, 1);	// 004D8500
     // jump_patch(updateFunc_WarpFlash,           0x004D85CA, 1);	// 004D8590
     // jump_patch(orders_ProtossBuildSelf_0,      0x004E4F80, 1);	// 004E4F40
     // jump_patch(orders_ProtossBuildSelf_1,      0x004E503D, 1);
     // jump_patch(orders_ProtossBuildSelf_2,      0x004E50BC, 1);
    }
}