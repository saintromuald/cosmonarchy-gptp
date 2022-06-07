#pragma once
#include "../SCBW/structures/common.h"
#include "graphics.h"
#include <string>

//V241 for VS2008

namespace graphics {

//Speed-efficient non-virtual struct for storing shapes that will be drawn.
//All shape setter methods merely copy data and contain no actual logic.
//All coordinates are relative to screen.
struct Shape {
    void set_text(s32 x, s32 y, s32 stringIndex, FontSize fontSize, CoordType coordType);
    void set_dot(s32 x, s32 y, ColorId color, CoordType coordType);
    void set_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType coordType);
    void set_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType);
    void set_circle(s32 x, s32 y, s32 radius, ColorId color, CoordType coordType);
    void set_filled_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType);
    void set_filled_circle(s32 x, s32 y, s32 radius, ColorId color, CoordType coordType);
    void set_ellipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType);
    void set_dotted_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType coordType);
    void set_dotted_elipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType);
    void draw() const;

      enum ShapeType {
      NONE,
      Text,
      DOT,
      LINE,
      BOX,
      CIRCLE,
      FILLED_BOX,
      FILLED_CIRCLE,
      ELLIPSE,
      DOTTED_LINE,
      DOTTED_ELLIPSE
    } type;
    CoordType coordType;
    Vector32 p1;
    Vector32 p2;
    s32 radius;
    ColorId color;
    FontSize fontSize;
    s32 stringIndex;
};

} //graphics
