#include "framebuffer.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../graphics/Bitmap.h"
#include "../../storm/storm.h"
#include "../../hooks/limits/sfx_extender.h"

#include <algorithm>

// Screen surface lifecycle functions sized from g_display instead of 640x480.
namespace {

u32 surface_bytes() {
    return u32(g_display.surface_width()) * u32(g_display.surface_height());
}

} // namespace

namespace hooks {
// 0041E050. Clears the layer table and dirty grid, and resets screen buffer dimensions.
void __cdecl init_screen_state() {
    *g_screen_layers = Layers{};

    auto cells = g_display.grid();
    for (u32 i = 0; i < g_display.grid_bytes(); ++i) {
        cells[i] = 0;
    }

    g_game_screen_buffer->width = g_display.surface_width();
    g_game_screen_buffer->height = g_display.surface_height();
    g_game_screen_buffer->data = nullptr;

    g_scratch_buffer = SMemAlloc(0x900);
}

// 004DB060. Allocates screen surface, initializes DirectDraw, palette, and sound.
void __cdecl graphics_init() {
    // 004BCB70 error record buffer; zeroed slack for reporter over-read.
    u32 sound_error[8] = {};

    helper::pre_graphics_init();

    g_game_screen_buffer->width = g_display.surface_width();
    g_game_screen_buffer->height = g_display.surface_height();
    g_game_screen_buffer->data = nullptr;
    g_game_screen_buffer->data = (u8*)SMemAlloc(surface_bytes());

    g_world_surface.width = g_display.surface_width();
    g_world_surface.height = g_display.surface_height();
    g_world_surface.data = (u8*)SMemAlloc(surface_bytes());
    if (g_world_surface.data) {
        std::fill_n(g_world_surface.data, surface_bytes(), 0);
    }

    helper::ddraw_init();

    g_graphics_ready = 1;
    helper::register_shutdown_proc(0x004DA310);

    g_target_palette = g_init_palette;

    helper::refresh_region();

    helper::load_sfx_data(0x00515498, 0x00502C78);

    // Reads extended SFX tables (004DB0E6 / 004DB0FA) or falls back to stock 0068DAA0.
    const auto sfx_files = sfx_dat::sound_file ? u32(sfx_dat::sound_file) : 0x0068DAA0u;
    g_sfx_tbl_handle = helper::load_sfx_table(0x6AF, 0x00502D0C, NEW_SOUND_TYPE_COUNT, 0x00502C68, sfx_files, 0x004D2E40);
    helper::register_shutdown_proc(0x004BBA50);

    if (!g_sound_disabled && helper::init_directsound(sound_error, u32(g_main_window))) {
        helper::register_shutdown_proc(0x004DA780);
    }
}

// 004DA310. Exit-time teardown, registered by graphics_init
// The vanilla duplicate at 0041DD90 is unreferenced dead code, so this is the only teardown that runs
void __cdecl graphics_shutdown() {
    g_graphics_state.fill(0);
    g_graphics_ready = 0;

    helper::ddraw_shutdown();

    if (g_game_screen_buffer->data) {
        SMemFree(g_game_screen_buffer->data);
        g_game_screen_buffer->data = nullptr;
    }
    if (g_world_surface.data) {
        SMemFree(g_world_surface.data);
        g_world_surface.data = nullptr;
    }
}

} // hooks
