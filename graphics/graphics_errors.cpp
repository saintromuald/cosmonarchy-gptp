#include "graphics_errors.h"
#include "Bitmap.h"
#include "../SCBW/scbwdata.h"

namespace graphics
{
	s32 errorFlags;
	void set_error(GraphicsErrorId error)
	{
		errorFlags |= error;
	}

	void clear_errors()
	{
		errorFlags = 0;
	}

	void draw_error_messages()
	{
#ifndef NDEBUG
		if (errorFlags & ERR_TOO_MANY_SHAPES)
		{
			gameScreenBuffer->blitString("Error: Too many shapes!", 10, 10, 2);
		}

		if (errorFlags & ERR_TOO_MANY_STRINGS)
		{
			gameScreenBuffer->blitString("Error: Too many strings!", 10, 40, 2);
		}

		if (errorFlags & ERR_UNKNOWN_SHAPE)
		{
			gameScreenBuffer->blitString("Error: Unknown shape found.", 10, 70, 2);
		}

		if (errorFlags & ERR_UNKNOWN_COORD_TYPE)
		{
			gameScreenBuffer->blitString("Error: Unknown coordinate type found.", 10, 100, 2);
		}
#endif

	}
}