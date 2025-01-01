#pragma once
#include "BW/api.h"
#include "BW/bwdata.h"

namespace hooks
{
	/*
	s32 soundThread(s32 edi0, s32 a2);			// 004BC620
	s32 soundInit(s32 a1, s32 a2);				// 004BCB70
	*/

	void inject_direct_sound_hooks();
}