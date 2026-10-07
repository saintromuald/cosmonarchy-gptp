#include "draw_hook.h"
#include "../BW/api.h"
#include "../hook_tools.h"
#include "../systems/display/zoom.h"
#include "graphics.h"

namespace {

static bool want_refresh = false;

void __stdcall draw_hook(graphics::Bitmap* surface, Bounds* bounds) {
	if (want_refresh) {
		want_refresh = false;
		bw::refresh_screen();
	}

	g_old_draw_game_proc(surface, bounds);

	if (g_graphics.draw_world_shapes() > 0) {
		want_refresh = true;
	}

	// Left here while the world is drawn at its own size, so the unzoomed
	// frame is composited in exactly the order it always was.
	if (!g_zoom.magnified()) {
		hooks::draw_deferred_shapes();
	}
}

}

namespace hooks {

void draw_deferred_shapes() {
	if (g_graphics.draw_screen_shapes() > 0) {
		want_refresh = true;
	}
}

void inject_draw_hook() {
	memory_patch(0x004BD68D, &draw_hook);
}

}
