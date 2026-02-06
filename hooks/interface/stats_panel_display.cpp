#include "stats_panel_display.h"

#include "BW/api.h"
#include "BW/stat_txt_spec.h"
#include "hooks/interface/buttonsets.h"
#include "trigger_system/trigger_system.h"
#include "DatExtend/datext.h"
#include "player_color.h"
#include "BW/structures/Unit.h"
#include "hooks/interface/weapon_armor_tooltip.h"
#include "hooks/interface/uifeatures/hotkeys.h"
#include "hooks/interface/console/console.h"

#include <ranges>

static const auto resource_tooltip_definitions = std::to_array<tooltip_resource_cost>({
    {tooltip_resource::Minerals, tooltip_ore_cost, &g_console_icons.mineral(), 0, true},
    {tooltip_resource::Gas, tooltip_gas_cost, &g_console_icons.gas(), 0, true},
    {tooltip_resource::Energy, tooltip_energy_cost, &g_console_icons.general(), 9, false},
    {tooltip_resource::Unused, tooltip_unused_cost, &g_console_icons.general(), 8, false},
    {tooltip_resource::BuildTime, tooltip_time_cost, &g_console_icons.general(), 7, false},
    {tooltip_resource::PSupply, tooltip_p_supply_cost, &g_console_icons.supply(), 0, true},
    {tooltip_resource::TSupply, tooltip_t_supply_cost, &g_console_icons.supply(), 0, true},
    {tooltip_resource::ZSupply, tooltip_z_supply_cost, &g_console_icons.supply(), 0, true},
});

u16 __fastcall get_stat_icon_graphic(u8 value) {
    u8 frame = 13; // 2nd stat icon frame in unit\cmdbtns\?cmdbtns.grp
    if (value >= 10) {
        frame = 16;
        if (value >= 100) {
            frame = 19 - (value < 200);
            value %= 100;
        }
        if (value >= 10) {
            frame -= value < 20;
            value %= 10;
        }
    }
    return frame - (value == 1);
}

namespace hooks {
    // Attempting to be true to the original results in unoptimized code. -GPTP
    void stats_panel_display(BinDlg* dialog) { // 00426C60
        Unit* active_unit = *g_active_portrait_unit;
        u32 index = 0;
        if (!(active_unit->status & UnitStatus::IsHallucination) || (!g_is_in_replay && active_unit->player != g_local_nation_id)) {
            bool no_air_weapon_display;
            u32 air_weapon;
            u8 air_weapon_main_unit;
            u8 air_weapon_subunit;
            u16 unit_id = helper::get_last_queue_slot_id(active_unit);
            if (units_dat::armor_upgrade[unit_id] != UpgradeId::GlobalUpgrade60) {
                helper::stats_armor_level(dialog, index);
                index++;
            }
            if (units_dat::shields_enabled[active_unit->id]) {
                helper::stats_shield_level(dialog, index);
                index++;
            }

            if (!unit_id_is_burrowed_attacker(active_unit->id) || active_unit->burrowed_attacker_can_attack()) {
                UnitExt* unit_ext = get_unit_ext(active_unit);
                bool no_ground_weapon_display = false;
                u32 ground_weapon;
                u8 ground_weapon_main_unit = units_dat::ground_weapon[active_unit->id];
                u8 ground_weapon_subunit = WeaponId::None;
                if (ground_weapon_main_unit == WeaponId::None) {
                    if (!active_unit->subunit) {
                        no_ground_weapon_display = true;
                    } else {
                        ground_weapon_subunit = units_dat::ground_weapon[active_unit->subunit->id];
                    }
                } else {
                    ground_weapon_subunit = ground_weapon_main_unit;
                }

                if (!no_ground_weapon_display) {
                    if (ground_weapon_subunit == WeaponId::None) {
                        no_ground_weapon_display = true;
                    } else if (unit_id_is_burrowed_attacker(active_unit->id) && !active_unit->burrowed_attacker_can_attack()) {
                        ground_weapon = WeaponId::None;
                    } else if (ground_weapon_main_unit != WeaponId::None) {
                        ground_weapon = ground_weapon_main_unit;
                    } else if (!active_unit->subunit) {
                        ground_weapon = WeaponId::None;
                    } else {
                        ground_weapon = units_dat::ground_weapon[active_unit->subunit->id];
                    }

                    if (!no_ground_weapon_display) {
                        helper::stats_weapon_level(dialog, index, ground_weapon);
                        index++;
                        // Display additional weapons
                        switch (active_unit->id) {
                            case UnitId::TerranCyclops:
                            case UnitId::TerranCyclopsFuturist: {
                                helper::stats_weapon_level(dialog, index, WeaponId::CyclopsBrontesPayload);
                                index++;
                            } break;
                            case UnitId::TerranTarasque: {
                                helper::stats_weapon_level(dialog, index, WeaponId::TarasqueAcceleratedPlasmid);
                                index++;
                            } break;
                            case UnitId::TerranDurendal: {
                                helper::stats_weapon_level(dialog, index, WeaponId::DurendalPhilistineSalvo);
                                index++;
                            } break;
                            case UnitId::TerranCentaur: {
                                helper::stats_weapon_level(dialog, index, WeaponId::CentaurSunchaserMissiles);
                                index++;
                            } break;
                            case UnitId::TerranDiadem: {
                                helper::stats_weapon_level(dialog, index, WeaponId::DiademHammerfallBeacon);
                                index++;
                            } break;
                            case UnitId::ProtossStarSovereign: {
                                helper::stats_weapon_level(dialog, index, WeaponId::StarSovereignInterventionMissiles);
                                index++;
                            } break;
                            case UnitId::ZergTethzorokor:
                            case UnitId::ZergZarcavrokor: {
                                helper::stats_weapon_level(dialog, index, WeaponId::ZergBonesofReflection);
                                index++;
                            } break;
                        }
                    }
                }
            }

            no_air_weapon_display = false;
            air_weapon_main_unit = units_dat::air_weapon[active_unit->id];
            if (air_weapon_main_unit != WeaponId::None) {
                air_weapon_subunit = air_weapon_main_unit;
            } else if (!active_unit->subunit) {
                no_air_weapon_display = true;
            } else {
                air_weapon_subunit = units_dat::air_weapon[active_unit->subunit->id];
            }

            if (!no_air_weapon_display) {
                if (air_weapon_subunit == WeaponId::None) {
                    no_air_weapon_display = true;
                } else if (air_weapon_main_unit != WeaponId::None) {
                    air_weapon = air_weapon_main_unit;
                } else {
                    if (!active_unit->subunit)
                    {
                        air_weapon = WeaponId::None;
                    } else {
                        air_weapon = units_dat::air_weapon[active_unit->subunit->id];
                    }
                }
                if (!no_air_weapon_display) {
                    u8 ground_weapon;
                    if (unit_id_is_burrowed_attacker(active_unit->id) && !active_unit->burrowed_attacker_can_attack()) {
                        ground_weapon = WeaponId::None;
                    } else {
                        ground_weapon = units_dat::ground_weapon[active_unit->id];
                        if (ground_weapon == WeaponId::None && active_unit->subunit) {
                            ground_weapon = units_dat::ground_weapon[active_unit->subunit->id];
                        }
                    }

                    if (air_weapon == ground_weapon) {
                        no_air_weapon_display = true;
                    } else if (air_weapon_main_unit != WeaponId::None) {
                        air_weapon = air_weapon_main_unit;
                    } else if (air_weapon_main_unit == WeaponId::None) {
                        if (!active_unit->subunit) {
                            air_weapon = WeaponId::None;
                        } else {
                            air_weapon = units_dat::air_weapon[active_unit->subunit->id]; // 00426E16
                        }
                    }
                    if (!no_air_weapon_display) {
                        helper::stats_weapon_level(dialog, index, air_weapon);
                        index++;
                    }
                }
            }
            if (active_unit->id == UnitId::ProtossSolarion || active_unit->id == UnitId::ProtossAccantorOld) { // Originally had hardcode for Gantrithor and Warbringer
                helper::stats_hangar_count(dialog, index);
                index++;
            }
            if (unit_id_is_mine_layer((UnitId::Enum)active_unit->id)) {
                if (helper::stat_spidermine_count(dialog, index)) {
                    index++;
                }
            }
            unit_id = helper::get_last_queue_slot_id(active_unit);
            if (unit_id == UnitId::TerranAnvil) {
                helper::stat_nuke_count(dialog, index);
                index++;
            }
        }

        BinDlg* current_dialog = dialog->parent;
        if (dialog->control_type == DialogControlType::DialogBoxDC) {
            current_dialog = dialog;
        }
        if (current_dialog->child_dialog) {
            current_dialog = current_dialog->child_dialog;
            while (current_dialog && current_dialog->index != index + 9) {
                current_dialog = current_dialog->next;
            }
            while (index < 4 && current_dialog) {
                helper::hide_dialog(current_dialog);
                current_dialog = current_dialog->next;
                index++;
            }
        }
    }

    // Hooked to enable/disable rank display based on new unit definitions -- Pr0nogo
    const char* get_unit_rank_string(Unit* unit) { // 0042F370
        if (key_held_this_frame(VK_MENU)) {
            static char buffer[260] = {};
            UnitExt* unit_ext = get_unit_ext(unit);
            sprintf_s(buffer, sizeof(buffer), "Damage taken: %d", unit_ext->total_damage_taken / 256);
            return buffer;
        }
        if (units_dat::base_property[unit->id] & UnitProperty::NeutralAccessories) {
            return "";
        }
        if (unit->player < PlayerLimit) {
            return g_player_data.full_player_names[unit->player];
        }
        return "";
    }

    void draw_unit_kills_string(BinDlg* dialog) { // 00425DD0
        Unit* unit = *g_active_portrait_unit;
        if (key_held_this_frame(VK_MENU)) {
            UnitExt* unit_ext = get_unit_ext(unit);
            sprintf_s(g_kills_buffer, 260, "Damage dealt: %d", unit_ext->total_damage_dealt / 256);
            helper::add_text_to_dialog(dialog, -21, g_kills_buffer);
        } else if (!bw::game_kind_supports_kills(bw::get_game_kind())) {
            if (unit->player < PlayerLimit) {
                helper::add_text_to_dialog(dialog, -21, g_player_data.full_force_names[unit->player]);
            }
        } else if (unit->kill_count) {
            const char* text = "";
            if (g_stat_txt_buffer.get_string_count32() > 763) {
                text = g_stat_txt_buffer.get_string32(764);
            }

            sprintf_s(g_kills_buffer, 260, "%s %d", text, unit->kill_count);
            helper::add_text_to_dialog(dialog, -21, g_kills_buffer);
        } else if (unit->player == g_local_nation_id && !g_is_in_replay) {
            BinDlg* actual_dialog = dialog->control_type != DialogControlType::DialogBoxDC ? dialog->parent : dialog;
            BinDlg* child = actual_dialog->child_dialog;
            while (child && child->index != 65515) {
                child = child->next;
            }
            if (child) {
                helper::hide_dialog(child);
            }
        } else {
            BinDlg* other = helper::get_control_from_index(dialog, -21);
            helper::hide_dialog(other);
        }
    }

    static void draw_new_building_info(BinDlg* dialog, Unit* unit) {
        helper::draw_armor(dialog, unit->id, 0);
        draw_unit_kills_string(dialog);
    }

    // TODO: Fix progress bar drawing during addon construction, unit training
    // Hooked to draw armor icons to buildings. - Pr0nogo
    void draw_building_info(BinDlg* dialog) { // 00427890
        Unit* unit = *g_active_portrait_unit;
        s32 shields = (*g_active_portrait_unit)->shields / 256;
        g_selection_group_hp[0] = unit->hitpoints;
        *g_ui_energy = (*g_active_portrait_unit)->energy;
        *g_ui_shields = shields;
        *g_redraw_transport_ui = helper::ui_needs_redraw();

        helper::generic_status_update_drawn_values();
        if (unit_id_is_resource_node(unit->id, false)) {
            helper::draw_gas_quantity(dialog);
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (!g_is_in_replay && !bw::unit_is_allied_or_detected(unit, g_local_nation_id)) {
            if (g_status_screen_func != 4) {
                helper::hide_optional_status_screen_controls(dialog);
                g_status_screen_func = 4;
            }
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            draw_new_building_info(dialog, unit);
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (!unit->is_completed()) {
            helper::draw_progress_bar(dialog);
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (unit->id == UnitId::TerranAnvil) {
            helper::draw_nuke_silo_info(dialog);
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            if (unit->main_order_id == OrderId::NukeTrain) {
                helper::draw_armor(dialog, unit->id, 0);
            } else {
                draw_new_building_info(dialog, unit);
            }
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (helper::unit_is_active_transport(unit)) {
            if (g_status_screen_func != 4) {
                helper::hide_optional_status_screen_controls(dialog);
                g_status_screen_func = 4;
            }
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            draw_new_building_info(dialog, unit); // For now we show this, but later when we have the real transport UI, we won't. -Veeq7
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (helper::is_queue_slot_active(unit, 0)) {
            helper::draw_training_progress(dialog);
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (helper::is_constructing_addon(unit) || helper::is_attempting_protoss_build(unit)) {
            helper::draw_addon_progress(dialog);
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (unit->building.tech_id != TechId::None) {
            helper::draw_tech_progress(dialog);
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (unit->building.upgrade_id != UpgradeId::None) {
            helper::draw_upgrade_progress(dialog);
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (helper::unit_is_geyser(unit)) {
            helper::draw_gas_quantity(dialog);
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            helper::draw_armor(dialog, unit->id, 0); // New. -Veeq7
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (units_dat::air_weapon[helper::get_last_queue_slot_id(unit)] == WeaponId::None
        &&  units_dat::ground_weapon[helper::get_last_queue_slot_id(unit)] == WeaponId::None
        && !units_dat::shields_enabled[unit->id])
        {
            if (g_status_screen_func != 4) {
                helper::hide_optional_status_screen_controls(dialog);
                g_status_screen_func = 4;
            }
            helper::set_text_string(dialog);
            helper::set_unit_status_string_text(dialog);
            draw_new_building_info(dialog, unit);
            helper::add_text_to_dialog(dialog, -5, unit->get_name());
            return;
        }
        if (g_status_screen_func != 3) {
            helper::hide_optional_status_screen_controls(dialog);
            helper::show_control_group(dialog, -20, 12);
            g_status_screen_func = 3;
        }
        helper::draw_display_buttons(dialog);
        helper::hide_dialog(helper::get_control_from_index(dialog, -21));
        helper::set_text_string(dialog);
        helper::set_unit_status_string_text(dialog);
        draw_new_building_info(dialog, unit);
        helper::add_text_to_dialog(dialog, -5, unit->get_name());
    }

    // Hooked to draw new values to icons. -- Pr0nogo
    void stats_armor_level(BinDlg* dialog, Unit* unit, u16 status_screen_index) { // 00425600
        // Second arg is unit id in one case and an empty Unit* in another, so I'm ignoring it entirely and just using g_active_portrait_unit. -Pr0nogo
        BinDlg* dialog_var = dialog;
        Unit* portrait_unit = *g_active_portrait_unit;
        s32 armor_value = units_dat::armor_amount[portrait_unit->id] + get_bonus_armor(portrait_unit, units_dat::armor_amount[portrait_unit->id]);
        sprintf_s(g_status_screen_info_button_unk[status_screen_index], 8, "%i", armor_value);
        if (dialog_var->control_type != DialogControlType::DialogBoxDC) {
            dialog_var = dialog_var->parent;
        }
        BinDlg* child = dialog_var->child_dialog;
        if (child) {
            while (child->index != status_screen_index + 9) {
                child = child->next;
                if (!child) {
                    child = 0;
                    break;
                }
            }
        } else {
            child = 0;
        }

        helper::show_dialog(child);
        helper::disable_dialog(child);
        
        GrpHead* icons_grp = g_cmdicons_grp;
        StatusScreenState* state = child->stat_button_user;
        Unit* active_unit = *g_active_portrait_unit;
        child->graphic = get_stat_icon_graphic(armor_value);
        state->button_grp = icons_grp;
        u32 final_unit_id = active_unit->build_queue[active_unit->build_queue_slot];
        if (active_unit->is_completed() || bw::is_zerg_building_morph_result(active_unit->build_queue[active_unit->build_queue_slot])) {
            final_unit_id = active_unit->id; // We also need to do this for birthing Zerg units
        }
        u16 frame = upgrades_dat::upgrade_icon[units_dat::armor_upgrade[final_unit_id]];
        if (final_unit_id == UnitId::None) {
            frame = upgrades_dat::upgrade_icon[units_dat::armor_upgrade[active_unit->id]];
        }
        state->grp_frame = frame;
        state->type = 1;
        state->id = units_dat::armor_upgrade[final_unit_id];
        child->text = g_status_screen_info_button_unk[status_screen_index];
        u32 flags = child->flags;
        if (!(child->flags & BinDlgFlags::Highlighted)) {
            child->flags = flags | BinDlgFlags::Highlighted;
            helper::update_dialog(child);
        }
    //	*g_unk_upgrade_count += armor_value;
    }

    // Hooked to draw new values to icons. - Pr0nogo
    void stats_weapon_level(BinDlg* dialog, u16 status_screen_index, u8 weapon_id) { // 00425790
        Unit* unit = *g_active_portrait_unit;
        UnitExt& unit_ext = unit->ext();
        u32 active_player = unit->player;

        u32 damage = get_modified_damage(unit, (WeaponId::Enum)weapon_id);
        u16 damage_value = damage * bw::get_damage_factor_for_tooltip(weapon_id, unit);
        sprintf_s(g_status_screen_info_button_unk[status_screen_index], 8, "%d", damage_value);

        BinDlg* dialog_var = dialog;
        if (dialog->control_type != DialogControlType::DialogBoxDC) {
            dialog_var = dialog->parent;
        }

        BinDlg* child = dialog_var->child_dialog;
        if (child) {
            while (child->index != status_screen_index + 9) {
                child = child->next;
                if (!child) {
                    child = 0;
                    break;
                }
            }
        } else {
            child = 0;
        }

        helper::show_dialog(child);
        helper::disable_dialog(child);
        StatusScreenState* state = child->stat_button_user;
        GrpHead* icons_grp = g_cmdicons_grp;
        child->graphic = get_stat_icon_graphic(damage_value);
        state->button_grp = icons_grp;
        state->grp_frame = weapons_dat::weapon_icon[weapon_id];
        state->type = 0;
        state->id = weapon_id;

        u32 flags = child->flags;
        child->text = g_status_screen_info_button_unk[status_screen_index];
        if (!(flags & 1)) {
            child->flags = flags | 1;
            helper::update_dialog(child);
        }
    //	*g_unk_upgrade_count += weapons_dat::damage_amount[weapon_id];
    }

    // Hooked to draw new values to icons. -Pr0nogo
    void stats_shield_level(BinDlg* dialog, u16 status_screen_index) { // 00425510
        BinDlg* dialog_var = dialog;
        Unit* unit = *g_active_portrait_unit;
        UnitExt* unit_ext = get_unit_ext(unit);

        u32 base_armor = units_dat::armor_amount[unit->id];
        if (unit_ext->values[ValueTriggerArmorBaseSet]) {
            base_armor = unit_ext->values[ValueTriggerArmorBaseAmount];
        }
        u32 armor_value = base_armor + get_bonus_armor(unit, units_dat::armor_amount[unit->id]);
        sprintf_s(g_status_screen_info_button_unk[status_screen_index], 8, "%d", armor_value);

        if (dialog_var->control_type != DialogControlType::DialogBoxDC) {
            dialog_var = dialog_var->parent;
        }
        BinDlg* child = dialog_var->child_dialog;
        if (child) {
            while (child->index != status_screen_index + 9) {
                child = child->next;
                if (!child) {
                    child = 0;
                    break;
                }
            }
        } else {
            child = 0;
        }

        helper::show_dialog(child);
        helper::disable_dialog(child);
        StatusScreenState* state = child->stat_button_user;
        GrpHead* icons_grp = g_cmdicons_grp;
        child->graphic = get_stat_icon_graphic(armor_value);
        state->button_grp = icons_grp;
        state->grp_frame = 310;
        state->type = 2;
        state->id = 15;

        u32 flags = child->flags;
        child->text = g_status_screen_info_button_unk[status_screen_index];
        if (!(flags & 1)) {
            child->flags = flags | 1;
            helper::update_dialog(child);
        }
    //	*g_unk_upgrade_count += armor_value;
    }

    // Hooked to correct string spacing. -Pr0nogo
    b32 stats_spidermine_count(BinDlg* dialog, u16 index) { // 00426300
        Unit* unit = *g_active_portrait_unit;
        if (helper::tech_use_allowed(unit, TechId::SpiderMines, unit->player) != 1) {
            return 0;
        }
        sprintf_s(g_status_screen_info_button_unk[index], 8, "%d", unit->mine_carrier.mine_count);
        if (dialog->control_type != DialogControlType::DialogBoxDC) {
            dialog = dialog->parent;
        }

        BinDlg* child = dialog->child_dialog;
        if (child) {
            while (child->index != index + 9) {
                child = child->next;
                if (!child) {
                    goto LABEL_8;
                }
            }
        } else {
        LABEL_8:
            child = 0;
        }
        helper::show_dialog(child);
        helper::disable_dialog(child);
        StatusScreenState* user = child->stat_button_user;
        GrpHead* grp = g_cmdicons_grp;
        child->graphic = get_stat_icon_graphic(unit->mine_carrier.mine_count);
        user->button_grp = g_cmdicons_grp;
        user->grp_frame = 13;
        user->type = 3;
        user->id = 13;

        u32 flags = child->flags;
        child->text = g_status_screen_info_button_unk[index];
        if (!(flags & BinDlgFlags::Highlighted)) {
            child->flags = flags | BinDlgFlags::Highlighted;
            helper::update_dialog(child);
        }
        return 1;
    }

    // Hooked to change the name of new building morphs.
    // Since August 2022, data extenders make use of their own hook for naming extended unit ids.
    // - Pr0nogo
    const char* get_unit_name(Unit* unit) { // 0047B5A0
        const auto unit_id = (UnitId::Enum)unit->id;
        if (unit->player < PlayerLimit && g_globals->players[unit->player].custom_unit_names.contains(unit_id)) {
            return g_globals->players[unit->player].custom_unit_names[unit_id].data();
        }

        u16 queue = unit->build_queue[unit->build_queue_slot];
        if (unit->is_completed() || !bw::is_zerg_building_morph_result(queue)) {
            return helper::get_unit_name_stat_string(unit->id);
        } else {
            return helper::get_unit_name_stat_string(queue);
        }
    }
    
    // 004594B0 0045952F - Hooked to add time costs to tooltip info. -Veeq7
    void set_tooltip_info_unit(u16 unit_id) {
        u32 mineral_cost = units_dat::mineral_cost[unit_id];
        u32 vespene_cost = units_dat::vespene_cost[unit_id];
        u32 time_cost = units_dat::time_cost[unit_id] / 24;

        u32 race = Unit::get_race(unit_id);
        u32 supply = 0;
        if (units_dat::base_property[unit_id] & UnitProperty::TwoUnitsIn1Egg) {
            supply = 2;
        }

        set_tooltip_info(mineral_cost, vespene_cost, 0, 0, 0,
            race == RaceId::Protoss ? supply : 0,
            race == RaceId::Terran ? supply : 0,
            race == RaceId::Zerg ? supply : 0);
        *tooltip_time_cost = time_cost;
    }

    // 00459360 004593B0 004597D5 - Hooked to add time costs to tooltip info. -Veeq7
    void reset_tooltip_info() {
        helper::refresh_selection_screen();

        *tooltip_ore_cost = 0;	
        *tooltip_gas_cost = 0;
        *tooltip_energy_cost = 0;
        *tooltip_unused_cost = 0;
        *tooltip_time_cost = 0;
        *tooltip_p_supply_cost = 0;
        *tooltip_t_supply_cost = 0;
        *tooltip_z_supply_cost = 0;
    }

    // 00458800 - Hooked to add time costs to tooltip info. -Veeq7
    void set_tooltip_info(u32 mineral_cost, u32 vespene_cost, u32 p_energy, u32 t_energy, u32 z_energy, u32 p_supply, u32 t_supply, u32 z_supply) {
        *tooltip_ore_cost = mineral_cost;	
        *tooltip_gas_cost = vespene_cost;
        *tooltip_energy_cost = p_energy + t_energy + z_energy;
        *tooltip_unused_cost = 0;
        *tooltip_time_cost = 0;
        *tooltip_p_supply_cost = p_supply;
        *tooltip_t_supply_cost = t_supply;
        *tooltip_z_supply_cost = z_supply;
    }

    // 00458EF0 - Hooked to use new icon grps -Jesse
    void draw_tooltip_with_icon(Box16* area) {
        const u32 space_width = 2;
        s16 max_x = space_width;
        s16 max_height = 0;

        auto has_value = [](const auto& r) { return *r.value; };
        for (const auto& resource : resource_tooltip_definitions | std::ranges::views::filter(has_value)) {
            auto index = resource.use_race_offset
                ? resource.index + race_icon_offset(static_cast<RaceId::Enum>(g_console_index))
                : resource.index;

            if (index >= resource.grp->frame_count()) index = 0;

            auto& current_frame = resource.grp->frame_headers()[index];
            s16 width = current_frame.w;
            s16 height = current_frame.h;
            if (current_frame.data_offset < 0) {
                current_frame.data_offset &= 0x7FFFFFFF;
                width += 256;
            }

            Bounds bounds{0, 0, width, height, width, height};
            FxnDrawIconData data{width, height, resource.grp->get_frame_pixel_data(index)};
            helper::bw_print_icon(&bounds, &data, max_x, area->bottom);
            helper::bw_print_xy(std::format("{}", *resource.value).c_str(),
                width + max_x + space_width, area->bottom);

            max_height = std::max(max_height, height);
            max_x = tooltip_required_area->right;
        }

        if (max_x > space_width) {
            area->right = std::max(area->right, max_x);
            area->bottom += max_height + 1;
        }
    }

    // Hooked by accident. -Pr0nogo
    s32 stat_data_unit_wireframe_selection(BinDlg* dialog, Event* event) { // 004583E0
        BinDlg* dialog_var = dialog;
        if (event->type == 3) {
            helper::status_screen_update_tooltip_mouse_held(dialog, event);
        } else if (event->type == 14) {
            switch (event->extended_command_type) {
                case 0: {
                    dialog->fxn_update = helper::stat_data_unit_wireframe_select_update;
                    dialog->graphic = 0;
                    dialog->user = (u32*)helper::SMemAlloc(8, "Starcraft\\SWAR\\lang\\statdata.cpp", 775, 0);
                } break;
                case 1: {
                    void* user = dialog->user;
                    if (user) {
                        helper::SMemFree(user, "Starcraft\\SWAR\\lang\\statdata.cpp", 780, 0);
                    }
                } break;
                case 2: {
                    helper::status_screen_button(dialog, dialog);
                    return 1;
                } break;
                case 4: {
                    if (!(dialog->flags & BinDlgFlags::Visible)) {
                        break;
                    }
                    return 1;
                } break;
                case 6: {
                    if (dialog == g_control_under_mouse) {
                        helper::refresh_selection_screen();
                        g_control_under_mouse = 0;
                    }
                } break;
            }
        }
        return generic_interact_functions[std::to_underlying(dialog_var->control_type)](dialog, event);
    }

    // Hooked to correct errors and replace building morph id checks. -Pr0nogo
    u16 get_last_queue_slot_id(Unit* unit) { // 0047B270
        if (unit->is_completed()) {
            return unit->id;
        }
        u16 queued_id = unit->build_queue[unit->build_queue_slot];
        if (!bw::is_zerg_building_morph_result(queued_id)) {
            return unit->id;
        }
        return queued_id;
    }

    // Hooked to investigate transport UI. -Pr0nogo
    void unit_stat_action_transport(BinDlg* dialog) { // 00427C90
        Unit* active_unit = *g_active_portrait_unit;
        // Disabled the block below to turn off transport UI display, since it has not yet been reimplemented. -Pr0nogo
        /*
        if ((*g_is_in_replay || active_unit->player == *g_local_nation_id)
        &&  !(active_unit->status & UnitStatus::IsHallucination)
        &&   (helper::transport_status_update_drawn_values(), helper::is_unit_loaded(active_unit)))
        {
            if (*g_status_screen_func != 11)
            {
                helper::hide_optional_status_screen_controls(dialog);
                *g_status_screen_func = 11;
            }
            helper::set_spell_special_button_graphic(dialog);
            helper::set_text_string(dialog);
            helper::get_active_portrait_unit_name(dialog);
        } else */
        if (active_unit->status & UnitStatus::GroundedBuilding) {
            helper::unit_stat_action_building(dialog);
        } else {
            helper::unit_stat_act_standard(dialog);
        }
    }

    // Hooked to show progress bars on allied and detected eggs. -Pr0nogo
    void unit_stat_act_egg(BinDlg* dialog) { // 00427260
        Unit* unit = *g_active_portrait_unit;
        if (!g_is_in_replay && !bw::unit_is_allied_or_detected(unit, g_local_nation_id) || unit->status & UnitStatus::IsHallucination) {
            helper::unit_stat_act_standard(dialog);
        } else {
            if (g_status_screen_func != 10) {
                helper::hide_optional_status_screen_controls(dialog);
                helper::show_control_group(dialog, -40, 17);
                unit = *g_active_portrait_unit;
                g_status_screen_func = 10;
            }
            u16 build_time = helper::get_remaining_build_time_percentage(unit);
            helper::dialog_set_progress_bar_value(dialog, 16, build_time);
            BinDlg* dialog_context = dialog;
            if (dialog->control_type != DialogControlType::DialogBoxDC) {
                dialog_context = dialog->parent;
            }
            BinDlg* child = dialog_context->child_dialog;
            while (child && child->index != 17) {
                child = child->next;
            }
            char* user = (char*)child->user;
            GrpHead* icons_grp = *DatExt::get_unit_icons_pointer();
            child->graphic = 2;
            *(GrpHead**)(user + 0) = icons_grp;
            *(u16*)(user + 4) = unit->build_queue[unit->build_queue_slot];
            *(u16*)(user + 6) = 3;
            *(u16*)(user + 8) = unit->build_queue[unit->build_queue_slot];

            u32 flags = child->flags;
            if (!(flags & 1)) {
                child->flags = flags | BinDlgFlags::Highlighted;
                helper::update_dialog(child);
            }

            const char* string = "Building";
            switch (unit->get_race()) {
                case RaceId::Terran:  string = "Under Construction"; break;
                case RaceId::Protoss: string = "Opening Warp Rift";  break;
                case RaceId::Zerg:    string = "Mutating";           break;
                case RaceId::Askosi:  string = "Assembling";         break;
            }
            helper::add_text_to_dialog(dialog_context, -40, string);
            helper::set_text_string(dialog_context);
            helper::add_text_to_dialog(dialog_context, -5, unit->get_name());
        }
    }

    // TODO @Veeq7 - 425F30 4260C0 4268D0 426FF0 427260 4273E0 - Make changes to these functions in order to show remaining build time seconds above progress bar.

    // Ported from GPTP to show remaining build time seconds above progress bar. -Pr0nogo
    void stats_addon_builder(BinDlg* dialog) { // 00425F30
        Unit* unit = *g_active_portrait_unit;
        if (g_is_in_replay != 0 || unit->player == g_local_nation_id) {
            if (g_status_screen_func != 9) {
                helper::hide_optional_status_screen_controls(dialog);
                helper::show_control_group(dialog, -40, 17);
                g_status_screen_func = 9;
            }

            Unit* built_unit = 0;
            if (unit->main_order_id == OrderId::BuildTerran
            ||	unit->main_order_id == OrderId::Repair1
            ||	unit->main_order_id == OrderId::ConstructingBuilding
            ||	helper::is_attempting_protoss_build(unit)) {
                built_unit = unit->order_target.unit;
            } else if (unit->secondary_order_id == OrderId::TrainUnit || unit->secondary_order_id == OrderId::TrainFighter || helper::is_constructing_addon(unit)) {
                built_unit = unit->current_build_unit;
            }

            helper::dialog_set_progress_bar_value(dialog, 16, helper::get_remaining_build_time_percentage(built_unit));
            u32 slot_unit_id = helper::get_last_queue_slot_id(built_unit);
            BinDlg* current_dialog = get_first_child_by_index(dialog, 17);
            current_dialog->graphic = 2;
            current_dialog->stat_button_user->button_grp = *DatExt::get_unit_icons_pointer();
            current_dialog->stat_button_user->grp_frame = slot_unit_id;
            current_dialog->stat_button_user->type = 3;
            current_dialog->stat_button_user->id = slot_unit_id;

            if (!(current_dialog->flags & BinDlgFlags::Highlighted)) {
                current_dialog->flags |= BinDlgFlags::Highlighted;
                helper::update_dialog(current_dialog);
            }
            const char* string = "Building";
            switch (unit->get_race()) {
                case RaceId::Terran:  string = "Under Construction"; break;
                case RaceId::Protoss: string = "Opening Warp Rift";  break;
                case RaceId::Zerg:    string = "Mutating";           break;
                case RaceId::Askosi:  string = "Assembling";         break;
            }
            helper::add_text_to_dialog(dialog, -40, string);
        }
    }

    // Ported from GPTP to show remaining build time seconds above progress bar. -Pr0nogo
    void stats_incomplete_unit(BinDlg* dialog) { // 004260C0
        Unit* unit = *g_active_portrait_unit;
        helper::show_control_group(dialog, -30, 13);
        if (g_status_screen_func != 6) {
            helper::hide_optional_status_screen_controls(dialog);
            g_status_screen_func = 6;
        }

        helper::dialog_set_progress_bar_value(dialog, 13, helper::get_remaining_build_time_percentage(unit));
        const char* string = "Building";
        switch (unit->get_race()) {
            case RaceId::Terran:  string = "Under Construction"; break;
            case RaceId::Protoss: string = "Opening Warp Rift";  break;
            case RaceId::Zerg:    string = "Mutating";           break;
            case RaceId::Askosi:  string = "Assembling";         break;
        }
        helper::add_text_to_dialog(dialog, -30, string);
    }

    // Ported from GPTP to show remaining build time seconds above progress bar. -Pr0nogo
    // Don't see any progress bar code here... is this even relevant?
    void stats_morph_progress(BinDlg* dialog, int index, u32 unk) { // 00457250
        BinDlg* current_dialog;
        if (dialog->control_type == DialogControlType::DialogBoxDC) {
            current_dialog = dialog->child_dialog;
        } else {
            current_dialog = dialog->parent->child_dialog;
        }

        while (current_dialog && current_dialog->index != (u16)index) {
            current_dialog = current_dialog->next; 
        }
        if (!(current_dialog->flags & BinDlgFlags::Visible)) {
            struct {
                u32 unknown_value;		// [EBP-0x14]	[00]
                u32 always_zero_here;	// [EBP-0x10]	[04]
                u32 not_allocated_2;	// [EBP-0x0C]	[08]
                u16 always_14_here;		// [EBP-0x08]	[0C]
                s16 mouse_x;			// [EBP-0x06]	[0E]
                s16 mouse_y;			// [EBP-0x04]	[10]
                u16 not_allocated_1;	// [EBP-0x02]	[12]
            } stack_placeholder;

            // Not allocated on original code, but result is probably more true to the original like this. -GPTP
            stack_placeholder.not_allocated_1 = 0;
            stack_placeholder.not_allocated_2 = 0;

            stack_placeholder.mouse_y = g_mouse->y;
            stack_placeholder.always_14_here = 14;
            stack_placeholder.unknown_value = 13;
            stack_placeholder.always_zero_here = 0;
            stack_placeholder.mouse_x = g_mouse->x;

            if (helper::fxn_interact(current_dialog, (u32)&stack_placeholder) != 0 && !(current_dialog->flags & BinDlgFlags::Highlighted)) {
                current_dialog->flags |= BinDlgFlags::Highlighted;
                helper::update_dialog(current_dialog);
            }
        }
        if (current_dialog->index == unk) {
            current_dialog = 0;
        } else {
            current_dialog = current_dialog->next;
        }
    }

    // Ported from GPTP to show remaining build time seconds above progress bar. -Pr0nogo
    void stats_nuke_silo(BinDlg* dialog) { // 00426FF0
        Unit* unit = *g_active_portrait_unit;
        BinDlg* current_dialog;
        if (units_dat::base_property[unit->build_queue[unit->build_queue_slot % 5]] & UnitProperty::Building) { // Originally checked for a Disruption Web ID range
            if (g_status_screen_func != 3) {
                helper::hide_optional_status_screen_controls(dialog);
                helper::show_control_group(dialog, -20, 12);
                g_status_screen_func = 3;
            }

            helper::draw_display_buttons(dialog);
            if (dialog->control_type == DialogControlType::DialogBoxDC) {
                current_dialog = dialog->child_dialog;
            } else {
                current_dialog = dialog->parent->child_dialog;
            }

            while (current_dialog && (s16)current_dialog->index != -20) {
                current_dialog = current_dialog->next;
            }

            helper::hide_dialog(current_dialog);
            if (dialog->control_type == DialogControlType::DialogBoxDC) {
                current_dialog = dialog->child_dialog;
            } else {
                current_dialog = dialog->parent->child_dialog;
            }

            while (current_dialog && (s16)current_dialog->index != -21) {
                current_dialog = current_dialog->next;
            }

            helper::hide_dialog(current_dialog);
        } else { // TODO: Something in this half of the hook is cucko. -Pr0nogo
            if (dialog->control_type == DialogControlType::DialogBoxDC) {
                current_dialog = dialog->child_dialog;
            } else {
                current_dialog = dialog->parent->child_dialog;
            }

            while (current_dialog && (s16)current_dialog->index != 2) {
                current_dialog = current_dialog->next;
            }

            if (g_status_screen_func != 14) {
                helper::hide_optional_status_screen_controls(dialog);
                g_status_screen_func = 14;
            }

            Unit* target = 0;
            if (unit->main_order_id == OrderId::BuildTerran
            ||	unit->main_order_id == OrderId::Repair1
            ||	unit->main_order_id == OrderId::ConstructingBuilding
            ||	helper::is_attempting_protoss_build(unit))
            {
                target = unit->order_target.unit;
            } else if (unit->secondary_order_id == OrderId::TrainUnit || unit->secondary_order_id == OrderId::TrainFighter || helper::is_constructing_addon(unit)) {
                target = unit->current_build_unit;
            }

            u32 progress = 0;
            u16 built_unit_id = 0;
            if (target) {
                progress = helper::get_remaining_build_time_percentage(target);
                built_unit_id = target->id;
            }
            helper::dialog_set_progress_bar_value(dialog, 7, progress);
            helper::bw_sprintf_s(g_queues_buffers[0], 8, FORMATSTRING_VALUE_SPACE, 1); // active slot number display

            current_dialog->graphic = 2;
            current_dialog->stat_button_user->button_grp = *DatExt::get_unit_icons_pointer();
            current_dialog->stat_button_user->grp_frame = built_unit_id;
            current_dialog->stat_button_user->type = PanelTooltipTypes::SpecialPanelTooltip;
            current_dialog->stat_button_user->id = built_unit_id;
            helper::enable_dialog(current_dialog);
            current_dialog->text = g_queues_buffers[0];
            helper::show_dialog(current_dialog);

            if (!(current_dialog->flags & BinDlgFlags::Highlighted)) {
                current_dialog->flags |= BinDlgFlags::Highlighted;
                helper::update_dialog(current_dialog);
            }
            helper::add_text_to_dialog(dialog, -15, (char*)g_stat_txt_tbl->get_string32(771 + unit->get_race())); // Zerg: "Morphing", Terran: "Building", Protoss: "Opening Warp Gate"
        }
    }

    // Ported from GPTP to show remaining build time seconds above progress bar. NOT TESTED because we don't use it anymore. -Pr0nogo
    // Used by original Archon and Mind Tyrant. Probably display function 5 in FireGraft. -GPTP
    void unit_stat_act_archon(BinDlg* dialog) { // 004273E0
        Unit* unit = *g_active_portrait_unit;
        if ((g_is_in_replay == 0 && unit->player != g_local_nation_id) || unit->main_order_id != OrderId::CompletingArchonSummon) {
            helper::unit_stat_act_standard(dialog);
        } else {
            helper::show_control_group(dialog, -30, 13);
            if (g_status_screen_func != 6) {
                helper::hide_optional_status_screen_controls(dialog);
                g_status_screen_func = 6;
            }
            helper::dialog_set_progress_bar_value(dialog, 13, helper::get_remaining_build_time_percentage(unit));

            // Skipping hardcoded use of stat_txt_tbl. -GPTP			
            helper::add_text_to_dialog(dialog, -30, "Summoning");
            helper::set_text_string(dialog);
            helper::add_text_to_dialog(dialog, -5, get_unit_name(unit));
        }
    }

    // Ported from GPTP. -Pr0nogo
    const int TEXTLABELINDEX_KILLS = -21;
    const int TEXTLABELINDEX_UNITSTATUS = -20;
    const int TEXTLABELINDEX_ENERGY_TIMER = -8;
    const int TEXTLABELINDEX_HP_SHIELDS = -7;
    
    // Ported from GPTP to show timed life beneath Rililisks. -Pr0nogo
    void stats_text_energy_timer(BinDlg* dialog) { // 00425A30
        Unit* unit = *g_active_portrait_unit;
        if (!(units_dat::base_property[unit->id] & UnitProperty::Spellcaster)
        ||	  unit->status & UnitStatus::IsHallucination
        ||	(g_is_in_replay == 0 && unit->player != g_local_nation_id))
        {
            if ((unit->status & UnitStatus::IsHallucination || unit_id_is_broodspawn(unit->id)) && (g_is_in_replay != 0 || unit->player == g_local_nation_id)) {
                u32 timer_max = 0; // 1/10 of internal value for display
                if (unit->status & UnitStatus::IsHallucination) {
                    timer_max = 135;
                } else if (unit_id_is_broodspawn(unit->id)) {
                    timer_max = 11;
                }

                u32 current_timer = unit->remove_timer / 10;
                if (current_timer < 0) {
                    current_timer++;
                }
                if (timer_max < 0) {
                    timer_max++;
                }

                helper::SC_sprintf_s_2(g_text_buffer, 32, FORMATSTRING_COLORWHITE_CURRENTVALUE_MAXVALUE, current_timer, timer_max);
                helper::add_text_to_dialog(dialog, TEXTLABELINDEX_ENERGY_TIMER, g_text_buffer);

            } else {
                helper::add_text_to_dialog(dialog, TEXTLABELINDEX_ENERGY_TIMER, 0);
            }
        } else {
            helper::SC_sprintf_s_2(g_text_buffer, 32, FORMATSTRING_COLORWHITE_CURRENTVALUE_MAXVALUE, unit->energy / 256, unit->get_max_energy() / 256 );
            helper::add_text_to_dialog(dialog, TEXTLABELINDEX_ENERGY_TIMER, g_text_buffer);
        }
    }

    // Hooked for 1/256 chance of BETTER unit names. -Pr0nogo
    void unit_stat_act_default(BinDlg* dialog) { // 00425EE0
        g_status_screen_func = 0;
        BinDlg* final_dialog = dialog;
        if (dialog->control_type == DialogControlType::DialogBoxDC) {
            final_dialog = *(BinDlg**)&dialog->child_dialog;
        }

        for (; final_dialog; final_dialog = final_dialog->next) {
            helper::hide_dialog(final_dialog);
        }

        Unit* unit = *g_active_portrait_unit;
        helper::add_text_to_dialog(dialog, -5, unit->get_name());
    }

    // Hooked for 1/256 chance of BETTER unit names. -Veeq7
    void unit_stat_act_standard(BinDlg* dialog) { // 00426F50
        Unit* unit = *g_active_portrait_unit;
        g_selection_group_hp[0] = unit->hitpoints;
        *g_ui_shields = unit->shields >> 8;
        *g_ui_energy = unit->energy >> 8;
        helper::generic_status_update_drawn_values();
        if (g_status_screen_func != 2) {
            helper::hide_optional_status_screen_controls(dialog);
            helper::show_control_group(dialog, -20, 12);
            g_status_screen_func = 2;
        }
        *g_unk_upgrade_count = 1;
        helper::draw_display_buttons(dialog);
        helper::set_unit_status_string_text(dialog);
        helper::set_kills_string_text(dialog);
        helper::set_text_string(dialog);
        helper::add_text_to_dialog(dialog, -5, unit->get_name());
    }

    // Hooked to investigate bugged addon status screen displays. -Pr0nogo
    // Not working yet...
    void __cdecl draw_status_screen() { // 00458120
        Unit* active_unit = *g_active_portrait_unit;
        if (active_unit) {
            if (g_client_selection_count != 1) {
                if (g_status_screen_redraw || helper::is_selection_group_updated()) {
                    helper::status_update_multiselection(g_statdata_dialog);
                    helper::status_update_tooltip();
                }
                g_status_screen_redraw = 0;
                return;
            }
            if (!g_status_screen_redraw) {
            /*	b32* function = (unit_stat_cond*)g_unit_stat_funcs_table[active_unit->id].unit_stat_cond_func();
                if (!function) {
                    g_status_screen_redraw = 0;
                    return;
                }*/
                active_unit = *g_active_portrait_unit; // Maybe redone in case of selection update mid-func? -Pr0nogo
            }
        //	(unit_stat_act*)g_unit_stat_funcs_table[active_unit->id].unit_stat_act_func(g_statdata_dialog);
            helper::status_update_single_wireframe();
            helper::status_update_tooltip();
            g_status_screen_redraw = 0;
            return;
        }
        if (!g_status_screen_redraw) {
            g_status_screen_redraw = 0;
            return;
        }
        
        BinDlg* stat_dialog = g_statdata_dialog;
        if (stat_dialog->control_type == DialogControlType::DialogBoxDC) {
            stat_dialog = *(BinDlg**)&g_statdata_dialog->child_dialog;
        }
        for (; stat_dialog; stat_dialog = stat_dialog->next) {
            helper::hide_dialog(stat_dialog);
        }
        g_status_screen_func = 0;
        g_status_screen_redraw = 0;
    }

    // Hooked to convert stat_txt.tbl to txt, and to improve in-game displays. -Pr0nogo
    void set_unit_status_string_text(BinDlg* dialog) { // 00425B50
        Unit* unit = *g_active_portrait_unit;
        if (!unit->is_completed()) {
            return;
        }
        if (unit->is_hallucination() && (bw::in_replay_or_witness() || unit->player == g_local_nation_id)) {
            helper::add_text_to_dialog(dialog, -20, "Hallucination");
            return;
        }

        if (unit->is_building()) {
            bool progress_bar_exists = helper::unit_is_training_or_morphing(unit) || unit->is_upgrading_or_researching();
            bool can_see_progress_bar = unit->player == g_local_nation_id || bw::is_allied_to(unit->player, g_local_nation_id)
                                     || bw::in_replay_or_witness()        || player_detects_unit(g_local_nation_id, unit);

            if (progress_bar_exists && can_see_progress_bar) {
                const char* string = "Training";
                switch (unit->get_race()) {
                    case RaceId::Terran:  string = "Deploying"; break;
                    case RaceId::Protoss: string = "Warping";   break;
                    case RaceId::Zerg:    string = "Morphing";  break;
                    case RaceId::Askosi:  string = "Raising";   break;
                }
                helper::add_text_to_dialog(dialog, -20, string);
                return;
            }
            if (unit->is_frozen()) {
                helper::add_text_to_dialog(dialog, -20, "Disabled");
            } else {
                helper::add_text_to_dialog(dialog, -20, helper::get_unit_rank_string(unit));
            }
        } else {
            helper::add_text_to_dialog(dialog, -20, helper::get_unit_rank_string(unit));
        }
    }

    // Hooked to convert stat_txt.tbl to txt. -Pr0nogo
    void draw_resource_amount(BinDlg* dialog) { // 00426190
        Unit* unit = *g_active_portrait_unit;
        g_supplies_available_temp = unit->building.resource.resource_amount;
        if (g_status_screen_func != 15) {
            helper::hide_optional_status_screen_controls(dialog);
            helper::show_control_group(dialog, -10, -13);
            g_status_screen_func = 15;
        }
        helper::add_text_to_dialog(dialog, -10, 0);
        helper::add_text_to_dialog(dialog, -12, 0);
        helper::add_text_to_dialog(dialog, -13, 0);
        if (unit_id_is_minerals(unit->id)) {
            sprintf_s(g_struct_char.buf, 260, "%s %d", g_stat_txt_buffer.get_string32(796), unit->building.resource.resource_amount);
            helper::add_text_to_dialog(dialog, -11, g_struct_char.buf);
            return;
        }

        u16 resources = unit->building.resource.resource_amount;
        if (resources) {
            sprintf_s(g_struct_char.buf, 260, "%s %d", g_stat_txt_buffer.get_string32(797), resources);
            helper::add_text_to_dialog(dialog, -11, g_struct_char.buf);
        } else {
            helper::add_text_to_dialog(dialog, -11, g_stat_txt_buffer.get_string32(798));
        }

        if (units_dat::shields_enabled[unit->id]) {
            helper::stats_shield_level(dialog, 0);
            return;
        }
        if (dialog->control_type != DialogControlType::DialogBoxDC) {
            dialog = dialog->parent;
        }
        BinDlg* child_dialog = dialog->child_dialog;
        if (child_dialog) {
            while (child_dialog->index != 9) {
                child_dialog = child_dialog->next;
                if (!child_dialog) {
                    child_dialog = 0;
                    helper::hide_dialog(child_dialog);
                    return;
                }
            }
        } else {
            child_dialog = 0;
        }
        helper::hide_dialog(child_dialog);
    }

    // Hooked to convert stat_txt.tbl to txt. -Pr0nogo
    void draw_upgrade_progress(BinDlg* dialog) { // 00426500
        Unit* unit = *g_active_portrait_unit;
        if (!g_is_in_replay && !unit->player == g_local_nation_id) {
            return;
        }
        if (g_status_screen_func != 8) {
            helper::hide_optional_status_screen_controls(dialog);
            helper::show_control_group(dialog, -35, 15);
            g_status_screen_func = 8;
        }
        u32 time_cost = helper::get_upgrade_time_cost(unit->player, unit->building.upgrade_id);
        helper::dialog_set_progress_bar_value(dialog, 14, 100 * (time_cost - unit->building.upgrade_research_time) / time_cost);

        BinDlg* active_dialog = dialog;
        if (dialog->control_type != DialogControlType::DialogBoxDC) {
            active_dialog = dialog->parent;
        }
        BinDlg* child_dialog = active_dialog->child_dialog;
        if (child_dialog) {
            while (child_dialog->index != 15) {
                child_dialog = child_dialog->next;
                if (!child_dialog) {
                    child_dialog = 0;
                }
            }
        } else {
            child_dialog = 0;
        }

        StatusScreenState* user = child_dialog->stat_button_user;
        child_dialog->graphic = 2;
        user->button_grp = g_cmdicons_grp;
        user->grp_frame = upgrades_dat::upgrade_icon[unit->building.upgrade_id];
        user->type = 5;
        user->id = unit->building.upgrade_id;
        helper::enable_dialog(child_dialog);
        if (!(child_dialog->flags & BinDlgFlags::Highlighted)) {
            child_dialog->flags |= BinDlgFlags::Highlighted;
            helper::update_dialog(child_dialog);
        }

        auto race = unit->get_race();
        u32 string_id = race + 765; // Upgrading string
        if (unit->is_frozen()) {
            string_id = race + 828; // Disabled / Unpowered
        }
        helper::add_text_to_dialog(dialog, -35, g_stat_txt_buffer.get_string32(string_id));
    }

    // Hooked to fix the hangar unit icon pointer. -Pr0nogo
    void stats_hangar_count(BinDlg* dialog, u32 iterations) { // 004253D0
        Unit* unit = *g_active_portrait_unit;
        u8 count = 0;
        if (unit->id == UnitId::ProtossSolarion) {
            count = unit->unit_with_hangar.in_hangar_count + unit->unit_with_hangar.out_hangar_count;
        }
        sprintf_s(g_status_screen_info_button_unk[iterations], 8, "%d ", count);
        BinDlg* main_dialog = dialog;
        if (dialog->control_type != DialogControlType::DialogBoxDC) {
            main_dialog = dialog->parent;
        }
        BinDlg* final_dialog = main_dialog->child_dialog;
        if (final_dialog) {
            while (final_dialog->index != iterations + 9) {
                final_dialog = final_dialog->next;
                if (!final_dialog) {
                    final_dialog = 0;
                    break;
                }
            }
        } else {
            final_dialog = 0;
        }
        helper::show_dialog(final_dialog);
        helper::disable_dialog(final_dialog);
        StatusScreenState* state = final_dialog->stat_button_user;
        final_dialog->graphic = 12;
        state->button_grp = g_cmdicons_grp;
        u32 hangar_unit_id = 0;
        if (unit->id == UnitId::ProtossSolarion) {
            hangar_unit_id = UnitId::ProtossSteward;
        }
        state->grp_frame = hangar_unit_id;
        state->type = 3;
        state->id = hangar_unit_id;
        final_dialog->text = g_status_screen_info_button_unk[iterations];
        if (!(final_dialog->flags & BinDlgFlags::Highlighted)) {
            final_dialog->flags |= BinDlgFlags::Highlighted;
            helper::update_dialog(final_dialog);
        }
    }
}