#include "byte_operations_utils.h"
#include <windows.h>


namespace ByteOperations {
	template <typename Y>
	INT8& SetLowByte<INT8, Y>(INT8& lvalue, Y value) {
		lvalue = LOBYTE(value);
		return lvalue;
	}

	template <typename Y>
	UINT8& SetLowByte<UINT8, Y>(UINT8& lvalue, Y value) {
		lvalue = LOBYTE(value);
		return lvalue;
	}

	template <typename Y>
	INT16& SetLowByte<INT16, Y>(INT16& lvalue, Y value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue);
		return lvalue;
	}

	template <typename Y>
	UINT16& SetLowByte<UINT16, Y>(UINT16& lvalue, Y value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue);
		return lvalue;
	}

	template <typename Y>
	INT32& SetLowByte<INT32, Y>(INT32& lvalue, Y value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue) | HIWORD(lvalue);
		return lvalue;
	}

	template <typename Y>
	UINT32& SetHighByte<UINT32, Y>(UINT32& lvalue, Y value) {
		lvalue = HIBYTE(value) | LOBYTE(lvalue) | HIWORD(lvalue);
		return lvalue;
	}

	template <typename Y>
	INT32& SetLowWord<INT32, Y>(INT32& lvalue, Y value) {
		lvalue = LOWORD(value) | HIWORD(lvalue);
		return lvalue;
	}

	template <typename Y>
	UINT32& SetHighWord<UINT32, Y>(UINT32& lvalue, Y value) {
		lvalue = HIWORD(value) | LOWORD(lvalue);
		return lvalue;
	}
}