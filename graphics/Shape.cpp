#include "Shape.h"
#include "graphics_misc.h"
#include "graphics_errors.h"
#include "Bitmap.h"
#include "../SCBW/scbwdata.h"

namespace graphics
{

	//-------- Registering shapes --------//

	void Shape::set_text(s32 x, s32 y, s32 stringIndex, FontSize fontSize, CoordType coordType)
	{
		this->type = Text;
		this->p1.x = x;
		this->p1.y = y;
		this->stringIndex = stringIndex;
		this->fontSize = fontSize;
		this->coordType = coordType;
	}

	void Shape::set_dot(s32 x, s32 y, ColorId color, CoordType coordType)
	{
		this->type = DOT;
		this->p1.x = x;
		this->p1.y = y;
		this->color = color;
		this->coordType = coordType;
	}

	void Shape::set_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType coordType)
	{
		this->type = LINE;
		this->p1.x = x1;
		this->p1.y = y1;
		this->p2.x = x2;
		this->p2.y = y2;
		this->color = color;
		this->coordType = coordType;
	}

	void Shape::set_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType)
	{
		this->type = BOX;
		this->p1.x = left;
		this->p1.y = top;
		this->p2.x = right;
		this->p2.y = bottom;
		this->color = color;
		this->coordType = coordType;
	}

	void Shape::set_ellipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType)
	{
		this->type = ELLIPSE;
		this->p1.x = left;
		this->p1.y = top;
		this->p2.x = right;
		this->p2.y = bottom;
		this->color = color;
		this->coordType = coordType;
	}

	void Shape::set_dotted_elipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType)
	{
		this->type = DOTTED_ELLIPSE;
		this->p1.x = left;
		this->p1.y = top;
		this->p2.x = right;
		this->p2.y = bottom;
		this->color = color;
		this->coordType = coordType;
	}

	void Shape::set_dotted_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType coordType)
	{
		this->type = DOTTED_LINE;
		this->p1.x = x1;
		this->p1.y = y1;
		this->p2.x = x2;
		this->p2.y = y2;
		this->color = color;
		this->coordType = coordType;
	}

	void Shape::set_circle(s32 x, s32 y, s32 radius, ColorId color, CoordType coordType)
	{
		this->type = CIRCLE;
		this->p1.x = x;
		this->p1.y = y;
		this->radius = radius;
		this->color = color;
		this->coordType = coordType;
	};

	void Shape::set_filled_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType coordType)
	{
		this->type = FILLED_BOX;
		this->p1.x = left;
		this->p1.y = top;
		this->p2.x = right;
		this->p2.y = bottom;
		this->color = color;
		this->coordType = coordType;
	};

	void Shape::set_filled_circle(s32 x, s32 y, s32 radius, ColorId color, CoordType coordType)
	{
		this->type = FILLED_CIRCLE;
		this->p1.x = x;
		this->p1.y = y;
		this->radius = radius;
		this->color = color;
		this->coordType = coordType;
	};

	//-------- Drawing --------//

	void Shape::draw() const
	{
		Vector32 p1 = this->p1, p2 = this->p2;
		switch (this->coordType)
		{
			case ON_SCREEN:
				break;
			case ON_MAP:
				p1.x -= *screenX;
				p1.y -= *screenY;
				p2.x -= *screenX;
				p2.y -= *screenY;
				break;
			case ON_MOUSE:
				p1.x += mouse->x;
				p1.y += mouse->y;
				p2.x += mouse->x;
				p2.y += mouse->y;
				break;
			default:
				set_error(ERR_UNKNOWN_COORD_TYPE);
				break;
		}

		switch (this->type)
		{

			case Text:
				//TODO: Add ability to change font count
				gameScreenBuffer->blitString(
					get_string_from_index(this->stringIndex).c_str(), p1.x, p1.y, this->fontSize);
				break;

			case DOT:
				gameScreenBuffer->draw_dot(p1.x, p1.y, this->color);
				break;

			case LINE:
				gameScreenBuffer->draw_line(p1.x, p1.y, p2.x, p2.y, this->color);
				break;

			case BOX:
				gameScreenBuffer->draw_box(p1.x, p1.y, p2.x, p2.y, this->color);
				break;

			case CIRCLE:
				gameScreenBuffer->draw_circle(p1.x, p1.y, this->radius, this->color);
				break;

			case FILLED_BOX:
				gameScreenBuffer->draw_filled_box(p1.x, p1.y, p2.x, p2.y, this->color);
				break;

			case FILLED_CIRCLE:
				gameScreenBuffer->draw_filled_circle(p1.x, p1.y, this->radius, this->color);
				break;

			case ELLIPSE:
				gameScreenBuffer->draw_ellipse(p1.x, p1.y, p2.x, p2.y, this->color);
				break;

			case DOTTED_LINE:
				gameScreenBuffer->draw_dotted_line(p1.x, p1.y, p2.x, p2.y, this->color);
				break;

			case DOTTED_ELLIPSE:
				gameScreenBuffer->draw_dotted_elipse(p1.x, p1.y, p2.x, p2.y, this->color);
				break;

			default:
				set_error(ERR_UNKNOWN_SHAPE);
				return;
		}
	}

} //graphics
