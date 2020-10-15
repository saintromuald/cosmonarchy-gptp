#pragma once
#include <windows.h>
#include "types.h"

namespace ByteOperations {
	template<typename T, typename Y>
	T& SetLowByte(T& lvalue, Y value) {
		lvalue = LOBYTE(value);
		return lvalue;
	}

	template<typename T, typename Y>
	T& SetHighByte(T& lvalue, Y value) {
		lvalue = HIBYTE(value);
		return lvalue;
	}

	template<typename T, typename Y>
	T& SetLowWord(T& lvalue, Y value) {
		lvalue = LOWORD(value) | HIWORD(lvalue);
		return lvalue;
	}

	template<typename T, typename Y>
	T& SetHighWord(T& lvalue, Y value) {
		lvalue = HIWORD(value) | LOWORD(lvalue);
		return lvalue;
	}
}