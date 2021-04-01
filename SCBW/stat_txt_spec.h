#pragma once
#include "globals.h"

class StatTxtStringTbl {
public:
	u16 getStringCount() const {
		return *buffer;
	}
	u32 getStringCountExt() const {
		return *extendedBuffer;
	}
	const char* getString(u32 index) const {
		if (Globals.tbl_extender) {
			if (index == 0) return nullptr;
			else if (index <= getStringCountExt())
				return (const char*)(extendedBuffer)+extendedBuffer[index];
			else
				return (const char*)0x00501B7D;
		}
		else {
			if ((u16)index == 0) return nullptr;
			else if ((u16)index <= getStringCount())
				return (const char*)(buffer)+buffer[(u16)index];
			else
				return (const char*)0x00501B7D;
		}
	}
private:
	union {
		u16* buffer;
		u32* extendedBuffer;
	};
};
const StatTxtStringTbl* const statTxtTblNew = (const StatTxtStringTbl*)0x006D1238;