#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks {
	bool openMapFile(s32 vector);							// 0045A290
	void injectFileOperationsHooks();

} //hooks