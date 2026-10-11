#include "hpbars.h"
#include "hook_tools.h"

namespace
{
	void _declspec(naked) compile_healthbar_wrapper() // 004D6010
	{
		static Image* image;
		static Sprite* sprite;
		_asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
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

	void __declspec(naked) remove_healthbar_wrapper() // 004D5030
	{
		static Image* image;
		__asm {
			MOV image, ESI
			PUSHAD
		}
		hooks::remove_healthbar(image);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) create_healthbar_wrapper() // 004D6420
	{
		static Image* result;
		static u32 unused;
		static Sprite* sprite;
		__asm {
			MOV unused, ECX
			MOV sprite, EDX
			PUSHAD
		}
		result = hooks::create_healthbar(unused, sprite);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_hp_bars_hooks()
	{
	//	jump_patch(compile_healthbar_wrapper,	0x004D6010, 1);	// Something about the hpbar placement doesn't take into account the unit's size correctly.
		jump_patch(remove_healthbar_wrapper,	0x004D5030, 2);
		jump_patch(create_healthbar_wrapper,	0x004D6420, 2); // Causes a crash after returning the value, but wrapper seems correct...
	}
}