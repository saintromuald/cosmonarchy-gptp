#include "campaign.h"
#include "hook_tools.h"
#include "BW/bwdata.h"
#include "BW/ui.h"

static void change_menu_wrapper() {
    g_glue_background_id = -1;
    change_menu();
}

namespace hooks {
void inject_campaign_hooks() {
    jump_patch(continue_campaign, 0x004DC320);

    // Replace every post-continue_campaign call to change_menu so we don't crash with extended menu ids
    call_patch(change_menu_wrapper, 0x004B4DD2); // glu_scorescreen
    call_patch(change_menu_wrapper, 0x004F736B); // glu_z_readyroom
    call_patch(change_menu_wrapper, 0x004F76CB); // glu_t_readyroom
    call_patch(change_menu_wrapper, 0x004F7A2B); // glu_p_readyroom
}
} // namespace hooks
