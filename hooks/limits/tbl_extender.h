#pragma once
#include "hooks\limits\extender.h"



/*class tblExtender {
public:
	u32* buffer;

	u32 getStringCount() {
		return *buffer;
	}
	const char* getString(u32 index) {
		if (index == 0) return 0;
		else if (index <= getStringCount())
			return (const char*)(buffer)+buffer[index];
		else
			return (const char*)0x00501B7D;
	}

};*/
namespace hooks {
	void injectTblExtender();
}
//static u32
static char tblNew[] = "rez\\stat_txt.tblex";
//extern tblExtender extendedTbl;
