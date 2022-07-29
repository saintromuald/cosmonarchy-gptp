#include "hpbars.h"
#include "hook_tools.h"

namespace
{
	void _declspec(naked) compile_healthbar_wrapper()
	{
		static Image* image;
		static Sprite* sprite;
		_asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV sprite, EAX
			POP EAX
			MOV image, ESI
			PUSHAD
		}
		hooks::compile_healthbar(image, sprite);
		_asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}
}

namespace hooks
{
	void inject_hp_bars_hooks()
	{
	//	jump_patch(compile_healthbar_wrapper, 0x004D6010, 1); // Crashes
	}
}