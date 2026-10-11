#include "glu_options.h"
#include "BW/bwdata.h"
#include "BW/ui.h"
#include "BW/enumerations.h"
#include "BW/structures/ui_structures.h"
#include "storm/smem.hpp"
#include "storm/storm.h"
#include "config.h"
#include "systems/triggers/trigger_system.h"
#include "systems/ui/tooltip.h"

#include <algorithm>
#include <array>
#include <format>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>

#include <magic_enum/magic_enum.hpp>

static constexpr auto TabHorizontalOffset = 99;
static constexpr auto TabHorizontalStart = 232;
static constexpr auto TabCount = 4;

static constexpr auto CheckboxHeight = 20;
static constexpr auto ComboboxHeight = 20;
static constexpr auto LabelHeight = 25;

static constexpr auto ColumnWidth = 230;
static constexpr auto UIBoolWidth = 80;
static constexpr auto ItemIndent = 10;
static constexpr auto SectionPadding = 10;
static constexpr auto ColumnPadding = 40;

static constexpr auto StartX = 30;
static constexpr auto StartY = 70;

static constexpr u8  SwatchShadeCount = 8;
static constexpr u16 SwatchBoxSize = 10;
static constexpr std::array<u8, SwatchShadeCount> SwatchPaletteSlots = {247, 248, 249, 250, 251, 252, 253, 255};

// Horizontal line under the inactive tabs to show they're not selected
static void add_tab_blockers(ui::Group* group, u8 index) {
    auto x = TabHorizontalStart;
    for (auto i = 0; i < TabCount; i++) {
        if (i != index) {
            group->image(DialogBounds::from_xywh(x, 40, 95, 4), "glue\\dynamic\\fullscreen_tab_block.pcx");
        }
        x += TabHorizontalOffset;
    }
}

static ui::Widget* options_label(ui::Group* tab, v2u16& cursor, std::string text) {
    auto label = tab->label(DialogBounds::from_xywh(cursor.x, cursor.y, ColumnWidth, LabelHeight), std::move(text));
    label->fontsize(ui::FontSize::Large16);
    cursor.y += LabelHeight;
    return label;
}

static ui::Widget* linked_checkbox(ui::Group* tab, v2u16& cursor, std::string text, bool& target) {
    auto checkbox = tab->checkbox(DialogBounds::from_xywh(cursor.x + ItemIndent, cursor.y, ColumnWidth, CheckboxHeight),
        std::move(text), target)->on_activate([&target](ui::Widget&) -> ui::ActivateResult {
            target = !target;
            return ui::ActivateResult::ContinueWithDefault;
        });
    cursor.y += CheckboxHeight;
    return checkbox;
}

static ui::Widget* race_flags_checkbox(ui::Group* tab, v2u16& cursor, std::string text, RaceFlags::Enum& target, RaceFlags::Enum flag) {
    auto checkbox = tab->checkbox(DialogBounds::from_xywh(cursor.x + ItemIndent, cursor.y, ColumnWidth, CheckboxHeight),
        std::move(text), target & flag)->on_activate([&target, flag](ui::Widget&) -> ui::ActivateResult {
            target ^= flag;
            return ui::ActivateResult::ContinueWithDefault;
        });
    cursor.y += CheckboxHeight;
    return checkbox;
}

static ui::Widget* linked_uibool(ui::Group* tab, v2u16& cursor, std::string text, UiBool& target) {
    tab->label(DialogBounds::from_xywh(cursor.x + ItemIndent, cursor.y, ColumnWidth, ComboboxHeight), std::move(text));
    auto combobox = tab->combobox(DialogBounds::from_xywh(cursor.x + ItemIndent + ColumnWidth, cursor.y, UIBoolWidth, ComboboxHeight));
    combobox->on_create([&target](ui::Widget& self) {
        for (auto name : ui_bool_names) {
            self.add_item(name);
        }
        quick_send_event(self.dialog()->to_bindlg(), DialogUserType_SetSelectionIndex, std::to_underlying(target.value));
    })->on_selection([&target](ui::Widget&, u16 index) {
        if (index >= std::to_underlying(EUiBool::Count)) return;
        target.value = EUiBool{index};
    });
    cursor.y += ComboboxHeight;
    return combobox;
}

// Advisor matrix
static constexpr auto MatrixCellSize = 18;
static constexpr auto MatrixLabelGap = 8;
static constexpr auto MatrixLabelWidth = 150;
static constexpr auto MatrixRowHeight = 20;

struct MatrixColumn { std::string_view header; RaceFlags::Enum flag; };
struct MatrixRow    { std::string_view label;  RaceFlags::Enum* flags; };
struct MatrixCell   { ui::Widget* checkbox; RaceFlags::Enum* flags; RaceFlags::Enum flag; };

static Advisor advisor_for_race_flag(RaceFlags::Enum flag) {
    switch (flag) {
        case RaceFlags::Terran:  return Advisor::Terran;
        case RaceFlags::Protoss: return Advisor::Protoss;
        case RaceFlags::Zerg:    return Advisor::Zerg;
        case RaceFlags::Askosi:  return Advisor::Askosi;
        default:                 return Advisor::Default;
    }
}

static std::vector<MatrixCell> checkbox_matrix(ui::Group* tab, v2u16& cursor,
    std::span<const MatrixColumn> columns, std::span<const MatrixRow> rows) {
    const auto grid_x  = static_cast<s16>(cursor.x + ItemIndent);
    const auto grid_y  = static_cast<s16>(cursor.y + MatrixRowHeight);
    const auto label_x = static_cast<s16>(grid_x + columns.size() * MatrixCellSize + MatrixLabelGap);

    for (size_t col = 0; col < columns.size(); ++col) {
        const auto x = static_cast<s16>(grid_x + col * MatrixCellSize);
        tab->label(DialogBounds::from_xywh(x, cursor.y, MatrixCellSize, MatrixRowHeight),
            std::string{columns[col].header})->horizontal_align(ui::Align::Center);
    }

    std::vector<MatrixCell> cells(rows.size() * columns.size());
    for (size_t row = 0; row < rows.size(); row++) {
        const auto y = static_cast<s16>(grid_y + row * MatrixCellSize);
        for (size_t col = 0; col < columns.size(); ++col) {
            const auto x = static_cast<s16>(grid_x + col * MatrixCellSize);
            RaceFlags::Enum* flags = rows[row].flags;
            const RaceFlags::Enum flag = columns[col].flag;
            ui::Widget* checkbox = tab->checkbox(DialogBounds::from_xywh(x, y, MatrixCellSize, MatrixCellSize), "", (*flags & flag));
            checkbox->on_activate([flags, flag](ui::Widget&) -> ui::ActivateResult {
                *flags ^= flag;
                return ui::ActivateResult::ContinueWithDefault;
            });
            cells[row * columns.size() + col] = { checkbox, flags, flag };
        }
        tab->label(DialogBounds::from_xywh(label_x, y, MatrixLabelWidth, MatrixRowHeight), std::string{rows[row].label});
    }

    cursor.y = static_cast<u16>(grid_y + rows.size() * MatrixCellSize + SectionPadding);
    return cells;
}

class ColorListEditor {
public:
    ColorListEditor(ui::Widget* listbox, std::vector<PlayerColor> colors, std::vector<std::vector<PlayerColor>*> targets)
        : m_listbox(listbox), m_colors(std::move(colors)), m_targets(std::move(targets)) {}

    ui::Widget* listbox() const { return m_listbox; }

    std::optional<PlayerColor> value_at(u8 display_row) const {
        if (display_row >= m_display.size()) return std::nullopt;
        return m_display[display_row];
    }

    void select_target(u8 index) {
        if (index >= m_targets.size()) return;
        m_active = index;
        m_highlight.reset();
        rebuild();
    }

    void toggle_row(u8 display_row) {
        if (display_row >= m_display.size()) return;
        const PlayerColor value = m_display[display_row];
        auto& vec = active();
        if (std::ranges::contains(vec, value)) {
            std::erase(vec, value);
        } else {
            vec.push_back(value); // Newly checked -> lowest priority
        }
        m_highlight = value; // Keep the highlight on the item as it moves sections
        rebuild();
    }

    void highlight_row(u8 display_row) {
        if (display_row >= m_display.size()) {
            m_highlight.reset();
        } else {
            m_highlight = m_display[display_row];
        }
        apply_highlight();
    }

    void move_highlighted(int delta) {
        if (!m_highlight) return;
        auto& vec = active();
        const auto it = std::ranges::find(vec, *m_highlight);
        if (it == vec.end()) return; // Only selected-section entries can reorder
        const auto pos  = it - vec.begin();
        const auto dest = pos + delta;
        if (dest < 0 || dest >= static_cast<decltype(dest)>(vec.size())) return;
        std::swap(vec[pos], vec[dest]);
        rebuild();
    }

    void check_all() {
        auto& vec = active();
        for (const PlayerColor value : m_colors) {
            if (!std::ranges::contains(vec, value)) {
                vec.push_back(value);
            }
        }
        rebuild();
    }

    void check_none() {
        active().clear();
        m_highlight.reset();
        rebuild();
    }

private:
    std::vector<PlayerColor>& active() { return *m_targets[m_active]; }

    void rebuild() {
        const auto& vec = active();

        // Display order: selected entries (vector order) then the rest (enum order)
        m_display.clear();
        m_display.reserve(m_colors.size());
        m_display.insert(m_display.end(), vec.begin(), vec.end());
        for (const PlayerColor value : m_colors) {
            if (!std::ranges::contains(vec, value)) {
                m_display.push_back(value);
            }
        }

        m_listbox->clear_items();
        for (const PlayerColor value : m_display) {
            m_listbox->add_item(std::string{magic_enum::enum_name(value)});
        }

        // Set every row explicitly -- add_item may reuse stale flag bytes
        // The first vec.size() rows are the selected (checked) section
        for (std::size_t i = 0; i < m_display.size(); ++i) {
            m_listbox->set_checked(static_cast<u8>(i), i < vec.size());
        }

        apply_highlight();
    }

    void apply_highlight() {
        if (m_highlight) {
            const auto it = std::ranges::find(m_display, *m_highlight);
            if (it != m_display.end()) {
                m_listbox->select_row(static_cast<u8>(it - m_display.begin()));
                return;
            }
        }
        m_listbox->clear_selection();
    }

    ui::Widget* m_listbox;
    std::vector<PlayerColor> m_colors;
    std::vector<std::vector<PlayerColor>*> m_targets;
    std::vector<PlayerColor> m_display;
    std::optional<PlayerColor> m_highlight;
    u8 m_active = 0;
};

// Every PlayerColor in [0, Count]: excludes Invalid/None and Count
static std::vector<PlayerColor> valid_colors() {
    std::vector<PlayerColor> out;
    const auto count = std::to_underlying(PlayerColor::Count);
    out.reserve(count);
    for (std::underlying_type_t<PlayerColor> i = 0; i < count; ++i) {
        out.push_back(static_cast<PlayerColor>(i));
    }
    return out;
}

static std::shared_ptr<ColorListEditor> player_color_selector(ui::Group* tab, v2u16& cursor,
    std::vector<std::pair<std::string, std::vector<PlayerColor>*>> targets,
    std::initializer_list<PlayerColor> exclude = {}) {
    v2u16 layout{cursor};

    tab->image(DialogBounds::from_xywh(layout.x - 4, layout.y + 1, 168, 257), "glue\\dynamic\\checked_listbox_back.pcx")->translucent();

    // Above the listbox: select all / none.
    auto select_none = tab->lightup_button(DialogBounds::from_xywh(layout.x, layout.y, 75, 24), "None");
    auto select_all  = tab->lightup_button(DialogBounds::from_xywh(layout.x + 85, layout.y, 75, 24), "All");

    const s16 list_x = static_cast<s16>(layout.x);
    const s16 list_y = static_cast<s16>(layout.y + 30);
    auto listbox = tab->checked_listbox(DialogBounds::from_xywh(list_x, list_y, 160, 200));

    // Right of the listbox: which target vector we're editing
    auto target_names = std::views::keys(targets) | std::ranges::to<std::vector<std::string_view>>();
    ui::Group* options = tab->radio_group(DialogBounds::from_xywh(list_x + 180, list_y, 120, 200), target_names);

    // Below the listbox: reorder the highlighted (selected) entry
    const s16 buttons_y = static_cast<s16>(list_y + 204);
    auto move_up = tab->lightup_button(DialogBounds::from_xywh(list_x, buttons_y, 75, 24), "Up")
        ->tooltip("Move selected color up in the priority list");
    auto move_down = tab->lightup_button(DialogBounds::from_xywh(list_x + 85, buttons_y, 75, 24), "Down")
        ->tooltip("Move selected color down in the priority list");

    cursor.y += 264;   // 30 (button row) + 200 (listbox) + 4 gap + 24 (buttons) + 6

    auto ordered = std::views::values(targets) | std::ranges::to<std::vector>();

    std::vector<PlayerColor> colors = valid_colors();
    if (exclude.size()) {
        std::erase_if(colors, [&](PlayerColor value) { return std::ranges::contains(exclude, value); });
    }

    auto editor = std::make_shared<ColorListEditor>(listbox, std::move(colors), std::move(ordered));

    options->on_radio_selection([editor](u8 index) { editor->select_target(index); });
    listbox->on_check_toggle([editor](ui::Widget&, u8 row, bool) { editor->toggle_row(row); });
    listbox->on_selection([editor](ui::Widget&, u16 row) { editor->highlight_row(static_cast<u8>(row)); });
    select_all ->on_activate([editor](ui::Widget&) { editor->check_all();  return ui::ActivateResult::ConsumeActivation; });
    select_none->on_activate([editor](ui::Widget&) { editor->check_none(); return ui::ActivateResult::ConsumeActivation; });
    move_up  ->on_activate([editor](ui::Widget&) { editor->move_highlighted(-1); return ui::ActivateResult::ConsumeActivation; });
    move_down->on_activate([editor](ui::Widget&) { editor->move_highlighted(+1); return ui::ActivateResult::ConsumeActivation; });
    listbox->on_create([editor](ui::Widget&) { editor->select_target(0); });
    return editor;
}

GluOptions::GluOptions()
    : ui::Menu("glue\\dynamic\\backgrounds\\options.pcx"), m_config(g_config), m_race_flags(RaceRandomizer::instance().race_flags()) {
    const char hotkey_color = static_cast<char>(LobbyTextColor::YellowGreen);
    const char revert_color = static_cast<char>(LobbyTextColor::Revert);

    image(DialogBounds::from_xywh(0, 0, 640, 480), "glue\\dynamic\\fullscreen_tabs.pcx")->translucent();
    m_cancel_button = lightup_button(DialogBounds::from_xywh(481, 429, 130, 34),
        std::format("{}C{}ancel", hotkey_color, revert_color))->enable_hotkey('c')->set_cancel();
    m_ok_button = lightup_button(DialogBounds::from_xywh(262, 429, 204, 34),
        std::format("Apply {}O{}ptions", hotkey_color, revert_color))->enable_hotkey('o')->set_default();

    m_misc_tab_button    = lightup_button(DialogBounds::from_xywh(236, 16, 86, 24),
        std::format("{}M{}isc", hotkey_color, revert_color))->enable_hotkey('m');
    m_ui_tab_button      = lightup_button(DialogBounds::from_xywh(335, 16, 86, 24),
        std::format("{}U{}i", hotkey_color, revert_color))->enable_hotkey('u');
    m_color_tab_button   = lightup_button(DialogBounds::from_xywh(434, 16, 86, 24), 
        std::format("Co{}l{}or", hotkey_color, revert_color))->enable_hotkey('l');
    m_control_tab_button = lightup_button(DialogBounds::from_xywh(533, 16, 86, 24),
        std::format("Cont{}r{}ol", hotkey_color, revert_color))->enable_hotkey('r')->disable(); // TODO: Implement control tab -Jesse

    m_misc_tab    = make_misc_tab();
    m_ui_tab      = make_ui_tab()->hide();
    m_color_tab   = make_color_tab()->hide();
    m_control_tab = make_control_tab()->hide();
    m_active_tab  = m_misc_tab;
}

void GluOptions::on_init() {
    m_misc_tab_button->on_activate([&](ui::Widget&) -> ui::ActivateResult {
        change_tab(m_misc_tab);
        return ui::ActivateResult::ConsumeActivation;
    });
    m_ui_tab_button->on_activate([&](ui::Widget&) -> ui::ActivateResult {
        change_tab(m_ui_tab);
        return ui::ActivateResult::ConsumeActivation;
    });
    m_color_tab_button->on_activate([&](ui::Widget&) -> ui::ActivateResult {
        change_tab(m_color_tab);
        return ui::ActivateResult::ConsumeActivation;
    });
    m_control_tab_button->on_activate([&](ui::Widget&) -> ui::ActivateResult {
        change_tab(m_control_tab);
        return ui::ActivateResult::ConsumeActivation;
    });

    m_ok_button->on_activate([&](ui::Widget&) -> ui::ActivateResult {
        if (std::popcount(std::to_underlying(m_race_flags)) < 2) {
            ui::modal_ok_cancel("You must select at least 2 races for the random pool");
            return ui::ActivateResult::ConsumeActivation;
        }
        if (!advisors_valid()) {
            ui::modal_ok_cancel("Each race must have at least one advisor selected");
            return ui::ActivateResult::ConsumeActivation;
        }

        save_advisor_flags();
        g_config = m_config;
        auto& race_randomizer = RaceRandomizer::instance();
        race_randomizer.set_race_flags(m_race_flags);
        race_randomizer.update_config();
        race_randomizer.cleanup_and_reset();
        save_config();
        return ui::ActivateResult::ContinueWithDefault;
    })->tooltip("Save changes and return to the main menu");
    m_cancel_button->tooltip("Discard changes and return");
}

void GluOptions::on_exit(u32 last_control) {
    g_gl_glues_mode = MenuId::GlueMainMenu;
}

void GluOptions::change_tab(ui::Group* new_tab) {
    if (m_active_tab == new_tab) return;
    if (m_active_tab) {
        m_active_tab->hide();
    }
    new_tab->show();
    m_active_tab = new_tab;
}

ui::Group* GluOptions::make_misc_tab() {
    v2u16 cursor{StartX, StartY};
    auto tab = group();
    add_tab_blockers(tab, 0);

    load_advisor_flags();

    options_label(tab, cursor, "Random Race Pool");
    race_flags_checkbox(tab, cursor, "Zerg", m_race_flags, RaceFlags::Zerg);
    race_flags_checkbox(tab, cursor, "Terran", m_race_flags, RaceFlags::Terran);
    race_flags_checkbox(tab, cursor, "Protoss", m_race_flags, RaceFlags::Protoss);
    race_flags_checkbox(tab, cursor, "Askosi", m_race_flags, RaceFlags::Askosi);
    cursor.y += SectionPadding;

    options_label(tab, cursor, "Launch Settings");
    linked_checkbox(tab, cursor, "Disable WMode Prompt", m_config.disable_wmode_prompt);
    linked_checkbox(tab, cursor, "Prefer WMode", m_config.prefer_wmode);
    linked_checkbox(tab, cursor, "Suppress Windows Key", m_config.suppress_windows_key);
    cursor.y += SectionPadding;

    options_label(tab, cursor, "Audiovisuals");
    linked_checkbox(tab, cursor, "Celebrations", m_config.celebrations_enabled);
    linked_checkbox(tab, cursor, "Flash Window on Lobby Join", m_config.flash_window_lobby_join)
        ->tooltip("Flashes the window when a player joins even if Cosmonarchy is in the background");
    cursor.y += SectionPadding;

    options_label(tab, cursor, "Replays");
    linked_checkbox(tab, cursor, "Auto Save", m_config.auto_save_replays);
    linked_checkbox(tab, cursor, "Show Progressbar", m_config.show_replay_progress_bar);
    linked_checkbox(tab, cursor, "SanitizeChat", m_config.sanitize_replay_chat)
        ->tooltip("Removes all chat messages except 'glhf' and 'gg'");
    cursor.y += SectionPadding;

    // Second column: which advisors are eligible for each race.
    cursor.x += ColumnWidth + ColumnPadding;
    cursor.y = StartY;
    options_label(tab, cursor, "Race Advisors");

    static constexpr Advisor AdvisorMatrixExclude[] = {
        Advisor::Default, Advisor::Goku,
    };
    static constexpr MatrixColumn RaceColumns[] = {
        {"T", RaceFlags::Terran},
        {"P", RaceFlags::Protoss},
        {"Z", RaceFlags::Zerg},
        {"A", RaceFlags::Askosi},
    };
    std::vector<MatrixRow> rows;
    for (u8 i = 0; i < std::to_underlying(Advisor::Count); ++i) {
        const auto advisor = static_cast<Advisor>(i);
        if (std::ranges::contains(AdvisorMatrixExclude, advisor)) continue;
        rows.push_back({ magic_enum::enum_name(advisor), &m_advisor_races[i] });
    }
    auto cells = checkbox_matrix(tab, cursor, RaceColumns, rows);

    // Reset the matrix to just each race's own advisor, refreshing the visuals
    tab->lightup_button(DialogBounds::from_xywh(cursor.x + ItemIndent, cursor.y, 72, 24), "Reset")
        ->on_activate([this, cells = std::move(cells)](ui::Widget&) -> ui::ActivateResult {
            reset_advisor_flags();
            for (const MatrixCell& cell : cells) {
                cell.checkbox->select((*cell.flags & cell.flag) != RaceFlags::None);
            }
            return ui::ActivateResult::ConsumeActivation;
            })->tooltip("Reset to each race's own advisor only");
    return tab;
}

void GluOptions::reset_advisor_flags() {
    m_advisor_races.fill(RaceFlags::None);
    for (RaceFlags::Enum flag : { RaceFlags::Terran, RaceFlags::Protoss, RaceFlags::Zerg, RaceFlags::Askosi }) {
        m_advisor_races[std::to_underlying(advisor_for_race_flag(flag))] |= flag;
    }
}

void GluOptions::load_advisor_flags() {
    m_advisor_races.fill(RaceFlags::None);
    auto assign = [&](Advisor advisor, RaceFlags::Enum flag) {
        if (advisor == Advisor::Default) {
            advisor = advisor_for_race_flag(flag);
        }
        m_advisor_races[std::to_underlying(advisor)] |= flag;
    };
    for (Advisor advisor : m_config.common_advisors) {
        assign(advisor, RaceFlags::Terran);
        assign(advisor, RaceFlags::Protoss);
        assign(advisor, RaceFlags::Zerg);
        assign(advisor, RaceFlags::Askosi);
    }
    for (Advisor advisor : m_config.terran_advisors) {
        assign(advisor, RaceFlags::Terran);
    }
    for (Advisor advisor : m_config.protoss_advisors) {
        assign(advisor, RaceFlags::Protoss);
    }
    for (Advisor advisor : m_config.zerg_advisors) {
        assign(advisor, RaceFlags::Zerg);
    }
    for (Advisor advisor : m_config.askosi_advisors) {
        assign(advisor, RaceFlags::Askosi);
    }
}

void GluOptions::save_advisor_flags() {
    m_config.common_advisors.clear();
    m_config.terran_advisors.clear();
    m_config.protoss_advisors.clear();
    m_config.zerg_advisors.clear();
    m_config.askosi_advisors.clear();

    auto races = m_advisor_races;

    bool is_default = true;
    for (RaceFlags::Enum flag : { RaceFlags::Terran, RaceFlags::Protoss, RaceFlags::Zerg, RaceFlags::Askosi }) {
        if ((races[std::to_underlying(advisor_for_race_flag(flag))] & flag) == RaceFlags::None) {
            is_default = false;
            break;
        }
    }
    if (is_default) {
        m_config.common_advisors.push_back(Advisor::Default);
        for (RaceFlags::Enum flag : { RaceFlags::Terran, RaceFlags::Protoss, RaceFlags::Zerg, RaceFlags::Askosi }) {
            races[std::to_underlying(advisor_for_race_flag(flag))] &= ~flag;
        }
    }

    for (size_t i = 0; i < races.size(); ++i) {
        const RaceFlags::Enum flags = races[i];
        const Advisor advisor = static_cast<Advisor>(i);
        if (flags == RaceFlags::None) continue;
        if (flags == RaceFlags::All) { m_config.common_advisors.push_back(advisor); continue; }
        if (flags & RaceFlags::Terran) {
            m_config.terran_advisors.push_back(advisor);
        }
        if (flags & RaceFlags::Protoss) {
            m_config.protoss_advisors.push_back(advisor);
        }
        if (flags & RaceFlags::Zerg) {
            m_config.zerg_advisors.push_back(advisor);
        }
        if (flags & RaceFlags::Askosi) {
            m_config.askosi_advisors.push_back(advisor);
        }
    }
}

bool GluOptions::advisors_valid() const {
    for (RaceFlags::Enum flag : { RaceFlags::Terran, RaceFlags::Protoss, RaceFlags::Zerg, RaceFlags::Askosi }) {
        const bool any = std::ranges::any_of(m_advisor_races,
            [flag](RaceFlags::Enum flags) { return (flags & flag) != RaceFlags::None; });
        if (!any) return false;
    }
    return true;
}

ui::Group* GluOptions::make_ui_tab() {
    v2u16 cursor{StartX, StartY};
    auto tab = group();
    add_tab_blockers(tab, 1);

    options_label(tab, cursor, "General UI");
    linked_checkbox(tab, cursor, "Extended Camera Hotkeys", m_config.extended_camera_hotkeys);
    linked_checkbox(tab, cursor, "Show Timer", m_config.show_timer);
    linked_checkbox(tab, cursor, "Show APM", m_config.show_apm);
    linked_checkbox(tab, cursor, "Always Show Ext. Tooltips", m_config.always_show_extended_tooltips);
    cursor.y += SectionPadding;

    options_label(tab, cursor, "Team UI");
    linked_checkbox(tab, cursor, "Show Team UI", m_config.show_team_ui);
    linked_checkbox(tab, cursor, "Show Team APM", m_config.show_team_ui_apm);
    linked_checkbox(tab, cursor, "Show Team Names", m_config.show_team_ui_names);
    linked_checkbox(tab, cursor, "Show Self in Team UI", m_config.show_team_ui_self);
    linked_checkbox(tab, cursor, "Show Global Queue", m_config.show_ingame_global_queue);
    cursor.x += ColumnWidth + ColumnPadding;
    cursor.y = StartY;

    options_label(tab, cursor, "Selection UI");
    linked_uibool(tab, cursor, "Show Rally Lines", m_config.show_rally_lines);
    linked_uibool(tab, cursor, "Show Order Lines", m_config.show_order_lines);
    linked_uibool(tab, cursor, "Queued Order Lines", m_config.show_queued_order_lines);
    linked_uibool(tab, cursor, "Unique Gas Order Lines", m_config.show_unique_gas_order_line_color);
    linked_uibool(tab, cursor, "Show Aura Circles", m_config.show_aura_circles);
    linked_uibool(tab, cursor, "Show Ability Ranges", m_config.show_ability_ranges);
    linked_uibool(tab, cursor, "Condensed Selection", m_config.always_show_condensed_selection);
    linked_uibool(tab, cursor, "Alt for Selection UI", m_config.use_alt_for_selection_ui);
    cursor.y += SectionPadding;

    options_label(tab, cursor, "Minimap UI");
    linked_uibool(tab, cursor, "Show Cameras", m_config.show_minimap_cameras);
    linked_uibool(tab, cursor, "Allow Terrain Toggle", m_config.can_toggle_minimap_terrain);

    return tab;
}

ui::Group* GluOptions::make_color_tab() {
    v2u16 cursor{StartX, StartY};
    auto tab = group();
    add_tab_blockers(tab, 2);

    cursor.x += 10;
    cursor.y += 26;
    tab->label(DialogBounds::from_xywh(cursor.x + ItemIndent, cursor.y, 186, 225), std::format(
        "{}Player Color Selection\n\n{}"
        "This list defines your preferred player colors.\n"
        "Colors will be chosen in order of preference. Use the up/down arrows to reorder.\n\n"
        "Hover your mouse over a color to see a preview.",
        static_cast<char>(LobbyTextColor::White), static_cast<char>(LobbyTextColor::Revert)));
    cursor.y += 225 + SectionPadding;
    linked_checkbox(tab, cursor, "Shuffle Colors", m_config.shuffle_preferred_colors);

    cursor = {StartX + 230, StartY + 26};
    Bitmap tunit{};
    load_bitmap("game\\tunit.pcx", tunit, &m_palette);
    m_indexes = smem::ptr<u8[]>{tunit.buffer};

    auto color_editor = player_color_selector(tab, cursor, {
        {"Common",  &m_config.common_preferred_colors},
        {"Terran",  &m_config.terran_preferred_colors},
        {"Protoss", &m_config.protoss_preferred_colors},
        {"Zerg",    &m_config.zerg_preferred_colors},
        {"Askosi",  &m_config.askosi_preferred_colors},}, {
        // Banned colors:
        PlayerColor::IridescentBlue,
        PlayerColor::IridescentGreen,
        PlayerColor::IridescentViolet,
        PlayerColor::IridescentYellow,
        PlayerColor::Magenta,
        PlayerColor::Rainbow,
        PlayerColor::RainbowNew,
        PlayerColor::PatrioticFreedom,
        PlayerColor::MetzenicProzac,
        PlayerColor::Harlequin,
    });
    ui::Widget* color_listbox = color_editor->listbox();

    ui::Tooltip color_swatch;
    color_swatch.padding(0).fixed_size(SwatchBoxSize, SwatchBoxSize * SwatchShadeCount).border();
    for (u8 slot : SwatchPaletteSlots) {
        color_swatch.fill(slot, SwatchBoxSize);
    }
    color_listbox->tooltip(std::move(color_swatch))
        ->tooltip_delay_ms(0)
        ->tooltip_placement(ui::Placement::Mouse);

    color_listbox->on_row_hover([this, color_editor](ui::Widget&, u8 row) {
        // Rows are reordered (selected-on-top), so map the display row back to
        // its PlayerColor, then load that colorway's shades into the scratch
        // slots. m_indexes stores the shades consecutively per color.
        auto color = color_editor->value_at(row);
        if (!color) return;
        const u32 base = std::to_underlying(*color) * SwatchShadeCount;
        for (u8 i = 0; i < SwatchShadeCount; ++i) {
            g_display_palette.colors[SwatchPaletteSlots[i]] = m_palette.colors[m_indexes[base + i]];
        }
        SDrawUpdatePalette(0, 256, &g_display_palette, 1);
    });

    return tab;
}

ui::Group* GluOptions::make_control_tab() {
    v2u16 cursor{StartX, StartY};
    auto tab = group();
    add_tab_blockers(tab, 3);

    return tab;
}
