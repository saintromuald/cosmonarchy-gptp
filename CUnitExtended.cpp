#include "CUnitExtended.h"

std::map<CUnit*, CUnitExtended*> CUnitExtended::flags;


void TimerSpecial1::init() {
	if (flags->unit->id == UnitId::ProtossDarkTemplar) {
		// init code for dts if needed
	}
}

void TimerSpecial1::tick() {
	Timer::tick();

	if (flags->unit->id == UnitId::ProtossDarkTemplar && (*timer) > 0) {
		// decloaked
	} else {
		// cloaked
	}
}