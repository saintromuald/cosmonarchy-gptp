#include "campaign.h"
#include "hook_tools.h"

namespace hooks {
void inject_campaign_hooks() {
    jump_patch(continue_campaign, 0x004DC320);
}
} // namespace hooks
