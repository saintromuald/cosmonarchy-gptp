#include "draw_hook.h"
#include "../BW/api.h"
#include "../hook_tools.h"
#include "graphics.h"

namespace {

static bool want_refresh = false;

void __stdcall draw_hook(graphics::Bitmap* surface, Bounds* bounds) {
	if (want_refresh) {
		want_refresh = false;
		bw::refresh_screen();
	}

	g_old_draw_game_proc(surface, bounds);

	if (g_graphics.draw_all_shapes() + g_graphics.draw_stat_res_shapes() > 0) {
		want_refresh = true;
	}
}

}

namespace hooks {

void inject_draw_hook() {
	memory_patch(0x004BD68D, &draw_hook);
}

}