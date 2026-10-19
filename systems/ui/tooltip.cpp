#include "tooltip.h"
#include "widgets.h"

#include "BW/bwhelper.h"
#include "BW/bwdata.h"
#include "BW/ui.h"
#include "graphics/Bitmap.h"
#include "graphics/Font.h"
#include "hooks/display/grid.h"
#include "storm/storm.h"
#include "systems/display/display.h"
#include "systems/graphics/grp.h"
#include "systems/text/wrap.h"

#include <algorithm>
#include <cassert>
#include <string_view>
#include <utility>
#include <vector>

namespace ui {
constexpr s32 TooltipFontSize = graphics::FontSize::FONT_SMALL;
constexpr v2s16 MousePositionOffset{50, 50};

Tooltip::Tooltip(std::string string) {
    text(std::move(string));
}

Tooltip& Tooltip::text(std::string string, TooltipColor color) {
    m_items.push_back(TextItem{std::move(string), color});
    m_layout.reset();
    return *this;
}

Tooltip& Tooltip::row(std::vector<Span> spans) {
    m_items.push_back(RowItem{std::move(spans)});
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
            [&](RowItem const& row)         { out.m_items.emplace_back(RowItem{row.spans}); },
            [&](ImageItem const& image)     { if (!image.path.empty()) out.image(image.path); },
            [&](FillItem const& fill)       { out.m_items.emplace_back(FillItem{fill.palette_index, fill.height}); },
            [&](DividerItem const& divider) { out.m_items.emplace_back(DividerItem{divider.palette_index}); },
            [&](SpacerItem const& spacer)   { out.m_items.emplace_back(SpacerItem{spacer.height}); },
        }, item);
    }
    return out;
}

bool Tooltip::operator==(const Tooltip& other) const {
    return m_items == other.m_items
        && m_bg_palette_index == other.m_bg_palette_index
        && m_has_border == other.m_has_border
        && m_padding == other.m_padding
        && m_max_width == other.m_max_width
        && m_fixed_size == other.m_fixed_size;
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

v2u16 Tooltip::measure_row(const RowItem& row) {
    u16 width = 0;
    u16 height = 0;
    for (auto const& span : row.spans) {
        std::visit(Overloaded{
            [&](TextSpan const& text) {
                width = static_cast<u16>(width + graphics::Font::getTextWidth(text.string, TooltipFontSize));
                height = std::max<u16>(height, static_cast<u16>(graphics::Font::getTextHeight(text.string, TooltipFontSize)));
            },
            [&](IconSpan const& icon) {
                if (!icon.grp || icon.frame >= icon.grp->frame_count()) return;
                const graphics::FrameSize frame = icon.grp->frame_size(icon.frame);
                width = static_cast<u16>(width + frame.width);
                height = std::max<u16>(height, static_cast<u16>(frame.height));
            },
            [&](GapSpan const& gap) { width = static_cast<u16>(width + gap.width); },
        }, span);
    }
    return v2u16{width, height};
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
                    width = static_cast<u16>(text::max_line_width(TooltipFontSize, body));
                }
                content_h = static_cast<u16>(content_h + graphics::Font::getTextHeight(body.c_str(), TooltipFontSize));
                if (width > content_w) {
                    content_w = width;
                }
                out.text.push_back(std::move(body));
            },
            [&](RowItem const& row) {
                const v2u16 size = measure_row(row);
                content_h = static_cast<u16>(content_h + size.y);
                content_w = std::max(content_w, size.x);
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
                colored.push_back(static_cast<char>(text.color.code));
                colored.append(body);
                canvas.draw_text(colored, cursor_x, cursor_y, TooltipFontSize);
                cursor_y = static_cast<s16>(cursor_y +
                    graphics::Font::getTextHeight(body.c_str(), TooltipFontSize));
            },
            [&](RowItem const& row) {
                s16 span_x = cursor_x;
                for (auto const& span : row.spans) {
                    std::visit(Overloaded{
                        [&](TextSpan const& text) {
                            colored.clear();
                            colored.push_back(static_cast<char>(text.color.code));
                            colored.append(text.string);
                            canvas.draw_text(colored, span_x, cursor_y, TooltipFontSize);
                            span_x = static_cast<s16>(span_x + graphics::Font::getTextWidth(text.string, TooltipFontSize));
                        },
                        [&](IconSpan const& icon) {
                            if (!icon.grp || icon.frame >= icon.grp->frame_count()) return;
                            icon.grp->blit_frame(icon.frame, canvas, {.x = span_x, .y = cursor_y, .format = icon.format});
                            span_x = static_cast<s16>(span_x + icon.grp->frame_size(icon.frame).width);
                        },
                        [&](GapSpan const& gap) { span_x = static_cast<s16>(span_x + gap.width); },
                    }, span);
                }
                cursor_y = static_cast<s16>(cursor_y + measure_row(row).y);
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

// --------------------------------
// DialogTooltipSurface
// --------------------------------
DialogTooltipSurface::DialogTooltipSurface(Group& root) : m_root(root) {}
DialogTooltipSurface::~DialogTooltipSurface() = default;

void DialogTooltipSurface::ensure_attached() {
    if (m_display) return;

    m_display = Group::make(DialogBounds::from_xywh(0, 0, 0, 0), DialogControlType::Image, SkipHitTestIndex);
    m_display->on_draw([this](Widget&) { draw(); });
    m_display->dialog()->flags = DialogFlags::None;

    m_root.attach_external_widget(m_display.get());
}

void DialogTooltipSurface::draw() {
    if (!(m_display->dialog()->flags & DialogFlags::Visible)) return;
    graphics::Bitmap* surface = g_compositor_active_draw_target;
    if (!surface || !surface->data) return;
    blit_raw_bitmap(*surface, m_pixels.data(), m_size, m_position);
}

void DialogTooltipSurface::show(std::span<const u8> pixels, v2u16 size, v2s16 position) {
    ensure_attached();
    auto* dialog = m_display->dialog();
    if (dialog->flags & DialogFlags::Visible) {
        // Content or size changed while visible; repaint what the old image covered
        m_display->invalidate_parent();
    }

    m_pixels = pixels;
    m_size = size;
    m_position = position;
    m_display->set_screen_bounds(DialogBounds::from_xywh(position.x, position.y, size.x, size.y));
    dialog->flags |= DialogFlags::Visible;
    dialog->flags |= DialogFlags::UpdateNeeded;
    helper::update_dialog(dialog->to_bindlg());
}

void DialogTooltipSurface::move(v2s16 position) {
    if (!m_display) return;
    m_position = position;
    m_display->set_screen_bounds(DialogBounds::from_xywh(position.x, position.y, m_size.x, m_size.y));
    m_display->invalidate_parent();
}

void DialogTooltipSurface::hide() {
    if (!m_display) return;
    m_display->dialog()->flags &= ~DialogFlags::Visible;
    m_display->invalidate_parent();
    m_pixels = {};
}

// --------------------------------
// LayerTooltipSurface
// --------------------------------
static LayerTooltipSurface* g_active_layer_surface = nullptr;

LayerTooltipSurface::LayerTooltipSurface() {
    assert(!g_active_layer_surface && "only one LayerTooltipSurface can own the context layer");
    g_active_layer_surface = this;

    Layer& layer = g_screen_layers.context;
    m_previous_update = layer.p_update;
    layer.p_update = &update_layer;
    layer.buffers = 1;
    layer.flags = LayerFlags::None;
    place_layer(v2s16{static_cast<s16>(display::SurfaceWidth), static_cast<s16>(display::SurfaceHeight)}, v2u16{});
    g_game_tooltip_visible = 0;
}

LayerTooltipSurface::~LayerTooltipSurface() {
    place_layer(v2s16{static_cast<s16>(display::SurfaceWidth), static_cast<s16>(display::SurfaceHeight)}, v2u16{});
    g_game_tooltip_visible = 0;
    g_screen_layers.context.p_update = m_previous_update;
    g_active_layer_surface = nullptr;
}

LayerTooltipSurface* LayerTooltipSurface::active() {
    return g_active_layer_surface;
}

void __stdcall LayerTooltipSurface::update_layer(graphics::Bitmap*, Bounds*) {
    const LayerTooltipSurface* self = g_active_layer_surface;
    if (!self || !self->m_visible) return;
    graphics::Bitmap* target = g_compositor_active_draw_target;
    if (!target || !target->data) return;

    const Layer& layer = g_screen_layers.context;
    blit_raw_bitmap(*target, self->m_pixels.data(), self->m_size,
        v2s16{static_cast<s16>(layer.left), static_cast<s16>(layer.top)});
}

void LayerTooltipSurface::place_layer(v2s16 position, v2u16 size) {
    Layer& layer = g_screen_layers.context;
    layer.left = static_cast<u16>(position.x);
    layer.top = static_cast<u16>(position.y);
    layer.width = size.x;
    layer.height = size.y;
    layer.flags |= LayerFlags::NeedsRefresh;
}

void LayerTooltipSurface::mark_layer_dirty() const {
    const Layer& layer = g_screen_layers.context;
    if (!layer.width || !layer.height) return;
    hooks::mark_rect(layer.left, layer.top, layer.left + layer.width - 1, layer.top + layer.height - 1);
}

void LayerTooltipSurface::show(std::span<const u8> pixels, v2u16 size, v2s16 position) {
    if (m_visible) {
        mark_layer_dirty();
    }
    m_pixels.assign(pixels.begin(), pixels.end());
    m_size = size;
    m_has_saved = false;
    place_layer(position, size);
    m_visible = true;
    g_game_tooltip_visible = 1;
    mark_layer_dirty();
}

void LayerTooltipSurface::move(v2s16 position) {
    if (!m_visible) return;
    mark_layer_dirty();
    m_has_saved = false;
    place_layer(position, m_size);
    mark_layer_dirty();
}

void LayerTooltipSurface::hide() {
    if (!m_visible) return;
    mark_layer_dirty();
    place_layer(v2s16{static_cast<s16>(display::SurfaceWidth), static_cast<s16>(display::SurfaceHeight)}, v2u16{});
    m_visible = false;
    m_has_saved = false;
    g_game_tooltip_visible = 0;
}

void LayerTooltipSurface::save_under(const graphics::Bitmap& dialog_bitmap, v2s16 dialog_top_left) {
    m_has_saved = false;
    if (!m_visible || !dialog_bitmap.data) return;

    const Layer& layer = g_screen_layers.context;
    m_saved_offset = v2s16{
        static_cast<s16>(layer.left - dialog_top_left.x),
        static_cast<s16>(layer.top - dialog_top_left.y),
    };
    m_saved.resize(static_cast<size_t>(m_size.x) * m_size.y);
    graphics::Bitmap saved{m_size.x, m_size.y, m_saved.data()};
    blit_raw_bitmap(saved, dialog_bitmap.data, v2u16{dialog_bitmap.width, dialog_bitmap.height},
        v2s16{static_cast<s16>(-m_saved_offset.x), static_cast<s16>(-m_saved_offset.y)});
    m_has_saved = true;
}

void LayerTooltipSurface::stamp(graphics::Bitmap& dialog_bitmap) const {
    if (!m_visible || !m_has_saved) return;
    blit_raw_bitmap(dialog_bitmap, m_pixels.data(), m_size, m_saved_offset);
}

void LayerTooltipSurface::restore(graphics::Bitmap& dialog_bitmap) {
    if (!m_has_saved) return;
    blit_raw_bitmap(dialog_bitmap, m_saved.data(), m_size, m_saved_offset);
    m_has_saved = false;
}

// --------------------------------
// TooltipController
// --------------------------------
TooltipController::TooltipController(std::unique_ptr<TooltipSurface> surface)
    : m_surface(std::move(surface)) {}

TooltipController::~TooltipController() = default;

void TooltipController::on_mouse_enter(Widget* widget) {
    if (widget == m_hover) return;
    hide_surface();
    m_request.reset();
    m_hover = widget;
    m_hover_since_ms = GetTickCount();
}

void TooltipController::on_mouse_leave(Widget* widget) {
    if (widget != m_hover) return;
    hide_surface();
    m_hover = nullptr;
    m_hover_since_ms = 0;
}

void TooltipController::on_mouse_over(Widget* widget) {
    if (!m_showing || widget != m_hover) return;
    if (widget->tooltip_placement() == Placement::Mouse) {
        reposition(widget_anchor(*widget));
    }
}

void TooltipController::show(TooltipSource source, TooltipAnchor anchor, Tooltip content) {
    start_request(source, anchor);
    m_request->builder = nullptr;
    set_request_content(std::move(content));
    update_request(GetTickCount());
}

void TooltipController::show(TooltipSource source, TooltipAnchor anchor, TooltipBuilder builder) {
    start_request(source, anchor);
    m_request->builder = std::move(builder);
    if (!rebuild_request()) {
        hide();
        return;
    }
    update_request(GetTickCount());
}

void TooltipController::tick(u32 now_ms) {
    if (!m_request) {
        tick_widget(now_ms);
        return;
    }
    if (!rebuild_request()) {
        hide();
        return;
    }
    update_request(now_ms);
}

void TooltipController::hide() {
    hide_surface();
    m_request.reset();
}

void TooltipController::clear() {
    hide();
    m_hover = nullptr;
    m_hover_since_ms = 0;
}

std::optional<TooltipSource> TooltipController::shown_source() const {
    if (!m_showing || !m_request) return std::nullopt;
    return m_request->source;
}

void TooltipController::start_request(TooltipSource source, TooltipAnchor anchor) {
    if (m_request && m_request->source == source) {
        if (!(m_request->anchor == anchor)) {
            m_request->anchor = anchor;
            m_request->anchor_changed = true;
        }
        return;
    }

    hide_surface();
    m_hover = nullptr;
    m_request = Request{.source = source, .anchor = anchor};
    m_hover_since_ms = GetTickCount();
}

void TooltipController::set_request_content(Tooltip content) {
    if (m_request->content && *m_request->content == content) return;
    m_request->content = std::move(content);
    m_request->content_changed = true;
}

bool TooltipController::rebuild_request() {
    if (!m_request->builder) return true;
    std::optional<Tooltip> built = m_request->builder();
    if (!built) return false;
    set_request_content(std::move(*built));
    return true;
}

void TooltipController::update_request(u32 now_ms) {
    if (!m_request->content) return;

    if (!m_showing) {
        if (static_cast<s32>(now_ms - m_hover_since_ms) < GameTooltipDelayMs) return;
        present(*m_request->content, m_request->anchor);
    } else if (m_request->content_changed || image_is_stale()) {
        present(*m_request->content, m_request->anchor);
    } else if (m_request->anchor_changed) {
        reposition(m_request->anchor);
    }
    m_request->content_changed = false;
    m_request->anchor_changed = false;
}

void TooltipController::tick_widget(u32 now_ms) {
    const bool still_over = m_hover && (m_hover->dialog()->flags & DialogFlags::MouseHovering);
    if (m_hover && !still_over) {
        hide_surface();
        m_hover = nullptr;
        m_hover_since_ms = 0;
    }
    if (!m_hover) return;

    const Tooltip* tooltip = m_hover->tooltip_content();
    if (m_showing) {
        if (!tooltip) {
            hide_surface();
        } else if (tooltip != m_image_source || image_is_stale()) {
            present(*tooltip, widget_anchor(*m_hover));
            m_image_source = tooltip;
        } else if (m_hover->tooltip_placement() == Placement::Mouse) {
            // Mouse-tracked tooltips follow the cursor as it moves
            reposition(widget_anchor(*m_hover));
        }
        return;
    }

    if (static_cast<s32>(now_ms - m_hover_since_ms) < m_hover->tooltip_delay_ms()) return;
    if (!tooltip) return;

    present(*tooltip, widget_anchor(*m_hover));
    m_image_source = tooltip;
}

void TooltipController::present(const Tooltip& tooltip, const TooltipAnchor& anchor) {
    const v2u16 size = tooltip.measure();
    if (!size.x || !size.y) {
        hide_surface();
        return;
    }

    m_image.resize(static_cast<size_t>(size.x) * size.y);
    tooltip.rasterize(m_image.data(), size);
    m_image_size = size;
    m_image_source = nullptr;
    m_image_bg = g_theme_dialog_bg;
    m_image_border = g_theme_border_color;

    m_position = compute_position(anchor, size);
    m_surface->show(m_image, size, m_position);
    m_showing = true;
}

void TooltipController::reposition(const TooltipAnchor& anchor) {
    if (!m_showing) return;
    const v2s16 position = compute_position(anchor, m_image_size);
    if (position == m_position) return;
    m_position = position;
    m_surface->move(position);
}

void TooltipController::hide_surface() {
    if (!m_showing) return;
    m_showing = false;
    m_image_source = nullptr;
    m_surface->hide();
}

bool TooltipController::image_is_stale() const {
    return m_image_bg != g_theme_dialog_bg || m_image_border != g_theme_border_color;
}

TooltipAnchor TooltipController::widget_anchor(const Widget& widget) const {
    return TooltipAnchor{.rect = widget.screen_bounds(), .placement = widget.tooltip_placement()};
}

v2s16 TooltipController::compute_position(const TooltipAnchor& anchor, v2u16 size) const {
    const DialogBounds& trigger = anchor.rect;

    const s16 trigger_left   = trigger.left();
    const s16 trigger_top    = trigger.top();
    const s16 trigger_right  = trigger.right();
    const s16 trigger_bottom = trigger.bottom();
    const s16 gap = anchor.gap;
    const s16 surface_width = static_cast<s16>(display::SurfaceWidth);

    s16 x = trigger_left;
    s16 y = static_cast<s16>(trigger_bottom + gap);
    bool left_aligned = false;
    switch (anchor.placement) {
        case Placement::Auto: {
            if (y + static_cast<s16>(size.y) > static_cast<s16>(display::SurfaceHeight)) {
                y = static_cast<s16>(trigger_top - static_cast<s16>(size.y) - gap);
            }
            left_aligned = true;
        } break;
        case Placement::Above: {
            y = static_cast<s16>(trigger_top - static_cast<s16>(size.y) - gap);
            left_aligned = true;
        } break;
        case Placement::Below: {
            left_aligned = true;
        } break;
        case Placement::Right: {
            x = static_cast<s16>(trigger_right + gap);
            y = trigger_top;
        } break;
        case Placement::Left: {
            x = static_cast<s16>(trigger_left - static_cast<s16>(size.x) - gap);
            y = trigger_top;
        } break;
        case Placement::Mouse: {
            x = static_cast<s16>(g_mouse->x + MousePositionOffset.x);
            y = static_cast<s16>(g_mouse->y + MousePositionOffset.y);
        } break;
        case Placement::At: {
            y = trigger_top;
        } break;
        case Placement::RightBelow: {
            x = static_cast<s16>(trigger_right + gap);
            y = trigger_bottom;
        } break;
    }

    // Tooltips aligned to the anchor's left edge flip to its right edge rather than when they'd run past the screen
    if (left_aligned && x + static_cast<s16>(size.x) > surface_width) {
        x = static_cast<s16>(trigger_right + 1 - static_cast<s16>(size.x));
    }

    // Clamp to screen
    if (x + static_cast<s16>(size.x) > surface_width) {
        x = static_cast<s16>(surface_width - size.x);
    }
    if (x < 0) {
        x = 0;
    }
    if (y + static_cast<s16>(size.y) > static_cast<s16>(display::SurfaceHeight)) {
        y = static_cast<s16>(display::SurfaceHeight - size.y);
    }
    if (y < 0) {
        y = 0;
    }

    return v2s16{x, y};
}

void hide_game_tooltip() {
    if (g_game_tooltips) {
        g_game_tooltips->hide();
    }
}

DialogBounds control_screen_bounds(const bw::Dialog& control) {
    if (control.type == DialogControlType::DialogBoxDC || !control.parent) return control.bounds;
    return control.bounds.offset(control.parent->bounds.left(), control.parent->bounds.top());
}

Tooltip game_text_tooltip(std::string text) {
    Tooltip tooltip;
    tooltip.text(std::move(text), TextColor::Grey).padding(GameTooltipPadding).max_width(GameTooltipMaxWidth);
    return tooltip;
}
} // namespace ui
