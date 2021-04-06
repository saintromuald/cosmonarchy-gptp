#include "misc_extenders.h"

uint32* PORTDAT_IdleDir;
uint32* PORTDAT_TalkingDir;
uint8* PORTDAT_IdleSMKChange;
uint8* PORTDAT_TalkingSMKChange;
uint8* PORTDAT_IdleUnknown1;
uint8* PORTDAT_TalkingUnknown1;




namespace hooks {
	void injectBulletLimits() {}
	void injectPortdataLimits() {
		//
		PORTDAT_IdleDir = new uint32[NEW_PORTDATA_COUNT];
		PORTDAT_TalkingDir = new uint32[NEW_PORTDATA_COUNT];
		PORTDAT_IdleSMKChange = new uint8[NEW_PORTDATA_COUNT];
		PORTDAT_TalkingSMKChange = new uint8[NEW_PORTDATA_COUNT];
		PORTDAT_IdleUnknown1 = new uint8[NEW_PORTDATA_COUNT];
		uint8* PORTDAT_TalkingUnknown1 = new uint8[NEW_PORTDATA_COUNT];
		//
		memoryPatch(0x0044EC20, PORTDAT_IdleDir);
		memoryPatch(0x0044F1E5, PORTDAT_IdleDir);
		memoryPatch(0x0044F213, PORTDAT_IdleDir);
		memoryPatch(0x0044F253, ((uint32)PORTDAT_IdleDir) + 4);
		memoryPatch(0x0044F281, ((uint32)PORTDAT_IdleDir) + 4);
		memoryPatch(0x0044F2C1, ((uint32)PORTDAT_IdleDir) + 8);
		memoryPatch(0x0044F2EF, ((uint32)PORTDAT_IdleDir) + 8);
		memoryPatch(0x0044F32F, ((uint32)PORTDAT_IdleDir) + 0x0C);
		memoryPatch(0x0044F35D, ((uint32)PORTDAT_IdleDir) + 0x0C);
		memoryPatch(0x0044F39D, ((uint32)PORTDAT_IdleDir) + 0x10);
		memoryPatch(0x0044F3CB, ((uint32)PORTDAT_IdleDir) + 0x10);
		memoryPatch(0x0045E810, PORTDAT_IdleDir);
		memoryPatch(0x0045F3D5, PORTDAT_IdleDir);
		memoryPatch(0x0045F403, PORTDAT_IdleDir);
		memoryPatch(0x0045F443, ((uint32)PORTDAT_IdleDir) + 4);
		memoryPatch(0x0045F471, ((uint32)PORTDAT_IdleDir) + 4);
		memoryPatch(0x0045F4B1, ((uint32)PORTDAT_IdleDir) + 8);
		memoryPatch(0x0045F4DF, ((uint32)PORTDAT_IdleDir) + 8);
		memoryPatch(0x0045F51F, ((uint32)PORTDAT_IdleDir) + 0x0C);
		memoryPatch(0x0045F54D, ((uint32)PORTDAT_IdleDir) + 0x0C);
		memoryPatch(0x0045F58D, ((uint32)PORTDAT_IdleDir) + 0x10);
		memoryPatch(0x0045F5BB, ((uint32)PORTDAT_IdleDir) + 0x10);
		memoryPatch(0x00513780, PORTDAT_IdleDir);
		memoryPatch(0x0044ED04, PORTDAT_TalkingDir);
		memoryPatch(0x0044F21C, PORTDAT_TalkingDir);
		memoryPatch(0x0044F24A, PORTDAT_TalkingDir);
		memoryPatch(0x0044F28A, ((uint32)PORTDAT_TalkingDir) + 4);
		memoryPatch(0x0044F2B8, ((uint32)PORTDAT_TalkingDir) + 4);
		memoryPatch(0x0044F2F8, ((uint32)PORTDAT_TalkingDir) + 8);
		memoryPatch(0x0044F326, ((uint32)PORTDAT_TalkingDir) + 8);
		memoryPatch(0x0044F366, ((uint32)PORTDAT_TalkingDir) + 0x0C);
		memoryPatch(0x0044F394, ((uint32)PORTDAT_TalkingDir) + 0x0C);
		memoryPatch(0x0044F3D4, ((uint32)PORTDAT_TalkingDir) + 0x10);
		memoryPatch(0x0044F402, ((uint32)PORTDAT_TalkingDir) + 0x10);
		memoryPatch(0x0045E900, PORTDAT_TalkingDir);
		memoryPatch(0x0045F40C, PORTDAT_TalkingDir);
		memoryPatch(0x0045F43A, PORTDAT_TalkingDir);
		memoryPatch(0x0045F47A, ((uint32)PORTDAT_TalkingDir) + 4);
		memoryPatch(0x0045F4A8, ((uint32)PORTDAT_TalkingDir) + 4);
		memoryPatch(0x0045F4E8, ((uint32)PORTDAT_TalkingDir) + 8);
		memoryPatch(0x0045F516, ((uint32)PORTDAT_TalkingDir) + 8);
		memoryPatch(0x0045F556, ((uint32)PORTDAT_TalkingDir) + 0x0C);
		memoryPatch(0x0045F584, ((uint32)PORTDAT_TalkingDir) + 0x0C);
		memoryPatch(0x0045F5C4, ((uint32)PORTDAT_TalkingDir) + 0x10);
		memoryPatch(0x0045F5F2, ((uint32)PORTDAT_TalkingDir) + 0x10);
		memoryPatch(0x0051378C, PORTDAT_TalkingDir);
		memoryPatch(0x0044F41A, PORTDAT_IdleSMKChange);
		memoryPatch(0x0044F438, PORTDAT_IdleSMKChange);
		memoryPatch(0x0045F60A, PORTDAT_IdleSMKChange);
		memoryPatch(0x0045F628, PORTDAT_IdleSMKChange);
		/*
		memoryPatch(0x0044F41A, ((uint32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(uint32)));
		memoryPatch(0x0044F438, ((uint32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(uint32)));
		memoryPatch(0x0045F60A, ((uint32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(uint32)));
		memoryPatch(0x0045F628, ((uint32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(uint32)));
		*/
		memoryPatch(0x00513798, PORTDAT_IdleSMKChange);
		memoryPatch(0x005137A4, PORTDAT_TalkingSMKChange);
		memoryPatch(0x005137B0, PORTDAT_IdleUnknown1);
		memoryPatch(0x005137BC, PORTDAT_TalkingUnknown1);

		memoryPatch(0x0044F40A, NEW_PORTDATA_COUNT * 4);
		memoryPatch(0x0045F5FA, NEW_PORTDATA_COUNT * 4);
		memoryPatch(0x00513788, NEW_PORTDATA_COUNT);
		memoryPatch(0x00513794, NEW_PORTDATA_COUNT);
		memoryPatch(0x005137A0, NEW_PORTDATA_COUNT);
		memoryPatch(0x005137AC, NEW_PORTDATA_COUNT);
		memoryPatch(0x005137B8, NEW_PORTDATA_COUNT);
		memoryPatch(0x005137C4, NEW_PORTDATA_COUNT);
	}
}