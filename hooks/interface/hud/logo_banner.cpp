#include "logo_banner.h"

#include "BW/enumerations.h"

LogoBanner::LogoBanner() : ui::Hud(DialogBounds::from_xywh(0, 400, 640, 80)) {
    m_background = image(DialogBounds::from_xywh(0, 0, 552, 80), "game\\logo_banner.pcx");
    m_logo = image(DialogBounds::from_xywh(6, -5, 80, 80), "game\\logo_80.pcx");
    m_name = label(DialogBounds::from_xywh(93, 27, 300, 20), "{}Cosmonarchy",
        TextColor::White)->fontsize(ui::FontSize::Large16x);
    m_helptext = label(DialogBounds::from_xywh(93, 47, 300, 20), "{}Press {}F11{} to show HUD",
        TextColor::Grey, TextColor::Red, TextColor::Revert)->fontsize(ui::FontSize::Small10);
    initialize();
}

void LogoBanner::show_logo(bool show) {
    if (show) {
        m_logo->show();
        m_name->show();
        m_helptext->show();
    } else {
        m_logo->hide();
        m_name->hide();
        m_helptext->hide();
    }
}
