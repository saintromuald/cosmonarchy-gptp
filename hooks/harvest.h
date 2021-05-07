#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {

void transferResourceToWorkerHook(Unit* worker, Unit* resource);

void injectHarvestResource();

} //hooks
