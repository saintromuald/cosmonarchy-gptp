#pragma once
#include "systems/ui/widgets.h"

class LogoBanner : public ui::Hud {
public:
    LogoBanner();

    void show_logo(bool show = true);

private:
    ui::Widget* m_background;
    ui::Widget* m_logo;
    ui::Widget* m_name;
    ui::Widget* m_helptext;
};
