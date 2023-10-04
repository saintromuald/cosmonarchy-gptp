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

	void __declspec(naked) init_singleplayer_custom_wrapper() // 004AD850
	{
		static Player* result;
		static BinDlg* dialog;
		__asm
		{
			MOV dialog, EAX
			PUSHAD
		}
		result = hooks::init_singleplayer_custom(dialog);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) on_player_join_wrapper() // 00470720
	{
		static u32 player_id;
		static BinDlg* dialog;
		static u32 data_1;
		static u32 data_2;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV data_2, EAX
			POP EAX
			MOV data_1, ESI
			MOV dialog, EDI
			MOV player_id, EAX
			PUSHAD
		}
		hooks::on_player_join(player_id, dialog, data_1, data_2);
		__asm
		{
			POPAD
			MOV EAX, 0
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	// PATCHES \\

	const u32 DLGMusicFade = 0x004A6310;
	const u32 loadMenu_gluJoin_B86CB = 0x004B86CB;
	void __declspec(naked) loadMenu_gluJoin_patch_B86C6() {
		__asm {
			MOV		AL, ds: [0x006D5BBC]		// musicPlaying
			TEST	AL, AL
			JNE		offset music_off
			MOV		EAX, 22						// music_title_wav
			CALL	DLGMusicFade

		music_off:
			MOV		EAX, ds : [0x006D0F24]		// chkFile
			JMP		loadMenu_gluJoin_B86CB
		}
	}
}

namespace hooks
{

	void inject_lobby_hooks()
	{
		jump_patch(init_game_wrapper,					0x004EE110, 0);
		jump_patch(start_game_wrapper,					0x00472060, 4);
	//	jump_patch(init_singleplayer_custom_wrapper,	0x004AD850, 0);
		jump_patch(on_player_join_wrapper,				0x00470720, 4);
		jump_patch(loadMenu_gluJoin_patch_B86C6,		0x004B86C6);
	}
}