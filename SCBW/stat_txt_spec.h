#pragma once
#include "globals.h"

class StatTxtStringTbl {
public:
	uint16 getStringCount() const {
		return *buffer;
	}
	uint32 getStringCountExt() const {
		return *extendedBuffer;
	}
	const char* getString(uint32 index) const {
		if (Globals.tbl_extender) {
			if (index == 0) return nullptr;
			else if (index <= getStringCountExt())
				return (const char*)(extendedBuffer)+extendedBuffer[index];
			else
				return (const char*)0x00501B7D;
		}
		else {
			if ((uint16)index == 0) return nullptr;
			else if ((uint16)index <= getStringCount())
				return (const char*)(buffer)+buffer[(uint16)index];
			else
				return (const char*)0x00501B7D;
		}
	}
private:
	union {
		uint16* buffer;
		uint32* extendedBuffer;
	};
};
const StatTxtStringTbl* const statTxtTblNew = (const StatTxtStringTbl*)0x006D1238;