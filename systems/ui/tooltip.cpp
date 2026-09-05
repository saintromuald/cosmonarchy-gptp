#include "tooltip.h"
#include "widgets.h"

#include "BW/bwhelper.h"
#include "BW/bwdata.h"
#include "BW/ui.h"
#include "graphics/Bitmap.h"
#include "graphics/Font.h"
#include "storm/storm.h"

#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

namespace ui {
constexpr s32 TooltipFontSize = graphics::FontSize::FONT_SMALL;
constexpr v2s16 MousePositionOffset{25, 25};

Tooltip::Tooltip(std::string string) {
    text(std::move(string));
}

Tooltip& Tooltip::text(std::string string, LobbyTextColor color) {
    m_items.push_back(TextItem{std::move(string), color});
    m_layout.reset();
    return *this;
}

Tooltip& Tooltip::image(const std::string& pcx_path) {
    Bitmap image{};
    if (!load_bitmap(pcx_path.c_str(), image, nullptr, SFileScope::DiskAndMpq, false)) {
        return *this;
    }
    m_items.push_back(ImageItem{
        .width  = image.width,
        .height = image.height,
        .buffer = smem::ptr<u8[]>{image.buffer},
        .path   = pcx_path,
    });
    m_layout.reset();
    return *this;
}

Tooltip& Tooltip::fill(u8 palette_index, u16 height) {
    m_items.push_back(FillItem{palette_index, height});
    m_layout.reset();
    return *this;
}

Tooltip& Tooltip::divider(std::optional<u8> palette_index) {
    m_items.push_back(DividerItem{palette_index});
    m_layout.reset();
    return *this;
}

Tooltip& Tooltip::spacer(u16 height) {
    m_items.push_back(SpacerItem{height});
    m_layout.reset();
    return *this;
}

Tooltip& Tooltip::background(std::optional<u8> palette_index) { m_bg_palette_index = palette_index; return *this; }
Tooltip& Tooltip::border(bool value)                          { m_has_border = value; m_layout.reset(); return *this; }
Tooltip& Tooltip::padding(u8 pixels)                          { m_padding = pixels; m_layout.reset(); return *this; }
Tooltip& Tooltip::max_width(std::optional<u16> pixels)        { m_max_width = pixels; m_layout.reset(); return *this; }
Tooltip& Tooltip::fixed_size(u16 w, u16 h)                    { m_fixed_size = v2u16{w, h}; m_layout.reset(); return *this; }

// More explicit than copy constructor
Tooltip Tooltip::clone() const {
    Tooltip out;
    out.m_bg_palette_index = m_bg_palette_index;
    out.m_has_border       = m_has_border;
    out.m_padding          = m_padding;
    out.m_max_width        = m_max_width;
    out.m_fixed_size       = m_fixed_size;
    for (auto const& item : m_items) {
        std::visit(Overloaded{
            [&](TextItem const& text)       { out.m_items.emplace_back(TextItem{text.string, text.color}); },
            [&](ImageItem const& image)     { if (!image.path.empty()) out.image(image.path); },
            [&](FillItem const& fill)       { out.m_items.emplace_back(FillItem{fill.palette_index, fill.height}); },
            [&](DividerItem const& divider) { out.m_items.emplace_back(DividerItem{divider.palette_index}); },
            [&](SpacerItem const& spacer)   { out.m_items.emplace_back(SpacerItem{spacer.height}); },
        }, item);
    }
    return out;
}

// TODO: If/when we refactor text rendering move this helper there -Jesse
struct WrappedText { std::string text; u16 width; };
static WrappedText wrap_text(const std::string& text, s32 wrap_px, s32 font_size) {
    WrappedText out;
    u16 widest = 0;
    std::string line;

    auto flush = [&] {
        if (!out.text.empty()) out.text.push_back('\n');
        out.text.append(line);
        widest = std::max<u16>(widest, static_cast<u16>(graphics::Font::getTextWidth(line.c_str(), font_size)));
        line.clear();
    };

    auto pos = 0;
    while (pos < text.size()) {
        if (text[pos] == '\n') { flush(); ++pos; continue; }
        if (text[pos] == ' ')  { ++pos; continue; }

        const auto word_end = text.find_first_of(" \n", pos);
        const std::string_view word{text.data() + pos, (word_end == std::string::npos ? text.size() : word_end) - pos};
        pos += word.size();

        if (line.empty()) {
            line.assign(word);
            continue;
        }
        std::string candidate = line + ' ';
        candidate.append(word);
        if (graphics::Font::getTextWidth(candidate.c_str(), font_size) <= wrap_px) {
            line = std::move(candidate);
        } else {
            flush();
            line.assign(word);
        }
    }
    flush();

    out.width = widest;
    return out;
}

const Tooltip::Layout& Tooltip::layout() const {
    if (m_layout) return *m_layout;

    const auto pad = static_cast<u16>(2 * m_padding);
    std::optional<s32> wrap_px;
    if (m_fixed_size) {
        wrap_px = std::max<s32>(1, m_fixed_size->x - pad);
    } else if (m_max_width) {
        wrap_px = std::max<s32>(1, *m_max_width - pad);
    }

    Layout out;
    u16 content_h = 0;
    u16 content_w = 0;
    for (auto const& item : m_items) {
        std::visit(Overloaded{
            [&](TextItem const& text) {
                std::string body = text.string;
                u16 width;
                if (wrap_px) {
                    WrappedText wrapped = wrap_text(text.string, *wrap_px, TooltipFontSize);
                    body = std::move(wrapped.text);
                    width = wrapped.width;
                } else {
                    width = static_cast<u16>(graphics::Font::getTextWidth(body.c_str(), TooltipFontSize));
                }
                content_h = static_cast<u16>(content_h + graphics::Font::getTextHeight(body.c_str(), TooltipFontSize));
                if (width > content_w) {
                    content_w = width;
                }
                out.text.push_back(std::move(body));
            },
            [&](ImageItem const& image) {
                content_h = static_cast<u16>(content_h + image.height);
                if (image.width > content_w) {
                    content_w = image.width;
                }
            },
            [&](FillItem const& fill)      { content_h = static_cast<u16>(content_h + fill.height); },
            [&](DividerItem const&)        { content_h = static_cast<u16>(content_h + 1); },
            [&](SpacerItem const& spacer)  { content_h = static_cast<u16>(content_h + spacer.height); },
        }, item);
    }

    u16 outer_w = static_cast<u16>(content_w + pad);
    if (m_max_width) {
        outer_w = std::min<u16>(*m_max_width, outer_w);
    }
    out.size = m_fixed_size.value_or(v2u16{outer_w, static_cast<u16>(content_h + pad)});

    m_layout = std::move(out);
    return *m_layout;
}

v2u16 Tooltip::measure() const {
    return layout().size;
}

static void blit_raw_bitmap(graphics::Bitmap& surface, const u8* source_buffer, v2u16 size, v2s16 destination) {
    if (!source_buffer || !surface.data || !size.x || !size.y) return;
    const s16 sw = static_cast<s16>(surface.getWidth());
    const s16 sh = static_cast<s16>(surface.getHeight());

    const s16 x0 = std::clamp<s16>(destination.x, 0, sw);
    const s16 y0 = std::clamp<s16>(destination.y, 0, sh);
    const s16 x1 = std::clamp<s16>(static_cast<s16>(destination.x + size.x), 0, sw);
    const s16 y1 = std::clamp<s16>(static_cast<s16>(destination.y + size.y), 0, sh);
    if (x0 >= x1 || y0 >= y1) return;

    const u16 copy_w = static_cast<u16>(x1 - x0);
    for (s16 y = y0; y < y1; ++y) {
        const s16 sy = static_cast<s16>(y - destination.y);
        const s16 sx = static_cast<s16>(x0 - destination.x);
        std::copy_n(source_buffer + sy * size.x + sx, copy_w, surface.data + y * sw + x0);
    }
}

void Tooltip::rasterize(u8* pixels, v2u16 size) const {
    if (!pixels || !size.x || !size.y) return;

    // Draw into a temporary canvas to automatically clip and cache
    const Layout& lay = layout();
    std::fill_n(pixels, static_cast<size_t>(size.x) * size.y, m_bg_palette_index.value_or(g_theme_dialog_bg));
    graphics::Bitmap canvas{size.x, size.y, pixels};

    const s16 content_left  = m_padding;
    const s16 content_right = static_cast<s16>(size.x - m_padding - 1);
    s16 cursor_x = content_left;
    s16 cursor_y = m_padding;

    std::string colored;
    size_t text_index = 0;
    for (auto const& item : m_items) {
        std::visit(Overloaded{
            [&](TextItem const& text) {
                const std::string& body = lay.text[text_index++];
                colored.clear();
                colored.push_back(static_cast<char>(text.color));
                colored.append(body);
                canvas.blitString(colored.c_str(), cursor_x, cursor_y, TooltipFontSize);
                cursor_y = static_cast<s16>(cursor_y +
                    graphics::Font::getTextHeight(body.c_str(), TooltipFontSize));
            },
            [&](ImageItem const& image) {
                blit_raw_bitmap(canvas, image.buffer.get(),
                    v2u16{image.width, image.height}, v2s16{cursor_x, cursor_y});
                cursor_y = static_cast<s16>(cursor_y + image.height);
            },
            [&](FillItem const& fill) {
                canvas.draw_filled_box(content_left, cursor_y, content_right,
                    static_cast<s16>(cursor_y + fill.height - 1), fill.palette_index);
                cursor_y = static_cast<s16>(cursor_y + fill.height);
            },
            [&](DividerItem const& divider) {
                const u8 color = divider.palette_index.value_or(g_theme_border_color);
                canvas.drawHorizontalLine(content_left, content_right, cursor_y, color);
                cursor_y = static_cast<s16>(cursor_y + 1);
            },
            [&](SpacerItem const& spacer) {
                cursor_y = static_cast<s16>(cursor_y + spacer.height);
            },
        }, item);
    }

    if (m_has_border) {
        canvas.draw_box(0, 0, static_cast<s16>(size.x - 1), static_cast<s16>(size.y - 1), g_theme_border_color);
    }
}

static u32 __fastcall tooltip_display_draw(bw::Dialog* dialog, DialogEvent*, u32, void*) {
    if (!(dialog->flags & DialogFlags::Visible)) return 1;
    auto* widget = static_cast<Widget*>(dialog->user);
    if (!widget || !widget->parent()) return 1;
    auto* controller = widget->parent()->root()->tooltip_controller();
    if (!controller) return 1;
    controller->draw();
    return 1;
}

TooltipController::TooltipController() = default;
TooltipController::~TooltipController() = default;

void TooltipController::ensure_attached(Group* root) {
    if (m_display || !root) return;

    m_display = Group::make(DialogBounds::from_xywh(0, 0, 0, 0), DialogControlType::Image, SkipHitTestIndex);
    auto* dialog = m_display->dialog();
    dialog->draw_function = &tooltip_display_draw;
    dialog->flags = DialogFlags::None;

    root->attach_external_widget(m_display.get());
}

void TooltipController::on_mouse_enter(Widget* widget) {
    if (widget && widget->parent()) {
        ensure_attached(widget->parent()->root());
    }
    if (widget == m_hover) return;
    if (m_showing) {
        hide();
    }
    m_hover = widget;
    m_hover_since_ms = GetTickCount();
}

void TooltipController::on_mouse_leave(Widget* widget) {
    if (widget != m_hover) return;
    if (m_showing) {
        hide();
    }
    m_hover = nullptr;
    m_hover_since_ms = 0;
}

void TooltipController::on_mouse_over(Widget* widget) {
    if (!m_showing || widget != m_hover) return;
    if (widget->tooltip_placement() == Placement::Mouse) {
        reposition();
    }
}

void TooltipController::tick(u32 now_ms) {
    const bool still_over = m_hover && (m_hover->dialog()->flags & DialogFlags::MouseHovering);
    if (m_hover && !still_over) {
        if (m_showing) {
            hide();
        }
        m_hover = nullptr;
        m_hover_since_ms = 0;
    }
    if (!m_hover) return;

    // Already up: mouse-tracked tooltips follow the cursor as it moves.
    if (m_showing) {
        if (m_hover->tooltip_placement() == Placement::Mouse) {
            reposition();
        }
        return;
    }

    if (static_cast<u32>(now_ms - m_hover_since_ms) < m_hover->tooltip_delay_ms()) return;

    const Tooltip* tooltip = m_hover->tooltip_content();
    if (!tooltip) return;

    show(m_hover, *tooltip);
}

void TooltipController::hide() {
    if (!m_showing || !m_display) return;
    m_showing = false;
    m_current = nullptr;
    m_raster_owner = nullptr;
    m_display->dialog()->flags &= ~DialogFlags::Visible;
    m_display->invalidate_parent();
}

void TooltipController::show(Widget* widget, const Tooltip& tooltip) {
    if (!m_display) return;
    m_current = &tooltip;
    m_raster_dirty = true;
    const v2u16 size = tooltip.measure();
    m_position = compute_position(*widget, size);
    m_display->set_screen_bounds(DialogBounds::from_xywh(m_position.x, m_position.y, size.x, size.y));
    m_showing = true;
    auto* dialog = m_display->dialog();
    dialog->flags |= DialogFlags::Visible;
    dialog->flags |= DialogFlags::UpdateNeeded;
    helper::update_dialog(dialog->to_bindlg());
}

void TooltipController::draw() {
    if (!m_current) return;
    graphics::Bitmap* surface = g_compositor_active_draw_target;
    if (!surface || !surface->data) return;

    const v2u16 size = m_current->measure();
    if (!size.x || !size.y) return;

    // Rasterize only on content change, repositioning just re-blits cached bitmap
    const bool stale = m_raster_dirty
        || m_raster_owner != m_current
        || m_raster_size != size
        || m_raster_bg != g_theme_dialog_bg
        || m_raster_border != g_theme_border_color;
    if (stale) {
        m_raster.resize(static_cast<size_t>(size.x) * size.y);
        m_current->rasterize(m_raster.data(), size);
        m_raster_owner = m_current;
        m_raster_size = size;
        m_raster_bg = g_theme_dialog_bg;
        m_raster_border = g_theme_border_color;
        m_raster_dirty = false;
    }

    blit_raw_bitmap(*surface, m_raster.data(), size, m_position);
}

void TooltipController::reposition() {
    if (!m_showing || !m_current || !m_display || !m_hover) return;
    const v2u16 size = m_current->measure();
    const v2s16 new_position = compute_position(*m_hover, size);
    if (new_position == m_position) return;

    m_position = new_position;
    m_display->set_screen_bounds(DialogBounds::from_xywh(m_position.x, m_position.y, size.x, size.y));
    // Mark parent dirty so the compositor repaints the vacated area with the
    // underlying widget content before our display redraws at the new spot
    m_display->invalidate_parent();
}

v2s16 TooltipController::compute_position(const Widget& widget, const v2u16& size) const {
    const Placement placement = widget.tooltip_placement();
    const DialogBounds trigger = widget.screen_bounds();

    const s16 trigger_left   = trigger.left();
    const s16 trigger_top    = trigger.top();
    const s16 trigger_right  = trigger.right();
    const s16 trigger_bottom = trigger.bottom();

    s16 x = trigger_left;
    s16 y = static_cast<s16>(trigger_bottom + 2);
    switch (placement) {
        case Placement::Auto: {
            if (y + static_cast<s16>(size.y) > static_cast<s16>(ScreenHeight)) {
                y = static_cast<s16>(trigger_top - static_cast<s16>(size.y) - 2);
            }
        } break;
        case Placement::Above: {
            y = static_cast<s16>(trigger_top - static_cast<s16>(size.y) - 2);
        } break;
        case Placement::Right: {
            x = static_cast<s16>(trigger_right + 2);
            y = trigger_top;
        } break;
        case Placement::Left: {
            x = static_cast<s16>(trigger_left - static_cast<s16>(size.x) - 2);
            y = trigger_top;
        } break;
        case Placement::Below: break;
        case Placement::Mouse: {
            x = static_cast<s16>(g_mouse->x + MousePositionOffset.x);
            y = static_cast<s16>(g_mouse->y + MousePositionOffset.y);
        } break;
    }

    // Clamp to screen
    if (x + static_cast<s16>(size.x) > static_cast<s16>(ScreenWidth)) {
        x = static_cast<s16>(ScreenWidth - size.x);
    }
    if (x < 0) {
        x = 0;
    }
    if (y + static_cast<s16>(size.y) > static_cast<s16>(ScreenHeight)) {
        y = static_cast<s16>(ScreenHeight - size.y);
    }
    if (y < 0) {
        y = 0;
    }

    return v2s16{x, y};
}
} // namespace ui
