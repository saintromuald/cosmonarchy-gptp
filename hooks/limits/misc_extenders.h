#pragma once
#include "hooks/limits/extender.h"

static uint32 NEW_BULLET_COUNT = 100;
//original count is 100

static uint32 NEW_PORTDATA_COUNT = 110;
//static uint32 NEW_PORTDATA_COUNT = 9999;
//original count is 110

//BG
extern uint32* PORTDAT_IdleDir;
extern uint32* PORTDAT_TalkingDir;
extern uint8* PORTDAT_IdleSMKChange;
extern uint8* PORTDAT_TalkingSMKChange;
extern uint8* PORTDAT_IdleUnknown1;
extern uint8* PORTDAT_TalkingUnknown1;


namespace hooks {
	void injectBulletLimits();
	//	void injectPortdataLimits();
}
