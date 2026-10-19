#include "game_tooltip.h"

#include "BW/api.h"
#include "BW/bwdata.h"
#include "BW/bwhelper.h"
#include "BW/structures.h"
#include "BW/structures/Unit.h"
#include "BW/structures/ui_structures.h"
#include "BW/ui.h"
#include "graphics/Bitmap.h"
#include "hooks/interface/hud/hud_manager.h"
#include "systems/graphics/grp.h"
#include "systems/ui/tooltip.h"

#include <array>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <vector>

namespace {

// Second byte of a command button's stat_txt string (after hotkey)
enum class ButtonType : u8 {
    Train    = 1,
    Upgrade  = 2,
    Spell    = 3,
    Research = 4,
    Build    = 5,
};

struct CommandCosts {
    u16 minerals = 0;
    u16 gas = 0;
    u16 energy = 0;
    u16 time = 0;
    u16 supply = 0;
};

struct CostIcon {
    u16 CommandCosts::* value;
    HudIcons::Kind kind;
    u16 base_frame;
    u32 HudManager::Config::* hud_frame; // Added to base_frame when set
};

constexpr auto cost_icons = std::to_array<CostIcon>({
    {&CommandCosts::minerals, HudIcons::Kind::Mineral, 0, &HudManager::Config::mineral_icon},
    {&CommandCosts::gas,      HudIcons::Kind::Gas,     0, &HudManager::Config::gas_icon},
    {&CommandCosts::energy,   HudIcons::Kind::General, 9, nullptr},
    {&CommandCosts::time,     HudIcons::Kind::General, 7, nullptr},
    {&CommandCosts::supply,   HudIcons::Kind::Supply,  0, &HudManager::Config::supply_icon},
});

constexpr u16 UpgradeLevelLabel = 795; // Next Level:

CommandCosts unit_costs(u16 unit_id) {
    const RaceId::Enum race = Unit::get_race(unit_id);
    const bool paired = units_dat::base_property[unit_id] & UnitProperty::TwoUnitsIn1Egg;
    return {
        .minerals = units_dat::mineral_cost[unit_id],
        .gas = units_dat::vespene_cost[unit_id],
        .time = static_cast<u16>(units_dat::time_cost[unit_id] / 24),
        .supply = static_cast<u16>(paired ? 2 : 0),
    };
}

CommandCosts upgrade_costs(u8 player, u8 upgrade_id) {
    const u8 level = bw::get_upgrade_level(player, upgrade_id);
    return {
        .minerals = static_cast<u16>(upgrades_dat::mineral_cost_base[upgrade_id] + upgrades_dat::mineral_cost_factor[upgrade_id] * level),
        .gas = static_cast<u16>(upgrades_dat::vespene_cost_base[upgrade_id] + upgrades_dat::vespene_cost_factor[upgrade_id] * level),
    };
}

CommandCosts spell_costs(Unit* caster, u16 tech_id) {
    const s8 race = helper::unit_get_race(caster);
    return {.energy = techdata_dat::energy_cost[tech_id]};
}

CommandCosts research_costs(u16 tech_id) {
    return {
        .minerals = techdata_dat::mineral_cost[tech_id],
        .gas = techdata_dat::vespene_cost[tech_id],
    };
}

void append_costs(ui::Tooltip& tooltip, const CommandCosts& costs) {
    if (!g_hud_manager) return;

    constexpr u16 IconTextGap = 2;
    std::vector<ui::Tooltip::Span> spans;
    for (const CostIcon& icon : cost_icons) {
        const u16 value = costs.*icon.value;
        if (!value) continue;

        graphics::GrpFile* grp = g_hud_manager->icons().get(icon.kind);
        if (!grp || !grp->frame_count()) continue;
        u16 frame = icon.hud_frame
            ? static_cast<u16>(icon.base_frame + g_hud_manager->current().*icon.hud_frame)
            : icon.base_frame;
        if (frame >= grp->frame_count()) frame = 0;

        spans.push_back(ui::Tooltip::IconSpan{grp, frame, graphics::FrameFormat::Raw});
        spans.push_back(ui::Tooltip::GapSpan{IconTextGap});
        spans.push_back(ui::Tooltip::TextSpan{std::format("{}", value), TextColor::Grey});
    }

    if (!spans.empty()) {
        tooltip.row(std::move(spans)).spacer(1);
    }
}

std::string upgrade_level_line(u8 level) {
    return std::format("\n{} {}", g_stat_txt_buffer.get_string32(UpgradeLevelLabel), level);
}

bool is_child_of(const bw::Dialog* buttonset, const bw::Dialog* button) {
    const auto* first_child = buttonset->child<bw::Dialog*>();
    for (const bw::Dialog* child = first_child ? *first_child : nullptr; child; child = child->next) {
        if (child == button) return true;
    }
    return false;
}

std::optional<ui::Tooltip> build_command_tooltip(const bw::Dialog* buttonset, const bw::Dialog* button_dialog) {
    if (!is_child_of(buttonset, button_dialog)) return std::nullopt;
    if (!(button_dialog->flags & DialogFlags::Visible)) return std::nullopt;

    const auto* button = static_cast<const Button*>(button_dialog->user);
    if (!button) return std::nullopt;

    if (button->requirement_string_id > g_stat_txt_buffer.get_string_count32()) return std::nullopt;
    const char* enabled_text = helper::get_stat_txt_string(button->requirement_string_id);
    if (!enabled_text) return std::nullopt;
    const auto type = static_cast<ButtonType>(enabled_text[1]);

    Unit* unit = *g_active_portrait_unit;
    const u8 player = unit ? unit->player : 0;

    if (button_dialog->flags & DialogFlags::Disabled) {
        if (!button->action_string_id) return std::nullopt;
        u16 string_id = button->action_string_id;
        if (type == ButtonType::Upgrade && unit) {
            string_id = static_cast<u16>(string_id + bw::get_upgrade_level(player, static_cast<u8>(button->action_var)));
        }
        const char* disabled_text = helper::get_stat_txt_string(string_id);
        if (!disabled_text) return std::nullopt;
        return ui::game_text_tooltip(disabled_text);
    }

    std::string text = enabled_text + 2;
    CommandCosts costs;
    switch (type) {
        case ButtonType::Train:
        case ButtonType::Build: {
            costs = unit_costs(button->action_var);
        } break;
        case ButtonType::Upgrade: {
            if (!unit) break;
            const u8 upgrade_id = static_cast<u8>(button->action_var);
            costs = upgrade_costs(player, upgrade_id);
            if (upgrades_dat::max_repeats[upgrade_id] > 1) {
                text += upgrade_level_line(static_cast<u8>(bw::get_upgrade_level(player, upgrade_id) + 1));
            }
        } break;
        case ButtonType::Spell: {
            if (unit) costs = spell_costs(unit, button->action_var);
        } break;
        case ButtonType::Research: {
            costs = research_costs(button->action_var);
        } break;
        default: break; // Other types show no costs
    }

    ui::Tooltip tooltip = ui::game_text_tooltip(std::move(text));
    append_costs(tooltip, costs);
    return tooltip;
}

void show_command_tooltip(bw::Dialog* buttonset, bw::Dialog* button) {
    if (!ui::g_game_tooltips) return;

    const ui::TooltipSource source{
        .kind = ui::TooltipSource::Kind::CommandButton,
        .id = static_cast<u32>(reinterpret_cast<std::uintptr_t>(button)),
    };
    const ui::TooltipAnchor anchor{
        .rect = ui::control_screen_bounds(*button),
        .placement = ui::Placement::Above,
        .gap = 0,
    };
    ui::g_game_tooltips->show(source, anchor, [buttonset, button] { return build_command_tooltip(buttonset, button); });
}

} // namespace

namespace hooks {

void show_game_tooltip(s32 x, s32 y, const char* text) {
    if (!ui::g_game_tooltips || !text) return;

    const ui::TooltipSource source{
        .kind = ui::TooltipSource::Kind::Text,
        .id = (static_cast<u32>(static_cast<u16>(x)) << 16) | static_cast<u16>(y),
    };
    const ui::TooltipAnchor anchor{
        .rect = DialogBounds::from_xywh(static_cast<s16>(x), static_cast<s16>(y), 1, 1),
        .placement = ui::Placement::At,
        .gap = 0,
    };
    ui::g_game_tooltips->show(source, anchor, ui::game_text_tooltip(text));
}

void update_command_tooltip(bw::Dialog* buttonset, bw::Dialog* hovered) {
    if (!buttonset || hovered == buttonset) {
        ui::hide_game_tooltip();
        return;
    }

    auto* buttonset_data = buttonset->data<bw::MenuData>();
    if (!hovered) {
        bw::Dialog* focused = buttonset_data ? buttonset_data->focused_control : nullptr;
        if (!focused) return;
        if (!(focused->flags & DialogFlags::UpdateNeeded)) {
            focused->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(focused);
        }
        buttonset_data->focused_control = nullptr;
        ui::hide_game_tooltip();
        return;
    }

    if (!(hovered->flags & DialogFlags::Visible)) {
        ui::hide_game_tooltip();
        return;
    }

    if (hovered->flags & DialogFlags::Disabled) {
        if (buttonset_data) buttonset_data->focused_control = hovered;
    } else {
        helper::dialog_set_focus(hovered);
    }

    show_command_tooltip(buttonset, hovered);
}

void save_under_game_tooltip(graphics::Bitmap* dialog_bitmap, s32 dialog_left, s32 dialog_top) {
    auto* surface = ui::LayerTooltipSurface::active();
    if (!surface || !dialog_bitmap) return;
    surface->save_under(*dialog_bitmap, v2s16{static_cast<s16>(dialog_left), static_cast<s16>(dialog_top)});
}

void __stdcall stamp_game_tooltip(graphics::Bitmap* dialog_bitmap) {
    auto* surface = ui::LayerTooltipSurface::active();
    if (!surface || !dialog_bitmap) return;
    surface->stamp(*dialog_bitmap);
}

void __stdcall restore_under_game_tooltip(graphics::Bitmap* dialog_bitmap) {
    auto* surface = ui::LayerTooltipSurface::active();
    if (!surface || !dialog_bitmap) return;
    surface->restore(*dialog_bitmap);
}

} // namespace hooks
