#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {

void transferResourceToWorkerHook(CUnit* worker, CUnit* resource);

void injectHarvestResource();

} //hooks
