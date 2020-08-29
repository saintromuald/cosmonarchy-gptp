#pragma once
#include <windows.h>
#include "types.h"

namespace ByteOperations {
	s8& SetLowByte(s8& lvalue, s8 value);
	u8& SetLowByte(u8& lvalue, u8 value);

	s16& SetLowByte(s16& lvalue, s8 value);
	u16& SetHighByte(s16& lvalue, u8 value);

	s32& SetLowByte(s32& lvalue, s8 value);
	u32& SetHighByte(s32& lvalue, u8 value);

	s32& SetLowWord(s32& lvalue, s16 value);
	u32& SetHighWord(s32& lvalue, u16 value);
}