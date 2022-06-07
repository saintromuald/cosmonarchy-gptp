#include "color_palette.h"

#define PaletteSize 256
#pragma pack(push, 1)

struct RGBA {
	u8 r;
	u8 g;
	u8 b;
	u8 a;
};

struct Palette {
	RGBA colors[PaletteSize];
};

#pragma pack(pop)

static Palette* const init_palette = (Palette*)0x005994E0;
static Palette* const game_palette = (Palette*)0x006CEB40;

static f32 clamp(f32 value, f32 min, f32 max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

static void set_color(u8* color, f32 value) {
	*color = clamp(value, 0, 255);
}

void set_color_palette(f32 r, f32 g, f32 b, f32 gamma) {
	for (u32 i = 0; i < PaletteSize; ++i) {
		set_color(&game_palette->colors[i].r, init_palette->colors[i].r * r * gamma);
		set_color(&game_palette->colors[i].g, init_palette->colors[i].g * g * gamma);
		set_color(&game_palette->colors[i].b, init_palette->colors[i].b * b * gamma);
	}
	helper::update_palette();
}