#include "ExtendedUnitFlags.h"

std::map<CUnit*, ExtendedUnitFlags> ExtendedUnitFlags::flags;


void SpecialTimer1::init() {
	if (flags->unit->id == UnitId::ProtossDarkTemplar) {
		flags->unit->status |= UnitStatus::CloakingForFree;
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