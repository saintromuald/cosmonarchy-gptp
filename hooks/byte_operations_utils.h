#pragma once

namespace ByteOperations {
	template <typename T, typename Y>
	extern void SetLowByte(T& lvalue, Y value);

	template <typename T, typename Y>
	extern void SetHighByte(T& lvalue, Y value);

	template <typename T, typename Y>
	extern void SetLowWord(T& lvalue, Y value);

	template <typename T, typename Y>
	extern void SetHighWord(T& lvalue, Y value);
}