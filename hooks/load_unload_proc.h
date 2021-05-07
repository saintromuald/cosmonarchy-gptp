#include <SCBW/structures/Unit.h>

namespace hooks
{

	void loadUnitProc(CUnit* unit, CUnit* unitToLoad);	//004E78E0
	b32 unloadUnitProc(CUnit* unit);					//004E7F70

	void injectLoadUnloadProcHooks();

} //hooks
