#pragma once
#include "../../types.h"
#include "../../BW/structures/common.h"
#include "display.h"

#include <array>
#include <vector>

namespace zoom {

// 10000 = 100% base zoom; 12.5% step ladder.
constexpr u32 BaseUnits = 10000;
// Zoom is window-relative magnification: the visible world is display / zoom.
// Fixed policy, not user-configurable: players are held at the vanilla-ish
// 200% floor (no zoom-out), witnesses and replays can fill the window.
constexpr u32 MinPercent = 100;
constexpr u32 MaxPercent = 300;
constexpr u32 PlayerMinPercent = 200;
constexpr u32 WitnessMinPercent = 100;
constexpr u32 DefaultPercent = 200;
static_assert(WitnessMinPercent >= MinPercent && WitnessMinPercent <= MaxPercent);
static_assert(PlayerMinPercent >= WitnessMinPercent && PlayerMinPercent <= MaxPercent);
static_assert(DefaultPercent >= PlayerMinPercent && DefaultPercent <= MaxPercent);
// Quadratic ease-out.
constexpr u32 TransitionMs = 120;
// 12.5% per wheel notch.
constexpr u32 StepUnits = 1250;
// Integer-magnification detents the wheel ladder snaps onto: stepping across one
// lands on it exactly (an extra notch is required to pass). Perfect pixel
// lattice stops on 1080p, with row-duplication fast paths in scale_world.
constexpr u32 Detent200 = 20000;
constexpr u32 Detent300 = 30000;

// Crop rectangle in world pixels relative to camera.
struct Crop {
    u16 left = 0;
    u16 top = 0;
    u16 width = 0;
    u16 height = 0;
};

// Screen/world anchor point held under cursor during wheel zoom transition.
struct Anchor {
    bool active = false;
    v2s32 surface;
    v2s32 world;
};

} // zoom

class WorldZoom {
public:
    void init();
    // Applies the role's zoom floor (see DisplayGeometry::set_min_zoom_percent)
    // and resets to the default. Called per game start, once the
    // replay/witness role is known.
    void begin_game();
    // Config toggle: gates wheel input only (adjust_by_wheel).
    // The resting magnification presents whether or not it is set.
    bool enabled() const;
    bool magnified() const;
    // A frame must run the transform while a transition is running, an anchor
    // awaits handoff (the 100% landing frame presents 1:1 but must still
    // release the anchor), or the world is magnified.
    bool needs_transform() const;

    u32 percent() const;

    void update_transform(bool advance_animation = true);
    bool adjust_by_wheel(s32 notches, v2s32 surface_point);
    bool resolve_camera_anchor(v2s32* adjusted);

    // Camera position that places the visible crop's top-left at the given world
    // point — the inverse of the edge-aware crop slide, for the minimap
    // click-to-jump. Returns the point unchanged when the world is not magnified.
    v2s32 camera_for_visible_origin(v2s32 origin) const;

    // Converts surface coordinate to magnified world coordinate relative to camera.
    v2s32 surface_to_world_offset(v2s32 surface_point) const;

    u16 source_left() const;
    u16 source_top() const;
    u16 visible_width() const;
    u16 visible_height() const;

    // Resamples world rectangle using precomputed lookups via scratch buffer.
    void scale_world(u8* surface);

private:
    u32 floor_units() const;
    u32 ceiling_units() const;
    u32 source_extent(u32 output_extent) const;
    void advance_transition(u32 now);
    void rebuild_lookup();

    u32 m_current_units = zoom::BaseUnits;
    u32 m_transition_start_units = zoom::BaseUnits;
    u32 m_target_units = zoom::BaseUnits;
    u32 m_transition_start_tick = 0;

    zoom::Crop m_crop;
    zoom::Anchor m_anchor;

    static_assert(display::SurfaceWidth <= display::MaxWidth && display::SurfaceHeight <= display::MaxHeight);

    std::array<u16, display::MaxWidth> m_x_lookup{};
    std::array<u16, display::MaxHeight> m_y_lookup{};
    bool m_lookup_valid = false;

    std::vector<u8> m_scratch;
};

inline WorldZoom g_zoom;
