#include "misc_extenders.h"

u32* PORTDAT_IdleDir;
u32* PORTDAT_TalkingDir;
u8* PORTDAT_IdleSMKChange;
u8* PORTDAT_TalkingSMKChange;
u8* PORTDAT_IdleUnknown1;
u8* PORTDAT_TalkingUnknown1;

namespace hooks
{
	void injectBulletLimits() {}
	void injectPortdataLimits()
	{
		//
		PORTDAT_IdleDir = new u32[NEW_PORTDATA_COUNT];
		PORTDAT_TalkingDir = new u32[NEW_PORTDATA_COUNT];
		PORTDAT_IdleSMKChange = new u8[NEW_PORTDATA_COUNT];
		PORTDAT_TalkingSMKChange = new u8[NEW_PORTDATA_COUNT];
		PORTDAT_IdleUnknown1 = new u8[NEW_PORTDATA_COUNT];
		u8* PORTDAT_TalkingUnknown1 = new u8[NEW_PORTDATA_COUNT];
		//
		memory_patch(0x0044EC20, PORTDAT_IdleDir);
		memory_patch(0x0044F1E5, PORTDAT_IdleDir);
		memory_patch(0x0044F213, PORTDAT_IdleDir);
		memory_patch(0x0044F253, ((u32)PORTDAT_IdleDir) + 4);
		memory_patch(0x0044F281, ((u32)PORTDAT_IdleDir) + 4);
		memory_patch(0x0044F2C1, ((u32)PORTDAT_IdleDir) + 8);
		memory_patch(0x0044F2EF, ((u32)PORTDAT_IdleDir) + 8);
		memory_patch(0x0044F32F, ((u32)PORTDAT_IdleDir) + 0x0C);
		memory_patch(0x0044F35D, ((u32)PORTDAT_IdleDir) + 0x0C);
		memory_patch(0x0044F39D, ((u32)PORTDAT_IdleDir) + 0x10);
		memory_patch(0x0044F3CB, ((u32)PORTDAT_IdleDir) + 0x10);
		memory_patch(0x0045E810, PORTDAT_IdleDir);
		memory_patch(0x0045F3D5, PORTDAT_IdleDir);
		memory_patch(0x0045F403, PORTDAT_IdleDir);
		memory_patch(0x0045F443, ((u32)PORTDAT_IdleDir) + 4);
		memory_patch(0x0045F471, ((u32)PORTDAT_IdleDir) + 4);
		memory_patch(0x0045F4B1, ((u32)PORTDAT_IdleDir) + 8);
		memory_patch(0x0045F4DF, ((u32)PORTDAT_IdleDir) + 8);
		memory_patch(0x0045F51F, ((u32)PORTDAT_IdleDir) + 0x0C);
		memory_patch(0x0045F54D, ((u32)PORTDAT_IdleDir) + 0x0C);
		memory_patch(0x0045F58D, ((u32)PORTDAT_IdleDir) + 0x10);
		memory_patch(0x0045F5BB, ((u32)PORTDAT_IdleDir) + 0x10);
		memory_patch(0x00513780, PORTDAT_IdleDir);
		memory_patch(0x0044ED04, PORTDAT_TalkingDir);
		memory_patch(0x0044F21C, PORTDAT_TalkingDir);
		memory_patch(0x0044F24A, PORTDAT_TalkingDir);
		memory_patch(0x0044F28A, ((u32)PORTDAT_TalkingDir) + 4);
		memory_patch(0x0044F2B8, ((u32)PORTDAT_TalkingDir) + 4);
		memory_patch(0x0044F2F8, ((u32)PORTDAT_TalkingDir) + 8);
		memory_patch(0x0044F326, ((u32)PORTDAT_TalkingDir) + 8);
		memory_patch(0x0044F366, ((u32)PORTDAT_TalkingDir) + 0x0C);
		memory_patch(0x0044F394, ((u32)PORTDAT_TalkingDir) + 0x0C);
		memory_patch(0x0044F3D4, ((u32)PORTDAT_TalkingDir) + 0x10);
		memory_patch(0x0044F402, ((u32)PORTDAT_TalkingDir) + 0x10);
		memory_patch(0x0045E900, PORTDAT_TalkingDir);
		memory_patch(0x0045F40C, PORTDAT_TalkingDir);
		memory_patch(0x0045F43A, PORTDAT_TalkingDir);
		memory_patch(0x0045F47A, ((u32)PORTDAT_TalkingDir) + 4);
		memory_patch(0x0045F4A8, ((u32)PORTDAT_TalkingDir) + 4);
		memory_patch(0x0045F4E8, ((u32)PORTDAT_TalkingDir) + 8);
		memory_patch(0x0045F516, ((u32)PORTDAT_TalkingDir) + 8);
		memory_patch(0x0045F556, ((u32)PORTDAT_TalkingDir) + 0x0C);
		memory_patch(0x0045F584, ((u32)PORTDAT_TalkingDir) + 0x0C);
		memory_patch(0x0045F5C4, ((u32)PORTDAT_TalkingDir) + 0x10);
		memory_patch(0x0045F5F2, ((u32)PORTDAT_TalkingDir) + 0x10);
		memory_patch(0x0051378C, PORTDAT_TalkingDir);
		memory_patch(0x0044F41A, PORTDAT_IdleSMKChange);
		memory_patch(0x0044F438, PORTDAT_IdleSMKChange);
		memory_patch(0x0045F60A, PORTDAT_IdleSMKChange);
		memory_patch(0x0045F628, PORTDAT_IdleSMKChange);
		/*
		memoryPatch(0x0044F41A, ((u32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(u32)));
		memoryPatch(0x0044F438, ((u32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(u32)));
		memoryPatch(0x0045F60A, ((u32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(u32)));
		memoryPatch(0x0045F628, ((u32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(u32)));
		*/
		memory_patch(0x00513798, PORTDAT_IdleSMKChange);
		memory_patch(0x005137A4, PORTDAT_TalkingSMKChange);
		memory_patch(0x005137B0, PORTDAT_IdleUnknown1);
		memory_patch(0x005137BC, PORTDAT_TalkingUnknown1);

		memory_patch(0x0044F40A, NEW_PORTDATA_COUNT * 4);
		memory_patch(0x0045F5FA, NEW_PORTDATA_COUNT * 4);
		memory_patch(0x00513788, NEW_PORTDATA_COUNT);
		memory_patch(0x00513794, NEW_PORTDATA_COUNT);
		memory_patch(0x005137A0, NEW_PORTDATA_COUNT);
		memory_patch(0x005137AC, NEW_PORTDATA_COUNT);
		memory_patch(0x005137B8, NEW_PORTDATA_COUNT);
		memory_patch(0x005137C4, NEW_PORTDATA_COUNT);
	}
}