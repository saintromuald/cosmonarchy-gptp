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

static u8 mix_color(u8 color, f32 multiplier) {
	f32 new_color = color / 255;
	new_color *= multiplier;

	return clamp(new_color, 0, 1) * 255;
}

void set_color_palette(f32 r, f32 g, f32 b, f32 gamma) {
	for (u32 i = 0; i < PaletteSize; ++i) {
		game_palette->colors[i].r = mix_color(init_palette->colors[i].r, r * gamma);
		game_palette->colors[i].g = mix_color(init_palette->colors[i].g, g * gamma);
		game_palette->colors[i].b = mix_color(init_palette->colors[i].b, b * gamma);
	}
	helper::update_palette();
}