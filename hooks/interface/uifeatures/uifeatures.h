#include "SCBW/scbwdata.h"
#include "SCBW/api.h"

namespace hooks {
	void drawUIFeatures();
	void targetedOrder(s16 a1, s32 a2, CUnit* a3, s16 a4, s32 a5, s32 a6, u8 a7, s32 a8, s32 a9, s32 a10);
	s32 selectUnits(CUnit** a1, s32 a2, u8 a3, s32 a4);
	u8 QueueGameCommand(const void* a1, u32 a2);

	void injectUIFeaturesHooks();

	void registerAction();
	void clearActions();
}