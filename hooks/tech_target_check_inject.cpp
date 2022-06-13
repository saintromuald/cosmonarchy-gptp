#include "tech_target_check.h"
#include <hook_tools.h>

namespace
{
	const u32 Func_GetTechUseErrorMessage = 0x00491E80;
	void __declspec(naked) get_tech_use_error_message_hook_wrapper() // 00491E80
	{
		static Unit* target;
		static u8 castingPlayer;
		static u16 techId;
		static u32 errorMessage;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV target, EAX
			MOV castingPlayer, BL
			MOV EAX, [EBP + 0x08]
			MOV techId, AX
			PUSHAD
		}
		errorMessage = hooks::get_tech_use_error_message_hook(target, castingPlayer, techId);
		__asm {
			POPAD
			MOV EAX, errorMessage
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void inject_tech_target_check_hooks()
	{
		jump_patch(get_tech_use_error_message_hook_wrapper, Func_GetTechUseErrorMessage, 4);
	}
}