#pragma once

namespace ByteOperations {
	template <typename T, typename Y>
	extern T& SetLowByte(T& lvalue, Y value);

	template <typename T, typename Y>
	extern T& SetHighByte(T& lvalue, Y value);

	template <typename T, typename Y>
	extern T& SetLowWord(T& lvalue, Y value);

	template <typename T, typename Y>
	extern T& SetHighWord(T& lvalue, Y value);
}