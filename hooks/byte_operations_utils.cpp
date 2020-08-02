#include "byte_operations_utils.h"
#include <windows.h>


namespace ByteOperations {
	template <typename T, typename Y>
	void SetLowByte<INT16>(T& lvalue, Y value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue);
	}
	template <typename T, typename Y>
	void SetLowByte<UINT16>(T& lvalue, Y value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue);
	}

	template <typename T, typename Y>
	void SetLowByte(T& lvalue, Y value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue) | HIWORD(lvalue);
	}

	template <typename T, typename Y>
	void SetHighByte(int& lvalue, Y value) {
		lvalue = HIBYTE(value) | LOBYTE(lvalue) | HIWORD(lvalue);
	}

	template <typename T, typename Y>
	void SetLowWord(int& lvalue, Y value) {
		lvalue = LOWORD(value) | HIWORD(lvalue);
	}

	template <typename T, typename Y>
	void SetHighWord(int& lvalue, Y value) {
		lvalue = HIWORD(value) | LOWORD(lvalue);
	}
}