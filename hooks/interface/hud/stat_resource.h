#pragma once

#include "systems/ui/widgets.h"
#include "hooks/unit_stats/supply_hooks.h"
#include "systems/graphics/grp.h"

#include <array>

class StatResource : public ui::OverlayHud {
public:
    StatResource();
    ~StatResource();

    StatResource(const StatResource&) = delete;
    StatResource& operator=(const StatResource&) = delete;

private:
    struct Row {
        ui::Widget* supply = nullptr;
        ui::Widget* gas = nullptr;
        ui::Widget* minerals = nullptr;
        ui::Widget* apm = nullptr;
        ui::Widget* name = nullptr;
        ui::Widget* color_box = nullptr;
        std::array<ui::Widget*, RowSize> production{};

        u8 player = 0;
        u16 icon_index = 0;
        bool use_player_color = false;
        std::array<u16, RowSize> production_type{};
    };

    static DialogBounds container_bounds();

    void build_row(u32 row_index);
    void update();
    void draw_player_stats(u32 row_index, u32 player, bool use_player_color, bool can_show_apm,
        bool show_production, bool show_name, bool show_color_box, bool count_in_production);
    void hide_row(u32 row_index);
    void update_displayed_resource(ui::Widget* widget, s32& displayed, s32 resource_amount);

    // Widget draw callbacks, invoked by the dialog repaint pass
    s16 draw_grp_icon(bw::Dialog* dialog, graphics::GrpFile* icons, u16 frame_index);
    void draw_resource(ui::Widget& widget, graphics::GrpFile* icons, Row& row);
    void draw_production_slot(ui::Widget& widget, Row& row, u32 slot);
    void draw_colortext(ui::Widget& widget, Row& row, s16 alignment, bool font_by_length);
    void draw_color_box(ui::Widget& widget, Row& row);

    static void __fastcall timer_proc(BinDlg* dialog, u16 index);

    std::array<Row, PlayerLimit> m_rows;
    graphics::Font* m_font = nullptr; // font_statres
    u32 m_last_frame = u32(-1);
    bool m_last_alt = false;
    bool m_last_hidden = false;
    bool m_last_in_replay = false;
};
