#include "byte_operations_utils.h"


namespace ByteOperations {
	s8& SetLowByte(s8& lvalue, s8 value) {
		lvalue = LOBYTE(value);
		return lvalue;
	}

	u8& SetLowByte(u8& lvalue, u8 value) {
		lvalue = LOBYTE(value);
		return lvalue;
	}

	s16& SetLowByte(s16& lvalue, s8 value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue);
		return lvalue;
	}

	u16& SetHighByte(u16& lvalue, u8 value) {
		lvalue = HIBYTE(value) | LOBYTE(lvalue);
		return lvalue;
	}

	s32& SetLowByte(s32& lvalue, s8 value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue) | HIWORD(lvalue);
		return lvalue;
	}

	u32& SetHighByte(u32& lvalue, u8 value) {
		lvalue = HIBYTE(value) | LOBYTE(lvalue) | HIWORD(lvalue);
		return lvalue;
	}

	s32& SetLowWord(s32& lvalue, s16 value) {
		lvalue = LOWORD(value) | HIWORD(lvalue);
		return lvalue;
	}

	u32& SetHighWord(u32& lvalue, u16 value) {
		lvalue = HIWORD(value) | LOWORD(lvalue);
		return lvalue;
	}
}