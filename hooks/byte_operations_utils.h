#pragma once
#include <windows.h>
#include "types.h"

class ByteOperations
{
private:
	static inline u16 m_LOWORD(u32 value)
	{
		return ((u16)(((u32)(value)) & 0xffff));
	}

	static inline u16 m_HIWORD(u32 value)
	{
		return ((u16)((((u32)(value)) >> 16) & 0xffff));
	}

	static inline u8 m_LOBYTE(u32 value)
	{
		return ((u8)(((u32)(value)) & 0xff));
	}

	static inline u8 m_HIBYTE(u32 value)
	{
		return ((u8)((((u32)(value)) >> 8) & 0xff));
	}

public:
	template<typename T, typename Y>
	static inline T& SetLowByte(T& lvalue, Y value)
	{
		lvalue = m_LOBYTE((u32)value);
		return lvalue;
	}

	template<typename T, typename Y>
	static inline T& SetHighByte(T& lvalue, Y value)
	{
		lvalue = m_HIBYTE((u32)value);
		return lvalue;
	}

	template<typename T, typename Y>
	static inline T& SetLowWord(T& lvalue, Y value)
	{
		lvalue = m_LOWORD((u32)value) | m_HIWORD((u32)lvalue);
		return lvalue;
	}

	template<typename T, typename Y>
	static inline T& SetHighWord(T& lvalue, Y value)
	{
		lvalue = m_HIWORD((u32)value) | m_LOWORD((u32)lvalue);
		return lvalue;
	}
};