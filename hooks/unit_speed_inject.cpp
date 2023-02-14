#include "unit_speed.h"
#include "../hook_tools.h"

namespace
{
	void __declspec(naked) get_modified_unit_speed_wrapper()
	{
		static Unit* unit;
		static u32 speed;
		__asm
		{
			MOV unit, EDX
			MOV speed, EAX
			PUSHAD
		}
		speed = hooks::get_modified_unit_speed(unit, speed);
		__asm
		{
			POPAD
			MOV EAX, speed
			RETN
		}
	}

	void __declspec(naked) get_modified_unit_acceleration_wrapper()
	{
		static Unit* unit;
		static u32 acceleration;
		__asm
		{
			MOV unit, ECX
			PUSHAD
		}
		acceleration = hooks::get_modified_unit_acceleration(unit);
		__asm
		{
			POPAD
			MOV EAX, acceleration
			RETN
		}
	}

	void __declspec(naked) get_modified_unit_turn_speed_wrapper()
	{
		static Unit* unit;
		static u32 turnSpeed;
		__asm
		{
			MOV unit, ECX
			PUSHAD
		}
		turnSpeed = hooks::get_modified_unit_turn_speed(unit);
		__asm
		{
			POPAD
			MOV EAX, turnSpeed
			RETN
		}
	}

	void __declspec(naked) set_unit_movement_speed_wrapper() // 004951C0
	{
		static Unit* result;
		static Unit* unit;
		static u32 speed;
		__asm
		{
			MOV speed, EDX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::set_unit_movement_speed(unit, speed);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) update_iscript_speed_wrapper() // 004951C0
	{
		static Unit* unit;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		hooks::update_iscript_speed(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) get_flingy_halt_distance_wrapper() // 00494F90
	{
		static s32 result;
		static Unit* unit;
		__asm
		{
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::get_flingy_halt_distance(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_unit_speed_hooks()
	{
		jump_patch(get_modified_unit_speed_wrapper,			0x0047B5F0, 2);
		jump_patch(get_modified_unit_acceleration_wrapper,	0x0047B8A0, 6);
		jump_patch(get_modified_unit_turn_speed_wrapper,	0x0047B850, 6);
	//	jump_patch(set_unit_movement_speed_wrapper,			0x004951C0, 1); // Causes psycho spinning!
	//	jump_patch(update_iscript_speed_wrapper,			0x0042D8C0, 1);
		jump_patch(get_flingy_halt_distance_wrapper,		0x00494F90, 0);
	}
}