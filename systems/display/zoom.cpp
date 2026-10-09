#include "zoom.h"
#include "display.h"
#include "../hooks/display/grid.h"
#include "../../config.h"
#include "../../BW/bwdata.h"
#include "../../graphics/expand.h"

#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <vector>

namespace {

// Returns source pixel offset centered on destination pixel midpoint.
u32 sample_offset(s32 presented, u32 extent, u32 output_extent) {
    presented = std::clamp(presented, 0, s32(output_extent) - 1);
    return u32((2 * u64(presented) + 1) * extent / (2 * u64(output_extent)));
}

// Computes crop offset along one axis to keep anchored world pixel under cursor.
// The crop slides within the rendered world buffer (container), not the larger
// display rect it is magnified into.
u32 anchored_crop(s64 world, u32 camera, s32 presented, u32 output_extent, u32 container_extent, u32 extent) {
    const s64 requested = world - s64(camera) - s64(sample_offset(presented, extent, output_extent));
    return u32(std::clamp<s64>(requested, 0, s64(container_extent) - s64(extent)));
}

// Computes crop position, sliding to boundary margins when camera reaches map edges.
u32 edge_aware_crop(u32 container_extent, u32 extent, u32 preferred, u32 camera, u32 max_camera) {
    const u32 slack = container_extent - extent;
    preferred = std::min(preferred, slack);
    if (max_camera == 0) {
        return preferred;
    }

    const u32 clamped_camera = std::min(camera, max_camera);

    if (max_camera < slack) {
        return u32(u64(slack) * clamped_camera / max_camera);
    }
    if (clamped_camera < preferred) {
        return clamped_camera;
    }

    const u32 far_margin = slack - preferred;
    const u32 distance_to_far_edge = max_camera - clamped_camera;
    if (distance_to_far_edge < far_margin) {
        return slack - distance_to_far_edge;
    }
    return preferred;
}

// Binary searches for monotonic camera position matching target world origin.
u32 camera_for_origin(s64 origin, u32 container_extent, u32 extent, u32 preferred, u32 max_camera) {
    const auto visible_origin = [=](u32 camera) {
        return s64(camera) + edge_aware_crop(container_extent, extent, preferred, camera, max_camera);
    };

    u32 low = 0;
    u32 high = max_camera;
    while (low < high) {
        const u32 middle = low + (high - low) / 2;
        if (visible_origin(middle) < origin) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    if (low && std::abs(visible_origin(low - 1) - origin) <= std::abs(visible_origin(low) - origin)) {
        --low;
    }
    return low;
}

// Lands wheel steps exactly on a detent when they would cross it, so an extra
// notch is required to pass and approaching always stops dead on the value.
u32 detent_target(u32 from_units, u32 target_units) {
    for (const u32 detent : {zoom::Detent200, zoom::Detent300}) {
        if ((from_units < detent && target_units > detent) || (from_units > detent && target_units < detent)) {
            return detent;
        }
    }
    return target_units;
}

u32 max_camera_x() {
    return u32(g_display.scroll_max_x(g_map_width_pixels));
}

u32 max_camera_y() {
    return u32(g_display.scroll_max_y(g_map_height_pixels));
}

} // namespace

void WorldZoom::rebuild_lookup() {
    const u32 output_width = g_display.display_width();
    const u32 output_height = g_display.display_height();

    for (u32 x = 0; x < output_width; ++x) {
        m_x_lookup[x] = u16((2 * u64(x) + 1) * m_crop.width / (2 * u64(output_width)));
    }
    for (u32 y = 0; y < output_height; ++y) {
        m_y_lookup[y] = u16((2 * u64(y) + 1) * m_crop.height / (2 * u64(output_height)));
    }
    m_lookup_valid = true;
}

void WorldZoom::advance_transition(u32 now) {
    if (m_current_units == m_target_units) {
        return;
    }

    const u32 elapsed = now - m_transition_start_tick;
    if (!g_config.smooth_world_zoom || elapsed >= zoom::TransitionMs) {
        m_current_units = m_target_units;
        return;
    }

    const u64 eased = u64(elapsed) * (2 * zoom::TransitionMs - elapsed);
    const s64 distance = s64(m_target_units) - s64(m_transition_start_units);
    const s64 span = s64(u64(zoom::TransitionMs) * zoom::TransitionMs);
    m_current_units = u32(s64(m_transition_start_units) + distance * s64(eased) / span);
}

u32 WorldZoom::source_extent(u32 output_extent) const {
    const u32 extent = u32(u64(output_extent) * zoom::BaseUnits / m_current_units);
    return std::clamp(extent, 1u, output_extent);
}

u32 WorldZoom::floor_units() const {
    return u32(g_display.min_zoom_percent()) * 100;
}

u32 WorldZoom::ceiling_units() const {
    return zoom::MaxPercent * 100;
}

u32 WorldZoom::default_units() const {
    return std::clamp(zoom::DefaultPercent * 100, floor_units(), ceiling_units());
}

void WorldZoom::init() {
    m_current_units = zoom::BaseUnits;
    m_transition_start_units = m_current_units;
    m_target_units = m_current_units;
    m_transition_start_tick = GetTickCount();
    m_last_adjusted_tick = 0;
    m_anchor = {};
    m_lookup_valid = false;

    // Sized for the largest crop any role can show (the full display rect);
    // begin_game() resets the zoom state once the role is known.
    m_scratch.assign(size_t(g_display.display_width()) * g_display.display_height(), 0);
}

void WorldZoom::begin_game() {
    const u32 units = default_units();
    m_current_units = units;
    m_transition_start_units = units;
    m_target_units = units;
    m_transition_start_tick = GetTickCount();
    m_last_adjusted_tick = 0;
    m_anchor = {};
    m_lookup_valid = false;
}

bool WorldZoom::enabled() const {
    return g_config.world_zoom;
}

bool WorldZoom::magnified() const {
    return m_current_units > zoom::BaseUnits;
}

bool WorldZoom::needs_transform() const {
    return m_current_units != m_target_units || m_anchor.active || m_current_units > zoom::BaseUnits;
}

u32 WorldZoom::percent() const {
    return (m_current_units + 50) / 100;
}

// Maps engine 100%-300% to 50%-150% (200% is player default)
u32 WorldZoom::player_percent() const {
    const u32 default_units = zoom::DefaultPercent * 100;
    return (m_current_units * 100 + default_units / 2) / default_units;
}

void WorldZoom::update_transform(bool advance_animation) {
    if (advance_animation) {
        advance_transition(GetTickCount());
    }

    const u32 output_width = g_display.display_width();
    const u32 output_height = g_display.display_height();
    const u32 world_width = g_display.world_width();
    const u32 world_height = g_display.world_height();
    u32 width = std::min(source_extent(output_width), world_width);
    u32 height = std::min(source_extent(output_height), world_height);

    // Preferred rest position: the crop centered within the rendered world buffer.
    u32 left = (world_width - width) / 2;
    u32 top = (world_height - height) / 2;

    if (m_anchor.active) {
        left = anchored_crop(m_anchor.world.x, g_move_to_x, m_anchor.surface.x, output_width, world_width, width);
        top = anchored_crop(m_anchor.world.y, g_move_to_y, m_anchor.surface.y, output_height, world_height, height);
    } else {
        left = edge_aware_crop(world_width, width, left, g_move_to_x, max_camera_x());
        top = edge_aware_crop(world_height, height, top, g_move_to_y, max_camera_y());
    }

    const bool extents_changed = !m_lookup_valid || m_crop.width != width || m_crop.height != height;
    m_crop.left = u16(left);
    m_crop.top = u16(top);
    m_crop.width = u16(width);
    m_crop.height = u16(height);
    if (extents_changed) {
        rebuild_lookup();
    }
}

u16 WorldZoom::source_left() const {
    return magnified() && m_lookup_valid ? m_crop.left : 0;
}

u16 WorldZoom::source_top() const {
    return magnified() && m_lookup_valid ? m_crop.top : 0;
}

u16 WorldZoom::visible_width() const {
    return magnified() && m_lookup_valid ? m_crop.width : g_display.world_width();
}

u16 WorldZoom::visible_height() const {
    return magnified() && m_lookup_valid ? m_crop.height : g_display.world_height();
}

v2s32 WorldZoom::surface_to_world_offset(v2s32 surface_point) const {
    if (!magnified() || !m_lookup_valid) {
        return surface_point;
    }

    const s32 x = std::clamp(surface_point.x, 0, s32(g_display.display_width()) - 1);
    const s32 y = std::clamp(surface_point.y, 0, s32(g_display.display_height()) - 1);
    return { s32(m_crop.left) + s32(m_x_lookup[x]), s32(m_crop.top) + s32(m_y_lookup[y]) };
}

void WorldZoom::begin_transition(u32 target, v2s32 surface_point) {
    const v2s32 surface = {
        std::clamp(surface_point.x, 0, s32(g_display.display_width()) - 1),
        std::clamp(surface_point.y, 0, s32(g_display.display_height()) - 1),
    };
    if (!m_anchor.active || m_anchor.surface != surface) {
        const v2s32 offset = surface_to_world_offset(surface);
        m_anchor.active = true;
        m_anchor.surface = surface;
        m_anchor.world = { s32(g_move_to_x) + offset.x, s32(g_move_to_y) + offset.y };
    }

    m_transition_start_units = m_current_units;
    m_target_units = target;
    m_transition_start_tick = GetTickCount();
}

bool WorldZoom::adjust_by_wheel(s32 notches, v2s32 surface_point) {
    if (!enabled() || notches == 0) {
        return false;
    }

    m_last_adjusted_tick = GetTickCount();

    const s64 stepped = s64(m_target_units) + s64(notches) * zoom::StepUnits;
    const u32 clamped = u32(std::clamp<s64>(stepped, floor_units(), ceiling_units()));
    const u32 target = detent_target(m_target_units, clamped);
    if (target == m_target_units) {
        return true;
    }
    begin_transition(target, surface_point);
    return true;
}

bool WorldZoom::reset_to_default(v2s32 surface_point) {
    if (!enabled()) {
        return false;
    }

    m_last_adjusted_tick = GetTickCount();

    const u32 target = default_units();
    if (target == m_target_units) {
        return true;
    }
    begin_transition(target, surface_point);
    return true;
}

bool WorldZoom::resolve_camera_anchor(v2s32* adjusted) {
    if (!m_anchor.active || !m_lookup_valid || !adjusted) {
        return false;
    }

    const u32 output_width = g_display.display_width();
    const u32 output_height = g_display.display_height();
    const u32 world_width = g_display.world_width();
    const u32 world_height = g_display.world_height();

    if (m_current_units == m_target_units) {
        // Hands off residual crop offset to camera upon transition landing and releases anchor.
        const u32 centred_left = (world_width - m_crop.width) / 2;
        const u32 centred_top = (world_height - m_crop.height) / 2;

        adjusted->x = s32(camera_for_origin(
            m_anchor.world.x - s64(sample_offset(m_anchor.surface.x, m_crop.width, output_width)),
            world_width, m_crop.width, centred_left, max_camera_x()));
        adjusted->y = s32(camera_for_origin(
            m_anchor.world.y - s64(sample_offset(m_anchor.surface.y, m_crop.height, output_height)),
            world_height, m_crop.height, centred_top, max_camera_y()));
        m_anchor = {};

        return true;
    }

    const s64 held_x = s64(g_move_to_x) + m_crop.left + s64(sample_offset(m_anchor.surface.x, m_crop.width, output_width));
    const s64 held_y = s64(g_move_to_y) + m_crop.top + s64(sample_offset(m_anchor.surface.y, m_crop.height, output_height));

    adjusted->x = s32(std::clamp<s64>(s64(g_move_to_x) + m_anchor.world.x - held_x, 0, max_camera_x()));
    adjusted->y = s32(std::clamp<s64>(s64(g_move_to_y) + m_anchor.world.y - held_y, 0, max_camera_y()));
    return u32(adjusted->x) != g_move_to_x || u32(adjusted->y) != g_move_to_y;
}

v2s32 WorldZoom::camera_for_visible_origin(v2s32 origin) const {
    if (!magnified() || !m_lookup_valid) {
        return origin;
    }

    const u32 world_width = g_display.world_width();
    const u32 world_height = g_display.world_height();
    return {
        s32(camera_for_origin(origin.x, world_width, m_crop.width, (world_width - m_crop.width) / 2, max_camera_x())),
        s32(camera_for_origin(origin.y, world_height, m_crop.height, (world_height - m_crop.height) / 2, max_camera_y())),
    };
}

v2s32 WorldZoom::camera_for_center(v2s32 point) const {
    return camera_for_visible_origin({
        point.x - s32(visible_width()) / 2,
        point.y - s32(visible_height()) / 2,
    });
}

// The lookup is monotonic non-decreasing, so the output pixels covering a world
// range are the contiguous span whose source columns fall inside it.
zoom::PixelSpan WorldZoom::surface_span_for_world_x(s32 world_begin, s32 world_end) const {
    zoom::PixelSpan span;
    if (!magnified() || !m_lookup_valid) {
        // World pixels are surface pixels at 100%.
        world_begin = std::max(world_begin, 0);
        world_end = std::min(world_end, s32(g_display.display_width()));
        if (world_begin >= world_end) {
            return span;
        }
        span.begin = u16(world_begin);
        span.end = u16(world_end);
        return span;
    }

    const s32 crop_begin = world_begin - s32(m_crop.left);
    const s32 crop_end = world_end - s32(m_crop.left);
    if (crop_end <= 0 || crop_begin >= s32(m_crop.width)) {
        return span;
    }

    const u16* const first = m_x_lookup.data();
    const u16* const last = m_x_lookup.data() + g_display.display_width();
    const u16* begin_pixel = std::lower_bound(first, last, u16(std::max(crop_begin, 0)));
    const u16* end_pixel = std::lower_bound(first, last, u16(std::min(crop_end, s32(m_crop.width))));
    span.begin = u16(begin_pixel - first);
    span.end = u16(end_pixel - first);
    return span;
}

zoom::PixelSpan WorldZoom::surface_span_for_world_y(s32 world_begin, s32 world_end) const {
    zoom::PixelSpan span;
    if (!magnified() || !m_lookup_valid) {
        world_begin = std::max(world_begin, 0);
        world_end = std::min(world_end, s32(g_display.display_height()));
        if (world_begin >= world_end) {
            return span;
        }
        span.begin = u16(world_begin);
        span.end = u16(world_end);
        return span;
    }

    const s32 crop_begin = world_begin - s32(m_crop.top);
    const s32 crop_end = world_end - s32(m_crop.top);
    if (crop_end <= 0 || crop_begin >= s32(m_crop.height)) {
        return span;
    }

    const u16* const first = m_y_lookup.data();
    const u16* const last = m_y_lookup.data() + g_display.display_height();
    const u16* begin_pixel = std::lower_bound(first, last, u16(std::max(crop_begin, 0)));
    const u16* end_pixel = std::lower_bound(first, last, u16(std::min(crop_end, s32(m_crop.height))));
    span.begin = u16(begin_pixel - first);
    span.end = u16(end_pixel - first);
    return span;
}

// Output pixels sample source columns via the same monotonic lookup, so the
// world range an output range reads is the lookup range at its edges.
zoom::PixelSpan WorldZoom::world_span_for_surface_x(s32 surface_begin, s32 surface_end) const {
    zoom::PixelSpan span;
    if (!magnified() || !m_lookup_valid) {
        surface_begin = std::max(surface_begin, 0);
        surface_end = std::min(surface_end, s32(g_display.display_width()));
        if (surface_begin >= surface_end) {
            return span;
        }
        span.begin = u16(surface_begin);
        span.end = u16(surface_end);
        return span;
    }

    const s32 width = s32(g_display.display_width());
    surface_begin = std::clamp(surface_begin, 0, width - 1);
    surface_end = std::clamp(surface_end, 0, width);
    if (surface_begin >= surface_end) {
        return span;
    }
    span.begin = u16(s32(m_crop.left) + m_x_lookup[surface_begin]);
    span.end = u16(s32(m_crop.left) + m_x_lookup[surface_end - 1] + 1);
    return span;
}

zoom::PixelSpan WorldZoom::world_span_for_surface_y(s32 surface_begin, s32 surface_end) const {
    zoom::PixelSpan span;
    if (!magnified() || !m_lookup_valid) {
        surface_begin = std::max(surface_begin, 0);
        surface_end = std::min(surface_end, s32(g_display.display_height()));
        if (surface_begin >= surface_end) {
            return span;
        }
        span.begin = u16(surface_begin);
        span.end = u16(surface_end);
        return span;
    }

    const s32 height = s32(g_display.display_height());
    surface_begin = std::clamp(surface_begin, 0, height - 1);
    surface_end = std::clamp(surface_end, 0, height);
    if (surface_begin >= surface_end) {
        return span;
    }
    span.begin = u16(s32(m_crop.top) + m_y_lookup[surface_begin]);
    span.end = u16(s32(m_crop.top) + m_y_lookup[surface_end - 1] + 1);
    return span;
}

void WorldZoom::scale_world(u8* destination, const u8* world, s32 world_pitch) {
    if (!magnified() || !m_lookup_valid || !destination || !world || m_scratch.empty()) {
        return;
    }

    const u32 pitch = g_display.surface_width();
    const u32 output_width = g_display.display_width();
    const u32 output_height = g_display.display_height();
    const u32 width = m_crop.width;
    const u32 height = m_crop.height;

    // Stage the crop contiguously; the integer fast paths expand from a packed source.
    for (u32 y = 0; y < height; ++y) {
        std::copy_n(world + size_t(m_crop.top + y) * world_pitch + m_crop.left, width, m_scratch.data() + size_t(y) * width);
    }

    // Integer fast paths: exact 2x/3x magnification is a byte/row expansion.
    if (width * 2 == output_width && height * 2 == output_height) {
        graphics::expand_integer<2>(m_scratch.data(), destination, pitch, width, height);
        return;
    }
    if (width * 3 == output_width && height * 3 == output_height) {
        graphics::expand_integer<3>(m_scratch.data(), destination, pitch, width, height);
        return;
    }

    for (u32 y = 0; y < output_height; ++y) {
        const u8* source_row = m_scratch.data() + size_t(m_y_lookup[y]) * width;
        u8* destination_row = destination + size_t(y) * pitch;
        for (u32 x = 0; x < output_width; ++x) {
            destination_row[x] = source_row[m_x_lookup[x]];
        }
    }
}

// Writes one output rect; the same lookup indexing as the full resample, reading
// the crop straight out of the world buffer.
void WorldZoom::resample_rect(u8* destination, const u8* world, s32 world_pitch,
    u32 out_left, u32 out_top, u32 out_right, u32 out_bottom) {
    const u32 pitch = g_display.surface_width();
    for (u32 y = out_top; y < out_bottom; ++y) {
        const u8* source_row = world + size_t(m_crop.top + m_y_lookup[y]) * world_pitch + m_crop.left;
        u8* destination_row = destination + size_t(y) * pitch + out_left;
        for (u32 x = 0; x < out_right - out_left; ++x) {
            destination_row[x] = source_row[m_x_lookup[out_left + x]];
        }
    }
}

// Resamples the exact output span of each marked world cell. Driving from the world
// grid (the cells that were actually reprocessed) rather than the surface grid keeps
// vanilla's invariant - presentation only reads content that was blitted+fogged this
// frame. Whole-surface-cell resampling at non-integer magnifications sampled past the
// marked world cells into unmarked ones, presenting stale content and sprite-redraw
// spills at the edges of updated regions.
void WorldZoom::scale_world_region(u8* destination, const u8* world, s32 world_pitch) {
    if (!magnified() || !m_lookup_valid || !destination || !world) {
        return;
    }

    const u32 cell = g_display.cell_size();
    const u32 cols = u32(display::world_phase_cols());
    const u32 rows = u32(display::world_phase_rows());
    const u8* cells = display::world_phase_cells();
    const u32 world_width = g_display.world_width();
    const u32 world_height = g_display.world_height();

    for (u32 row = 0; row < rows; ++row) {
        for (u32 col = 0; col < cols; ++col) {
            if (!cells[row * cols + col]) {
                continue;
            }
            const u32 world_left = col * cell;
            const u32 world_top = row * cell;
            const u32 world_right = std::min(world_left + cell, world_width);
            const u32 world_bottom = std::min(world_top + cell, world_height);

            const zoom::PixelSpan span_x = surface_span_for_world_x(s32(world_left), s32(world_right));
            const zoom::PixelSpan span_y = surface_span_for_world_y(s32(world_top), s32(world_bottom));
            if (span_x.empty() || span_y.empty()) {
                continue;
            }
            resample_rect(destination, world, world_pitch, span_x.begin, span_y.begin, span_x.end, span_y.end);
        }
    }
}
