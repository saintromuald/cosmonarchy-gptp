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
      void setText(s32 x, s32 y, s32 stringIndex, FontSize fontSize, CoordType coordType);
    void setDot(s32 x, s32 y, ColorId color, CoordType coordType);
    void setLine(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType coordType);
    void setBox(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType);
    void setCircle(s32 x, s32 y, s32 radius, ColorId color, CoordType coordType);
    void setFilledBox(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType);
    void setFilledCircle(s32 x, s32 y, s32 radius, ColorId color, CoordType coordType);
    void setEllipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType);
    void setDottedLine(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType coordType);
    void setDottedEllipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType);
    void draw() const;

      enum ShapeType {
      NONE,
      TEXT,
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
