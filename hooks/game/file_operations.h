#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks {
	bool openMapFile(int32 vector);							// 0045A290
	int32 loadMap(int32 a1, char* a2, int32 a3);					// 004BF520
	void injectFileOperationsHooks();

} //hooks