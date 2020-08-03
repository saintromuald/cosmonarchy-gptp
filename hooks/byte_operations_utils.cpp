#include "byte_operations_utils.h"
#include <windows.h>


namespace ByteOperations {/*
	template <typename T, typename Y>
	void SetLowByte<INT8>(T& lvalue, Y value) {
		lvalue = LOBYTE(value);
	}

	template <typename T, typename Y>
	void SetLowByte<UINT8>(T& lvalue, Y value) {
		lvalue = LOBYTE(value);
	}

	template <typename T, typename Y>
	void SetLowByte<INT16>(T& lvalue, Y value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue);
	}

	template <typename T, typename Y>
	void SetLowByte<UINT16>(T& lvalue, Y value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue);
	}

	template <typename T, typename Y>
	void SetLowByte<INT32>(T& lvalue, Y value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue) | HIWORD(lvalue);
	}

	template <typename T, typename Y>
	void SetHighByte<UINT32>(int& lvalue, Y value) {
		lvalue = HIBYTE(value) | LOBYTE(lvalue) | HIWORD(lvalue);
	}

	template <typename T, typename Y>
	void SetLowWord<INT32>(int& lvalue, Y value) {
		lvalue = LOWORD(value) | HIWORD(lvalue);
	}

	template <typename T, typename Y>
	void SetHighWord<UINT32>(int& lvalue, Y value) {
		lvalue = HIWORD(value) | LOWORD(lvalue);
	}*/
}