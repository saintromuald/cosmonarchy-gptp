#include "hook_tools.h"
#include "BW/bwdata.h"
#include "BW/bwhelper.h"

char* g_iscript_filenames[] = {
    (char*)"scripts\\iscript_general.bin",
    (char*)"scripts\\iscript_terran.bin",
    (char*)"scripts\\iscript_protoss.bin",
    (char*)"scripts\\iscript_zerg.bin",
    (char*)"scripts\\iscript_askosi.bin",
};

static constexpr u32 IscriptFileCount = sizeof(g_iscript_filenames) / 4;
static char* iscript_handles[IscriptFileCount];
static Image* temp_image;
// I normally would inline these, but there's no inlining inside declspec(naked) funcs. -DF
static __declspec(naked) Image* ebx() {
	__asm {
		MOV EAX, EBX
		RETN
	}
};
static __declspec(naked) Image* ecx() {
	__asm {
		MOV EAX, ECX
		RETN
	}
}
static __declspec(naked) Image* esi() {
	__asm {
		MOV EAX, ESI
		RETN
	}
}
static __declspec(naked) Image* edi() {
	__asm {
		MOV EAX, EDI
		RETN
	}
}
static __declspec(naked) Image* ebp() {
	__asm {
		MOV EAX, [EBP - 0xC]
		RETN
	}
}

namespace hooks {
	// Returns the pointer to the iscript file defined by the image's iscript key.
	//
	// We'd only need to pass the iscript_key value itself, but instead I'm passing
	// the Image pointer because it's simpler to work with the jump patches.
	//
	// Also not actually a hook, so pick a different namespace if so desired.
	inline char* get_iscript_file(Image* image) {
		return iscript_handles[image->iscript_key];
	}

	void init_iscript_bin() {
		for (u32 index = 0; index < IscriptFileCount; ++index) {
			HANDLE* file = nullptr;
			u32 read = 0;

			if (!helper::sfile_open_file_ex(nullptr, g_iscript_filenames[index], 0, &file)) {
				helper::syswarn_file_not_found(helper::serr_get_last_error(), g_iscript_filenames[index]);
			}

			u32 file_size = helper::SFileGetFileSize(file, 0);

			if (file_size == -1) {
				helper::file_fatal(file, GetLastError());
			}

			if (!file_size) {
				helper::syswarn_file_not_found(24, g_iscript_filenames[index]);
			}

			char* item = (char*)helper::SMemAlloc(file_size, "Starcraft\\SWAR\\lang\\gamedata.cpp", 210, 0);

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
			iscript_handles[index] = item;
		}

		g_iscript_data = iscript_handles[0];
	}

	void cleanup_iscript_handles() {
		for (u32 index = 0; index < IscriptFileCount; ++index) {
			if (iscript_handles[index]) {
				helper::SMemFree(iscript_handles[index], "Starcraft\\SWAR\\lang\\CImage.cpp", 1626, 0);
				iscript_handles[index] = nullptr;
			}
		}
	}

	void __declspec(naked) iscript_update_speed() {
		static const u32 iscript_update_speed_retn = 0x0042D8E4;
		__asm {
			MOV EDX, [EAX + 0x14]
			MOV temp_image, EAX
			MOV AL, 11
			JMP iscript_update_speed_retn
		}
	}

	void __declspec(naked) sub_42DB50() {
		static const u32 sub_42DB50_retn = 0x0042DC22;
		__asm {
			MOV ds : [0x006C2314] , EAX//g_temp_iscript_return
			MOV temp_image, ECX
			JMP sub_42DB50_retn
		}
	}

	void __declspec(naked) iscript_has_animation() {
		static const u32 iscript_has_animation_retn = 0x0042D60D;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(temp_image);

		__asm {
			POPAD
			MOV EDX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_has_animation_retn
		}
	}

	void __declspec(naked) ground_attack_init() {
		static const u32 ground_attack_init_retn = 0x004886A0;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(esi());

		__asm {
			POPAD
			MOV EDX, ds: [0x006D1200]//g_iscript_data
			JMP ground_attack_init_retn
		}
	}

	void __declspec(naked) is_valid_iscript() {
		static const u32 is_valid_iscript_retn = 0x004D6645;
		__asm {
			MOV EAX, EDI
			SHR EAX, 12				// u32 iscript_key = imagesdat_iscript_id[image->id] >> 12;
			MOVZX EDI, DI			// u32 iscript_id = (u16)imagesdat_iscript_id[image->id];
			AND[EBX + 0xD], 0xF	// image->flags &= 0x0FFF;
			OR[EBX + 0xD], AL		// image->flags |= iscript_key;
			PUSHAD
		}

		g_iscript_data = get_iscript_file(ebx());

		__asm {
			POPAD
			MOV EAX, ds: [0x006D1200]//g_iscript_data
			JMP is_valid_iscript_retn
		}
	}

	void __declspec(naked) iscript_cases_D74DE() {
		static const u32 iscript_cases_D74DE_retn = 0x004D74E3;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			MOV EAX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_D74DE_retn
		}
	}

	void __declspec(naked) iscript_cases_waitrand() {
		static const u32 iscript_cases_waitrand_retn = 0x004D761E;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			MOV ECX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_waitrand_retn
		}
	}

	void __declspec(naked) iscript_cases_goto() {
		static const u32 iscript_cases_goto_retn = 0x004D7640;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			ADD EAX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_goto_retn
		}
	}

	void __declspec(naked) iscript_cases_D7BAF() {
		static const u32 iscript_cases_D7BAF_retn = 0x004D7BB5;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			ADD EAX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_D7BAF_retn
		}
	}

	void __declspec(naked) iscript_cases_end() {
		static const u32 iscript_cases_end_retn = 0x004D7BD7;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(esi());

		__asm {
			POPAD
			MOV EAX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_end_retn
		}
	}

	void __declspec(naked) iscript_cases_randcondjmp() {
		static const u32 iscript_cases_randcondjmp_retn = 0x004D7F55;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			MOV ECX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_randcondjmp_retn
		}
	}

	void __declspec(naked) iscript_cases_trgtrangecondjmp() {
		static const u32 iscript_cases_trgtrangecondjmp_retn = 0x004D7FC0;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			MOV ECX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_trgtrangecondjmp_retn
		}
	}

	void __declspec(naked) iscript_cases_trgtarccondjmp() {
		static const u32 iscript_cases_trgtarccondjmp_retn = 0x004D8048;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			ADD EDX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_trgtarccondjmp_retn
		}
	}

	void __declspec(naked) iscript_cases_call() {
		static const u32 iscript_cases_call_retn = 0x004D82CF;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			MOV ECX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_call_retn
		}
	}

	void __declspec(naked) iscript_cases_return() {
		static const u32 iscript_cases_return_retn = 0x004D82F7;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			ADD EAX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_return_retn
		}
	}

	void __declspec(naked) iscript_cases_wait() {
		static const u32 iscript_cases_wait_retn = 0x004D8331;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			SUB EDI, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_wait_retn
		}
	}

	void __declspec(naked) iscript_cases_ignorerest() {
		static const u32 iscript_cases_ignorerest_retn = 0x004D8344;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebp());

		__asm {
			POPAD
			MOV EDX, ds: [0x006D1200]//g_iscript_data
			JMP iscript_cases_ignorerest_retn
		}
	}

	void __declspec(naked) play_image_iscript() {
		static const u32 play_image_iscript_retn = 0x004D84D3;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ecx());

		__asm {
			POPAD
			MOV EDI, ds: [0x006D1200]//g_iscript_data
			JMP play_image_iscript_retn
		}
	}

	void __declspec(naked) play_warp_in_overlay() {
		static const u32 play_warp_in_overlay_retn = 0x004D8519;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebx());

		__asm {
			POPAD
			MOV EDX, ds: [0x006D1200]//g_iscript_data
			JMP play_warp_in_overlay_retn
		}
	}

	void __declspec(naked) updateFunc_WarpFlash() {
		static const u32 updateFunc_WarpFlash_retn = 0x004D85D0;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(ebx());

		__asm {
			POPAD
			MOV EDI, ds: [0x006D1200]//g_iscript_data
			JMP updateFunc_WarpFlash_retn
		}
	}

	void __declspec(naked) orders_ProtossBuildSelf_0() {
		static const u32 orders_ProtossBuildSelf_0_retn = 0x004E4F86;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(edi());

		__asm {
			POPAD
			MOV EDX, ds: [0x006D1200]//g_iscript_data
			JMP orders_ProtossBuildSelf_0_retn
		}
	}

	void __declspec(naked) orders_ProtossBuildSelf_1() {
		static const u32 orders_ProtossBuildSelf_1_retn = 0x004E5043;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(edi());

		__asm {
			POPAD
			MOV EDX, ds: [0x006D1200]//g_iscript_data
			JMP orders_ProtossBuildSelf_1_retn
		}
	}

	void __declspec(naked) orders_ProtossBuildSelf_2() {
		static const u32 orders_ProtossBuildSelf_2_retn = 0x004E50C2;
		__asm PUSHAD
		g_iscript_data = get_iscript_file(edi());

		__asm {
			POPAD
			MOV EDX, ds: [0x006D1200]//g_iscript_data
			JMP orders_ProtossBuildSelf_2_retn
		}
	}

	void inject_iscript_extender() {
		memory_patch(0x004D739C, (u8)0xE9);//JMP...
		memory_patch(0x004D739D, (u32)0xB1);//...0x004D7452;
		memory_patch(0x004D50C5, (u16)0x14EB);//JMP 0x004D50DB
		nop_patch(0x004D745C, 6);

		call_patch(init_iscript_bin, 0x004D7397, 0);	// 004D7390
		call_patch(cleanup_iscript_handles, 0x004D50C0, 0);	// 004D50C0
		jump_patch(iscript_update_speed, 0x0042D8DF, 0);	// 0042D8C0
		jump_patch(sub_42DB50, 0x0042DC1D, 0);	// 0042DB50
		jump_patch(iscript_has_animation, 0x0042D607, 1);	// 0042D600
		//	jump_patch(ground_attack_init,				0x0048869A, 1);	// 00488660
		jump_patch(is_valid_iscript, 0x004D6640, 0);	// 004D6640
		jump_patch(iscript_cases_D74DE, 0x004D74DE, 0);	// 004D74C0
		jump_patch(iscript_cases_waitrand, 0x004D7618, 1);
		jump_patch(iscript_cases_goto, 0x004D763A, 1);
		jump_patch(iscript_cases_D7BAF, 0x004D7BAF, 1);
		jump_patch(iscript_cases_end, 0x004D7BD2, 0);
		jump_patch(iscript_cases_randcondjmp, 0x004D7F4F, 1);
		jump_patch(iscript_cases_trgtrangecondjmp, 0x004D7FBA, 1);
		jump_patch(iscript_cases_trgtarccondjmp, 0x004D8042, 1);
		jump_patch(iscript_cases_call, 0x004D82C9, 1);
		jump_patch(iscript_cases_return, 0x004D82F1, 1);
		jump_patch(iscript_cases_wait, 0x004D832B, 1);
		jump_patch(iscript_cases_ignorerest, 0x004D833E, 1);
		//	jump_patch(play_image_iscript,				0x004D84CD, 1);	// 004D8470
		//	jump_patch(play_warp_in_overlay,			0x004D8513, 1);	// 004D8500
		//	jump_patch(updateFunc_WarpFlash,			0x004D85CA, 1);	// 004D8590
		//	jump_patch(orders_ProtossBuildSelf_0,		0x004E4F80, 1);	// 004E4F40
		//	jump_patch(orders_ProtossBuildSelf_1,		0x004E503D, 1);
		//	jump_patch(orders_ProtossBuildSelf_2,		0x004E50BC, 1);
	}
}
