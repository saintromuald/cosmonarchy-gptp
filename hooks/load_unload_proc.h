#include <SCBW/structures/Unit.h>

namespace hooks
{

	void loadUnitProc(Unit* unit, Unit* unitToLoad);	//004E78E0
	b32 unloadUnitProc(Unit* unit);					//004E7F70

	void injectLoadUnloadProcHooks();

} //hooks
