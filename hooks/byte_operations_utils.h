#pragma once
#include <windows.h>

namespace ByteOperations {
	extern INT8& SetLowByte(INT8& lvalue, INT8 value);
	extern UINT8& SetLowByte(UINT8& lvalue, UINT8 value);

	extern INT16& SetLowByte(INT16& lvalue, INT8 value);
	extern UINT16& SetHighByte(UINT16& lvalue, UINT8 value);

	extern INT32& SetLowByte(INT32& lvalue, INT8 value);
	extern UINT32& SetHighByte(UINT32& lvalue, UINT8 value);

	extern INT32& SetLowWord(INT32& lvalue, INT16 value);
	extern UINT32& SetHighWord(UINT32& lvalue, UINT16 value);
}