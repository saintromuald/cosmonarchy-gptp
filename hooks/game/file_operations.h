#pragma once
#include "../BW/structures/Unit.h"

namespace hooks
{
	bool open_map_file(s32 vector);							// 0045A290
	s32 load_map(s32 a1, char* a2, s32 a3);					// 004BF520
	void injectFileOperationsHooks();

} //hooks