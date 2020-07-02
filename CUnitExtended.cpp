#include "CUnitExtended.h"

std::map<CUnit*, CUnitExtended*> CUnitExtended::flags;


void TimerSpecial1::init() {
	if (flags->unit->id == UnitId::ProtossDarkTemplar) {
		// init code for dts if needed
	}
}

void TimerSpecial1::tick() {
	Timer::tick();

	if (flags->unit->id == UnitId::ProtossDarkTemplar) {
		if ((*timer) > 0) {
			// decloaked
			flags->unit->shields += 128;
		} else {
			// cloaked
		}
	}
}