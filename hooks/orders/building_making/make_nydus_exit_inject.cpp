#include "make_nydus_exit.h"
#include <hook_tools.h>
#include "globals.h"

static u32 playerCache = 0; // not the best solution of transfusion case, it's temporary until hook is available

namespace
{
	void __declspec(naked) orders_Build5Wrapper() // 0045DC20
	{
		static Unit* nydus_canal;
		__asm
		{
			MOV nydus_canal, EBX
			PUSHAD
		}
		hooks::orders_Build5(nydus_canal);
		__asm
		{
			POPAD
			RETN
		}
	}

	const u32 jmp_7315c = 0x0047315C;
	void __declspec(naked) nydusPlaceExitTileCheckWrapper1()
	{
		__asm
		{
			MOVZX EAX, AL
			LEA ECX, DWORD PTR[EAX + EAX * 0x8]
			MOV playerCache, EAX
			JMP jmp_7315c
		}
	}

	const u32 jmp_731C8 = 0x004731C8;
	void __declspec(naked) nydusPlaceExitTileCheckWrapper2()
	{
		static u32 tile_flags;
		static u32 player_id;
		__asm
		{
			ADD ECX, 0x00400000
			MOV tile_flags, ECX
			PUSHAD
		}
		// player id must be moved from somewhere else
		tile_flags = hooks::adjustFlags(tile_flags, playerCache);
		__asm
		{
			POPAD
			MOV ECX, tile_flags
			JMP jmp_731C8
		}
		// ecx is tile_flags, ebx is flags in question, [ebp+8] is player id, required for nydus transfusion
	}
}

namespace hooks
{
	void inject_make_nydus_exit_hooks()
	{
		jump_patch(orders_Build5Wrapper,			0x0045DC20, 2);
//		jump_patch(nydusPlaceExitTileCheckWrapper1,	0x00473156, 1);
//		jump_patch(nydusPlaceExitTileCheckWrapper2,	0x004731C2, 1);
	}
}