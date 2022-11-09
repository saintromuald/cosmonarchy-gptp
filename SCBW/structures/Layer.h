#pragma once
#include "../../types.h"

namespace graphics { struct Bitmap; }
struct bounds;

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
/*10*/  void (__stdcall *p_update)(graphics::Bitmap *p_surface, bounds *p_bounds);
};
C_ASSERT(sizeof(Layer) == 20); /*0x14*/

struct Layers {
  Layer mouse;
  Layer context;
  Layer dialog;
  Layer place_building_a;
  Layer place_building_b;
  Layer game;
};