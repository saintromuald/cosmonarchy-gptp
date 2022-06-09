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

static u8 mix_color(f32 base, f32 blend_shadow, f32 blend_highlight, f32 brightness) {
	f32 darkness = 1 - brightness;
	f32 color = (base * blend_shadow * darkness) + (base * blend_highlight * brightness);
	return clamp(color, 0, 1) * 255;
}

Recolor lerp_recolor(f32 t, Recolor a, Recolor b) {
	a.r     = a.r     * t + b.r     * (1 - t);
	a.g     = a.g     * t + b.g     * (1 - t);
	a.b     = a.b     * t + b.b     * (1 - t);
	a.gamma = a.gamma * t + b.gamma * (1 - t);
	return a;
}

void set_palette_recolor(Recolor shadows, Recolor highlights) {
	for (u32 i = 0; i < PaletteSize; ++i) {
		f32 brightness = std::max({init_palette->colors[i].r, init_palette->colors[i].g, init_palette->colors[i].b}) / 255.0;
		game_palette->colors[i].r = mix_color(init_palette->colors[i].r / 255.0, shadows.r * shadows.gamma, highlights.r * highlights.gamma, brightness);
		game_palette->colors[i].g = mix_color(init_palette->colors[i].g / 255.0, shadows.g * shadows.gamma, highlights.g * highlights.gamma, brightness);
		game_palette->colors[i].b = mix_color(init_palette->colors[i].b / 255.0, shadows.b * shadows.gamma, highlights.b * highlights.gamma, brightness);
	}
	helper::update_palette();
}