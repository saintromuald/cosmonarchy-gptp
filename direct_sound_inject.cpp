#include "direct_sound.h"
#include <hook_tools.h>

namespace
{
	/*
	void __declspec(naked) soundThread_Wrapper() // 004BC620
	{
		static s32 edi0;
		static s32 a2;
		static s32 result;
		__asm
		{
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV a2, EAX
			POP EAX
			MOV edi0, EDI
			PUSHAD
		}
		result = hooks::soundThread(edi0, a2);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) soundInit_Wrapper() // 004BCB70
	{
		static s32 a1;
		static s32 a2;
		static s32 result;
		__asm
		{
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV a2, EAX
			POP EAX
			MOV a1, EAX
			PUSHAD
		}
		result = hooks::soundInit(a1, a2);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
	*/
}

namespace hooks
{
	void injectDirectSoundSub()
	{
	//	jump_patch(soundThread_Wrapper,		0x004BC620, 4);
	//	jump_patch(soundInit_Wrapper,		0x004BCB70, 1);
	}

	void injectDirectSoundHooks()
	{
		injectDirectSoundSub();
	}
}