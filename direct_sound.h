#pragma once
#include "SCBW/api.h"
#include "SCBW/scbwdata.h"

namespace hooks
{
	s32 soundThread(s32 edi0, s32 a2);	// 004BC620
	s32 soundInit(s32 a1, s32 a2);		// 004BCB70

	void injectDirectSoundHooks();
}