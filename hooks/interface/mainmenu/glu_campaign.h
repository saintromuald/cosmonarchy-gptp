#pragma once
#include "systems/ui/widgets.h"

class GluCampaignList : public ui::Menu {
public:
    GluCampaignList();

    void on_init() override;
    void on_exit(u32 last_control) override;

private:
    void load_campaign_by_index(u16 index);

private:
    ui::Widget* m_cancel_button;
    ui::Widget* m_ok_button;
    ui::Widget* m_replay_button;
    ui::Widget* m_custom_button;
    ui::Widget* m_listbox;
    ui::Widget* m_title;
    ui::Widget* m_description;
    ui::Widget* m_mission_count;
    ui::Widget* m_author;
    std::string m_background;
};

class GluCampaignDetails : public ui::Menu {
public:
    GluCampaignDetails();

    void on_init() override;
    void on_exit(u32 last_control) override;

private:
    ui::Widget* m_cancel_button;
    ui::Widget* m_ok_button;
    ui::Widget* m_title;
    ui::Widget* m_subtitle;

    ui::Widget* m_combobox;
    ui::Widget* m_reset_button;

    ui::Widget* m_listbox;
    ui::Widget* m_description;
    ui::Group* m_description_box;
};