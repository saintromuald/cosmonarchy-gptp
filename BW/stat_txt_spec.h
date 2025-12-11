#pragma once
#include "globals.h"

struct StatTxtStringTbl {
	u16 get_string_count() const {
		return *buffer;
	}
	u32 get_string_count_ext() const {
		return *extended_buffer;
	}
	const char* get_string(u32 index) const {
        if (index == 0) {
            return nullptr;
        } else if (index <= get_string_count()) {
            return (const char*)(buffer)+buffer[index];
        } else {
            return (const char*)0x00501B7D;
        }
	}
	union {
		u16* buffer;
		u32* extended_buffer;
	};
};
const StatTxtStringTbl* const g_stat_txt_tbl_new = (const StatTxtStringTbl*)0x006D1238;