#include "byte_operations_utils.h"


namespace ByteOperations {
	INT8& SetLowByte(INT8& lvalue, INT8 value) {
		lvalue = LOBYTE(value);
		return lvalue;
	}

	UINT8& SetLowByte(UINT8& lvalue, UINT8 value) {
		lvalue = LOBYTE(value);
		return lvalue;
	}

	INT16& SetLowByte(INT16& lvalue, INT8 value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue);
		return lvalue;
	}

	UINT16& SetHighByte(UINT16& lvalue, UINT8 value) {
		lvalue = HIBYTE(value) | LOBYTE(lvalue);
		return lvalue;
	}

	INT32& SetLowByte(INT32& lvalue, INT8 value) {
		lvalue = LOBYTE(value) | HIBYTE(lvalue) | HIWORD(lvalue);
		return lvalue;
	}

	UINT32& SetHighByte(UINT32& lvalue, UINT8 value) {
		lvalue = HIBYTE(value) | LOBYTE(lvalue) | HIWORD(lvalue);
		return lvalue;
	}

	INT32& SetLowWord(INT32& lvalue, INT16 value) {
		lvalue = LOWORD(value) | HIWORD(lvalue);
		return lvalue;
	}

	UINT32& SetHighWord(UINT32& lvalue, UINT16 value) {
		lvalue = HIWORD(value) | LOWORD(lvalue);
		return lvalue;
	}
}