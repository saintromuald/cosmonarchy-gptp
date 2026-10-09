#pragma once
#include "../../types.h"
#include "common.h"

#include <magic_enum/magic_enum_all.hpp>
using namespace magic_enum::bitwise_operators;

namespace graphics { struct Bitmap; }

enum class LayerFlags : u8 {
    None         = 0,
    NeedsRefresh = 0x01, // full redraw requested; image updates skip their own marking while it is set
    Marked       = 0x02, // update owed even if the cells read clean (mark_rect_cells, fog, reveal cheats)
    CellsDirty   = 0x04, // the present walk found dirty cells under the layer
    AlwaysUpdate = 0x20, // the update runs on every present; only the dialog layer sets this
};

struct Layer {
/*00*/  u8  buffers;
/*01*/  LayerFlags flags;
/*02*/  u16 left;
/*04*/  u16 top;
/*06*/  u16 width;
/*08*/  u16 height;
/*0A*/  u16 alignment;
/*0C*/  graphics::Bitmap  *p_surface;
        // p_bounds carries screen bounds relative to layer coordinates, followed by screen dimensions.
/*10*/  void (__stdcall *p_update)(graphics::Bitmap *p_surface, Bounds *p_bounds);
};
C_ASSERT(sizeof(Layer) == 20); /*0x14*/

struct Layers {
  Layer mouse;
  Layer context;
  Layer dialog;
  Layer place_building_a;
  Layer place_building_b;
  Layer game;
  // Walked by 0041E280 beneath the game layer; initialized with the rest of
  // the table but never populated by the engine.
  Layer spare_a;
  Layer spare_b;
};
C_ASSERT(sizeof(Layers) == 0xA0); /*0x6CEF50..0x6CEFF0; g_game_screen_buffer follows*/
