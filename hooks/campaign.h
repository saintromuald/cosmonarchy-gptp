#pragma once
#include "types.h"

b32 __stdcall continue_campaign(b32 victory); // 004DC320

namespace hooks {
void inject_campaign_hooks();
} // namespace hooks
