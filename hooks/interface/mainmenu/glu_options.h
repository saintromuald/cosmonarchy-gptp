#pragma once
#include "systems/ui/widgets.h"
#include "hooks/race_randomization.h"
#include "config.h"
#include "storm/smem.hpp"
#include "systems/triggers/trigger_system.h" // For Advisor

#include <array>

class GluOptions : public ui::Menu {
public:
    GluOptions();

    void on_init() override;
    void on_exit(u32 last_control) override;

private:
    void change_tab(ui::Group* new_tab);

    void load_advisor_flags();
    void save_advisor_flags();
    void reset_advisor_flags();
    bool advisors_valid() const;

    ui::Group* make_misc_tab();
    ui::Group* make_ui_tab();
    ui::Group* make_color_tab();
    ui::Group* make_control_tab();

private:
    ui::Widget* m_cancel_button;
    ui::Widget* m_ok_button;
    
    ui::Widget* m_misc_tab_button;
    ui::Group* m_misc_tab;
    RaceFlags::Enum m_race_flags = RaceFlags::Original;
    std::array<RaceFlags::Enum, (size_t)Advisor::Count> m_advisor_races{};

    ui::Widget* m_ui_tab_button;
    ui::Group* m_ui_tab;

    ui::Widget* m_color_tab_button;
    ui::Group* m_color_tab;
    Palette m_palette;
    smem::ptr<u8[]> m_indexes;

    ui::Widget* m_control_tab_button;
    ui::Group* m_control_tab;

    ui::Group* m_active_tab;
    Config m_config;
};
