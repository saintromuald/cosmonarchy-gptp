#pragma once
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"

namespace hooks {
	void drawUIFeatures();
	void targetedOrder(int16 a1, int32 a2, CUnit* a3, int16 a4, int32 a5, int32 a6, uint8 a7, int32 a8, int32 a9, int32 a10);
	int32 selectUnits(CUnit** a1, int32 a2, uint8 a3, int32 a4);
	uint8 QueueGameCommand(const void* a1, uint32 a2);

	void injectUIFeaturesHooks();

	void registerAction();
	void clearActions();
}