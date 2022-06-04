#pragma once
#include "hooks\limits\extender.h"



/*struct tblExtender {
	u32* buffer;

	u32 get_string_count() {
		return *buffer;
	}
	const char* get_string(u32 index) {
		if (index == 0) return 0;
		else if (index <= get_string_count())
			return (const char*)(buffer)+buffer[index];
		else
			return (const char*)0x00501B7D;
	}

};*/
namespace hooks
{
	void injectTblExtender();
}
//static u32
static char tblNew[] = "rez\\stat_txt.tblex";
//extern tblExtender extendedTbl;
