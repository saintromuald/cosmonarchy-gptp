#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks
{
    bool openMapFile(s32 vector);							// 0045A290
    s32 loadMap(s32 a1, char* a2, s32 a3);					// 004BF520
    void injectFileOperationsHooks();

} //hooks