#pragma once
#include "hooks/limits/extender.h"

static u32 NEW_BULLET_COUNT = 100;
//original count is 100

//BG
extern u32* PORTDAT_IdleDir;
extern u32* PORTDAT_TalkingDir;
extern u8* PORTDAT_IdleSMKChange;
extern u8* PORTDAT_TalkingSMKChange;
extern u8* PORTDAT_IdleUnknown1;
extern u8* PORTDAT_TalkingUnknown1;

namespace hooks
{
	void injectBulletLimits();
}