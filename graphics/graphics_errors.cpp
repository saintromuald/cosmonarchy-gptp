#include "graphics_errors.h"
#include "Bitmap.h"
#include "../BW/bwdata.h"

namespace graphics {

static s32 g_error_flags;

void set_error(GraphicsErrorId error) {
	g_error_flags |= error;
}

void clear_errors() {
	g_error_flags = 0;
}

void draw_error_messages() {
#ifndef NDEBUG
	auto* const target = (graphics::Bitmap*)*g_current_surface;
	if (!target) {
		return;
	}

	if (g_error_flags & ERR_UNKNOWN_SHAPE) {
		target->draw_text("Error: Unknown shape found.", 10, 70, 2);
	}

	if (g_error_flags & ERR_UNKNOWN_COORD_TYPE) {
		target->draw_text("Error: Unknown coordinate flingy_id found.", 10, 100, 2);
	}
#endif
}

}