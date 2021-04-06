#pragma once
#include "../../types.h"

//V241 for VS2008


namespace graphics { class Bitmap; }
struct bounds;

struct Layer {
/*00*/  uint8  buffers;
//uint8  bits;
/*01*/  uint8  hasBeenRefreshed  : 1;
/*01*/  uint8  unk               : 7;
/*02*/  uint16 left;
/*04*/  uint16 top;
/*06*/  uint16 width;
/*08*/  uint16 height;
/*0A*/  uint16 alignment;
/*0C*/  graphics::Bitmap  *pSurface;
/*10*/  void (__stdcall *pUpdate)(graphics::Bitmap *pSurface, bounds *pBounds);
};

C_ASSERT(sizeof(Layer) == 20); /*0x14*/
//static_assert(sizeof(Layer) == 20, "The size of the Layer structure is invalid");


struct Layers {
  Layer mouse;
  Layer context;
  Layer dialog;
  Layer placeBuildingA;
  Layer placeBuildingB;
  Layer game;
};
