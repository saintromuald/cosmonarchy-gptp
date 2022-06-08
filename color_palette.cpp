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

static u8 mix_color(f32 base, f32 blend, f32 brightness) {
	f32 darkness = 1 - brightness;

	// Overlay
	//f32 multiply = base * blend;
	//f32 screen   = 1 - (1 - base) * (1 - blend);
	//f32 color    = darkness * multiply + brightness * screen;

	f32 color = base * blend;

	return clamp(color, 0, 1) * 255;
}

void set_color_palette(f32 r, f32 g, f32 b, f32 gamma) {
	for (u32 i = 0; i < PaletteSize; ++i) {
		f32 brightness = std::max({init_palette->colors[i].r, init_palette->colors[i].g, init_palette->colors[i].b}) / 255.0;
		game_palette->colors[i].r = mix_color(init_palette->colors[i].r / 255.0, r * gamma, brightness);
		game_palette->colors[i].g = mix_color(init_palette->colors[i].g / 255.0, g * gamma, brightness);
		game_palette->colors[i].b = mix_color(init_palette->colors[i].b / 255.0, b * gamma, brightness);
	}
	helper::update_palette();
}