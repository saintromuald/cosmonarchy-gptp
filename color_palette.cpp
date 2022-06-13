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

static f32 lerp(f32 t, f32 a, f32 b) {
	return a * t + b * (1 - t);
}

static u8 mix_color(f32 base, f32 blend_shadow, f32 blend_highlight, f32 saturation_shadow, f32 saturation_highlight, f32 brightness) {
	f32 darkness = 1 - brightness;
	f32 saturation = clamp((saturation_shadow * darkness) + (saturation_highlight * brightness), 0, 1);

	base = lerp(saturation, brightness, base);
	f32 color = (base * blend_shadow * darkness) + (base * blend_highlight * brightness);
	return clamp(color, 0, 1) * 255;
}

Recolor lerp_recolor(f32 t, Recolor a, Recolor b) {
	a.r          = a.r          * t + b.r          * (1 - t);
	a.g          = a.g          * t + b.g          * (1 - t);
	a.b          = a.b          * t + b.b          * (1 - t);
	a.gamma      = a.gamma      * t + b.gamma      * (1 - t);
	a.saturation = a.saturation * t + b.saturation * (1 - t);
	return a;
}

void set_palette_recolor(Recolor shadows, Recolor highlights) {
	for (u32 i = 0; i < PaletteSize; ++i) {
		f32 brightness = std::max({init_palette->colors[i].r, init_palette->colors[i].g, init_palette->colors[i].b}) / 255.0;
		game_palette->colors[i].r = mix_color(init_palette->colors[i].r / 255.0, shadows.r * shadows.gamma, highlights.r * highlights.gamma, shadows.saturation, highlights.saturation, brightness);
		game_palette->colors[i].g = mix_color(init_palette->colors[i].g / 255.0, shadows.g * shadows.gamma, highlights.g * highlights.gamma, shadows.saturation, highlights.saturation, brightness);
		game_palette->colors[i].b = mix_color(init_palette->colors[i].b / 255.0, shadows.b * shadows.gamma, highlights.b * highlights.gamma, shadows.saturation, highlights.saturation, brightness);
	}
	helper::update_palette();
}