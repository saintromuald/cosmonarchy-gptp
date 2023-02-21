#include "lobby.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) init_game_wrapper() // 004EE110
	{
		static s32 result;
		__asm
		{
			PUSHAD
		}
		result = hooks::init_game();
		__asm
		{
			POPAD
			MOV result, EAX
			RETN
		}
	}

	void __declspec(naked) start_game_wrapper() // 00472060
	{
		static s32 result;
		static void* lobby_state;
		static s32 player;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			POP EAX
			MOV lobby_state, ECX
			PUSHAD
		}
		result = hooks::start_game(lobby_state, player);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{

	void inject_lobby_hooks()
	{
		jump_patch(init_game_wrapper,	0x004EE110, 0);
		jump_patch(start_game_wrapper,	0x00472060, 4);
	}
}