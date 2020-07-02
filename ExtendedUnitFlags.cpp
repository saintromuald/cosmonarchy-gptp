#include "ExtendedUnitFlags.h"

std::map<CUnit*, ExtendedUnitFlags> ExtendedUnitFlags::flags;


void SpecialTimer1::init() {
	if (flags->unit->id == UnitId::ProtossDarkTemplar) {
		// init code for dts if needed
	}
}

void SpecialTimer1::tick() {
	Timer::tick();

	if (flags->unit->id == UnitId::ProtossDarkTemplar && (*timer) > 0) {
		// decloaked
	} else {
		// cloaked
	}
}