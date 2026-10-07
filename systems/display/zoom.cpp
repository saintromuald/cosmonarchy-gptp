#include "zoom.h"
#include "display.h"
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

void WorldZoom::init() {
    m_current_units = zoom::BaseUnits;
    m_transition_start_units = m_current_units;
    m_target_units = m_current_units;
    m_transition_start_tick = GetTickCount();
    m_anchor = {};
    m_lookup_valid = false;

    // Sized for the largest crop any role can show (the full display rect);
    // begin_game() resets the zoom state once the role is known.
    m_scratch.assign(size_t(g_display.display_width()) * g_display.display_height(), 0);
}

void WorldZoom::begin_game() {
    const u32 units = std::clamp(zoom::DefaultPercent * 100, floor_units(), ceiling_units());
    m_current_units = units;
    m_transition_start_units = units;
    m_target_units = units;
    m_transition_start_tick = GetTickCount();
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

bool WorldZoom::adjust_by_wheel(s32 notches, v2s32 surface_point) {
    if (!enabled() || notches == 0) {
        return false;
    }

    const s64 stepped = s64(m_target_units) + s64(notches) * zoom::StepUnits;
    const u32 clamped = u32(std::clamp<s64>(stepped, floor_units(), ceiling_units()));
    const u32 target = detent_target(m_target_units, clamped);
    if (target == m_target_units) {
        return true;
    }

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

void WorldZoom::scale_world(u8* surface) {
    if (!magnified() || !m_lookup_valid || !surface || m_scratch.empty()) {
        return;
    }

    const u32 pitch = g_display.surface_width();
    const u32 output_width = g_display.display_width();
    const u32 output_height = g_display.display_height();
    const u32 width = m_crop.width;
    const u32 height = m_crop.height;

    for (u32 y = 0; y < height; ++y) {
        std::copy_n(surface + size_t(m_crop.top + y) * pitch + m_crop.left, width, m_scratch.data() + size_t(y) * width);
    }

    // Integer fast paths: exact 2x/3x magnification is a byte/row expansion.
    if (width * 2 == output_width && height * 2 == output_height) {
        graphics::expand_integer<2>(m_scratch.data(), surface, pitch, width, height);
        return;
    }
    if (width * 3 == output_width && height * 3 == output_height) {
        graphics::expand_integer<3>(m_scratch.data(), surface, pitch, width, height);
        return;
    }

    for (u32 y = 0; y < output_height; ++y) {
        const u8* source_row = m_scratch.data() + size_t(m_y_lookup[y]) * width;
        u8* destination_row = surface + size_t(y) * pitch;
        for (u32 x = 0; x < output_width; ++x) {
            destination_row[x] = source_row[m_x_lookup[x]];
        }
    }
}
