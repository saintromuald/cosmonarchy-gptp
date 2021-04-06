#pragma once
#include <windows.h>
#include "types.h"

class ByteOperations {
private:
	static inline uint16 m_LOWORD(uint32 value) {
		return ((uint16)(((uint32)(value)) & 0xffff));
	}

	static inline uint16 m_HIWORD(uint32 value) {
		return ((uint16)((((uint32)(value)) >> 16) & 0xffff));
	}

	static inline uint8 m_LOBYTE(uint32 value) {
		return ((uint8)(((uint32)(value)) & 0xff));
	}

	static inline uint8 m_HIBYTE(uint32 value) {
		return ((uint8)((((uint32)(value)) >> 8) & 0xff));
	}

public:
	template<typename T, typename Y>
	static inline T& SetLowByte(T& lvalue, Y value) {
		lvalue = m_LOBYTE((uint32)value);
		return lvalue;
	}

	template<typename T, typename Y>
	static inline T& SetHighByte(T& lvalue, Y value) {
		lvalue = m_HIBYTE((uint32)value);
		return lvalue;
	}

	template<typename T, typename Y>
	static inline T& SetLowWord(T& lvalue, Y value) {
		lvalue = m_LOWORD((uint32)value) | m_HIWORD((uint32)lvalue);
		return lvalue;
	}

	template<typename T, typename Y>
	static inline T& SetHighWord(T& lvalue, Y value) {
		lvalue = m_HIWORD((uint32)value) | m_LOWORD((uint32)lvalue);
		return lvalue;
	}
};