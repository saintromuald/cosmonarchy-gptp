#include <cassert>
#include "graphics.h"
#include "graphics_errors.h"
#include "graphics_misc.h"
#include "Shape.h"

namespace graphics {

	const s32 MAX_SHAPES = 10000;
	const s32 MAX_STRINGS = 2000;

	//Static arrays for fast drawing
	Shape shapes[MAX_SHAPES];
	std::string strings[MAX_STRINGS];
	s32 shapeCount = 0;
	s32 stringCount = 0;

	void resetAllGraphics() {
		shapeCount = 0;
		stringCount = 0;
		clearErrors();
	}


	void drawText(s32 x, s32 y, const std::string& str, FontSize fontSize, CoordType ct) {
		if (stringCount >= MAX_STRINGS) {
			setError(ERR_TOO_MANY_STRINGS);
			return;
		}
		if (shapeCount >= MAX_SHAPES) {
			setError(ERR_TOO_MANY_SHAPES);
			return;
		}
		strings[stringCount] = str;
		shapes[shapeCount++].setText(x, y, stringCount++, fontSize, ct);
	}

	void drawDot(s32 x, s32 y, ColorId color, CoordType ct) {
		if (shapeCount >= MAX_SHAPES) {
			setError(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shapeCount++].setDot(x, y, color, ct);
	}

	void drawLine(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType ct) {
		if (shapeCount >= MAX_SHAPES) {
			setError(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shapeCount++].setLine(x1, y1, x2, y2, color, ct);
	}

	void drawBox(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
		if (shapeCount >= MAX_SHAPES) {
			setError(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shapeCount++].setBox(left, top, right, bottom, color, ct);
	}

	void drawCircle(s32 x, s32 y, s32 radius, ColorId color, CoordType ct) {
		if (shapeCount >= MAX_SHAPES) {
			setError(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shapeCount++].setCircle(x, y, radius, color, ct);
	}

	void drawFilledBox(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
		if (shapeCount >= MAX_SHAPES) {
			setError(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shapeCount++].setFilledBox(left, top, right, bottom, color, ct);
	}

	void drawFilledCircle(s32 x, s32 y, s32 radius, ColorId color, CoordType ct) {
		if (shapeCount >= MAX_SHAPES) {
			setError(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shapeCount++].setFilledCircle(x, y, radius, color, ct);
	}

	void drawEllipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
		if (shapeCount >= MAX_SHAPES) {
			setError(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shapeCount++].setEllipse(left, top, right, bottom, color, ct);
	}

	void drawDottedLine(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType ct) {
		if (shapeCount >= MAX_SHAPES) {
			setError(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shapeCount++].setDottedLine(x1, y1, x2, y2, color, ct);
	}

	void drawDottedEllipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {

		if (shapeCount >= MAX_SHAPES) {
			setError(ERR_TOO_MANY_SHAPES);
			return;
		}

		shapes[shapeCount++].setDottedEllipse(left, top, right, bottom, color, ct);
	}

	const std::string& getStringFromIndex(s32 stringIndex) {
		assert(0 <= stringIndex && stringIndex < MAX_STRINGS);
		return strings[stringIndex];
	}

	s32 drawAllShapes() {
		s32 drawnShapeCount = 0;
		for (s32 i = 0; i < shapeCount; ++i)
			shapes[i].draw();
		drawErrorMessages();
		return shapeCount;  //TODO: Fix this
	}

} //graphics
