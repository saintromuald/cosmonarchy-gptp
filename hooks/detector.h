#pragma once
#include "../SCBW/structures/Unit.h"

struct DetectorCheckParam {
  u32 vision_flags;
  Unit* target;
};
C_ASSERT(sizeof(DetectorCheckParam) == 8);

namespace hooks
{
	bool unit_can_detect(Unit* unit);
	void get_cloaked_target_visibility(Unit* detector, DetectorCheckParam* param);

	void inject_detector_hooks();
}