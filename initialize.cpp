#include "definitions.h"
#include "Plugin.h"
#include "globals.h"
#include "hook_tools.h"
#include <chrono>
#include <thread>

// Hook header files
#include "hooks/game_hooks.h"
#include "hooks/replay_hooks.h"
#include "ReplayKraken.h"
#include "graphics/draw_hook.h"
#include "config.h"
#include "systems/display/display.h"
#include "systems/display/zoom.h"
#include "systems/text/text.h"
#include "hooks/display/refresh.h"
#include "hooks/display/framebuffer.h"
#include "hooks/display/blit.h"
#include "hooks/display/dialog_clip.h"
#include "hooks/display/popup_dialog.h"
#include "hooks/display/widget_dialog_init.h"
#include "hooks/display/tooltip_bounds.h"
#include "hooks/display/mapblit.h"
#include "hooks/display/grid.h"
#include "hooks/display/translucent_image.h"
#include "hooks/display/screen_text.h"
#include "hooks/display/wheel.h"
#include "hooks/display/gamemap.h"
#include "hooks/display/terrain.h"
#include "hooks/display/tiles.h"
#include "hooks/display/fog.h"
#include "hooks/display/starfield.h"
#include "hooks/display/sprites.h"
#include "hooks/display/mouse.h"
#include "hooks/display/camera.h"
#include "hooks/display/frame_pacing.h"
#include "hooks/text/text_print.h"
#include "hooks/text/text_width.h"
#include "hooks/maxhp.h"
#include "hooks/apply_upgrade_flags.h"
#include "hooks/attack_priority.h"
#include "hooks/bunker_hooks.h"
#include "hooks/cloak_nearby_units.h"
#include "hooks/cloak_tech.h"
#include "hooks/detector.h"
#include "hooks/harvest.h"
#include "hooks/rally_point.h"
#include "hooks/recharge_shields.h"
#include "hooks/spider_mine.h"
#include "hooks/stim_packs.h"
#include "hooks/tech_target_check.h"
#include "hooks/transfer_tech_upgrades.h"
#include "hooks/unit_speed.h"
#include "hooks/update_status_effects.h"
#include "hooks/update_unit_state.h"
#include "hooks/weapons/weapon_cooldown.h"
#include "hooks/weapons/weapon_damage.h"
#include "hooks/weapons/weapon_fire.h"
#include "hooks/unit_destructor_special.h"
#include "hooks/psi_field.h"
#include "hooks/unit_stats/armor_bonus.h"
#include "hooks/unit_stats/sight_range.h"
#include "hooks/unit_stats/max_energy.h"
#include "hooks/unit_stats/weapon_range.h"
#include "hooks/interface/weapon_armor_tooltip.h"
#include "hooks/orders/ai/ai_orders.h"
#include "hooks/orders/base_orders/attack_orders.h"
#include "hooks/interface/btns_cond.h"
#include "hooks/orders/building_making/building_zerg.h"
#include "hooks/interface/buttonsets.h"
#include "hooks/orders/building_making/building_protoss.h"
#include "hooks/orders/building_making/building_terran.h"
#include "hooks/recv_commands/burrow_tech.h"
#include "hooks/orders/spells/cast_order.h"
#include "hooks/recv_commands/CMDRECV_Build.h"
#include "hooks/recv_commands/CMDRECV_Cancel.h"
#include "hooks/recv_commands/CMDRECV_MergeArchon.h"
#include "hooks/recv_commands/CMDRECV_Morph.h"
#include "hooks/recv_commands/CMDRECV_SiegeTank.h"
#include "hooks/recv_commands/CMDRECV_Stop.h"
#include "hooks/create_init_units.h"
#include "hooks/orders/base_orders/die_order.h"
#include "hooks/orders/enter_nydus.h"
#include "hooks/orders/spells/feedback_spell.h"
#include "hooks/give_unit.h"
#include "hooks/orders/spells/hallucination_spell.h"
#include "hooks/orders/infestation.h"
#include "hooks/orders/larva_creep_spawn.h"
#include "hooks/orders/liftland.h"
#include "hooks/orders/load_unload_orders.h"
#include "hooks/load_unload_proc.h"
#include "hooks/interface/mainmenu/lobby.h"
#include "hooks/orders/building_making/make_nydus_exit.h"
#include "hooks/orders/medic_orders.h"
#include "hooks/orders/merge_units.h"
#include "hooks/orders/spells/mindcontrol_spell.h"
#include "hooks/miss_chance.h"
#include "hooks/orders/spells/nuke_orders.h"
#include "hooks/orders/spells/recall_spell.h"
#include "hooks/recv_commands/receive_command.h"
#include "hooks/orders/repair_order.h"
#include "hooks/orders/research_upgrade_orders.h"
#include "hooks/interface/select_larva.h"
#include "hooks/interface/selection.h"
#include "hooks/audiovisuals/sfxhooks.h"
#include "hooks/audiovisuals/music_hooks.h"
#include "hooks/orders/siege_transform.h"
#include "hooks/interface/stats_panel_display.h"
#include "hooks/orders/base_orders/stopholdpos_orders.h"
#include "hooks/recv_commands/train_cmd_receive.h"
#include "hooks/orders/unit_making/unit_morph.h"
#include "hooks/orders/unit_making/unit_train.h"
#include "hooks/utils/unit_utils.h"
#include "hooks/interface/wireframe.h"
#include "hooks/weapons/weapon_spellhit.h"
#include "hooks/weapon_effect.h"
#include "hooks/use_tech.h"
#include "hooks/limits/ccmu.h"
#include "hooks/economic_units.h"
#include "hooks/limits/extender.h"
#include "hooks/build_time.h"
#include "AI/spellcasting.h"
#include "hooks/misc/cosmonarchy_specific.h"
#include "hooks/interface/uifeatures/hotkeys.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "hooks/trap_hooks.h"
#include "hooks/target_related.h"
#include "hooks/unit_movement.h"
#include "hooks/neutral_related.h"
#include "hooks/game/file_operations.h"
#include "hooks/interface/mainmenu/glucustom.h"
#include "hooks/orders/pathfinding.h"
#include "hooks/placement.h"
#include "hooks/interface/briefing/briefing.h"
#include "hooks/unit_stats/supply_hooks.h"
#include "hooks/unit_stats/new_requirements.h"
#include "hooks/unit_ai.h"
#include "hooks/aiscript/ai_requests.h"
#include "BW/ExtendSightLimit.h"
// #include "hooks/requirements/requirements.h"
#include "systems/triggers/trigger_hooks.h"
#include "hooks/loading_hook.h"
#include "hooks/DatExtend/dat_load.h"
#include "DatExtend/datext.h"
#include "hooks/interface/hpbars/hpbars.h"
#include "BW/structures/Image.h"
#include "hooks/orders/orders_root.h"
#include "selection_extender.h"
#include "hooks/update_unit_sight.h"
#include "utility/RegConfig.h"
#include "systems/scripting/scripting.h"
#include "hooks/audiovisuals/image_render.h"
#include "hooks/interface/minimap.h"
#include "hooks/game/creategame.h"
#include "BW/extender/iscript_extender.h"
#include "hooks/limits/tbl_extender.h"
#include "BW/game_runner.h"
#include "hooks/fraud_launcher.h"
#include "hooks/interface/hud/hud.h"
#include "hooks/race_randomization.h"
#include "hooks/interface/mainmenu/custom_glues.h"
#include "hooks/snp.h"
#include "hooks/lobby_command_receive.h"
#include "hooks/campaign.h"
#include "hooks/resarea.h"

/// This function is called when the plugin is loaded into StarCraft.
/// You can enable/disable each group of hooks by commenting them.
/// You can also add custom modifications to StarCraft.exe by using:
///	memory_patch(address_to_patch, value_to_patch_with);

namespace {
    void load_wmode() {
        LoadLibraryA("./WMODE.dll");
        LoadLibraryA("./WMODE_FIX.dll");
        g_wmode_loaded = true;
    }

    void run_wmode(bool hide_prompt, bool prefer_wmode) {
        if (!hide_prompt) {
            // KYSXD windowed mode
            s32 msgbox_id = MessageBoxA(nullptr, "Do you want to play in windowed mode?", PLUGIN_NAME, MB_YESNOCANCEL);
            switch (msgbox_id) {
                case IDYES: {
                    load_wmode();
                } break;
                case IDCANCEL: {
                    exit(0);
                } break;
            }
        } else if (prefer_wmode) {
            load_wmode();
        }
    }

    static HHOOK g_keyboard_hook = nullptr;

    // This is a low-level hook that's installed system-wide
    // Do not extend this for other keys unless they also need blocked system-wide
    // Use hotkeys.cpp instead
    static LRESULT CALLBACK key_press_hook_block_win(int code, WPARAM wparam, LPARAM lparam) {
        if (code >= 0) {
            HWND foreground_window = GetForegroundWindow();
            if (foreground_window == g_main_window) {
                KBDLLHOOKSTRUCT* keypress = (KBDLLHOOKSTRUCT*)lparam;

                if (keypress->vkCode == VK_LWIN || keypress->vkCode == VK_RWIN) {
                    return true;
                }
            }
        }

        return CallNextHookEx(g_keyboard_hook, code, wparam, lparam);
    }

    static void install_win_key_hook() {
        if (!g_config.suppress_windows_key) return;
        if (g_keyboard_hook) return;

        g_keyboard_hook = SetWindowsHookExA(
            WH_KEYBOARD_LL,
            key_press_hook_block_win,
            GetModuleHandleA(nullptr),
            0
        );
    }

    static void remove_win_key_hook() {
        if (!g_keyboard_hook) return;

        UnhookWindowsHookEx(g_keyboard_hook);
        g_keyboard_hook = nullptr;
    }
}

BOOL WINAPI Plugin::InitializePlugin(IMPQDraftServer* lpMPQDraftServer) {
    init_float_precision();

    load_config();
    g_display.init();
    g_dirty_cell_grids.init();
    g_zoom.init();
    text::TextSystem::instance().init();
    run_wmode(g_config.disable_wmode_prompt, g_config.prefer_wmode);
    // StarCraft.exe version check
    char exe_path[300];
    const DWORD pathLen = GetModuleFileNameA(nullptr, exe_path, sizeof(exe_path));

    if (pathLen == sizeof(exe_path)) {
        MessageBoxA(nullptr, "Error: Cannot check version of StarCraft.exe. The file path is too long.", nullptr, MB_OK);
        return FALSE;
    }

    if (!check_starcraft_exe_version(exe_path)) {
        return FALSE;
    }

    // I am not sure if this is early enough for it work on the current run
    // might require a game restart...                               -Veeq7
    RegConfig::overwrite_reg_config();

    lua::ScriptManager::generate_lsp_annotations();

    // In case we need to attach a debugger on launch
    /*
    std::chrono::milliseconds sleepq7(15000);
    std::this_thread::sleep_for(sleepq7);
    */

    // Hook windows key to prevent accidentally breaking window focus
    install_win_key_hook();

    DatExt::extend_data();

    // Fix to make vanilla campaigns playable from Firegraft executables -UndeadStar
//	jump_patch((void*)0x15017960, 0x004100C4); // insert a "jump to storm.dll function" instead of a "jump to firegraft function" in an array

    // Jesse, start
    if (g_config.allow_multiple_instances) {
        // skip the jump to 0x004e0380 which checks for other instances and exits if found
        nop_patch(0x004e0b02, 5);
    }
    // Jesse, end

    // Veeq7, start
    // Quicker drop timeout
    memory_patch(0x004A345D + 2, (u32)0);

    // Allow starting the game alone (singleplayer)
    jump_patch((void*)(0x004AEE9E), 0x004AEE83, 2);
    jump_patch((void*)(0x004A8222), 0x004A81CA, 0);

    // Allow starting the game alone (multiplayer)
    const u8 human_slots = 0x01;
    memory_patch((void*)(0x004B995B + 2), &human_slots, sizeof(u8)); 
    memory_patch((void*)(0x004B9988 + 2), &human_slots, sizeof(u8));

    // Allow all players addons to connect
    nop_patch(0x00440314, 6);

    // Custom Aise game command replay support
    u32 new_size = 0xC;
    memory_patch(0x005005F8 + 99 * sizeof(u32), new_size);

    // Remove minimap checksum checks.
//	nop_patch(0x0047CCB0, 22);				// Allows for silent desyncs

    // Update vision update timer to happen faster
    memory_patch(0x004D94D0 + 6, 2);

    // Remove mouse/keyboard conflict
    nop_patch(0x004D21A3, 2);
    nop_patch(0x004D2264, 2);
    nop_patch(0x004D1F26, 2);
    nop_patch(0x004D1FAE, 6);

    // Fix irradiate field being created on replace unit with type call
    nop_patch(0x00454EB9, 7);
    // Veeq7, end

    // iquare, start
    // disable powerup hardcode
    // FLAG id only
    const u8 maxunsigned_2[5] = {0xff, 0xff};
    memory_patch(0x0044095D + 4, maxunsigned_2, 2);	// flag
    memory_patch(0x0045FE5C + 1, maxunsigned_2, 2);	// flag
    memory_patch(0x00460902 + 1, maxunsigned_2, 2);	// flag
    memory_patch(0x0049F145 + 4, maxunsigned_2, 2);	// flag
    memory_patch(0x004E4153 + 4, maxunsigned_2, 2);	// flag
    memory_patch(0x004E416E + 4, maxunsigned_2, 2);	// flag
//	memory_patch(0x004E426B + 1, maxunsigned_2, 2); // ctf create flag, this code doesn't run at all because CTF are spell orders now
    memory_patch(0x004E5F68 + 3, maxunsigned_2, 2);
    memory_patch(0x004E70E4 + 4, maxunsigned_2, 2);
    memory_patch(0x004F3A9C + 4, maxunsigned_2, 2);
    memory_patch(0x004F3C26 + 4, maxunsigned_2, 2);
    memory_patch(0x004F3CD5 + 4, maxunsigned_2, 2);

    const u8 none = 0xE4;
    // TODO: hook parent function and make AI assign different units based on personality
//	memory_patch(0x004357FB + 2, &none, sizeof(u8)); // disable AI placing units near bunkers
//	memory_patch(0x0043583F + 1, &none, sizeof(u8)); // disable AI placing units near missile turrets

    // Matriarch - Pazuzu id hardcode
    memory_patch(0x0040275A + 2, &none, sizeof(u8)); // Hooked
    memory_patch(0x0043CF74 + 2, &none, sizeof(u8));
    memory_patch(0x00455E49 + 3, &none, sizeof(u8)); // Hooked
    memory_patch(0x00455EC7 + 3, &none, sizeof(u8)); // Hooked
    memory_patch(0x00475B68 + 2, &none, sizeof(u8));
    memory_patch(0x004C210A + 2, &none, sizeof(u8)); // Hooked but disabled

    // Rhynadon id hardcode
    memory_patch(0x0042E352 + 1, &none, sizeof(u8));
    memory_patch(0x00431574 + 2, &none, sizeof(u8));
    memory_patch(0x00431B37 + 2, &none, sizeof(u8));
    memory_patch(0x00437E0B + 2, &none, sizeof(u8));
    memory_patch(0x00437E3A + 2, &none, sizeof(u8));
    memory_patch(0x0047B454 + 2, &none, sizeof(u8));
    memory_patch(0x0047B7C9 + 2, &none, sizeof(u8));
    memory_patch(0x004CBE49 + 2, &none, sizeof(u8));

    // Flag Beacon id hardcode
    const u8 jmp = 0xEB;
    const u8 min = 0x00;
    memory_patch(0x0042F134 + 2, &none, sizeof(u8));
    memory_patch(0x0042F13A, &jmp, 1); // JA
    memory_patch(0x0043A602 + 2, &none, sizeof(u8));
//	memory_patch(0x0043A609 + 2, &none, sizeof(u8));
    memory_patch(0x004558A3 + 2, &none, sizeof(u8));
//	memory_patch(0x004558A9 + 2, &none, sizeof(u8));
    memory_patch(0x00455940 + 2, &none, sizeof(u8));
//	memory_patch(0x00455946 + 2, &none, sizeof(u8));
    memory_patch(0x00455AF2 + 2, &none, sizeof(u8));
//	memory_patch(0x00455AF8 + 2, &none, sizeof(u8));
    memory_patch(0x00473A16 + 2, &none, sizeof(u8));
//	memory_patch(0x00473A1C + 2, &none, sizeof(u8));
    memory_patch(0x00473BD6 + 2, &none, sizeof(u8));
//	memory_patch(0x00473BE3 + 2, &none, sizeof(u8));
    memory_patch(0x0047B2C4 + 2, &none, sizeof(u8));
//	memory_patch(0x0047B2CA + 2, &none, sizeof(u8));
    memory_patch(0x004E3DC4 + 2, &none, sizeof(u8));
    memory_patch(0x004E3DCA, &jmp, 1); // JA
    const u8 moveaxpatch[5] = {0xb8,  0x00, 0x00, 0x00, 0x00};
    memory_patch((void*)0x00487C93, moveaxpatch, sizeof(moveaxpatch)); // disable unit selectable check in placement function

    // disable terran structure building transport condition
    const u8 fourth = 0x3;
    memory_patch(0x004E6E7A + 1, &fourth, sizeof(u8));
    const u8 jnz = 0x75;
    memory_patch(0x004E6E7C, &jnz, sizeof(u8));

//	hooks::inject_economic_unit_hooks();
    // test for globals
//	nop_patch(0x004AAF23, 5);
//	const u8 retn = 0xc3;
//	memory_patch(0x004AAF23 + 5, &retn, 1);

    // Didact attack hardcode
    /*
    nop_patch(0x0043AEE0, 10);
    nop_patch(0x0043CF89, 4);
    memory_patch(0x0043CF8D, &jmp, 1);
    nop_patch(0x0047681B, 5);
    memory_patch(0x00476820, &jmp, 1);
    nop_patch(0x00476E67, 6);
    nop_patch(0x00477AB7, 5);
    memory_patch(0x00477ABC, &jmp, 1);
    nop_patch(0x00477E09, 5);
    nop_patch(0x00478613, 5);
    nop_patch(0x00478613, 5);
    nop_patch(0x0048B0A1, 5);
    memory_patch(0x0048B0A6, &jmp, 1);
    */

    memory_patch(0x00477AB7 + 4, &none, 1);
    memory_patch(0x0043CF89 + 2, &none, 1);
    memory_patch(0x0047681B + 4, &none, 1);
    memory_patch(0x00476E67 + 2, &none, 1);
    memory_patch(0x00477E09 + 4, &none, 1);
    memory_patch(0x00478613 + 4, &none, 1);
    memory_patch(0x0048B0A1 + 4, &none, 1);

    // town hall code
    // worker code
    // 00434C36, 0043378C, 0043214B
    // townhall code
    // 00433f62, 0043A100, 004365f6, 00433EB5, 0044387B, 00446230

    u8 patch_last_unit[] = {0xE4};
    // allow pissed/yes sounds for >= Command Center
    // memory_patch(0x0048EA20 + 2, patch_last_unit, 1);
    // hook required
    // nop_patch(0x0048EAB0, 5);

    // Bengalaas id hardcode
    memory_patch(0x0043157A + 2, patch_last_unit, 1);
    memory_patch(0x00431B3D + 2, patch_last_unit, 1);
    memory_patch(0x00437E11 + 2, patch_last_unit, 1);
    memory_patch(0x00437E40 + 2, patch_last_unit, 1);
    memory_patch(0x0047B45A + 2, patch_last_unit, 1);
    memory_patch(0x0047B7CF + 2, patch_last_unit, 1);
    memory_patch(0x004CBE4E + 2, patch_last_unit, 1);

    // Scantid id hardcode
    memory_patch(0x0043158C + 2, patch_last_unit, 1);
    memory_patch(0x00431B4F + 2, patch_last_unit, 1);
    memory_patch(0x00437E23 + 2, patch_last_unit, 1);
    memory_patch(0x00437E52 + 2, patch_last_unit, 1);
    memory_patch(0x0047B46C + 2, patch_last_unit, 1);
    memory_patch(0x0047B7E1 + 2, patch_last_unit, 1);
    memory_patch(0x004CBE58 + 2, patch_last_unit, 1);

    // Ragnasaur id hardcode
    memory_patch(0x00431580 + 2, patch_last_unit, 1);
    memory_patch(0x00431B43 + 2, patch_last_unit, 1);
    memory_patch(0x00437E17 + 2, patch_last_unit, 1);
    memory_patch(0x00437E46 + 2, patch_last_unit, 1);
    memory_patch(0x0047B460 + 2, patch_last_unit, 1);
    memory_patch(0x0047B7D5 + 2, patch_last_unit, 1);
    memory_patch(0x004CBE53 + 2, patch_last_unit, 1);

    // Kakaru id hardcode
    memory_patch(0x00431586 + 2, patch_last_unit, 1);
    memory_patch(0x00431B49 + 2, patch_last_unit, 1);
    memory_patch(0x00437E1D + 2, patch_last_unit, 1);
    memory_patch(0x00437E4C + 2, patch_last_unit, 1);
    memory_patch(0x0047B466 + 2, patch_last_unit, 1);
    memory_patch(0x0047B7DB + 2, patch_last_unit, 1);
    memory_patch(0x0047C425 + 4, patch_last_unit, 1);
    memory_patch(0x0047C476 + 4, patch_last_unit, 1);
    memory_patch(0x004CBE5D + 2, patch_last_unit, 1);

    // Ursadon id hardcode
    memory_patch(0x00431592 + 2, patch_last_unit, 1);
    memory_patch(0x00431B55 + 2, patch_last_unit, 1);
    memory_patch(0x00437E29 + 2, patch_last_unit, 1);
    memory_patch(0x00437E58 + 2, patch_last_unit, 1);
    memory_patch(0x0047B472 + 2, patch_last_unit, 1);
    memory_patch(0x0047B7E7 + 2, patch_last_unit, 1);
    memory_patch(0x004CBE62 + 2, patch_last_unit, 1);

    // Warbringer id hardcode
    memory_patch(0x004F6C1C + 2, patch_last_unit, 1);
    memory_patch(0x004F6578 + 2, patch_last_unit, 1);
    memory_patch(0x004F656B + 2, patch_last_unit, 1);
    memory_patch(0x004E36EA + 2, patch_last_unit, 1);
    memory_patch(0x004E3029 + 3, patch_last_unit, 1);
    memory_patch(0x004CD28C + 2, patch_last_unit, 1);
    memory_patch(0x004C8ADE + 3, patch_last_unit, 1);
    memory_patch(0x004C8A94 + 2, patch_last_unit, 1);
    memory_patch(0x004C8A5F + 2, patch_last_unit, 1);
    memory_patch(0x004C20FA + 2, patch_last_unit, 1);
    memory_patch(0x004C189C + 2, patch_last_unit, 1);
    memory_patch(0x004C1859 + 2, patch_last_unit, 1);
    memory_patch(0x004C1836 + 2, patch_last_unit, 1);
    memory_patch(0x0049F4B9 + 2, patch_last_unit, 1);
    memory_patch(0x0047CA8C + 2, patch_last_unit, 1);
    memory_patch(0x004761D5 + 2, patch_last_unit, 1);
    memory_patch(0x00475CAB + 2, patch_last_unit, 1);
    memory_patch(0x00475B59 + 2, patch_last_unit, 1);
    memory_patch(0x00468222 + 2, patch_last_unit, 1);
    memory_patch(0x004668A5 + 2, patch_last_unit, 1);
    memory_patch(0x004667A9 + 2, patch_last_unit, 1);
    memory_patch(0x00466764 + 2, patch_last_unit, 1);
    memory_patch(0x004659FD + 2, patch_last_unit, 1);
    memory_patch(0x0046571A + 2, patch_last_unit, 1);
    memory_patch(0x00465466 + 2, patch_last_unit, 1);
    memory_patch(0x00465412 + 2, patch_last_unit, 1);
    memory_patch(0x004653A7 + 3, patch_last_unit, 1);
    memory_patch(0x0046536A + 2, patch_last_unit, 1);
    memory_patch(0x0046534B + 2, patch_last_unit, 1);
    memory_patch(0x004608D9 + 2, patch_last_unit, 1);
    memory_patch(0x0045FEBD + 2, patch_last_unit, 1);
    memory_patch(0x0045FE98 + 2, patch_last_unit, 1);
    memory_patch(0x004407B5 + 2, patch_last_unit, 1);
    memory_patch(0x0043CF4D + 2, patch_last_unit, 1);
    memory_patch(0x004286B5 + 3, patch_last_unit, 1);
    memory_patch(0x00426E4A + 2, patch_last_unit, 1);
    memory_patch(0x004254D2 + 2, patch_last_unit, 1);
    memory_patch(0x0042549C + 2, patch_last_unit, 1);
    memory_patch(0x004253E6 + 2, patch_last_unit, 1);
    memory_patch(0x0042491E + 3, patch_last_unit, 1);
    memory_patch(0x004247C4 + 3, patch_last_unit, 1);
    memory_patch(0x004014C6 + 2, patch_last_unit, 1);
    memory_patch(0x0040149A + 2, patch_last_unit, 1);

    // Mind Tyrant - energy of controlled Mind Tyrants is no longer set to 0
//	memory_patch(0x004F6938 + 4, patch_last_unit, 1);

    nop_patch(0x0049EEBD, 0xa); // disable beacon collision hardcode
    u8 p8a = {0x8a};
    u8 ff = {0xff};
    memory_patch(0x00497764 + 3, &p8a, sizeof(u8));
    memory_patch(0x00497784 + 3, &ff, sizeof(u8)); // disable burrow hardcode for image ids: Flag, Young Chrysalis, Uraj, Khalis

    // CTF orders
    u8 patchCTF1[] = {0xE9, 0xCC, 0x02, 0x00, 0x00};
    memory_patch(0x004EC80A, patchCTF1, sizeof patchCTF1); // CTFCOPInit - Wyvern?
    u8 patchCTF2[] = {0xE9, 0xC2, 0x02, 0x00, 0x00};
    memory_patch(0x004EC814, patchCTF2, sizeof patchCTF2); // CTFCOP1 - Essence Drain (now unused)
    // iquare, end

    // DarkenedFantasies, start
    // Race icons frame color fix. (function score_screen 004B4AF0)
    memory_patch(0x004B4C31, (u16)0xB0);							// MOV AL, 00 (null-terminate "tminimap.pcx" string (0x005032F8))
    memory_patch(0x004B4C33, (u8)0x68);								// PUSH...
    memory_patch(0x004B4C34, (u32)0x100);							// ...00 00 01 00 (change score screen tminimap.pcx buffer size from 12 to 256 (0x0059B730))

    // Score fill color fix. (function gluscore_score_fill_color 004B3A40)
    memory_patch(0x004B3B11, (u8)0x8B);								// MOV ECX, [EBP - 0xC], formerly CL
    call_patch(hooks::sub_4EF710, 0x004B3B23);						// Redirect to the patched subroutines.
    memory_patch(0x004B3B49, (u8)0x8B);								// MOV ECX, [EBP - 0x10], formerly CL
    call_patch(hooks::sub_4EF710, 0x004B3B4D);						// Redirect to the patched subroutines.

    // Move race icons from iScore.grp into new grp.
    jump_patch(hooks::loadMenu_gluScore_pIcons, 0x004B4B80, 13);	// Insert code for loading the pIcons.grp file.
    memory_patch(0x004B38BB, (u16)0x9090);							// Remove +3 frame offset for race icons.
    memory_patch(0x004B38BD, (u8)0x90);
    memory_patch(0x004B3A63, (u32)0x59B728);						// New LoadGraphic address for iScore.grp; pIcons.grp gets the original iScore address.
    memory_patch(0x004B4BE4, (u32)0x59B728);

    // Remove offsets from iscore.grp.
    memory_patch(0x004B3AC1, (u16)0x9010);							// Remove +6 frame offset for score bar fills.
    memory_patch(0x004B3AC5, (u8)0x3);								// Remove +9 frame offset for score bar borders.

    // Extend font colors.
    memory_patch(0x00417A41, &g_tfontgame_image_data[0x1]);			// Update buffer size and redirect to the expanded buffer for "sub_417A10".
    memory_patch(0x00417A5A, &g_tfontgame_image_data[0x10]);
    memory_patch(0x00417AAA, &g_tfontgame_image_data[0x11]);
    memory_patch(0x00417B05, &g_tfontgame_image_data[0x12]);
    memory_patch(0x00417B61, &g_tfontgame_image_data[0x13]);
    memory_patch(0x0041FB41, &g_tfontgame_image_data);					// ...for "SetFontHandle".
    memory_patch(0x004BA127, sizeof(g_tfontgame_image_data));			// ...for "get_tFontGam_PCX_0".
    memory_patch(0x004BA12C, &g_tfontgame_image_data);
    memory_patch(0x004BA137, &g_tfontgame_image_data);
    memory_patch(0x004BA167, sizeof(g_tfontgame_image_data));			// ...for "get_tFontGam_PCX".
    memory_patch(0x004BA16C, &g_tfontgame_image_data);
    memory_patch(0x004BA177, &g_tfontgame_image_data);
    memory_patch(0x004D48C9, sizeof(g_tfontgame_image_data));			// ...for "titleInit".
    memory_patch(0x004D48CE, &g_tfontgame_image_data);
    memory_patch(0x004D48E0, &g_tfontgame_image_data);
    memory_patch(0x004D8A34, sizeof(g_tfontgame_image_data));			// ...for "creditsSetFontColorFromFile".
    memory_patch(0x004D8A67, &g_tfontgame_image_data);
    memory_patch(0x004DD8C8, sizeof(g_tfontgame_image_data));			// ...for "loadTFontPcx".
    memory_patch(0x004DDB71, &g_tfontgame_image_data);					// ...for "registerMenuFunctions".

    // Extend and add per-player support for font colors.
    jump_patch(hooks::writeFontApplyColour_patch, 0x0041FDC6);
    jump_patch(hooks::get_color_handle, 0x0041F610, 4);
    memory_patch(0x004200B6, (u8)0x00);								// <0B>
    memory_patch(0x004200B9, (u32)0x05050505);						// tfont offsets for <0E> through <11>.
    memory_patch(0x004200BF, (u8)0x00);								// <14>
    memory_patch(0x004200C0, (u32)0x08080808);						// tfont offsets for <15> through <1F>.
    memory_patch(0x004200C4, (u32)0x08080808);
    memory_patch(0x004200C8, (u16)0x0808);
    memory_patch(0x004200CA, (u8)0x08);

    memory_patch(0x0048CF34, (u8)0x1A);								// Change default in-game transmission text color.

    // Enable ESC to skip triggers in campaign mode.
    memory_patch(0x00489DAF, (u32)0x90909090);
    memory_patch(0x00489DB3, (u32)0x90909090);
    memory_patch(0x00489DB7, (u32)0x90909090);
    memory_patch(0x00489DBB, (u16)0x9090);

    // Set to 1 to speed up force switching in lobby.
    memory_patch(0x004D3EE9, (u32)1);

    // Prevent pause triggers from indefinitely freezing replays.
    memory_patch(0x00488855, (u8)0xC3);//RET;
    memory_patch(0x00488856, (u32)0x90909090);

    // Change quick in-game shortcuts.
    call_patch(hooks::RestartGameMenu, 0x00484785);					// Change leaderboard toggle shortcut action to mission restart shortcut action.
    memory_patch(0x00484B40, (u32)0x00484B24);						// Default switch table case (do nothing) for save game shortcut.
    memory_patch(0x00484B44, (u32)0x00484B24);						// Default switch table case (do nothing) for load game shortcut.

    // Diplomacy in singleplayer.
    
    // sub_490300 (0x00490300)
    jump_patch(hooks::sub_490300_patch_903CA, 0x004903CA, 3);		// Exclude players from the list if Race is set to Inactive or Type isn't Computer or Human.
    memory_patch(0x00490481, (u16)0x7402);							// Disable "allies" checkboxes for non-humans.
    memory_patch(0x004904F7, (u16)0x7402);							// Disable "shared vision" checkboxes for non-humans.
    
    // allyfltr_AllyVictoryBox (0x00490C40)
    call_patch(hooks::sub_4AABB0_new, 0x00490D47);					// Disable allied victory checkbox for singleplayer mode.

    // sub_490E90 (0x00490E90)
    jump_patch(hooks::sub_490E90_patch_90EC3, 0x00490EC3);			// Hide "Accept" button for singleplayer mode.

    // Note on 490ED1: Don't need to check game mode again for "Cancel" button visibility flag because it should always be the same as the "Accept" button.
    memory_patch(0x00490ED1, (u32)0x8B056A51);						// PUSH ECX; PUSH 05; MOV EAX, ...
    memory_patch(0x00490ED5, (u16)0xE8C3);							// ...EBX; CALL ...
    memory_patch(0x00490ED7, (u32)0x0003CB55);						// ...UpdateCancelButton (relative offset)

    // Note on 490EDB: Again, don't need to check game mode. "Return to Game" button visibility flag is inverse of Accept/Cancel buttons.
    memory_patch(0x00490EDB, (u32)0x5108F183);						// XOR ECX, 8; PUSH ECX;
    memory_patch(0x00490EDF, (u32)0xC38B046A);						// PUSH 4; MOV EAX, EBX;
    call_patch(hooks::UpdateOKButton2, 0x00490EE3);
    memory_patch(0x00490EE8, (u16)0xC35B);							// POP EBX; RETN;

    // minimap_dlg_Create (0x004A5230)
    jump_patch(hooks::minimap_dlg_Create_patch_A5242, 0x004A5242);
    memory_patch(0x004A526E, (u8)0x74);								// Change JBE to JE.
    
    // load_gluMinimap (0x004A5A80)
    call_patch(hooks::minimap_style, 0x004A5B82);
    call_patch(hooks::minimap_style, 0x004A5BBB);
    call_patch(hooks::minimap_style, 0x004A5C74);
    
    // genericBtnInteract (0x004E1C20)
    memory_patch(0x004E1C38, (u16)0x73FC);

    // Use new pcolor system for player messages.
    call_patch(hooks::getColourID_new, 0x0048D2EC);
    // BWFXN_DisplayText (0x0048D1C0)
    memory_patch(0x0048D2F1, (u32)0x8A08E0C1);//SHL EAX, 8; MOV...
    memory_patch(0x0048D2F5, (u32)0x48D47889);//...ECX, [ECX * 2 + 0x0048D478];
    memory_patch(0x0048D2F9, (u32)0x66C10900);//OR ECX,EAX;
    memory_patch(0x0048D2FD, (u32)0xEBFC4D89);//MOV [EBP - 4], CX; JMP...
    memory_patch(0x0048D301, (u8)0x73);//...0x0048D376;
    // DisplayTextMessage (0x0048CD30)
    memory_patch(0x0048CDC0, (u8)0x8B);//change AL to EAX
    call_patch(hooks::set_display_text_color, 0x0048CDC3, 1);
    // off_48D478
    memory_patch(0x0048D478, (u32)0x0B0F1708);

    // Show map preview in UMS. Temporary until we can do so conditionally.
    // gluChat_Main (0x004B9C20)
    memory_patch(0x004B9D48, (u32)0x90909090);
    // preview_minimap (0x004EEFD0)
    jump_patch(hooks::preview_minimap_patch_EF066, 0x004EF066);	

    // Disable mode selection after activating singleplayer/multiplayer in the main menu
    memory_patch(0x004DB6F1, (u8)0xB8);
    memory_patch(0x004DB6F2, (u32)7);

    // Fix target acquisition for decloaking units
    memory_patch(0x0049B4C3, (u8)0xFC);
    memory_patch(0x0049B4EA, (u8)0xFC);
    nop_patch(0x0049B4C6, 6);
    nop_patch(0x0049B4ED, 6);

    // Allow setting slots to Computer in UMS lobbies
    memory_patch(0x00451134, (u32)5);
    nop_patch(0x00451A2E, 2);
    nop_patch(0x00451A6D, 2);

    // Allow switching to forces with Computer slots
    memory_patch(0x004523D9, (u8)0xFF);

    // Real-time minimap updates
    nop_patch(0x004A580B, 5);

    // Fix crash on alt-F4 while 'save replay' dialog is open
    u8 Score_tMinimap[128];
    memory_patch(0x004B3918, &Score_tMinimap);
    memory_patch(0x004B4C3C, &Score_tMinimap);

    // Show player slots in singleplayer UMS previews. Not working yet.
//	memory_patch(0x004AAFD4, (u32)0x339001B2);

    // Fix briefing transmission text duration.
    memory_patch(0x0042816B, (u32)0x57001F0F);//dword ptr [EAX] (3-byte NOP); PUSH EDI;

    // Fix ability cast ranges to take into account caster collision, like weapon ranges do.
//	jump_patch(can_cast_ordered_spell_now_patch_91E64, 0x00491E64);
//	memory_patch(0x00491E73, (u8)0x0);

    // Fix Droleth teleportation bug
    memory_patch(0x004E9D99, (u32)0x20);

    // Fix speed calc tooltip
    memory_patch(0x0042D92C, (u16)0x3D89);//MOV EDI, ...
    memory_patch(0x0042D92E, (u32)0x006D11FC);//...active_iscript_unit;
    memory_patch(0x0042D935, (u16)0x36EB);//JMP 0x0042D96D

    // Fix dual-birth unit's facing direction being overridden by units.dat facing direction
    memory_patch(0x0045D9DB, (u32)0x00042811);//Change CALL address from turn_unit to update_unit_stats_finish_building.
    memory_patch(0x0045D9E2, (u32)0x0003853A);//Inverse of above.

    memory_patch(0x004EB8C4, (u8)8); // Improve melee edge performance

    memory_patch(0x004AF7B3, (u32)0x0059686F); // Skip briefings for competitive - singleplayer by reading Unit Stats game mode field
    memory_patch(0x004AAFD6, (u8)0xA);
    memory_patch(0x004D4369, (u32)0x0059686F); // Skip briefings for competitive - multiplayer

    // Remove cursor centering after singleplayer center view and do not break drag-selection.
    memory_patch(0x0047F109, (u16)0x28EB);//JMP 0x47F133
    memory_patch_df(0x0047F10B, (u8)0x90, 3);
    memory_patch(0x0047EB39, (u32)0x8AC005C6);//MOV byte ptr [0x00658AC0], ...
    memory_patch(0x0047EB3D, (u32)0xC3000065);//...0; RETN;
    memory_patch_df(0x0047EB41, (u8)0x90, 2);

    // Fix potential spin cases when iscript units decollide
    memory_patch(0x00495D05, (u32)0x7E224E3A);

    // Skip music fade delay between menus
    memory_patch(0x004A5F65, (u16)0xE990);

    // Show score screen after singleplayer quit
    memory_patch(0x004C9600, (u8)7);

    // Display rank string on structures
//	nop_patch(0x00425D99, 2); // Needs more robust handling.

    // Disable user select check for starting units.
    memory_patch(0x004CD8AD, (u8)0xEB);

    // Calculate iscript speed less frequently
    // iscript_update_speed@42D8C0
    memory_patch(0x0042D90A, (u32)1);
    nop_patch(0x0042D983, 9);
    // UMScoutPath@46AB10
    memory_patch(0x0046AB5C, (u8)1);
    memory_patch(0x0046AB94, (u8)1);

    // DarkenedFantasies, end

    // Pr0nogo, start
    // Enable "archon mode" based on map name
    jump_patch(team_game_check_wrapper, 0x004A9DC2);	

    call_patch(create_next_campaign_game_4DC1F7, 0x004DC1F7);
    // Pr0nogo, end

    // FaRTy1billion, start
    // crash fix for irregular minimap sizes
    const u8 nop3[3] = {0x8d,0x76,0x00};
    const u8 b74 = 0x74;
    u8 swtab[193];
    s32 i = 0;
    for (; i <= 64; i++) swtab[i] = 0;
    for (; i <= 128; i++) swtab[i] = 1;
    for (; i <= 192; i++) swtab[i] = 2;
    memory_patch((void*)(0x004A4424 + 2), &b74, sizeof(u8));
    memory_patch((void*)0x004A4584, swtab, sizeof(swtab));
    memory_patch((void*)0x004A441B, nop3, sizeof(nop3));

    // Extended tile groups (display and buildability mask widenings) moved to
    // hooks/display/tiles_inject.cpp with the rest of the tile pipeline.
    // FaRTy1billion, end

    // INJECTION \\

    inject_globals_hooks();
    hooks::inject_cosmonarchy_specific();
    hooks::inject_limit_hooks();
    hooks::inject_target_related_hooks();
    hooks::inject_image_hooks();
//	hooks::unitsDatWrapperPatches();

    hooks::inject_game_hooks();
    hooks::inject_replay_hooks();
    hooks::inject_draw_hook();
    hooks::inject_refresh_hooks();
    hooks::inject_framebuffer_hooks();
    hooks::inject_blit_hooks();
    hooks::inject_dialog_clip_hooks();
    hooks::inject_popup_dialog_hooks();
    hooks::inject_widget_dialog_init_hooks();
    hooks::inject_tooltip_bounds_hooks();
    hooks::inject_mapblit_hooks();
    hooks::inject_grid_hooks();
    hooks::inject_translucent_image_hooks();
    hooks::inject_screen_text_hooks();
    hooks::inject_text_print_hooks();
    hooks::inject_text_width_hooks();
    hooks::inject_wheel_hooks();
    hooks::inject_gamemap_hooks();
    hooks::inject_terrain_hooks();
    hooks::inject_tiles_hooks();
    hooks::inject_fog_hooks();
    hooks::inject_starfield_hooks();
    hooks::inject_sprite_hooks();
    hooks::inject_mouse_hooks();
    hooks::inject_camera_hooks();
    hooks::inject_frame_pacing_hooks();
    hooks::inject_loading_hook();
    hooks::inject_dat_load_hooks();

    hooks::inject_siege_transform_hooks();
    hooks::inject_buttonset_hooks();

    hooks::inject_selection_hooks();
    hooks::inject_merge_units_hooks();

    hooks::inject_larva_creep_spawn_hooks();
    hooks::inject_lift_land_hooks();
//	hooks::inject_stop_hold_position_orders_hooks();
    hooks::inject_recall_spell_hooks();
    hooks::inject_enter_nydus_hooks();
    hooks::inject_cast_order_hooks();
    hooks::inject_weapon_spell_hit_hooks();
    hooks::inject_building_zerg_hooks();
    hooks::inject_make_nydus_exit_hooks();
    hooks::inject_unit_morph_hooks();
    hooks::inject_wireframe_hooks();
    hooks::inject_die_order_hooks();
    hooks::inject_building_terran_hook();
    hooks::inject_building_protoss_hooks();
    hooks::inject_unit_train_hooks();

    hooks::inject_load_unload_proc_hooks();
    hooks::inject_load_unload_orders_hooks();
    hooks::inject_nuke_orders_hooks();
    hooks::inject_burrow_tech_hooks();
//	hooks::injectResearchUpgradeOrdersHooks();
    hooks::inject_medic_orders_hooks();
    hooks::inject_hallucination_spell_hooks();
//	hooks::inject_feedback_spell();
//	hooks::inject_btns_cond_hooks();
//	hooks::inject_receive_command_hooks();
    hooks::inject_create_initial_units_hooks();
    hooks::inject_give_unit_hooks();
    hooks::inject_train_cmdrecv_hooks();
    hooks::inject_cmdrecv_siege_tank_hooks();
//	hooks::injectCMDRECV_MergeArchonHooks();

    hooks::inject_cmdrecv_morph_hooks();
    hooks::inject_cmdrecv_stop_hooks();
    hooks::inject_cmdrecv_cancel_hooks();
    hooks::inject_repair_order_hooks();
    hooks::inject_stats_panel_display_hooks();
    hooks::inject_tyranny_spell_hooks();
    hooks::inject_cmdrecv_build_hooks();
    
    hooks::inject_apply_upgrade_flags_hooks();
    hooks::inject_attack_priority_hooks();
    hooks::inject_bunker_hooks();
    hooks::inject_cloak_nearby_units_hooks();
    hooks::inject_cloaking_tech_hooks();
//	hooks::inject_detector_hooks();
    hooks::inject_harvest_resource();

    hooks::inject_rally_hooks();
    hooks::inject_infestation_hooks();

    hooks::inject_recharge_shields_hooks();
    hooks::inject_spider_mine_hooks();

    hooks::inject_stim_packs_hooks();
    hooks::inject_tech_target_check_hooks();
//	hooks::injectTransferTechAndUpgradesHooks();
    hooks::inject_unit_speed_hooks();
    hooks::inject_update_status_effects_hooks();
    hooks::inject_update_unit_state_hooks();
    hooks::inject_weapon_cooldown_hooks();
    hooks::inject_weapon_damage_hooks();
    hooks::inject_weapon_fire_hooks();
    hooks::inject_unit_destructor_special_hooks();
    hooks::inject_weapon_effect_hooks();
    hooks::inject_use_tech_hooks();
    hooks::inject_psi_field_hooks();

    hooks::inject_armor_bonus_hooks();
    hooks::inject_sight_range_hooks();
    hooks::inject_unit_max_energy_hooks();
    hooks::inject_weapon_range_hooks();
    hooks::inject_unit_tooltip_hooks();
    hooks::inject_max_hp_hooks();
    hooks::inject_spellcaster_ai_hooks();
    hooks::inject_ai_orders_hooks();
    hooks::inject_sfx_hooks();
    hooks::inject_music_hooks();
    hooks::inject_miss_chance_hooks();
    hooks::inject_build_time_hooks();
    hooks::inject_hotkey_hooks();
//	hooks::inject_hp_bars_hooks();
    hooks::inject_ui_features_hooks();
    hooks::inject_trap_hooks();

    hooks::inject_lobby_hooks();
    hooks::inject_unit_movement_hooks();
    hooks::inject_neutral_related_hooks();
//	hooks::inject_file_operations_hooks();
    hooks::inject_glucustom_hooks();
    hooks::inject_pathfinding_hooks();
    hooks::inject_placement_hooks();
    hooks::inject_briefing_hooks();
    hooks::inject_supply_hooks();

//	hooks::inject_requirement_hooks();
    hooks::inject_new_requirements_hooks();
    hooks::inject_unit_ai_hooks();
    hooks::inject_ai_request_hooks();

    hooks::inject_attack_orders_hooks();

    hooks::inject_stop_hold_position_orders_hooks();

    hooks::inject_unit_utils_hooks();
    hooks::inject_select_larva_hooks();

    hooks::inject_cmdrecv_siege_tank_hooks();
    hooks::inject_bullet_extender();
    inject_trigger_system_hooks();
    hooks::inject_hp_bars_hooks();
    hooks::inject_image_render_hooks();
    hooks::inject_minimap_hooks();

    // Limits - most likely needs to be patched in aise, aice, and mtl
    hooks::inject_unit_extender();
    hooks::inject_sprite_extender();
    hooks::inject_order_extender();
    hooks::inject_image_extender();
    hooks::inject_thingy_extender();

    hooks::inject_orders_root_hooks();
    hooks::inject_selection_extender();
    
    hooks::inject_sight_update_hooks();
    hooks::inject_creategame_hooks();

    hooks::inject_iscript_extender();
    hooks::inject_tbl_extender();

    hooks::inject_menu_hooks();
    hooks::inject_hud_hooks();
    hooks::inject_randomization_hooks();
    hooks::inject_custom_glues();
    hooks::inject_snp_hooks();
    hooks::inject_lobby_command_hooks();
    hooks::inject_campaign_hooks();

    hooks::inject_resarea_hooks();
    
    return TRUE;
}

BOOL WINAPI Plugin::TerminatePlugin() {
    g_replay_kraken.shutdown();
    remove_win_key_hook();

    return true;
}
