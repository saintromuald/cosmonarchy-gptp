#include "SCBW/scbwdata.h"
#include "SCBW/api.h"

namespace hooks {
	void drawUIFeatures();
	void targetedOrder(__int16 a1, int a2, CUnit* a3, __int16 a4, int a5, int a6, char a7, int a8, int a9, int a10);
	signed int selectUnits(CUnit** a1, int a2, char a3, int a4);

	void injectUIFeaturesHooks();

	void noteAction();
}