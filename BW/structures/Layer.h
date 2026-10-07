#pragma once
#include "../../types.h"
#include "common.h"

namespace graphics { struct Bitmap; }

struct Layer {
/*00*/  u8  buffers;
/*01*/  u8  has_been_refreshed  : 1;
/*01*/  u8  unk					: 7;
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
