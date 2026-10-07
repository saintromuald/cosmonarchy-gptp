#include "framebuffer.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../graphics/Bitmap.h"
#include "../../storm/storm.h"
#include "../../hooks/limits/sfx_extender.h"

// Screen surface lifecycle functions sized from g_display instead of 640x480.
namespace {
// Neither the engine nor Storm cares what these say beyond allocation logging.
const char* const AllocTag = "display";

// Whole-graphics init at 004DB060 calls these with no arguments.
BW_FUNC(0x00480C20, void __cdecl, pre_graphics_init);

BW_FUNC(0x0041D930, void __cdecl, ddraw_init);

BW_FUNC(0x0041D8B0, void __cdecl, ddraw_shutdown);

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

    g_scratch_buffer = SMemAlloc(0x900, AllocTag, 0x8D, 0);
}

// 004DB060. Allocates screen surface, initializes DirectDraw, palette, and sound.
void __cdecl graphics_init() {
    // 004BCB70 error record buffer; zeroed slack for reporter over-read.
    u32 sound_error[8] = {};

    pre_graphics_init();

    g_game_screen_buffer->width = g_display.surface_width();
    g_game_screen_buffer->height = g_display.surface_height();
    g_game_screen_buffer->data = nullptr;
    g_game_screen_buffer->data = (u8*)SMemAlloc(surface_bytes(), AllocTag, 0x37, 0);

    ddraw_init();

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

    ddraw_shutdown();

    if (g_game_screen_buffer->data) {
        SMemFree(g_game_screen_buffer->data, AllocTag, 0, 0);
        g_game_screen_buffer->data = nullptr;
    }
}

} // hooks
