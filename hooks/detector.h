#pragma once
#include "../SCBW/structures/Unit.h"

struct DetectorCheckParam {
  u32 visionFlags;
  Unit* target;
};

C_ASSERT(sizeof(DetectorCheckParam) == 8);

namespace hooks {

bool unitCanDetectHook(Unit* unit);
void getCloakedTargetVisibility(Unit* detector, DetectorCheckParam* param);

void injectDetectorHooks();

} //hooks
