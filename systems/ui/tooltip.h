#pragma once
#include "BW/enumerations.h"
#include "BW/structures/common.h"
#include "BW/structures/ui_structures.h"
#include "BW/ui.h"
#include "storm/smem.hpp"

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ui {

class Widget;
enum class Placement {
    Auto, // Prefer below but flip if needed
    Below,
    Above,
    Right,
    Left,
    Mouse,
};

// Reusable rich-content tooltip. Move-only because it owns image buffers
class Tooltip {
public:
    Tooltip() = default;
    Tooltip(const Tooltip&) = delete;
    Tooltip& operator=(const Tooltip&) = delete;
    Tooltip(Tooltip&&) = default;
    Tooltip& operator=(Tooltip&&) = default;

    // Simple text-only construction.
    explicit Tooltip(std::string text);

    // Content
    Tooltip& text(std::string string, LobbyTextColor color = LobbyTextColor::Green);
    Tooltip& image(const std::string& pcx_path);
    Tooltip& fill(u8 palette_index, u16 height);
    Tooltip& divider(std::optional<u8> palette_index = {}); // 1px line, nullopt = theme default color
    Tooltip& spacer(u16 height);

    // Appearance
    Tooltip& background(std::optional<u8> palette_index = {}); // Background fill color, nullopt = theme default
    Tooltip& border(bool value = true);
    Tooltip& padding(u8 pixels);
    Tooltip& max_width(std::optional<u16> pixels); // Text wraps to this box width; nullopt disables wrapping (default 200)
    Tooltip& fixed_size(u16 w, u16 h);

    // Duplicate this tooltip and its image buffers
    Tooltip clone() const;

private:
    friend class TooltipController;

    v2u16 measure() const;
    void rasterize(u8* pixels, v2u16 size) const;

    struct TextItem    { std::string string; LobbyTextColor color; };
    struct ImageItem   { u16 width; u16 height; smem::ptr<u8[]> buffer; std::string path; };
    struct FillItem    { u8 palette_index; u16 height; };
    struct DividerItem { std::optional<u8> palette_index; };
    struct SpacerItem  { u16 height; };
    using Item = std::variant<TextItem, ImageItem, FillItem, DividerItem, SpacerItem>;

    std::deque<Item> m_items;
    std::optional<u8> m_bg_palette_index; // Default filled with theme dialog bg
    bool m_has_border = true;
    u8 m_padding = 4;
    std::optional<u16> m_max_width = 200; // Text wraps to this box width; nullopt disables wrapping
    std::optional<v2u16> m_fixed_size;

    struct Layout {
        v2u16 size;
        std::vector<std::string> text; // Wrapped body per TextItem, in item order
    };
    const Layout& layout() const; // Lazily builds and caches m_layout
    mutable std::optional<Layout> m_layout; // Cached size + wrapped text, invalidated by any size-affecting mutator
};

class Group;

// Per top level dialog tooltip controller.
class TooltipController {
public:
    TooltipController();
    ~TooltipController();

    TooltipController(const TooltipController&) = delete;
    TooltipController& operator=(const TooltipController&) = delete;

    void on_mouse_enter(Widget* widget);
    void on_mouse_leave(Widget* widget);

    // Fires per-frame while cursor is over a widget
    void on_mouse_over(Widget* widget);
    void tick(u32 now_ms);
    void hide();

    // Rasterizes the current tooltip (cached) and blits it; called by the display widget's draw_function
    void draw();

private:
    void ensure_attached(Group* root);
    void show(Widget* widget, const Tooltip& tt);
    void reposition(); // For updating Placement::Mouse tooltips after mouse move
    v2s16 compute_position(const Widget& widget, const v2u16& size) const;

    std::unique_ptr<Widget> m_display;
    Widget* m_hover = nullptr;
    u32 m_hover_since_ms = 0;
    bool m_showing = false;
    const Tooltip* m_current = nullptr;
    v2s16 m_position{}; // Screen space coords

    std::vector<u8> m_raster; // Cached tooltip raster for rendering efficiency
    v2u16 m_raster_size{};
    const Tooltip* m_raster_owner = nullptr;
    u8 m_raster_bg = 0;
    u8 m_raster_border = 0;
    bool m_raster_dirty = true;
};
} // namespace ui
