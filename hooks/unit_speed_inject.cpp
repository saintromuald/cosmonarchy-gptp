#include "unit_speed.h"
#include "../hook_tools.h"

namespace
{
	void __declspec(naked) getModifiedUnitSpeedWrapper()
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

	void __declspec(naked) getModifiedUnitAccelerationWrapper()
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

	void __declspec(naked) getModifiedUnitTurnSpeedWrapper()
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
}

namespace hooks
{
	void inject_unit_speed_hooks()
	{
		jump_patch(getModifiedUnitSpeedWrapper,			0x0047B5F0, 2);
		jump_patch(getModifiedUnitAccelerationWrapper,	0x0047B8A0, 6);
		jump_patch(getModifiedUnitTurnSpeedWrapper,		0x0047B850, 6);
	}
}
