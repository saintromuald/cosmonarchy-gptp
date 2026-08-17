#include "custom_glues.h"
#include "BW/ui.h"
#include "BW/enumerations.h"
#include "BW/bwhelper.h"
#include "config.h"
#include "hooks/race_randomization.h"

#include <algorithm>

static std::unique_ptr<CompositeBinDlg> g_options_bin;
static u32 misc_tab;
static u32 ui_tab;

static void load_checkbox(BinDlg* dialog, GluOptionsControls control, bool value) {
    BinDlg* control_dialog = get_first_child_by_index(dialog, control);
    if (!control_dialog) return;
    control_dialog->checkBox0x36.is_selected = value;
}

static void apply_checkbox(BinDlg* dialog, GluOptionsControls control, bool &value) {
    BinDlg* control_dialog = get_first_child_by_index(dialog, control);
    if (!control_dialog) return;
    value = control_dialog->checkBox0x36.is_selected;
}

static void load_uibool(BinDlg* dialog, GluOptionsControls control, UiBool &uibool) {
    BinDlg* control_dialog = get_first_child_by_index(dialog, control);
    if (!control_dialog) return;
    quick_send_event(control_dialog, DialogUserType_SetSelectionIndex, std::to_underlying(uibool.value));
    helper::scroll_selection_into_view(control_dialog, std::to_underlying(uibool.value));
}

static void apply_uibool(BinDlg* dialog, GluOptionsControls control, UiBool &uibool) {
    BinDlg* control_dialog = get_first_child_by_index(dialog, control);
    if (!control_dialog) return;
    switch (control_dialog->ComboBox0x46.selection_index) {
        case 0: {
            uibool.value = EUiBool::False;
        } break;
        case 1: {
            uibool.value = EUiBool::InGameOnly;
        } break;
        case 2: {
            uibool.value = EUiBool::WitnessOnly;
        } break;
        case 3: {
            uibool.value = EUiBool::True;
        } break;
    }
}

static void load_options(BinDlg* dialog) {
    // Misc Tab
    // Selective Random options
    auto& race_randomizer = RaceRandomizer::instance();
    auto race_flags = race_randomizer.race_flags();
    load_checkbox(dialog, GluOptionsControls::RandomZCheck, has_flag(race_flags, RaceFlags::Zerg));
    load_checkbox(dialog, GluOptionsControls::RandomTCheck, has_flag(race_flags, RaceFlags::Terran));
    load_checkbox(dialog, GluOptionsControls::RandomPCheck, has_flag(race_flags, RaceFlags::Protoss));
    load_checkbox(dialog, GluOptionsControls::RandomACheck, has_flag(race_flags, RaceFlags::Askosi));

    // Launch Options
    load_checkbox(dialog, GluOptionsControls::LaunchDisableWmodePrompt, g_config.disable_wmode_prompt);
    load_checkbox(dialog, GluOptionsControls::LaunchWmodePrefer, g_config.prefer_wmode);
    load_checkbox(dialog, GluOptionsControls::SupressWindowsKey, g_config.suppress_windows_key);

    // Audiovisuals
    load_checkbox(dialog, GluOptionsControls::Celebrations, g_config.celebrations_enabled);
    load_checkbox(dialog, GluOptionsControls::FlashWindow, g_config.flash_window_lobby_join);

    // Replays
    load_checkbox(dialog, GluOptionsControls::ReplayAutoSave, g_config.auto_save_replays);
    load_checkbox(dialog, GluOptionsControls::ReplaySanitize, g_config.sanitize_replay_chat);

    // Debug
    load_checkbox(dialog, GluOptionsControls::DebugVerbose, g_config.verbose);
    load_checkbox(dialog, GluOptionsControls::DebugRandoms, g_config.log_randoms);

    // UI Tab
    // General UI
    load_checkbox(dialog, GluOptionsControls::ExtendedCameraHotkeys, g_config.extended_camera_hotkeys);
    load_checkbox(dialog, GluOptionsControls::ShowTimer, g_config.show_timer);
    load_checkbox(dialog, GluOptionsControls::ShowAPM, g_config.show_apm);
    load_checkbox(dialog, GluOptionsControls::AlwaysShowExtendedTooltips, g_config.always_show_extended_tooltips);

    // Witness and Replay UI
    load_checkbox(dialog, GluOptionsControls::ShowReplayProgressBar, g_config.show_replay_progress_bar);

    // Selection UI
    load_uibool(dialog, GluOptionsControls::ShowRallyLines, g_config.show_rally_lines);
    load_uibool(dialog, GluOptionsControls::ShowOrderLines, g_config.show_order_lines);
    load_uibool(dialog, GluOptionsControls::ShowQueuedOrderLines, g_config.show_queued_order_lines);
    load_uibool(dialog, GluOptionsControls::ShowUniqueGasOrderLines, g_config.show_unique_gas_order_line_color);
    load_uibool(dialog, GluOptionsControls::ShowAuraCircles, g_config.show_aura_circles);
    load_uibool(dialog, GluOptionsControls::ShowAbilityRanges, g_config.show_ability_ranges);
    load_uibool(dialog, GluOptionsControls::AlwaysCondensedSelection, g_config.always_show_condensed_selection);
    load_uibool(dialog, GluOptionsControls::AltForSelectionUi, g_config.use_alt_for_selection_ui);

    // Minimap UI
    load_uibool(dialog, GluOptionsControls::ShowMinimapCameras, g_config.show_minimap_cameras);
    load_uibool(dialog, GluOptionsControls::AllowMinimapTerrainToggle, g_config.can_toggle_minimap_terrain);

    // Team UI
    load_checkbox(dialog, GluOptionsControls::ShowTeamUi, g_config.show_team_ui);
    load_checkbox(dialog, GluOptionsControls::ShowTeamUiApm, g_config.show_team_ui_apm);
    load_checkbox(dialog, GluOptionsControls::ShowTeamUiNames, g_config.show_team_ui_names);
    load_checkbox(dialog, GluOptionsControls::ShowTeamUiSelf, g_config.show_team_ui_self);
    load_checkbox(dialog, GluOptionsControls::ShowGlobalQueue, g_config.show_ingame_global_queue);
}

static void apply_options(BinDlg* dialog) {
    auto& race_randomizer = RaceRandomizer::instance();
    RaceFlags raceflags = RaceFlags::None;

    auto control = get_first_child_by_index(dialog, GluOptionsControls::RandomZCheck);
    if (control && control->checkBox0x36.is_selected) raceflags |= RaceFlags::Zerg;
    control = get_first_child_by_index(dialog, GluOptionsControls::RandomTCheck);
    if (control && control->checkBox0x36.is_selected) raceflags |= RaceFlags::Terran;
    control = get_first_child_by_index(dialog, GluOptionsControls::RandomPCheck);
    if (control && control->checkBox0x36.is_selected) raceflags |= RaceFlags::Protoss;
    control = get_first_child_by_index(dialog, GluOptionsControls::RandomACheck);
    if (control && control->checkBox0x36.is_selected) raceflags |= RaceFlags::Askosi;

    race_randomizer.set_race_flags(raceflags);
    race_randomizer.update_config();
    race_randomizer.cleanup_and_reset();

    // Misc Tab
    // Launch Options
    apply_checkbox(dialog, GluOptionsControls::LaunchDisableWmodePrompt, g_config.disable_wmode_prompt);
    apply_checkbox(dialog, GluOptionsControls::LaunchWmodePrefer, g_config.prefer_wmode);
    apply_checkbox(dialog, GluOptionsControls::SupressWindowsKey, g_config.suppress_windows_key);

    // Audiovisuals
    apply_checkbox(dialog, GluOptionsControls::Celebrations, g_config.celebrations_enabled);
    apply_checkbox(dialog, GluOptionsControls::FlashWindow, g_config.flash_window_lobby_join);

    // Replays
    apply_checkbox(dialog, GluOptionsControls::ReplayAutoSave, g_config.auto_save_replays);
    apply_checkbox(dialog, GluOptionsControls::ReplaySanitize, g_config.sanitize_replay_chat);

    // Debug
    apply_checkbox(dialog, GluOptionsControls::DebugVerbose, g_config.verbose);
    apply_checkbox(dialog, GluOptionsControls::DebugRandoms, g_config.log_randoms);

    // UI Tab
    // General UI
    apply_checkbox(dialog, GluOptionsControls::ExtendedCameraHotkeys, g_config.extended_camera_hotkeys);
    apply_checkbox(dialog, GluOptionsControls::ShowTimer, g_config.show_timer);
    apply_checkbox(dialog, GluOptionsControls::ShowAPM, g_config.show_apm);
    apply_checkbox(dialog, GluOptionsControls::AlwaysShowExtendedTooltips, g_config.always_show_extended_tooltips);

    // Witness and Replay UI
    apply_checkbox(dialog, GluOptionsControls::ShowReplayProgressBar, g_config.show_replay_progress_bar);

    // Selection UI
    apply_uibool(dialog, GluOptionsControls::ShowRallyLines, g_config.show_rally_lines);
    apply_uibool(dialog, GluOptionsControls::ShowOrderLines, g_config.show_order_lines);
    apply_uibool(dialog, GluOptionsControls::ShowQueuedOrderLines, g_config.show_queued_order_lines);
    apply_uibool(dialog, GluOptionsControls::ShowUniqueGasOrderLines, g_config.show_unique_gas_order_line_color);
    apply_uibool(dialog, GluOptionsControls::ShowAuraCircles, g_config.show_aura_circles);
    apply_uibool(dialog, GluOptionsControls::ShowAbilityRanges, g_config.show_ability_ranges);
    apply_uibool(dialog, GluOptionsControls::AlwaysCondensedSelection, g_config.always_show_condensed_selection);
    apply_uibool(dialog, GluOptionsControls::AltForSelectionUi, g_config.use_alt_for_selection_ui);

    // Minimap UI
    apply_uibool(dialog, GluOptionsControls::ShowMinimapCameras, g_config.show_minimap_cameras);
    apply_uibool(dialog, GluOptionsControls::AllowMinimapTerrainToggle, g_config.can_toggle_minimap_terrain);

    // Team UI
    apply_checkbox(dialog, GluOptionsControls::ShowTeamUi, g_config.show_team_ui);
    apply_checkbox(dialog, GluOptionsControls::ShowTeamUiApm, g_config.show_team_ui_apm);
    apply_checkbox(dialog, GluOptionsControls::ShowTeamUiNames, g_config.show_team_ui_names);
    apply_checkbox(dialog, GluOptionsControls::ShowTeamUiSelf, g_config.show_team_ui_self);
    apply_checkbox(dialog, GluOptionsControls::ShowGlobalQueue, g_config.show_ingame_global_queue);

    save_config();
}

static u32 __fastcall uibool_combobox_interact(BinDlg* dialog, DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
            case DialogUserType_Create: {
                dialog->flags |= DialogFlags::PropagateUpdate;
                for (auto name : ui_bool_names) {
                    auto index = helper::combobox_add_item(dialog, BinDlgComboboxFlags::ProvideStrings, name);
                    dialog->combobox_output_array_u32[index] = index;
                }
                if (dialog->flags & DialogFlags::PropagateUpdate) {
                    dialog->flags &= ~DialogFlags::PropagateUpdate;
                    helper::generic_combobox_update(dialog);
                }
            } break;
            case DialogUserType_InitChildren: {
                dialog->flags |= DialogFlags::RemoveStyles;
                helper::generic_combobox_interact(dialog, event);
            } break;
        }
    }
    return helper::generic_combobox_interact(dialog, event);
}

static void enable_tabs(BinDlg* dialog) {
    enable_dialogs_with_index(dialog, GluOptionsControls::MiscTabButton);
    enable_dialogs_with_index(dialog, GluOptionsControls::UiTabButton);
    //enable_dialogs_with_index(g_options_bin->bin_dlg(), GluOptionsControls::HotkeysTabButton);
    //enable_dialogs_with_index(g_options_bin->bin_dlg(), GluOptionsControls::ColorsTabButton);
}

static void highlight_tab(BinDlg* dialog, GluOptionsControls control) {
    hide_dialogs_with_index(dialog, GluOptionsControls::MiscTabHighlight);
    hide_dialogs_with_index(dialog, GluOptionsControls::UiTabHighlight);
    hide_dialogs_with_index(dialog, GluOptionsControls::HotkeysTabHighlight);
    hide_dialogs_with_index(dialog, GluOptionsControls::ColorsTabHighlight);
    show_dialogs_with_index(dialog, control);
}

static void options_on_init_children(BinDlg* dialog, DialogEvent* event) {
    for (auto child = dialog->child_dialog; child; child = child->next) {
        if (child->control_type == DialogControlType::ComboBox) {
            child->interaction_function = uibool_combobox_interact;
        }
    }
    bindlg_load_background_pcx(dialog, "glue\\dynamic\\backgrounds\\options.pcx", true);
}

static u32 options_on_activate(BinDlg* dialog, DialogEvent* event) {
    switch (static_cast<GluOptionsControls>(last_control_index)) {
        case GluOptionsControls::MiscTabButton: {
            g_options_bin->show_only(misc_tab);
            highlight_tab(dialog, GluOptionsControls::MiscTabHighlight);
            return 1;
        } break;
        case GluOptionsControls::UiTabButton: {
            g_options_bin->show_only(ui_tab);
            highlight_tab(dialog, GluOptionsControls::UiTabHighlight);
            return 1;
        } break;
        case GluOptionsControls::OK: {
            apply_options(dialog);
        };
    }
    return helper::generic_dialog_interact(dialog, event);
}

static u32 __fastcall dlgproc_glu_options(BinDlg* dialog, DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
        case DialogUserType_Create: {
            bindlg_load_background_palette(dialog);
            g_options_bin->show_only(misc_tab);
            highlight_tab(dialog, GluOptionsControls::MiscTabHighlight);
            load_options(dialog);
            return helper::generic_dialog_interact(dialog, event);
        } break;
        case DialogUserType_Activate: {
            return options_on_activate(dialog, event);
        } break;
        case DialogUserType_Destroy: {
        } break;
        case DialogUserType_InitChildren: {
            options_on_init_children(dialog, event);
        } break;
        }
    }
    return helper::generic_dialog_interact(dialog, event);
}

void glu_options() {
    g_options_bin = std::make_unique<CompositeBinDlg>("rez\\gluoptions.bin");

    misc_tab = g_options_bin->add_tab("rez\\options\\tab-misc.bin", 55, 95, 0);
    ui_tab = g_options_bin->add_tab("rez\\options\\tab-ui.bin", 55, 95, 0);

    helper::glu_load_bin_dialog(g_options_bin->release(), dlgproc_glu_options);

    g_gl_glues_mode = MenuId::GlueMainMenu;
    change_menu();
    g_options_bin.reset();
}