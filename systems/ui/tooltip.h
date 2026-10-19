#pragma once
#include "BW/enumerations.h"
#include "BW/structures/common.h"
#include "BW/structures/ui_structures.h"
#include "BW/ui.h"
#include "storm/smem.hpp"
#include "systems/graphics/grp.h"

#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace ui {

// Hover delay for tooltips shown through the anchored (non-widget) path, i.e. in-game
constexpr u16 GameTooltipDelayMs = 0;

constexpr u16 GameTooltipMaxWidth = 320;
constexpr u8 GameTooltipPadding = 4;

class Widget;
class Group;

enum class Placement {
    Auto, // Prefer below but flip if needed
    Below,
    Above,
    Right,
    Left,
    Mouse,
    At, // Top-left corner at the anchor's top-left
    RightBelow, // Right of the anchor, top edge at the anchor's bottom
};

struct TooltipColor {
    constexpr TooltipColor(TextColor color) : code(std::to_underlying(color)) {}
    constexpr TooltipColor(LobbyTextColor color) : code(std::to_underlying(color)) {}
    u8 code;
    bool operator==(const TooltipColor&) const = default;
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

    // Pieces of a single horizontal row, laid out left to right and top-aligned
    struct TextSpan {
        std::string string;
        TooltipColor color;
        bool operator==(const TextSpan&) const = default;
    };
    struct IconSpan {
        const graphics::GrpFile* grp;
        u16 frame;
        graphics::FrameFormat format = graphics::FrameFormat::Rle;
        bool operator==(const IconSpan&) const = default;
    };
    struct GapSpan {
        u16 width;
        bool operator==(const GapSpan&) const = default;
    };
    using Span = std::variant<TextSpan, IconSpan, GapSpan>;

    // Content
    Tooltip& text(std::string string, TooltipColor color = LobbyTextColor::Green);
    Tooltip& row(std::vector<Span> spans);
    Tooltip& image(const std::string& pcx_path);
    Tooltip& fill(u8 palette_index, u16 height);
    Tooltip& divider(std::optional<u8> palette_index = {}); // 1px line, nullopt = theme default color
    Tooltip& spacer(u16 height);

    // Appearance
    Tooltip& background(std::optional<u8> palette_index = {}); // Background fill color, nullopt = theme default
    Tooltip& border(bool value = true);
    Tooltip& padding(u8 pixels);
    Tooltip& max_width(std::optional<u16> pixels); // Text wraps to this box width; nullopt disables wrapping
    Tooltip& fixed_size(u16 w, u16 h);

    // Duplicate this tooltip and its image buffers
    Tooltip clone() const;

    // Same visible content and appearance; images compare by path
    bool operator==(const Tooltip& other) const;

private:
    friend class TooltipController;

    v2u16 measure() const;
    void rasterize(u8* pixels, v2u16 size) const;

    struct TextItem {
        std::string string;
        TooltipColor color;
        bool operator==(const TextItem&) const = default;
    };
    struct RowItem {
        std::vector<Span> spans;
        bool operator==(const RowItem&) const = default;
    };
    struct ImageItem {
        u16 width;
        u16 height;
        smem::ptr<u8[]> buffer;
        std::string path;
        bool operator==(const ImageItem& other) const {
            return path == other.path && width == other.width && height == other.height;
        }
    };
    struct FillItem {
        u8 palette_index;
        u16 height;
        bool operator==(const FillItem&) const = default;
    };
    struct DividerItem {
        std::optional<u8> palette_index;
        bool operator==(const DividerItem&) const = default;
    };
    struct SpacerItem {
        u16 height;
        bool operator==(const SpacerItem&) const = default;
    };
    using Item = std::variant<TextItem, RowItem, ImageItem, FillItem, DividerItem, SpacerItem>;

    static v2u16 measure_row(const RowItem& row);

    std::deque<Item> m_items;
    std::optional<u8> m_bg_palette_index; // Default filled with theme dialog bg
    bool m_has_border = true;
    u8 m_padding = 4;
    std::optional<u16> m_max_width = 400; // Text wraps to this box width; nullopt disables wrapping
    std::optional<v2u16> m_fixed_size;

    struct Layout {
        v2u16 size;
        std::vector<std::string> text; // Wrapped body per TextItem, in item order
    };
    const Layout& layout() const; // Lazily builds and caches m_layout
    mutable std::optional<Layout> m_layout; // Cached size + wrapped text, invalidated by any size-affecting mutator
};

// Screen-space rect the tooltip is placed relative to
struct TooltipAnchor {
    DialogBounds rect;
    Placement placement = Placement::Auto;
    u8 gap = 2; // Pixels between the anchor and the tooltip
    bool operator==(const TooltipAnchor&) const = default;
};

// Showing again for the same source updates the content without restarting the hover delay.
struct TooltipSource {
    enum class Kind : u8 {
        None,
        Text,
        CommandButton,
        StatWeapon,
        StatArmor,
        StatShield,
        StatSpecial,
        StatAction,
        Minimap,
        GameMenu,
    };
    Kind kind = Kind::None;
    u32 id = 0;
    bool operator==(const TooltipSource&) const = default;
};

// Rebuilds the tooltip content every controller tick; nullopt hides the tooltip
using TooltipBuilder = std::move_only_function<std::optional<Tooltip>()>;

// Where a rasterized tooltip is presented. Pixels passed to show() stay valid until the next show() or hide() call.
class TooltipSurface {
public:
    virtual ~TooltipSurface() = default;
    virtual void show(std::span<const u8> pixels, v2u16 size, v2s16 position) = 0;
    virtual void move(v2s16 position) = 0;
    virtual void hide() = 0;
};

// Presents the tooltip as a child dialog of a menu or modal's top dialog
class DialogTooltipSurface : public TooltipSurface {
public:
    explicit DialogTooltipSurface(Group& root);
    ~DialogTooltipSurface() override;

    void show(std::span<const u8> pixels, v2u16 size, v2s16 position) override;
    void move(v2s16 position) override;
    void hide() override;

private:
    void ensure_attached();
    void draw();

    Group& m_root;
    std::unique_ptr<Widget> m_display;
    std::span<const u8> m_pixels;
    v2u16 m_size{};
    v2s16 m_position{};
};

// Presents the tooltip on the screen's context layer (g_screen_layers.context), drawn above every dialog.
// Only one can exist, since there is one context layer.
class LayerTooltipSurface : public TooltipSurface {
public:
    LayerTooltipSurface();
    ~LayerTooltipSurface() override;

    LayerTooltipSurface(const LayerTooltipSurface&) = delete;
    LayerTooltipSurface& operator=(const LayerTooltipSurface&) = delete;

    static LayerTooltipSurface* active();

    void show(std::span<const u8> pixels, v2u16 size, v2s16 position) override;
    void move(v2s16 position) override;
    void hide() override;

    // Dialog repaints (0041C810) draw over the layer, so a dialog bitmap overlapping the tooltip gets it stamped in before presenting and restored afterwards
    void save_under(const graphics::Bitmap& dialog_bitmap, v2s16 dialog_top_left);
    void stamp(graphics::Bitmap& dialog_bitmap) const;
    void restore(graphics::Bitmap& dialog_bitmap);

private:
    static void __stdcall update_layer(graphics::Bitmap* surface, Bounds* bounds);

    void place_layer(v2s16 position, v2u16 size);
    void mark_layer_dirty() const;

    std::vector<u8> m_pixels;
    v2u16 m_size{};
    bool m_visible = false;

    std::vector<u8> m_saved; // Dialog pixels under the tooltip, tooltip-sized
    v2s16 m_saved_offset{};  // Tooltip position relative to the saved dialog
    bool m_has_saved = false;

    void (__stdcall* m_previous_update)(graphics::Bitmap*, Bounds*) = nullptr;
};

class TooltipController {
public:
    explicit TooltipController(std::unique_ptr<TooltipSurface> surface);
    ~TooltipController();

    TooltipController(const TooltipController&) = delete;
    TooltipController& operator=(const TooltipController&) = delete;

    // Widget hover path
    void on_mouse_enter(Widget* widget);
    void on_mouse_leave(Widget* widget);
    void on_mouse_over(Widget* widget); // Fires per-frame while cursor is over a widget

    // Anchored path, for hover detection done elsewhere (vanilla dialogs)
    void show(TooltipSource source, TooltipAnchor anchor, Tooltip content);
    void show(TooltipSource source, TooltipAnchor anchor, TooltipBuilder builder);

    // Drives hover delays, builder refreshes and theme changes
    void tick(u32 now_ms);
    void hide();
    // Also forgets the hovered widget; call before widgets are destroyed
    void clear();

    std::optional<TooltipSource> shown_source() const;

private:
    struct Request {
        TooltipSource source;
        TooltipAnchor anchor;
        std::optional<Tooltip> content;
        TooltipBuilder builder;
        bool content_changed = false;
        bool anchor_changed = false;
    };

    void start_request(TooltipSource source, TooltipAnchor anchor);
    void set_request_content(Tooltip content);
    bool rebuild_request(); // False when the builder dropped the tooltip
    void update_request(u32 now_ms);
    void tick_widget(u32 now_ms);

    void present(const Tooltip& tooltip, const TooltipAnchor& anchor);
    void reposition(const TooltipAnchor& anchor);
    void hide_surface();
    bool image_is_stale() const;
    TooltipAnchor widget_anchor(const Widget& widget) const;
    v2s16 compute_position(const TooltipAnchor& anchor, v2u16 size) const;

    std::unique_ptr<TooltipSurface> m_surface;

    Widget* m_hover = nullptr;
    std::optional<Request> m_request;
    u32 m_hover_since_ms = 0;
    bool m_showing = false;
    v2s16 m_position{}; // Screen space coords

    std::vector<u8> m_image;
    v2u16 m_image_size{};
    const Tooltip* m_image_source = nullptr; // Widget tooltip the image was rendered from
    u8 m_image_bg = 0;
    u8 m_image_border = 0;
};

// In-game tooltips on the context layer. Exists while the in-game HUD is loaded
inline std::unique_ptr<TooltipController> g_game_tooltips;

void hide_game_tooltip();
DialogBounds control_screen_bounds(const bw::Dialog& control);
Tooltip game_text_tooltip(std::string text);
} // namespace ui
